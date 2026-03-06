// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#if BUILD_FOR_UE
#include "QAIRTPlatformUtils.h"
#endif
#include <string>

namespace GenieLLMInternal
{
#if BUILD_FOR_UE
//-----------------------------------------------------------------------------
//
//
class FScopedDllDirectory
{
  public:
    FScopedDllDirectory() : FScopedDllDirectory(UQAIRTPlatformUtils::GetBinariesPath())
    {
    }
    FScopedDllDirectory(const FString &DllDir) : DllDir(DllDir)
    {
        FPlatformProcess::PushDllDirectory(*DllDir);
    }
    ~FScopedDllDirectory()
    {
        FPlatformProcess::PopDllDirectory(*DllDir);
    }

  private:
    FString DllDir;
};
#endif

std::string LoadFileToString(std::string filename);

} // namespace GenieLLMInternal
