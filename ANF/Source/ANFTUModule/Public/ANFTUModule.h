//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "Modules/ModuleManager.h"
#include "RHIDefinitions.h"

DECLARE_LOG_CATEGORY_EXTERN(LogANFTU, Verbose, All);

class FANFTU;
#if ENGINE_MINOR_VERSION > 2
using IANFTemporalUpscaler = UE::Renderer::Private::ITemporalUpscaler;
#else
class ITemporalUpscaler;
#endif

class FANFTUModule final : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	void SetTU(TSharedPtr<FANFTU, ESPMode::ThreadSafe> Upscaler);

	FANFTU* GetANFU() const;
#if ENGINE_MINOR_VERSION > 2
	IANFTemporalUpscaler* GetTU() const;
#else
	ITemporalUpscaler* GetTU() const;
#endif
private:
	TSharedPtr<FANFTU, ESPMode::ThreadSafe> TemporalUpscaler;
};
