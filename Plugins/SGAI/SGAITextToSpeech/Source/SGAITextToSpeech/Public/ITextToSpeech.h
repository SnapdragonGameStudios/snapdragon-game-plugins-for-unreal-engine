// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "TextToSpeechTypes.h"
#include <functional>

using TTSAudioDataAvailableCallbackFn = std::function<void(const uint8 *, const uint32)>;
using TTSErrorCallbackFn = std::function<void(const int)>;
using TTSStartCallbackFn = std::function<void(int, int, int)>;
using TTSCompleteCallbackFn = std::function<void()>;

// Interface for text-to-speech functionality.
// This abstract class defines the methods required for text-to-speech operations.
class ITextToSpeech
{
  public:
    virtual ~ITextToSpeech() = 0;

    // Initializes the text-to-speech system with the specified configuration.
    // \param ConfigurationSettings The configuration settings to apply.
    // \return true if initialization was successful; false otherwise.
    virtual bool Initialize(const FTTSConfigurationSettings &ConfigurationSettings) = 0;

    virtual void Destroy() = 0;

    // Returns the audio data for the given text string, through the OnAudioDataAvailable Callback
    // \param Text The text to be spoken.
    virtual void Speak(const FString &, const TTSStartCallbackFn &, const TTSAudioDataAvailableCallbackFn &,
                       const TTSCompleteCallbackFn &, const TTSErrorCallbackFn &) = 0;

    // Stops any ongoing speech synthesis.
    virtual void Stop() = 0;
};
