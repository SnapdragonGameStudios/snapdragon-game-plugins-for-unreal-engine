// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ITextToSpeechModuleInterface.h"

class FVoiceAITTSModule : public ITextToSpeechModuleInterface
{
  public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    TUniquePtr<ITextToSpeech> GetTTS() override;
};
