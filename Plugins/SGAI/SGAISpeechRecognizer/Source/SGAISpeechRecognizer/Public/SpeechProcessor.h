// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "IAudioStream.h"
#include "SpeechRecognizer.h"

#include "SpeechProcessor.generated.h"

/**
 * @class USpeechProcessor
 * @brief High-level processor that integrates voice capture and speech recognition.
 *
 * USpeechProcessor provides a unified interface for capturing audio from a microphone
 * and processing it through the speech recognition engine. It manages the lifecycle
 * of both the voice capture worker and coordinating audio data flow from the worker
 * to the recognizer.
 *
 * This class acts as a facade that simplifies the speech-to-text workflow by:
 * - Managing the voice capture worker thread
 * - Coordinating audio data transfer to the speech recognizer
 * - Providing Blueprint-accessible methods for easy integration
 *
 * @see USpeechRecognizer
 * @see SGAISpeechRecognizer::VoiceCaptureWorker
 *
 * Example usage:
 * @code
 * USpeechProcessor* Processor = NewObject<USpeechProcessor>();
 * Processor->Initialize(SpeechRecognizerInstance);
 * Processor->StartCapture(OnStarted, OnStopped, OnError, OnTranscription);
 * // ... later ...
 * Processor->StopCapture();
 * Processor->Uninitialize();
 * @endcode
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SGAISPEECHRECOGNIZER_API USpeechProcessor : public UObject
{
    GENERATED_BODY()

  public:
    /**
     * @brief Initializes the speech processor with a speech recognizer instance.
     *
     * Sets up the speech processor by associating it with a speech recognizer object.
     * This method must be called before starting audio capture. The speech recognizer
     * should already be initialized before being passed to this method.
     *
     * @param speechRecgonizer Pointer to an initialized USpeechRecognizer instance that
     *                         will process the captured audio data.
     *
     * @return True if initialization was successful, false otherwise.
     *
     * @note The speech processor maintains a reference to the provided speech recognizer.
     * @note This method should only be called once per processor instance.
     *
     * @warning Ensure the speech recognizer remains valid for the lifetime of this processor.
     *
     * @see Uninitialize()
     * @see USpeechRecognizer::Initialize()
     */
    UFUNCTION(BlueprintCallable)
    bool Initialize(USpeechRecognizer *speechRecgonizer);

    /**
     * @brief Uninitializes the speech processor and releases resources.
     *
     * Stops any active audio capture, cleans up the voice capture worker, and
     * releases the reference to the speech recognizer. This method should be called
     * when speech processing is no longer needed.
     *
     * @note This method is safe to call multiple times.
     * @note Any active capture will be stopped automatically.
     *
     * @see Initialize()
     */
    UFUNCTION(BlueprintCallable)
    void Uninitialize();

    /**
     * @brief Called when the object finishes the destruction process.
     *
     * Overrides UObject::FinishDestroy to ensure proper cleanup of speech processor
     * resources, including stopping any active capture and releasing the audio worker.
     *
     * @note This is called automatically by Unreal's garbage collection system.
     */
    virtual void FinishDestroy() override;

    /**
     * @brief Called when the object begins the destruction process.
     *
     * Overrides UObject::BeginDestroy to initiate cleanup of speech processor resources.
     * Ensures that audio capture is stopped and resources are released before the
     * object is fully destroyed.
     *
     * @note This is called automatically by Unreal's garbage collection system.
     */
    virtual void BeginDestroy() override;

    /**
     * @brief Starts capturing audio from the microphone and processing it for speech recognition.
     *
     * Initiates the voice capture worker thread which begins recording audio from the
     * default microphone device. Captured audio is automatically fed to the speech
     * recognizer for transcription. Multiple delegates are provided to handle various
     * events during the capture session.
     *
     * @param OnStarted Delegate called when audio capture and speech recognition have
     *                  successfully started.
     * @param OnStopped Delegate called when audio capture and speech recognition have
     *                  stopped (either by calling StopCapture or due to an error).
     * @param OnError Delegate called if an error occurs during capture or recognition.
     *                The error code parameter indicates the type of error.
     * @param OnAudioCapture Delegate called when transcription results are available.
     *                       This is called for both interim results (IsFinal=false) and
     *                       final results (IsFinal=true).
     *
     * @note The processor must be initialized before calling this method.
     * @note Only one capture session can be active at a time.
     * @note Audio capture runs on a background thread to avoid blocking the game thread.
     *
     * @warning Ensure proper microphone permissions are granted on the target platform.
     *
     * @see StopCapture()
     * @see Initialize()
     */
    UFUNCTION(BlueprintCallable)
    void StartCapture(const FOnSpeechToTextStartedDelegate &OnStarted, const FOnSpeechToTextStoppedDelegate &OnStopped,
                      const FOnSpeechToTextErrorDelegate &OnError,
                      const FOnSpeechToTextTranscriptionDelegate &OnAudioCapture);

    /**
     * @brief Stops the current audio capture and speech recognition session.
     *
     * Halts the voice capture worker, stops recording audio from the microphone,
     * and signals the speech recognizer to stop processing. Any pending transcriptions
     * will be delivered before the session fully stops.
     *
     * @note This method is safe to call even if no capture is active.
     * @note The OnStopped delegate (provided to StartCapture) will be called when
     *       the capture has fully stopped.
     *
     * @see StartCapture()
     */
    UFUNCTION(BlueprintCallable)
    void StopCapture();

  private:
    /**
     * @brief Worker thread responsible for capturing audio from the microphone.
     *
     * This unique pointer manages the lifetime of the voice capture worker, which
     * runs on a separate thread to continuously capture audio data without blocking
     * the game thread. The worker feeds captured audio to the speech recognizer
     * for processing.
     *
     * @note This is created during StartCapture and destroyed during StopCapture or Uninitialize.
     */
    TUniquePtr<SGAISpeechRecognizer::IAudioStream> audioStream = nullptr;

    /**
     * @brief Reference to the speech recognizer that processes captured audio.
     *
     * This object pointer maintains a reference to the USpeechRecognizer instance
     * that was provided during initialization. The speech recognizer receives audio
     * data from the voice capture worker and converts it to text transcriptions.
     *
     * @note This is set during Initialize and cleared during Uninitialize.
     * @note The UPROPERTY macro ensures proper garbage collection handling.
     */
    UPROPERTY()
    TObjectPtr<USpeechRecognizer> SpeechRecognizerObj = nullptr;
};
