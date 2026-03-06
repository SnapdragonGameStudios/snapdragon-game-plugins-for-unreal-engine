// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "IPromptFormatter.h"
#include "IToolCallParser.h"
#include "LLMPipelinesNative/ILLM.h"
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * Configuration for conversation engine
 */

/**
 * Native conversation engine
 * Combines PromptBuilder + LLM + ToolCallParser into unified system
 * Generic implementation that works with any ILLMModelContext
 */
class LLMPIPELINES_API FConversationEngineNative
{
  public:
    struct FConfig
    {
        std::string SystemPrompt;
        std::string ToolDefinitions;
        std::map<std::string, std::string> ModelContextConfig;
    };

    /**
     * Conversation message structure
     */
    struct FMessage
    {
        /**
         * Message role types
         */
        enum class ERole
        {
            System,
            User,
            Assistant,
            Tool
        };

        ERole Role;
        std::string Content;

        FMessage(ERole InRole, const std::string &InContent) : Role(InRole), Content(InContent)
        {
        }
    };

    using ToolExecutorCallback = std::function<std::string(const LLMPipelinesNative::FToolCall &)>;

    /**
     * Callback for streaming tokens
     */
    using TokenStreamCallback = std::function<void(LLMPipelinesNative::ESentenceCode, const std::string &)>;

    ~FConversationEngineNative();

    /**
     * Init the conversation engine
     */
    bool Init(LLMPipelinesNative::ILLMModel *Model, const FConversationEngineNative::FConfig &Config,
              ToolExecutorCallback ToolExecutor, TokenStreamCallback TokenCallback);

    bool SendMessage(const std::string &UserMessage);

    void Reset();
    /**
     * Abort current processing
     */
    void Abort();

    /**
     * Destroy and cleanup resources
     */
    void Destroy();

  private:
    // Core components
    std::unique_ptr<LLMPipelinesNative::IPromptFormatter> PromptBuilder;
    std::unique_ptr<LLMPipelinesNative::IToolCallParser> ToolCallParser;
    std::unique_ptr<LLMPipelinesNative::ILLMModelContext> LLMContext;

    FConfig Configuration;
    std::string AccumulatedTokens = "";
    bool bIsFirstQuery = true;

    // Current query state
    ToolExecutorCallback CurrentToolExecutor;
    TokenStreamCallback CurrentTokenCallback;

  private:
    void OnToken(LLMPipelinesNative::ESentenceCode Code, const std::string &Token);
    void OnToken_NoToolCall(LLMPipelinesNative::ESentenceCode Code, const std::string &Token);
    bool SendQueryInternal(const std::vector<FConversationEngineNative::FMessage> &messages,
                           TokenStreamCallback callback);
    std::vector<FConversationEngineNative::FMessage> ProcessToolCalls(
        const std::vector<LLMPipelinesNative::FToolCall> &);
};
