// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VoiceAISpeechRecognizerAndroidModule.h"

#include "Modules/ModuleManager.h"
#include "VoiceAISpeechRecognizerAndroid.h"

#define LOCTEXT_NAMESPACE "FVoiceAISpeechRecognizerAndroidModule"

TUniquePtr<ISpeechRecognizer> FVoiceAISpeechRecognizerAndroidModule::GetSpeechRecognizer()
{
    return MakeUnique<VoiceAISpeechRecognizerAndroid>();
}

void FVoiceAISpeechRecognizerAndroidModule::StartupModule()
{
}

void FVoiceAISpeechRecognizerAndroidModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoiceAISpeechRecognizerAndroidModule, VoiceAISpeechRecognizerAndroid)
