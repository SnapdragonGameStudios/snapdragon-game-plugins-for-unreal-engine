// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"
#include "CoreMinimal.h"
#include "WhisperJava.h"

//
//
class FWhisperBuilder : public FJavaClassObject
{
    FJavaClassMethod SetAILogLevelMethodId;
    FJavaClassMethod SetVadLenFutureMethodId;
    FJavaClassMethod SetNoStartSpeechTimeoutMethodId;
    FJavaClassMethod SetVadLenHangoverMethodId;
    FJavaClassMethod SetVadThresholdMethodId;
    FJavaClassMethod SetVadCmdEndDetectionThresholdMethodId;
    FJavaClassMethod SetTranslationEnabledMethodId;
    FJavaClassMethod SetLanguageCodeMethodId;
    FJavaClassMethod SuppressNonSpeechMethodId;
    FJavaClassMethod ShowProgressIndicatorsMethodId;
    FJavaClassMethod SetListenerMethodId;
    FJavaClassMethod EnableContinuousTranscriptionMethodId;
    FJavaClassMethod EnablePartialTranscriptionsMethodId;
    FJavaClassMethod BuildMethodId;

  public:
    FWhisperBuilder();
    virtual ~FWhisperBuilder();
    FWhisperBuilder *SetVadLenFuture(int Value);
    FWhisperBuilder *SetAILogLevel(int Level);
    FWhisperBuilder *SetNoStartSpeechTimeout(int Value);
    FWhisperBuilder *SetVadLenHangover(int Value);
    FWhisperBuilder *SetVadThreshold(int Value);
    FWhisperBuilder *SetVadCmdEndDetectionThreshold(int Value);
    FWhisperBuilder *SetTranslationEnabled(bool bEnabled);
    FWhisperBuilder *SetLanguageCode(const FString &LanguageCode);
    FWhisperBuilder *SuppressNonSpeech(bool bSuppress);
    FWhisperBuilder *ShowProgressIndicators(bool bShow);
    FWhisperBuilder *SetListener(FWhisperResponseListenerNativeProxy *Listener);
    FWhisperBuilder *EnableContinuousTranscription(bool bEnabled);
    FWhisperBuilder *EnablePartialTranscriptions(bool bEnabled);
    TUniquePtr<FWhisperJava> Build();
};
