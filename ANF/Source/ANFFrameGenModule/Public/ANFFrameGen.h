//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "Modules/ModuleManager.h"
#include "ANFConfig.h"

//class FANFFrameEstimator;
class FANFFrameGenViewExtension;

enum class EPluginStatus
{
	Supported,
	NotSupported,
	NotSupportedUnsupportedGPU,
	NotSupportedUnsupportedDriver,
	NotSupportedIncompatibleAPICaptureToolActive
};

class ANFFRAMEGENMODULE_API FANFFrameGenModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual EPluginStatus GetPluginStatus() const;

	static inline bool IsFrameEstimationEnabled()
	{
		return GANFFrameGenEnabled != 0;
	}

	void SetupFrameGenModule();

private:
	TSharedPtr<FANFFrameGenViewExtension, ESPMode::ThreadSafe> ANFFrameGenViewExtension;


	EPluginStatus m_PluginStatus = EPluginStatus::NotSupported;
};

