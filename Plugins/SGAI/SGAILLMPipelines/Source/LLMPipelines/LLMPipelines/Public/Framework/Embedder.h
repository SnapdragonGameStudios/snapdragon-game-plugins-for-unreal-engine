// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Framework/LLMPipelinesTypes.h"
#include "LLMPipelinesNative/IEmbedder.h"

#include "Embedder.generated.h"

/**
 * Blueprint wrapper for IEmbedder
 */
UCLASS(BlueprintType)
class LLMPIPELINES_API UEmbedder : public UObject
{
    GENERATED_BODY()

  public:
    /**
     * Initialize the embedder
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Embedder")
    void Initialize(const TMap<FString, FString> &Config, FOnOperationComplete OnComplete);

    /**
     * Generate embedding for text
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Embedder")
    void Generate(const FString &Text, bool bNormalize, FOnEmbeddingComplete OnComplete);

    /**
     * Get embedding dimension
     */
    UFUNCTION(BlueprintPure, Category = "LLM|Embedder")
    int32 Dimension() const
    {
        return EmbeddingDimension;
    }

    /**
     * Destroy the embedder
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|Embedder")
    void Destroy();

    // Internal access for C++ code
    TSharedPtr<LLMPipelinesNative::IEmbedder> GetEmbedder() const
    {
        return Embedder;
    }

  private:
    TSharedPtr<LLMPipelinesNative::IEmbedder> Embedder;
    int32 EmbeddingDimension = 0;
};
