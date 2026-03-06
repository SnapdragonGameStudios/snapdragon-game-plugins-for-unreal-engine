// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Framework/LLMPipelinesTypes.h"
#include "LLMPipelinesNative/IVectorDB.h"

#include "VectorDB.generated.h"

/**
 * Blueprint wrapper for IVectorDB
 */
UCLASS(BlueprintType)
class LLMPIPELINES_API UVectorDB : public UObject
{
    GENERATED_BODY()

  public:
    /**
     * Initialize the vector database
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|VectorDB")
    void Initialize(const TMap<FString, FString> &Config, FOnOperationComplete OnComplete);

    /**
     * Query the database for relevant context
     */
    UFUNCTION(BlueprintCallable, Category = "LLM|VectorDB")
    void Query(const TArray<float> &Embedding, FOnVectorDBQueryComplete OnComplete);

    // Internal access
    TSharedPtr<LLMPipelinesNative::IVectorDB> GetVectorDB() const
    {
        return VectorDB;
    }

  private:
    TSharedPtr<LLMPipelinesNative::IVectorDB> VectorDB;
};
