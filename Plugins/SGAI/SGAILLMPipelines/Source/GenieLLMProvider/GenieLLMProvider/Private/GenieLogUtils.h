// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "GenieLog.h"
#include "Log.h"

namespace GenieLLMInternal
{

GenieLLMInternal::LOGLEVEL getLogLevel(const GenieLog_Level_t &);

GenieLog_Level_t getLogLevel(const std::string &levelStr);

std::string getLogLevelString(GenieLog_Level_t logLevel);

void __GenieLogCallback(const GenieLog_Handle_t handle, const char *fmt, GenieLog_Level_t level, uint64_t timestamp,
                        va_list args);
} // namespace GenieLLMInternal
