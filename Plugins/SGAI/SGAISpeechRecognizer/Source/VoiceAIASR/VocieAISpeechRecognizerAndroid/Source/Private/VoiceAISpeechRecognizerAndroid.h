// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "AudioBufferQueue.h"
#include "CoreMinimal.h"
#include "ISpeechRecognizer.h"
#include "Wrappers/WhisperJava.h"
#include "Wrappers/WhisperResponseListenerNativeProxyJava.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoiceAIASRAndroid, Log, All)

class VoiceAISpeechRecognizerAndroid : public ISpeechRecognizer, public IWhisperResponseListener, public IInputStream
{
    TUniquePtr<FWhisperResponseListenerNativeProxy> WhisperResponseListenerObj;
    TUniquePtr<FWhisperJava> WhisperObj;
    TUniquePtr<FNativeInputStreamJava> InputStreamObj;
    TUniquePtr<FAudioBufferQueue> AudioBufferQueue;

    FThreadSafeBool bHasStarted = false;

    SpeechToTextEventCallbackFn OnTranscriptionCallback;
    SpeechToTextStartedCallbackFn OnStartedCallback;
    SpeechToTextStoppedCallbackFn OnStoppedCallback;
    SpeechToTextErrorCallbackFn OnErrorCallback;

  public:
    /**
     *
     */
    virtual ~VoiceAISpeechRecognizerAndroid() override;

    /**
     *
     */
    virtual bool Initialize(const FSpeechRecognizerSettings &Settings) override;

    /**
     *
     */
    virtual void Destroy() override;

    /**
     *
     */
    virtual void Start(const SpeechToTextStartedCallbackFn &, const SpeechToTextStoppedCallbackFn &,
                       const SpeechToTextErrorCallbackFn &, const SpeechToTextEventCallbackFn &) override;

    /**
     *
     */
    virtual bool ProcessAudio(const uint8 *bytes, const uint32 numBytes) override;

    /**
     *
     */
    virtual void Stop() override;

    /**
     *
     */
    void OnEvent(int Event);

    /**
     *
     */
    void Reset();

    /**
     *
     */
    void ForceStop();

    /**
     *
     */
    void OnTranscription(const std::string &transcription, const std::string &language, bool finalized,
                         int code) override;
    /**
     *
     */
    void OnError(int errorCode) override;
    /**
     *
     */
    void OnRecordingStopped() override;
    /**
     *
     */
    void OnFinished() override;
    /**
     *
     */
    void OnSpeechStart();
    /**
     *
     */
    void OnSpeechEnd();

    /**
     *
     */
    int Read(uint8 *arr, int offset, int length) override;
};
