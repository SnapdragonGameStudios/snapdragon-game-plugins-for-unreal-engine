//============================================================================================================
//
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "QcomShadowDenoiserPluginSettings.h"
#include "Engine/DeveloperSettings.h"

// Statics that can be set by CVar (and/or settings)
static int32 GQcomShadowDenoiserMode = (int32)EDenoiserMode::Temporal;
static int32 GQcomShadowDenoiserRejectMode = (int32)EDenoiserTemporalReject::None;
static bool GQcomShadowDenoiserClassificationOptimization = true;

// Global accessors
EDenoiserMode GetQcomShadowDenoiserMode()
{
	return (EDenoiserMode) GQcomShadowDenoiserMode;
}
EDenoiserTemporalReject GetGQcomShadowDenoiserRejectMode()
{
	return (EDenoiserTemporalReject) GQcomShadowDenoiserRejectMode;
}
bool GetGQcomShadowDenoiserClassificationOptimization()
{
	return GQcomShadowDenoiserClassificationOptimization;
}

static FAutoConsoleVariableRef CVarQcomShadowDenoiserMode(
	TEXT("r.QcomShadowDenoiser.Mode"),
	GQcomShadowDenoiserMode,
	TEXT("Which denoiser mode to use."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarQcomShadowDenoiserRejectMode(
	TEXT("r.QcomShadowDenoiser.RejectMode"),
	GQcomShadowDenoiserRejectMode,
	TEXT("Which denoiser mode to use."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarQcomShadowDenoiserClassificationOptimization(
	TEXT("r.QcomShadowDenoiser.ClassificationOptimization"),
	GQcomShadowDenoiserClassificationOptimization,
	TEXT("Enable denoiser tile optimization."),
	ECVF_Default
);

UQcomShadowDenoiserPluginSettings::UQcomShadowDenoiserPluginSettings(const FObjectInitializer& obj)
	: Super(obj)
	, Mode(EDenoiserMode::Temporal)
	, TemporalReject(EDenoiserTemporalReject::None)
	, TileClassificationOptimization(true)
{
}

void UQcomShadowDenoiserPluginSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	if (IsTemplate())
	{
		ImportConsoleVariableValues();
	}
#endif
}

#if WITH_EDITOR
void UQcomShadowDenoiserPluginSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
}
#endif

