// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "SpeechRecognizerConfigurationSettings.h"
#include <functional>
/**
 * @typedef SpeechToTextEventCallbackFn
 * @brief Callback function type for speech-to-text transcription events.
 *
 * This callback is invoked when transcription results are available from the
 * speech recognition engine.
 *
 * @param bool IsFinal - True if this is the final transcription result, false for interim/partial results.
 * @param FString Transcription - The transcribed text from the speech input.
 *
 * @note Interim results may be called multiple times as the recognizer refines its transcription.
 * @note Final results indicate the recognizer has completed processing a speech segment.
 */
using SpeechToTextEventCallbackFn = std::function<void(const bool, const FString &)>;

/**
 * @typedef SpeechToTextStartedCallbackFn
 * @brief Callback function type invoked when speech recognition has started.
 *
 * This callback is triggered after the speech recognition session has successfully
 * begun and is ready to process audio data.
 */
using SpeechToTextStartedCallbackFn = std::function<void()>;

/**
 * @typedef SpeechToTextStoppedCallbackFn
 * @brief Callback function type invoked when speech recognition has stopped.
 *
 * This callback is triggered after the speech recognition session has ended,
 * either due to a Stop() call or after processing is complete.
 */
using SpeechToTextStoppedCallbackFn = std::function<void()>;

/**
 * @typedef SpeechToTextErrorCallbackFn
 * @brief Callback function type invoked when an error occurs during speech recognition.
 *
 * This callback is triggered when the speech recognition engine encounters an error
 * that prevents normal operation.
 *
 * @param int ErrorCode - Platform-specific error code indicating the type of error that occurred.
 *
 * @note Error codes are implementation-specific and may vary by platform.
 * @note When an error occurs, the recognition session is typically terminated.
 */
using SpeechToTextErrorCallbackFn = std::function<void(int)>;

/**
 *
 */
using SpeechRecognizerConfigurationSettings = TMap<ESpeechRecognizerConfigurationSettings, FString>;

/**
 * @interface ISpeechRecognizer
 * @brief Platform-agnostic interface for speech-to-text recognition engines.
 *
 * ISpeechRecognizer defines the contract that all platform-specific speech recognition
 * implementations must fulfill. This interface provides a unified API for:
 * - Initializing and destroying speech recognition engines
 * - Starting and stopping recognition sessions
 * - Processing audio data for transcription
 * - Configuring recognition parameters
 *
 * The interface uses callback functions to deliver asynchronous results, allowing
 * speech recognition to operate without blocking the main thread.
 *
 * ## Lifecycle
 * 1. Create an instance (platform-specific implementation)
 * 2. Call Initialize()
 * 3. Optionally call SetConfiguration() to customize behavior
 * 4. Call Start() to begin a recognition session
 * 5. Call ProcessAudio() repeatedly with audio data
 * 6. Call Stop() to end the session
 * 7. Call Destroy() to clean up resources
 * 8. Delete the instance
 *
 * ## Thread Safety
 * Implementations should be thread-safe for ProcessAudio() calls, as audio data
 * may be provided from a background capture thread.
 *
 * ## Platform Implementations
 * - VoiceAISpeechRecognizerWindows: Windows implementation using Whisper
 * - VoiceAISpeechRecognizerAndroid: Android implementation
 *
 * @note This is a pure virtual interface - all methods must be implemented by derived classes.
 * @note Implementations should handle resource cleanup in both Destroy() and the destructor.
 *
 * @see USpeechRecognizer
 * @see USpeechProcessor
 *
 * Example usage:
 * @code
 * TUniquePtr<ISpeechRecognizer> recognizer = GetPlatformSpecificRecognizer();
 *
 * recognizer->Initialize([](bool success) {
 *     if (success) {
 *         UE_LOG(LogTemp, Log, TEXT("Recognizer initialized"));
 *     }
 * });
 *
 * recognizer->Start(
 *     []() { UE_LOG(LogTemp, Log, TEXT("Started")); },
 *     []() { UE_LOG(LogTemp, Log, TEXT("Stopped")); },
 *     [](int error) { UE_LOG(LogTemp, Error, TEXT("Error: %d"), error); },
 *     [](bool isFinal, const FString& text) {
 *         UE_LOG(LogTemp, Log, TEXT("Transcription: %s"), *text);
 *     }
 * );
 *
 * // Feed audio data...
 * recognizer->ProcessAudio(audioBuffer);
 *
 * // When done...
 * recognizer->Stop();
 * recognizer->Destroy();
 * @endcode
 */
