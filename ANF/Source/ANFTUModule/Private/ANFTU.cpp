//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFTU.h"
#include "ANFTUModule.h"
#include "ANFTUHistory.h"

#include "ANFPreProcessor.h"
#include "ANFBackend.h"
#include "ANFRHI.h"

#include "HAL/Platform.h"
#include "SceneTextureParameters.h"
#include "TranslucentRendering.h"
#include "ScenePrivate.h"
#include "ANFConfig.h"
#include "ANFFrameDumper.h"
#include "LegacyScreenPercentageDriver.h"
#include "PlanarReflectionSceneProxy.h"
#include "Serialization/MemoryImage.h"
#include "Serialization/MemoryLayout.h"
#include "FXSystem.h"
#include "PostProcess/SceneRenderTargets.h"
#if ENGINE_MINOR_VERSION > 1
#include "DataDrivenShaderPlatformInfo.h"
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 8
#include "SceneViewState.h"
#endif
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"

DECLARE_GPU_STAT(ANFPass);
DECLARE_GPU_STAT_NAMED(ANFDispatch, TEXT("ANF Dispatch"));

BEGIN_SHADER_PARAMETER_STRUCT(FANFUpscaleParameters, )
	RDG_TEXTURE_ACCESS(InputDepth, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(InputMotion, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(InputColor, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(OutputColor, ERHIAccess::CopyDest)
	SHADER_PARAMETER(int, frameNumber)
	SHADER_PARAMETER(int, reset)
	SHADER_PARAMETER(FVector2f, fJitter)
END_SHADER_PARAMETER_STRUCT()

/// TU implementation
FANFTU::FANFTU()
{
}

FANFTU::~FANFTU()
{
	Cleanup();
}

const TCHAR* FANFTU::GetDebugName() const
{
	return TEXT("FANFTU");
}

void FANFTU::Releasestate(ANFstateRef state)
{
}

void FANFTU::Cleanup() const
{
}

ANFPassOutput FANFTU::AddPasses(
	FRDGBuilder& GraphBuilder,
	const ANFView& SceneView,
	const ANFPassInput& PassInputs) const
{
#if ENGINE_MINOR_VERSION > 2
	FRDGTextureRef SceneColor = PassInputs.SceneColor.Texture;
	FRDGTextureRef SceneDepth = PassInputs.SceneDepth.Texture;
	FRDGTextureRef SceneVelocity = PassInputs.SceneVelocity.Texture;
#else
	FRDGTextureRef SceneColor = PassInputs.SceneColorTexture;
	FRDGTextureRef SceneDepth = PassInputs.SceneDepthTexture;
	FRDGTextureRef SceneVelocity = PassInputs.SceneVelocityTexture;
#endif

	///output extent 
	const FViewInfo& View = (FViewInfo&)(SceneView);
	FIntPoint InputExtents = View.ViewRect.Size();
	FIntPoint OutputExtents = View.GetSecondaryViewRectSize();
	OutputExtents = FIntPoint(FMath::Max(InputExtents.X, OutputExtents.X), FMath::Max(InputExtents.Y, OutputExtents.Y));

	ANFBackendInterface* backendInterface = ANFBackendInterface::Get();

	FIntVector2 originalInputSize = { InputExtents.X, InputExtents.Y };
	FIntVector2 originalOutputSize = { OutputExtents.X, OutputExtents.Y };
	FIntVector2 forcedInputSize = { GANFInWidth, GANFInHeight };

	int32_t srcWidth = originalInputSize.X;
	int32_t srcHeight = originalInputSize.Y;

	int32_t dstWidth = originalOutputSize.X;
	int32_t dstHeight = originalOutputSize.Y;

    const float scaleFactor = GANFUpscaleFactor;

	if (forcedInputSize.X > 0 && forcedInputSize.Y > 0)
	{
		srcWidth = forcedInputSize.X;
		srcHeight = forcedInputSize.Y;

	}
    
	{
        // AIF FSDK Only suports 4x upscaling (2x width, 2x height)
        // UE5 will often not have this scaling properly enabled the first frame or so
        // Make sure the ANF output buffer matches the expected output here
		int32_t expectedDstWidth = srcWidth * scaleFactor;
		int32_t expectedDstHeight = srcHeight * scaleFactor;
        
        dstWidth = expectedDstWidth;
        dstHeight = expectedDstHeight;
	}

	FIntVector2 finalInputSize = { srcWidth , srcHeight };
	FIntVector2 finalOutputSize = { dstWidth , dstHeight };

	bool returnPassthroughOutputs = false;
	if (dstWidth <= GANFMinWidth || dstHeight <= GANFMinHeight)
	{
		UE_LOG(LogANFTU, Warning, TEXT("ANF SR (%dx%d -> %dx%d) is less than minimum output dimensions (%dx%d)"),
			srcWidth, srcHeight, dstWidth, dstHeight, GANFMinWidth, GANFMinHeight);
		returnPassthroughOutputs = true;
	}
	
	if (!returnPassthroughOutputs  && backendInterface->SrNeedsReInit(finalInputSize, finalOutputSize))
	{
		if (!backendInterface->InitializeANF_SR(finalInputSize, finalOutputSize))
		{
			UE_LOG(LogANFTU, Error, TEXT("Failed to initialize ANF SR (%dx%d -> %dx%d); passing through input unchanged."),
				srcWidth, srcHeight, dstWidth, dstHeight);
			returnPassthroughOutputs = true;
		}
	}

	if (returnPassthroughOutputs)
	{
		IANFTemporalUpscaler::FOutputs PassthroughOutputs;
		PassthroughOutputs.FullRes.Texture = SceneColor;
#if ENGINE_MINOR_VERSION > 2
		PassthroughOutputs.FullRes.ViewRect = PassInputs.SceneColor.ViewRect;
		PassthroughOutputs.NewHistory = PassInputs.PrevHistory;
#else
		PassthroughOutputs.FullRes.ViewRect = FIntRect(0, 0, srcWidth, srcHeight);
#endif
		return PassthroughOutputs;
	}

#if ENGINE_MINOR_VERSION <= 2
	if (View.PrimaryScreenPercentageMethod != EPrimaryScreenPercentageMethod::TemporalUpscale)
	{
		IANFTemporalUpscaler::FOutputs PassthroughOutputs;
		PassthroughOutputs.FullRes.Texture = SceneColor;
		PassthroughOutputs.FullRes.ViewRect = FIntRect(0, 0, srcWidth, srcHeight);
		return PassthroughOutputs;
	}
#else
	check((View.PrimaryScreenPercentageMethod == EPrimaryScreenPercentageMethod::TemporalUpscale));
#endif
	{
		IANFTemporalUpscaler::FOutputs Outputs;

		RDG_GPU_STAT_SCOPE(GraphBuilder, ANFPass);

#if ENGINE_MAJOR_VERSION <= 5 && ENGINE_MINOR_VERSION < 5	// before UE5.5
		static const auto CVarPostPropagateAlpha = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.PostProcessing.PropagateAlpha"));
		const bool bSupportsAlpha = (CVarPostPropagateAlpha && CVarPostPropagateAlpha->GetValueOnRenderThread() != 0);
#else
		static const auto CVarPostPropagateAlpha = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessing.PropagateAlpha"));
		const bool bSupportsAlpha = (CVarPostPropagateAlpha && CVarPostPropagateAlpha->GetBool());
#endif
		const bool bPreferHalfFloatOutputs = GANFPreferHalfFloatOutput != 0;

		FIntPoint outputDimensions = FIntPoint(dstWidth, dstHeight);
		FIntPoint inputDimensions = FIntPoint(srcWidth, srcHeight);

		FRDGTextureDesc OutputDesc = FRDGTextureDesc::Create2D(
			outputDimensions,
			bPreferHalfFloatOutputs ? PF_FloatRGBA : PF_FloatR11G11B10,
			FClearValueBinding::None,
			TexCreate_ShaderResource | TexCreate_RenderTargetable | TexCreate_UAV | ANFRHI::GetRHI()->GetExtraFlags_ColorOutput(false));

		FRDGTextureRef ColorOutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("ANF.SR.Output"));

		FRDGTextureRef InputMotionVectorTexture = nullptr;
		FRDGTextureRef InputColorTexture = nullptr;
		FRDGTextureRef InputDepthTexture = nullptr;

		float prevJitterOffset[2] = {};
		backendInterface->GetPreviousJitterOffset(prevJitterOffset);

		ANFBackendInterface::PerFrameData perFrameData = {};
        perFrameData.frameNumber = backendInterface->IsFirstFrame() ? 0 : (backendInterface->GetPreviousFrameNumber() + 1);
        perFrameData.jitterId = View.TemporalJitterIndex;
        perFrameData.jitterOffset[0] = View.TemporalJitterPixels.X;
		perFrameData.jitterOffset[1] = View.TemporalJitterPixels.Y;
		perFrameData.prevJitterOffset[0] = prevJitterOffset[0];
		perFrameData.prevJitterOffset[1] = prevJitterOffset[1];
        perFrameData.preExposure = (View.PreExposure != 0) ? View.PreExposure : 1.0f;
		perFrameData.colorGamma[0] = View.FinalPostProcessSettings.ColorGamma.X;
		perFrameData.colorGamma[1] = View.FinalPostProcessSettings.ColorGamma.Y;
		perFrameData.colorGamma[2] = View.FinalPostProcessSettings.ColorGamma.Z;
		perFrameData.colorGamma[3] = View.FinalPostProcessSettings.ColorGamma.W;

		backendInterface->SetPerFrameData(perFrameData);

		FMatrix44f clipToPrevClip = FMatrix44f::Identity;

		auto viewShaderMap = View.ShaderMap;

		FANFPreProcessor::ANFPreProcessorOutputs preprocOutputs = FANFPreProcessor::Get()->Execute(
			View,
			GraphBuilder,
			SceneColor,
			SceneDepth,
			SceneVelocity,
			FIntVector2(inputDimensions.X, inputDimensions.Y),
			FIntVector2(outputDimensions.X, outputDimensions.Y),
			FVector4f(perFrameData.jitterOffset[0], perFrameData.jitterOffset[1], prevJitterOffset[0], prevJitterOffset[1]),
			GFrameNumber,
			false);

		clipToPrevClip = preprocOutputs.clipToPrevClip;


		FANFUpscaleParameters* upscaleParameters = GraphBuilder.AllocParameters< FANFUpscaleParameters>();
		upscaleParameters->InputDepth = preprocOutputs.depth;
		upscaleParameters->InputMotion = preprocOutputs.motion;
		upscaleParameters->InputColor = preprocOutputs.color;
		upscaleParameters->OutputColor = ColorOutputTexture;
		upscaleParameters->frameNumber = perFrameData.frameNumber;
		upscaleParameters->fJitter.X = View.TemporalJitterPixels.X;
		upscaleParameters->fJitter.Y = View.TemporalJitterPixels.Y;
		upscaleParameters->reset = View.bCameraCut ? 1 : 0;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("ANF %dx%d -> %dx%d",
				srcWidth, srcHeight,
				dstWidth, dstHeight),
			upscaleParameters,
			ERDGPassFlags::Copy | ERDGPassFlags::SkipRenderPass | ERDGPassFlags::Raster | ERDGPassFlags::NeverCull,
			[upscaleParameters, viewShaderMap, backendInterface](FRHICommandListImmediate& RHICmdList)
			{
				// Upscale
				if (backendInterface->HasBackendWrapper())
				{
					ANFBackendInterface::ANFExecutionData srData = {};
					srData.frameNumber = upscaleParameters->frameNumber;
					srData.reset = upscaleParameters->reset == 1 ? true : false;
					srData.jitterOffset = upscaleParameters->fJitter;
					srData.inputColor = upscaleParameters->InputColor->GetRHI();
					srData.inputDepth = upscaleParameters->InputDepth->GetRHI();
					srData.inputMotion = upscaleParameters->InputMotion->GetRHI();
					srData.outputColor = upscaleParameters->OutputColor->GetRHI();

					backendInterface->ExecuteANF_SR(RHICmdList, srData);

				}
			}
		);

		// Dump Frames
		if (ShouldDumpANFFrames(GFrameNumber))
		{
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ANF - Dump Data"),
				upscaleParameters,
				ERDGPassFlags::Readback,
				[upscaleParameters, perFrameData, clipToPrevClip](FRHICommandListImmediate& RHICmdList) {
					FANFFrameDumper::Execute(
						RHICmdList,
						GFrameNumber,
						upscaleParameters->InputColor->GetRHI(),
						upscaleParameters->InputDepth->GetRHI(),
						upscaleParameters->InputMotion->GetRHI(),
						upscaleParameters->OutputColor->GetRHI(),
						FVector4f(perFrameData.jitterOffset[0], perFrameData.jitterOffset[1], perFrameData.prevJitterOffset[0], perFrameData.prevJitterOffset[1]),
						clipToPrevClip,
						TEXT("SR"));

				});
		}

		Outputs.FullRes.Texture = ColorOutputTexture;
		if (finalOutputSize != originalOutputSize)
		{
			FRDGTextureDesc FinalOutputDesc = FRDGTextureDesc::Create2D(
				FIntPoint(originalOutputSize.X, originalOutputSize.Y),
				(bSupportsAlpha || bPreferHalfFloatOutputs) ? PF_FloatRGBA : PF_FloatR11G11B10,
				FClearValueBinding::None,
				TexCreate_ShaderResource | TexCreate_RenderTargetable | TexCreate_UAV);

			FRDGTextureRef FinalColorOutputTexture = GraphBuilder.CreateTexture(FinalOutputDesc, TEXT("ANF.FinalOutput"));

			ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, ColorOutputTexture, FinalColorOutputTexture, true);
			Outputs.FullRes.Texture = FinalColorOutputTexture;
		}
		check(Outputs.FullRes.Texture != nullptr);
		Outputs.FullRes.ViewRect = FIntRect(FIntPoint::ZeroValue, View.GetSecondaryViewRectSize());

		const bool bWritePrevViewInfo = !View.bStatePrevViewInfoIsReadOnly && View.ViewState;
		if (bWritePrevViewInfo)
		{
			FIntPoint HistorySize;
			FIntPoint HistoryExtentsQuantized;
			const float HistoryFactor = 1.f;
			FIntPoint HistoryExtents = FIntPoint(FMath::CeilToInt(OutputExtents.X * HistoryFactor),
				FMath::CeilToInt(OutputExtents.Y * HistoryFactor));
			HistorySize = FIntPoint(FMath::CeilToInt(OutputExtents.X * HistoryFactor),
				FMath::CeilToInt(OutputExtents.Y * HistoryFactor));

			FIntRect HistoryRect = FIntRect(FIntPoint(0, 0), HistorySize);
			// Quantize the buffers to match UE behavior
			QuantizeSceneBufferSize(HistoryRect.Max, HistoryExtentsQuantized);

			// Releases the existing history texture inside the wrapper object, this doesn't release NewHistory itself
			View.ViewState->PrevFrameViewInfo.TemporalAAHistory.SafeRelease();

			View.ViewState->PrevFrameViewInfo.TemporalAAHistory.ViewportRect = FIntRect(FIntPoint(0, 0), HistoryRect.Size());
			View.ViewState->PrevFrameViewInfo.TemporalAAHistory.ReferenceBufferSize = HistoryExtents;
#if ENGINE_MINOR_VERSION > 2
			Outputs.NewHistory = new FANFTUHistory(ANFstateRef(), const_cast<FANFTU*>(this));
#endif
		}
		else
		{
#if ENGINE_MINOR_VERSION > 2
			Outputs.NewHistory = PassInputs.PrevHistory;
#endif
		}

		Cleanup(); 
		return Outputs;
	}
}
#if ENGINE_MINOR_VERSION > 0
IANFTemporalUpscaler* FANFTU::Fork_GameThread(const class FSceneViewFamily& InViewFamily) const
{
	static const FANFTUModule& ANFModule = FModuleManager::GetModuleChecked<FANFTUModule>(TEXT("ANFTUModule"));
	return new FANFTUFork(ANFModule.GetTU());
}
#endif

