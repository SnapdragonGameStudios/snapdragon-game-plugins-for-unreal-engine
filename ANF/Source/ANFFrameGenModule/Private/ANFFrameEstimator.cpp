//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFFrameEstimator.h"
#include "ANFFrameGen.h"

#include "ANFRHI.h"
#include "ANFBackend.h"
#include "ANFConfig.h"

#include "ANFPreProcessor.h"
#include "ANFFrameDumper.h"
#include "ANFPresenter.h"

#include "SceneRendering.h"
#include "SceneTextures.h"

#include "Rendering/SlateRenderer.h"
#include "SlateRHIRenderer.h"
#include "PostProcess/PostProcessMaterial.h"
#if ENGINE_MINOR_VERSION > 1
#include <DataDrivenShaderPlatformInfo.h>
#endif
DEFINE_LOG_CATEGORY(LogANFFrameEstimator);

BEGIN_SHADER_PARAMETER_STRUCT(FCopyHistoryTextureParameters, )
    RDG_TEXTURE_ACCESS(InputColor, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(InputDepth, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(InputMotion, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(OutputColor, ERHIAccess::CopyDest)
    RDG_TEXTURE_ACCESS(OutputDepth, ERHIAccess::CopyDest)
    RDG_TEXTURE_ACCESS(OutputMotion, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FCopyDepthParameters, )
    RDG_TEXTURE_ACCESS(InputDepth, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(OutputDepth, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FDebugResourceDumpParams, )
    RDG_TEXTURE_ACCESS(InputColor, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(InputDepth, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(InputMotion, ERHIAccess::CopySrc)
    RDG_TEXTURE_ACCESS(OutputColor, ERHIAccess::CopySrc)
END_SHADER_PARAMETER_STRUCT()

struct FCopyHistoryTextureArgs
{
    FRDGTextureRef InputColor;
    FRDGTextureRef InputDepth;
    FRDGTextureRef InputMotion;
    FRDGTextureRef OutputColor;
    FRDGTextureRef OutputDepth;
    FRDGTextureRef OutputMotion;
};

#if ENGINE_MINOR_VERSION == 0
static void TransitionAndCopyTexture(FRHICommandList& RHICmdList, FRHITexture* Source, FRHITexture* Destination, const FRHICopyTextureInfo& CopyInfo)
{
    FRHITransitionInfo TransitionsBefore[] = {
        FRHITransitionInfo(Source, ERHIAccess::Unknown, ERHIAccess::CopySrc),
        FRHITransitionInfo(Destination, ERHIAccess::Unknown, ERHIAccess::CopyDest)
    };

    RHICmdList.Transition(MakeArrayView(TransitionsBefore, UE_ARRAY_COUNT(TransitionsBefore)));

    RHICmdList.CopyTexture(Source, Destination, CopyInfo);

    FRHITransitionInfo TransitionsAfter[] = {
        FRHITransitionInfo(Source, ERHIAccess::CopySrc, ERHIAccess::SRVMask),
        FRHITransitionInfo(Destination, ERHIAccess::CopyDest, ERHIAccess::SRVMask)
    };

    RHICmdList.Transition(MakeArrayView(TransitionsAfter, UE_ARRAY_COUNT(TransitionsAfter)));
}
#endif

inline bool CheckEnabled()
{
    return FANFFrameGenModule::IsFrameEstimationEnabled();
}

inline bool CheckSupported()
{
    return ANFBackendInterface::Get()->IsFgTechniqueSupported();
}

FANFFrameGenViewExtension::FANFFrameGenViewExtension(const FAutoRegister& AutoRegister)
    : FSceneViewExtensionBase(AutoRegister)
    , MainState(FrameEstimatorMainState::MAIN_STATE_START)
    , m_initialized(false)
    , m_validPrevMotion(false)
    , m_anfBacked(nullptr)
    , m_lastInputFrames(0)
    , m_customPresentSet(false)
    , m_drawWindowState(DrawWindowState::DrawWindowState_None)
{
    m_renderSize = FIntVector2(0, 0);
    m_displaySize = FIntVector2(0, 0);
    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FANFFrameGenViewExtension::OnPostEngineInit);
    OnPostEngineInit();
}

void FANFFrameGenViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
    if (PassId == EPostProcessingPass::VisualizeDepthOfField)
    {
        InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FANFFrameGenViewExtension::AfterPostProcessing_RenderThread));
    }
}

void FANFFrameGenViewExtension::OnPostEngineInit()
{
    if (FSlateApplication::IsInitialized())
    {
        // Hook the Slate Present path supplied by the ANF engine patches.
        FSlateApplication& App = FSlateApplication::Get();
        FSlateRenderer* SlateRenderer = App.GetRenderer();

#if ANF_HAS_DRAW_WINDOWS_DELEGATES
        SlateRenderer->OnDrawWindows_RenderThread().AddRaw(this, &FANFFrameGenViewExtension::OnDrawWindowsRenderThread);
        SlateRenderer->OnDrawWindow_RenderThread().AddRaw(this, &FANFFrameGenViewExtension::OnDrawWindowRenderThread);
#else
        SlateRenderer->OnDrawUI().AddRaw(this, &FANFFrameGenViewExtension::OnDrawUI);
#endif // ANF_HAS_DRAW_WINDOWS_DELEGATES

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
        SlateRenderer->OnBackBufferReadyToPresent().AddRaw(this, &FANFFrameGenViewExtension::OnBackBufferReadyToPresent);
#endif

        UpdateMainState(FrameEstimatorMainState::MAIN_STATE_BLIT);
    }
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
void FANFFrameGenViewExtension::OnBackBufferReadyToPresent(SWindow& Window, ISlateViewportProvider& ViewportProvider)
{
    if (GEngine->GameViewport &&
        GEngine->GameViewport->Viewport == Window.GetViewport().Get() && 
        GANFFrameGenEnabled != 0 &&
        GANFFrameGenUseFramePacer != 0 &&
        !m_customPresentSet)
    {
        SetCustomPresent(ViewportProvider);
    }
}
#endif

void FANFFrameGenViewExtension::SetCustomPresent(CustomPresentSetType ViewportRHI)
{
    if (GANFFrameGenEnabled == 0 || GANFFrameGenUseFramePacer == 0 || m_customPresentSet)
    {
        return;
    }

    {
        static auto CVarUseSwappy = IConsoleManager::Get().FindConsoleVariable(TEXT("a.UseSwappyForFramePacing"));
        const bool swappyEnabled = CVarUseSwappy != nullptr && CVarUseSwappy->GetInt() != 0;

        if (swappyEnabled)
        {
            UE_LOG(LogANFFrameEstimator, Warning, TEXT("SwappyVK (a.UseSwappyForFramePacing) is enabled alongside ANF frame pacer - this will liekly impact performance"));
        }

        static auto CVarUseVkExtension = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Vulkan.ExtensionFramePacer"));
        const bool vkExtensionExnabled = CVarUseVkExtension != nullptr && CVarUseVkExtension->GetInt() != 0;

        if (vkExtensionExnabled)
        {
            UE_LOG(LogANFFrameEstimator, Warning, TEXT("Google display timing extension (r.Vulkan.ExtensionFramePacer) is enabled alongside ANF frame pacer - this will liekly impact performance"));
        }

        static auto CVarCPUPacer = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Vulkan.CPURenderthreadFramePacer"));
        const bool cpuPacerEnabled = CVarCPUPacer != nullptr && CVarCPUPacer->GetInt() != 0;

        if (cpuPacerEnabled)
        {
            UE_LOG(LogANFFrameEstimator, Warning, TEXT("CPU frame pacing (r.Vulkan.CPURenderthreadFramePacer) is enabled alongside ANF frame pacer - this will liekly impact performance"));
        }
    }

    if (!m_presenter.IsValid())
    {
        m_presenter = TSharedPtr<ANFPresenter>(new ANFPresenter());
    }

    UE_LOG(LogANFFrameEstimator, Verbose, TEXT("Setting Custom present for ANF FrameGen"));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 7
    ViewportRHI.SetCustomPresent(m_presenter.Get());
#else
    ViewportRHI->SetCustomPresent(m_presenter.Get());
#endif
    m_customPresentSet = true;
}

void FANFFrameGenViewExtension::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
    if (GANFFrameGenEnabled)
    {
        const uint64_t currentFrame = GFrameCounter;
        if (currentFrame != m_lastInputFrames)
        {
            SetInputs(GraphBuilder, InView, nullptr);
        }
    }
}

FScreenPassTexture FANFFrameGenViewExtension::AfterPostProcessing_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
    if (GANFFrameGenEnabled)
    {
        SetInputs(GraphBuilder, View, &InOutInputs);
    }
#if ENGINE_MINOR_VERSION > 3
    return InOutInputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
#else
    const FScreenPassTexture SceneColor = InOutInputs.GetInput(EPostProcessMaterialInput::SceneColor);
    const FScreenPassRenderTarget& OverrideOutput = InOutInputs.OverrideOutput;

    if (OverrideOutput.IsValid())
    {
        const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
        AddDrawTexturePass(GraphBuilder, ViewInfo, SceneColor.Texture, OverrideOutput.Texture);
        return FScreenPassTexture(OverrideOutput);
    }

    return SceneColor;
#endif
}

inline FRDGTextureRef ANFFrameGenCreateViewFamilyTexture(FRDGBuilder& GraphBuilder, const FSceneViewFamily* ViewFamily)
{
    FRHITexture* TextureRHI = ViewFamily->RenderTarget->GetRenderTargetTexture();
    FRDGTextureRef Texture = nullptr;
    if (TextureRHI)
    {
        Texture = RegisterExternalTexture(GraphBuilder, TextureRHI, TEXT("ViewFamilyTexture"));
        GraphBuilder.SetTextureAccessFinal(Texture, ERHIAccess::SRVCompute);
    }
    return Texture;
}

void FANFFrameGenViewExtension::SetInputs(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs* pInOutInputs)
{
    bool enabled = CheckEnabled();
    bool supported = CheckSupported();

    if (enabled && !supported)
    {
        UE_LOG(LogANFFrameEstimator, Error, TEXT("ANF FrameGen is enabled but not supported on this device; disabling."));
        return;
    }

    if (!enabled || !supported)
    {
        return;
    }

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 7
    if (enabled && GANFFrameGenUseFramePacer != 0 && !m_customPresentSet)
    {
        SetCustomPresent(GEngine->GameViewport->Viewport->GetViewportRHI());
    }
#endif

    m_lastInputFrames = GFrameCounter;

    const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);

    StateFlags &= ~INPUT_INVALID;

#if ENGINE_MINOR_VERSION > 0
    FSceneTextures SceneTextures = ViewInfo.GetSceneTextures();
#else
    const FSceneTextures& SceneTextures = FSceneTextures::Get(GraphBuilder);
#endif

#if ENGINE_MINOR_VERSION > 3
    FRDGTextureRef SceneColorTexture = pInOutInputs ? pInOutInputs->GetInput(EPostProcessMaterialInput::SceneColor).TextureSRV->GetParent() : nullptr;
#else 
    FRDGTextureRef SceneColorTexture = pInOutInputs ? pInOutInputs->GetInput(EPostProcessMaterialInput::SceneColor).Texture : nullptr;
#endif
    if (!SceneColorTexture)
    {
        SceneColorTexture = ANFFrameGenCreateViewFamilyTexture(GraphBuilder, View.Family);
    }
#if ENGINE_MINOR_VERSION > 3
    FRDGTextureRef SceneVelocityTexture = pInOutInputs ? pInOutInputs->GetInput(EPostProcessMaterialInput::Velocity).TextureSRV->GetParent() : nullptr;
#else
    FRDGTextureRef SceneVelocityTexture = pInOutInputs ? pInOutInputs->GetInput(EPostProcessMaterialInput::Velocity).Texture : nullptr;
#endif

    if (!SceneVelocityTexture)
    {
#if ENGINE_MINOR_VERSION > 0
        SceneVelocityTexture = SceneTextures.Velocity;
#else 
        SceneVelocityTexture = SceneTextures.Velocity;
#endif
    }

#if ENGINE_MINOR_VERSION > 0
    FRDGTextureRef SceneDepthTexture = SceneTextures.Depth.Resolve;
#else
    FRDGTextureRef SceneDepthTexture = SceneTextures.Depth.Resolve;
#endif

    if (SceneColorTexture == nullptr ||
        SceneDepthTexture == nullptr ||
        SceneVelocityTexture == nullptr)
    {
        UE_LOG(LogANFFrameEstimator, Error, TEXT("FANFFrameEstimator SetInputs inputs null"));
        StateFlags |= INPUT_INVALID;
        return;
    }

    const auto& inputColorSize = SceneColorTexture->Desc.GetSize();
    const auto& inputDepthSize = SceneDepthTexture->Desc.GetSize();
    const auto& inputVelocitySize = SceneVelocityTexture->Desc.GetSize();
    const FIntVector2 forcedInputSize = { GANFInWidth, GANFInHeight };

    int32_t srcWidth = inputDepthSize.X;
    int32_t srcHeight = inputDepthSize.Y;

    int32_t dstWidth = inputColorSize.X;
    int32_t dstHeight = inputColorSize.Y;

    if (inputColorSize.X == 0 || inputColorSize.Y == 0)
    {
        UE_LOG(LogANFFrameEstimator, Error, TEXT("FANFFrameEstimator SetInputs inputColorSize invalid"));
        StateFlags |= INPUT_INVALID;
        return;
    }

    if (inputColorSize.X <= GANFMinWidth || inputColorSize.Y <= GANFMinHeight)
    {
        UE_LOG(LogANFFrameEstimator, Warning, TEXT("FANFFrameEstimator SetInputs inputColorSize (%dx%d) is less than minimum (%dx%d))"),
            inputColorSize.X, inputColorSize.Y,
            GANFMinWidth, GANFMinHeight);
        StateFlags |= INPUT_INVALID;
        return;
    }

    /* Velocity */
    bool isTheInputMotionValid = (inputVelocitySize == FIntVector(1, 1, 1)) ? false : true;
    isTheInputMotionValid &= (SceneVelocityTexture->HasBeenProduced() || SceneVelocityTexture->IsExternal());
    if (isTheInputMotionValid)
    {
        /* Depth and motion inputs must have the same dimensions. */
        if (inputDepthSize.X != inputVelocitySize.X ||
            inputDepthSize.Y != inputVelocitySize.Y)
        {
            UE_LOG(LogANFFrameEstimator, Error, TEXT("FANFFrameEstimator SetInputs Depth & Motion size not match"));
            StateFlags |= INPUT_INVALID;
            return;
        }
    }

    m_displaySize = FIntVector2(dstWidth, dstHeight);
    m_renderSize = FIntVector2(srcWidth, srcHeight);

    FCopyHistoryTextureArgs args = {};
    args.InputColor = SceneColorTexture;
    args.InputDepth = SceneDepthTexture;
    args.InputMotion = SceneVelocityTexture;
    args.OutputColor = nullptr;
    args.OutputDepth = nullptr;
    args.OutputMotion = nullptr;

    m_jitter.Z = m_jitter.X;
    m_jitter.W = m_jitter.Y;

    m_jitter.X = ViewInfo.TemporalJitterPixels.X;
    m_jitter.Y = ViewInfo.TemporalJitterPixels.Y;

    FANFPreProcessor::ANFPreProcessorOutputs preprocOutputs = FANFPreProcessor::Get()->Execute(
        ViewInfo,
        GraphBuilder,
        args.InputColor,
        args.InputDepth,
        args.InputMotion,
        m_renderSize,
        m_displaySize,
        m_jitter,
        GFrameNumber,
        true);

    args.OutputColor = preprocOutputs.color;
    args.OutputDepth = preprocOutputs.depth;
    args.OutputMotion = preprocOutputs.motion;

    m_clipToPrevClip = preprocOutputs.clipToPrevClip;
    
    check(args.OutputColor);
    check(args.OutputDepth);
    check(args.OutputMotion);

    InputColorTexture = GraphBuilder.ConvertToExternalTexture(args.OutputColor);
    InputDepthTexture = GraphBuilder.ConvertToExternalTexture(args.OutputDepth);
    InputMotionTexture = GraphBuilder.ConvertToExternalTexture(args.OutputMotion);

    if (GANFFrameGenAddFinalBlit != 0)
    {
        FRHITextureCreateDesc outputDesc =
            FRHITextureCreateDesc::Create2D(TEXT("ANF.FG.OutputTexture.RHI"), m_displaySize.X, m_displaySize.Y, PF_R8G8B8A8)
            .SetFlags(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV | ETextureCreateFlags::RenderTargetable | ANFRHI::GetRHI()->GetExtraFlags_ColorOutput(true));

        FRHITexture* outputTextureRHI = RHICreateTexture(outputDesc);
        FRDGTexture* outputTextureRGD = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(outputTextureRHI, TEXT("ANF.FG.OutputTexture.RDG")));
        OutputColorTexture = GraphBuilder.ConvertToExternalTexture(outputTextureRGD);
    }

    if (GANFFrameGenSubmissionOrder == 1)
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
        const FIntVector2 inputSize =
        { InputColorTexture->GetDesc().Extent.X, InputColorTexture->GetDesc().Extent.Y };

        const FIntVector2 currentSize = !SavedRenderedFrame.IsValid() ? FIntVector2(0, 0) :
            FIntVector2(SavedRenderedFrame->GetDesc().Extent.X, SavedRenderedFrame->GetDesc().Extent.Y);
