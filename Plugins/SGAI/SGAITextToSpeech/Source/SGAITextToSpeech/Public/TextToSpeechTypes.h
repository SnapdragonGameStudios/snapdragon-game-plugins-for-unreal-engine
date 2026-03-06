// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TextToSpeechTypes.generated.h"

/**
 * Audio encoding formats supported by TextToSpeech
 */
UENUM(BlueprintType)
enum class ETTSAudioEncoding : uint8
{
    LINEAR16 UMETA(DisplayName = "Linear PCM 16-bit"),
    MP3 UMETA(DisplayName = "MP3"),
    OGG_OPUS UMETA(DisplayName = "OGG Opus"),
    MULAW UMETA(DisplayName = "Mu-law"),
    ALAW UMETA(DisplayName = "A-law")
};

/**
 * Supported languages for TextToSpeech
 */
UENUM(BlueprintType)
enum class ETTSLanguage : uint8
{
    English UMETA(DisplayName = "English"),
    Chinese UMETA(DisplayName = "Chinese"),
    Spanish UMETA(DisplayName = "Spanish")
};

/**
 * TextToSpeech error codes
 */
UENUM(BlueprintType)
enum class ETTSErrorCode : uint8
{
    Success UMETA(DisplayName = "Success"),
    InitializationFailed UMETA(DisplayName = "Initialization Failed"),
    InvalidParameter UMETA(DisplayName = "Invalid Parameter"),
    NotInitialized UMETA(DisplayName = "Not Initialized"),
    AlreadyRunning UMETA(DisplayName = "Already Running"),
    InternalError UMETA(DisplayName = "Internal Error"),
    PlatformNotSupported UMETA(DisplayName = "Platform Not Supported")
};

/**
 * Configuration structure for TextToSpeech engine
 */
USTRUCT(BlueprintType)
struct FTTSConfigurationSettings
{
    GENERATED_BODY()

    /** Path to the TextToSpeech model files */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech")
    FString Model;

    /** Language for speech synthesis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech")
    ETTSLanguage Language = ETTSLanguage::English;

    /** Audio encoding format */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech")
    ETTSAudioEncoding AudioEncoding = ETTSAudioEncoding::LINEAR16;

    /** Speech rate (0.25 to 4.0, default 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech", meta = (ClampMin = "0.25", ClampMax = "4.0"))
    float SpeechRate = 1.0f;

    /** Pitch adjustment (-20.0 to 20.0, default 0.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech",
              meta = (ClampMin = "-20.0", ClampMax = "20.0"))
    float Pitch = 0.0f;

    /** Volume gain in dB (-96.0 to 16.0, default 0.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech",
              meta = (ClampMin = "-96.0", ClampMax = "16.0"))
    float VolumeGain = 0.0f;

    /** Sample rate in Hz (default 44100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextToSpeech")
    int32 SampleRate = 44100;

    FTTSConfigurationSettings()
        : Model(TEXT("")), Language(ETTSLanguage::English), AudioEncoding(ETTSAudioEncoding::LINEAR16),
          SpeechRate(1.0f), Pitch(0.0f), VolumeGain(0.0f), SampleRate(44100)
    {
    }
};

/**
 * Audio format information
 */
USTRUCT(BlueprintType)
struct FTextToSpeechAudioFormat
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TextToSpeech")
    int32 SampleRate = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TextToSpeech")
    int32 AudioFormat = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TextToSpeech")
    int32 ChannelCount = 0;

    FTextToSpeechAudioFormat() : SampleRate(0), AudioFormat(0), ChannelCount(0)
    {
    }

    FTextToSpeechAudioFormat(int32 InSampleRate, int32 InAudioFormat, int32 InChannelCount)
        : SampleRate(InSampleRate), AudioFormat(InAudioFormat), ChannelCount(InChannelCount)
    {
    }
};
