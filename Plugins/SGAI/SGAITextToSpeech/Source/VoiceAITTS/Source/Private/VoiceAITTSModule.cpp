// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VoiceAITTSModule.h"
#include "Modules/ModuleManager.h"
#include "VoiceAITTS.h"

#define LOCTEXT_NAMESPACE "FVoiceAITTSModule"

TUniquePtr<ITextToSpeech> FVoiceAITTSModule::GetTTS()
{
    return MakeUnique<VoiceAITTS>();
}

void FVoiceAITTSModule::StartupModule()
{
}

void FVoiceAITTSModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoiceAITTSModule, VoiceAITTS)
