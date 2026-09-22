//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "ANFTUHistory.h"

#include "Engine/Engine.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessUpscale.h"
#include "PostProcess/TemporalAA.h"
#include "Containers/LockFreeList.h"

struct FPostProcessingInputs;

#if ENGINE_MINOR_VERSION > 2
#include "TemporalUpscaler.h"
using IANFTemporalUpscaler = UE::Renderer::Private::ITemporalUpscaler;
using ANFPassInput = UE::Renderer::Private::ITemporalUpscaler::FInputs;
using ANFPassOutput = UE::Renderer::Private::ITemporalUpscaler::FOutputs;
using ANFView = FSceneView;
using ICustomTemporalAAHistory = UE::Renderer::Private::ITemporalUpscaler::IHistory;
#else
using IANFTemporalUpscaler = ITemporalUpscaler;
using ANFPassInput = ITemporalUpscaler::FPassInputs;
using ANFPassOutput = ITemporalUpscaler::FOutputs;
using ANFView = FViewInfo;
#endif

typedef enum ANFMsgtype
{
	ANF_MESSAGE_TYPE_ERROR = 0,
	ANF_MESSAGE_TYPE_WARNING = 1,
	ANF_MESSAGE_TYPE_COUNT
} ANFMsgtype;

class FANFTU final : public IANFTemporalUpscaler
{
	friend class FANFFXSystem;

public:
	FANFTU();
	/*FANFTU(IANFTemporalUpscaler* TU);*/
	virtual ~FANFTU();

	const TCHAR* GetDebugName() const override;
	void Releasestate(ANFstateRef state);


	ANFPassOutput AddPasses(
		FRDGBuilder& GraphBuilder,
		const ANFView& View,
		const ANFPassInput& PassInputs) const override;

	float GetMinUpsampleResolutionFraction() const override;
	float GetMaxUpsampleResolutionFraction() const override;

#if ENGINE_MINOR_VERSION > 0
	IANFTemporalUpscaler* Fork_GameThread(const class FSceneViewFamily& InViewFamily) const override;
#endif
	void EndofFrame();

	FVector2D GetANFInputResolution();
	FVector2D GetANFOutputResolution();
	float GetANFScreenPercentage();

	bool IsANFSRSupported();

private:
	void Cleanup() const;
};

class FANFTUFork final : public IANFTemporalUpscaler 
{
public:
	FANFTUFork(IANFTemporalUpscaler* TU);
	virtual ~FANFTUFork();

	const TCHAR* GetDebugName() const override;

	IANFTemporalUpscaler::FOutputs AddPasses(
		FRDGBuilder& GraphBuilder,
		const ANFView& View,
		const ANFPassInput& PassInputs) const override;

	float GetMinUpsampleResolutionFraction() const override;
	float GetMaxUpsampleResolutionFraction() const override;

#if ENGINE_MINOR_VERSION > 0
	IANFTemporalUpscaler* Fork_GameThread(const class FSceneViewFamily& InViewFamily) const override;
#endif

private:
	IANFTemporalUpscaler* TemporalUpscaler;
};

class FANFExecManager : public FSelfRegisteringExec
{
public:
	inline static FANFExecManager& Get()
	{
		return s_instance;
	}

	inline bool GetCurrentState() const
	{
		return m_currentState;
	}

private:
	FANFExecManager();
	~FANFExecManager() {};

	/** FSelfRegisteringExec implementation */
#if ENGINE_MINOR_VERSION > 2
	virtual bool Exec_Runtime(UWorld* Inworld, const TCHAR* Cmd, FOutputDevice& Ar) override;
#else
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
#endif
	/** ~FSelfRegisteringExec implementation */


	static FANFExecManager s_instance;

	bool m_currentState;

};

