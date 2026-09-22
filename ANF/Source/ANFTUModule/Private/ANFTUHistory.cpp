//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFTUHistory.h"
#include "ANFTU.h"

FANFTUHistory::FANFTUHistory(ANFstateRef state, FANFTU* _upscaler)
{
	upscaler = _upscaler;
	Setstate(state);
}

FANFTUHistory::~FANFTUHistory()
{
	upscaler->Releasestate(ANF);
}

#if ENGINE_MINOR_VERSION > 2
const TCHAR* FANFTUHistory::GetDebugName() const
{
	check(upscaler);
	return upscaler->GetDebugName();
}

uint64 FANFTUHistory::GetGPUSizeBytes() const
{
	return 0;
}
#endif
void FANFTUHistory::Setstate(ANFstateRef state)
{
	upscaler->Releasestate(ANF);
	ANF = state;
}
