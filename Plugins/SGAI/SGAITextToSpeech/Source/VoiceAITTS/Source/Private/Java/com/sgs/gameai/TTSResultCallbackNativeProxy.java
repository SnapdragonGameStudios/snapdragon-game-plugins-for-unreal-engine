// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

package com.sgs.gameai;
import android.util.Log;
import android.os.Bundle;
import com.qualcomm.qti.voice.assist.tts.sdk.TTSResultCallback;

public class TTSResultCallbackNativeProxy implements TTSResultCallback {
    private long nativeHandle;

    public TTSResultCallbackNativeProxy(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    @Override
    public void onStart(int a, int b, int c) {
        nativeOnStart(nativeHandle, a, b, c);
    }

    @Override
    public void onError(int errorCode) {
        nativeOnError(nativeHandle, errorCode);
    }

    @Override
    public void onDone() {
        nativeOnDone(nativeHandle);
    }

 @Override
 public void onAudioAvailable(byte[] buff, int offset,
 int size) {
 //custom code
 nativeOnAudioAvailable(nativeHandle, buff, offset, size);
 }


    // Native callbacks into UE C++
    private static native void nativeOnStart(long handle, int a, int b, int c);
    private static native void nativeOnError(long handle, int errorCode);
    private static native void nativeOnDone(long handle);
    private static native void nativeOnAudioAvailable(long handle, byte[] bytes, int offset, int size);
}
