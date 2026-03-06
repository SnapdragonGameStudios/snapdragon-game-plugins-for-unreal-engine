// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "IProviderModuleInterface.h"

class GenieLLMProviderModule : public IProviderModuleInterface
{
    void *LibraryHandle = nullptr;

  public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual std::unique_ptr<LLMPipelinesNative::ILLMModel> CreateLLMModel(
        const std::map<std::string, std::string> &config) override;
    virtual std::unique_ptr<LLMPipelinesNative::IEmbedder> CreateEmbedder(
        const std::map<std::string, std::string> &config) override;
    virtual std::unique_ptr<LLMPipelinesNative::IVectorDB> CreateVectorDB(
        const std::map<std::string, std::string> &config) override;
};
