// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"

namespace SGAISpeechRecognizer
{
class IAudioStream
{
  public:
    using AudioStreamCallback = TFunction<void(const uint8 *bytes, const uint32 numBytes)>;
    virtual ~IAudioStream() = default;
    virtual bool OpenStream() = 0;
    virtual void CloseStream() = 0;
    virtual bool StopCapture() = 0;
    virtual bool StartCapture(const AudioStreamCallback &func) = 0;
};
} // namespace SGAISpeechRecognizer
