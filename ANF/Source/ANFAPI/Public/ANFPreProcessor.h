//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once

#include "Engine/Engine.h"
#include "HAL/CriticalSection.h"
#include "Modules/ModuleManager.h"
#include "RHIDefinitions.h"
#include "Math/MathFwd.h"
#include "Containers/UnrealString.h"

#include "ANFConfig.h"

class FRDGTexture;
class FRHITexture;
class FRHICommandListImmediate;

DECLARE_LOG_CATEGORY_EXTERN(LogANFPreProcessor, Log, All);

class ANFAPI_API FANFPreProcessor
{
public:
	static FANFPreProcessor* Get()
	{
		FScopeLock Lock(&s_instanceLock);
		if (s_instance == nullptr)
		{
			s_instance = new FANFPreProcessor();
		}
		return s_instance;
	}

	static void Destroy()
	{
		FScopeLock Lock(&s_instanceLock);
		delete s_instance;
		s_instance = nullptr;
	}

	FANFPreProcessor();
	~FANFPreProcessor();

	struct ANFPreProcessorOutputs
	{
		FRDGTexture* color;
		FRDGTexture* depth;
		FRDGTexture* motion;
		FMatrix44f clipToPrevClip;
	};

	ANFPreProcessorOutputs Execute(
		const FViewInfo& View,
		FRDGBuilder& GraphBuilder,
		FRDGTexture* inputColor,
		FRDGTexture* inputDepth,
		FRDGTexture* inputMotion,
		FIntVector2 renderSize,
		FIntVector2 displaySize,
		FVector4f jitter,
		uint32 frameNumber,
		bool frameGenPass);

	inline FRHITexture* GetDepth32Texture(bool frameGen)
	{ 
		if (frameGen && GANFFrameGenSupportsLowResInputs == 0)
		{
			return m_storedImages[ANFStoredImage::FullResDepth32];
		}
		return m_storedImages[ANFStoredImage::Depth32];
	}

protected:
	static FANFPreProcessor* s_instance;
	static FCriticalSection s_instanceLock;

	enum ANFStoredImage
	{
		Color_SR,
		Color_FG,
		Motion,
		Depth,
		Depth32,
		FullResMotion,
		FullResDepth,
		FullResDepth32,
		Count
	};

	TRefCountPtr<FRHITexture> m_storedImages[ANFStoredImage::Count];
	FIntVector2 m_storedImagesDimms[ANFStoredImage::Count];
	uint32 m_storedImageFrameNumber[ANFStoredImage::Count];

	FRHITexture* CheckTexture(FRDGBuilder& GraphBuilder, uint32 frame, ANFStoredImage eid, FIntVector2 size);
};
