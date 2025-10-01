//============================================================================================================
//
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "QcomShadowDenoiserImpl.h"
#include "QcomShadowDenoiserPluginSettings.h"
#include "CoreMinimal.h"
#include "String.h"
#include <algorithm>
#include <array>
#include <cassert>

#include "StaticBoundShaderState.h"
#include "SceneUtils.h"
#include "PostProcess/SceneRenderTargets.h"
#include "SceneRenderTargetParameters.h"
#include "ScenePrivate.h"
#include "ClearQuad.h"
#include "PipelineStateCache.h"
#include "SceneTextureParameters.h"
#include "Lumen/Lumen.h"

DECLARE_GPU_STAT(QcomShadowsDenoiser)

static const bool bForceDisableReproject = false;


//
// Shader passes
//
BEGIN_SHADER_PARAMETER_STRUCT(FQcomShadowDenoiserSharedParameters, )
	SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureParameters, SceneTextures)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
	SHADER_PARAMETER(int32, bCameraCut)
END_SHADER_PARAMETER_STRUCT();


class FQcomShadowDenoiserBitMaskCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserBitMaskCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserBitMaskCS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
		OutEnvironment.CompilerFlags.Add(CFLAG_WarningsAsErrors);
		OutEnvironment.CompilerFlags.Add(CFLAG_WaveOperations);
		OutEnvironment.CompilerFlags.Add(CFLAG_InlineRayTracing);	// Force SM6
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputRtShadowTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputShadowBitBuffer)
		SHADER_PARAMETER(FVector4f, ThreadIdToBufferUV)
		SHADER_PARAMETER(FUintVector2, BlockMaxSize)
	END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserClassificationCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserClassificationCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserClassificationCS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
		OutEnvironment.CompilerFlags.Add(CFLAG_WaveOperations);
		OutEnvironment.CompilerFlags.Add(CFLAG_InlineRayTracing);	// Force SM6
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputShadowBitBuffer)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputHistoryClassificationBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputClassificationBuffer)
		SHADER_PARAMETER(FVector4f, ThreadIdToBufferUV)
		SHADER_PARAMETER(FUintVector2, BlockMaxSize)
	END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserBlockMinMaxCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserBlockMinMaxCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserBlockMinMaxCS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
		OutEnvironment.CompilerFlags.Add(CFLAG_WaveOperations);
		OutEnvironment.CompilerFlags.Add(CFLAG_InlineRayTracing);	// Force SM6
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputRtShadowTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputMinMaxBuffer)
		SHADER_PARAMETER(FVector4f, ThreadIdToBufferUV)
		SHADER_PARAMETER(FUintVector2, BlockMaxSize)
	END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserFixedFilterHorizCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserFixedFilterHorizCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserFixedFilterHorizCS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputShadowBitBuffer)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTileClassificationTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, InputFixedFilterLookup)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, Output)
		END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserFixedFilterVertCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserFixedFilterVertCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserFixedFilterVertCS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputHorizontallyFiltered)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTileClassificationTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, Output)
	END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserReprojectCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserReprojectCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserReprojectCS, FGlobalShader);

	class FTemporalRejectMode : SHADER_PERMUTATION_INT("PERMUTATION_TEMPORAL_REJECTMODE", 4);
	using FPermutationDomain = TShaderPermutationDomain<FTemporalRejectMode>;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		//OutEnvironment.CompilerFlags.Add(CFLAG_Wave32);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
		OutEnvironment.CompilerFlags.Add(CFLAG_WaveOperations);
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryDepthTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputRtShadowTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryShadowTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryMomentTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryDistanceTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTileClassificationTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTileMinMaxTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputFixedFilterTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputShadowBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputMomentBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputDistanceBuffer)
		SHADER_PARAMETER(FVector4f, ThreadIdToBufferUV)
		SHADER_PARAMETER(FVector4f, HistoryBufferUVMinMax)
	END_SHADER_PARAMETER_STRUCT()
};

class FQcomShadowDenoiserFilterCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQcomShadowDenoiserFilterCS);
	SHADER_USE_PARAMETER_STRUCT(FQcomShadowDenoiserFilterCS, FGlobalShader);

	class FATrousStepDim : SHADER_PERMUTATION_INT("ATROUS_STEP", 3);

	using FPermutationDomain = TShaderPermutationDomain<FATrousStepDim>;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || Parameters.Platform == SP_VULKAN_ES3_1_ANDROID;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// DXC crash workaround on some platforms
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceOptimization);
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);		// for 16bit types
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FQcomShadowDenoiserSharedParameters, SharedParameters)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputShadowBuffer)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTileClassificationTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, Output)
	END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserBitMaskCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserBitMask.usf", "BitMaskCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserClassificationCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserBlock.usf", "BlockCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserBlockMinMaxCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserBlockMinMax.usf", "BlockMinMaxCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserFixedFilterHorizCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserFixedFilterHoriz.usf", "FilterCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserFixedFilterVertCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserFixedFilterVert.usf", "FilterCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserReprojectCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserReproject.usf", "ReprojectCS", SF_Compute)
