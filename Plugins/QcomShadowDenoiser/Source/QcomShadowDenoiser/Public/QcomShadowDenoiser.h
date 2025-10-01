//============================================================================================================
//
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FQcomShadowDenoiserImplementation;

class FQcomShadowDenoiserModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void PostEngineInit();
	void PreExit();

	void RegisterSettings();
	void UnregisterSettings();

	TUniquePtr<FQcomShadowDenoiserImplementation> m_DenoiserImplementation;
};
