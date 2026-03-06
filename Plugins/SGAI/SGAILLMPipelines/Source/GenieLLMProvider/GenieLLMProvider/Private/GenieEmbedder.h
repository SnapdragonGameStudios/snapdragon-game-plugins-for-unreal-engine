// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "GenieEmbedding.h"
#include "LLMPipelinesNative/IEmbedder.h"
#include <functional>

namespace GenieLLMInternal
{
void __EmbeddingCallback(const uint32_t *dimensions, const uint32_t rank, const float *embeddingBuffer,
                         const void *userData);

class GenieEmbedder : public LLMPipelinesNative::IEmbedder
{
    friend void __EmbeddingCallback(const uint32_t *dimensions, const uint32_t rank, const float *embeddingBuffer,
                                    const void *userData);

  private:
    GenieEmbedding_Handle_t mGenieEmbeddingHandle = nullptr;
    LLMPipelinesNative::VectorEmbeddingCallback OnGenerated = nullptr;
    GenieLog_Handle_t m_loggerHandle = nullptr;

  public:
    bool Init(const std::map<std::string, std::string> &configurations) override;
    LLMPipelinesNative::VectorEmbedding Generate(const std::string &input, bool bNormalize = true) const override;
    void Destroy() override;
    ~GenieEmbedder() override;
};
} // namespace GenieLLMInternal
