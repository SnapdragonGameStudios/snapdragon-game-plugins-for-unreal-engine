//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFPreProcessor.h"
#include "ANFConfig.h"
#include "ANFRHI.h"

#include "HAL/Platform.h"
#include "SceneRendering.h"
#include "SceneTextureParameters.h"
#include "ScenePrivate.h"
#include "LegacyScreenPercentageDriver.h"
#include "PlanarReflectionSceneProxy.h"
#include "Serialization/MemoryImage.h"
#include "Serialization/MemoryLayout.h"
#include "FXSystem.h"
#if ENGINE_MINOR_VERSION > 1
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"

DEFINE_LOG_CATEGORY(LogANFPreProcessor);

FANFPreProcessor* FANFPreProcessor::s_instance = nullptr;
FCriticalSection FANFPreProcessor::s_instanceLock;

BEGIN_SHADER_PARAMETER_STRUCT(FCopyANFDepthparamns, )
	RDG_TEXTURE_ACCESS(InputDepth, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(OutputDepth, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

class ANFPreProcess : public FGlobalShader
{
public:
	static const uint32_t TILE_SIZE_X = 32;
	static const uint32_t TILE_SIZE_Y = 32;

	class FPreprocessMotion : SHADER_PERMUTATION_BOOL("ANF_PREPROCESS_MOTION");
	class FPreprocessColor : SHADER_PERMUTATION_BOOL("ANF_PREPROCESS_COLOR");
	class FPreprocessDepth : SHADER_PERMUTATION_BOOL("ANF_PREPROCESS_DEPTH");
	class FPreprocessFrameGen : SHADER_PERMUTATION_BOOL("ANF_PREPROCESS_FRAME_GEN");
	using FPermutationDomain = TShaderPermutationDomain < FPreprocessMotion, FPreprocessColor, FPreprocessDepth, FPreprocessFrameGen>;

	DECLARE_GLOBAL_SHADER(ANFPreProcess);
	SHADER_USE_PARAMETER_STRUCT(ANFPreProcess, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, InputDepth)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InputVelocity)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, InputColor)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputColorSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputVelocitySampler)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>, ANFMotionVector)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, ANFDepth)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, ANFColor)
		SHADER_PARAMETER(FUintVector2, iAnfRenderSize)
		SHADER_PARAMETER(FUintVector2, iAnfDisplaySize)
		SHADER_PARAMETER(FUintVector2, iUeRenderSize)
		SHADER_PARAMETER(FUintVector2, iUeDisplaySize)
		SHADER_PARAMETER(FVector2f, fMotionVectorJitterCancellation)
		SHADER_PARAMETER(int, projMatSign)
		SHADER_PARAMETER(FMatrix44f, clipToPrevClip)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsVulkanPlatform(Parameters.Platform);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("COMPUTE_SHADER"), 1);
		OutEnvironment.SetDefine(TEXT("TILE_SIZE_X"), TILE_SIZE_X);
		OutEnvironment.SetDefine(TEXT("TILE_SIZE_Y"), TILE_SIZE_Y);
	}
};
IMPLEMENT_GLOBAL_SHADER(ANFPreProcess, "/Plugin/ANF/Private/ANF_PreProcess.usf", "ANFPreProcess", SF_Compute);

FANFPreProcessor::FANFPreProcessor()
{
	for (uint32_t ii = 0; ii < ANFStoredImage::Count; ++ii)
	{
		m_storedImages[ii] = nullptr;
		m_storedImageFrameNumber[ii] = UINT32_MAX;
	}
}

FANFPreProcessor::~FANFPreProcessor()
{
}