#else
        const FIntVector2 inputSize =
        { InputColorTexture->GetSizeXYZ().X, InputColorTexture->GetSizeXYZ().Y };

        const FIntVector2 currentSize = !SavedRenderedFrame.IsValid() ? FIntVector2(0, 0) :
            FIntVector2(SavedRenderedFrame->GetSizeXYZ().X, SavedRenderedFrame->GetSizeXYZ().Y);
#endif

        if ((!SavedRenderedFrame.IsValid() || (currentSize != inputSize)))
        {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
            FRHITextureCreateDesc outputDesc =
                FRHITextureCreateDesc::Create2D(TEXT("ANF.FG.StoredFrame"), 
                    inputSize.X, inputSize.Y, 
                    InputColorTexture->GetDesc().Format)
                .SetFlags(ETextureCreateFlags::ShaderResource);

            SavedRenderedFrame = RHICreateTexture(outputDesc);
#else
            FRHIResourceCreateInfo CreateInfo = FRHIResourceCreateInfo(TEXT("ANF.FG.StoredFrame"));

            SavedRenderedFrame = RHICreateTexture2D(inputSize.X, inputSize.Y
                , InputColorTexture->GetFormat(), 1, 1, ETextureCreateFlags::ShaderResource, CreateInfo);

#endif
        }
    }

    if (!m_initialized || m_anfBacked->FgNeedsReInit(m_renderSize, m_displaySize))
    {
        m_anfBacked = ANFBackendInterface::Get();

        check(m_anfBacked);
        if (!m_anfBacked->InitializeANF_FG(m_renderSize, m_displaySize))
        {
            UE_LOG(LogRHI, Error, TEXT("Failed to initialize ANF FrameGen!!!"));
        }

        m_initialized = true;
    }

    bool motionReset = (!m_validPrevMotion || !isTheInputMotionValid);

    if ((View.bCameraCut || motionReset) && m_anfBacked)
    {
        m_needsReset = true;
    }

    /* mark content ready(valid) */
    UpdateMainState(FrameEstimatorMainState::MAIN_STATE_ESTIMATE);
    /* end copy c/d/v */

    m_validPrevMotion = isTheInputMotionValid;

    return;
}

