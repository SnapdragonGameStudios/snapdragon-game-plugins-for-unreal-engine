// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * SGAITextToSpeech module interface
 */
class FSGAITextToSpeechModule : public IModuleInterface
{
  public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
