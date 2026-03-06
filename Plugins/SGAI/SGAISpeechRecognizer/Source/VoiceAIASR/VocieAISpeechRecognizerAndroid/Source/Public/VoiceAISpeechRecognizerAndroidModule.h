// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ISpeechRecognizerModuleInterface.h"

class FVoiceAISpeechRecognizerAndroidModule : public ISpeechRecognizerModuleInterface
{
  public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    TUniquePtr<ISpeechRecognizer> GetSpeechRecognizer() override;
};
