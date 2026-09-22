//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFTUModule.h"
#include "ANFTU.h"
#include "ANFConfig.h"

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ConfigCacheIni.h"
#if ENGINE_MINOR_VERSION > 0
#include "Misc/ConfigUtilities.h"
#endif
DEFINE_LOG_CATEGORY(LogANFTU);

IMPLEMENT_MODULE(FANFTUModule, ANFTUModule)

#define LOCTEXT_NAMESPACE "ANF"

void FANFTUModule::StartupModule()
{
	FString PluginUshaderdir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ANF"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ANF"), PluginUshaderdir);
#if ENGINE_MINOR_VERSION > 0
	UE::ConfigUtilities::ApplyCVarSettingsFromIni(TEXT("/Script/ANFTUModule.ANFSettings"), *GEngineIni, ECVF_SetByProjectSetting);
#else
	ApplyCVarSettingsFromIni(TEXT("/Script/ANFTUModule.ANFSettings"), *GEngineIni, ECVF_SetByProjectSetting);
#endif
	UE_LOG(LogANFTU, Log, TEXT("ANFTUModule Started"));
}

void FANFTUModule::ShutdownModule()
{
	UE_LOG(LogANFTU, Log, TEXT("ANFTUModule Shutdown"));
}

void FANFTUModule::SetTU(TSharedPtr<FANFTU, ESPMode::ThreadSafe> Upscaler)
{
	TemporalUpscaler = Upscaler;
}

IANFTemporalUpscaler* FANFTUModule::GetTU() const
{
	return TemporalUpscaler.Get();
}

FANFTU* FANFTUModule::GetANFU() const
{
	return TemporalUpscaler.Get();
}

#undef LOCTEXT_NAMESPACE