FANFPreProcessor::ANFPreProcessorOutputs FANFPreProcessor::Execute(
	const FViewInfo& View,
	FRDGBuilder& GraphBuilder,
	FRDGTexture* inputColor,
	FRDGTexture* inputDepth,
	FRDGTexture* inputMotion,
	FIntVector2 renderSize,
	FIntVector2 displaySize,
	FVector4f jitter,
	uint32 frameNumber,
	bool frameGenPass)
{
	ANFPreProcessorOutputs outputs = {};

	FRHITexture* outputColor = frameGenPass ? CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::Color_FG, displaySize) :
		CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::Color_SR, renderSize);
	FRHITexture* outputDepth = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::Depth, renderSize);
	FRHITexture* outputDepth32 = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::Depth32, renderSize);
	FRDGTextureRef outputDepth32RDG = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(outputDepth32, TEXT("ANF.PreprocessesDepth32")));
	FRHITexture* outputMotion = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::Motion, renderSize);

	const bool hasUpcomingFrameGenPass = !frameGenPass && GANFFrameGenEnabled != 0;

#if ENGINE_MINOR_VERSION > 0
	const bool preprocessColor = frameGenPass ||
		inputColor->Desc.Format != outputColor->GetDesc().Format ||
		inputColor->Desc.Extent != outputColor->GetDesc().Extent;
#else
	const bool preprocessColor = frameGenPass ||
		inputColor->Desc.Format != outputColor->GetFormat() ||
		inputColor->Desc.Extent != FIntPoint(outputColor->GetSizeXYZ().X, outputColor->GetSizeXYZ().Y);
#endif

	const bool preprocessMotion = true;

#if ENGINE_MINOR_VERSION > 0
	const bool preprocessDepth = (hasUpcomingFrameGenPass) ||
		frameGenPass ||
		(inputDepth->Desc.Extent != outputDepth->GetDesc().Extent) ||
		(inputDepth->Desc.Format != outputDepth->GetDesc().Format);
#else
	const bool preprocessDepth = (hasUpcomingFrameGenPass) ||
		frameGenPass ||
		(inputDepth->Desc.Extent != FIntPoint(outputDepth->GetSizeXYZ().X, outputDepth->GetSizeXYZ().Y)) ||
		(inputDepth->Desc.Format != outputDepth->GetFormat());
#endif

	const bool useCachedMotion =
		frameGenPass &&
		m_storedImageFrameNumber[ANFStoredImage::Motion] == frameNumber;

	const bool useCachedDepth = 
		frameGenPass &&
		m_storedImageFrameNumber[ANFStoredImage::Depth] == frameNumber;

	if (!preprocessColor)
	{
		outputs.color = inputColor;
	}
	else
	{
		outputs.color = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(outputColor, TEXT("ANF.PreprocColor.RDG")));
	}

	if (!preprocessDepth && !useCachedDepth)
	{
		outputs.depth = inputDepth;
	}
	else
	{
		outputs.depth = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(outputDepth, TEXT("ANF.PreprocDepth.RDG")));
	}

	outputs.motion = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(outputMotion, TEXT("ANF.PreprocMotion.RDG")));

	FMatrix44f clipToPrevClip = FMatrix44f::Identity;
	const bool jitteredClipToPrevClip = (GANFJitterPrevClip != 0);

	const FViewMatrices& ViewMatrices = View.ViewMatrices;
	const FViewMatrices& PrevViewMatrices = View.PrevViewInfo.ViewMatrices;
	FVector DeltaTranslation = PrevViewMatrices.GetPreViewTranslation() - ViewMatrices.GetPreViewTranslation();
	FMatrix InvViewProj = ViewMatrices.ComputeInvProjectionNoAAMatrix() * (ViewMatrices.GetTranslatedViewMatrix().RemoveTranslation().GetTransposed());
	FMatrix PrevViewProj = FTranslationMatrix(DeltaTranslation) * PrevViewMatrices.GetTranslatedViewMatrix() * PrevViewMatrices.ComputeProjectionNoAAMatrix();
	if (jitteredClipToPrevClip)
	{
		FMatrix curInvProjMat = ViewMatrices.GetInvProjectionMatrix();
		FMatrix prevProjMat = PrevViewMatrices.GetProjectionMatrix();

		InvViewProj = curInvProjMat * (ViewMatrices.GetTranslatedViewMatrix().RemoveTranslation().GetTransposed());
		PrevViewProj = FTranslationMatrix(DeltaTranslation) * PrevViewMatrices.GetTranslatedViewMatrix() * prevProjMat;
	}

	clipToPrevClip = FMatrix44f(InvViewProj * PrevViewProj);

	outputs.clipToPrevClip = clipToPrevClip;

	const TCHAR* perfMonPassName = frameGenPass ? TEXT("ANF FrameGen PreProcess") : TEXT("ANF SuperResolution PreProcess");
	const bool doTimings = GANFTimePasses != 0;
	if (doTimings)
	{
		ANFRHI::GetRHI()->AddStartTimedPass(GraphBuilder, perfMonPassName);
	}

	if (preprocessDepth && !useCachedDepth)
	{
		ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, inputDepth, outputs.depth);
	}

	const bool blitColor = preprocessColor;
	const bool bNeedColorShader = preprocessColor && !blitColor;
	if (blitColor)
	{
		ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, inputColor, outputs.color);
	}

	const bool saveDepth32 = GANFDumpFrames != 0;
	const FIntPoint dispatchSize = (frameGenPass && bNeedColorShader) ? FIntPoint(displaySize.X, displaySize.Y) : FIntPoint(renderSize.X, renderSize.Y);

	if ((preprocessMotion && !useCachedMotion) || bNeedColorShader || (saveDepth32 && !useCachedDepth))
	{
		ANFPreProcess::FParameters* prepassParams = GraphBuilder.AllocParameters<ANFPreProcess::FParameters>();
		prepassParams->InputDepth = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(inputDepth));
		prepassParams->InputVelocity = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(inputMotion));
		prepassParams->InputColor = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(inputColor));
		prepassParams->InputColorSampler = TStaticSamplerState<SF_Point, AM_Border, AM_Border, AM_Border>::GetRHI();
		prepassParams->InputVelocitySampler = TStaticSamplerState<SF_Point, AM_Border, AM_Border, AM_Border>::GetRHI();
