// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "LLMPipelinesNative/ConversationEngineNative.h"
#include "DefaultToolCallParser.h"
#include "PromptFormatterInstance.h"
#include "nlohmann/json.hpp"

namespace
{
namespace ConversationEngineUtils
{
std::string GetModelPath(LLMPipelinesNative::ILLMModel *Model)
{
    std::string path = "";
    std::map<std::string, std::string> config = Model->GetConfiguration();
    if (config.find("config") != config.end())
    {
        path = config.at("config");
    }
    return path;
}

std::string RoleToString(FConversationEngineNative::FMessage::ERole Role)
{
    switch (Role)
    {
    case FConversationEngineNative::FMessage::ERole::System:
        return "system";
    case FConversationEngineNative::FMessage::ERole::User:
        return "user";
    case FConversationEngineNative::FMessage::ERole::Assistant:
        return "assistant";
    case FConversationEngineNative::FMessage::ERole::Tool:
        return "tool";
    default:
        return "user";
    }
}

std::string ConvertMessagesToJson(const std::vector<FConversationEngineNative::FMessage> &Msgs)
{
    // Build messages array using nlohmann/json as input to prompt formatter
    nlohmann::json messagesArray = nlohmann::json::array();

    for (const auto &Msg : Msgs)
    {
        nlohmann::json messageObj;
        messageObj["role"] = RoleToString(Msg.Role);
        messageObj["content"] = Msg.Content;

        messagesArray.push_back(messageObj);
    }

    return messagesArray.dump();
}

std::vector<FConversationEngineNative::FMessage> GetMessages(bool bIsFirstQuery, const std::string &UserPrompt,
                                                             const std::string &SystemPrompt)
{
    std::vector<FConversationEngineNative::FMessage> Messages;
    if (bIsFirstQuery)
    {
        Messages.push_back(
            FConversationEngineNative::FMessage(FConversationEngineNative::FMessage::ERole::System, SystemPrompt));
        Messages.push_back(
            FConversationEngineNative::FMessage(FConversationEngineNative::FMessage::ERole::User, UserPrompt));
    }
    else
    {
        Messages.push_back(
            FConversationEngineNative::FMessage(FConversationEngineNative::FMessage::ERole::User, UserPrompt));
    }
    return Messages;
}

} // namespace ConversationEngineUtils
} // namespace

FConversationEngineNative::~FConversationEngineNative()
{
    Destroy();
}

bool FConversationEngineNative::Init(LLMPipelinesNative::ILLMModel *Model,
                                     const FConversationEngineNative::FConfig &Config,
                                     ToolExecutorCallback ToolExecutor, TokenStreamCallback TokenCallback)
{
    Configuration = Config;
    CurrentToolExecutor = ToolExecutor;
    CurrentTokenCallback = TokenCallback;

    if (Model != nullptr)
    {
        LLMContext = Model->CreateContext(Configuration.ModelContextConfig);
        if (LLMContext != nullptr)
        {
            PromptBuilder = PromptFormatter::CreateInstance();
            if (PromptBuilder->Init(ConversationEngineUtils::GetModelPath(Model)))
            {
                if (!Config.ToolDefinitions.empty())
                {
                    ToolCallParser = std::make_unique<FDefaultToolCallParser>();
                }
            }
            return true;
        }
    }
    return false;
}

void FConversationEngineNative::OnToken(LLMPipelinesNative::ESentenceCode Code, const std::string &Token)
{
    LLMPipelinesNative::IToolCallParser::EParseState CurrentState =
        (ToolCallParser != nullptr) ? ToolCallParser->GetCurrentState()
                                    : LLMPipelinesNative::IToolCallParser::EParseState::NoToolCall;
    switch (CurrentState)
    {
    case LLMPipelinesNative::IToolCallParser::EParseState::ToolCallExists:
        ToolCallParser->ProcessToken(Code, Token);
        break;
    case LLMPipelinesNative::IToolCallParser::EParseState::Unknown:
        AccumulatedTokens += Token;
        ToolCallParser->ProcessToken(Code, Token);
        if (ToolCallParser->GetCurrentState() == LLMPipelinesNative::IToolCallParser::EParseState::NoToolCall)
        {
            if (CurrentTokenCallback)
            {
                CurrentTokenCallback(Code, AccumulatedTokens);
                AccumulatedTokens = "";
            }
        }
        break;
    case LLMPipelinesNative::IToolCallParser::EParseState::NoToolCall:
        if (CurrentTokenCallback)
        {
            CurrentTokenCallback(Code, Token);
        }
        break;
    }
}

