// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ISpeechRecognizer.h"
#include "Modules/ModuleManager.h"

//
//
class ISpeechRecognizerModuleInterface : public IModuleInterface
{
  public:
    //
    //
    virtual TUniquePtr<ISpeechRecognizer> GetSpeechRecognizer() = 0;
};
