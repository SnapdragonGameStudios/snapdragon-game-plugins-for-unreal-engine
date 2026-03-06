// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "DefaultToolCallParser.h"
#include <nlohmann/json.hpp>

bool FDefaultToolCallParser::Init()
{
    Reset();
    return true;
}

LLMPipelinesNative::FToolCall FDefaultToolCallParser::ParseToolCall(const std::string &JsonStdStr)
{
    LLMPipelinesNative::FToolCall Result;

    if (!JsonStdStr.empty())
    {
        // Parse JSON
        auto JsonObj = nlohmann::json::parse(JsonStdStr, nullptr, false);
        if (!JsonObj.is_discarded())
        {
            // Check for required fields
            if (JsonObj.contains("name") && JsonObj.contains("arguments"))
            {
                // Extract tool name
                if (JsonObj["name"].is_string())
                {
                    Result.ToolName = JsonObj["name"];

                    // Extract arguments (convert back to JSON string)
                    if (JsonObj["arguments"].is_object() || JsonObj["arguments"].is_array())
                    {
                        Result.ArgumentsJson = JsonObj["arguments"].dump();
                    }
                    else if (JsonObj["arguments"].is_string())
                    {
                        // Some models might provide arguments as a string
                        Result.ArgumentsJson = JsonObj["arguments"];
                    }
                }
            }
        }
    }
    return Result;
}

void FDefaultToolCallParser::Reset()
{
    ParseState = EParseState::Unknown;
    CurrentToolCallStr.clear();
    ToolCalls.clear();
}

LLMPipelinesNative::IToolCallParser::EParseState FDefaultToolCallParser::ProcessToken(
    LLMPipelinesNative::ESentenceCode code, const std::string &Token)
{
    std::vector<std::string>::iterator it;

    switch (ParseState)
    {
    case EParseState::NoToolCall:
        break;

    case EParseState::Unknown:
        it = std::find(ToolCallStartToken.begin(), ToolCallStartToken.end(), Token);
        ParseState = (it != ToolCallStartToken.end()) ? EParseState::ToolCallExists : EParseState::NoToolCall;
        break;

    case EParseState::ToolCallExists:
        it = std::find(ToolCallStartToken.begin(), ToolCallStartToken.end(), Token);
        if (it != ToolCallStartToken.end())
        {
            CurrentToolCallStr = "";
        }
        else
        {
            it = std::find(ToolCallEndToken.begin(), ToolCallEndToken.end(), Token);
            if (it != ToolCallEndToken.end() || (code == LLMPipelinesNative::ESentenceCode::SENTENCECODE_END) ||
                (code == LLMPipelinesNative::ESentenceCode::SENTENCECODE_ABORT))
            {
                LLMPipelinesNative::FToolCall toolCall = ParseToolCall(CurrentToolCallStr);
                if (toolCall.IsValid())
                {
                    ToolCalls.push_back(toolCall);
                }
                CurrentToolCallStr = "";
            }
            else
            {
                CurrentToolCallStr += Token;
            }
        }
        break;
    }

    return ParseState;
}

LLMPipelinesNative::IToolCallParser::EParseState FDefaultToolCallParser::GetCurrentState()
{
    return ParseState;
}

std::vector<LLMPipelinesNative::FToolCall> FDefaultToolCallParser::GetToolRequests()
{
    return ToolCalls;
}

FDefaultToolCallParser::~FDefaultToolCallParser()
{
    Reset();
}
