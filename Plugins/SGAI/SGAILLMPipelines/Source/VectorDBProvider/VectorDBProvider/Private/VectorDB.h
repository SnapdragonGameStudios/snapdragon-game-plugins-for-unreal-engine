// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "AnnoyVectorStore.h"
#include "BasicChunkStore.h"
#include "LLMPipelinesNative/IVectorDB.h"
#include <memory>

class VectorDB : public LLMPipelinesNative::IVectorDB
{
    AnnoyVectorStore vectorStore;
    BasicChunkStore chunkStore;

  public:
    VectorDB() = default;
    bool Init(const std::map<std::string, std::string> &config) override;
    LLMPipelinesNative::Chunk Query(const LLMPipelinesNative::VectorEmbedding &) const override;
    virtual ~VectorDB() override;
};
