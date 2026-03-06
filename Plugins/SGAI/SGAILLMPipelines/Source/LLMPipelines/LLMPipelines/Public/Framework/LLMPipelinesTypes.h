// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "LLMPipelinesTypes.generated.h"

/**
 * Blueprint-friendly enum for LLM sentence codes
 */
UENUM(BlueprintType)
enum class ELLMSentenceCode : uint8
{
    Complete = 0 UMETA(DisplayName = "Complete"),
    Begin = 1 UMETA(DisplayName = "Begin"),
    Continue = 2 UMETA(DisplayName = "Continue"),
    End = 3 UMETA(DisplayName = "End"),
    Abort = 4 UMETA(DisplayName = "Abort")
};

/**
 * Message structure for Blueprint
 */
USTRUCT(BlueprintType)
struct LLMPIPELINES_API FLLMMessage
{
    GENERATED_BODY()

    /** Role: "system", "user", or "assistant" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
    FString Role;

    /** Message content */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
    FString Content;

    FLLMMessage() : Role(TEXT("user")), Content(TEXT(""))
    {
    }

    FLLMMessage(const FString &InRole, const FString &InContent) : Role(InRole), Content(InContent)
    {
    }
};

/**
 * Tool definition structure for Blueprint
 */
USTRUCT(BlueprintType)
struct LLMPIPELINES_API FLLMToolDefinition
{
    GENERATED_BODY()

    /** Tool name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
    FString Name;

    /** Tool description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
    FString Description;

    /** Parameters JSON schema */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
    FString ParametersJson;

    FLLMToolDefinition() : Name(TEXT("")), Description(TEXT("")), ParametersJson(TEXT("{}"))
    {
    }
};

/**
 * Blueprint-friendly tool call structure
 * Mirrors FToolCall but with USTRUCT for Blueprint exposure
 */
USTRUCT(BlueprintType)
struct LLMPIPELINES_API FToolCall
{
    GENERATED_BODY()

    /** Name of the tool to call */
    UPROPERTY(BlueprintReadOnly, Category = "Tool Call")
    FString ToolName;

    /** Arguments as JSON string */
    UPROPERTY(BlueprintReadOnly, Category = "Tool Call")
    FString ArgumentsJson;

    FToolCall() : ToolName(TEXT("")), ArgumentsJson(TEXT(""))
    {
    }

    FToolCall(const FString &InName, const FString &InArgs) : ToolName(InName), ArgumentsJson(InArgs)
    {
    }
};

/**
 * Structure for RAG query results
 */
USTRUCT(BlueprintType)
struct LLMPIPELINES_API FVectorDBQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "RAG")
    FString RetrievedText;

    UPROPERTY(BlueprintReadOnly, Category = "RAG")
    float SimilarityScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "RAG")
    bool bSuccess = false;
};

/**
 * Delegate for token generation callbacks
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLLMTokenGenerated, ELLMSentenceCode, SentenceCode, FString, Token);

/**
 * Delegate for tool call requests
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FString, FOnLLMToolCall, FString, ToolName, FString, ToolArguments);

/**
 * Generic operation complete (with success flag)
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnOperationComplete, bool, bSuccess);

/**
 * Simple completion (no params)
 */
DECLARE_DYNAMIC_DELEGATE(FOnComplete);

/**
 * Tool execution callback
 * Takes tool call as input and returns response string via output parameter
 * Return "ERROR: message" for errors, or the result string for success
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnExecuteTool, FToolCall, ToolCall, FString &, OutResponse);

/**
 * Embedding result callback
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnEmbeddingComplete, const TArray<float> &, Embedding, bool, bSuccess);

/**
 * RAG query result callback
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVectorDBQueryComplete, FVectorDBQueryResult, Result);
