//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFPresenter.h"

DEFINE_LOG_CATEGORY(LogANFPresenter);

ANFPresenter::ANFPresenter()
	: FRHICustomPresent()
{
	for (uint32 mm = 0; mm < ANFPresenterMode_Count; ++mm)
	{
		m_averagePresentTime[mm] = 0.0;
		m_lastResetTime[mm] = 0.0;
	}

	m_lastPresentTime = 0.0;
	m_curMode = ANFPresenterMode_Generated;
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
bool ANFPresenter::Present(FRHIViewport* Viewport, IRHICommandContext& RHICmdContext, int32& InOutSyncInterval)
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 6
bool ANFPresenter::Present(IRHICommandContext& RHICmdContext, int32& InOutSyncInterval)
#else
bool ANFPresenter::Present(int32& InOutSyncInterval)
#endif
{
	PaceFrame(m_curMode);

	m_curMode = (m_curMode == ANFPresenterMode_Generated) ? ANFPresenterMode_Rendered : ANFPresenterMode_Generated;

	return true;
}

void ANFPresenter::PaceFrame(ANFPresenterMode mode)
{
	if ((GANFFrameGenUseFramePacer > 0) && (GANFFrameGenEnabled > 0))
	{
		static const double sMaxDeltaTimeMS = 1000.0;
		const int32 maxRefreshRate = GANFFrameGenFramePacerTargetFPS == 0 ? FPlatformMisc::GetMaxRefreshRate() : GANFFrameGenFramePacerTargetFPS;
		const double framePaceInterval = 1.0 / (double)maxRefreshRate;
		const int32 sampleSize = GANFFrameGenFramePacerSampleCount == 0 ? (maxRefreshRate / 4) : GANFFrameGenFramePacerSampleCount;

		const double sNewSampleMul = 1.0 / (double)sampleSize;
		const double sAvgMul = 1.0 - sNewSampleMul;
		const int32 SyncInterval = GANFFrameGenFramePacerTargetFPS == 0 ? RHIGetSyncInterval() : 1;

		const double currentTime = FPlatformTime::Seconds();
		const double deltaTimeMS = (currentTime - m_lastPresentTime) * 1000.0;

		const double targetTimeInterval = ((double)SyncInterval * framePaceInterval * 1000.0);
		const double targetThreashold = targetTimeInterval * 0.025;

		const bool resetAverageTime =
			((currentTime - m_lastResetTime[mode] > sMaxDeltaTimeMS) ||
				(m_averagePresentTime[mode] == 0.0));

		if (resetAverageTime && (deltaTimeMS >= sMaxDeltaTimeMS))
		{
			m_averagePresentTime[mode] = 0.0;
		}
		else if (resetAverageTime)
		{
			m_lastResetTime[mode] = currentTime;
			m_averagePresentTime[mode] = deltaTimeMS;
		}
		else
		{
			m_averagePresentTime[mode] = (m_averagePresentTime[mode] * sAvgMul) + (deltaTimeMS * sNewSampleMul);
		}

		const double averageTimeThreashold = m_averagePresentTime[mode] + targetThreashold;

		if (averageTimeThreashold < targetTimeInterval)
		{
			FRenderThreadIdleScope IdleScope(ERenderThreadIdleTypes::WaitingForGPUPresent);
			FPlatformProcess::SleepNoStats((targetTimeInterval - averageTimeThreashold) * 0.001f);
		}

		const double endTime = FPlatformTime::Seconds();
		if (GANFFrameGenPacerDebugPrint == 1)
		{
			UE_LOG(LogANFPresenter, Verbose, TEXT("[ANF] currentTime = %.04f, endTime = %0.4f, lastPresentTime = %0.4f, maxRefreshRate = %d, SyncInterval = %d, deltaTimeMS = %.04f, targetTimeInterval = %.04f, averagePresentTime[%d] = %.04f, averageTimeThreashold = %0.4f, Sleep Time = %.04f"),
				currentTime,
				endTime,
				m_lastPresentTime,
				maxRefreshRate,
				SyncInterval,
				deltaTimeMS,
				targetTimeInterval,
				mode,
				m_averagePresentTime[mode],
				averageTimeThreashold,
				averageTimeThreashold < targetTimeInterval ? targetTimeInterval - averageTimeThreashold : 0.0);
		}

		m_lastPresentTime = endTime;
	}
}