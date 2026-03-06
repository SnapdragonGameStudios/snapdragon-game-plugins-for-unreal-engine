// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "SGAISpeechRecognizerModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FSGAISpeechRecognizerModule"

//
//
void FSGAISpeechRecognizerModule::StartupModule()
{
}

//
//
void FSGAISpeechRecognizerModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSGAISpeechRecognizerModule, SGAISpeechRecognizer)
