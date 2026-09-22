//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once

#include "SceneViewExtension.h"

class ANFTUMODULE_API FANFViewExtension final : public FSceneViewExtensionBase
{
public:
	FANFViewExtension(const FAutoRegister& AutoRegister);

	void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}

	void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;
	void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;
	void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;

private:
	int32 PreviousANFState;
};

