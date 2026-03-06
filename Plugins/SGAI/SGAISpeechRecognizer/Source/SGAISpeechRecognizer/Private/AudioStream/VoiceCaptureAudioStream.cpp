// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VoiceCaptureAudioStream.h"

#include "AudioCaptureCore.h"
#include "Interfaces/VoiceCapture.h"
#include "SpeechRecognizer.h"
#include "VoiceModule.h"

namespace SGAISpeechRecognizer
{
constexpr int PCM_SAMPLE_RATE = 16000;
constexpr int NUM_CHANNELS = 1;
constexpr float VOICE_CAPTURE_DELAY = 0.06f;

FVoiceCaptureAudioStream::~FVoiceCaptureAudioStream()
{
    CloseStream();
}

//
//
bool FVoiceCaptureAudioStream::Init()
{

    return true;
}

//
//
void FVoiceCaptureAudioStream::Stop()
{
    bExit = true;
    bStopCapture = true;
    if (event != nullptr)
    {
        event->Trigger();
    }
}

//
// Called after thread has finished. Performing cleanup
void FVoiceCaptureAudioStream::Exit()
{
    bExit = false;
    bStopCapture = false;
}

bool FVoiceCaptureAudioStream::OpenStream()
{
    bExit = false;
    bStopCapture = false;

    if (thread == nullptr)
    {
        event = FPlatformProcess::GetSynchEventFromPool(false);
        if (event != nullptr)
        {
            thread = FRunnableThread::Create(this, TEXT("VoiceCaptureAudioStreamThread"));
        }
    }

    return true;
}

void FVoiceCaptureAudioStream::CloseStream()
{
    Stop();
    if (thread != nullptr)
    {
        thread->WaitForCompletion();
        delete thread;
        thread = nullptr;
    }

    FPlatformProcess::ReturnSynchEventToPool(event);
    event = nullptr;
}

//
// StartCapture
bool FVoiceCaptureAudioStream::StartCapture(const IAudioStream::AudioStreamCallback &func)
{
    FScopeLock Lock(&CS);
    this->OnDataAvailable = func;
    bStopCapture = false;
    event->Trigger();

    return true;
}

//
//
bool FVoiceCaptureAudioStream::StopCapture()
{
    bStopCapture = true;
    return true;
}

uint32 ReadAllBytes(TSharedPtr<IVoiceCapture, ESPMode::ThreadSafe> &VoiceCapture, TArray<uint8> &VoiceData,
                    const int VoiceDataSize, IAudioStream::AudioStreamCallback &OnDataAvailableCallback)
{
    uint32 BytesAvailable = 0;

    EVoiceCaptureState::Type captureState = VoiceCapture->GetCaptureState(BytesAvailable);
    if (captureState == EVoiceCaptureState::Type::Ok)
    {
        int32 Remaining = BytesAvailable;
        while (Remaining > 0)
        {
            uint32 BytesRead = 0;
            EVoiceCaptureState::Type state = VoiceCapture->GetVoiceData(VoiceData.GetData(), VoiceDataSize, BytesRead);
            if ((EVoiceCaptureState::Type::Ok == state) && (BytesRead > 0))
            {
                VoiceData.SetNum(BytesRead, EAllowShrinking::No);
                if (OnDataAvailableCallback != nullptr)
                {
                    OnDataAvailableCallback(VoiceData.GetData(), BytesRead);
                }
                Remaining -= BytesRead;
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}

//
//
uint32 FVoiceCaptureAudioStream::Run()
{
    TArray<uint8> VoiceData;
    TSharedPtr<IVoiceCapture, ESPMode::ThreadSafe> VoiceCapture =
        FVoiceModule::Get().CreateVoiceCapture("", PCM_SAMPLE_RATE, NUM_CHANNELS);

    while (true)
    {
        event->Wait();

        if (bExit)
        {
            break;
        }

        SGAISpeechRecognizer::IAudioStream::AudioStreamCallback OnDataAvailableCallback = nullptr;

        {
            FScopeLock Lock(&CS);
            OnDataAvailableCallback = this->OnDataAvailable;
        }

        {
            if (VoiceCapture != nullptr)
            {
                uint32 VoiceDataSize = VoiceCapture->GetBufferSize();
                VoiceData.SetNum(VoiceDataSize);
                VoiceCapture->Start();

                while (true)
                {
                    if (bExit || bStopCapture)
                    {
                        break;
                    }

                    ReadAllBytes(VoiceCapture, VoiceData, VoiceDataSize, OnDataAvailableCallback);
                    FPlatformProcess::Sleep(VOICE_CAPTURE_DELAY);
                }

                ReadAllBytes(VoiceCapture, VoiceData, VoiceDataSize, OnDataAvailableCallback);
                VoiceCapture->Stop();
            }
        }
    }

    return 0;
}
} // namespace SGAISpeechRecognizer