// Present hook supplied by the ANF engine patches.
#if ANF_HAS_DRAW_WINDOWS_DELEGATES

void FANFFrameGenViewExtension::OnDrawWindowsRenderThread(FSlateRenderer::FDrawWindowsRenderThread fxDrawWindows)
{
    if (!GANFFrameGenEnabled)
    {
        return;
    }
    UpdateStateFlags();

    m_drawWindowState = DrawWindowState::DrawWindowState_Execute;
    fxDrawWindows();
    m_drawWindowState = DrawWindowState::DrawWindowState_Blit;
}

void FANFFrameGenViewExtension::OnDrawWindowRenderThread(FRDGBuilder& GraphBuilder, FRHICommandListImmediate& RHICmdList, FTextureRHIRef& OutputTexture)
{
    if (!GANFFrameGenEnabled)
    {
        return;
    }

    switch (m_drawWindowState)
    {
    case DrawWindowState::DrawWindowState_Execute:
        if (GANFFrameGenSubmissionOrder == 1)
        {
            PopulateHistoryColor(RHICmdList, OutputTexture, 1);
            if (InputColorTexture.IsValid() && SavedRenderedFrame.IsValid())
            {
                TransitionAndCopyTexture(RHICmdList, InputColorTexture->GetRHI(), SavedRenderedFrame, {});
            }
        }
        else
        {
            RHICmdList.Transition(FRHITransitionInfo(OutputTexture, ERHIAccess::Unknown, ERHIAccess::CopyDest));
            ExecuteFrameGen(RHICmdList, OutputTexture);
        }
        RHICmdList.Transition(FRHITransitionInfo(OutputTexture, ERHIAccess::Unknown, ERHIAccess::RTV));
        m_drawWindowState = DrawWindowState::DrawWindowState_Blit;
        break;
    case DrawWindowState::DrawWindowState_Blit:
        if (GANFFrameGenSubmissionOrder == 1)
        {
            RHICmdList.Transition(FRHITransitionInfo(OutputTexture, ERHIAccess::Unknown, ERHIAccess::CopyDest));
            ExecuteFrameGen(RHICmdList, OutputTexture.GetReference());
        }
        else
        {
            PopulateHistoryColor(RHICmdList, OutputTexture, 0);
        }
        RHICmdList.Transition(FRHITransitionInfo(OutputTexture, ERHIAccess::Unknown, ERHIAccess::RTV));
        m_drawWindowState = DrawWindowState::DrawWindowState_None; // Reset state to ensure we are fresh for next frame
        break;
    case DrawWindowState::DrawWindowState_None:
    default:
        UE_LOG(LogANFFrameEstimator, Fatal, TEXT("Invalid Draw window state!  Something has gone wrong, check plugin logic!"));
        break;
    }
}
#else

