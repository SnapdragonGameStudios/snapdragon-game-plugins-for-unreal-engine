//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFBackend.h"
#include "ANFConfig.h"
#include "ANFRHI.h"
#include "ANFPreProcessor.h"

#include "Misc/ConfigCacheIni.h"

#if ENGINE_MINOR_VERSION > 0
#include "Misc/ConfigUtilities.h"
#endif

#include "anf.h"
#include "anf_sr.h"
#include "anf_fg.h"

#if PLATFORM_ANDROID
#include <dlfcn.h>
#endif

#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogANFAPI);

struct FANFSDKInterface
{
    AnfFunctions anfFunctions{};
    AnfInstance anfInstance = nullptr;
    AnfTechnique anfSrTechnique = nullptr;
    AnfTechnique anfFgTechnique = nullptr;
    bool hasTechnique = false;
    bool hasFgTechnique = false;
};

ANFBackendInterface* ANFBackendInterface::s_instance = nullptr;
FCriticalSection ANFBackendInterface::s_instanceLock;

ANFBackendInterface::ANFBackendInterface()
    : m_anfSDKInterface(nullptr)
    , m_perFrameData()
    , m_previousJitterInfo()
    , m_isFirstFrame(true)
    , m_isSrApiSupported(EANFSupportState::Unknown)
    , m_isFgApiSupported(EANFSupportState::Unknown)
    , m_isSrTechniqueSupported(EANFSupportState::Unknown)
    , m_isFgTechniqueSupported(EANFSupportState::Unknown)
    , m_isAdapterInfoSet(false)
    , m_createdDebugOverlayValue(-1)
{
    static_assert((uint32_t)ANF_TECHNIQUE_ID_SR_TEMPORAL == (uint32_t)ANFBackendInterface::ANF_TECHNIQUE_SUPER_RESOLUTION);
    static_assert((uint32_t)ANF_TECHNIQUE_ID_FG_TEMPORAL == (uint32_t)ANFBackendInterface::ANF_TECHNIQUE_FRAME_GENERATION);
}

ANFBackendInterface::~ANFBackendInterface()
{
    if (HasBackendWrapper())
    {
        ReleaseBackendWrapper();
    }
}

bool ANFBackendInterface::InitializeANF_SR(const FIntVector2& srcSize, const FIntVector2& dstSize)
{
    if (!IsSrApiSupported())
    {
        return false;
    }

    if (!SrNeedsReInit(srcSize, dstSize))
    {
        return true;
    }

    if (!InitializeBackendWrapper(ANF_TECHNIQUE_SUPER_RESOLUTION, srcSize, dstSize))
    {
        return false;
    }

    return true;
}

bool ANFBackendInterface::HasSrTechnique() const
{
    return HasBackendWrapper() && m_anfSDKInterface->hasTechnique;
}

bool ANFBackendInterface::HasFgTechnique() const
{
    return HasBackendWrapper() && m_anfSDKInterface->hasFgTechnique;
}

