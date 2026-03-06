// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "GenieLogUtils.h"
#include "Log.h"
#include <vector>

namespace GenieLLMInternal
{

GenieLog_Level_t getLogLevel(const std::string &levelStr)
{
    GenieLog_Level_t logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE;
    if (levelStr == "verbose")
    {
        logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE;
    }
    else if (levelStr == "warn")
    {
        logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_WARN;
    }
    else if (levelStr == "info")
    {
        logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_INFO;
    }
    else if (levelStr == "error")
    {
        logLevel = GenieLog_Level_t::GENIE_LOG_LEVEL_ERROR;
    }

    return logLevel;
}

std::string getLogLevelString(GenieLog_Level_t logLevel)
{
    switch (logLevel)
    {
    case GenieLog_Level_t::GENIE_LOG_LEVEL_ERROR:
        return "ERROR";
    case GenieLog_Level_t::GENIE_LOG_LEVEL_WARN:
        return "WARN";
    case GenieLog_Level_t::GENIE_LOG_LEVEL_INFO:
        return "INFO";
    case GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE:
        return "VERBOSE";
    }
    return "";
}

LOGLEVEL getLogLevel(const GenieLog_Level_t &logLevel)
{
    switch (logLevel)
    {
    case GenieLog_Level_t::GENIE_LOG_LEVEL_ERROR:
        return LOGLEVEL ::SEVERE;
    case GenieLog_Level_t::GENIE_LOG_LEVEL_WARN:
        return LOGLEVEL::WARN;
    case GenieLog_Level_t::GENIE_LOG_LEVEL_INFO:
        return LOGLEVEL::INFO;
    case GenieLog_Level_t::GENIE_LOG_LEVEL_VERBOSE:
        return LOGLEVEL::VERBOSE;
    }
    return LOGLEVEL::SEVERE;
}

void __GenieLogCallback(const GenieLog_Handle_t handle, const char *fmt, GenieLog_Level_t level, uint64_t timestamp,
                        va_list args)
{
    std::string output;
    va_list args2;
    va_copy(args2, args);

    int len = vsnprintf(nullptr, 0, fmt, args);

    if (len > 0)
    {
        std::vector<char> buffer(len + 1);
        vsnprintf(buffer.data(), buffer.size(), fmt, args2);
        output = std::string(buffer.begin(), buffer.end());
    }

    GenieLLMInternal::Log(getLogLevel(level), output);
}

} // namespace GenieLLMInternal
