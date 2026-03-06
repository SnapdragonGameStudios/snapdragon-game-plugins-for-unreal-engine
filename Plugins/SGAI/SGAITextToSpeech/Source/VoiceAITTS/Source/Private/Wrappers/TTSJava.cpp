// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

#include "TTSJava.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJavaEnv.h"

TUniquePtr<FTTSJava> FTTSJava::NewInstance(FTTSResultCallbackNativeProxy *listener)
{
    TUniquePtr<FTTSJava> tts = nullptr;
    if (listener != nullptr)
    {
        tts = MakeUnique<FTTSJava>();
        JNIEnv *Env = FAndroidApplication::GetJavaEnv();
        // jclass ttsClass = FJavaWrapper::FindClass(Env, "com/qualcomm/qti/voice/assist/tts/sdk/TTS", false);

        // 3. Get the method ID for the constructor (signature for (String)void)
        // The signature for the constructor that takes a single String argument is "(Ljava/lang/String;)V"
        jmethodID NewInstanceMethodID = FJavaWrapper::FindStaticMethod(
            Env, tts->Class, "newInstance",
            "(Lcom/qualcomm/qti/voice/assist/tts/sdk/TTSResultCallback;)Lcom/qualcomm/qti/voice/assist/tts/sdk/TTS;",
            false);

        jobject obj =
            FJavaWrapper::CallStaticObjectMethod(Env, tts->Class, NewInstanceMethodID, listener->GetJObject());

        Env->DeleteGlobalRef(tts->Object);
        tts->Object = Env->NewGlobalRef(obj);
    }
    return tts;
}

FTTSJava::FTTSJava()
    : FJavaClassObject("com/qualcomm/qti/voice/assist/tts/sdk/TTS", "()V"),
      SetModelPathMethodId(GetClassMethod("setModelPath", "(Ljava/lang/String;)V")),
      SetLanguageMethodId(GetClassMethod("setLanguage", "(Ljava/lang/String;)V")),
      SetAudioEncodingMethodId(GetClassMethod("setAudioEncoding", "(Ljava/lang/String;)V")),
      SetSpeechRateMethodId(GetClassMethod("setSpeechRate", "(Ljava/lang/String;)V")),
      SetPitchMethodId(GetClassMethod("setPitch", "(Ljava/lang/String;)V")),
      SetVolumeGainMethodId(GetClassMethod("setVolumeGain", "(Ljava/lang/String;)V")),
      SetSampleRateMethodId(GetClassMethod("setSampleRate", "(Ljava/lang/String;)V")),
      SetSkelLibPathMethodId(GetClassMethod("setSkelLibPath", "(Ljava/lang/String;)V")),
      InitMethodId(GetClassMethod("init", "()V")),                     //..
      StartMethodId(GetClassMethod("start", "(Ljava/lang/String;)V")), //..
      StopMethodId(GetClassMethod("stop", "()V")),                     //..
      DeInitMethodId(GetClassMethod("deInit", "()V")),                 //..
      ReleaseMethodId(GetClassMethod("release", "()V"))
{
}

FTTSJava::~FTTSJava()
{
}

