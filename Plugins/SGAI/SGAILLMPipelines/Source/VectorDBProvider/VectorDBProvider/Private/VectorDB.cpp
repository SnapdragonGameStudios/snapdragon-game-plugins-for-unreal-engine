// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VectorDB.h"

using namespace LLMPipelinesNative;

bool VectorDB::Init(const std::map<std::string, std::string> &config)
{
    bool bInitialized = false;

    if (config.find("config") != config.end())
    {
        std::string path = config.at("config");
        bInitialized = vectorStore.Init(path + "\\vector_store.ann") && chunkStore.Init(path + "\\chunks.bin");
    }

    return bInitialized;
}

Chunk VectorDB::Query(const VectorEmbedding &embedding) const
{
    Chunk chunk = std::make_pair(0.0f, "");
    {
        std::vector<VectorQueryResult> results = vectorStore.Query(embedding, 1);
        if (results.size() > 0)
        {
            VectorQueryResult emd = results[0];
            chunk = std::make_pair(emd.first, chunkStore.Get(emd.second));
        }
    }

    return chunk;
}

VectorDB::~VectorDB()
{
}
