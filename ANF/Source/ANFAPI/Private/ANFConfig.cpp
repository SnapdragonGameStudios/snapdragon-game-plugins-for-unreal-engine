//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFConfig.h"

#include "ANFBackend.h"

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ConfigCacheIni.h"
#if ENGINE_MINOR_VERSION > 0
#include "Misc/ConfigUtilities.h"
#endif
#define LOCTEXT_NAMESPACE "FANFModule"

int32 GANFInWidth = 0;
int32 GANFInHeight = 0;
int32 GANFMinWidth = 32;
int32 GANFMinHeight = 32;
float GANFUpscaleFactor = 2.f;
int32 GANFMatchResolution = 0;
int32 GANFJitterPrevClip = 0;
int32 GANFTimePasses = 0;
float GANFProfilePrintTime = 1.f;
int32 GANFUseJitterPattern = 1;
int32 GANFMaxNumInFlight = 3;

int32 GEnableANF = 0;
int32 GANFPreferHalfFloatOutput = 0;
int32 GANFPreferUnormInputs = 0;

int32 GANFFrameGenEnabled = 0;
int32 GANFFrameGenSupportsLowResInputs = 0;
int32 GANFFrameGenSupportsD24S8 = 0;
int32 GANFFrameGenAddFinalBlit = 0;

int32 GANFFrameGenSubmissionOrder = 1;
int32 GANFFrameGenUseFramePacer = 0;
int32 GANFFrameGenPacerDebugPrint = 0;
int32 GANFFrameGenFramePacerSampleCount = 0;
int32 GANFFrameGenFramePacerTargetFPS = 0;

int32 GANFDumpFrames = 0;
int32 GANFDumpFramesFirstFrame = 0;
int32 GANFDumpFramesNumFrames = 0;
int32 GANFDebugSkipDispatch = 0;
int32 GANFDebugAllowD32S8 = 1;
int32 GANFDebugShowInfoMessages = 0;

int32 GANFDebugOverlayAllowed = 0;
int32 GANFDebugOverlayMode = 0;
int32 GANFDebugOverlayHUDCorner = 0;
float GANFDebugOverlayMvHeatmapScale = 1.f;
int32 GANFDebugOverlayShowRawMv = 0;
float GANFDebugOverlayMvValueScaleX = 1.0;
float GANFDebugOverlayMvValueScaleY = 1.0;
float GANFDebugOverlayJitterScaleX = 1.0;
float GANFDebugOverlayJitterScaleY = 1.0;
float GANFDebugOverlayJitterAccumAlpha = 0.5;
int32 GANFDebugOverlayAccumulateJitter = 0;
int32 GANFDebugOverlayHUDFlipY = 0;
int32 GANFDebugOverlayDepthInvert = 0;
int32 GANFDebugOverlayDepthScale = 1;

bool ShouldDumpANFFrames(uint32 curFrameNumber)
{
	if (GANFDumpFrames != 0)
	{
		int32_t firstFrame = GANFDumpFramesFirstFrame;
		int32_t numFrames = GANFDumpFramesNumFrames;
		if (firstFrame >= 0 && numFrames >= 0)
		{
			int32_t lastFrame = (firstFrame + numFrames);
			int32_t curFrame = (int32_t)curFrameNumber;
			return ((curFrame >= firstFrame) && ((numFrames == 0) || (curFrame < lastFrame)));
		}
		return true;
	}
	return false;
}

