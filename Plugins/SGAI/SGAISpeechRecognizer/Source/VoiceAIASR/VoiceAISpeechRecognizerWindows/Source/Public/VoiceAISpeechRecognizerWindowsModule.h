// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ISpeechRecognizerModuleInterface.h"

class FVoiceAISpeechRecognizerWindowsModule : public ISpeechRecognizerModuleInterface
{
    void *WhisperComponentLibraryHandle = nullptr;
    void *WhisperLibLibraryHandle = nullptr;

  public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual TUniquePtr<ISpeechRecognizer> GetSpeechRecognizer() override;
};