void FANFFrameGenViewExtension::OnDrawUI(FSlateRenderer::DrawUiOnBackbufferFx drawUiOnBackbuffer, FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef& SlateRenderTarget, FTexture2DRHIRef& PostProcessBuffer, FViewportInfo& ViewportInfo, bool bRenderedStereo, bool bLockToVSync)
{
    if (!GANFFrameGenEnabled)
    {
        return;
    }
    UpdateStateFlags();

    if (!CanEstimate() || !GANFFrameGenEnabled)
    {
        return;
    }

    if (GANFFrameGenSubmissionOrder == 1)
    {
        PopulateHistoryColor(RHICmdList, SlateRenderTarget.GetReference(), 1);
        TransitionAndCopyTexture(RHICmdList, InputColorTexture->GetRHI(), SavedRenderedFrame, {});
    }
    else
    {
        ExecuteFrameGen(RHICmdList, SlateRenderTarget.GetReference());
    }
    RHICmdList.Transition(FRHITransitionInfo(SlateRenderTarget, ERHIAccess::Unknown, ERHIAccess::RTV));
    drawUiOnBackbuffer();

    /* swap the buffers */
    RHICmdList.EndDrawingViewport(ViewportInfo.ViewportRHI, true, bLockToVSync);

    /* now a new frame start */
    FTexture2DRHIRef ViewportRT = bRenderedStereo ? nullptr : ViewportInfo.GetRenderTargetTexture();
    FTexture2DRHIRef BackBuffer = (ViewportRT) ? ViewportRT : RHIGetViewportBackBuffer(ViewportInfo.ViewportRHI);
    PostProcessBuffer = BackBuffer;

    const uint32 ViewportWidth = (ViewportRT) ? ViewportRT->GetSizeX() : ViewportInfo.Width;
    const uint32 ViewportHeight = (ViewportRT) ? ViewportRT->GetSizeY() : ViewportInfo.Height;

    RHICmdList.BeginDrawingViewport(ViewportInfo.ViewportRHI, FTextureRHIRef());
    RHICmdList.SetViewport(0, 0, 0, ViewportWidth, ViewportHeight, 0.0f);

    // When GVulkanDelayAcquireImage is LazyAcquire,
    // backbuffer image is acquired when texture transition to a writable layout(for example: RTV)
    // Add this transition to avoid crash for image being null during backbuffer image transition
    RHICmdList.Transition(FRHITransitionInfo(BackBuffer, ERHIAccess::Unknown, ERHIAccess::RTV));

    if (GANFFrameGenSubmissionOrder == 1)
    {
        RHICmdList.Transition(FRHITransitionInfo(BackBuffer, ERHIAccess::Unknown, ERHIAccess::CopyDest));
        ExecuteFrameGen(RHICmdList, BackBuffer.GetReference());
    }
    else
    {
        /* populate last color to backbuffer */
        PopulateHistoryColor(RHICmdList, BackBuffer.GetReference(), 0);
    }

    RHICmdList.Transition(FRHITransitionInfo(BackBuffer, ERHIAccess::SRVMask, ERHIAccess::RTV));
}

