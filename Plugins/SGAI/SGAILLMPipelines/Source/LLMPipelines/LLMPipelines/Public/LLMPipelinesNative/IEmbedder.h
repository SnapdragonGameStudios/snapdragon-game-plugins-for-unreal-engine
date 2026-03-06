// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace LLMPipelinesNative
{
typedef std::vector<float> VectorEmbedding;
typedef std::function<void(const VectorEmbedding &)> VectorEmbeddingCallback;

class IEmbedder
{
  public:
    virtual bool Init(const std::map<std::string, std::string> &configurations) = 0;
    virtual VectorEmbedding Generate(const std::string &input, bool bNormalize) const = 0;
    virtual void Destroy() = 0;
    virtual ~IEmbedder() = default;
};
} // namespace LLMPipelinesNative
