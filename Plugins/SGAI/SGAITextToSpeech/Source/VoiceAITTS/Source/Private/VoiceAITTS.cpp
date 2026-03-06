// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VoiceAITTS.h"

#include "CoreMinimal.h"
#include "Wrappers/TTSJava.h"

VoiceAITTS::~VoiceAITTS()
{
    Destroy();
}

//
//
bool VoiceAITTS::Initialize(const FTTSConfigurationSettings &ConfigurationSetting)
{
    Reset();
    bool bResult = false;

    if (TTSObj == nullptr)
    {
        ListenerObj = MakeUnique<FTTSResultCallbackNativeProxy>(this);
        TTSObj = FTTSJava::NewInstance(ListenerObj.Get());

        // call all configs

        //  TTS.newInstance(ListenerObj->GetJObject());
        //   IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectDir() / "models"))
        std::map<std::string, std::string> m;
        bResult = TTSObj->Init(m);

        if (bResult == false)
        {
            UE_LOG(LogTemp, Error, TEXT("TTS Init Failed"));
            Reset();
        }
    }
    return bResult;
    // success;
}

void VoiceAITTS::Destroy()
{
    Reset();
}

void VoiceAITTS::Reset()
{
    if (TTSObj != nullptr)
    {
        TTSObj->Stop();
        TTSObj->DeInit();
        TTSObj.Reset();
    }

    if (ListenerObj != nullptr)
    {
        ListenerObj.Reset(nullptr);
    }
}

void VoiceAITTS::OnAudioAvailable(const uint8 *bytes, int size)
{
    if (OnAudioDataAvailableCallbackFn)
    {
        OnAudioDataAvailableCallbackFn(bytes, size);
    }
}

void VoiceAITTS::OnError(int errorCode)
{
    if (OnErrorCallbackFn)
    {
        OnErrorCallbackFn(errorCode);
    }
}

void VoiceAITTS::OnStart(int a, int b, int c)
{
    if (OnStartCallbackFn)
    {
        OnStartCallbackFn(a, b, c);
    }
}

void VoiceAITTS::OnDone()
{
    if (OnCompleteCallbackFn)
    {
        OnCompleteCallbackFn();
    }
}

//
//
void VoiceAITTS::Speak(const FString &Text, const TTSStartCallbackFn &StartFn,
                       const TTSAudioDataAvailableCallbackFn &DataFn, const TTSCompleteCallbackFn &CompleteFn,
                       const TTSErrorCallbackFn &ErrorFn)
{
    this->OnStartCallbackFn = StartFn;
    this->OnAudioDataAvailableCallbackFn = DataFn;
    this->OnErrorCallbackFn = ErrorFn;
    this->OnCompleteCallbackFn = CompleteFn;

    if (TTSObj != nullptr)
    {
        TTSObj->Speak(Text);
    }
}

//
//
void VoiceAITTS::Stop()
{
    if (TTSObj != nullptr)
    {
        TTSObj->Stop();
    }
}
