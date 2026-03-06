// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/IEmbedder.h"
#include "LLMPipelinesNative/ILLM.h"
#include "LLMPipelinesNative/IVectorDB.h"
#include <memory>

namespace GenieLLMInternal
{
std::unique_ptr<LLMPipelinesNative::ILLMModel> CreateLLMModel(const std::map<std::string, std::string> &config);

std::unique_ptr<LLMPipelinesNative::IEmbedder> CreateEmbedder(const std::map<std::string, std::string> &config);

} // namespace GenieLLMInternal
