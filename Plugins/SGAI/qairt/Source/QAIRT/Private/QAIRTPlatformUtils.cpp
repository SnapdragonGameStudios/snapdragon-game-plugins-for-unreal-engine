// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "QAIRTPlatformUtils.h"
#include "Interfaces/IPluginManager.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wow64apiset.h>
#endif

bool UQAIRTPlatformUtils::IsARM64Host()
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
#pragma warning(push)
#pragma warning(disable : 4191)
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
#pragma warning(pop)
#endif
    UE_LOG(LogTemp, Display, TEXT("Is Running on ARM64 = %d"), bIsNativeMachineArm64);
#elif PLATFORM_ANDROID
    bIsNativeMachineArm64 = true;
#endif

    return bIsNativeMachineArm64;
}

FString UQAIRTPlatformUtils::GetBinariesPath()
{
    FString platform_sub_dir;

#if PLATFORM_WINDOWS
#if defined(_M_ARM64)
    platform_sub_dir = "arm64x-windows-msvc";
#else
    platform_sub_dir = IsARM64Host() ? "arm64x-windows-msvc" : "x86_64-windows-msvc";
#endif
#endif

#if PLATFORM_ANDROID

#endif
    return FPaths::ConvertRelativePathToFull(FPaths::Combine(*(IPluginManager::Get().FindPlugin("QAIRT")->GetBaseDir()),
                                                             "Source/ThirdParty/qairt/lib", *platform_sub_dir));
}
