// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Log.h"
#include <iostream>

namespace GenieLLMInternal
{

void Log(const std::string &str)
{
    Log(LOGLEVEL::SEVERE, str);
}

void Log(GenieLLMInternal::LOGLEVEL level, const std::string &str)
{
#ifdef LLMAGENTS__BUILD_FOR_UE
    switch (level)
    {
    case LOGLEVEL::SEVERE:
        UE_LOG(LogTemp, Error, TEXT("%hs"), str.c_str());
        break;
    case LOGLEVEL::WARN:
        UE_LOG(LogTemp, Warning, TEXT("%hs"), str.c_str());
        break;
    case LOGLEVEL::INFO:
        UE_LOG(LogTemp, Verbose, TEXT("%hs"), str.c_str());
        break;
    case LOGLEVEL::VERBOSE:
        UE_LOG(LogTemp, VeryVerbose, TEXT("%hs"), str.c_str());
        break;
    }
#else
    std::cout << str << std::endl;
#endif
}
} // namespace GenieLLMInternal
