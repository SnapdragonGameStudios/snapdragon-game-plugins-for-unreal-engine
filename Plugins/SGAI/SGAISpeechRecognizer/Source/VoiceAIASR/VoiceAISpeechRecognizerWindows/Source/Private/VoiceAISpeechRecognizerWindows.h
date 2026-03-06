// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "ISpeechRecognizer.h"
#include "InputStream.h"
#include "Whisper.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoiceAIASRWindows, Log, All)

class VoiceAISpeechRecognizerWindows : public ISpeechRecognizer
{
    /**
     * Whisper Listener
     */
    class FWhisperResponseListenerImpl : public WhisperFunction::WhisperResponseListener
    {
        /**
         *
         */
        VoiceAISpeechRecognizerWindows *Parent;

      public:
        /**
         *
         */
        FWhisperResponseListenerImpl(VoiceAISpeechRecognizerWindows *);

      private:
        /**
         *
         */
        virtual void onTranscription(const std::map<std::string, std::string> &Results) override;

        /**
         *
         */
        virtual void onEvent(int Event) override;

        /**
         *
         */
        virtual void onError(int Error) override;
    };

    FThreadSafeBool bHasStarted = false;

    /**
     *
     */
    FWhisperResponseListenerImpl *WhisperListener = nullptr;
    /**
     *
     */
    SpeechToTextEventCallbackFn OnEventCallback;
    SpeechToTextStartedCallbackFn OnStartedCallback;
    SpeechToTextStoppedCallbackFn OnStoppedCallback;
    SpeechToTextErrorCallbackFn OnErrorCallback;
    /**
     *
     */
    WhisperFunction::Whisper *WhisperObj = nullptr;

    /**
     *
     */
    WhisperFunction::InputStream *InputStream = nullptr;

  public:
    virtual ~VoiceAISpeechRecognizerWindows() override;

    /**
     *
     */
    virtual bool Initialize(const FSpeechRecognizerSettings &Configuration) override;

    /**
     *
     */
    virtual void Destroy() override;

    /**
     *
     */
    virtual void Start(const SpeechToTextStartedCallbackFn &, const SpeechToTextStoppedCallbackFn &,
                       const SpeechToTextErrorCallbackFn &, const SpeechToTextEventCallbackFn &) override;

    void InjectSilence(float TimeInSeconds);
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
    void OnTranscription(const std::map<std::string, std::string> &Results) const;

    /**
     *
     */
    void OnEvent(int Event);

    /**
     *
     */
    void OnError(int Error);

    /**
     *
     */
    void Reset();

  private:
    /**
     *
     */
    void ForceStop();

    /**
     *
     */
    void SetConfiguration(const SpeechRecognizerConfigurationSettings &Configuration);
};