#if ENGINE_MINOR_VERSION <= 2
		prepassParams->ANFMotionVector =  (preprocessMotion && !useCachedMotion) ? GraphBuilder.CreateUAV(outputs.motion, ERDGUnorderedAccessViewFlags::None) : nullptr;
		prepassParams->ANFDepth =  (saveDepth32 && !useCachedDepth) ? GraphBuilder.CreateUAV(outputDepth32RDG, ERDGUnorderedAccessViewFlags::None) : nullptr;
		prepassParams->ANFColor = bNeedColorShader ? GraphBuilder.CreateUAV(outputs.color, ERDGUnorderedAccessViewFlags::None) : nullptr;
#else
		prepassParams->ANFMotionVector =  (preprocessMotion && !useCachedMotion) ? GraphBuilder.CreateUAV(outputs.motion, ERDGUnorderedAccessViewFlags::None, outputs.motion->Desc.Format) : nullptr;
		prepassParams->ANFDepth =  (saveDepth32 && !useCachedDepth) ? GraphBuilder.CreateUAV(outputDepth32RDG, ERDGUnorderedAccessViewFlags::None, PF_R32_FLOAT) : nullptr;
		prepassParams->ANFColor = bNeedColorShader ? GraphBuilder.CreateUAV(outputs.color, ERDGUnorderedAccessViewFlags::None, outputs.color->Desc.Format) : nullptr;
