// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"
#include "TTSResultCallbackNativeProxyJava.h"
#include "TextToSpeechTypes.h"
#include <map>
#include <memory>
#include <string>

//
//
class FTTSJava : public FJavaClassObject
{
  public:
    FTTSJava();
    static TUniquePtr<FTTSJava> NewInstance(FTTSResultCallbackNativeProxy *);
    virtual ~FTTSJava();
    void Speak(const FString &Text);
    // Lifecycle
    bool Init(const std::map<std::string, std::string> &config);
    void DeInit();
    void Stop();

  private:
    FJavaClassMethod SetModelPathMethodId;
    FJavaClassMethod SetLanguageMethodId;
    FJavaClassMethod SetAudioEncodingMethodId;
    FJavaClassMethod SetSpeechRateMethodId;
    FJavaClassMethod SetPitchMethodId;
    FJavaClassMethod SetVolumeGainMethodId;
    FJavaClassMethod SetSampleRateMethodId;
    FJavaClassMethod SetSkelLibPathMethodId;
    FJavaClassMethod InitMethodId;
    FJavaClassMethod StartMethodId;
    FJavaClassMethod StopMethodId;
    FJavaClassMethod DeInitMethodId;
    FJavaClassMethod ReleaseMethodId;
};
