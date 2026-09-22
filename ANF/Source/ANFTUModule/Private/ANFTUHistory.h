//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "CoreMinimal.h"
#include "SceneRendering.h"
#include "HAL/Platform.h"

#if ENGINE_MAJOR_VERSION == 5

#if ENGINE_MINOR_VERSION >= 8
typedef void AddRefRetType;
typedef FReturnedRefCountValue HistoryFxReturnType;
class FANFTUHistory;
typedef TRefCountingMixin<FANFTUHistory> THistoryParent;
#define HISTORY_FX_PREFIX virtual
#define HISTORY_FX_SUFFIX override
#elif ENGINE_MINOR_VERSION <= 5
typedef uint32 AddRefRetType;
typedef uint32 HistoryFxReturnType;
typedef FRefCountBase THistoryParent;
#define HISTORY_FX_PREFIX
#define HISTORY_FX_SUFFIX final
#else
typedef FRefCountBase THistoryParent;
typedef FReturnedRefCountValue AddRefRetType;
typedef uint32 HistoryFxReturnType;
#define HISTORY_FX_PREFIX
#define HISTORY_FX_SUFFIX final
#endif

#else
typedef FRefCountBase THistoryParent;
typedef uint32 AddRefRetType;
typedef uint32 HistoryFxReturnType;
#define HISTORY_FX_PREFIX
#define HISTORY_FX_SUFFIX final
#endif

class FANFTU;

//////ANF state, deletion handled by RHI
struct FANFState : public FRHIResource
{
	FANFState()
		: FRHIResource(RRT_None)
		, LastUsedFrame(~0u)
	{
	}
	~FANFState()
	{
	}

	uint32 AddRef() const
	{
		return FRHIResource::AddRef();
	}

	uint32 Release() const
	{
		return FRHIResource::Release();
	}

	uint32 GetRefCount() const
	{
		return FRHIResource::GetRefCount();
	}

	uint64 LastUsedFrame;
	uint32 ViewID;
};
typedef TRefCountPtr<FANFState> ANFstateRef;

#if ENGINE_MINOR_VERSION > 2
using ICustomTemporalAAHistory = UE::Renderer::Private::ITemporalUpscaler::IHistory;
#endif

/////ICustomTemporalAAHistory for ANF
class FANFTUHistory final : public ICustomTemporalAAHistory, public THistoryParent
{
public:
	FANFTUHistory(ANFstateRef state, FANFTU* upscaler);
	virtual ~FANFTUHistory();
#if ENGINE_MINOR_VERSION > 2
	virtual const TCHAR* GetDebugName() const override;
	virtual uint64 GetGPUSizeBytes() const override;
#endif

	//////originally in .cpp
	void Setstate(ANFstateRef state);

	inline ANFstateRef const& Getstate() const
	{
		return ANF;
	}

	HISTORY_FX_PREFIX AddRefRetType AddRef() const HISTORY_FX_SUFFIX
	{
		return THistoryParent::AddRef();
	}

	HISTORY_FX_PREFIX HistoryFxReturnType Release() const HISTORY_FX_SUFFIX
	{
		return THistoryParent::Release();
	}

	HISTORY_FX_PREFIX HistoryFxReturnType GetRefCount() const HISTORY_FX_SUFFIX
	{
		return THistoryParent::GetRefCount();
	}

private:
	ANFstateRef ANF;
	FANFTU* upscaler;
};