void ANFBackendInterface::ExecuteANF(ANFTechnique technique, FRHICommandListImmediate& RHICmdList, ANFExecutionData& data)
{
    RHICmdList.Transition(FRHITransitionInfo(data.inputColor, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(data.inputDepth, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(data.inputMotion, ERHIAccess::Unknown, ERHIAccess::CopySrc));
    RHICmdList.Transition(FRHITransitionInfo(data.outputColor, ERHIAccess::Unknown, ERHIAccess::CopyDest));

    {
        if (RHICmdList.IsInsideRenderPass())
        {
            RHICmdList.EndRenderPass();
        }

        if (GANFDebugSkipDispatch)
        {
            ANFRHI::GetRHI()->RHIAddBlitTexturePass(RHICmdList, data.inputColor, data.outputColor, true);
        }
        else
        {
            ANFExecutionData localData = data;

            RHICmdList.EnqueueLambda([technique, localData, this](FRHICommandListImmediate& InCmdList) mutable
                {
                    DispatchBackendWrapper(technique, InCmdList, localData);
                });
        }

    }
    
    RHICmdList.Transition(FRHITransitionInfo(data.outputColor, ERHIAccess::CopyDest, ERHIAccess::SRVMask));
}

bool ANFBackendInterface::InitializeANF_FG(const FIntVector2& depthMotionSize, const FIntVector2& ColorSize)
{
    if (!IsFgApiSupported())
    {
        return false;
    }

    if (!FgNeedsReInit(depthMotionSize, ColorSize))
    {
        return true;
    }

    if (!InitializeBackendWrapper(ANF_TECHNIQUE_FRAME_GENERATION, depthMotionSize, ColorSize))
    {
        return false;
    }

    return true;
}

void LogANFSDKMsg(AnfLogLevel logLevel, const char* pMessage)
{
    switch (logLevel)
    {
    case ANF_LOG_LEVEL_ERROR:
        UE_LOG(LogANFAPI, Error, TEXT("[ANF::ANF_LOG_LEVEL_ERROR] %s"), ANSI_TO_TCHAR(pMessage));
        break;
    case ANF_LOG_LEVEL_WARNING:
        UE_LOG(LogANFAPI, Warning, TEXT("[ANF::ANF_LOG_LEVEL_WARNING] %s"), ANSI_TO_TCHAR(pMessage));
        break;
    case ANF_LOG_LEVEL_INFO:
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF::ANF_LOG_LEVEL_INFO] %s"), ANSI_TO_TCHAR(pMessage));
        break;
    case ANF_LOG_LEVEL_NONE:
    default:
        break;
    }
}

void ANFBackendInterface::ReCreateInstance()
{
    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Re-creating instance"));
    if (HasBackendWrapper())
    {
        ReleaseBackendWrapper();
    }
    CreateANFInstance();
}

bool ANFBackendInterface::InitializeBackendWrapper(ANFTechnique technique, const FIntVector2& srcSize, const FIntVector2& dstSize)
{
    if (m_createdDebugOverlayValue != GANFDebugOverlayAllowed)
    {
        ReCreateInstance();
    }

    if (m_anfSDKInterface == nullptr)
    {
        UE_LOG(LogANFAPI, Warning, TEXT("[ANF] Technique %d %s called without an instance, this is likely a mistake!"), technique, ANSI_TO_TCHAR(__FUNCTION__));
        if (!CreateANFInstance())
        {
            UE_LOG(LogANFAPI, Warning, TEXT("[ANF] Technique %d Failed to create ANF instance when initializing the wrapper!"), technique);
            return false;
        }
    }

    if (!m_isAdapterInfoSet)
    {
        AnfStructHeader* pClientApiDeviceInfo = ANFRHI::GetRHI()->GetAdapterInfo();

        AnfResult anfResult = m_anfSDKInterface->anfFunctions.SetInstanceClientAPIAdapterInfo(m_anfSDKInterface->anfInstance, pClientApiDeviceInfo);
        if (anfResult != ANF_RESULT_SUCCESS)
        {
            UE_LOG(LogANFAPI, Error, TEXT("[ANF] Failed to set client API adapter info, error %d!"), anfResult);
            return false;
        }

        m_isAdapterInfoSet = true;
    }
    
    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Technique %d Checking technique support"), technique);
    AnfTechniqueId techniqueId = ANF_TECHNIQUE_ID_SR_TEMPORAL;
    bool bSupported = true;
    if (technique == ANF_TECHNIQUE_FRAME_GENERATION)
    {
        bSupported = IsFgTechniqueSupported();
        techniqueId = ANF_TECHNIQUE_ID_FG_TEMPORAL;
    }
    else
    {
        bSupported = IsSrTechniqueSupported();
    }

    if (!bSupported)
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] Technique %d ANF SDK technique is not supported!"), technique);
        return false;
    }

    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Technique %d Creating ANF technique"), technique);

    AnfTechniqueCreateInfo techCreateInfo = {};
    techCreateInfo.header.type = ANF_STYPE_TECHNIQUE_CREATE_INFO;
    techCreateInfo.header.pNext = NULL;
    techCreateInfo.flags = 0x0; ///< Currently this flag must be set
    techCreateInfo.pTechniqueGroupCreateInfo = nullptr;
    techCreateInfo.techniqueId = techniqueId;
    techCreateInfo.pClientApiCreateInfo = nullptr;
    techCreateInfo.maxInFlight = GANFMaxNumInFlight;

    ANFRHI::GetRHI()->AddANFTechniqueCreateInfoRHI(&techCreateInfo);

    AnfSRCreateInfo srCreateInfo = {};
    AnfFGCreateInfo fgCreateInfo = {};
    AnfTechnique* pOutputTechnique = nullptr;
    bool* pHasTechPtr = nullptr;

    if (technique == ANF_TECHNIQUE_SUPER_RESOLUTION)
    {
        techCreateInfo.pTechniqueGroupCreateInfo = &srCreateInfo.header;

        srCreateInfo.header.type = ANF_STYPE_SR_CREATE_INFO;
        srCreateInfo.inputSize.width = srcSize.X;
        srCreateInfo.inputSize.height = srcSize.Y;
        srCreateInfo.outputSize.width = dstSize.X;
        srCreateInfo.outputSize.height = dstSize.Y;

        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] srCreateInfo.inputSize.width = %d"), srCreateInfo.inputSize.width);
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] srCreateInfo.inputSize.height = %d"), srCreateInfo.inputSize.height);
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] srCreateInfo.outputSize.width = %d"), srCreateInfo.outputSize.width);
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] srCreateInfo.outputSize.height = %d"), srCreateInfo.outputSize.height);

        if (HasSrTechnique())
        {
            UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Re-creating SR Technique"));
            m_anfSDKInterface->anfFunctions.DestroyTechnique(m_anfSDKInterface->anfInstance, m_anfSDKInterface->anfSrTechnique);
            m_anfSDKInterface->anfSrTechnique = nullptr;
            m_anfSDKInterface->hasTechnique = false;
        }

        pOutputTechnique = &m_anfSDKInterface->anfSrTechnique;
        pHasTechPtr = &m_anfSDKInterface->hasTechnique;

        m_expectedInputSize = srcSize;
        m_expectedOutputSize = dstSize;
    }
    else if (technique == ANF_TECHNIQUE_FRAME_GENERATION)
    {
        techCreateInfo.pTechniqueGroupCreateInfo = &fgCreateInfo.header;

        fgCreateInfo.header.type = ANF_STYPE_FG_CREATE_INFO;
        fgCreateInfo.inputSize.width = dstSize.X;
        fgCreateInfo.inputSize.height = dstSize.Y;

        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] fgCreateInfo.inputSize.width = %d"), fgCreateInfo.inputSize.width);
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] fgCreateInfo.inputSize.height = %d"), fgCreateInfo.inputSize.height);

        if (HasFgTechnique())
        {
            UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Re-creating FG Technique"));
            m_anfSDKInterface->anfFunctions.DestroyTechnique(m_anfSDKInterface->anfInstance, m_anfSDKInterface->anfFgTechnique);
            m_anfSDKInterface->anfFgTechnique = nullptr;
            m_anfSDKInterface->hasFgTechnique = false;
        }

        pOutputTechnique = &m_anfSDKInterface->anfFgTechnique;
        pHasTechPtr = &m_anfSDKInterface->hasFgTechnique;

        m_fgDepthMotionSize = srcSize;
        m_fgColorSize = dstSize;
    }

    AnfResult anfResult = m_anfSDKInterface->anfFunctions.CreateTechnique(m_anfSDKInterface->anfInstance, &techCreateInfo, pOutputTechnique);

    if (anfResult != ANF_RESULT_SUCCESS)
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] Technique %d Failed to create ANF SDK technique with error %d!"), technique, anfResult);
        return false;
    }
    check(*pOutputTechnique != nullptr);
    *pHasTechPtr = true;
    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Technique %d ANF context created"), technique);

    return true;
}

