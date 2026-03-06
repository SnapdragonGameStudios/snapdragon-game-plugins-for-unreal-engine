// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "GenieLLMProvider.h"
#include "GenieEmbedder.h"
#include "GenieLLM.h"

namespace GenieLLMInternal
{
std::unique_ptr<LLMPipelinesNative::ILLMModel> CreateLLMModel(const std::map<std::string, std::string> &config)
{
    auto model = std::make_unique<GenieLLMInternal::GenieLLMModel>();
    if ((model != nullptr) && model->Init(config))
    {
        return model;
    }
    return nullptr;
}
std::unique_ptr<LLMPipelinesNative::IEmbedder> CreateEmbedder(const std::map<std::string, std::string> &config)
{
    auto embedder = std::make_unique<GenieLLMInternal::GenieEmbedder>();
    if ((embedder != nullptr) && embedder->Init(config))
    {
        return embedder;
    }
    return nullptr;
}
} // namespace GenieLLMInternal
