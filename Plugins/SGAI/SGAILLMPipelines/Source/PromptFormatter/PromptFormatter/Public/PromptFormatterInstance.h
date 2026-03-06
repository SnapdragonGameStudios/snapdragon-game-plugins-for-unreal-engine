// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "LLMPipelinesNative/IPromptFormatter.h"
#include <memory>

namespace PromptFormatter
{
PROMPTFORMATTER_API std::unique_ptr<LLMPipelinesNative::IPromptFormatter> CreateInstance();
}
