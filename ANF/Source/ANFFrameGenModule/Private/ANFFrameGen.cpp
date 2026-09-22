//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFFrameGen.h"
#include "ANFFrameEstimator.h"
#include "ANFConfig.h"

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FANFFrameGenModule"

void FANFFrameGenModule::StartupModule()
{
}

void FANFFrameGenModule::ShutdownModule()
{
	ANFFrameGenViewExtension = nullptr;
}

/* not used for now, leave here for future use. */
EPluginStatus FANFFrameGenModule::GetPluginStatus() const
{
	return m_PluginStatus;
}

void FANFFrameGenModule::SetupFrameGenModule()
{
	// Stand-alone engine: the view extension hooks Present via Slate delegates itself.
	ANFFrameGenViewExtension = FSceneViewExtensions::NewExtension<FANFFrameGenViewExtension>();
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FANFFrameGenModule, ANFFrameGenModule)
