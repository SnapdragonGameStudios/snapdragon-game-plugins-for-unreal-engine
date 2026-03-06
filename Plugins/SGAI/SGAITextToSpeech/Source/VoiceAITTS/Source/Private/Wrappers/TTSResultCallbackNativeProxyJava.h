// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"
//
//
class ITTSResultCallback
{
  public:
    virtual void OnAudioAvailable(const uint8 *ptr, int size) = 0;
    virtual void OnError(int errorCode) = 0;
    virtual void OnStart(int, int, int) = 0;
    virtual void OnDone() = 0;
    virtual ~ITTSResultCallback() = 0;
};

//
//
class FTTSResultCallbackNativeProxy : public FJavaClassObject
{
  public:
    FTTSResultCallbackNativeProxy(ITTSResultCallback *);
    virtual ~FTTSResultCallbackNativeProxy();
};