#endif // ANF_HAS_DRAW_WINDOWS_DELEGATES

void FANFFrameGenViewExtension::PopulateHistoryColor(FRHICommandListImmediate& RHICmdList, FRHITexture* OutSceneColorTexture, int Age)
{
    if (!CheckEnabled())
    {
        return;
    }

    if (InputColorTexture == nullptr)
    {
        return;
    }

    FRHITexture* sourceTexture = (Age == 0 ? InputColorTexture->GetRHI() : SavedRenderedFrame.GetReference());

#if ENGINE_MINOR_VERSION > 0
    if ((sourceTexture->GetSizeX() == OutSceneColorTexture->GetSizeX()) &&
        (sourceTexture->GetSizeY() == OutSceneColorTexture->GetSizeY()) &&
        (sourceTexture->GetFormat() == OutSceneColorTexture->GetFormat()))
#else
    if ((sourceTexture->GetSizeXYZ().X == OutSceneColorTexture->GetSizeXYZ().X) &&
        (sourceTexture->GetSizeXYZ().Y == OutSceneColorTexture->GetSizeXYZ().Y) &&
        (sourceTexture->GetFormat() == OutSceneColorTexture->GetFormat()))
#endif
    {
        TransitionAndCopyTexture(RHICmdList, sourceTexture, OutSceneColorTexture, {});
    }
    else
    {
        RHICmdList.Transition({
            FRHITransitionInfo(sourceTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc),
            FRHITransitionInfo(OutSceneColorTexture, ERHIAccess::Unknown, ERHIAccess::CopyDest)
            });

        ANFRHI::GetRHI()->RHIAddBlitTexturePass(RHICmdList, sourceTexture, OutSceneColorTexture, true);

        RHICmdList.Transition({
            FRHITransitionInfo(sourceTexture, ERHIAccess::CopySrc,  ERHIAccess::SRVMask),
            FRHITransitionInfo(OutSceneColorTexture, ERHIAccess::CopyDest, ERHIAccess::SRVMask)
            });
    }
}