#endif
		prepassParams->iAnfRenderSize = FUintVector2(renderSize.X, renderSize.Y);
		prepassParams->iAnfDisplaySize = FUintVector2(displaySize.X, displaySize.Y);
		prepassParams->iUeRenderSize = FUintVector2(inputDepth->Desc.Extent.X, inputDepth->Desc.Extent.Y);
		prepassParams->iUeDisplaySize = FUintVector2(inputColor->Desc.Extent.X, inputColor->Desc.Extent.Y);
		prepassParams->fMotionVectorJitterCancellation[0] = 0.f;
		prepassParams->fMotionVectorJitterCancellation[1] = 0.f;
		if (jitteredClipToPrevClip)
		{
			prepassParams->fMotionVectorJitterCancellation[0] = -2.0f * (jitter.X - jitter.Z) / (float)renderSize.X;
			prepassParams->fMotionVectorJitterCancellation[1] = -2.0f * (jitter.Y - jitter.W) / (float)renderSize.Y;
		}


		float projMatVal = View.ViewMatrices.GetProjectionMatrix().M[2][3];
		int32_t projMatSign = 0;
		if (projMatVal < 0.f)
		{
			projMatSign = -1;
		}
		else
		{
			projMatSign = 1;
		}

		prepassParams->projMatSign = projMatSign;
		prepassParams->clipToPrevClip = clipToPrevClip;

		ANFPreProcess::FPermutationDomain prepassDomain;
		prepassDomain.Set<ANFPreProcess::FPreprocessMotion>((preprocessMotion && !useCachedMotion) ? 1 : 0);
		prepassDomain.Set<ANFPreProcess::FPreprocessColor>(bNeedColorShader ? 1 : 0);
		prepassDomain.Set<ANFPreProcess::FPreprocessDepth>((saveDepth32 && !useCachedDepth) ? 1 : 0);
		prepassDomain.Set<ANFPreProcess::FPreprocessFrameGen>(frameGenPass ? 1 : 0);

		check((preprocessMotion && !useCachedMotion) || preprocessColor || (saveDepth32 && !useCachedDepth));

		TShaderMapRef<ANFPreProcess> PreProcessShaderMotion(View.ShaderMap, prepassDomain);
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("ANF PrePass"),
			ERDGPassFlags::Compute,
			PreProcessShaderMotion,
			prepassParams,
			FComputeShaderUtils::GetGroupCount(dispatchSize,
				FIntPoint(ANFPreProcess::TILE_SIZE_X * 2, ANFPreProcess::TILE_SIZE_Y * 2)));
	}

	const bool hasLowResInputs = (renderSize.X < displaySize.X) || (renderSize.Y < displaySize.Y);

	if (frameGenPass && GANFFrameGenSupportsLowResInputs == 0 && hasLowResInputs)
	{
		// Copy low res motion to full res
		{
			FRHITexture* fullResOutputMotion = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::FullResMotion, displaySize);
			FRDGTextureRef fullResMotionRDG = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(fullResOutputMotion, TEXT("ANF.PreprocMotion.FullRes.RDG")));
			ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, outputs.motion, fullResMotionRDG, true);

			outputs.motion = fullResMotionRDG;
		}

		// Copy low res depth to full res
		{
			FRHITexture* fullResOutputDepth = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::FullResDepth, displaySize);
			FRDGTextureRef fullResDepthRDG = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(fullResOutputDepth, TEXT("ANF.PreprocDepth.FullRes.RDG")));
			ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, outputs.depth, fullResDepthRDG, true);

			outputs.depth = fullResDepthRDG;

			FRHITexture* fullResOutputDepth32 = CheckTexture(GraphBuilder, frameNumber, ANFStoredImage::FullResDepth32, displaySize);
			if (saveDepth32)
			{
				FRDGTextureRef fullResDepth32RDG = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(fullResOutputDepth32, TEXT("ANF.PreprocessesDepth32.FullRes")));
				ANFRHI::GetRHI()->AddBlitTexturePass(GraphBuilder, outputDepth32RDG, fullResDepth32RDG, true);
			}
		}
	}

	if (doTimings)
	{
		ANFRHI::GetRHI()->AddEndTimedPass(GraphBuilder, perfMonPassName);
	}

	if (!hasUpcomingFrameGenPass)
	{
		// reset stored frame numbers since frame 0 can repeat
		for (uint32_t ii = 0; ii < ANFStoredImage::Count; ++ii)
		{
			m_storedImageFrameNumber[ii] = UINT32_MAX;
		}
	}
	else
	{
		if (preprocessDepth)
		{
			m_storedImageFrameNumber[ANFStoredImage::Depth] = frameNumber;
		}
		if (saveDepth32)
		{
			m_storedImageFrameNumber[ANFStoredImage::Depth32] = frameNumber;
		}
		if (preprocessMotion)
		{
			m_storedImageFrameNumber[ANFStoredImage::Motion] = frameNumber;
		}
		if (preprocessColor)
		{
			m_storedImageFrameNumber[frameGenPass ? ANFStoredImage::Color_FG : ANFStoredImage::Color_SR] = frameNumber;
		}
	}

	return outputs;
}

