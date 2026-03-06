// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"

//
//
class IInputStream
{
  public:
    virtual int Read(uint8 *arr, int offset, int length) = 0;
};

//
//
class FNativeInputStreamJava : public FJavaClassObject
{
  public:
    FNativeInputStreamJava(IInputStream *);
    ~FNativeInputStreamJava();
};