float FANFTU::GetMinUpsampleResolutionFraction() const
{
	return 1.f / GANFUpscaleFactor;
}

float FANFTU::GetMaxUpsampleResolutionFraction() const
{
	return 1.f / GANFUpscaleFactor;
}

///release retained resources
void FANFTU::EndofFrame()
{
}

FVector2D FANFTU::GetANFInputResolution()
{
	FVector2D Result = FVector2D(GANFInWidth, GANFInHeight);
	return Result;

}

FVector2D FANFTU::GetANFOutputResolution()
{
	float fScale = GANFUpscaleFactor;
	return FVector2D(GANFInWidth * fScale, GANFInHeight * fScale);
}

float FANFTU::GetANFScreenPercentage()
{
	return (1.f / GANFUpscaleFactor) * 100.f;
}

bool FANFTU::IsANFSRSupported()
{
	return ANFBackendInterface::Get()->IsSrTechniqueSupported();
}

FANFTUFork::FANFTUFork(IANFTemporalUpscaler* TU)
	: TemporalUpscaler(TU)
{
}

FANFTUFork::~FANFTUFork()
{
}

const TCHAR* FANFTUFork::GetDebugName() const
{
	return TemporalUpscaler->GetDebugName();
}

IANFTemporalUpscaler::FOutputs FANFTUFork::AddPasses(
	FRDGBuilder& GraphBuilder,
	const ANFView& View,
	const ANFPassInput& PassInputs) const
{
	return TemporalUpscaler->AddPasses(GraphBuilder, View, PassInputs);
}

