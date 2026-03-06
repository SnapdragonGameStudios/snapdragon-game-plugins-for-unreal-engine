// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Modules/ModuleManager.h"

//
//
class FSGAISpeechRecognizerModule : public IModuleInterface
{
  public:
    //
    //
    virtual void StartupModule() override;

    //
    //
    virtual void ShutdownModule() override;
};