void ANFBackendInterface::ReleaseBackendWrapper()
{
    if (!HasBackendWrapper())
        return;

    if (m_anfSDKInterface->anfInstance != nullptr)
    {
        if (GDynamicRHI != nullptr &&
            (m_anfSDKInterface->anfSrTechnique != nullptr ||
                m_anfSDKInterface->anfFgTechnique != nullptr))
        {
            // Wait until any submitted work is completed to avoid freeing resources in use by the SDK
            GDynamicRHI->RHIBlockUntilGPUIdle();
        }

        if (m_anfSDKInterface->anfSrTechnique != nullptr)
        {
            m_anfSDKInterface->anfFunctions.DestroyTechnique(m_anfSDKInterface->anfInstance, m_anfSDKInterface->anfSrTechnique);
        }

        if (m_anfSDKInterface->anfFgTechnique != nullptr)
        {
            m_anfSDKInterface->anfFunctions.DestroyTechnique(m_anfSDKInterface->anfInstance, m_anfSDKInterface->anfFgTechnique);
        }
        m_anfSDKInterface->anfFunctions.DestroyInstance(m_anfSDKInterface->anfInstance);
        m_isAdapterInfoSet = false;
    }

    delete m_anfSDKInterface;
    m_anfSDKInterface = nullptr;
}

inline AnfResourceLabel ExternalResourceToResourceLabel(uint32_t rid)
{
    switch (rid)
    {
    case ANFExternalResource::ANF_EXT_RESOURCE_INPUT_COLOR:
        return ANF_RESOURCE_LABEL_INPUT_COLOR;
    case ANFExternalResource::ANF_EXT_RESOURCE_INPUT_DEPTH:
        return ANF_RESOURCE_LABEL_DEPTH;
    case ANFExternalResource::ANF_EXT_RESOURCE_INPUT_VELOCITY:
        return ANF_RESOURCE_LABEL_MOTION_VECTORS;
    case ANFExternalResource::ANF_EXT_RESOURCE_OUTPUT_COLOR:
        return ANF_RESOURCE_LABEL_OUTPUT_COLOR;
    default:
        check(false);
        return ANF_RESOURCE_LABEL_INPUT_COLOR;
    }
}

inline AnfFormat UEFormatToANFFormat(EPixelFormat frmt)
{
    switch (frmt)
    {
    case EPixelFormat::PF_R8G8B8A8:
        return ANF_FORMAT_R8G8B8A8_UNORM;
    case EPixelFormat::PF_FloatR11G11B10:
        return ANF_FORMAT_B10G11R11_UFLOAT;
    case EPixelFormat::PF_DepthStencil:
        return ANF_FORMAT_D24S8_UNORM;
    case EPixelFormat::PF_G16R16F:
        return ANF_FORMAT_R16G16_FLOAT;
    case EPixelFormat::PF_FloatRGBA:
        return ANF_FORMAT_R16G16B16A16_FLOAT;
    default:
        check(false);
        return ANF_FORMAT_UNKNOWN;
    }
}

inline EPixelFormat ANFFormatToUEFormat(AnfFormat  frmt)
{
    switch (frmt)
    {
    case ANF_FORMAT_R8G8B8A8_UNORM:
        return EPixelFormat::PF_R8G8B8A8;
    case ANF_FORMAT_B10G11R11_UFLOAT:
        return EPixelFormat::PF_FloatR11G11B10;
    case ANF_FORMAT_D24S8_UNORM:
        return EPixelFormat::PF_DepthStencil;
    case ANF_FORMAT_R16G16_FLOAT:
        return EPixelFormat::PF_G16R16F;
    case ANF_FORMAT_R16G16B16A16_FLOAT:
        return EPixelFormat::PF_FloatRGBA;
    default:
        check(false);
        return EPixelFormat::PF_Unknown;
    }
}