FRHITexture* FANFPreProcessor::CheckTexture(FRDGBuilder& GraphBuilder, uint32 frame, ANFStoredImage eid, FIntVector2 size)
{
	check(eid < ANFStoredImage::Count);

	bool bRecreateImage = (m_storedImages[eid] == nullptr);
	if (!bRecreateImage)
	{
		bRecreateImage = ((m_storedImagesDimms[eid].X != size.X) || (m_storedImagesDimms[eid].Y != size.Y));
	}

	if (bRecreateImage)
	{
		EPixelFormat frmt = EPixelFormat::PF_Unknown;
		FString textureName;
		bool isDepth = false;

		ETextureCreateFlags flags = ETextureCreateFlags::ShaderResource;
		switch (eid)
		{
			case ANFStoredImage::Color_SR:
				frmt = EPixelFormat::PF_FloatR11G11B10;
				textureName = TEXT("ANF.SR.InputColor");
				flags |= ANFRHI::GetRHI()->GetExtraFlags_ColorInput(false);
				break;
			case ANFStoredImage::Color_FG:
				frmt = EPixelFormat::PF_R8G8B8A8;
				textureName = TEXT("ANF.FG.InputColor");
				flags |= ANFRHI::GetRHI()->GetExtraFlags_ColorInput(true);
				break;
			case ANFStoredImage::Motion:
			case ANFStoredImage::FullResMotion:
				frmt = EPixelFormat::PF_G16R16F;
				textureName = TEXT("ANF.MotionVector");
				flags |= ANFRHI::GetRHI()->GetExtraFlags_MotionInput();
				break;
			case ANFStoredImage::Depth:
			case ANFStoredImage::FullResDepth:
				frmt = EPixelFormat::PF_DepthStencil;
				textureName = TEXT("ANF.DepthBuffer");
				isDepth = true;
				flags |= ANFRHI::GetRHI()->GetExtraFlags_DepthInput();
				break;
			case ANFStoredImage::Depth32:
			case ANFStoredImage::FullResDepth32:
				frmt = EPixelFormat::PF_R32_FLOAT;
				textureName = TEXT("ANF.DepthBufferF32");
				break;
			default: UE_LOG(LogANFPreProcessor, Fatal, TEXT("Invalid Stored Image ID %d"), eid);  check(false); break;
		}

		if (isDepth)
		{
			flags |= ETextureCreateFlags::DepthStencilTargetable;
		}
		else
		{
			flags |= (ETextureCreateFlags::UAV | ETextureCreateFlags::RenderTargetable);
		}
#if ENGINE_MINOR_VERSION > 0
		FRHITextureCreateDesc desc =
			FRHITextureCreateDesc::Create2D(*textureName, size.X, size.Y, frmt)
			.SetFlags(flags);
		
		m_storedImages[eid] = RHICreateTexture(desc);
#else
		FRHIResourceCreateInfo CreateInfo = FRHIResourceCreateInfo(*textureName);

		m_storedImages[eid] = RHICreateTexture2D(size.X, size.Y
			, frmt, 1, 1, flags, CreateInfo);

		check(m_storedImages[eid] != nullptr);
#endif
		check(m_storedImages[eid] != nullptr);
		m_storedImageFrameNumber[eid] = UINT32_MAX;

		m_storedImagesDimms[eid] = size;
	}
	check(m_storedImages[eid] != nullptr);
	return m_storedImages[eid];
}
