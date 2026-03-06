// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "ProviderHelpers.h"
#include "IProviderModuleInterface.h"
#include "Modules/ModuleManager.h"

std::unique_ptr<LLMPipelinesNative::ILLMModel> FProviderHelpers::CreateLLMModel(const TMap<FString, FString> &Config)
{
    std::map<std::string, std::string> config;
    FString ModuleName = "";
    if (IProviderModuleInterface *ProviderModule = GetProviderModule(ModuleName))
    {
        // Call creation method
        std::unique_ptr<LLMPipelinesNative::ILLMModel> Model = ProviderModule->CreateLLMModel(config);
        if (!Model)
        {
            UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Module '%s' does not provide LLM models"), *ModuleName);
            return nullptr;
        }

        return Model;
    }

    return nullptr;
}

std::unique_ptr<LLMPipelinesNative::IEmbedder> FProviderHelpers::CreateEmbedder(const TMap<FString, FString> &Config)
{
    std::map<std::string, std::string> config;
    FString ModuleName = "";
    if (IProviderModuleInterface *ProviderModule = GetProviderModule(ModuleName))
    {
        // Call creation method
        std::unique_ptr<LLMPipelinesNative::IEmbedder> Embedder = ProviderModule->CreateEmbedder(config);
        if (!Embedder)
        {
            UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Module '%s' does not provide embedders"), *ModuleName);
            return nullptr;
        }

        return Embedder;
    }

    return nullptr;
}

std::unique_ptr<LLMPipelinesNative::IVectorDB> FProviderHelpers::CreateVectorDB(const TMap<FString, FString> &Config)
{
    std::map<std::string, std::string> config;
    FString ModuleName = "";
    if (IProviderModuleInterface *ProviderModule = GetProviderModule(ModuleName))
    {
        // Call creation method
        std::unique_ptr<LLMPipelinesNative::IVectorDB> VectorDB = ProviderModule->CreateVectorDB(config);
        if (!VectorDB)
        {
            UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Module '%s' does not provide vector databases"),
                   *ModuleName);
            return nullptr;
        }

        return VectorDB;
    }

    return nullptr;
}

IProviderModuleInterface *FProviderHelpers::GetProviderModule(const FString &ModuleName)
{
    FModuleManager &ModuleManager = FModuleManager::Get();

    // Check if module exists
    if (!ModuleManager.ModuleExists(*ModuleName))
    {
        UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Module '%s' does not exist"), *ModuleName);
        return nullptr;
    }

    // Load module if not already loaded
    if (!ModuleManager.IsModuleLoaded(*ModuleName))
    {
        if (!ModuleManager.LoadModule(*ModuleName))
        {
            UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Failed to load module '%s'"), *ModuleName);
            return nullptr;
        }
    }

    // Get module interface
    IModuleInterface *ModuleInterface = ModuleManager.GetModule(*ModuleName);
    if (!ModuleInterface)
    {
        UE_LOG(LogTemp, Error, TEXT("FProviderHelpers: Failed to get module interface for '%s'"), *ModuleName);
        return nullptr;
    }

    return static_cast<IProviderModuleInterface *>(ModuleInterface);
}