inline FString StringifyANFFormat(AnfFormat  frmt)
{
    switch (frmt)
    {
#define HANDLE_CASE(c) case c: return FString(TEXT(#c));
        HANDLE_CASE(ANF_FORMAT_R8G8B8A8_UNORM);
        HANDLE_CASE(ANF_FORMAT_B10G11R11_UFLOAT);
        HANDLE_CASE(ANF_FORMAT_D32_FLOAT);
        HANDLE_CASE(ANF_FORMAT_D24S8_UNORM);
        HANDLE_CASE(ANF_FORMAT_R16G16_FLOAT);
        HANDLE_CASE(ANF_FORMAT_R16G16B16A16_FLOAT);
#undef HANDLE_CASE
    default:
        check(false);
        return FString(TEXT("UNKNOWN"));
    }
}

inline FString StringifyANFLabel(AnfResourceLabel label)
{
    switch (label)
    {
#define HANDLE_CASE(c) case c: return FString(TEXT(#c));
        HANDLE_CASE(ANF_RESOURCE_LABEL_DEPTH);
        HANDLE_CASE(ANF_RESOURCE_LABEL_MOTION_VECTORS);
        HANDLE_CASE(ANF_RESOURCE_LABEL_INPUT_COLOR);
        HANDLE_CASE(ANF_RESOURCE_LABEL_OUTPUT_COLOR);
#undef HANDLE_CASE
    default:
        check(false);
        return FString(TEXT("UNKNOWN"));
    }
}

