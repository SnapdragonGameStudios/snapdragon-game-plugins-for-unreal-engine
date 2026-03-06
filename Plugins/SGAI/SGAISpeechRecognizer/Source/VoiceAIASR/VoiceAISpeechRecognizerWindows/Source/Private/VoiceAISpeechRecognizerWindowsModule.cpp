// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "VoiceAISpeechRecognizerWindowsModule.h"
#include "Modules/ModuleManager.h"
#include "PathUtils.h"
#include "QAIRTPlatformUtils.h"
#include "VoiceAISpeechRecognizerWindows.h"

#define LOCTEXT_NAMESPACE "FVoiceAISpeechRecognizerWindowsModule"

/**
 *
 */
TUniquePtr<ISpeechRecognizer> FVoiceAISpeechRecognizerWindowsModule::GetSpeechRecognizer()
{
    return MakeUnique<VoiceAISpeechRecognizerWindows>();
}

namespace
{
namespace LibUtils
{
//
//
void Free(void *&Handle)
{
    if (Handle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(Handle);
        Handle = nullptr;
    }
}

// get the directory name from build.cs
//  built version - x64, arm64ec, arm64
//  platform version - x64, arm64
void *Load(const FString &LibraryName, const FString &Folder)
{
    void *Handle = nullptr;

    FPlatformProcess::PushDllDirectory(*Folder);
    {
        // Load WhisperLib.dll - This shouldn't be needed as it should be dynamically loaded. Can remove this once fixed
        // in the library.
        const FString LibraryPath = FPaths::Combine(*Folder, LibraryName);
        Handle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
        if (!Handle)
        {
            UE_LOG(LogVoiceAIASRWindows, Error, TEXT("Failed to load WhisperLib from %s"), *LibraryPath);
        }
    }
    FPlatformProcess::PopDllDirectory(*Folder);
    return Handle;
}
} // namespace LibUtils
} // namespace

//
//
void FVoiceAISpeechRecognizerWindowsModule::StartupModule()
{
    // load dlls
#if PLATFORM_WINDOWS
    {
        // Set Env Var for WhisperLib to load QNN Binarie from the correct path.
        FString env = FString::Printf(TEXT("ADSP_LIBRARY_PATH=%s"), *UQAIRTPlatformUtils::GetBinariesPath());
        _putenv(TCHAR_TO_UTF8(*env));
    }

    const FString VoiceAILibDir = VoiceAIASR::GetBinariesPath();
    WhisperLibLibraryHandle = LibUtils::Load("WhisperLib.dll", VoiceAILibDir);
    WhisperComponentLibraryHandle = LibUtils::Load("WhisperComponent.dll", VoiceAILibDir);
#endif
}

//
//
void FVoiceAISpeechRecognizerWindowsModule::ShutdownModule()
{
    LibUtils::Free(WhisperLibLibraryHandle);
    LibUtils::Free(WhisperComponentLibraryHandle);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoiceAISpeechRecognizerWindowsModule, VoiceAISpeechRecognizerWindows)