TAutoConsoleVariable<int32> CVarEnableANF(
	TEXT("r.ANF.Enabled"),
	GEnableANF,
	TEXT("Enable QCOM Game Super Resolution for Temporal Upsampling"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFInWidth(
	TEXT("r.ANF.Width"),
	GANFInWidth,
	TEXT("ANF model input width."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFInHeight(
	TEXT("r.ANF.Height"),
	GANFInHeight,
	TEXT("ANF model input height."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFMinWidth(
	TEXT("r.ANF.MinWidth"),
	GANFInWidth,
	TEXT("Minimum ANF model output width."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFMinHeight(
	TEXT("r.ANF.MinHeight"),
	GANFInHeight,
	TEXT("Minimum ANF model output height."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFUpscaleFactor(
	TEXT("r.ANF.UpscaleFactor"),
	GANFUpscaleFactor,
	TEXT("ANF upscale factor. Default is 2."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFMatchResolution(
	TEXT("r.ANF.MatchResolution"),
	GANFMatchResolution,
	TEXT("Match ANF resolution"),
	ECVF_Default
);


static FAutoConsoleVariableRef CVarANFJitterPrevClip(
	TEXT("r.ANF.JitterPrevClip"),
	GANFJitterPrevClip,
	TEXT("ANF Correct jitter prev clip.  0 to not, 1 to use (default)"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDumpFrames(
	TEXT("r.ANF.Debug.FrameDump"),
	GANFDumpFrames,
	TEXT("Dump ANF Frames"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFPreferHalfFloatOutput(
	TEXT("r.ANF.HalfFloatOutput"),
	GANFPreferHalfFloatOutput,
	TEXT("use half float outputs (1) or 111110 (0)"),
	ECVF_ReadOnly
);

static FAutoConsoleVariableRef CVarANFPreferUnormInputs(
	TEXT("r.ANF.UnormInputs"),
	GANFPreferUnormInputs,
	TEXT("use Unorm inputs (1) or 11110 (0)"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFTimePasses(
	TEXT("r.ANF.TimePasses"),
	GANFTimePasses,
	TEXT("Time ANF passes and print results to logcat"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFProfilePrintTime(
	TEXT("r.ANF.ProfilePrintTime"),
	GANFProfilePrintTime,
	TEXT("Seconds to print profiling results, if enabled"),
	ECVF_Default
);

TAutoConsoleVariable<int32> CVarANFUseJitterPattern(
	TEXT("r.ANF.UseJitterPattern"),
	GANFUseJitterPattern,
	TEXT("Use special ANF jitter pattern"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFMaxNumInFlight(
	TEXT("r.ANF.MaxNumInFlight"),
	GANFMaxNumInFlight,
	TEXT("Maximum in flight SDK dispatches"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFFrameGenSupportsLowResInputs(
	TEXT("r.ANF.FrameGen.SupportsLowResInputs"),
	GANFFrameGenSupportsLowResInputs,
	TEXT("1 if FrameGen supports low res inputs, 0 otherwise"),
	ECVF_ReadOnly
);

static FAutoConsoleVariableRef CVarANFFrameGenAddFinalBlit(
	TEXT("r.ANF.FrameGen.AddFinalBlit"),
	GANFFrameGenAddFinalBlit,
	TEXT("1 to create a temporary output buffer then blit, 0 to not"),
	ECVF_Default
); 

static FAutoConsoleVariableRef CVarANFFrameGenSubmissionOrder(
	TEXT("r.ANF.FrameGen.SubmissionOrder"),
	GANFFrameGenSubmissionOrder,
	TEXT("0 to use default order, 1 to rearrange for better GPU utilization"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFFrameGenUseFramePacer(
	TEXT("r.ANF.FrameGen.UseFramePacer"),
	GANFFrameGenUseFramePacer,
	TEXT("0 to use UE frame pacers, 1 enable if FrameGen is enabled"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFFrameGenPacerDebugPrint(
	TEXT("r.ANF.FrameGen.Debug.FramePacerPrint"),
	GANFFrameGenPacerDebugPrint,
	TEXT("1 to print frame pacer information if enabled, 0 to not"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFFrameGenFramePacerSampleCount(
	TEXT("r.ANF.FrameGen.FramePacerSampleCount"),
	GANFFrameGenFramePacerSampleCount,
	TEXT("Samples to keep for the runing frame pacer average."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFFrameGenFramePacerTargetFPS(
	TEXT("r.ANF.FrameGen.FramePacerTargetFPS"),
	GANFFrameGenFramePacerTargetFPS,
	TEXT("FPS to target with the CPU frame pacer"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarDebugSkipDispatch(
	TEXT("r.ANF.Debug.SkipDispatch"),
	GANFDebugSkipDispatch,
	TEXT("Skip ANF technique dispatch and copy textures instead"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarDebugAllowD32S8(
	TEXT("r.ANF.Debug.AllowD32S8"),
	GANFDebugAllowD32S8,
	TEXT("Allow passing D32S8 as a D32 buffer for ANF dispatches"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarDebugShowInfoMessages(
	TEXT("r.ANF.Debug.ShowInfoMessages"),
	GANFDebugShowInfoMessages,
	TEXT("Show INFO messages from the SDK"),
	ECVF_ReadOnly
);

TAutoConsoleVariable<int32> CVarANFFrameGenEnabled(
	TEXT("r.ANF.FrameGen.Enable"),
	GANFFrameGenEnabled,
	TEXT("Whether ANF frame generation is enabled."),
	ECVF_Default);

static FAutoConsoleVariableRef CVarANFFrameGenDumpFramesFirstFrame(
	TEXT("r.ANF.Debug.FrameDump.FirstFrame"),
	GANFDumpFramesFirstFrame,
	TEXT("Dump ANF Frames"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDumpFramesNumFrames(
	TEXT("r.ANF.Debug.FrameDump.NumFrames"),
	GANFDumpFramesNumFrames,
	TEXT("Dump ANF Frames"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayAllowed(
	TEXT("r.ANF.DebugOverlay.Allowed"),
	GANFDebugOverlayAllowed,
	TEXT("1 to allow using the ANF debug overlay, 0 to disable.Needs to be set at ANF instance creation time."),
	ECVF_Default // ReadOnly since this needs to be set early to take effect
);

static FAutoConsoleVariableRef CVarANFDebugOverlayMode(
	TEXT("r.ANF.DebugOverlay.Mode"),
	GANFDebugOverlayMode,
	TEXT("ANF debug overlay mode to use:\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_NONE = 0,  ///< No overlay drawn this dispatch.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_INPUT_COLOR = 1,  ///< Bilinear-upsampled jittered input color, replacing the output (technique still runs). Knobs: none.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_MV_HEATMAP = 2,  ///< HSV heatmap of the motion-vector field blended over the output. Knobs: mvHeatmapScale, showRawMv, mvValueScale.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_DEPTH_VIS = 3,  ///< Depth buffer color ramp. Shows depth discontinuities. Knobs: depthInvert, depthScale.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_WARP_PREDICT = 4,  ///< Previous output warped by MVs. Shows warp geometry. Knobs: mvValueScale.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_REPROJECT_ERROR = 5,  ///< Warp-vs-current error heat map: blue=good MVs, red=bad. Knobs: mvValueScale.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_JITTER_PLOT = 6,  ///< Scatter plot of recent jitter offsets. Knobs: none.\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_JITTER_ACCUMULATE = 7,  ///< Phase-based jitter accumulation; shows sub-pixel coverage correctness. Knobs: accumulateJitter, jitterAccumAlpha, jitterScale (sign only).\n")
	TEXT("ANF_DEBUG_OVERLAY_MODE_SR_JITTER_SCALE = 8,  ///< Output with jitter scaled by jitterScale before neural dispatch. Knobs: jitterScale (value).\n"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayHUDCorner(
	TEXT("r.ANF.DebugOverlay.HUDCorner"),
	GANFDebugOverlayHUDCorner,
	TEXT("ANF debug overlay Cornet to dispaly the overlay:\n")
	TEXT("ANF_DEBUG_OVERLAY_HUD_CORNER_TOP_LEFT = 0,\n")
	TEXT("ANF_DEBUG_OVERLAY_HUD_CORNER_TOP_RIGHT = 1,\n")
	TEXT("ANF_DEBUG_OVERLAY_HUD_CORNER_BOTTOM_LEFT = 2,\n")
	TEXT("ANF_DEBUG_OVERLAY_HUD_CORNER_BOTTOM_RIGHT = 3,\n"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayMvHeatmapScale(
	TEXT("r.ANF.DebugOverlay.MvHeatmapScale"),
	GANFDebugOverlayMvHeatmapScale,
	TEXT("MV_HEATMAP color saturation scale"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayShowRawMv(
	TEXT("r.ANF.DebugOverlay.ShowRawMv"),
	GANFDebugOverlayShowRawMv,
	TEXT("MV_HEATMAP: raw direction-colored channels instead of blended heatmap"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayMvValueScaleX(
	TEXT("r.ANF.DebugOverlay.MvValueScaleX"),
	GANFDebugOverlayMvValueScaleX,
	TEXT("Per-axis multiplier on raw MV (x). Modes 2/4/5."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayMvValueScaleY(
	TEXT("r.ANF.DebugOverlay.MvValueScaleY"),
	GANFDebugOverlayMvValueScaleY,
	TEXT("Per-axis multiplier on raw MV (y). Modes 2/4/5."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayJitterScaleX(
	TEXT("r.ANF.DebugOverlay.JitterScaleX"),
	GANFDebugOverlayJitterScaleX,
	TEXT("Jitter sign/scale (x). Modes 7 (sign) and 8 (value)."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayJitterScaleY(
	TEXT("r.ANF.DebugOverlay.JitterScaleY"),
	GANFDebugOverlayJitterScaleY,
	TEXT("Jitter sign/scale (y). Modes 7 (sign) and 8 (value)."),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayJitterAccumAlpha(
	TEXT("r.ANF.DebugOverlay.JitterAccumAlpha"),
	GANFDebugOverlayJitterAccumAlpha,
	TEXT("JITTER_ACCUMULATE blend alpha (0..1)"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayAccumulateJitter(
	TEXT("r.ANF.DebugOverlay.AccumulateJitter"),
	GANFDebugOverlayAccumulateJitter,
	TEXT("JITTER_ACCUMULATE enable"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayHUDFlipY(
	TEXT("r.ANF.DebugOverlay.HUDFlipY"),
	GANFDebugOverlayHUDFlipY,
	TEXT("Flip HUD text for Y-inverted framebuffers"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayDepthInvert(
	TEXT("r.ANF.DebugOverlay.DepthInvert"),
	GANFDebugOverlayDepthInvert,
	TEXT("DEPTH_VIS: invert depth before colorizing"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVarANFDebugOverlayDepthScale(
	TEXT("r.ANF.DebugOverlay.DepthScale"),
	GANFDebugOverlayDepthScale,
	TEXT("DEPTH_VIS: integer depth multiplier (1-99)"),
	ECVF_Default
);

inline void CreateBackendIfNeeded()
{
	const bool hasBackend = ANFBackendInterface::Get()->HasBackendWrapper();
	if (!hasBackend)
	{
		if (!ANFBackendInterface::Get()->CreateANFInstance())
		{
			UE_LOG(LogANFAPI, Error, TEXT("Failed to re-initialize backend interface after version change!"));
		}
	}
}

void RegisterANFCVarCallbacks()
{
	GEnableANF = CVarEnableANF.GetValueOnAnyThread();
	GANFUseJitterPattern = CVarANFUseJitterPattern.GetValueOnAnyThread();
	GANFFrameGenEnabled = CVarANFFrameGenEnabled.GetValueOnAnyThread();

	if (GEnableANF == 1 || GANFFrameGenEnabled == 1)
	{
		CreateBackendIfNeeded();
	}

	CVarEnableANF.AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* Var)
			{
				GEnableANF = Var->GetInt();
				if (GEnableANF == 1)
				{
					CreateBackendIfNeeded();
				}
			})
	);

	CVarANFUseJitterPattern.AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* Var)
			{
				GANFUseJitterPattern = Var->GetInt();
			})
	);

	CVarANFFrameGenEnabled.AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* Var)
			{
				GANFFrameGenEnabled = Var->GetInt();
				if (GANFFrameGenEnabled == 1)
				{
					CreateBackendIfNeeded();
				}
			})
	);
}

UANFSettings::UANFSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FName UANFSettings::GetContainerName() const
{
	static const FName ContainerName("Project");
	return ContainerName;
}

FName UANFSettings::GetCategoryName() const
{
	static const FName EditorCategoryName("Plugins");
	return EditorCategoryName;
}

FName UANFSettings::GetSectionName() const
{
	static const FName EditorSectionName("ANF");
	return EditorSectionName;
}

void UANFSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	if (IsTemplate())
	{
		ImportConsoleVariableValues();
	}
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void UANFSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
