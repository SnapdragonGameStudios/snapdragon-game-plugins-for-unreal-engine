#include "SpeechRecognizer.h"
#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wow64apiset.h>
#endif
#include "ISpeechRecognizerModuleInterface.h"

namespace
{
namespace PlatformUtils
{

//
//
bool IsARM64Host()
{
    bool bIsNativeMachineArm64 = false;
#if PLATFORM_WINDOWS
    USHORT process, nativeMachineArch;

#if (_WIN32_WINNT >= 0x0A00)
    if (IsWow64Process2(GetCurrentProcess(), &process, &nativeMachineArch))
    {
        bIsNativeMachineArm64 = (nativeMachineArch == IMAGE_FILE_MACHINE_ARM64);
    }
#else
    HINSTANCE wow64apiset = LoadLibrary(TEXT("kernel32.dll"));
    if (wow64apiset != nullptr)
    {
        using ISWOW64PROCESS2FUNCPTR = BOOL (*)(HANDLE, USHORT *, USHORT *);
        ISWOW64PROCESS2FUNCPTR ptr =
            reinterpret_cast<ISWOW64PROCESS2FUNCPTR>(GetProcAddress(wow64apiset, "IsWow64Process2"));
        if (ptr != nullptr && ptr(GetCurrentProcess(), &process, &nativeMachineArch))
        {
            bIsNativeMachineArm64 = (nativeMachineArch == IMAGE_FILE_MACHINE_ARM64);
        }
        FreeLibrary(wow64apiset);
    }
#endif

#elif PLATFORM_ANDROID
    bIsNativeMachineArm64 = true;
#endif

    return bIsNativeMachineArm64;
}
} // namespace PlatformUtils
} // namespace

//
//
TUniquePtr<ISpeechRecognizer> USpeechRecognizer::GetSpeechRecognizerInstance()
{
    ISpeechRecognizerModuleInterface *recognizerModule = nullptr;
    // todo: read config for the recognizerModule string
    if (false)
    {
        recognizerModule = nullptr;
    }
    else
    {
#if PLATFORM_WINDOWS
        if (PlatformUtils::IsARM64Host())
        {
            recognizerModule = static_cast<ISpeechRecognizerModuleInterface *>(
                FModuleManager::Get().LoadModule("VoiceAISpeechRecognizerWindows"));
        }
#elif PLATFORM_ANDROID
        recognizerModule =
            (ISpeechRecognizerModuleInterface *)(FModuleManager::Get().LoadModule("VoiceAISpeechRecognizerAndroid"));
#endif
    }

    if (recognizerModule != nullptr)
    {
        return recognizerModule->GetSpeechRecognizer();
    }

    return nullptr;
}