void ANFBackendInterface::DispatchBackendWrapper(ANFTechnique technique, FRHICommandListImmediate& RHICmdList, ANFExecutionData& data)
{
    if (!HasBackendWrapper())
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] ANFBackendInterface::DispatchBackendWrapper(technique = %d) called without a wrapper!"), technique);
        return;
    }

    AnfResourceParamDesc resourceParams[ANFExternalResource::ANF_EXT_RESOURCE_COUNT];

    FTextureRHIRef ueResourceList[ANFExternalResource::ANF_EXT_RESOURCE_COUNT] = {};
    ueResourceList[ANFExternalResource::ANF_EXT_RESOURCE_INPUT_COLOR] = data.inputColor;
    ueResourceList[ANFExternalResource::ANF_EXT_RESOURCE_INPUT_DEPTH] = data.inputDepth;
    ueResourceList[ANFExternalResource::ANF_EXT_RESOURCE_INPUT_VELOCITY] = data.inputMotion;
    ueResourceList[ANFExternalResource::ANF_EXT_RESOURCE_OUTPUT_COLOR] = data.outputColor;

    bool invalidInput = false;

    for (uint32_t resIndex = 0; resIndex < ANFExternalResource::ANF_EXT_RESOURCE_COUNT; resIndex++)
    {
        FTextureRHIRef image = ueResourceList[resIndex];

#if ENGINE_MINOR_VERSION > 0
        FIntPoint dimensions = FIntPoint(image->GetDesc().Extent.X, image->GetDesc().Extent.Y);
        EPixelFormat ueFrmt = image->GetDesc().Format;
#else
        FIntPoint dimensions = FIntPoint(image->GetSizeXYZ().X, image->GetSizeXYZ().Y);
        EPixelFormat ueFrmt = image->GetFormat();
#endif

        resourceParams[resIndex].header.type = ANF_STYPE_RESOURCE_PARAM_DESC;
        resourceParams[resIndex].resourceLabel = ExternalResourceToResourceLabel(resIndex);
        resourceParams[resIndex].resourceDesc.header.type = ANF_STYPE_RESOURCE_DESC;
        resourceParams[resIndex].resourceDesc.type = ANF_RESOURCE_TYPE_TEXTURE2D;
        resourceParams[resIndex].resourceDesc.format = UEFormatToANFFormat(ueFrmt);
        resourceParams[resIndex].resourceDesc.layout = ANFRHI::GetRHI()->GetResourceParameterLayout(resIndex);
        if (ueFrmt == PF_DepthStencil)
        {
            if (ANFRHI::GetRHI()->Is32BitDepthFormat(image))
            {
                if (GANFDebugAllowD32S8)
                {
                    resourceParams[resIndex].resourceDesc.format = ANF_FORMAT_D32_FLOAT;
                }
                else
                {
                    UE_LOG(LogANFAPI, Error, TEXT("Depth format is D32S8 which is not officially supported!  Using D24S8, expect bad results"));
                }
            }
        }
        resourceParams[resIndex].resourceDesc.textureDims.width = dimensions.X;
        resourceParams[resIndex].resourceDesc.textureDims.height = dimensions.Y;
        resourceParams[resIndex].resourceDesc.textureDims.numArrayLayers = 1; // image->GetDesc().ArraySize;
        resourceParams[resIndex].resourceDesc.textureDims.mipCount = 1; // image->GetDesc().NumMips;
        resourceParams[resIndex].resourceDesc.resource = ANFRHI::GetRHI()->GetImageHandle(image); //< VkImage handle
        check(resourceParams[resIndex].resourceDesc.resource != VK_NULL_HANDLE);

#if ENGINE_MINOR_VERSION > 0
        if (image->GetDesc().Extent.X <= 1 && image->GetDesc().Extent.Y <= 1)
#else
        if (image->GetSizeXYZ().X <= 1 && image->GetSizeXYZ().Y <= 1)
#endif
        {
            invalidInput = true;
        }
    }

    AnfSRDispatch srDispatch = {};
    AnfFGDispatch fgDispatch = {};
    AnfStructHeader* ptechniqueHeader = nullptr;

    AnfTechniqueDispatchInfo dispatchInfo = {};
    dispatchInfo.header.type = ANF_STYPE_TECHNIQUE_DISPATCH_INFO;
    dispatchInfo.header.pNext = nullptr; // not needed for indirect dispatch
    dispatchInfo.commandList = ANFRHI::GetRHI()->GetCommandList(RHICmdList);
    check(dispatchInfo.commandList != nullptr);
    dispatchInfo.pResources = resourceParams;
    dispatchInfo.numResources = ANFExternalResource::ANF_EXT_RESOURCE_COUNT;
    dispatchInfo.reset = (invalidInput || data.reset) ? ANF_TRUE : ANF_FALSE;
    dispatchInfo.pTechniqueGroupDispatchInfo = &srDispatch.header;

    AnfTechnique dispatchTechnique = nullptr;

    if (technique == ANF_TECHNIQUE_SUPER_RESOLUTION)
    {
        srDispatch.header.type = ANF_STYPE_SR_DISPATCH;
        srDispatch.jitterOffset = { data.jitterOffset.X, data.jitterOffset.Y };

        ptechniqueHeader = &srDispatch.header;
        dispatchTechnique = m_anfSDKInterface->anfSrTechnique;
    }
    else if (technique == ANF_TECHNIQUE_FRAME_GENERATION)
    {
        fgDispatch.header.type = ANF_STYPE_FG_DISPATCH;

        ptechniqueHeader = &fgDispatch.header;
        dispatchTechnique = m_anfSDKInterface->anfFgTechnique;
    }

    AnfDebugOverlayConfig debugOverlayConfig;
    if (GANFDebugOverlayAllowed != 0)
    {
        debugOverlayConfig.header.type = ANF_STYPE_DEBUG_OVERLAY_CONFIG;
        debugOverlayConfig.header.pNext = ptechniqueHeader->pNext;
        debugOverlayConfig.mode = static_cast<AnfDebugOverlayMode>(GANFDebugOverlayMode);
        debugOverlayConfig.mvHeatmapScale = GANFDebugOverlayMvHeatmapScale;
        debugOverlayConfig.showRawMv = GANFDebugOverlayShowRawMv;
        debugOverlayConfig.mvValueScale.x = GANFDebugOverlayMvValueScaleX;
        debugOverlayConfig.mvValueScale.y = GANFDebugOverlayMvValueScaleY;
        debugOverlayConfig.jitterScale.x = GANFDebugOverlayJitterScaleX;
        debugOverlayConfig.jitterScale.y = GANFDebugOverlayJitterScaleY;
        debugOverlayConfig.jitterAccumAlpha = GANFDebugOverlayJitterAccumAlpha;
        debugOverlayConfig.accumulateJitter = GANFDebugOverlayAccumulateJitter;
        debugOverlayConfig.hudCorner = static_cast<AnfDebugOverlayHudCorner>(GANFDebugOverlayHUDCorner);
        debugOverlayConfig.hudFlipY = GANFDebugOverlayHUDFlipY;
        debugOverlayConfig.depthInvert = GANFDebugOverlayDepthInvert;
        debugOverlayConfig.depthScale = GANFDebugOverlayDepthScale;

        ptechniqueHeader->pNext = &debugOverlayConfig.header;
    }

    dispatchInfo.pTechniqueGroupDispatchInfo = ptechniqueHeader;
        
    if (dispatchInfo.pTechniqueGroupDispatchInfo == nullptr || dispatchTechnique == nullptr)
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] DispatchBackendWrapper: unrecognised technique %d, aborting dispatch."), technique);
        return;
    }

    const bool timePasses = GANFTimePasses != 0;
    const TCHAR* timedPassName = technique == ANF_TECHNIQUE_SUPER_RESOLUTION ? TEXT("SuperResolution") : TEXT("FrameGeneration");
    if (timePasses)
    {
        ANFRHI::GetRHI()->StartTimedPass(RHICmdList, timedPassName);
    }

    AnfResult result = m_anfSDKInterface->anfFunctions.DispatchTechnique(dispatchTechnique, &dispatchInfo);
    if (result != ANF_RESULT_SUCCESS)
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] Technique %d Failed to dispatch ANF technique with error %d"), technique, result);
        return;
    }
    if (timePasses)
    {
        ANFRHI::GetRHI()->EndTimedPass(RHICmdList, timedPassName);
    }
}

