// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "WhisperResponseListenerNativeProxyJava.h"

#define LOG_TAG "WhisperJNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C"
{

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnTranscription(
        JNIEnv *env, jclass /*clazz*/, jlong handle, jstring transcription, jstring language, jboolean finalized,
        jint code)
    {
        //  Convert Java strings to C++ strings
        const char *transcriptionStr = env->GetStringUTFChars(transcription, nullptr);
        const char *languageStr = env->GetStringUTFChars(language, nullptr);

        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnTranscription(transcriptionStr, languageStr, finalized, code);
        }

        // Release string resources
        env->ReleaseStringUTFChars(transcription, transcriptionStr);
        env->ReleaseStringUTFChars(language, languageStr);
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnError(
        JNIEnv *env, jclass /*clazz*/, jlong handle, jint errorCode, jstring message)
    {
        const char *messageStr = message ? env->GetStringUTFChars(message, nullptr) : "null";

        LOGD("nativeOnError: handle=%ld, errorCode=%d, message='%s'", handle, errorCode, messageStr);

        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnError(errorCode);
        }

        if (message)
        {
            env->ReleaseStringUTFChars(message, messageStr);
        }
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnRecordingStopped(
        JNIEnv * /*env*/, jclass /*clazz*/, jlong handle)
    {
        LOGD("nativeOnRecordingStopped: handle=%ld", handle);
        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnRecordingStopped();
        }
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnSpeechStart(JNIEnv * /*env*/,
                                                                                                      jclass /*clazz*/,
                                                                                                      jlong handle)
    {
        LOGD("nativeOnSpeechStart: handle=%ld", handle);
        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnSpeechStart();
        }
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnSpeechEnd(JNIEnv * /*env*/,
                                                                                                    jclass /*clazz*/,
                                                                                                    jlong handle)
    {
        LOGD("nativeOnSpeechEnd: handle=%ld", handle);
        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnSpeechEnd();
        }
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_WhisperResponseListenerNativeProxy_nativeOnFinished(JNIEnv * /*env*/,
                                                                                                   jclass /*clazz*/,
                                                                                                   jlong handle)
    {
        LOGD("nativeOnFinished: handle=%ld", handle);
        IWhisperResponseListener *listener = (IWhisperResponseListener *)(handle);
        if (listener != nullptr)
        {
            listener->OnFinished();
        }
    }

} // extern "C"

FWhisperResponseListenerNativeProxy::FWhisperResponseListenerNativeProxy(IWhisperResponseListener *ptr)
    : FJavaClassObject("com/sgs/gameai/WhisperResponseListenerNativeProxy", "(J)V", (long)(ptr))
{
}

FWhisperResponseListenerNativeProxy::~FWhisperResponseListenerNativeProxy()
{
}
