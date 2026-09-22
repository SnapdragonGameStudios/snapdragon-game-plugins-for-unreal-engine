//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#include "ANFRHIVK.h"
#if ENGINE_MINOR_VERSION > 1
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"

#include "VulkanRHIPrivate.h"
#include "VulkanDynamicRHI.h"
#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#if ENGINE_MINOR_VERSION == 0
#include "VulkanRHIBridge.h"
#endif

inline VkDevice GetVkDeviceFromUEDevice(FVulkanDevice* ueDevice)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 6
    return ueDevice->GetInstanceHandle();
#else
    return ueDevice->GetHandle();
#endif

}

inline VkCommandBuffer ANFGetVkCommandBuffer(FRHICommandList& RHICmdList)
{
    FVulkanCommandListContext& context = (FVulkanCommandListContext&)RHICmdList.GetContext().GetLowestLevelContext();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
    FVulkanCmdBuffer* activeCmdBuffer = context.GetCommandBufferManager()->GetActiveCmdBuffer();
    return activeCmdBuffer->GetHandle(); 
#else
    FVulkanCommandBuffer& activeCmdBuffer = context.GetCommandBuffer();
    return activeCmdBuffer.GetHandle(); 
#endif
}

ANFRHIVK::~ANFRHIVK()
{
    if (m_passQueryData.Num() > 0)
    {
#if ENGINE_MINOR_VERSION > 2
        FVulkanDevice* UEDevice = FVulkanDynamicRHI::Get().GetDevice();
#else
        FVulkanDevice* UEDevice = GVulkanRHI->GetDevice();
#endif
        VkDevice vkDevice = GetVkDeviceFromUEDevice(UEDevice);
        for (auto& Entry : m_passQueryData)
        {
            if (Entry.Value.queryPool != VK_NULL_HANDLE)
            {
                VulkanRHI::vkDestroyQueryPool(vkDevice, Entry.Value.queryPool, nullptr);
                Entry.Value.queryPool = VK_NULL_HANDLE;
            }
        }
        m_passQueryData.Empty();
    }
}

AnfStructHeader* ANFRHIVK::GetAdapterInfo()
{
#if ENGINE_MINOR_VERSION > 2
    FVulkanDevice* UEDevice = FVulkanDynamicRHI::Get().GetDevice();
#else
    FVulkanDevice* UEDevice = GVulkanRHI->GetDevice();
#endif
    AnfAdapterInfoVulkan adapterInfo = {};
    adapterInfo.header.type = ANF_STYPE_ADAPTER_INFO_VULKAN;
    adapterInfo.physicalDevice = UEDevice->GetPhysicalHandle();
    adapterInfo.device = GetVkDeviceFromUEDevice(UEDevice);
    adapterInfo.instance =
#if ENGINE_MINOR_VERSION > 2
        FVulkanDynamicRHI::Get().GetInstance();
#else 
        GVulkanRHI->GetInstance();
#endif

    m_vulkanAdapterInfo = adapterInfo;

    return &m_vulkanAdapterInfo.header;
}

void ANFRHIVK::AddANFTechniqueCreateInfoRHI(AnfTechniqueCreateInfo* pCreateInfo)
{
    AnfTechniqueVulkanCreateInfo techniqueVkInfo = {};
    techniqueVkInfo.header.type = ANF_STYPE_TECHNIQUE_VULKAN_CREATE_INFO;

    m_vulkanTechniqueInfo = techniqueVkInfo;

    pCreateInfo->pClientApiCreateInfo = &m_vulkanTechniqueInfo.header;
}

AnfHandle ANFRHIVK::GetImageHandle(FRHITexture* image)
{
#if ENGINE_MINOR_VERSION > 0
    FVulkanTexture* vkImage = static_cast<FVulkanTexture*>(image);
    VkImage vkHandle = vkImage->Image;
#else
    FVulkanTexture2D* vkImage = (FVulkanTexture2D*)image->GetTexture2D();
    VkImage vkHandle = vkImage->Surface.Image;
#endif
    check(vkHandle != VK_NULL_HANDLE);
    return reinterpret_cast<AnfHandle>(vkHandle);
}

