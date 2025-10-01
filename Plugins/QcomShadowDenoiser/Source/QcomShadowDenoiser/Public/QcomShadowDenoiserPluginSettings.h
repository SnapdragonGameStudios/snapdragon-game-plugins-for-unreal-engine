//============================================================================================================
//
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"
#include "QcomShadowDenoiserPluginSettings.generated.h"

UENUM()
enum class EDenoiserMode
{
	Temporal = 0	UMETA(DisplayName = "Temporal"),
	Unreal			UMETA(DisplayName = "Unreal (built-in)"),
	None			UMETA(DisplayName = "None")
};
	
UENUM()
enum class EDenoiserTemporalReject
{
	None = 0		UMETA(DisplayName = "None"),
	Depth			UMETA(DisplayName = "Depth Region"),
	Filter			UMETA(DisplayName = "Filter Region")
};

// Accessors for settings (updated by both settings and CVar)
extern EDenoiserMode			GetQcomShadowDenoiserMode();
extern EDenoiserTemporalReject	GetGQcomShadowDenoiserRejectMode();
extern bool						GetGQcomShadowDenoiserClassificationOptimization();


/** Settings for the Qualcomm shadow denoiser plug-in */
UCLASS(config = Engine, meta = (DisplayName = "Qcom Shadow Denoiser"))
class UQcomShadowDenoiserPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UQcomShadowDenoiserPluginSettings(const FObjectInitializer& obj);

#if WITH_EDITOR
	bool SupportsAutoRegistration() const override { return false; }
#endif // WITH_EDITOR

	UPROPERTY(config, EditAnywhere, Category = "QcomShadowDenoiser", meta = (ConsoleVariable = "r.QcomShadowDenoiser.Mode", DisplayName = "Denoiser Mode"))
		EDenoiserMode Mode;
	UPROPERTY(config, EditAnywhere, Category = "QcomShadowDenoiser", meta = (ConsoleVariable = "r.QcomShadowDenoiser.RejectMode", DisplayName = "Temporal Reject Mode", EditCondition = "Mode == EDenoiserMode::Temporal"))
		EDenoiserTemporalReject TemporalReject;
	UPROPERTY(config, EditAnywhere, Category = "QcomShadowDenoiser", AdvancedDisplay, meta = (ConsoleVariable = "r.QcomShadowDenoiser.ClassificationOptimization", DisplayName = "Tile Classification Optimization", EditCondition = "Mode == EDenoiserMode::Temporal"))
		bool TileClassificationOptimization;

protected:
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
