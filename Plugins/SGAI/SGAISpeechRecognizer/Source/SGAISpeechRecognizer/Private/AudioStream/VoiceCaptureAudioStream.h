// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "IAudioStream.h"
#include "Interfaces/VoiceCapture.h"

namespace SGAISpeechRecognizer
{
class SGAISPEECHRECOGNIZER_API FVoiceCaptureAudioStream : public SGAISpeechRecognizer::IAudioStream, public FRunnable
{
    //
    FCriticalSection CS;
    //
    SGAISpeechRecognizer::IAudioStream::AudioStreamCallback OnDataAvailable = nullptr;

    //
    FEvent *event = nullptr;

    //
    FRunnableThread *thread = nullptr;

    //
    FThreadSafeBool bExit = false;

    //
    FThreadSafeBool bStopCapture = false;

  public:
    FVoiceCaptureAudioStream() = default;
    virtual ~FVoiceCaptureAudioStream() override;

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;

    virtual bool OpenStream() override;
    virtual void CloseStream() override;
    virtual bool StopCapture() override;
    virtual bool StartCapture(const SGAISpeechRecognizer::IAudioStream::AudioStreamCallback &func) override;
};
} // namespace SGAISpeechRecognizer
