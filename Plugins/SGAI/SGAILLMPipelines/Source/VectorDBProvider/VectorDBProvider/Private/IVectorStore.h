// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#include "LLMPipelinesNative/IEmbedder.h"
#include <string>
#include <vector>

typedef std::pair<float, int> VectorQueryResult;

class IVectorStore
{
  public:
    virtual bool Init(const std::string &) = 0;
    virtual std::vector<VectorQueryResult> Query(const LLMPipelinesNative::VectorEmbedding &input,
                                                 unsigned int count) const = 0;
    virtual VectorQueryResult Query(const LLMPipelinesNative::VectorEmbedding &input) const = 0;
    virtual ~IVectorStore() = default;
};
