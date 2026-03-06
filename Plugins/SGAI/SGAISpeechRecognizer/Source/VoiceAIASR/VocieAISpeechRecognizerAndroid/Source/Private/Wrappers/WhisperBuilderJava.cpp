// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "WhisperBuilderJava.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJavaEnv.h"
#include <android/log.h>
#include <jni.h>
#include <string>

FWhisperBuilder::FWhisperBuilder()
    : FJavaClassObject("com/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder", "()V"),
      SetAILogLevelMethodId(
          GetClassMethod("setAILogLevel", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetVadLenFutureMethodId(
          GetClassMethod("setVadLenFuture", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetNoStartSpeechTimeoutMethodId(
          GetClassMethod("setNoStartSpeechTimeout", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetVadLenHangoverMethodId(
          GetClassMethod("setVadLenHangover", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetVadThresholdMethodId(
          GetClassMethod("setVadThreshold", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetVadCmdEndDetectionThresholdMethodId(GetClassMethod(
          "setVadCmdEndDetectionThreshold", "(I)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetTranslationEnabledMethodId(
          GetClassMethod("setTranslationEnabled", "(Z)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetLanguageCodeMethodId(GetClassMethod(
          "setLanguageCode", "(Ljava/lang/String;)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SuppressNonSpeechMethodId(
          GetClassMethod("suppressNonSpeech", "(Z)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      ShowProgressIndicatorsMethodId(
          GetClassMethod("showProgressIndicators", "(Z)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      SetListenerMethodId(GetClassMethod("setListener",
                                         "(Lcom/qualcomm/qti/voice/assist/whisper/sdk/WhisperResponseListener;)"
                                         "Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      EnableContinuousTranscriptionMethodId(GetClassMethod(
          "enableContinuousTranscription", "(Z)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      EnablePartialTranscriptionsMethodId(GetClassMethod(
          "enablePartialTranscriptions", "(Z)Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper$Builder;")),
      BuildMethodId(GetClassMethod("build", "()Lcom/qualcomm/qti/voice/assist/whisper/sdk/Whisper;"))

{
}

FWhisperBuilder::~FWhisperBuilder()
{
}

FWhisperBuilder *FWhisperBuilder::SetVadLenFuture(int Value)
{
    CallMethod<jobject>(SetVadLenFutureMethodId, Value);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetAILogLevel(int Level)
{
    CallMethod<jobject>(SetAILogLevelMethodId, Level);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetNoStartSpeechTimeout(int Value)
{
    CallMethod<jobject>(SetNoStartSpeechTimeoutMethodId, Value);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetVadLenHangover(int Value)
{
    CallMethod<jobject>(SetVadLenHangoverMethodId, Value);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetVadThreshold(int Value)
{
    CallMethod<jobject>(SetVadThresholdMethodId, Value);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetVadCmdEndDetectionThreshold(int Value)
{
    CallMethod<jobject>(SetVadCmdEndDetectionThresholdMethodId, Value);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetTranslationEnabled(bool bEnabled)
{
    CallMethod<jobject>(SetTranslationEnabledMethodId, bEnabled);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetLanguageCode(const FString &LanguageCode)
{
    FScopedJavaObject<jstring> jLanguageCode = GetJString(LanguageCode);
    CallMethod<jobject>(SetLanguageCodeMethodId, *jLanguageCode);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SuppressNonSpeech(bool bSuppress)
{
    CallMethod<jobject>(SuppressNonSpeechMethodId, bSuppress);
    return this;
}

FWhisperBuilder *FWhisperBuilder::ShowProgressIndicators(bool bShow)
{
    CallMethod<jobject>(ShowProgressIndicatorsMethodId, bShow);
    return this;
}

FWhisperBuilder *FWhisperBuilder::SetListener(FWhisperResponseListenerNativeProxy *Listener)
{
    CallMethod<jobject>(SetListenerMethodId, Listener->GetJObject());
    return this;
}

FWhisperBuilder *FWhisperBuilder::EnableContinuousTranscription(bool bEnabled)
{
    CallMethod<jobject>(EnableContinuousTranscriptionMethodId, bEnabled);
    return this;
}

FWhisperBuilder *FWhisperBuilder::EnablePartialTranscriptions(bool bEnabled)
{
    CallMethod<jobject>(EnablePartialTranscriptionsMethodId, bEnabled);
    return this;
}

TUniquePtr<FWhisperJava> FWhisperBuilder::Build()
{
    jobject JavaObject = CallMethod<jobject>(BuildMethodId);
    return FWhisperJava::Wrap(JavaObject);
}
