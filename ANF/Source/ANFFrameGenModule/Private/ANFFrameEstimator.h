//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once
#include "ANFBackend.h"

#include "Core.h"
#include "RendererInterface.h"
#include "Rendering/SlateRenderer.h"
#include "PostProcess/PostProcessMobile.h"
#include "SceneViewExtension.h"
#include "RenderTargetPool.h"

// Presentation uses the Slate delegates supplied by the ANF engine patches.

#if ENGINE_MAJOR_VERSION == 5

#if ENGINE_MINOR_VERSION <= 4
#define ANF_HAS_DRAW_WINDOWS_DELEGATES 0
#else
#define ANF_HAS_DRAW_WINDOWS_DELEGATES 1
#endif

#else
#define ANF_HAS_DRAW_WINDOWS_DELEGATES 0
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogANFFrameEstimator, Log, All);

class ANFPresenter;

class ANFFRAMEGENMODULE_API FANFFrameGenViewExtension final : public FSceneViewExtensionBase
{
public:
	FANFFrameGenViewExtension(const FAutoRegister& AutoRegister);

	void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}

	void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {}
	void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {}

	void SubscribeToPostProcessingPass(EPostProcessingPass PassId, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;
private:

	void OnPostEngineInit();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
	typedef ISlateViewportProvider& CustomPresentSetType;
	void OnBackBufferReadyToPresent(SWindow& Window, ISlateViewportProvider& ViewportProvider);
#else
	typedef FViewportRHIRef CustomPresentSetType;
#endif

	void SetCustomPresent(CustomPresentSetType ViewportRHI);

	FScreenPassTexture AfterPostProcessing_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs);
	void SetInputs(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs* pInOutInputs);

	// Present hook supplied by the ANF engine patches.
#if ANF_HAS_DRAW_WINDOWS_DELEGATES
	void OnDrawWindowsRenderThread(FSlateRenderer::FDrawWindowsRenderThread fxDrawWindows);
	void OnDrawWindowRenderThread(FRDGBuilder& GraphBuilder, FRHICommandListImmediate& RHICmdList, FTextureRHIRef& OutputTexture);
#else
	void OnDrawUI(FSlateRenderer::DrawUiOnBackbufferFx drawUiOnBackbuffer, FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef& SlateRenderTarget, FTexture2DRHIRef& PostProcessBuffer, FViewportInfo& ViewportInfo, bool bRenderedStereo, bool bLockToVSync);
#endif // ANF_HAS_DRAW_WINDOWS_DELEGATES

	void PopulateHistoryColor(FRHICommandListImmediate& RHICmdList, FRHITexture* OutSceneColorTexture, int Age);
	bool CanEstimate() const;
	void ExecuteFrameGen(FRHICommandListImmediate& RHICmdList, FRHITexture* OutSceneColorTexture);

	enum FrameEstimatorMainState
	{
		MAIN_STATE_START = 0,
		MAIN_STATE_BLIT,
		MAIN_STATE_ESTIMATE,
		MAIN_STATE_PAUSE,
		MAIN_STATE_STOP
	};


	enum ANFFrameGenStateFlag : uint32_t
	{
		RESOLUTION_DIRTY = 0x00000001,
		INPUT_INVALID = 0x00000002,
		RESOURCE_READY = 0x00000004,
		LIB_INTERNAL_ERROR = 0x00000008
	};

	void UpdateStateFlags();
	void UpdateMainState(FrameEstimatorMainState state);

	FrameEstimatorMainState MainState;

	TRefCountPtr<IPooledRenderTarget> InputColorTexture;
	TRefCountPtr<IPooledRenderTarget> SavedRawColorTexture;
	TRefCountPtr<IPooledRenderTarget> InputDepthTexture;
	TRefCountPtr<IPooledRenderTarget> InputMotionTexture;

	TRefCountPtr<IPooledRenderTarget> OutputColorTexture;

	TRefCountPtr<FRHITexture> SavedRenderedFrame;

	TSharedPtr<ANFPresenter> m_presenter;

	FIntVector2 m_renderSize;
	FIntVector2 m_displaySize;

	/// <summary>
	/// bit 0: resolution dirty
	/// bit 1: input invalid
	/// bit 2: resource ready
	/// </summary>
	uint32_t StateFlags = 0;

	FVector4f m_jitter;
	FMatrix44f m_clipToPrevClip;

	bool m_initialized;
	bool m_validPrevMotion;

	ANFBackendInterface* m_anfBacked;

	bool m_needsReset;

	uint64_t m_lastInputFrames;

	bool m_customPresentSet;

	enum DrawWindowState
	{
		DrawWindowState_None,
		DrawWindowState_Execute,
		DrawWindowState_Blit,
	};

	DrawWindowState m_drawWindowState;
};


extern RENDERCORE_API TGlobalResource<FRenderTargetPool> GRenderTargetPool;
