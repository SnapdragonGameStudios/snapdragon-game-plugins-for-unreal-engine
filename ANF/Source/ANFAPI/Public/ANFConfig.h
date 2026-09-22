//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#if defined(_WIN32)
#include "Microsoft/WindowsHWrapper.h"
#endif
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "RHIDefinitions.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"

#include "ANFConfig.generated.h"

extern ANFAPI_API int32 GANFInWidth;
extern ANFAPI_API int32 GANFInHeight;
extern ANFAPI_API int32 GANFMinWidth;
extern ANFAPI_API int32 GANFMinHeight;
extern ANFAPI_API float GANFUpscaleFactor;
extern ANFAPI_API int32 GANFMatchResolution;
extern ANFAPI_API int32 GANFJitterPrevClip;
extern ANFAPI_API int32 GANFTimePasses;
extern ANFAPI_API float GANFProfilePrintTime;
extern ANFAPI_API int32 GANFUseJitterPattern;
extern ANFAPI_API int32 GANFMaxNumInFlight;

extern ANFAPI_API int32 GEnableANF;
extern ANFAPI_API int32 GANFPreferHalfFloatOutput;
extern ANFAPI_API int32 GANFPreferUnormInputs;

extern ANFAPI_API int32 GANFFrameGenEnabled;
extern ANFAPI_API int32 GANFFrameGenSupportsLowResInputs;
extern ANFAPI_API int32 GANFFrameGenSupportsD24S8;
extern ANFAPI_API int32 GANFFrameGenAddFinalBlit;

extern ANFAPI_API int32 GANFFrameGenSubmissionOrder;
extern ANFAPI_API int32 GANFFrameGenUseFramePacer;
extern ANFAPI_API int32 GANFFrameGenPacerDebugPrint;
extern ANFAPI_API int32 GANFFrameGenFramePacerSampleCount;
extern ANFAPI_API int32 GANFFrameGenFramePacerTargetFPS;

extern ANFAPI_API int32 GANFDumpFrames;
extern ANFAPI_API int32 GANFDumpFramesFirstFrame;
extern ANFAPI_API int32 GANFDumpFramesNumFrames;
extern ANFAPI_API int32 GANFDebugSkipDispatch;
extern ANFAPI_API int32 GANFDebugAllowD32S8;
extern ANFAPI_API int32 GANFDebugShowInfoMessages;

extern ANFAPI_API int32 GANFDebugOverlayAllowed;
extern ANFAPI_API int32 GANFDebugOverlayMode;
extern ANFAPI_API int32 GANFDebugOverlayHUDCorner;
extern ANFAPI_API float GANFDebugOverlayMvHeatmapScale;
extern ANFAPI_API int32 GANFDebugOverlayShowRawMv;
extern ANFAPI_API float GANFDebugOverlayMvValueScaleX;
extern ANFAPI_API float GANFDebugOverlayMvValueScaleY;
extern ANFAPI_API float GANFDebugOverlayJitterScaleX;
extern ANFAPI_API float GANFDebugOverlayJitterScaleY;
extern ANFAPI_API float GANFDebugOverlayJitterAccumAlpha;
extern ANFAPI_API int32 GANFDebugOverlayAccumulateJitter;
extern ANFAPI_API int32 GANFDebugOverlayHUDFlipY;
extern ANFAPI_API int32 GANFDebugOverlayDepthInvert;
extern ANFAPI_API int32 GANFDebugOverlayDepthScale;

extern ANFAPI_API TAutoConsoleVariable<int32> CVarEnableANF;
extern ANFAPI_API TAutoConsoleVariable<int32> CVarANFUseJitterPattern;
extern ANFAPI_API TAutoConsoleVariable<int32> CVarANFFrameGenEnabled;

extern ANFAPI_API bool ShouldDumpANFFrames(uint32 curFrameNumber);

extern ANFAPI_API void RegisterANFCVarCallbacks();

UCLASS(Config = Engine, DefaultConfig, DisplayName = "ANF Pre-Release")
class ANFAPI_API UANFSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()
public:
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(Config, EditAnywhere, Category = "General Settings", meta = (ConsoleVariable = "r.ANF.Enabled", DisplayName = "Enabled"))
	bool bEnabled;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", meta = (ConsoleVariable = "r.ANF.Width", DisplayName = "Input Width"))
	int32 anfWidth;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", meta = (ConsoleVariable = "r.ANF.Height", DisplayName = "Input Height"))
	int32 anfHeight;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", meta = (ConsoleVariable = "r.ANF.UpscaleFactor", DisplayName = "Scale Factor"))
	float anfScaleFactor;

	UPROPERTY(Config, EditAnywhere, Category = "General Settings", meta = (ConsoleVariable = "r.ANF.FrameGen.Enable", DisplayName = "Enable Frame Gen"))
	bool bFrameGenEnabled;
};
