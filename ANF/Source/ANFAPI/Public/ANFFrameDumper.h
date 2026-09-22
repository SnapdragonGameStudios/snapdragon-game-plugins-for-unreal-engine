//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "RHIDefinitions.h"
#include "Math/MathFwd.h"
#include "Containers/UnrealString.h"

class FRHITexture;
class FRHICommandListImmediate;

class ANFAPI_API FANFFrameDumper
{
public:
	static void Execute(
		FRHICommandListImmediate& RHICmdList,
		uint32_t frameNumber,
		FRHITexture* inputColor,
		FRHITexture* inputDepth,
		FRHITexture* inputMotion,
		FRHITexture* outputColor,
		FVector4f jitterInfo,
		FMatrix44f clipToPrevClip,
		FString nameSuffix);

};