bool ANFBackendInterface::QueryTechniqueSupport(ANFTechnique technique)
{
    if (!HasBackendWrapper())
    {
        return false;
    }
    
    AnfTechniqueId techniqueId = ANF_TECHNIQUE_ID_SR_TEMPORAL;
    if (technique == ANF_TECHNIQUE_FRAME_GENERATION)
    {
        techniqueId = ANF_TECHNIQUE_ID_FG_TEMPORAL;
    }

    AnfResult result = m_anfSDKInterface->anfFunctions.IsTechniqueSupported(m_anfSDKInterface->anfInstance, techniqueId, ANFRHI::GetRHI()->GetAdapterInfo());

    if (result != ANF_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

void ANFBackendInterface::SetPerFrameData(const PerFrameData& curData)
{
    m_previousJitterInfo = PrevJitterInfo(curData);
    m_perFrameData = curData;
    m_isFirstFrame = false;
}

bool LogANFResourceRequirements(FANFSDKInterface* sdkInterface, AnfTechniqueId techniqueID)
{
    const AnfTechniqueRequirements* pTechniqueRequirements = NULL;
    AnfResult queryResult = sdkInterface->anfFunctions.QueryTechniqueRequirements(sdkInterface->anfInstance, techniqueID, &pTechniqueRequirements);

    static const AnfFormat gExpectedFormats[2][4] =
    {
        // ANF_TECHNIQUE_ID_SR_TEMPORAL
        {
            //ANF_RESOURCE_LABEL_DEPTH
            ANF_FORMAT_D24S8_UNORM,
            //ANF_RESOURCE_LABEL_MOTION_VECTORS
            ANF_FORMAT_R16G16_FLOAT,
            //ANF_RESOURCE_LABEL_INPUT_COLOR
            ANF_FORMAT_B10G11R11_UFLOAT,
            //ANF_RESOURCE_LABEL_OUTPUT_COLOR
            GANFPreferHalfFloatOutput == 0 ? ANF_FORMAT_B10G11R11_UFLOAT : ANF_FORMAT_R16G16B16A16_FLOAT,
        },
        // ANF_TECHNIQUE_ID_FG_TEMPORAL
        {
            //ANF_RESOURCE_LABEL_DEPTH
            ANF_FORMAT_D24S8_UNORM,
            //ANF_RESOURCE_LABEL_MOTION_VECTORS
            ANF_FORMAT_R16G16_FLOAT,
            //ANF_RESOURCE_LABEL_INPUT_COLOR
            ANF_FORMAT_R8G8B8A8_UNORM,
            //ANF_RESOURCE_LABEL_OUTPUT_COLOR
            ANF_FORMAT_R8G8B8A8_UNORM,
        },
    };

    if (queryResult == ANF_RESULT_SUCCESS)
    {
        for (uint32_t i = 0; i < pTechniqueRequirements->numRequiredResources; i++)
        {
            const AnfResourceRequirements* pReqs = NULL;
            const AnfResourceLabel         label = pTechniqueRequirements->pRequiredResources[i];

            queryResult = sdkInterface->anfFunctions.QueryTechniqueResourceRequirements(sdkInterface->anfInstance, techniqueID, label, &pReqs);

            if (queryResult == ANF_RESULT_SUCCESS)
            {
                const AnfFormat expectedFormat = gExpectedFormats[techniqueID][label];
                bool formatFound = false;
                // ... Create image with corresponding requirements (format, usage, size, etc) ...
                FString formatList = FString("");
                for (uint32_t jj = 0; jj < pReqs->numSupportedFormats; ++jj)
                {
                    if (techniqueID == ANF_TECHNIQUE_ID_FG_TEMPORAL && label == ANF_RESOURCE_LABEL_DEPTH && pReqs->pSupportedFormats[jj] == ANF_FORMAT_D24S8_UNORM)
                    {
                        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Enabling D24S8 for Frame Gen"));
                        GANFFrameGenSupportsD24S8 = 1;
                    }
                    if (pReqs->pSupportedFormats[jj] == expectedFormat)
                    {
                        formatFound = true;
                    }
                    formatList += StringifyANFFormat(pReqs->pSupportedFormats[jj]) + FString(TEXT(","));
                }
                UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] TechniqueID %d Label %s supported formats = {%s}"), techniqueID , *StringifyANFLabel(label), *formatList);

                if (!formatFound && 
                    techniqueID == ANF_TECHNIQUE_ID_SR_TEMPORAL && 
                    label == ANF_RESOURCE_LABEL_OUTPUT_COLOR)
                {
                    const AnfFormat altExpectedFormat = GANFPreferHalfFloatOutput == 0 ? ANF_FORMAT_R16G16B16A16_FLOAT : ANF_FORMAT_B10G11R11_UFLOAT;
                    for (uint32_t jj = 0; jj < pReqs->numSupportedFormats; ++jj)
                    {
                        if (pReqs->pSupportedFormats[jj] == altExpectedFormat)
                        {
                            UE_LOG(LogANFAPI, 
                                Warning, 
                                TEXT("[ANF] TechniqueID %d Label %s requested format %s was not supoprted, using %s instead"),
                                techniqueID,
                                *StringifyANFLabel(label),
                                *StringifyANFFormat(expectedFormat),
                                *StringifyANFFormat(altExpectedFormat))
                            GANFPreferHalfFloatOutput = (GANFPreferHalfFloatOutput == 0) ? 1 : 0;
                            formatFound = true;
                        }
                    }
                }

                if (!formatFound)
                {
                    UE_LOG(LogANFAPI,
                        Error,
                        TEXT("[ANF] TechniqueID %d resource %d (Label %s) expected to support format %s, but it was not found!"),
                        techniqueID,
                        i,
                        *StringifyANFLabel(label),
                        *StringifyANFFormat(expectedFormat));
                    return false;
                }

                if (pReqs->resourceType != ANF_RESOURCE_TYPE_TEXTURE2D)
                {
                    UE_LOG(LogANFAPI, 
                        Error, 
                        TEXT("[ANF] TechniqueID %d resource %d (Label %s) rquires resource type %d, but only 2D textures (%d) are implemented!"),
                        techniqueID,
                        i,
                        *StringifyANFLabel(label),
                        pReqs->resourceType,
                        ANF_RESOURCE_TYPE_TEXTURE2D);
                    return false;
                }

                ANFRHI::GetRHI()->SetResourceRequirements(techniqueID, label, pReqs);
            }
            else
            {
                UE_LOG(LogANFAPI, Error, TEXT("[ANF] TechniqueID %d Failed to query technique resource requirements %d"), techniqueID, i);
                return false;
            }
        }
    }
    else
    {
        UE_LOG(LogANFAPI, Error, TEXT("[ANF] TechniqueID %d Failed to query technique requirements"), techniqueID);
        return false;
    }
    return true;
}

bool ANFBackendInterface::CreateANFInstance()
{
    check(m_anfSDKInterface == nullptr);

    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Initializing ANF SDK interface"));
    m_anfSDKInterface = new FANFSDKInterface();
    check(m_anfSDKInterface);

    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Getting ANF SDK functions"));
#if PLATFORM_ANDROID
    GetAnfFunctions(&m_anfSDKInterface->anfFunctions);
    check(m_anfSDKInterface->anfFunctions.CreateInstance != nullptr);
    check(m_anfSDKInterface->anfFunctions.DestroyInstance != nullptr);
    check(m_anfSDKInterface->anfFunctions.IsTechniqueSupported != nullptr);
    check(m_anfSDKInterface->anfFunctions.QueryTechniqueRequirements != nullptr);
    check(m_anfSDKInterface->anfFunctions.QueryTechniqueResourceRequirements != nullptr);
    check(m_anfSDKInterface->anfFunctions.CreateTechnique != nullptr);
    check(m_anfSDKInterface->anfFunctions.DestroyTechnique != nullptr);
    check(m_anfSDKInterface->anfFunctions.DispatchTechnique != nullptr);
#endif // PLATFORM_ANDROID

    AnfResult anfResult = ANF_RESULT_SUCCESS;

    // EngineName will be of the form "UnrealEngine4.21", with the minor version ("21" in this example)
    // updated with every quarterly release
    FString EngineName = FApp::GetEpicProductIdentifier() + FEngineVersion::Current().ToString(EVersionComponent::Minor);
    FTCHARToUTF8 EngineNameConverter(*EngineName);
    FTCHARToUTF8 ProjectNameConverter(FApp::GetProjectName());


    AnfApplicationInfo appInfo = {};
    appInfo.header.type = ANF_STYPE_APPLICATION_INFO;
    appInfo.pApplicationName = ProjectNameConverter.Get();
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = EngineNameConverter.Get();
    appInfo.engineVersion = FEngineVersion::Current().GetMinor();
    
    TSharedPtr<IPlugin> AnfPlugin = IPluginManager::Get().FindPlugin(TEXT("ANF"));
    if (AnfPlugin.IsValid())
    {
        FPluginDescriptor Descriptor = AnfPlugin->GetDescriptor();
        appInfo.applicationVersion = Descriptor.Version;
    }

    AnfInstanceCreateInfo instInfo = {};
    instInfo.header.type = ANF_STYPE_INSTANCE_CREATE_INFO;
    instInfo.flags = 0x0;
    if (GANFDebugOverlayAllowed != 0)
    {
        instInfo.flags |= ANF_INSTANCE_CREATE_FLAG_ENABLE_DEBUG_OVERLAY;
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Setting AnfInstanceCreateInfo flag ANF_INSTANCE_CREATE_FLAG_ENABLE_DEBUG_OVERLAY"));
    }
    m_createdDebugOverlayValue = GANFDebugOverlayAllowed;
    instInfo.logLevel = GANFDebugShowInfoMessages == 1 ? ANF_LOG_LEVEL_INFO : ANF_LOG_LEVEL_WARNING;
    instInfo.logMessageCallback = LogANFSDKMsg;
    instInfo.clientApi = ANFRHI::GetRHI()->GetClientAPI();
    instInfo.pApplicationInfo = &appInfo;

    UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] Creating ANF SDK instance"));
    anfResult = m_anfSDKInterface->anfFunctions.CreateInstance(&instInfo, &m_anfSDKInterface->anfInstance);
    m_anfSDKInterface->anfSrTechnique = nullptr;
    m_anfSDKInterface->hasTechnique = false;
    m_anfSDKInterface->anfFgTechnique = nullptr;
    m_anfSDKInterface->hasFgTechnique = false;

    AnfVersion currentLibVersion;
    if (m_anfSDKInterface->anfFunctions.QueryAnfVersion(&currentLibVersion) == ANF_RESULT_SUCCESS)
    {
        UE_LOG(LogANFAPI, Verbose, TEXT("[ANF] SDK Version = %d.%d.%d"), currentLibVersion.major, currentLibVersion.minor, currentLibVersion.build);
    }
    else
    {
        UE_LOG(LogANFAPI, Warning, TEXT("Failed to query ANF SDK Version!"));
    }

    if (anfResult != ANF_RESULT_SUCCESS)
    {
        UE_LOG(LogANFAPI, Error, TEXT("Failed to create ANF instance! Result = %d"), anfResult);
        delete m_anfSDKInterface;
        m_anfSDKInterface = nullptr;
        return false;
    }

    m_isSrApiSupported = LogANFResourceRequirements(m_anfSDKInterface, ANF_TECHNIQUE_ID_SR_TEMPORAL)
        ? EANFSupportState::Supported : EANFSupportState::NotSupported;
    if (m_isSrApiSupported != EANFSupportState::Supported)
    {
        UE_LOG(LogANFAPI, Error, TEXT("Failed to get ANF requirements for SR!"));
    }

    m_isFgApiSupported = LogANFResourceRequirements(m_anfSDKInterface, ANF_TECHNIQUE_ID_FG_TEMPORAL)
        ? EANFSupportState::Supported : EANFSupportState::NotSupported;
    if (m_isFgApiSupported != EANFSupportState::Supported)
    {
        UE_LOG(LogANFAPI, Error, TEXT("Failed to get ANF requirements for FG!"));
    }

    return true;
}

