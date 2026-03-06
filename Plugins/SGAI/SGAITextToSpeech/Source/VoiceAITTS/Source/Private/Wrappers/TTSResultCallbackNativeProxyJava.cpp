// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "TTSResultCallbackNativeProxyJava.h"

#define LOG_TAG "TTSJava"
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C"
{
    JNIEXPORT void JNICALL Java_com_sgs_gameai_TTSResultCallbackNativeProxy_nativeOnError(JNIEnv *env, jclass /*clazz*/,
                                                                                          jlong handle, jint errorCode)
    {
        LOGD("nativeOnError: handle=%ld, errorCode=%d", handle, errorCode);
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_TTSResultCallbackNativeProxy_nativeOnDone(JNIEnv * /*env*/,
                                                                                         jclass /*clazz*/, jlong handle)
    {
        LOGD("nativeOnDone: handle=%ld", handle);
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_TTSResultCallbackNativeProxy_nativeOnStart(JNIEnv * /*env*/,
                                                                                          jclass /*clazz*/,
                                                                                          jlong handle, jint, jint,
                                                                                          jint)
    {
        LOGD("nativeOnSpeechStart: handle=%ld", handle);
    }

    JNIEXPORT void JNICALL Java_com_sgs_gameai_TTSResultCallbackNativeProxy_nativeOnAudioAvailable(
        JNIEnv *Env, jclass /*clazz*/, jlong handle, jbyteArray Buff, jint Offset, jint Size)
    {
        LOGD("nativeOnAudioAvailable: handle=%ld", handle);

        ITTSResultCallback *callback = (ITTSResultCallback *)(handle);
        if (callback != nullptr)
        {
            jboolean IsCopy = JNI_FALSE;
            jbyte *Raw = Env->GetByteArrayElements(Buff, &IsCopy);
            const uint8 *AudioPtr = reinterpret_cast<uint8 *>(Raw + Offset);
            callback->OnAudioAvailable(AudioPtr, Size);
            Env->ReleaseByteArrayElements(Buff, Raw, JNI_ABORT);
        }
    }
} // extern "C"

ITTSResultCallback::~ITTSResultCallback()
{
}

FTTSResultCallbackNativeProxy::FTTSResultCallbackNativeProxy(ITTSResultCallback *ptr)
    : FJavaClassObject("com/sgs/gameai/TTSResultCallbackNativeProxy", "(J)V", (long)(ptr))
{
}

FTTSResultCallbackNativeProxy::~FTTSResultCallbackNativeProxy()
{
}
