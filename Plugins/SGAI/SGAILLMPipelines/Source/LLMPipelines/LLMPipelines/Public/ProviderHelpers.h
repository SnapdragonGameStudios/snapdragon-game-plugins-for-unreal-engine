// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "IProviderModuleInterface.h"
#include <memory>

/**
 * Helper functions for loading provider modules and creating provider instances.
 *
 * CAUTION: Naming a module that isn't an implementation of IProviderModuleInterface will trigger UB.
 * CAUTION: Call these helpers from the GAME THREAD ONLY (necessary for loading modules).
 */
class LLMPIPELINES_API FProviderHelpers
{
  public:
    /**
     * Create an LLM model instance from the specified provider module
     * @param ModuleName The name of the provider module (e.g., "GenieLLMProvider")
     * @return Pointer to LLM model instance, or nullptr if module doesn't support LLMs or fails to load
     */
    static std::unique_ptr<LLMPipelinesNative::ILLMModel> CreateLLMModel(const TMap<FString, FString> &);

    /**
     * Create an embedder instance from the specified provider module
     * @param ModuleName The name of the provider module (e.g., "GenieLLMProvider")
     * @return Pointer to embedder instance, or nullptr if module doesn't support embedders or fails to load
     */
    static std::unique_ptr<LLMPipelinesNative::IEmbedder> CreateEmbedder(const TMap<FString, FString> &);

    /**
     * Create a vector database instance from the specified provider module
     * @param ModuleName The name of the provider module (e.g., "VectorDBProvider")
     * @return Pointer to vector DB instance, or nullptr if module doesn't support vector databases or fails to load
     */
    static std::unique_ptr<LLMPipelinesNative::IVectorDB> CreateVectorDB(const TMap<FString, FString> &);

    static IProviderModuleInterface *GetProviderModule(const FString &ModuleName);
};
