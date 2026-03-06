// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/IEmbedder.h"
#include <memory>
#include <string>

namespace LLMPipelinesNative
{
typedef std::pair<float, std::string> Chunk;

class IVectorDB
{
  public:
    virtual bool Init(const std::map<std::string, std::string> &config) = 0;
    virtual Chunk Query(const LLMPipelinesNative::VectorEmbedding &) const = 0;
    virtual ~IVectorDB() = default;
};
} // namespace LLMPipelinesNative