bool FTTSJava::Init(const std::map<std::string, std::string> &config)
{
    //// Set model path
    // FScopedJavaObject<jstring> jModelPath = GetJString(Config.ModelPath);
    // CallMethod<void>(SetModelPathMethodId, jModelPath);

    //// Set language
    // FScopedJavaObject<jstring> JLanguage = GetJString(LanguageToString(Config.Language));
    // CallMethod<void>(SetLanguageMethodId, JLanguage);

    //// Set audio encoding
    // FScopedJavaObject<jstring> JEncoding = GetJString(AudioEncodingToString(Config.AudioEncoding));
    // CallMethod<void>(SetAudioEncodingMethodId, JEncoding);

    //// Set speech rate
    // FScopedJavaObject<jstring> JSpeechRate = GetJString(FString::Printf(TEXT("%.2f"), Config.SpeechRate));
    // CallMethod<void>(SetSpeechRateMethodId, JSpeechRate);

    //// Set pitch
    // FScopedJavaObject<jstring> JPitch = GetJString(FString::Printf(TEXT("%.2f"), Config.Pitch));
    // CallMethod<void>(SetPitchMethodId, JPitch);

    //// Set volume gain
    // FScopedJavaObject<jstring> JVolumeGain = GetJString(FString::Printf(TEXT("%.2f"), Config.VolumeGain));
    // CallMethod<void>(SetVolumeGainMethodId, JVolumeGain);

    //// Set sample rate
    // FScopedJavaObject<jstring> JSampleRate = GetJString(FString::Printf(TEXT("%d"), Config.SampleRate));
    // CallMethod<void>(SetSampleRateMethodId, JSampleRate);

    //{
    //    JNIEnv *Env = AndroidJavaEnv::GetJavaEnv();
    //    jmethodID GetNativeLibDirMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
    //                                                               "getNativeLibDir", "()Ljava/lang/String;", false);
    //    FString NativeLibDir = FJavaHelper::FStringFromLocalRef(
    //        Env, (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, GetNativeLibDirMethod));

    //    // Set skeleton library path if provided
    //    FScopedJavaObject<jstring> JSkelLibPath = GetJString(NativeLibDir);
    //    CallMethod<void>(SetSkelLibPathMethodId, JSkelLibPath);
    //}

    //// Initialize the TTS engine
    // CallMethod<void>(Method_Init);

    // if (Env->ExceptionCheck())
    //{
    //     Env->ExceptionDescribe();
    //     Env->ExceptionClear();
    //     UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Exception during initialization"));
    //     return ETTSErrorCode::InitializationFailed;
    // }

    // UE_LOG(LogTemp, Log, TEXT("TextToSpeechEngineAndroid: Initialized successfully"));
    return true; // ETTSErrorCode::Success;
}

void FTTSJava::Speak(const FString &Text)
{
    if (!Text.IsEmpty())
    {
        FScopedJavaObject<jstring> JText = GetJString(Text);
        CallMethod<void>(StartMethodId, *JText);
    }
    // JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    // if (!Env || !JavaTTSInstance)
    // {
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Invalid JNI environment or instance"));
    // 	return ETTSErrorCode::NotInitialized;
    // }

    // if (Env->ExceptionCheck())
    // {
    // 	Env->ExceptionDescribe();
    // 	Env->ExceptionClear();
    // 	UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Exception during speak"));
    // 	return ETTSErrorCode::InternalError;
    // }

    /*return ETTSErrorCode::Success;*/
}

void FTTSJava::Stop()
{
    CallMethod<void>(StopMethodId);
}

void FTTSJava::DeInit()
{
    // JNIEnv *Env = FAndroidApplication::GetJavaEnv();
    //  if (!Env || !JavaTTSInstance)
    //{
    //      return;
    //  }

    CallMethod<void>(DeInitMethodId);
    CallMethod<void>(ReleaseMethodId);

    // if (Env->ExceptionCheck())
    //{
    //     Env->ExceptionDescribe();
    //     Env->ExceptionClear();
    // }

    // CleanupJNI();
}

// bool FTTSJava::InitializeJNI()
// {
// 	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
// 	if (!Env)
// 	{
// 		return false;
// 	}

// 	// Find TTS class
// 	jclass LocalTTSClass = FAndroidApplication::FindJavaClass("");
// 	if (!LocalTTSClass)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Failed to find TTS class"));
// 		return false;
// 	}
// 	JavaTTSClass = (jclass)Env->NewGlobalRef(LocalTTSClass);
// 	Env->DeleteLocalRef(LocalTTSClass);

// 	// Find callback class
// 	jclass LocalCallbackClass =
// FAndroidApplication::FindJavaClass("com/qualcomm/qti/voice/assist/tts/sdk/TTSResultCallback"); 	if
// (!LocalCallbackClass)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Failed to find TTSResultCallback class"));
// 		return false;
// 	}
// 	JavaCallbackClass = (jclass)Env->NewGlobalRef(LocalCallbackClass);
// 	Env->DeleteLocalRef(LocalCallbackClass);