bool QueryANFRequirementsHelper(FANFSDKInterface* sdkInterface, AnfTechniqueId techniqueID, TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions)
{
    check(sdkInterface);

    // Validate technique is supported
    const AnfTechniqueRequirements* pTechniqueRequirements = NULL;
    AnfResult result = sdkInterface->anfFunctions.QueryTechniqueRequirements(sdkInterface->anfInstance, techniqueID, &pTechniqueRequirements);

    if (result == ANF_RESULT_SUCCESS)
    {
        bool vulkanSupported = ANFRHI::GetRHI()->GetTechniqueExtensions(techniqueID, pTechniqueRequirements->pClientApiRequirements, deviceExtensions, instanceExtensions);


        if (!vulkanSupported)
        {
            UE_LOG(LogANFAPI, Error, TEXT("Failed to find ANF vulkan requirements!"));
            return false;
        }
    }
    else
    {
        UE_LOG(LogANFAPI, Error, TEXT("Failed to query ANF technique requirements!"));
        return false;
    }

    return true;
}

void ANFBackendInterface::QueryANFRequirements(TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions)
{
    if (m_anfSDKInterface == nullptr)
    {
        if (!CreateANFInstance())
        {
            UE_LOG(LogANFAPI, Error, TEXT("Failed to create ANF instance before querying requirements!"));
            return;
        }
    }

    if (m_isSrApiSupported != EANFSupportState::NotSupported)
    {
        m_isSrApiSupported = QueryANFRequirementsHelper(m_anfSDKInterface, ANF_TECHNIQUE_ID_SR_TEMPORAL, deviceExtensions, instanceExtensions)
            ? EANFSupportState::Supported : EANFSupportState::NotSupported;
        if (m_isSrApiSupported != EANFSupportState::Supported)
        {
            UE_LOG(LogANFAPI, Warning, TEXT("ANF SuperResolution is not supported!"));
        }
    }

    if (m_isFgApiSupported != EANFSupportState::NotSupported)
    {
        m_isFgApiSupported = QueryANFRequirementsHelper(m_anfSDKInterface, ANF_TECHNIQUE_ID_FG_TEMPORAL, deviceExtensions, instanceExtensions)
            ? EANFSupportState::Supported : EANFSupportState::NotSupported;
        if (m_isFgApiSupported != EANFSupportState::Supported)
        {
            UE_LOG(LogANFAPI, Warning, TEXT("ANF FrameGeneration is not supported!"));
        }
    }
}