bool FANFFrameGenViewExtension::CanEstimate() const
{
    if (!CheckEnabled())
    {
        return false;
    }

    if (!CheckSupported())
    {
        return false;
    }

    /* plugin StateFlags ready && no error && libState ready */
    return (((MainState == MAIN_STATE_ESTIMATE) || (MainState == MAIN_STATE_BLIT)) && ((StateFlags & RESOURCE_READY) == RESOURCE_READY));
}

void FANFFrameGenViewExtension::ExecuteFrameGen(FRHICommandListImmediate& RHICmdList, FRHITexture* OutSceneColorTexture)
{
    if (!CheckEnabled())
    {
        return;
    }

    /* handle invalid input first, once input invalid, frame estimation should stop */
    if (StateFlags & (INPUT_INVALID | LIB_INTERNAL_ERROR))
    {
        UpdateMainState(FrameEstimatorMainState::MAIN_STATE_STOP);
        return;
    }

    /* handle resolution change */
    if (StateFlags & RESOLUTION_DIRTY)
    {
        if (m_anfBacked)
        {
            m_needsReset = true;
        }
        StateFlags = (StateFlags & ~RESOLUTION_DIRTY);
    }

    /* check data of two frames, if ready and resolution not dirty, can start estimate */
    if ((StateFlags & RESOURCE_READY))
    {
        if (MainState != FrameEstimatorMainState::MAIN_STATE_ESTIMATE)
        {
            UpdateMainState(FrameEstimatorMainState::MAIN_STATE_ESTIMATE);
        }
    }
    else
    {
        if (MainState != FrameEstimatorMainState::MAIN_STATE_BLIT)
        {
            UpdateMainState(FrameEstimatorMainState::MAIN_STATE_BLIT);
        }
    }

    if (MainState == FrameEstimatorMainState::MAIN_STATE_ESTIMATE)
    {
        uint32_t frameNum = GFrameNumber;

        /* state control */
        check(m_anfBacked);

        FRHITexture* anfOutputTexture = GANFFrameGenAddFinalBlit == 0 ? OutSceneColorTexture : OutputColorTexture->GetRHI();

        ANFBackendInterface::ANFExecutionData fgData = {};
        fgData.frameNumber = frameNum;
        fgData.reset = m_needsReset;
        fgData.jitterOffset = FVector2f(m_jitter.X, m_jitter.Y);
        fgData.inputColor = InputColorTexture->GetRHI();
        fgData.inputDepth = InputDepthTexture->GetRHI();
        fgData.inputMotion = InputMotionTexture->GetRHI();
        fgData.outputColor = anfOutputTexture;

#if ENGINE_MINOR_VERSION == 0
        {
            FRHITransitionInfo InputTransitions[] = {
                FRHITransitionInfo(fgData.inputColor,  ERHIAccess::CopySrc, ERHIAccess::SRVGraphics),
                FRHITransitionInfo(fgData.inputDepth,  ERHIAccess::CopySrc, ERHIAccess::SRVGraphics),
                FRHITransitionInfo(fgData.inputMotion, ERHIAccess::CopySrc, ERHIAccess::SRVGraphics),
            };
            RHICmdList.Transition(MakeArrayView(InputTransitions, UE_ARRAY_COUNT(InputTransitions)));
        }
#endif
        m_anfBacked->ExecuteANF_FG(RHICmdList, fgData);
        if (GANFFrameGenAddFinalBlit != 0)
        {
            RHICmdList.Transition({
                FRHITransitionInfo(anfOutputTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc),
                FRHITransitionInfo(OutSceneColorTexture, ERHIAccess::Unknown, ERHIAccess::CopyDest)
                });

            ANFRHI::GetRHI()->RHIAddBlitTexturePass(RHICmdList, anfOutputTexture, OutSceneColorTexture, true);

            RHICmdList.Transition({
                FRHITransitionInfo(anfOutputTexture, ERHIAccess::CopySrc,  ERHIAccess::SRVMask),
                FRHITransitionInfo(OutSceneColorTexture, ERHIAccess::CopyDest, ERHIAccess::SRVMask)
                });
        }

        m_needsReset = false;

        // Dump Frames
        if (ShouldDumpANFFrames(frameNum))
        {
            FANFFrameDumper::Execute(
                RHICmdList,
                GFrameNumber,
                InputColorTexture->GetRHI(),
                InputDepthTexture->GetRHI(),
                InputMotionTexture->GetRHI(),
                OutSceneColorTexture,
                m_jitter,
                m_clipToPrevClip,
                TEXT("FG"));
        }
    }
}

void FANFFrameGenViewExtension::UpdateStateFlags()
{
    if (!CheckEnabled())
    {
        return;
    }

    if (InputColorTexture != nullptr && InputDepthTexture != nullptr && InputMotionTexture != nullptr)
    {
        StateFlags |= RESOURCE_READY;
    }
    else
    {
        StateFlags &= ~RESOURCE_READY;
    }
}


void FANFFrameGenViewExtension::UpdateMainState(FrameEstimatorMainState state)
{
    MainState = state;
}