// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#include "SGAITextToSpeechModule.h"
#include "TextToSpeechEngine.h"


#define LOCTEXT_NAMESPACE "FSGAITextToSpeechModule"

void FSGAITextToSpeechModule::StartupModule()
{
}

void FSGAITextToSpeechModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSGAITextToSpeechModule, SGAITextToSpeech)
