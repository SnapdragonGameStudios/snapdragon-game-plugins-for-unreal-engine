// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VectorDBProviderModule.h"
#include "Modules/ModuleManager.h"
#include "VectorDB.h"

#define LOCTEXT_NAMESPACE "VectorDBProviderModule"

std::unique_ptr<LLMPipelinesNative::ILLMModel> VectorDBProviderModule::CreateLLMModel(
    const std::map<std::string, std::string> &config)
{
    return nullptr;
}
std::unique_ptr<LLMPipelinesNative::IEmbedder> VectorDBProviderModule::CreateEmbedder(
    const std::map<std::string, std::string> &config)
{
    return nullptr;
}
std::unique_ptr<LLMPipelinesNative::IVectorDB> VectorDBProviderModule::CreateVectorDB(
    const std::map<std::string, std::string> &config)
{
    std::unique_ptr<LLMPipelinesNative::IVectorDB> vectordb = std::make_unique<VectorDB>();
    if ((vectordb != nullptr) && vectordb->Init(config))
    {
        return vectordb;
    }
    return nullptr;
}

void VectorDBProviderModule::StartupModule()
{
}

void VectorDBProviderModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(VectorDBProviderModule, VectorDBProvider)
