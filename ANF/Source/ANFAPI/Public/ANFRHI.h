//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once

#include "Engine/Engine.h"
#include "HAL/CriticalSection.h"

#include "anf.h"
#include "anf_sr.h"
#include "anf_fg.h"

class FRHICommandListImmediate;
class FRDGBuilder;
class FRDGTexture;
class FRHITexture;

DECLARE_LOG_CATEGORY_EXTERN(LogANFRHI, Verbose, All);

class ANFAPI_API ANFRHI
{
public:
	static ANFRHI* GetRHI();
	static void Destroy();

	ANFRHI();
	virtual ~ANFRHI();

	void AddBlitTexturePass(FRDGBuilder& GraphBuilder, FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit = false);
	void AddBlitTexturePass_RenderThread(FRHICommandListImmediate& RHICmdList, FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit = false);

	void AddStartTimedPass(FRDGBuilder& GraphBuilder, const TCHAR* passName);
	void AddEndTimedPass(FRDGBuilder& GraphBuilder, const TCHAR* passName);

	void StartTimedPass(FRHICommandListImmediate& RHICmdList, const TCHAR* passName);
	void EndTimedPass(FRHICommandListImmediate& RHICmdList, const TCHAR* passName);

	void SetResourceRequirements(AnfTechniqueId techniqueID, AnfResourceLabel label, const AnfResourceRequirements* pReqs);

	inline void RHIAddBlitTexturePass(FRHICommandListImmediate& RHICmdList, FRHITexture* inputTexture, FRHITexture* outputTexture, bool finalBlit)
	{
		AddBlitTexturePass_Internal(RHICmdList, inputTexture, outputTexture, finalBlit);
	}

	virtual AnfStructHeader* GetAdapterInfo() = 0;
	virtual void AddANFTechniqueCreateInfoRHI(AnfTechniqueCreateInfo* pCreateInfo) = 0;
	virtual AnfHandle GetImageHandle(FRHITexture* image) = 0;
	virtual uint32 GetResourceParameterLayout(uint32_t resIndex) = 0;
	virtual AnfHandle GetCommandList(FRHICommandListImmediate& RHICmdList) = 0;
	
	virtual void AddExtensions(TArrayView<const ANSICHAR* const> deviceExtensions, TArrayView<const ANSICHAR* const> instanceExtensions) = 0;

	virtual bool Is32BitDepthFormat(FRHITexture* image) = 0;
	
	virtual ETextureCreateFlags GetExtraFlags_DepthInput() = 0;
	virtual ETextureCreateFlags GetExtraFlags_MotionInput() = 0;
	virtual ETextureCreateFlags GetExtraFlags_ColorInput(bool fg) = 0;
	virtual ETextureCreateFlags GetExtraFlags_ColorOutput(bool fg) = 0;

	virtual bool GetTechniqueExtensions(AnfTechniqueId techniqueID, const AnfStructHeader* pClientApiRequirements, TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions) = 0;
	virtual AnfClientApi GetClientAPI() = 0;
protected:
	virtual void AddBlitTexturePass_Internal(FRHICommandListImmediate& RHICmdList, FRHITexture* inputTexture, FRHITexture* outputTexture, bool finalBlit) = 0;

	virtual float WriteNewTimeStamp(FRHICommandListImmediate& RHICmdList, uint32 passId, uint32 queryId, bool start) = 0;

	virtual void RHISetResourceRequirements(AnfTechniqueId techniqueID, AnfResourceLabel label, const AnfResourceRequirements* pReqs) = 0;

	static const uint32_t s_numQueries = 5;
	static const float s_alpha;

	inline float CalculateAverage(float newVal, float curAvg) const
	{
		if (curAvg <= 0.f)
		{
			return newVal;
		}
		else
		{
			return (s_alpha * newVal) + ((1.f - s_alpha) * curAvg);
		}
	}

	struct TimingData
	{
		float averageTime;
		float lastTime;
		FDateTime lastPrintTime;
		uint32 queryId;

		TimingData()
			: averageTime(-1.f)
			, lastTime(-1.f)
			, lastPrintTime()
			, queryId(0)
		{
		}
	};

	TMap<uint32, TimingData> m_passTimingData;

	static bool RequiresBlit(FRDGTexture* inputTexture, FRDGTexture* outputTexture, bool finalBlit);

	static ANFRHI* s_instance;
	static FCriticalSection s_instanceLock;
};
