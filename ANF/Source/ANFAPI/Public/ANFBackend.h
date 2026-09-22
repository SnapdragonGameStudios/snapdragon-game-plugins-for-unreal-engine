//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once
#include "ANFConfig.h"

#include "Modules/ModuleManager.h"
#include "RHIDefinitions.h"
#include "HAL/CriticalSection.h"

DECLARE_LOG_CATEGORY_EXTERN(LogANFAPI, Verbose, All);

struct FANFSDKInterface;

typedef enum ANFExternalResource
{
    ANF_EXT_RESOURCE_INPUT_COLOR = 0,
    ANF_EXT_RESOURCE_INPUT_DEPTH = 1,
    ANF_EXT_RESOURCE_INPUT_VELOCITY = 2,
    ANF_EXT_RESOURCE_OUTPUT_COLOR = 3,
    ANF_EXT_RESOURCE_FIRST = ANF_EXT_RESOURCE_INPUT_COLOR,
    ANF_EXT_RESOURCE_LAST = ANF_EXT_RESOURCE_OUTPUT_COLOR,
    ANF_EXT_RESOURCE_COUNT = ANF_EXT_RESOURCE_LAST + 1,
} ANFExternalResource;

enum class EANFSupportState : uint8
{
    Unknown  = 0,
    Supported    = 1,
    NotSupported = 2,
};

class ANFAPI_API ANFBackendInterface
{
public:
    inline static ANFBackendInterface* Get()
    {
        FScopeLock Lock(&s_instanceLock);
        if (!s_instance)
        {
            s_instance = new ANFBackendInterface();
        }
        return s_instance;
    }

