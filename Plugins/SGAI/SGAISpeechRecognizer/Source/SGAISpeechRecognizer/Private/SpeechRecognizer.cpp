// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "SpeechRecognizer.h"
#include "Async/Async.h"
#include "ISpeechRecognizerModuleInterface.h"

DEFINE_LOG_CATEGORY(LogSpeechRecognizer);

//
//
ISpeechRecognizer::~ISpeechRecognizer()
{
}

//
//
USpeechRecognizer::USpeechRecognizer() = default;

//
//
USpeechRecognizer::~USpeechRecognizer() = default;

//
//
USpeechRecognizer::USpeechRecognizer(FVTableHelper &Helper) : UObject(Helper)
{
}

//
//
void USpeechRecognizer::BeginDestroy()
{
    UE_LOG(LogSpeechRecognizer, Verbose, TEXT("SpeechRecognizer::BeginDestroy"));

    Uninitialize();

    Super::BeginDestroy();
}

//
//
void USpeechRecognizer::Initialize(FSpeechRecognizerSettings Configuration,
                                   const FSpeechToTextOnInitializedDelegate &OnConnected)
{
    UE_LOG(LogSpeechRecognizer, Verbose, TEXT("SpeechRecognizer::Initialize"));
    if (SpeechRecognizerImpl == nullptr)
    {
        SpeechRecognizerImpl = GetSpeechRecognizerInstance();
        IsInitialized = Async(EAsyncExecution::Thread, [this, Configuration, OnConnected]() {
            bool isInitialized = false;
            if (SpeechRecognizerImpl != nullptr)
            {
                isInitialized = SpeechRecognizerImpl->Initialize(Configuration);
                AsyncTask(ENamedThreads::GameThread,
                          [OnConnected, isInitialized]() { OnConnected.ExecuteIfBound(isInitialized); });
            }
            return isInitialized;
        });
    }
}

//
//
void USpeechRecognizer::Uninitialize()
{
    UE_LOG(LogSpeechRecognizer, Verbose, TEXT("SpeechRecognizer::Uninitialize"));
    IsInitialized.Wait();
    if (SpeechRecognizerImpl != nullptr)
    {
        SpeechRecognizerImpl->Destroy();
        SpeechRecognizerImpl.Reset();
    }
}

//
//
void USpeechRecognizer::Start(const FOnSpeechToTextStartedDelegate &OnStarted,
                              const FOnSpeechToTextStoppedDelegate &OnStopped,
                              const FOnSpeechToTextErrorDelegate &OnError,
                              const FOnSpeechToTextTranscriptionDelegate &OnTranscription)
{
    if (SpeechRecognizerImpl != nullptr)
    {
        SpeechRecognizerImpl->Start(
            [this, OnStarted]() {
                // Started,
                AsyncTask(ENamedThreads::GameThread, [OnStarted]() { OnStarted.ExecuteIfBound(); });
            },
            [this, OnStopped]() {
                // Stopped,
                AsyncTask(ENamedThreads::GameThread, [OnStopped]() { OnStopped.ExecuteIfBound(); });
            },
            [this, OnError](int id) {
                // Error,
                AsyncTask(ENamedThreads::GameThread, [OnError, id]() { OnError.ExecuteIfBound(id); });
            },
            [this, OnTranscription](const bool isFinal, const FString &text) {
                AsyncTask(ENamedThreads::GameThread,
                          [this, OnTranscription, isFinal, text]() { OnTranscription.ExecuteIfBound(isFinal, text); });
            });
    }
}

//
//
bool USpeechRecognizer::ProcessAudio(const TArray<uint8> &bytes)
{
    return ProcessAudio(bytes.GetData(), bytes.Num());
}

//
//
bool USpeechRecognizer::ProcessAudio(const uint8 *bytes, const uint32 numBytes)
{
    UE_LOG(LogSpeechRecognizer, Verbose, TEXT("SpeechRecognizer::ProcessAudio"));
    if (SpeechRecognizerImpl != nullptr)
    {
        SpeechRecognizerImpl->ProcessAudio(bytes, numBytes);
        return true;
    }
    return true;
}

//
//
void USpeechRecognizer::Stop()
{
    AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this]() {
        if (SpeechRecognizerImpl != nullptr)
        {
            SpeechRecognizerImpl->Stop();
        }
    });
}
