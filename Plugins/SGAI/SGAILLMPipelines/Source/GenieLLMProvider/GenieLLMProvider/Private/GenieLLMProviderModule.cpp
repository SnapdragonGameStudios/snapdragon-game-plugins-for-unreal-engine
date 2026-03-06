// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "GenieLLMProviderModule.h"
#include "AndroidUtils.h"
#include "GenieLLMProvider.h"
#include "Modules/ModuleManager.h"
#include "QAIRTPlatformUtils.h"

#define LOCTEXT_NAMESPACE "GenieLLMProviderModule"

std::unique_ptr<LLMPipelinesNative::ILLMModel> GenieLLMProviderModule::CreateLLMModel(
    const std::map<std::string, std::string> &config)
{
    FString QNNDllDir = UQAIRTPlatformUtils::GetBinariesPath();
    FPlatformProcess::PushDllDirectory(*QNNDllDir);
    std::unique_ptr<LLMPipelinesNative::ILLMModel> ptr = GenieLLMInternal::CreateLLMModel(config);
    FPlatformProcess::PopDllDirectory(*QNNDllDir);
    return ptr;
}
std::unique_ptr<LLMPipelinesNative::IEmbedder> GenieLLMProviderModule::CreateEmbedder(
    const std::map<std::string, std::string> &config)
{
    FString QNNDllDir = UQAIRTPlatformUtils::GetBinariesPath();
    FPlatformProcess::PushDllDirectory(*QNNDllDir);
    std::unique_ptr<LLMPipelinesNative::IEmbedder> ptr = GenieLLMInternal::CreateEmbedder(config);
    FPlatformProcess::PopDllDirectory(*QNNDllDir);
    return ptr;
}
std::unique_ptr<LLMPipelinesNative::IVectorDB> GenieLLMProviderModule::CreateVectorDB(
    const std::map<std::string, std::string> &config)
{
    return nullptr;
}

namespace
{
namespace LibUtils
{
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
        const FString LibraryPath = FPaths::Combine(*Folder, LibraryName);
        Handle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
        if (!Handle)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load lib from %s"), *LibraryPath);
        }
    }
    FPlatformProcess::PopDllDirectory(*Folder);
    return Handle;
}
} // namespace LibUtils
} // namespace

void GenieLLMProviderModule::StartupModule()
{
#if PLATFORM_ANDROID
    SetupEnvironmentForGENIE();
#endif // PLATFORM_ANDROID
#if PLATFORM_WINDOWS
    LibraryHandle = LibUtils::Load("Genie.dll", UQAIRTPlatformUtils::GetBinariesPath());
#endif // PLATFORM_WINDOWS
}

void GenieLLMProviderModule::ShutdownModule()
{
    LibUtils::Free(LibraryHandle);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(GenieLLMProviderModule, GenieLLMProvider)
