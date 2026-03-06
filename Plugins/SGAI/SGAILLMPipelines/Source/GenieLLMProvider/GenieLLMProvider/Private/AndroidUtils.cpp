#include "AndroidUtils.h"
#include "CoreMinimal.h"

#if PLATFORM_ANDROID
#if USE_ANDROID_JNI
#include "Android/AndroidJavaEnv.h"
#endif // USE_ANDROID_JNI

FString GetAndroidNativeLibraryDir()
{
    FString Result = "";

#if USE_ANDROID_JNI
    // ** use the JNI to execute: `GameActivity.this.getApplication().getApplicationInfo().nativeLibraryDir` **

    JNIEnv *JEnv = AndroidJavaEnv::GetJavaEnv();
    check(JEnv != nullptr);

    jobject GameActivity = AndroidJavaEnv::GetGameActivityThis();
    jclass GameActivityClass = JEnv->GetObjectClass(GameActivity);
    check(GameActivityClass != nullptr);

    jmethodID GetApplicationMethod =
        JEnv->GetMethodID(GameActivityClass, "getApplication", "()Landroid/app/Application;");
    jobject Application = JEnv->CallObjectMethod(GameActivity, GetApplicationMethod);
    check(Application != nullptr);

    jclass ApplicationClass = JEnv->GetObjectClass(Application);
    check(ApplicationClass != nullptr);

    jmethodID GetApplicationInfoMethod =
        JEnv->GetMethodID(ApplicationClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    jobject ApplicationInfo = JEnv->CallObjectMethod(Application, GetApplicationInfoMethod);
    check(ApplicationInfo != nullptr);

    jclass ApplicationInfoClass = JEnv->GetObjectClass(ApplicationInfo);
    check(ApplicationInfoClass != nullptr);

    jfieldID NativeLibraryDir = JEnv->GetFieldID(ApplicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");
    auto JavaString = (jstring)JEnv->GetObjectField(ApplicationInfo, NativeLibraryDir);
    const auto Chars = JEnv->GetStringUTFChars(JavaString, 0);
    Result = FString(UTF8_TO_TCHAR(Chars));

#endif // USE_ANDROID_JNI

    return Result;
}

void SetupEnvironmentForGENIE()
{
    const FString EnvVariableName = "ADSP_LIBRARY_PATH";

    FString NativeLibraryPath = GetAndroidNativeLibraryDir();
    FString ADSPLibraryPaths = NativeLibraryPath;

    UE_LOG(LogAndroid, Verbose, TEXT("SNPE: setting %s to %s"), *EnvVariableName, *ADSPLibraryPaths);
    if (setenv(TCHAR_TO_UTF8(*EnvVariableName), TCHAR_TO_UTF8(*ADSPLibraryPaths), 1) != 0)
    {
        UE_LOG(LogAndroid, Warning, TEXT("SNPE: Failed to set %s - DSP execution will NOT be supported!"),
               *EnvVariableName);
    }
    else
    {
        UE_LOG(LogAndroid, Verbose, TEXT("ADSP_LIBRARY_PATH: set %s successfully, getenv() returns: %s"),
               *EnvVariableName, ANSI_TO_TCHAR(getenv(TCHAR_TO_UTF8(*EnvVariableName))));
    }
}
#endif // PLATFORM_ANDROID