class SGAISPEECHRECOGNIZER_API ISpeechRecognizer
{
  public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     *
     * Ensures that derived class destructors are called correctly when deleting
     * through a base class pointer. Implementations should clean up any remaining
     * resources in their destructors.
     *
     * @note This is a pure virtual destructor, requiring an implementation even though it's =0.
     * @note Derived classes should call Destroy() before destruction if not already called.
     */
    virtual ~ISpeechRecognizer() = 0;

    /**
     * @brief Initializes the speech recognition engine.
     *
     * Prepares the speech recognizer for use by loading models, allocating resources,
     * and establishing any necessary connections. This method must be called before
     * any other operations can be performed.
     *
     * The initialization process is asynchronous, and the result is delivered through
     * the provided callback function.
     *
     * @param OnConnected Callback function invoked when initialization completes.
     *                    Receives true if initialization succeeded, false otherwise.
     *
     * @note This method should only be called once per instance.
     * @note Initialization may take significant time, especially on first run when models are loaded.
     * @note The callback may be invoked on a background thread.
     *
     * @warning Do not call other methods until the callback indicates successful initialization.
     *
     * @see Destroy()
     * @see SetConfiguration()
     */
    virtual bool Initialize(const FSpeechRecognizerSettings &Settings) = 0;

    /**
     * @brief Destroys the speech recognition engine and releases all resources.
     *
     * Cleans up all resources allocated by the speech recognizer, including models,
     * buffers, and any active connections. This method should be called when the
     * recognizer is no longer needed.
     *
     * @note This method will stop any active recognition session.
     * @note After calling Destroy(), the recognizer cannot be reused - create a new instance instead.
     * @note It is safe to call this method multiple times.
     *
     * @see Initialize()
     */
    virtual void Destroy() = 0;

    /**
     * @brief Starts a speech recognition session.
     *
     * Begins a new recognition session that will process audio data and generate
     * transcriptions. Multiple callbacks are provided to handle different events
     * during the session lifecycle.
     *
     * @param OnStarted Callback invoked when the recognition session has successfully started
     *                  and is ready to process audio.
     * @param OnStopped Callback invoked when the recognition session has ended, either
     *                  due to Stop() being called or after processing completes.
     * @param OnError Callback invoked if an error occurs during recognition. Receives
     *                a platform-specific error code.
     * @param OnTranscription Callback invoked when transcription results are available.
     *                        Receives a boolean indicating if the result is final, and
     *                        the transcribed text.
     *
     * @note The recognizer must be initialized before calling this method.
     * @note Only one recognition session can be active at a time.
     * @note Callbacks may be invoked on background threads.
     * @note After Start() is called, use ProcessAudio() to feed audio data.
     *
     * @see Stop()
     * @see ProcessAudio()
     */
    virtual void Start(const SpeechToTextStartedCallbackFn &OnStarted, const SpeechToTextStoppedCallbackFn &OnStopped,
                       const SpeechToTextErrorCallbackFn &OnError,
                       const SpeechToTextEventCallbackFn &OnTranscription) = 0;

    /**
     * @brief Processes audio data for speech recognition.
     *
     * Feeds raw audio data to the speech recognition engine for processing. This method
     * should be called repeatedly with chunks of audio data during an active recognition
     * session. The audio format should match the recognizer's expected format (typically
     * 16-bit PCM at 16kHz).
     *
     * @param buffer Array of audio data bytes to process. The format should match the
     *               recognizer's requirements (sample rate, bit depth, channels).
     *
     * @return True if the audio was successfully queued for processing, false if the
     *         audio could not be processed (e.g., session not started, buffer invalid).
     *
     * @note This method can be called from any thread (typically from an audio capture thread).
     * @note Audio data is typically processed asynchronously.
     * @note Transcription callbacks may be triggered as audio is processed.
     * @note The buffer should contain raw PCM audio data.
     *
     * @warning Ensure the audio format matches the recognizer's expectations.
     * @warning Do not call this method before Start() or after Stop().
     *
     * @see Start()
     */
    virtual bool ProcessAudio(const uint8 *bytes, const uint32 numBytes) = 0;

    /**
     * @brief Gracefully stops the recognition session.
     *
     * Ends the current recognition session, allowing the recognizer to finish processing
     * any pending audio data before stopping. This ensures that all queued audio is
     * transcribed before the session ends.
     *
     * Implementations may inject silence to signal the end of speech and trigger final
     * transcription results.
     *
     * @note This method allows pending audio to be processed before stopping.
     * @note The OnStopped callback will be invoked after all processing completes.
     * @note This method is safe to call even if no session is active.
     * @note There may be a delay before the session fully stops.
     *
     * @see Start()
     */
    virtual void Stop() = 0;
};