// 	// Create callback instance
// 	if (!RegisterNativeMethods(Env))
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Failed to register native methods"));
// 		return false;
// 	}

// 	// Get newInstance method
// 	jmethodID NewInstanceMethod = Env->GetStaticMethodID(JavaTTSClass, "newInstance",
// 		"(Lcom/qualcomm/qti/voice/assist/tts/sdk/TTSResultCallback;)Lcom/qualcomm/qti/voice/assist/tts/sdk/TTS;");
// 	if (!NewInstanceMethod)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Failed to find newInstance method"));
// 		return false;
// 	}

// 	// Create TTS instance
// 	jobject LocalTTSInstance = Env->CallStaticObjectMethod(JavaTTSClass, NewInstanceMethod, JavaCallbackInstance);
// 	if (!LocalTTSInstance)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("TextToSpeechEngineAndroid: Failed to create TTS instance"));
// 		return false;
// 	}
// 	JavaTTSInstance = Env->NewGlobalRef(LocalTTSInstance);
// 	Env->DeleteLocalRef(LocalTTSInstance);

// 	return true;
// }

FString LanguageToString(ETTSLanguage Language)
{
    switch (Language)
    {
    case ETTSLanguage::English:
        return TEXT("English");
    case ETTSLanguage::Chinese:
        return TEXT("Chinese");
    case ETTSLanguage::Spanish:
        return TEXT("Spanish");
    default:
        return TEXT("English");
    }
}

FString AudioEncodingToString(ETTSAudioEncoding Encoding)
{
    switch (Encoding)
    {
    case ETTSAudioEncoding::LINEAR16:
        return TEXT("0");
    case ETTSAudioEncoding::MP3:
        return TEXT("1");
    case ETTSAudioEncoding::OGG_OPUS:
        return TEXT("2");
    case ETTSAudioEncoding::MULAW:
        return TEXT("3");
    case ETTSAudioEncoding::ALAW:
        return TEXT("4");
    default:
        return TEXT("0");
    }
}

// void FTTSJava::OnStartCallback(JNIEnv *Env, jobject Thiz, jlong NativePtr, jint SampleRate, jint AudioFormat,
//                                jint ChannelCount)
//{
//     FTTSJava *Engine = reinterpret_cast<FTTSJava *>(NativePtr);
//     if (Engine && Engine->IsValidLowLevel())
//     {
//         Engine->BroadcastSpeechStarted(SampleRate, AudioFormat, ChannelCount);
//     }
// }

// void FTTSJava::OnAudioAvailableCallback(JNIEnv *Env, jobject Thiz, jlong NativePtr, jbyteArray AudioData, jint
// Offset,
//                                         jint Size)
//{
//     FTTSJava *Engine = reinterpret_cast<FTTSJava *>(NativePtr);
//     if (Engine && Engine->IsValidLowLevel())
//     {
//         // Convert Java byte array to TArray<uint8>
//         jbyte *Bytes = Env->GetByteArrayElements(AudioData, nullptr);
//         TArray<uint8> Data;
//         Data.SetNum(Size);
//         FMemory::Memcpy(Data.GetData(), Bytes + Offset, Size);
//         Env->ReleaseByteArrayElements(AudioData, Bytes, JNI_ABORT);
//
//         Engine->BroadcastAudioDataAvailable(Data);
//     }
// }
//
// void FTTSJava::OnDoneCallback(JNIEnv *Env, jobject Thiz, jlong NativePtr)
//{
//     FTTSJava *Engine = reinterpret_cast<FTTSJava *>(NativePtr);
//     if (Engine && Engine->IsValidLowLevel())
//     {
//         Engine->BroadcastSpeechCompleted();
//     }
// }
//
// void FTTSJava::OnErrorCallback(JNIEnv *Env, jobject Thiz, jlong NativePtr, jint ErrorCode)
//{
//     FTTSJava *Engine = reinterpret_cast<FTTSJava *>(NativePtr);
//     if (Engine && Engine->IsValidLowLevel())
//     {
//         Engine->BroadcastSpeechError(ETTSErrorCode::InternalError);
//     }
// }

#endif // PLATFORM_ANDROID
