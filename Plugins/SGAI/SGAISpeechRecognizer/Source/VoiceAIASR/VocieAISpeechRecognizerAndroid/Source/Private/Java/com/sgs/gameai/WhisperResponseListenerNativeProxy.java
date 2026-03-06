// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

package com.sgs.gameai;
import android.util.Log;
import android.os.Bundle;
import com.qualcomm.qti.voice.assist.whisper.sdk.WhisperResponseListener;

public class WhisperResponseListenerNativeProxy implements WhisperResponseListener {
    private long nativeHandle;

    public WhisperResponseListenerNativeProxy(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    @Override
    public void onTranscription(String transcription, String language, boolean finalized, int code, Bundle bundle) {
        nativeOnTranscription(nativeHandle, transcription, language, finalized, code);
    }

    @Override
    public void onError(int errorCode, Exception ex) {
        nativeOnError(nativeHandle, errorCode, ex != null ? ex.getMessage() : "");
    }

    @Override
    public void onRecordingStopped() {
        nativeOnRecordingStopped(nativeHandle);
    }

    @Override
    public void onSpeechStart() {
        nativeOnSpeechStart(nativeHandle);
    }

    @Override
    public void onSpeechEnd() {
        nativeOnSpeechEnd(nativeHandle);
    }

    @Override
    public void onFinished() {
        nativeOnFinished(nativeHandle);
    }

    // Native callbacks into UE C++
    private static native void nativeOnTranscription(long handle, String transcription, String language, boolean finalized, int code);
    private static native void nativeOnError(long handle, int errorCode, String message);
    private static native void nativeOnRecordingStopped(long handle);
    private static native void nativeOnSpeechStart(long handle);
    private static native void nativeOnSpeechEnd(long handle);
    private static native void nativeOnFinished(long handle);
}
