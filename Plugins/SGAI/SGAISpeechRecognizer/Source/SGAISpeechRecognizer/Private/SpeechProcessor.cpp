// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "SpeechProcessor.h"

#if PLATFORM_WINDOWS
#include "AudioStream/VoiceCaptureAudioStream.h"
#elif PLATFORM_ANDROID
#include "AudioStream/OboeAudioStream.h"
#endif

//
//
bool USpeechProcessor::Initialize(USpeechRecognizer *speechRecgonizer)
{
    if (SpeechRecognizerObj == nullptr)
    {
        GEngine->Exec(GetWorld(), TEXT("voice.MicNoiseGateThreshold  0"));
        GEngine->Exec(GetWorld(), TEXT("voice.SilenceDetectionThreshold 0"));

        this->SpeechRecognizerObj = speechRecgonizer;
#if PLATFORM_WINDOWS
        audioStream = MakeUnique<SGAISpeechRecognizer::FVoiceCaptureAudioStream>();
#elif PLATFORM_ANDROID
        audioStream = MakeUnique<SGAISpeechRecognizer::FOboeAudioStream>();
#endif
        audioStream->OpenStream();
        return true;
    }

    return false;
}

//
//
void USpeechProcessor::Uninitialize()
{
    if (audioStream != nullptr)
    {
        audioStream->CloseStream();
    }
}

//
//
void USpeechProcessor::StartCapture(const FOnSpeechToTextStartedDelegate &OnStarted,
                                    const FOnSpeechToTextStoppedDelegate &OnStopped,
                                    const FOnSpeechToTextErrorDelegate &OnError,
                                    const FOnSpeechToTextTranscriptionDelegate &OnTranscription)
{
    if (SpeechRecognizerObj)
    {
        SpeechRecognizerObj->Start(OnStarted, OnStopped, OnError, OnTranscription);
    }

    if (audioStream != nullptr)
    {
        audioStream->StartCapture([this](const uint8 *bytes, const uint32 numBytes) {
            if (SpeechRecognizerObj)
            {
                SpeechRecognizerObj->ProcessAudio(bytes, numBytes);
            }
        });
    }
}

//
//
void USpeechProcessor::StopCapture()
{
    if (audioStream != nullptr)
    {
        audioStream->StopCapture();
    }

    if (SpeechRecognizerObj != nullptr)
    {
        SpeechRecognizerObj->Stop();
    }
}

//
//
void USpeechProcessor::BeginDestroy()
{
    Uninitialize();

    Super::BeginDestroy();
}

//
//
void USpeechProcessor::FinishDestroy()
{
    audioStream.Reset();
    Super::FinishDestroy();
}
