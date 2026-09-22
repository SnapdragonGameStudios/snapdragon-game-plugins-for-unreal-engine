//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#pragma once

#include "ANFRHI.h"
#include "ANFBackend.h"
#include "VulkanRHIPrivate.h"

#include "anf_types_vk.h"

class ANFAPI_API ANFRHIVK : public ANFRHI
{
public:
	ANFRHIVK() {}
	virtual ~ANFRHIVK();

	virtual AnfStructHeader* GetAdapterInfo() override;
	virtual void AddANFTechniqueCreateInfoRHI(AnfTechniqueCreateInfo* pCreateInfo) override;
	virtual AnfHandle GetImageHandle(FRHITexture* image) override;
	virtual uint32 GetResourceParameterLayout(uint32_t resIndex) override;
	virtual AnfHandle GetCommandList(FRHICommandListImmediate& RHICmdList) override;

	virtual void AddExtensions(TArrayView<const ANSICHAR* const> deviceExtensions, TArrayView<const ANSICHAR* const> instanceExtensions) override;

	virtual bool Is32BitDepthFormat(FRHITexture* image) override;

	virtual ETextureCreateFlags GetExtraFlags_DepthInput() override
	{
		return m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_DEPTH, true).ID].ueFlags | 
			m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_DEPTH, false).ID].ueFlags;
	}
	virtual ETextureCreateFlags GetExtraFlags_MotionInput() override
	{
		return m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_MOTION_VECTORS, true).ID].ueFlags |
			m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_MOTION_VECTORS, false).ID].ueFlags;
	}
	virtual ETextureCreateFlags GetExtraFlags_ColorInput(bool fg) override
	{
		return m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_INPUT_COLOR, fg).ID].ueFlags;
	}
	virtual ETextureCreateFlags GetExtraFlags_ColorOutput(bool fg) override
	{
		return m_resourceUsageFlags[ResourceID(ANF_RESOURCE_LABEL_OUTPUT_COLOR, fg).ID].ueFlags;
	}

	virtual bool GetTechniqueExtensions(AnfTechniqueId techniqueID, const AnfStructHeader* pClientApiRequirements, TArray<ANSICHAR const*>& deviceExtensions, TArray<ANSICHAR const*>& instanceExtensions) override;
	virtual AnfClientApi GetClientAPI() override
	{
		return ANF_CLIENT_API_VULKAN;
	}
private:
	virtual void AddBlitTexturePass_Internal(FRHICommandListImmediate& RHICmdList, FRHITexture* inputTexture, FRHITexture* outputTexture, bool finalBlit) override;
	virtual float WriteNewTimeStamp(FRHICommandListImmediate& RHICmdList, uint32 passId, uint32 queryId, bool start) override;

	virtual void RHISetResourceRequirements(AnfTechniqueId techniqueID, AnfResourceLabel label, const AnfResourceRequirements* pReqs) override;

	static ETextureCreateFlags VkImageUsageFlagsToUECreateFlags(VkImageUsageFlags vkFlags);

	AnfTechniqueVulkanCreateInfo m_vulkanTechniqueInfo;
	AnfAdapterInfoVulkan m_vulkanAdapterInfo;

	struct PassQueryData
	{
		VkQueryPool queryPool;
		bool querySubmitted[ANFRHI::s_numQueries];

		PassQueryData()
			: queryPool(VK_NULL_HANDLE)
		{
			FMemory::Memzero(querySubmitted, sizeof(querySubmitted));
		}
	};

	struct ImageUsageFlags
	{
		VkImageUsageFlags vkFlags;
		ETextureCreateFlags ueFlags;

		ImageUsageFlags(VkImageUsageFlags inFlags)
			: vkFlags(inFlags)
		{
			ueFlags = VkImageUsageFlagsToUECreateFlags(inFlags);
		}
	};

	struct ResourceID
	{
		union
		{
			uint16 ID;
			struct
			{
				AnfResourceLabel label : 15;
				bool fg : 1;
			};
		};

		ResourceID() : ID(0) {}
		ResourceID(uint32 inID) : ID(inID) {}
		ResourceID(AnfResourceLabel inLabel, bool isFg)
			: label(inLabel)
			, fg(isFg)
		{
		}
	};

	TMap<uint32, ImageUsageFlags> m_resourceUsageFlags;

	TMap<uint32, PassQueryData> m_passQueryData;

};