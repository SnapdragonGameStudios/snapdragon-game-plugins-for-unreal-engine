// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "ISpeechRecognizer.h"

#include "SpeechRecognizer.generated.h"

/**
 * @brief Delegate called when the speech recognizer initialization completes.
 * @param IsInitialized True if initialization was successful, false otherwise.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FSpeechToTextOnInitializedDelegate, bool, IsInitialized);

/**
 * @brief Delegate called when the speech recognizer disconnects.
 */
DECLARE_DYNAMIC_DELEGATE(FSpeechToTextOnDisconnectDelegate);

/**
 * @brief Delegate called when a transcription result is available.
 * @param IsFinal True if this is the final transcription result, false for interim results.
 * @param Transcription The transcribed text from the speech input.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSpeechToTextTranscriptionDelegate, const bool, IsFinal, const FString &,
                                   Transcription);

/**
 * @brief Delegate called when speech recognition has started.
 */
DECLARE_DYNAMIC_DELEGATE(FOnSpeechToTextStartedDelegate);

/**
 * @brief Delegate called when speech recognition has stopped.
 */
DECLARE_DYNAMIC_DELEGATE(FOnSpeechToTextStoppedDelegate);

/**
 * @brief Delegate called when an error occurs during speech recognition.
 * @param ErrorCode The error code indicating the type of error that occurred.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSpeechToTextErrorDelegate, const int, ErrorCode);

/** @brief Log category for speech recognizer messages. */
DECLARE_LOG_CATEGORY_EXTERN(LogSpeechRecognizer, Log, All)

/**
 * @class USpeechRecognizer
 * @brief Unreal Engine Object for speech-to-text recognition functionality.
 *
 * This class provides a Blueprint-accessible interface for converting speech audio
 * into text transcriptions. It manages the lifecycle of speech recognition sessions,
 * processes audio data, and delivers transcription results through delegate callbacks.
 *
 * The speech recognizer operates asynchronously using a background worker thread to
 * ensure non-blocking operation in the game thread.
 *
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SGAISPEECHRECOGNIZER_API USpeechRecognizer : public UObject
{
    GENERATED_BODY()

  public:
    /**
     * @brief Constructor for VTable helper (used by Unreal's reflection system).
     * @param Helper VTable helper reference.
     */
    USpeechRecognizer(FVTableHelper &Helper);

    /**
     * @brief Default constructor.
     */
    USpeechRecognizer();

    /**
     * @brief Destructor.
     */
    virtual ~USpeechRecognizer() override;

    /**
     * @brief Initializes the speech recognizer.
     *
     * This method must be called before starting speech recognition. It sets up
     * the underlying speech recognition engine and prepares it for use.
     *
     * @param OnInitialized Delegate called when initialization completes, indicating success or failure.
     */
    UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer", meta = (AutoCreateRefTerm = "ConfigurationSettings"))
    void Initialize(FSpeechRecognizerSettings Settings, const FSpeechToTextOnInitializedDelegate &OnInitialized);

    /**
     * @brief Uninitializes the speech recognizer and releases resources.
     *
     * This method should be called when speech recognition is no longer needed.
     * It stops any active recognition session and cleans up resources.
     */
    UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
    void Uninitialize();

    /**
     * @brief Called when the object begins the destruction process.
     *
     * Overrides UObject::BeginDestroy to ensure proper cleanup of speech recognizer resources.
     */
    virtual void BeginDestroy() override;

    /**
     * @brief Starts the speech recognition session.
     *
     * Begins capturing and processing audio for speech-to-text conversion.
     * Multiple delegates can be provided to handle different events during the recognition session.
     *
     * @param OnStarted Delegate called when recognition has successfully started.
     * @param OnStopped Delegate called when recognition has stopped.
     * @param OnError Delegate called if an error occurs during recognition.
     * @param OnTranscription Delegate called when transcription results are available (both interim and final).
     */
    UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
    void Start(const FOnSpeechToTextStartedDelegate &OnStarted, const FOnSpeechToTextStoppedDelegate &OnStopped,
               const FOnSpeechToTextErrorDelegate &OnError,
               const FOnSpeechToTextTranscriptionDelegate &OnTranscription);

    /**
     * @brief Stops the current speech recognition session.
     *
     * Halts audio processing and ends the recognition session. Any pending transcriptions
     * will be delivered before stopping completes.
     */
    UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
    void Stop();

    /**
     * @brief Processes raw audio data for speech recognition.
     *
     * Feeds audio data to the speech recognition engine for processing. This method
     * should be called repeatedly with audio chunks during an active recognition session.
     *
     * @param buffer Array of audio data bytes to process.
     * @return True if the audio was successfully queued for processing, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
    bool ProcessAudio(const TArray<uint8> &buffer);

    /**
     * @brief Processes raw audio data for speech recognition.
     *
     * Feeds audio data to the speech recognition engine for processing. This method
     * should be called repeatedly with audio chunks during an active recognition session.
     *
     * @param buffer Array of audio data bytes to process.
     * @return True if the audio was successfully queued for processing, false otherwise.
     */
    bool ProcessAudio(const uint8 *bytes, const uint32 numBytes);

  private:
    /**
     * @brief Creates and returns a platform-specific speech recognizer instance.
     *
     * Factory method that instantiates the appropriate ISpeechRecognizer implementation
     * based on the current platform.
     *
     * @return Unique pointer to the speech recognizer instance.
     */
    TUniquePtr<ISpeechRecognizer> GetSpeechRecognizerInstance();

    /** @brief The underlying speech recognizer implementation. */
    TUniquePtr<ISpeechRecognizer> SpeechRecognizerImpl;

    TFuture<bool> IsInitialized;
};
