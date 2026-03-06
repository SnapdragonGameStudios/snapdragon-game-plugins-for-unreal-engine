// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "GenieEmbedder.h"
#include "GenieConfigUtils.h"
#include "GenieLogUtils.h"
#include <algorithm>
#include <iostream>
#include <numeric>

using namespace LLMPipelinesNative;

namespace GenieLLMInternal
{

// ** helpers **

void NormalizeEmbedding(VectorEmbedding &embedding)
{
    float sumSq = std::accumulate(embedding.begin(), embedding.end(), 0.0f,
                                  [](float running, float elt) { return running + elt * elt; });

    float norm = std::sqrt(sumSq);

    std::transform(embedding.begin(), embedding.end(), embedding.begin(), [norm](float val) { return val / norm; });
}

void __EmbeddingCallback(const uint32_t *dimensions, const uint32_t rank, const float *embeddingBuffer,
                         const void *userData)
{
    VectorEmbedding *vectorEmbedding = (VectorEmbedding *)(userData);
    if (vectorEmbedding != nullptr)
    {
        VectorEmbedding embedding(embeddingBuffer, embeddingBuffer + dimensions[1]);
        *vectorEmbedding = embedding;
    }
}

// ** GenieEmbedder implementation **

bool GenieEmbedder::Init(const std::map<std::string, std::string> &configurations)
{
    GenieEmbeddingConfig_Handle_t configHandle = nullptr;
    mGenieEmbeddingHandle = nullptr;

    std::string config = "";
    bool bLog = false;
    GenieLog_Level_t logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE;

    if (configurations.find("config") != configurations.end())
    {
        config = GenieLLMInternal::GetPatchedConfig(configurations.at("config") + "\\" + "embedder.json");
    }
    if (configurations.find("logging") != configurations.end())
    {
        logLevel = GenieLLMInternal::getLogLevel(configurations.at("logging"));
        bLog = true;
    }

    if (GENIE_STATUS_SUCCESS == GenieEmbeddingConfig_createFromJson(config.c_str(), &configHandle))
    {
        if (bLog && (GenieLog_create(nullptr, GenieLLMInternal::__GenieLogCallback, logLevel, &m_loggerHandle)))
        {
            GenieEmbeddingConfig_bindLogger(configHandle, m_loggerHandle);
        }

        if (GENIE_STATUS_SUCCESS != GenieEmbedding_create(configHandle, &mGenieEmbeddingHandle))
        {
            LOG("Failed to create Genie Embedding!");
            mGenieEmbeddingHandle = nullptr;
        }

        GenieEmbeddingConfig_free(configHandle);
    }
    else
    {
        LOG("GenieEmbedder: Failed to parse config file.");
    }

    return (mGenieEmbeddingHandle != nullptr);
}

void GenieEmbedder::Destroy()
{
    GenieEmbedding_free(mGenieEmbeddingHandle);
    mGenieEmbeddingHandle = nullptr;
    GenieLog_free(m_loggerHandle);
    m_loggerHandle = nullptr;
}

GenieEmbedder::~GenieEmbedder()
{
    GenieEmbedder::Destroy();
}

VectorEmbedding GenieEmbedder::Generate(const std::string &input, bool normalize) const
{
    VectorEmbedding outputEmbedding;
    GenieEmbedding_generate(mGenieEmbeddingHandle, input.c_str(), __EmbeddingCallback, (void *)(&outputEmbedding));
    if (normalize)
    {
        NormalizeEmbedding(outputEmbedding);
    }
    return outputEmbedding;
}

} // namespace GenieLLMInternal