// FANFBackendModule
void FANFBackendModule::StartupModule()
{
#if ENGINE_MINOR_VERSION > 0
    UE::ConfigUtilities::ApplyCVarSettingsFromIni(TEXT("/Script/ANFAPIModule.ANFSettings"), *GEngineIni, ECVF_SetByProjectSetting);
#else
    ApplyCVarSettingsFromIni(TEXT("/Script/ANFAPIModule.ANFSettings"), *GEngineIni, ECVF_SetByProjectSetting);
#endif

	if (FApp::CanEverRender())
	{
#if PLATFORM_WINDOWS
		TCHAR const* DynamicRHIModuleName = GetSelectedDynamicRHIModuleName(false);
#else
		TCHAR const* DynamicRHIModuleName = TEXT("VulkanRHI");

        if (FString("VulkanRHI") == FString(DynamicRHIModuleName))
        {
            TArray<ANSICHAR const*> deviceExtensionsToAdd;
            TArray<ANSICHAR const*> instanceExtensionsToAdd;
            GetBackendInterface()->QueryANFRequirements(deviceExtensionsToAdd, instanceExtensionsToAdd);

            ANFRHI::GetRHI()->AddExtensions(deviceExtensionsToAdd, instanceExtensionsToAdd);
        }
#endif // PLATFORM_WINDOWS
	}
}

void FANFBackendModule::ShutdownModule()
{
	ANFBackendInterface::Destroy();
    FANFPreProcessor::Destroy();
    ANFRHI::Destroy();
}

IMPLEMENT_MODULE(FANFBackendModule, ANFAPI)
