//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "Modules/ModuleManager.h"

class FANFViewExtension;

class FANFModule final : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

private:
	TSharedPtr<FANFViewExtension, ESPMode::ThreadSafe> ANFViewExtension;
};