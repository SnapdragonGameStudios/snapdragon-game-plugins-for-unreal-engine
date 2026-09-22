//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

/**
********************************************************************************************************************************
* @file  anf_types_vk.h
* @brief ANF API Vulkan specific types
********************************************************************************************************************************
*/

#ifndef ANF_TYPES_VK_H
#define ANF_TYPES_VK_H

#include "anf_types.h"
#include <vulkan/vulkan.h>

/*******************************************************************************************************************************
*   @brief
*       Adapter info for Vulkan
*******************************************************************************************************************************/
struct AnfAdapterInfoVulkan
{
    AnfStructHeader     header;             ///< Standard struct header
    VkInstance          instance;           ///< Vulkan instance
    VkPhysicalDevice    physicalDevice;     ///< Vulkan physical device
    VkDevice            device;             ///< Vulkan logical device
};

/*******************************************************************************************************************************
*   @brief
*       Queue info for Vulkan.  Extends AnfAdapterInfoVulkan
*******************************************************************************************************************************/
struct AnfQueueInfoVulkan
{
    AnfStructHeader     header;              ///< Standard struct header
    uint32_t            numQueues;           ///< Number of queues provided
    VkQueue*            pQueues;             ///< Array of size numQueues. Contains VkQueue handles this instance will submit to.
    uint32_t*           pQueueFamilyIndices; ///< Array of size numQueues. Contains the queue family index of the corresponding 
                                              ///< VkQueue in pQueues.
};

/*******************************************************************************************************************************
*   @brief
*       Requirements for a given technique that are specific to the Vulkan client API
*******************************************************************************************************************************/
struct AnfTechniqueVulkanApiRequirements
{
    AnfStructHeader     header;                 ///< Standard struct header

    // Required Vulkan instance extensions
    uint32_t            numInstanceExtensions;  ///< Number of required Vulkan instance extensions
    const char* const*  ppInstanceExtensions;   ///< Array of char* values, each pointing to a null-terminated char array string
                                                 ///< with the name of a required Vulkan instance extension

    // Required Vulkan device extensions
    uint32_t            numDeviceExtensions;    ///< Number of required Vulkan device extensions
    const char* const*  ppDeviceExtensions;     ///< Array of char* values, each pointing to a null-terminated char array string
                                                 ///< with the name of a required Vulkan device extension

    VkQueueFlags        techniqueQueueFlags;    ///< The queue flags this technique's operations require.  
                                                 ///< The VkCommandBuffer provided to DispatchTechnique must be submitted to a
                                                 ///< queue whose QueueFamilyIndex supports all of these flags.
                                                 ///< If using the flag ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE, then an
                                                 ///< AnfVulkanQueueInfo struct must have been in the pNext chain of the 
                                                 ///< AnfSetClientAdapterInfo and for each bit set in techniqueQueueFlags, 
                                                 ///< contain a queue supporting that flag.
};

/*******************************************************************************************************************************
*   @brief
*       Requirements for a given resource to be used in the queried technique that are specific to the Vulkan client API
*******************************************************************************************************************************/
struct AnfResourceVulkanApiRequirements
{
    AnfStructHeader     header;                             ///< Standard struct header

    union /// usage flags depending on resource type
    {
        VkImageUsageFlags               imageUsageFlags;    ///< Image usage flags
        VkBufferUsageFlags              bufferUsageFlags;   ///< Buffer usage flags
    };

    VkExternalMemoryHandleTypeFlags     externalMemoryType; ///< External memory type.  If != 0 the resource must be bound to
                                                             ///< external memory
};

/*******************************************************************************************************************************
*   @brief
*       Technique create info specific to Vulkan
*******************************************************************************************************************************/
struct AnfTechniqueVulkanCreateInfo
{
    AnfStructHeader     header;                 ///< Standard struct header
};

#endif // ANF_TYPES_VK_H