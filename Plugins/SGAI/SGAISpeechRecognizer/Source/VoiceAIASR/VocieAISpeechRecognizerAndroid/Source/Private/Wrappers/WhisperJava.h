// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"
#include "CoreMinimal.h"
#include "Wrappers/NativeInputStreamJava.h"
#include "Wrappers/WhisperResponseListenerNativeProxyJava.h"

//
//
class FWhisperJava : public FJavaClassObject
{
  public:
    /**
     *
     */
    static TUniquePtr<FWhisperJava> Wrap(jobject);

    /**
     *
     */
    FWhisperJava();

    /**
     *
     */
    virtual ~FWhisperJava();

    /**
     *
     */
    int Init(const FString &ModelDirPath);

    /**
     *
     */
    void DeInit();

    /**
     *
     */
    void Stop();

    /**
     *
     */
    void Start(FNativeInputStreamJava *); // PCM16 mono

    /**
     *
     */
    FString GetVersion();

    /**
     *
     */
    void EnableDebug(bool);

  private:
    FJavaClassMethod InitializeMethodId;
    FJavaClassMethod DeInitializeMethodId;
    FJavaClassMethod StartRecognitionMethodId;
    FJavaClassMethod StopRecognitionMethodId;
    FJavaClassMethod SetListenerMethodId;
    FJavaClassMethod GetVersionMethodId;
    FJavaClassMethod EnableDebugMethodId;
    FJavaClassMethod SetDebugDirMethodId;
};