IMPLEMENT_GLOBAL_SHADER(FQcomShadowDenoiserFilterCS, "/Plugin/QcomShadowDenoiser/Private/QcomShadowDenoiserFilter.usf", "FilterCS", SF_Compute)


//FQcomShadowDenoiserImplementation::FQcomShadowDenoiserImplementation() : IScreenSpaceDenoiser()
//{
//}

void FQcomShadowDenoiserImplementation::Initialize()
{
	CreateFixedFilterLookupTexture();
}

void FQcomShadowDenoiserImplementation::Shutdown()
{
	DestroyFixedFilterLookupTexture();
}

const TCHAR* FQcomShadowDenoiserImplementation::GetDebugName() const
{
	return TEXT("QcomShadowDenoiser");
}
	
static bool IsSupportedLightType(ELightComponentType LightType)
{
	return LightType == LightType_Point || LightType == LightType_Directional || LightType == LightType_Rect || LightType == LightType_Spot;
}

//
// Lookup texture for the fixed filter.  Since horizontal pass of fixed filter is 17 pixels wide and pixels are 0 or 1 we can reduce that to 2 lookups into a 512x1 texture (with x coord as binary representation of the 8 or 9 relevant bits).
void FQcomShadowDenoiserImplementation::CreateFixedFilterLookupTexture()
{
	const size_t cFilterRadius = 8;
	static const float cFilterWeights[cFilterRadius+1]={1.0,
														0.963640444,
														0.862303357,
														0.716531311,
														0.552892001,
														0.39616443 ,
														0.263597138,
														0.162868066,
														0.0934461102 };
	const size_t cFilterLookupWidth = 1 << (cFilterRadius + 1); // radius and the center
	const size_t cFilterLookupHeight = 2;
	std::array<float, cFilterLookupWidth * cFilterLookupHeight> FilterLookup{};
	for (auto k = 0; k <= cFilterRadius; ++k)
	{
		uint32_t maskRight = 1 << k;
		uint32_t maskLeft = 1 << (cFilterRadius - k);
		float Weight = cFilterWeights[k];
		if (k == 0)
			Weight /= 2.0f;    // the lookup is all the bits on one side of the filter PLUS the center.  We sample the lookup twice (once for left+center, once for right+center, so we need to halve the center wight as it is added twice at runtime).
		for (auto i = 0; i < cFilterLookupWidth; ++i)
		{
			if ((maskRight & i) != 0)
			{
				FilterLookup[cFilterLookupWidth + i] += Weight; // lookup for 9 bits on 'right' side of filter
			}
			if ((maskLeft & i) != 0)
			{
				FilterLookup[i] += Weight;                      // lookup for 9 bits on 'left' side of filter (center bit is in both left and right)
			}
		}
	}
	const float Scale = 1.f / (FilterLookup.back() * 2.f);				// last entry is max value for one half (including half the center), double to get the 
	std::for_each(std::begin(FilterLookup), std::end(FilterLookup), [Scale](auto& v) { v *= Scale; });

	// Create the texture
	UTexture2D* pShadowBitFilterLookupTexture = UTexture2D::CreateTransient(cFilterLookupWidth, cFilterLookupHeight, PF_R16F);

	// Lock the texture for modification
	auto& Mip = pShadowBitFilterLookupTexture->GetPlatformData()->Mips[0];
	uint16* MipData = reinterpret_cast<uint16*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	check(MipData);

	// Fill the texture with the fp16 lookup data
	for (auto i = 0; i < FilterLookup.size(); ++i)
	{
		MipData[i] = FFloat16(FilterLookup[i]).Encoded;
	}

	// Unlock the texture data
	Mip.BulkData.Unlock();
	pShadowBitFilterLookupTexture->UpdateResource();

	ShadowBitFilterLookupTexture = TStrongObjectPtr<UTexture2D>(pShadowBitFilterLookupTexture);
}

void FQcomShadowDenoiserImplementation::DestroyFixedFilterLookupTexture()
{
	if (ShadowBitFilterLookupTexture->GetResource())
	{
		ShadowBitFilterLookupTexture->ReleaseResource();
	}
	ShadowBitFilterLookupTexture->MarkAsGarbage();
	ShadowBitFilterLookupTexture = nullptr;
}

