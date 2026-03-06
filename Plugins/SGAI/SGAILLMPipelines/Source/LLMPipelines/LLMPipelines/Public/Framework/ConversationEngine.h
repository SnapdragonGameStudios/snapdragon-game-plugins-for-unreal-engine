// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Framework/LLMPipelinesTypes.h"
#include "LLMModel.h"
#include "LLMPipelinesNative/ConversationEngineNative.h"

#include "ConversationEngine.generated.h"

USTRUCT(BlueprintType)
struct LLMPIPELINES_API FConversationEngineConfig
{
    GENERATED_BODY()
  public:
    UPROPERTY(BlueprintReadWrite)
    FString SystemPrompt;
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, FString> ModelContextConfig;
    UPROPERTY(BlueprintReadWrite)
    ULLMModel *Model = nullptr;
    UPROPERTY(BlueprintReadWrite)
    TArray<FLLMToolDefinition> Tools;
};

/**
 * Blueprint wrapper for conversation engine
 */
UCLASS(BlueprintType)
class LLMPIPELINES_API UConversationEngine : public UObject
{
    GENERATED_BODY()

  public:
    /**
     * Initialize the conversation engine
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Conversation")
    void Initialize(const FConversationEngineConfig &Config, const FOnOperationComplete &OnComplete,
                    const FOnExecuteTool &OnExecuteTool, const FOnLLMTokenGenerated &OnToken);

    /**
     * Send user message with callbacks
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Conversation")
    void SendMessage(const FString &UserMessage);

    /**
     * Reset conversation
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Conversation")
    void Reset();

    /**
     * Abort current processing
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Conversation")
    void Abort();

  private:
    TUniquePtr<FConversationEngineNative> Engine;

    UPROPERTY()
    FConversationEngineConfig Config;

    std::string ExecuteToolInternal(const LLMPipelinesNative::FToolCall &ToolCall, FOnExecuteTool Executor);
};