    static inline void Destroy()
    {
        FScopeLock Lock(&s_instanceLock);
        if (s_instance)
        {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    struct ANFExecutionData
    {
        uint32_t frameNumber = 0;
        bool reset = false;
        FVector2f jitterOffset = {};
        FTextureRHIRef inputColor = nullptr;
        FTextureRHIRef inputDepth = nullptr;
        FTextureRHIRef inputMotion = nullptr;
        FTextureRHIRef outputColor = nullptr;
    };

    inline bool SrNeedsReInit(const FIntVector2& srcSize, const FIntVector2& dstSize)
    {
        return (m_anfSDKInterface == nullptr || 
            !HasSrTechnique() ||
            m_expectedInputSize != srcSize || 
            m_expectedOutputSize != dstSize ||
            m_createdDebugOverlayValue != GANFDebugOverlayAllowed);
    }

    inline bool FgNeedsReInit(const FIntVector2& depthMotionSize, const FIntVector2& colorSize)
    {
        return (m_anfSDKInterface == nullptr ||
            !HasFgTechnique() ||
            m_fgDepthMotionSize != depthMotionSize ||
            m_fgColorSize != colorSize ||
            m_createdDebugOverlayValue != GANFDebugOverlayAllowed);
    }

    bool InitializeANF_SR(const FIntVector2& srcSize, const FIntVector2& dstSize);
    inline void ExecuteANF_SR(FRHICommandListImmediate& RHICmdList, ANFExecutionData& data)
    {
        ExecuteANF(ANF_TECHNIQUE_SUPER_RESOLUTION, RHICmdList, data);
    }

    bool InitializeANF_FG(const FIntVector2& depthMotionSize, const FIntVector2& ColorSize);
    inline void ExecuteANF_FG(FRHICommandListImmediate& RHICmdList, ANFExecutionData& data)
    {
        ExecuteANF(ANF_TECHNIQUE_FRAME_GENERATION, RHICmdList, data);
    }

    inline bool HasBackendWrapper() const { return (m_anfSDKInterface != nullptr); }
    inline FANFSDKInterface* GetWrapperFunctions() { return m_anfSDKInterface; }
    inline uint32_t GetPreviousFrameNumber() const { return m_previousJitterInfo.frameNumber;}
    inline void GetPreviousJitterOffset(float* pOutOffset) const { pOutOffset[0] = m_previousJitterInfo.offset[0]; pOutOffset[1] = m_previousJitterInfo.offset[1]; }
    inline bool IsFirstFrame() const { return m_isFirstFrame; }

    bool HasSrTechnique() const;
    bool HasFgTechnique() const;

    typedef struct PerFrameData
    {
        uint32_t frameNumber;
        int32_t jitterId;
        float   jitterOffset[2];
        float   prevJitterOffset[2];
        float   preExposure;
        float   colorGamma[4];

        PerFrameData()
            : frameNumber(0)
            , jitterId(-1)
			, jitterOffset{ 0.f, 0.f }
			, prevJitterOffset{ 0.f, 0.f  }
            , preExposure(-1.f)
			, colorGamma{ -1.f, -1.f, -1.f, -1.f }
        {}

        PerFrameData(const PerFrameData& i)
            : frameNumber(i.frameNumber)
            , jitterId(i.jitterId)
			, jitterOffset{ i.jitterOffset[0], i.jitterOffset[1] }
			, prevJitterOffset{ i.prevJitterOffset[0], i.prevJitterOffset[1] }
            , preExposure(i.preExposure)
			, colorGamma{ i.colorGamma[0], i.colorGamma[1], i.colorGamma[2], i.colorGamma[3] }
        {}

    } PerFrameData;

    void SetPerFrameData(const PerFrameData& curData);

    inline void GetPerFrameData(PerFrameData& curData) const
    {
        curData = m_perFrameData;
    }

    bool CreateANFInstance();
    void QueryANFRequirements(TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions);

    inline bool IsSrApiSupported() const { return m_isSrApiSupported == EANFSupportState::Supported; }
    inline bool IsFgApiSupported() const { return m_isFgApiSupported == EANFSupportState::Supported; }

    inline bool IsSrTechniqueSupported()
    {
        if (!IsSrApiSupported() || !HasBackendWrapper())
        {
            return false;
        }

        if (m_isSrTechniqueSupported == EANFSupportState::Unknown)
        {
            m_isSrTechniqueSupported = QueryTechniqueSupport(ANF_TECHNIQUE_SUPER_RESOLUTION)
                ? EANFSupportState::Supported : EANFSupportState::NotSupported;
        }
        return m_isSrTechniqueSupported == EANFSupportState::Supported;
    }

    inline bool IsFgTechniqueSupported()
    {
        if (!IsFgApiSupported() || !HasBackendWrapper())
        {
            return false;
        }

        if (m_isFgTechniqueSupported == EANFSupportState::Unknown)
        {
            m_isFgTechniqueSupported = QueryTechniqueSupport(ANF_TECHNIQUE_FRAME_GENERATION)
                ? EANFSupportState::Supported : EANFSupportState::NotSupported;
        }
        return m_isFgTechniqueSupported == EANFSupportState::Supported;
    }

    void ReCreateInstance();

protected:
    enum ANFTechnique
    {
        ANF_TECHNIQUE_SUPER_RESOLUTION = 0,
        ANF_TECHNIQUE_FRAME_GENERATION = 1,
        ANF_TECHNIQUE_COUNT = 2,
    };

    bool InitializeBackendWrapper(ANFTechnique technique, const FIntVector2& srcSize, const FIntVector2& dstSize);
    void ReleaseBackendWrapper();
    void ExecuteANF(ANFTechnique technique, FRHICommandListImmediate& RHICmdList, ANFExecutionData& data);
    void DispatchBackendWrapper(ANFTechnique technique, FRHICommandListImmediate& RHICmdList, ANFExecutionData& data);

    bool QueryTechniqueSupport(ANFTechnique technique);

    typedef struct PrevJitterInfo
    {
        uint32_t frameNumber;
        int32_t index;
        float offset[2];

        PrevJitterInfo() 
        : frameNumber(0)
        , index(0)
        , offset{0.f, 0.f}
        {}

        PrevJitterInfo(uint32_t f, int32_t i, float x, float y) 
        : frameNumber(f)
        , index(i)
        , offset{x, y}
        {}

        PrevJitterInfo(const PrevJitterInfo& i) 
            : frameNumber(i.frameNumber)
            , index(i.index)
			, offset{ i.offset[0], i.offset[1] }
        {}

        PrevJitterInfo(const PerFrameData& frameData)
            : frameNumber(frameData.frameNumber)
            , index(frameData.jitterId)
            , offset{ frameData.jitterOffset[0], frameData.jitterOffset[1] }
        {
        }
    } PrevJitterInfo;

    FANFSDKInterface* m_anfSDKInterface;
    PerFrameData m_perFrameData;
    PrevJitterInfo m_previousJitterInfo;
    bool m_isFirstFrame;

    static ANFBackendInterface* s_instance;
    static FCriticalSection s_instanceLock;

private:
    ANFBackendInterface();
    virtual ~ANFBackendInterface();

    FIntVector2 m_expectedInputSize;
    FIntVector2 m_expectedOutputSize;

    FIntVector2 m_fgDepthMotionSize;
    FIntVector2 m_fgColorSize;

    EANFSupportState m_isSrApiSupported;
    EANFSupportState m_isFgApiSupported;
    EANFSupportState m_isSrTechniqueSupported;
    EANFSupportState m_isFgTechniqueSupported;

    bool m_isAdapterInfoSet;

    int32 m_createdDebugOverlayValue;
};

class FANFBackendModule final : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	inline ANFBackendInterface* GetBackendInterface() const { return ANFBackendInterface::Get(); }
};