IScreenSpaceDenoiser::EShadowRequirements FQcomShadowDenoiserImplementation::GetShadowRequirements(
	const FViewInfo& View,
	const FLightSceneInfo& LightSceneInfo,
	const FShadowRayTracingConfig& RayTracingConfig) const
{
	const EDenoiserMode Mode = GetQcomShadowDenoiserMode();
	switch (Mode) {
	case EDenoiserMode::Unreal:
		return GetDefaultDenoiser()->GetShadowRequirements(View, LightSceneInfo, RayTracingConfig);
	case EDenoiserMode::None:
		return EShadowRequirements::Bailout;
	case EDenoiserMode::Temporal:
		return EShadowRequirements::PenumbraAndClosestOccluder;
	default:
		assert(0);
		return EShadowRequirements::PenumbraAndClosestOccluder;
	}
}

void FQcomShadowDenoiserImplementation::DenoiseShadowVisibilityMasks(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const TStaticArray<FShadowVisibilityParameters, IScreenSpaceDenoiser::kMaxBatchSize>& InputParameters,
	const int32 InputParameterCount,
	TStaticArray<FShadowVisibilityOutputs, IScreenSpaceDenoiser::kMaxBatchSize>& Outputs) const
{
	const auto& settings = GetMutableDefault<UQcomShadowDenoiserPluginSettings>();
	const EDenoiserMode Mode = GetQcomShadowDenoiserMode();
	if (Mode == EDenoiserMode::Unreal)
	{
		GetDefaultDenoiser()->DenoiseShadowVisibilityMasks(GraphBuilder, View, PreviousViewInfos, SceneTextures, InputParameters, InputParameterCount, Outputs);
		return;
	}

	RDG_GPU_STAT_SCOPE(GraphBuilder, QcomShadowsDenoiser);

	auto Viewport = View.ViewRect;

	TStaticArray<FScreenSpaceDenoiserHistory*, kMaxBatchSize> PrevHistories;
	TStaticArray<FScreenSpaceDenoiserHistory*, kMaxBatchSize> NewHistories;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> ShadowedBitMaskDescs;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> TileClassificationDescs;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> BlockMinMaxDescs;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> OutputDescs;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> DistanceDescs;
	TStaticArray<FRDGTextureDesc, kMaxBatchSize> FixedFilterDescs;

	const EDenoiserTemporalReject TemporalRejectMode = GetGQcomShadowDenoiserRejectMode();

	for (int32 BatchedSignalId = 0; BatchedSignalId < InputParameterCount; ++BatchedSignalId)
	{
		const FShadowVisibilityParameters& Parameters = InputParameters[BatchedSignalId];
		const FLightSceneProxy* Proxy = Parameters.LightSceneInfo->Proxy;

		// Code from UE5 ScreenSpaceDenoise to setup the history buffers (for reprojection).
		const ULightComponent* LightComponent = Proxy->GetLightComponent();
		TSharedPtr<FScreenSpaceDenoiserHistory>* PrevHistoryEntry = PreviousViewInfos->ShadowHistories.Find(LightComponent);
		PrevHistories[BatchedSignalId] = PrevHistoryEntry ? PrevHistoryEntry->Get() : nullptr;
		NewHistories[BatchedSignalId] = nullptr;

		ensure(IsSupportedLightType(ELightComponentType(Proxy->GetLightType())));

		if (!View.bStatePrevViewInfoIsReadOnly)
		{
			check(View.ViewState);
			TSharedPtr<FScreenSpaceDenoiserHistory>* NewHistoryEntry = View.ViewState->PrevFrameViewInfo.ShadowHistories.Find(LightComponent);
			if (NewHistoryEntry == nullptr)
			{
				FScreenSpaceDenoiserHistory* NewHistory = new FScreenSpaceDenoiserHistory;
				View.ViewState->PrevFrameViewInfo.ShadowHistories.Emplace(LightComponent, NewHistory);
				NewHistories[BatchedSignalId] = NewHistory;
			}
			else
			{
				NewHistories[BatchedSignalId] = NewHistoryEntry->Get();
			}
		}

		const FIntPoint InputExtent = InputParameters[BatchedSignalId].InputTextures.Mask->Desc.Extent;
		const FIntPoint BlockExtent = FIntPoint::DivideAndRoundUp( InputExtent, FComputeShaderUtils::kGolden2DGroupSize );
		ShadowedBitMaskDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(BlockExtent, PF_R32G32_UINT, FClearValueBinding::None,
																				TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);
		TileClassificationDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(BlockExtent, PF_R8_SINT, FClearValueBinding::None,
																				   TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);
		BlockMinMaxDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(BlockExtent, PF_G16R16F, FClearValueBinding::None,
																			TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);
		OutputDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(InputExtent, PF_FloatRGBA, FClearValueBinding::None,
																	   TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);
		DistanceDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(InputExtent, PF_G16R16F, FClearValueBinding::None,
																		 TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);
		FixedFilterDescs[BatchedSignalId] = FRDGTextureDesc::Create2D(InputExtent, PF_G16, FClearValueBinding::None,
																			TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable);

		// Inputs and outputs are batched but we denoise each item seperately.

		static_assert(kMaxBatchSize == 4);
		static const TCHAR* ShadowedBitMaskTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowBitMask.0"), TEXT("QcomDenoiseShadowBitMask.1"), TEXT("QcomDenoiseShadowBitMask.2"), TEXT("QcomDenoiseShadowBitMask.3") };
		static const TCHAR* TileClassificationTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowTileClassification.0"), TEXT("QcomDenoiseShadowTileClassification.1"), TEXT("QcomDenoiseShadowTileClassification.2"), TEXT("QcomDenoiseShadowTileClassification.3") };
		static const TCHAR* TileMinMaxTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowBlockMinMax.0"), TEXT("QcomDenoiseShadowBlockMinMax.1"), TEXT("QcomDenoiseShadowBlockMinMax.2"), TEXT("QcomDenoiseShadowBlockMinMax.3") };
		static const TCHAR* ReprojectedTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowReprojected.0"), TEXT("QcomDenoiseShadowReprojected.1"), TEXT("QcomDenoiseShadowReprojected.2"), TEXT("QcomDenoiseShadowReprojected.3") };
		static const TCHAR* Intermediate0TextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowIntermediate0Mask.0"), TEXT("QcomDenoiseShadowIntermediate0Mask.1"), TEXT("QcomDenoiseShadowIntermediate0Mask.2"), TEXT("QcomDenoiseShadowIntermediate0Mask.3") };
		static const TCHAR* Intermediate1TextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowIntermediate1Mask.0"), TEXT("QcomDenoiseShadowIntermediate1Mask.1"), TEXT("QcomDenoiseShadowIntermediate1Mask.2"), TEXT("QcomDenoiseShadowIntermediate1Mask.3") };
		static const TCHAR* OutputTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowOutputMask.0"), TEXT("QcomDenoiseShadowOutputMask.1"), TEXT("QcomDenoiseShadowOutputMask.2"), TEXT("QcomDenoiseShadowOutputMask.3") };
		static const TCHAR* MomentTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowMoment.0"), TEXT("QcomDenoiseShadowMoment.1"), TEXT("QcomDenoiseShadowMoment.2"), TEXT("QcomDenoiseShadowMoment.3") };
		static const TCHAR* DistanceTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseShadowDistance.0"), TEXT("QcomDenoiseShadowDistance.1"), TEXT("QcomDenoiseShadowDistance.2"), TEXT("QcomDenoiseShadowDistance.3") };
		static const TCHAR* FixedFilterTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseFixed.0"), TEXT("QcomDenoiseFixed.1"),TEXT("QcomDenoiseFixed.2"),TEXT("QcomDenoiseFixed.3") };
		static const TCHAR* FixedFilterIntermediateTextureNames[kMaxBatchSize] = { TEXT("QcomDenoiseFixedIntermediate.0"), TEXT("QcomDenoiseFixedIntermediate.1"),TEXT("QcomDenoiseFixedIntermediate.2"),TEXT("QcomDenoiseFixedIntermediate.3") };
		auto ShadowedBitMaskTexture = GraphBuilder.CreateTexture(ShadowedBitMaskDescs[BatchedSignalId], ShadowedBitMaskTextureNames[BatchedSignalId]);
		auto ReprojectedTexture = GraphBuilder.CreateTexture(OutputDescs[BatchedSignalId], ReprojectedTextureNames[BatchedSignalId]);
		auto IntermediateTexture0 = GraphBuilder.CreateTexture(OutputDescs[BatchedSignalId], Intermediate0TextureNames[BatchedSignalId]);
		auto IntermediateTexture1 = GraphBuilder.CreateTexture(OutputDescs[BatchedSignalId], Intermediate1TextureNames[BatchedSignalId]);
		auto OutputTexture = GraphBuilder.CreateTexture(OutputDescs[BatchedSignalId], OutputTextureNames[BatchedSignalId]);
		auto MomentTexture = GraphBuilder.CreateTexture(OutputDescs[BatchedSignalId], MomentTextureNames[BatchedSignalId]);
		auto DistanceTexture = GraphBuilder.CreateTexture(DistanceDescs[BatchedSignalId], DistanceTextureNames[BatchedSignalId]);

		FScreenSpaceDenoiserHistory DummyPrevFrameHistory;
		FScreenSpaceDenoiserHistory* PrevFrameHistory = PrevHistories[BatchedSignalId] ? PrevHistories[BatchedSignalId] : &DummyPrevFrameHistory;

		FQcomShadowDenoiserSharedParameters SharedParameters;
		SharedParameters.SceneTextures = SceneTextures;
		SharedParameters.ViewUniformBuffer = View.ViewUniformBuffer;
		SharedParameters.bCameraCut = (View.bCameraCut || !PrevFrameHistory->IsValid()) ? 1 : 0;

		if (!SceneTextures.GBufferVelocityTexture)
			SharedParameters.SceneTextures.GBufferVelocityTexture = GSystemTextures.GetBlackDummy(GraphBuilder);

		//
		// Bit Mask generation pass (convert screen pixels lit/unlit boolean into a bitmask image - 64bits per pixel arranged as RG )
		//
		{
			FQcomShadowDenoiserBitMaskCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserBitMaskCS::FParameters>();

			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			FIntPoint BufferExtent = SceneTextures.SceneDepthTexture->Desc.Extent;
			PassParameters->ThreadIdToBufferUV = FVector4f(1.0f / BufferExtent.X, 1.0f / BufferExtent.Y, 0.0f, 0.0f);
			PassParameters->BlockMaxSize = FUintVector2(BlockExtent.X, BlockExtent.Y);
			PassParameters->InputRtShadowTexture = InputParameters[BatchedSignalId].InputTextures.Mask;
			PassParameters->OutputShadowBitBuffer = GraphBuilder.CreateUAV(ShadowedBitMaskTexture);

			TShaderMapRef<FQcomShadowDenoiserBitMaskCS> ComputeShader(View.ShaderMap);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Bitmask %dx%d", BlockExtent.X, BlockExtent.Y),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(BlockExtent, FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// Block min/max pass (determines min/max shadow distance values per 8x8 block)
		//
		const bool MinMaxPassNeeded = TemporalRejectMode == EDenoiserTemporalReject::Depth;
		const auto TileMinMaxTexture = MinMaxPassNeeded ? GraphBuilder.CreateTexture(BlockMinMaxDescs[BatchedSignalId], TileMinMaxTextureNames[BatchedSignalId]) : GSystemTextures.GetBlackDummy(GraphBuilder);
		if (MinMaxPassNeeded)
		{
			FQcomShadowDenoiserBlockMinMaxCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserBlockMinMaxCS::FParameters>();

			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			FIntPoint BufferExtent = SceneTextures.SceneDepthTexture->Desc.Extent;
			PassParameters->ThreadIdToBufferUV = FVector4f(1.0f / BufferExtent.X, 1.0f / BufferExtent.Y, 0.0f, 0.0f);
			PassParameters->BlockMaxSize = FUintVector2(BlockExtent.X, BlockExtent.Y);
			PassParameters->InputRtShadowTexture = InputParameters[BatchedSignalId].InputTextures.Mask;
			PassParameters->OutputMinMaxBuffer = GraphBuilder.CreateUAV(TileMinMaxTexture);

			TShaderMapRef<FQcomShadowDenoiserBlockMinMaxCS> ComputeShader(View.ShaderMap);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise BlockMinMax %dx%d", BlockExtent.X, BlockExtent.Y),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(BlockExtent, FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// Tile Classification pass (identify which blocks of screen pixels are fully shadowed or fully lit)
		//
		const bool TileClassificationPassNeeded = GetGQcomShadowDenoiserClassificationOptimization();
		const auto TileClassificationTexture = TileClassificationPassNeeded ? GraphBuilder.CreateTexture(TileClassificationDescs[BatchedSignalId], TileClassificationTextureNames[BatchedSignalId]) : GSystemTextures.GetBlackDummy(GraphBuilder);
		if (TileClassificationPassNeeded)
		{
			FQcomShadowDenoiserClassificationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserClassificationCS::FParameters>();

			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			FIntPoint BufferExtent = SceneTextures.SceneDepthTexture->Desc.Extent;
			PassParameters->ThreadIdToBufferUV = FVector4f(1.0f / BufferExtent.X, 1.0f / BufferExtent.Y, 0.0f, 0.0f);
			PassParameters->BlockMaxSize = FUintVector2(BlockExtent.X, BlockExtent.Y);
			PassParameters->InputShadowBitBuffer = ShadowedBitMaskTexture;
			PassParameters->InputHistoryClassificationBuffer = RegisterExternalTextureWithFallback(
				GraphBuilder, PrevFrameHistory->TileClassification, GSystemTextures.BlackDummy/*default to 'unknown' ie run the denoiser on all tile blocks*/);
			PassParameters->OutputClassificationBuffer = GraphBuilder.CreateUAV(TileClassificationTexture);

			TShaderMapRef<FQcomShadowDenoiserClassificationCS> ComputeShader(View.ShaderMap);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Classification %dx%d", BlockExtent.X, BlockExtent.Y),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(BlockExtent, FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// Fixed Filter pass
		// Filter 17x17 shadow area to determine average shadow value (then is then used to clamp temporal filter)
		//
		const bool FixedFilterPassNeeded = TemporalRejectMode == EDenoiserTemporalReject::Filter;
		auto FixedFilterTexture = FixedFilterPassNeeded ? GraphBuilder.CreateTexture(FixedFilterDescs[BatchedSignalId], FixedFilterTextureNames[BatchedSignalId]) : GSystemTextures.GetBlackDummy(GraphBuilder);
		if (FixedFilterPassNeeded)
		{
			//
			// Horizontal filter (fixed 17x17 area)
			//
			auto FilterIntermediateTexture = GraphBuilder.CreateTexture(FixedFilterDescs[BatchedSignalId], FixedFilterIntermediateTextureNames[BatchedSignalId]);
			{
				FQcomShadowDenoiserFixedFilterHorizCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserFixedFilterHorizCS::FParameters>();

				// Shared shader parameters
				PassParameters->SharedParameters = SharedParameters;
				// Custom shader parameters
				PassParameters->InputShadowBitBuffer = ShadowedBitMaskTexture;
				PassParameters->InputTileClassificationTexture = TileClassificationTexture;
				PassParameters->InputFixedFilterLookup = ShadowBitFilterLookupTexture->GetResource()->TextureRHI;
				PassParameters->Output = GraphBuilder.CreateUAV(FilterIntermediateTexture);

				TShaderMapRef<FQcomShadowDenoiserFixedFilterHorizCS> ComputeShader(View.ShaderMap);
				FComputeShaderUtils::AddPass(
					GraphBuilder,
					RDG_EVENT_NAME("Qcom ShadowDenoise FixedHoriz %dx%d", Viewport.Width(), Viewport.Height()),
					ComputeShader,
					PassParameters,
					FComputeShaderUtils::GetGroupCount(Viewport.Size(), FComputeShaderUtils::kGolden2DGroupSize));
			}
			//
			// Vertical filter (fixed 17x17 area)
			//
			{
				FQcomShadowDenoiserFixedFilterVertCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserFixedFilterVertCS::FParameters>();

				// Shared shader parameters
				PassParameters->SharedParameters = SharedParameters;
				// Custom shader parameters
				PassParameters->InputHorizontallyFiltered = FilterIntermediateTexture;
				PassParameters->InputTileClassificationTexture = TileClassificationTexture;
				PassParameters->Output = GraphBuilder.CreateUAV(FixedFilterTexture);

				TShaderMapRef<FQcomShadowDenoiserFixedFilterVertCS> ComputeShader(View.ShaderMap);
				FComputeShaderUtils::AddPass(
					GraphBuilder,
					RDG_EVENT_NAME("Qcom ShadowDenoise FixedVert %dx%d", Viewport.Width(), Viewport.Height()),
					ComputeShader,
					PassParameters,
					FComputeShaderUtils::GetGroupCount(Viewport.Size(), FIntPoint{ 16,8 }));
			}
		}

		//
		// Reproject pass (temporal reprojection of accumulated 'history' shadow data into current screen space)
		//
		{
			FQcomShadowDenoiserReprojectCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserReprojectCS::FParameters>();

			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			FIntPoint BufferExtent = SceneTextures.SceneDepthTexture->Desc.Extent;
			PassParameters->ThreadIdToBufferUV = FVector4f(1.0f / BufferExtent.X, 1.0f / BufferExtent.Y, 0.0f, 0.0f);
			//PassParameters->HistoryBufferUVMinMax = FVector4f(0.0f, 0.0f, 1.0f, 1.0f);
			PassParameters->HistoryBufferUVMinMax = FVector4f(View.ViewRect.Min, View.ViewRect.Max);

			FQcomShadowDenoiserReprojectCS::FPermutationDomain PermutationVector;
			PermutationVector.Set<FQcomShadowDenoiserReprojectCS::FTemporalRejectMode>((int)TemporalRejectMode);

			const bool bValidHistory = !bForceDisableReproject && !View.bCameraCut;

			FRDGTextureRef DepthBufferHistoryTexture = GraphBuilder.RegisterExternalTexture(
				bValidHistory && View.PrevViewInfo.DepthBuffer.IsValid()
				? View.PrevViewInfo.DepthBuffer
				: GSystemTextures.BlackDummy);
			PassParameters->HistoryDepthTexture = DepthBufferHistoryTexture;
			PassParameters->InputRtShadowTexture = InputParameters[BatchedSignalId].InputTextures.Mask;
			PassParameters->HistoryShadowTexture = RegisterExternalTextureWithFallback(
				GraphBuilder, PrevFrameHistory->RT[0], GSystemTextures.BlackDummy);
			PassParameters->HistoryMomentTexture = RegisterExternalTextureWithFallback(
				GraphBuilder, PrevFrameHistory->RT[1], GSystemTextures.BlackDummy);
			PassParameters->HistoryDistanceTexture = RegisterExternalTextureWithFallback(
				GraphBuilder, PrevFrameHistory->RT[2], GSystemTextures.BlackDummy);
			PassParameters->InputTileClassificationTexture = TileClassificationTexture;
			PassParameters->InputTileMinMaxTexture = TileMinMaxTexture;
			PassParameters->InputFixedFilterTexture = FixedFilterTexture;
			PassParameters->OutputShadowBuffer = GraphBuilder.CreateUAV(ReprojectedTexture);
			PassParameters->OutputMomentBuffer = GraphBuilder.CreateUAV(MomentTexture);
			PassParameters->OutputDistanceBuffer = GraphBuilder.CreateUAV(DistanceTexture);

			TShaderMapRef<FQcomShadowDenoiserReprojectCS> ComputeShader(View.ShaderMap, PermutationVector);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Reproject %dx%d", Viewport.Width(), Viewport.Height()),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(Viewport.Size(), FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// A-trous Filter pass 1
		//
		{
			FQcomShadowDenoiserFilterCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserFilterCS::FParameters>();

			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			PassParameters->InputShadowBuffer = ReprojectedTexture;
			PassParameters->InputTileClassificationTexture = TileClassificationTexture;
			PassParameters->Output = GraphBuilder.CreateUAV(IntermediateTexture0);

			FQcomShadowDenoiserFilterCS::FPermutationDomain PermutationVector;
			PermutationVector.Set<FQcomShadowDenoiserFilterCS::FATrousStepDim>(0);

			TShaderMapRef<FQcomShadowDenoiserFilterCS> ComputeShader(View.ShaderMap, PermutationVector);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Filter1 %dx%d", Viewport.Width(), Viewport.Height()),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(Viewport.Size(), FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// A-trous Filter pass 2
		//
		{
			FQcomShadowDenoiserFilterCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserFilterCS::FParameters>();
			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			PassParameters->InputShadowBuffer = IntermediateTexture0;
			PassParameters->InputTileClassificationTexture = TileClassificationTexture;
			PassParameters->Output = GraphBuilder.CreateUAV(IntermediateTexture1);

			FQcomShadowDenoiserFilterCS::FPermutationDomain PermutationVector;
			PermutationVector.Set<FQcomShadowDenoiserFilterCS::FATrousStepDim>(1);

			TShaderMapRef<FQcomShadowDenoiserFilterCS> ComputeShader(View.ShaderMap, PermutationVector);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Filter2 %dx%d", Viewport.Width(), Viewport.Height()),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(Viewport.Size(), FComputeShaderUtils::kGolden2DGroupSize));
		}

		//
		// A-trous Filter pass 3
		//
		{
			FQcomShadowDenoiserFilterCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FQcomShadowDenoiserFilterCS::FParameters>();
			// Shared shader parameters
			PassParameters->SharedParameters = SharedParameters;
			// Custom shader parameters
			PassParameters->InputShadowBuffer = IntermediateTexture1;
			PassParameters->InputTileClassificationTexture = TileClassificationTexture;
			PassParameters->Output = GraphBuilder.CreateUAV(OutputTexture);

			FQcomShadowDenoiserFilterCS::FPermutationDomain PermutationVector;
			PermutationVector.Set<FQcomShadowDenoiserFilterCS::FATrousStepDim>(2);

			TShaderMapRef<FQcomShadowDenoiserFilterCS> ComputeShader(View.ShaderMap, PermutationVector);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Qcom ShadowDenoise Filter3 %dx%d", Viewport.Width(), Viewport.Height()),
				ComputeShader,
				PassParameters,
				FComputeShaderUtils::GetGroupCount(Viewport.Size(), FComputeShaderUtils::kGolden2DGroupSize));
		}
		Outputs[BatchedSignalId].Mask = OutputTexture;

		if (!View.bStatePrevViewInfoIsReadOnly)
		{
			//
			// Store/queue textures we need next frame (history)
			// 
			// Depth history (something may already be doing this)
			GraphBuilder.QueueTextureExtraction(SceneTextures.SceneDepthTexture, &View.ViewState->PrevFrameViewInfo.DepthBuffer);
			// Denoiser output history
			FScreenSpaceDenoiserHistory* NewHistory = NewHistories[BatchedSignalId];
			check(NewHistory);
			GraphBuilder.QueueTextureExtraction(IntermediateTexture0, &NewHistory->RT[0]);	// feedback output of first a-trous spacial filter pass
			GraphBuilder.QueueTextureExtraction(MomentTexture, &NewHistory->RT[1]);
			GraphBuilder.QueueTextureExtraction(DistanceTexture, &NewHistory->RT[2]);
            if (TileClassificationPassNeeded)
    			GraphBuilder.QueueTextureExtraction(TileClassificationTexture, &NewHistory->TileClassification);
		}
	}
}


// Fallback to default denoiser
IScreenSpaceDenoiser::FPolychromaticPenumbraOutputs FQcomShadowDenoiserImplementation::DenoisePolychromaticPenumbraHarmonics(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FPolychromaticPenumbraHarmonics& Inputs) const
{
	return GetDefaultDenoiser()->DenoisePolychromaticPenumbraHarmonics(GraphBuilder, View, PreviousViewInfos, SceneTextures, Inputs);
}


// Fallback to default denoiser
IScreenSpaceDenoiser::FReflectionsOutputs FQcomShadowDenoiserImplementation::DenoiseReflections(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FReflectionsInputs& ReflectionInputs,
	const FReflectionsRayTracingConfig RayTracingConfig) const
{
	return GetDefaultDenoiser()->DenoiseReflections(GraphBuilder, View, PreviousViewInfos, SceneTextures, ReflectionInputs, RayTracingConfig);
}


// Fallback to default denoiser
IScreenSpaceDenoiser::FReflectionsOutputs FQcomShadowDenoiserImplementation::DenoiseWaterReflections(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FReflectionsInputs& ReflectionInputs,
	const FReflectionsRayTracingConfig RayTracingConfig) const
{
	return GetDefaultDenoiser()->DenoiseWaterReflections(GraphBuilder, View, PreviousViewInfos, SceneTextures, ReflectionInputs, RayTracingConfig);
}


// Fallback to default denoiser
IScreenSpaceDenoiser::FAmbientOcclusionOutputs FQcomShadowDenoiserImplementation::DenoiseAmbientOcclusion(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FAmbientOcclusionInputs& AOInputs,
	const FAmbientOcclusionRayTracingConfig RayTracingConfig) const
{
	return GetDefaultDenoiser()->DenoiseAmbientOcclusion(GraphBuilder, View, PreviousViewInfos, SceneTextures, AOInputs, RayTracingConfig);
}


// Fallback to default denoiser
FSSDSignalTextures FQcomShadowDenoiserImplementation::DenoiseDiffuseIndirect(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FDiffuseIndirectInputs& Inputs,
	const FAmbientOcclusionRayTracingConfig Config) const
{
	return GetDefaultDenoiser()->DenoiseDiffuseIndirect(GraphBuilder, View, PreviousViewInfos, SceneTextures, Inputs, Config);
}


// Fallback to default denoiser
IScreenSpaceDenoiser::FDiffuseIndirectOutputs FQcomShadowDenoiserImplementation::DenoiseSkyLight(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FDiffuseIndirectInputs& Inputs,
	const FAmbientOcclusionRayTracingConfig Config) const
{
	return GetDefaultDenoiser()->DenoiseSkyLight(GraphBuilder, View, PreviousViewInfos, SceneTextures, Inputs, Config);
}


// Fallback to default denoiser
FSSDSignalTextures FQcomShadowDenoiserImplementation::DenoiseDiffuseIndirectHarmonic(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FDiffuseIndirectHarmonic& Inputs,
	const HybridIndirectLighting::FCommonParameters& CommonDiffuseParameters) const
{
	return GetDefaultDenoiser()->DenoiseDiffuseIndirectHarmonic(GraphBuilder, View, PreviousViewInfos, SceneTextures, Inputs, CommonDiffuseParameters);
}


bool FQcomShadowDenoiserImplementation::SupportsScreenSpaceDiffuseIndirectDenoiser(EShaderPlatform Platform) const
{
	return false;
}


FSSDSignalTextures FQcomShadowDenoiserImplementation::DenoiseScreenSpaceDiffuseIndirect(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	FPreviousViewInfo* PreviousViewInfos,
	const FSceneTextureParameters& SceneTextures,
	const FDiffuseIndirectInputs& Inputs,
	const FAmbientOcclusionRayTracingConfig Config) const
{
	return GetDefaultDenoiser()->DenoiseScreenSpaceDiffuseIndirect(GraphBuilder, View, PreviousViewInfos, SceneTextures, Inputs, Config);
}
