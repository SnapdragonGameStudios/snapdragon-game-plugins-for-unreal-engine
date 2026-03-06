// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#include "OboeAudioStream.h"

#include "CoreMinimal.h"

#if PLATFORM_ANDROID

DECLARE_LOG_CATEGORY_EXTERN(LogAndroidAudioStream, Log, All);
DEFINE_LOG_CATEGORY(LogAndroidAudioStream);

namespace SGAISpeechRecognizer
{

oboe::DataCallbackResult FOboeAudioStream::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32 numFrames)
{
    OnAudioCapture(audioData, numFrames, 0.0, false);
    return oboe::DataCallbackResult::Continue;
}

bool FOboeAudioStream::OpenStream()
{
    oboe::AudioStreamBuilder StreamBuilder;
    StreamBuilder.setDeviceId(0);
    StreamBuilder.setCallback(this);
    StreamBuilder.setDirection(oboe::Direction::Input);
    StreamBuilder.setSampleRate(SampleRate);
    StreamBuilder.setChannelCount(NumChannels);
    StreamBuilder.setFormat(oboe::AudioFormat::I16);

    oboe::AudioStream *NewStream;
    oboe::Result Result = StreamBuilder.openStream(&NewStream);

    bool bSuccess = Result == oboe::Result::OK;

    InputOboeStream.Reset(NewStream);

    if (!bSuccess)
    {
        // Log error on failure.
        FString ErrorString(UTF8_TO_TCHAR(convertToText(Result)));
        UE_LOG(LogAndroidAudioStream, Error, TEXT("Failed to open oboe capture stream: %s"), *ErrorString)
    }

    return bSuccess;
}

void FOboeAudioStream::CloseStream()
{
    if (InputOboeStream)
    {
        InputOboeStream->close();
        InputOboeStream.Reset();
    }
}

bool FOboeAudioStream::StartCapture(const IAudioStream::AudioStreamCallback &InOnCapture)
{
    if (!InputOboeStream)
    {
        return false;
    }

    OnCapture = InOnCapture;

    return InputOboeStream->requestStart() == oboe::Result::OK;
}

bool FOboeAudioStream::StopCapture()
{
    if (!InputOboeStream)
    {
        return false;
    }

    return InputOboeStream->stop() == oboe::Result::OK;
}

void FOboeAudioStream::OnAudioCapture(void *InBuffer, uint32 InBufferFrames, double StreamTime, bool bOverflow)
{
    OnCapture(static_cast<uint8 *>(InBuffer), InBufferFrames * 2);
}

} // namespace SGAISpeechRecognizer

#endif
