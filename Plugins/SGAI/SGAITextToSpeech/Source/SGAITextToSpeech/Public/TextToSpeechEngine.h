// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ITextToSpeech.h"
#include "TextToSpeechTypes.h"

#include "TextToSpeechEngine.generated.h"
// Delegate declarations
/**
 * @brief Delegate called when the tts engine initialization completes.
 * @param IsInitialized True if initialization was successful, false otherwise.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTTSInitialized, bool, IsInitialized);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTTSEvent, int, EventId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTTSError, int, ErrorCode);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTTSAudioAvailable, const TArray<uint8> &, bytes);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SGAITEXTTOSPEECH_API UTextToSpeechEngine : public UObject
{
    GENERATED_BODY()

  public:
    UTextToSpeechEngine();
    /**
     * @brief Constructor for VTable helper (used by Unreal's reflection system).
     * @param Helper VTable helper reference.
     */
    UTextToSpeechEngine(FVTableHelper &Helper);

    virtual ~UTextToSpeechEngine();

    /**
     * Initialize the TTS engine with the given configuration
     * @param Config Configuration parameters for the TTS engine
     * @return Error code indicating success or failure
     */
    UFUNCTION(BlueprintCallable, Category = "Text To Speech")
    virtual void Initialize(const FTTSConfigurationSettings &Config, const FOnTTSInitialized &OnInitialized);

    /**
     * Deinitialize the TTS engine and release resources
     */
    UFUNCTION(BlueprintCallable, Category = "Text To Speech")
    virtual void Uninitialize();

    /**
     * @brief Called when the object begins the destruction process.
     *
     * Overrides UObject::BeginDestroy to ensure proper cleanup of speech recognizer resources.
     */
    virtual void BeginDestroy() override;

    /**
     * @brief Called when the object finishes the destruction process.
     *
     * Overrides UObject::FinishDestroy to complete cleanup operations.
     */
    virtual void FinishDestroy() override;

    /**
     * Start speech synthesis for the given text
     * @param Text The text to convert to speech
     * @return Error code indicating success or failure
     */
    UFUNCTION(BlueprintCallable, Category = "Text To Speech")
    virtual ETTSErrorCode Speak(const FString &Text, const FOnTTSAudioAvailable &OnAudioAvailable,
                                const FOnTTSEvent &OnEvent, const FOnTTSError &OnError);

    /**
     * Stop the current speech synthesis
     */
    UFUNCTION(BlueprintCallable, Category = "Text To Speech")
    virtual void Stop();

    /**
     * Get the current configuration
     */
    UFUNCTION(BlueprintPure, Category = "Text To Speech")
    const FTTSConfigurationSettings &GetConfig() const
    {
        return ConfigurationSettings;
    }

  private:
    TUniquePtr<ITextToSpeech> GetTextToSpeechInstance();

  private:
    TUniquePtr<ITextToSpeech> TextToSpeechImpl;
    TFuture<bool> IsInitialized;
    FTTSConfigurationSettings ConfigurationSettings;
};