void FConversationEngineNative::OnToken_NoToolCall(LLMPipelinesNative::ESentenceCode Code, const std::string &Token)
{
    if (CurrentTokenCallback)
    {
        CurrentTokenCallback(Code, Token);
    }
}

bool FConversationEngineNative::SendQueryInternal(const std::vector<FConversationEngineNative::FMessage> &messages,
                                                  TokenStreamCallback Callback)
{
    std::string Prompt =
        PromptBuilder->Format(ConversationEngineUtils::ConvertMessagesToJson(messages), Configuration.ToolDefinitions);

    // Reset tool call parser
    if (ToolCallParser != nullptr)
    {
        ToolCallParser->Reset();
    }
    AccumulatedTokens.clear();

    return LLMContext->Query(Prompt, Callback);
}

bool FConversationEngineNative::SendMessage(const std::string &UserMessage)
{
    std::string Prompt = "";
    std::vector<FConversationEngineNative::FMessage> messages;

    messages = ConversationEngineUtils::GetMessages(bIsFirstQuery, UserMessage, Configuration.SystemPrompt);

    if (!messages.empty())
    {
        bIsFirstQuery = false;

        // Send initial query to LLM with user message
        {
            SendQueryInternal(messages, [this](LLMPipelinesNative::ESentenceCode Code, const std::string &Token) {
                OnToken(Code, Token);
            });
        }

        if (ToolCallParser != nullptr)
        {
            // Check if LLM requested any tool calls in its response
            std::vector<LLMPipelinesNative::FToolCall> ToolCalls = ToolCallParser->GetToolRequests();
            if (ToolCalls.size() > 0)
            {
                // Send follow-up query with tool execution results
                messages = ProcessToolCalls(ToolCalls);
                SendQueryInternal(messages, [this](LLMPipelinesNative::ESentenceCode Code, const std::string &Token) {
                    OnToken_NoToolCall(Code, Token);
                });
            }
        }

        return true;
    }
    return false;
}

std::vector<FConversationEngineNative::FMessage> FConversationEngineNative::ProcessToolCalls(
    const std::vector<LLMPipelinesNative::FToolCall> &ToolCalls)
{
    std::vector<FConversationEngineNative::FMessage> responses;
    for (const LLMPipelinesNative::FToolCall &Tool : ToolCalls)
    {
        // Execute tool via user callback - returns string response
        responses.push_back(FConversationEngineNative::FMessage(FConversationEngineNative::FMessage::ERole::Tool,
                                                                CurrentToolExecutor(Tool)));
    }
    return responses;
}

void FConversationEngineNative::Reset()
{
    if (LLMContext != nullptr)
    {
        LLMContext->Reset();
    }

    if (ToolCallParser != nullptr)
    {
        ToolCallParser->Reset();
    }
    AccumulatedTokens.clear();
    bIsFirstQuery = true;
}

void FConversationEngineNative::Abort()
{
    if (LLMContext != nullptr)
    {
        LLMContext->Abort();
    }

    if (ToolCallParser != nullptr)
    {
        ToolCallParser->Reset();
    }
}

void FConversationEngineNative::Destroy()
{
    if (LLMContext != nullptr)
    {
        LLMContext.reset();
    }

    if (PromptBuilder != nullptr)
    {
        PromptBuilder.reset();
    }

    if (ToolCallParser != nullptr)
    {
        ToolCallParser.reset();
    }
    AccumulatedTokens.clear();
    bIsFirstQuery = true;
}
