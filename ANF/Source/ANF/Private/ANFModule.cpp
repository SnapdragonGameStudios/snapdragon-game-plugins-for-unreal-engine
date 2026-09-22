//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFModule.h"
#include "ANFConfig.h"
#include "ANFViewExtension.h"
#include "ANFFrameGen.h"

#include "Runtime/Launch/Resources/Version.h"

IMPLEMENT_MODULE(FANFModule, ANF)

void FANFModule::StartupModule()
{
	RegisterANFCVarCallbacks();

	//allow ANF to exist with other upscalers
	ANFViewExtension = FSceneViewExtensions::NewExtension<FANFViewExtension>();

	FANFFrameGenModule& FrameGenModule = FModuleManager::GetModuleChecked<FANFFrameGenModule>(TEXT("ANFFrameGenModule"));
	FrameGenModule.SetupFrameGenModule();
}

void FANFModule::ShutdownModule()
{
	ANFViewExtension = nullptr;
} 