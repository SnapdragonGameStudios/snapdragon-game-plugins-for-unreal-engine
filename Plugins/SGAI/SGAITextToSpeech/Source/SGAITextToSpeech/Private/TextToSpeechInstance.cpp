// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#include "ITextToSpeechModuleInterface.h"
#include "TextToSpeechEngine.h"

TUniquePtr<ITextToSpeech> UTextToSpeechEngine::GetTextToSpeechInstance()
{
    ITextToSpeechModuleInterface *recognizerModule = nullptr;

#if PLATFORM_ANDROID
    recognizerModule = (ITextToSpeechModuleInterface *)(FModuleManager::Get().LoadModule("VoiceAITTS"));
#endif

    if (recognizerModule != nullptr)
    {
        return recognizerModule->GetTTS();
    }

    return nullptr;
}
