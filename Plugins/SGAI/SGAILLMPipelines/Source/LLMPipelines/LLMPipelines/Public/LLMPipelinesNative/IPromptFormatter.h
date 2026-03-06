// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

namespace LLMPipelinesNative
{
class IPromptFormatter
{
  public:
    virtual bool Init(const std::string &path) = 0;
    virtual std::string Format(const std::string &messages, const std::string &tools = "") = 0;
    virtual ~IPromptFormatter() = default;
};
} // namespace LLMPipelinesNative
