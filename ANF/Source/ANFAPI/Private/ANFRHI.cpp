//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFRHI.h"
#include "RHI/Vulkan/ANFRHIVK.h"
#if ENGINE_MINOR_VERSION > 1
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"

BEGIN_SHADER_PARAMETER_STRUCT(FANFCopyParams, )
	RDG_TEXTURE_ACCESS(Input, ERHIAccess::CopySrc)
	RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

DEFINE_LOG_CATEGORY(LogANFRHI);

const float ANFRHI::s_alpha = 0.1f;

ANFRHI* ANFRHI::GetRHI()
{
	FScopeLock Lock(&s_instanceLock);
	if (s_instance == nullptr)
	{
#if PLATFORM_WINDOWS
		TCHAR const* DynamicRHIModuleName = GetSelectedDynamicRHIModuleName(false);
#else
		TCHAR const* DynamicRHIModuleName = TEXT("VulkanRHI");
#endif // PLATFORM_WINDOWS

		if (FString("VulkanRHI") == FString(DynamicRHIModuleName))
		{
			s_instance = new ANFRHIVK();
		}
		else
		{
			UE_LOG(LogANFRHI, Fatal, TEXT("ANF is only supported on Vulkan!"));
		}
	}
	return s_instance;
}

void ANFRHI::Destroy()
{
	FScopeLock Lock(&s_instanceLock);
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

void ANFRHI::AddBlitTexturePass(FRDGBuilder& GraphBuilder, FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit)
{

	FANFCopyParams* copyParams = GraphBuilder.AllocParameters<FANFCopyParams>();
	copyParams->Input = inputTexture;
	copyParams->Output = outputTexture;

	ERDGPassFlags flags = ERDGPassFlags::Copy;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("ANFBlitTexture %s -> %s", inputTexture->Name, outputTexture->Name),
		copyParams,
		flags,
		[this, copyParams, finalBlit](FRHICommandListImmediate& RHICmdList)
		{
			AddBlitTexturePass_RenderThread(RHICmdList, copyParams->Input, copyParams->Output, finalBlit);
		});
}

void ANFRHI::AddBlitTexturePass_RenderThread(FRHICommandListImmediate& RHICmdList, FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit)
{
	if (inputTexture == outputTexture)
	{
		return;
	}
	else
	{
		if (RequiresBlit(inputTexture, outputTexture, finalBlit))
		{
			RHIAddBlitTexturePass(RHICmdList, inputTexture->GetRHI(), outputTexture->GetRHI(), finalBlit);
		}
		else
		{
			FRHICopyTextureInfo copyInfo = FRHICopyTextureInfo();
			copyInfo.Size.X = FMath::Min(inputTexture->Desc.Extent.X, outputTexture->Desc.Extent.X);
			copyInfo.Size.Y = FMath::Min(inputTexture->Desc.Extent.Y, outputTexture->Desc.Extent.Y);
			copyInfo.Size.Z = 1;

			RHICmdList.CopyTexture(inputTexture->GetRHI(), outputTexture->GetRHI(), copyInfo);
		}
	}
}

void ANFRHI::AddStartTimedPass(FRDGBuilder& GraphBuilder, const TCHAR* passName)
{
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("%s Timer Start", passName),
		ERDGPassFlags::None | ERDGPassFlags::NeverCull,
		[this, passName](FRHICommandListImmediate& RHICmdList)
		{
			if (RHICmdList.IsInsideRenderPass())
			{
				RHICmdList.EndRenderPass();
			}
			RHICmdList.EnqueueLambda([this, passName](FRHICommandListImmediate& InCmdList) mutable
				{
					StartTimedPass(InCmdList, passName);
				});
		});
}

void ANFRHI::AddEndTimedPass(FRDGBuilder& GraphBuilder, const TCHAR* passName)
{
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("%s Timer End", passName),
		ERDGPassFlags::None | ERDGPassFlags::NeverCull,
		[this, passName](FRHICommandListImmediate& RHICmdList)
		{
			if (RHICmdList.IsInsideRenderPass())
			{
				RHICmdList.EndRenderPass();
			}
			RHICmdList.EnqueueLambda([this, passName](FRHICommandListImmediate& InCmdList) mutable
				{
					EndTimedPass(InCmdList, passName);
				});
		});
}

void ANFRHI::StartTimedPass(FRHICommandListImmediate& RHICmdList, const TCHAR* passName)
{
	const uint32 passId = GetTypeHash(FStringView(passName));
	TimingData& timingDataRef = m_passTimingData.FindOrAdd(passId);
	TimingData* curTimingData = &timingDataRef;

	const float curPassTime = WriteNewTimeStamp(RHICmdList, passId, curTimingData->queryId, true);
	if (curPassTime > 0.f)
	{
		curTimingData->averageTime = CalculateAverage(curPassTime, curTimingData->averageTime);
		curTimingData->lastTime = curPassTime;

		const float printDeltaTime = GANFProfilePrintTime;
		FDateTime curTime = FDateTime::Now();
		FTimespan deltaTime = (curTime - curTimingData->lastPrintTime);
		if (deltaTime.GetTotalSeconds() >= printDeltaTime)
		{
			UE_LOG(LogANFRHI, Verbose, TEXT("ANF %s Average Time = %.2f uS, Last Time = %.2f uS"), passName, curTimingData->averageTime, curTimingData->lastTime);
			curTimingData->lastPrintTime = curTime;
		}
	}
}

void ANFRHI::EndTimedPass(FRHICommandListImmediate& RHICmdList, const TCHAR* passName)
{
	const uint32 passId = GetTypeHash(FStringView(passName));
	TimingData* curTimingData = m_passTimingData.Find(passId);
	check(curTimingData != nullptr);

	WriteNewTimeStamp(RHICmdList, passId, curTimingData->queryId, false);
	curTimingData->queryId = (curTimingData->queryId + 1) % s_numQueries;
}

void ANFRHI::SetResourceRequirements(AnfTechniqueId techniqueID, AnfResourceLabel label, const AnfResourceRequirements* pReqs)
{
	RHISetResourceRequirements(techniqueID, label, pReqs);
}

bool ANFRHI::RequiresBlit(FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit)
{
	const FRDGTextureDesc& inputDesc = inputTexture->Desc;
	const FRDGTextureDesc& outputDesc = outputTexture->Desc;
	return
		inputDesc.Format != outputDesc.Format ||
		(!finalBlit &&
			(inputDesc.Extent.X > outputDesc.Extent.X ||
				inputDesc.Extent.Y > outputDesc.Extent.Y)) ||
		(finalBlit &&
			(inputDesc.Extent.X < outputDesc.Extent.X ||
				inputDesc.Extent.Y < outputDesc.Extent.Y));
		
}

ANFRHI::ANFRHI() {}
ANFRHI::~ANFRHI() {}

ANFRHI* ANFRHI::s_instance = nullptr;
FCriticalSection ANFRHI::s_instanceLock;
