// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "ITextToSpeech.h"
#include "Wrappers/TTSJava.h"
#include "Wrappers/TTSResultCallbackNativeProxyJava.h"

class VoiceAITTS : public ITextToSpeech, public ITTSResultCallback
{
    TUniquePtr<FTTSResultCallbackNativeProxy> ListenerObj;
    TUniquePtr<FTTSJava> TTSObj;

    FThreadSafeBool bHasStarted = false;

    TTSStartCallbackFn OnStartCallbackFn;
    TTSAudioDataAvailableCallbackFn OnAudioDataAvailableCallbackFn;
    TTSCompleteCallbackFn OnCompleteCallbackFn;
    TTSErrorCallbackFn OnErrorCallbackFn;

  public:
    virtual ~VoiceAITTS() override;

    virtual bool Initialize(const FTTSConfigurationSettings &ConfigurationSettings) override;

    virtual void Destroy() override;

    virtual void Speak(const FString &, const TTSStartCallbackFn &, const TTSAudioDataAvailableCallbackFn &,
                       const TTSCompleteCallbackFn &, const TTSErrorCallbackFn &) override;

    virtual void Stop() override;

  private:
    void Reset();
    void OnAudioAvailable(const uint8 *, int size) override;
    void OnError(int errorCode) override;
    void OnStart(int, int, int) override;
    void OnDone() override;
};
