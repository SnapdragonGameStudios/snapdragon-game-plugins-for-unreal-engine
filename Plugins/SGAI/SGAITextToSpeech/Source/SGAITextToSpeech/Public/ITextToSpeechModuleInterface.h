// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ITextToSpeech.h"
#include "Modules/ModuleManager.h"

class ITextToSpeechModuleInterface : public IModuleInterface
{
  public:
    virtual TUniquePtr<ITextToSpeech> GetTTS() = 0;
};