uint32_t ANFRHIVK::GetResourceParameterLayout(uint32_t resIndex)
{
    check(resIndex < ANFExternalResource::ANF_EXT_RESOURCE_COUNT);
    return static_cast<uint32>((resIndex == ANFExternalResource::ANF_EXT_RESOURCE_OUTPUT_COLOR) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}

AnfHandle ANFRHIVK::GetCommandList(FRHICommandListImmediate& RHICmdList)
{
    return reinterpret_cast<AnfHandle>(ANFGetVkCommandBuffer(RHICmdList));
}

void ANFRHIVK::AddExtensions(TArrayView<const ANSICHAR* const> deviceExtensions, TArrayView<const ANSICHAR* const> instanceExtensions)
{
#if ENGINE_MINOR_VERSION > 0
    IVulkanDynamicRHI::AddEnabledDeviceExtensionsAndLayers(deviceExtensions, TArray<ANSICHAR const*>());
    IVulkanDynamicRHI::AddEnabledInstanceExtensionsAndLayers(instanceExtensions, TArray<ANSICHAR const*>());
#else
    VulkanRHIBridge::AddEnabledDeviceExtensionsAndLayers(TArray<const ANSICHAR*>(deviceExtensions), TArray<ANSICHAR const*>());
    VulkanRHIBridge::AddEnabledInstanceExtensionsAndLayers(TArray<const ANSICHAR*>(instanceExtensions), TArray<ANSICHAR const*>());
#endif
}

bool ANFRHIVK::GetTechniqueExtensions(AnfTechniqueId techniqueID, const AnfStructHeader* pClientApiRequirements, TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions)
{
    check(pClientApiRequirements != nullptr);
    if (pClientApiRequirements->type != ANF_STYPE_TECHNIQUE_VULKAN_API_REQUIREMENTS)
    {
        UE_LOG(LogANFRHI, Error, TEXT("[ANF] TechniqueID %d unexpected type for pClientApiRequirements 0x%X"), techniqueID, pClientApiRequirements->type);
        return false;
    }

    const AnfTechniqueVulkanApiRequirements* pVkReqs = reinterpret_cast<const AnfTechniqueVulkanApiRequirements*>(pClientApiRequirements);

    for (unsigned int j = 0; j < pVkReqs->numDeviceExtensions; j++)
    {
        UE_LOG(LogANFRHI, Verbose, TEXT("[ANF] TechniqueID %d Adding required device extension %s"), techniqueID, ANSI_TO_TCHAR(pVkReqs->ppDeviceExtensions[j]));
        deviceExtensions.Push(pVkReqs->ppDeviceExtensions[j]);
    }

    instanceExtensions.Reserve(pVkReqs->numInstanceExtensions);
    for (unsigned int j = 0; j < pVkReqs->numInstanceExtensions; j++)
    {
        UE_LOG(LogANFRHI, Verbose, TEXT("[ANF] TechniqueID %d Adding required instance extension %s"), techniqueID, ANSI_TO_TCHAR(pVkReqs->ppInstanceExtensions[j]));
        instanceExtensions.Push(pVkReqs->ppInstanceExtensions[j]);
    }

    static const VkQueueFlags SupportedQueueFlags = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    const VkQueueFlags extraRequiredQueueFlags = (pVkReqs->techniqueQueueFlags & ~SupportedQueueFlags);
    if (extraRequiredQueueFlags != 0)
    {
        UE_LOG(LogANFRHI, Error, TEXT("[ANF] TechniqueID %d requires extra queue flags 0x%X unsupported by UE5"), techniqueID, extraRequiredQueueFlags);
        return false;
    }

    return true;
}

void ANFRHIVK::AddBlitTexturePass_Internal(FRHICommandListImmediate& RHICmdList, FRHITexture* inputTexture, FRHITexture* outputTexture, bool finalBlit)
{
    FRHITexture* inputTextureRHI = inputTexture;
    FRHITexture* outputTextureRHI = outputTexture;

    RHICmdList.EnqueueLambda([this, inputTextureRHI, outputTextureRHI, finalBlit](FRHICommandListImmediate& InCmdList) mutable
        {
#if ENGINE_MINOR_VERSION > 0
            const bool isDepth = inputTextureRHI->GetDesc().Format == PF_DepthStencil;
#else
            const bool isDepth = inputTextureRHI->GetFormat() == PF_DepthStencil;
#endif
            VkImageBlit blitInfo = {};
            if (isDepth)
            {
                blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else
            {
                blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            blitInfo.srcSubresource.layerCount = 1;
#if ENGINE_MINOR_VERSION > 0
            blitInfo.srcOffsets[1].x = inputTextureRHI->GetDesc().Extent.X;
            blitInfo.srcOffsets[1].y = inputTextureRHI->GetDesc().Extent.Y;
#else
            blitInfo.srcOffsets[1].x = inputTextureRHI->GetSizeXYZ().X;
            blitInfo.srcOffsets[1].y = inputTextureRHI->GetSizeXYZ().Y;
#endif
            blitInfo.srcOffsets[1].z = 1;
            if (isDepth)
            {
                blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else
            {
                blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            blitInfo.dstSubresource.layerCount = 1;
#if ENGINE_MINOR_VERSION > 0
            blitInfo.dstOffsets[1].x = outputTextureRHI->GetDesc().Extent.X;
            blitInfo.dstOffsets[1].y = outputTextureRHI->GetDesc().Extent.Y;
#else
            blitInfo.dstOffsets[1].x = outputTextureRHI->GetSizeXYZ().X;
            blitInfo.dstOffsets[1].y = outputTextureRHI->GetSizeXYZ().Y;
#endif
            blitInfo.dstOffsets[1].z = 1;

            VulkanRHI::vkCmdBlitImage(
                ANFGetVkCommandBuffer(InCmdList),
                reinterpret_cast<VkImage>(GetImageHandle(inputTextureRHI)),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                reinterpret_cast<VkImage>(GetImageHandle(outputTextureRHI)),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blitInfo,
                VK_FILTER_NEAREST
            );
        });
}

float ANFRHIVK::WriteNewTimeStamp(FRHICommandListImmediate& RHICmdList, uint32 passId, uint32 queryId, bool start)
{
    PassQueryData* curPassData = m_passQueryData.Find(passId);
#if ENGINE_MINOR_VERSION > 2
    FVulkanDevice* UEDevice = FVulkanDynamicRHI::Get().GetDevice();
#else
    FVulkanDevice* UEDevice = GVulkanRHI->GetDevice();
#endif
    VkDevice vkDevice = GetVkDeviceFromUEDevice(UEDevice);

    float returnTime = -1.f;

    if (curPassData == nullptr)
    {
        ensure(queryId == 0);

        PassQueryData& newEntry = m_passQueryData.FindOrAdd(passId);
        curPassData = &newEntry;

        VkQueryPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        poolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        poolInfo.queryCount = s_numQueries * 2;
        VkResult poolStatus = VulkanRHI::vkCreateQueryPool(
            vkDevice,
            &poolInfo,
            nullptr,
            &curPassData->queryPool
        );
        ensure(poolStatus == VK_SUCCESS);
    }
    ensure(curPassData != nullptr);

    if (start && curPassData->querySubmitted[queryId])
    {
        uint64 rawQueryData[2] = { 0, 0 };
        VkResult queryStatus = VulkanRHI::vkGetQueryPoolResults(
            vkDevice,
            curPassData->queryPool,
            queryId * 2,
            2,
            2 * sizeof(uint64),
            &rawQueryData,
            sizeof(uint64),
            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
        );

        ensure(queryStatus == VK_SUCCESS);

        const uint64 timestampDelta = rawQueryData[1] - rawQueryData[0];

        const VkPhysicalDeviceLimits& Limits = UEDevice->GetDeviceProperties().limits;
        const double queryTimeNS = static_cast<double>(timestampDelta) * static_cast<double>(Limits.timestampPeriod);
        returnTime = static_cast<float>(queryTimeNS * 1e-3);

        curPassData->querySubmitted[queryId] = false;
    }

    VkCommandBuffer cmdBuf = ANFGetVkCommandBuffer(RHICmdList);
    if (start)
    {
        VulkanRHI::vkCmdResetQueryPool(
            cmdBuf,
            curPassData->queryPool,
            queryId * 2,
            2
        );
    }
    else
    {
        curPassData->querySubmitted[queryId] = true;
    }

    VulkanRHI::vkCmdWriteTimestamp(
        cmdBuf,
        start ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        curPassData->queryPool,
        queryId * 2 + (start ? 0 : 1)
    );

    return returnTime;
}

ETextureCreateFlags ANFRHIVK::VkImageUsageFlagsToUECreateFlags(VkImageUsageFlags vkFlags)
{
    auto hasBitSet = [](VkImageUsageFlags flags, uint32 bit)
        {
            return ((flags & bit) == bit) ? true : false;
        };

    auto clearBit = [](VkImageUsageFlags flags, uint32 bit)
    {
        return (VkImageUsageFlags)(flags & ~bit);
    };

    ETextureCreateFlags retFlags = ETextureCreateFlags::None;

    VkImageUsageFlags tempFlags = vkFlags;
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT) || 
        hasBitSet(tempFlags, VK_IMAGE_USAGE_TRANSFER_DST_BIT) ||
        hasBitSet(tempFlags, VK_IMAGE_USAGE_SAMPLED_BIT))
    {
        retFlags |= TexCreate_ShaderResource;
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    {
        retFlags |= TexCreate_RenderTargetable;
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_STORAGE_BIT))
    {
        retFlags |= TexCreate_UAV;
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_STORAGE_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        retFlags |= TexCreate_DepthStencilTargetable;
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
    {
        retFlags |= TexCreate_InputAttachmentRead;
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    }

    if (tempFlags != 0)
    {
        UE_LOG(LogANFRHI, Error, TEXT("Unhandled VK Image usage flags 0x%X (%d)!"), tempFlags, tempFlags);
    }

    return retFlags;
}

FString VkImageUsageFlagsToString(VkImageUsageFlags vkFlags)
{
    auto hasBitSet = [](VkImageUsageFlags flags, uint32 bit)
        {
            return ((flags & bit) == bit) ? true : false;
        };

    auto clearBit = [](VkImageUsageFlags flags, uint32 bit)
        {
            return (VkImageUsageFlags)(flags & ~bit);
        };

    FString retString;

    VkImageUsageFlags tempFlags = vkFlags;
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT) ||
        hasBitSet(tempFlags, VK_IMAGE_USAGE_TRANSFER_DST_BIT) ||
        hasBitSet(tempFlags, VK_IMAGE_USAGE_SAMPLED_BIT))
    {
        retString += TEXT("TexCreate_ShaderResource|");
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    {
        retString += TEXT("TexCreate_RenderTargetable|");
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_STORAGE_BIT))
    {
        retString += TEXT("TexCreate_UAV|");
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_STORAGE_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        retString += TEXT("TexCreate_DepthStencilTargetable|");
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    if (hasBitSet(tempFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
    {
        retString += TEXT("TexCreate_InputAttachmentRead|");
        tempFlags = clearBit(tempFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    }

    if (tempFlags != 0)
    {
        retString += FString::Printf(TEXT("UNKNOWN(0x%08X)"), tempFlags);
    }

    return retString;
}

void ANFRHIVK::RHISetResourceRequirements(AnfTechniqueId techniqueID, AnfResourceLabel label, const AnfResourceRequirements* pReqs)
{
    check(pReqs != nullptr);
    if (pReqs->pClientApiRequirements->type != ANF_STYPE_RESOURCE_VULKAN_API_REQUIREMENTS)
    {
        UE_LOG(LogANFRHI, Fatal, TEXT("Invalid pClientApiRequirements type 0x%X!"), pReqs->pClientApiRequirements->type);
        return;
    }
    const AnfResourceVulkanApiRequirements* pVulkanReqs = reinterpret_cast<const AnfResourceVulkanApiRequirements*>(pReqs->pClientApiRequirements);

    const bool isFg = (techniqueID == ANF_TECHNIQUE_ID_FG_TEMPORAL);
    const ResourceID curResource = ResourceID(label, isFg);

    ImageUsageFlags curUsageFlags = ImageUsageFlags(pVulkanReqs->imageUsageFlags);

    if (pVulkanReqs->externalMemoryType != 0)
    {
        curUsageFlags.ueFlags |= TexCreate_External;
    }

    m_resourceUsageFlags.Emplace(curResource.ID, curUsageFlags);

    UE_LOG(LogANFRHI, Verbose, TEXT("Technique %d Label %d: UE Create Flags (0x%X), VK Usage Flags (0x%X), External = %s, %s"),
        techniqueID,
        label,
        m_resourceUsageFlags[curResource.ID].ueFlags,
        m_resourceUsageFlags[curResource.ID].vkFlags,
        (pVulkanReqs->externalMemoryType != 0) ? TEXT("TRUE") : TEXT("FALSE"),
        *VkImageUsageFlagsToString(m_resourceUsageFlags[curResource.ID].vkFlags));

    // Since FG directly writes to the backbuffer image, tell UE 5 to set these usages
    if (isFg && label == ANF_RESOURCE_LABEL_OUTPUT_COLOR)
    {
        FVulkanSwapChain::SetExtraBackbufferUsageFlags(
            m_resourceUsageFlags[curResource.ID].ueFlags,
            m_resourceUsageFlags[curResource.ID].vkFlags);
    }
}

bool ANFRHIVK::Is32BitDepthFormat(FRHITexture* image)
{
#if ENGINE_MINOR_VERSION > 0
    FVulkanTexture* vkImage = static_cast<FVulkanTexture*>(image);
    VkFormat vkFormat = vkImage->StorageFormat;
#else
    FVulkanTexture2D* vkImage = (FVulkanTexture2D*)image->GetTexture2D();
    VkFormat vkFormat = vkImage->Surface.ViewFormat;
#endif
    return (vkFormat == VK_FORMAT_D32_SFLOAT_S8_UINT);
}