float FANFTUFork::GetMinUpsampleResolutionFraction() const
{
	return TemporalUpscaler->GetMinUpsampleResolutionFraction();
}

float FANFTUFork::GetMaxUpsampleResolutionFraction() const
{
	return TemporalUpscaler->GetMaxUpsampleResolutionFraction();
}

#if ENGINE_MINOR_VERSION > 0
IANFTemporalUpscaler* FANFTUFork::Fork_GameThread(const class FSceneViewFamily& InViewFamily) const
{
	return new FANFTUFork(TemporalUpscaler);
}
#endif

FANFExecManager::FANFExecManager()
{
	m_currentState = GEnableANF != 0;
}

#if ENGINE_MINOR_VERSION > 2
bool FANFExecManager::Exec_Runtime(UWorld* Inworld, const TCHAR* Cmd, FOutputDevice& Ar)
#else
bool FANFExecManager::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
#endif
{
	if (FParse::Command(&Cmd, TEXT("anf.sr.enable")))
	{
		auto CVarEnable = CVarEnableANF.AsVariable();
		auto CVarJitterPattern = CVarANFUseJitterPattern.AsVariable();
		if (CVarEnable)
		{
			CVarEnable->Set(1, EConsoleVariableFlags::ECVF_SetByCode);
		}
		if (CVarJitterPattern)
		{
			CVarJitterPattern->Set(1, EConsoleVariableFlags::ECVF_SetByCode);
		}
		m_currentState = true;
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("anf.sr.disable")))
	{
		auto CVarEnable = CVarEnableANF.AsVariable();
		auto CVarJitterPattern = CVarANFUseJitterPattern.AsVariable();
		if (CVarEnable)
		{
			CVarEnable->Set(0, EConsoleVariableFlags::ECVF_SetByCode);
		}
		if (CVarJitterPattern)
		{
			CVarJitterPattern->Set(0, EConsoleVariableFlags::ECVF_SetByCode);
		}
		m_currentState = false;
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("anf.frc.enable")))
	{
		auto CVarEnable = CVarANFFrameGenEnabled.AsVariable();
		auto CVarJitterPattern = CVarANFUseJitterPattern.AsVariable();
		if (CVarEnable)
		{
			CVarEnable->Set(1, EConsoleVariableFlags::ECVF_SetByCode);
		}
		if (CVarJitterPattern)
		{
			CVarJitterPattern->Set(1, EConsoleVariableFlags::ECVF_SetByCode);
		}
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("anf.frc.disable")))
	{
		auto CVarEnable = CVarANFFrameGenEnabled.AsVariable();
		auto CVarJitterPattern = CVarANFUseJitterPattern.AsVariable();
		if (CVarEnable)
		{
			CVarEnable->Set(0, EConsoleVariableFlags::ECVF_SetByCode);
		}
		if (CVarJitterPattern)
		{
			CVarJitterPattern->Set(0, EConsoleVariableFlags::ECVF_SetByCode);
		}
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("anf.dumpframes.start")))
	{
		GANFDumpFrames = 1;
		GANFDumpFramesFirstFrame = 0;
		GANFDumpFramesNumFrames = 0;

		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("anf.dumpframes.stop")))
	{
		GANFDumpFrames = 0;
		GANFDumpFramesFirstFrame = 0;
		GANFDumpFramesNumFrames = 0;

		return true;
	}
	return false;
}

FANFExecManager FANFExecManager::s_instance;
