// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#include "TextToSpeechEngine.h"
#include "Async/Async.h"

ITextToSpeech::~ITextToSpeech()
{
}
UTextToSpeechEngine::UTextToSpeechEngine() = default;
UTextToSpeechEngine::~UTextToSpeechEngine() = default;

UTextToSpeechEngine::UTextToSpeechEngine(FVTableHelper &Helper) : UObject(Helper)
{
}

void UTextToSpeechEngine::BeginDestroy()
{
    UE_LOG(LogTemp, Verbose, TEXT("UTextToSpeechEngine::BeginDestroy"));

    Uninitialize();

    Super::BeginDestroy();
}

void UTextToSpeechEngine::FinishDestroy()
{
    UE_LOG(LogTemp, Verbose, TEXT("UTextToSpeechEngine::FinishDestroy"));
    Super::FinishDestroy();
}

void UTextToSpeechEngine::Initialize(const FTTSConfigurationSettings &Config, const FOnTTSInitialized &OnInitialized)
{

    if (TextToSpeechImpl == nullptr)
    {
        // Store configuration
        ConfigurationSettings = Config;
        IsInitialized = Async(EAsyncExecution::Thread, [this, OnInitialized]() {
            bool isInitialized = false;
            if (TextToSpeechImpl != nullptr)
            {
                isInitialized = TextToSpeechImpl->Initialize(ConfigurationSettings);
                AsyncTask(ENamedThreads::GameThread,
                          [OnInitialized, isInitialized]() { OnInitialized.ExecuteIfBound(isInitialized); });
            }
            return isInitialized;
        });
    }
    // if (!bIsInitialized)
    // {
    // 	UE_LOG(LogTemp, Warning, TEXT("TextToSpeechEngine: Already initialized"));
    // 	return ETTSErrorCode::Success;
    // }

    // // Validate configuration
    // if (Config.ModelPath.IsEmpty())
    // {
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngine: Model path is empty"));
    // 	return ETTSErrorCode::InvalidParameter;
    // }

    // Call platform-specific initialization
    // ETTSErrorCode Result = InitializeInternal(Config);

    // if (Result == ETTSErrorCode::Success)
    // {
    // 	bIsInitialized = true;
    // 	UE_LOG(LogTemp, Log, TEXT("TextToSpeechEngine: Initialized successfully"));
    // }
    // else
    // {
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngine: Initialization failed with error code %d"), (int32)Result);
    // }

    // return Result;
}

ETTSErrorCode UTextToSpeechEngine::Speak(const FString &Text, const FOnTTSAudioAvailable &OnAudioAvailable,
                                         const FOnTTSEvent &OnEvent, const FOnTTSError &OnError)
{
    // FScopeLock Lock(&CriticalSection);

    ETTSErrorCode Result = ETTSErrorCode::NotInitialized;

    if (TextToSpeechImpl != nullptr)
    {
        if (!Text.IsEmpty())
        {
            AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, Text]() {
                if (TextToSpeechImpl != nullptr)
                {
                    // Stop ?
                    TextToSpeechImpl->Speak(
                        Text,
                        [this](int code, int, int) {
                            // start
                            //  send to user
                            AsyncTask(ENamedThreads::GameThread, [this]() {});
                        },
                        [this](const uint8 *bytes, const uint32 num) {
                            // on audio data
                            //.. copy bytes to TArray
                            AsyncTask(ENamedThreads::GameThread, [this]() {});
                        },
                        [this]() {
                            // complete
                            // send to user
                            AsyncTask(ENamedThreads::GameThread, [this]() {});
                        },
                        [this](int code) {
                            // send to user
                            AsyncTask(ENamedThreads::GameThread, [this]() {});
                        });
                }
            });
            Result = ETTSErrorCode::Success;
        }
        else
        {
            Result = ETTSErrorCode::InvalidParameter;
        }
    }
    else
    {
        Result = ETTSErrorCode::NotInitialized;
    }
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngine: Not initialized"));
    return Result;
    // }

    // if (Text.IsEmpty())
    // {
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngine: Text is empty"));
    // 	return ETTSErrorCode::InvalidParameter;
    // }

    // if (bIsSpeaking)
    // {
    // 	UE_LOG(LogTemp, Warning, TEXT("TextToSpeechEngine: Already speaking"));
    // 	return ETTSErrorCode::AlreadyRunning;
    // }

    // bIsSpeaking = true;

    // Call platform-specific speak implementation
    // ETTSErrorCode Result = SpeakInternal(Text);

    // if (Result != ETTSErrorCode::Success)
    // {
    // 	bIsSpeaking = false;
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngine: Speak failed with error code %d"), (int32)Result);
    // }

    // return Result;
}

void UTextToSpeechEngine::Stop()
{
    AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this]() {
        if (TextToSpeechImpl != nullptr)
        {
            TextToSpeechImpl->Stop();
        }
    });

    // FScopeLock Lock(&CriticalSection);

    // if (!bIsInitialized)
    // {
    // 	UE_LOG(LogTemp, Warning, TEXT("TextToSpeechEngine: Not initialized"));
    // 	return;
    // }

    // if (!bIsSpeaking)
    // {
    // 	return;
    // }

    // StopInternal();
    // bIsSpeaking = false;

    UE_LOG(LogTemp, Log, TEXT("TextToSpeechEngine: Stopped"));
}

void UTextToSpeechEngine::Uninitialize()
{
    UE_LOG(LogTemp, Verbose, TEXT("SpeechRecognizer::Uninitialize"));
    IsInitialized.Wait();
    if (TextToSpeechImpl != nullptr)
    {
        TextToSpeechImpl->Destroy();
        TextToSpeechImpl.Reset();
    }

    UE_LOG(LogTemp, Log, TEXT("TextToSpeechEngine: Deinitialized"));
}

// void UTextToSpeechEngine::BroadcastSpeechStarted(int32 SampleRate, int32 AudioFormat, int32 ChannelCount)
//{
//     // Broadcast on game thread
//     AsyncTask(ENamedThreads::GameThread, [this, SampleRate, AudioFormat, ChannelCount]() {
//         if (IsValid(this))
//         {
//             FTTSAudioFormat Format(SampleRate, AudioFormat, ChannelCount);
//             OnSpeechStarted.Broadcast(Format);
//         }
//     });
// }
//
// void UTextToSpeechEngine::BroadcastAudioDataAvailable(const TArray<uint8> &AudioData)
//{
//     // Broadcast on game thread
//     AsyncTask(ENamedThreads::GameThread, [this, AudioData]() {
//         if (IsValid(this))
//         {
//             OnAudioDataAvailable.Broadcast(AudioData);
//         }
//     });
// }
//
// void UTextToSpeechEngine::BroadcastSpeechCompleted()
//{
//     // Broadcast on game thread
//     AsyncTask(ENamedThreads::GameThread, [this]() {
//         if (IsValid(this))
//         {
//             bIsSpeaking = false;
//             OnSpeechCompleted.Broadcast();
//         }
//     });
// }
//
// void UTextToSpeechEngine::BroadcastSpeechError(ETTSErrorCode ErrorCode)
//{
//     // Broadcast on game thread
//     AsyncTask(ENamedThreads::GameThread, [this, ErrorCode]() {
//         if (IsValid(this))
//         {
//             bIsSpeaking = false;
//             OnSpeechError.Broadcast(ErrorCode);
//         }
//     });
// }
