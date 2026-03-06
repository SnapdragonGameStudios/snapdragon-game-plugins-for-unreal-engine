// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

#define LOG(string) (GenieLLMInternal::Log(GenieLLMInternal::LOGLEVEL::SEVERE, (string)))

namespace GenieLLMInternal
{
enum LOGLEVEL
{
    SEVERE = 1,
    WARN = 2,
    INFO = 3,
    VERBOSE = 4
};

void Log(GenieLLMInternal::LOGLEVEL level, const std::string &str);
} // namespace GenieLLMInternal
