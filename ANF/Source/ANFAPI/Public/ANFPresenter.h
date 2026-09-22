//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once

#include "Engine/Engine.h"
#include "HAL/CriticalSection.h"
#include "RHIDefinitions.h"
#include "Math/MathFwd.h"
#include "Containers/UnrealString.h"

#include "ANFConfig.h"

DECLARE_LOG_CATEGORY_EXTERN(LogANFPresenter, Verbose, All);

class ANFAPI_API ANFPresenter : public FRHICustomPresent
{
public:
	ANFPresenter();

	virtual void OnBackBufferResize() override {};
	virtual bool NeedsNativePresent() override { return true; };
	virtual bool NeedsAdvanceBackbuffer() override  { return false; };
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
	virtual bool Present(FRHIViewport* Viewport, IRHICommandContext& RHICmdContext, int32& InOutSyncInterval) override;
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 6
	virtual bool Present(IRHICommandContext& RHICmdContext, int32& InOutSyncInterval) override;
#else
	virtual bool Present(int32& InOutSyncInterval) override;
#endif

protected:
	enum ANFPresenterMode
	{
		ANFPresenterMode_Generated = 0,
		ANFPresenterMode_Rendered = 1,
		ANFPresenterMode_Count = 2,
	};

	void PaceFrame(ANFPresenterMode mode);

	double m_lastResetTime[ANFPresenterMode_Count];
	double m_lastPresentTime;
	double m_averagePresentTime[ANFPresenterMode_Count];
    ANFPresenterMode m_curMode;
    
};
