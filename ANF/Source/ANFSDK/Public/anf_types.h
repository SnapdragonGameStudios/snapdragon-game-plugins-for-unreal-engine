//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

/**
********************************************************************************************************************************
* @file  anf_types.h
* @brief ANF API Types Header
********************************************************************************************************************************
*/

#ifndef ANF_TYPES_H
#define ANF_TYPES_H

#include <stdint.h>
#include <vulkan/vulkan.h>

/*******************************************************************************************************************************
*   @brief
*       Convenient for checking whether an enum index is set in the given bitfield.
*       Returns a logical boolean.
*******************************************************************************************************************************/
#define ANF_IS_BIT_INDEX_SET(_bitfield, _index)   ((( (1 << (_index)) ) & (_bitfield)) != 0)
#define ANF_TYPED_HANDLE(objectType) typedef struct objectType##_T* objectType;

#define ANF_FALSE    0
#define ANF_TRUE     1

typedef     uint32_t        AnfBool;               ///< Boolean value, should be one of ANF_FALSE or ANF_TRUE
typedef     uint64_t        AnfGpuAddr;            ///< GPU address
typedef     uint32_t        AnfFlags32;            ///< 32-bit bitfield of flags
typedef     uint64_t        AnfFlags64;            ///< 64-bit bitfield of flags

ANF_TYPED_HANDLE(AnfInstance);                    ///< ANF instance handle
ANF_TYPED_HANDLE(AnfTechnique);                   ///< ANF technique handle

typedef     void*           AnfHandle;             ///< ANF general handle type
/// (Client API specific handle types)
typedef     AnfHandle       AnfClientSyncObject;   ///< ANF sync object - 
                                                    ///< Vulkan: VkSemaphore

typedef     AnfHandle       AnfClientCommandList;  ///< ANF command list -
                                                    ///< Vulkan: VkCommandBuffer

typedef     AnfHandle       AnfClientResource;     ///< ANF resource -
                                                    ///< Vulkan: VkImage, VkBuffer, VkTensorARM

/*******************************************************************************************************************************
*   @brief
*       Structure type
*******************************************************************************************************************************/
enum AnfStructType : uint32_t
{
    ANF_STYPE_SR_CREATE_INFO,
    ANF_STYPE_SR_DISPATCH,
    ANF_STYPE_SR_REQUIREMENTS,
    ANF_STYPE_FG_CREATE_INFO,
    ANF_STYPE_FG_DISPATCH,
    ANF_STYPE_FG_REQUIREMENTS,
    ANF_STYPE_ADAPTER_INFO_VULKAN,
    ANF_STYPE_TECHNIQUE_VULKAN_API_REQUIREMENTS,
    ANF_STYPE_INSTANCE_CREATE_INFO,
    ANF_STYPE_TECHNIQUE_REQUIREMENTS,
    ANF_STYPE_RESOURCE_REQUIREMENTS,
    ANF_STYPE_RESOURCE_DESC,
    ANF_STYPE_RESOURCE_PARAM_DESC,
    ANF_STYPE_DISPATCH_IMMEDIATE_RESOURCE_INFO_VK_IMAGE,
    ANF_STYPE_TECHNIQUE_VULKAN_CREATE_INFO,
    ANF_STYPE_TECHNIQUE_CREATE_INFO,
    ANF_STYPE_MODEL_WEIGHTS_INFO,
    ANF_STYPE_RESOURCE_VULKAN_API_REQUIREMENTS,
    ANF_STYPE_QUEUE_INFO_VULKAN,
    ANF_STYPE_DEBUG_OVERLAY_CONFIG,
    ANF_STYPE_TECHNIQUE_DISPATCH_INFO,
    ANF_STYPE_TECHNIQUE_DISPATCH_IMMEDIATE_INFO,
    ANF_STYPE_APPLICATION_INFO,
};

/*******************************************************************************************************************************
*   @brief
*       Base struct header. Most structs must have this at the start of their declaration.
*******************************************************************************************************************************/
struct AnfStructHeader
{
    AnfStructType      type;       ///< The structure type. Must be the correct type for the given structure.
    AnfStructHeader*   pNext;      ///< Pointer to the next structure. Can be used to extend the current structure. Can be
                                    ///< NULL.
};

/*******************************************************************************************************************************
*   @brief
*       Format.
*
*       Some general definitions:
*       - UNORM  - unsigned normalized
*       - FLOAT  - signed floating point
*       - UFLOAT - unsigned floating point
*******************************************************************************************************************************/
enum AnfFormat : uint32_t
{
    ANF_FORMAT_UNKNOWN,                        ///< Unknown format
    ANF_FORMAT_R8G8B8A8_UNORM,                 ///< 8-bit unsigned normalized RGBA
    ANF_FORMAT_B10G11R11_UFLOAT,               ///< Unsigned floating point values; B 10 bits, G 11 bits, R 11 bits - same as
                                                ///< VK_FORMAT_B10G11R11_UFLOAT_PACK32
    ANF_FORMAT_D32_FLOAT,                      ///< 32-bit float depth
    ANF_FORMAT_D24S8_UNORM,                    ///< 24-bit unsigned normalized depth and 8 bit unsigned normalized stencil
    ANF_FORMAT_R16G16_FLOAT,                   ///< 16-bit float RG
    ANF_FORMAT_R16G16B16A16_FLOAT,             ///< 16-bit float RGBA
};

/*******************************************************************************************************************************
*   @brief
*       Resource type
*******************************************************************************************************************************/
enum AnfResourceType : uint32_t
{
    ANF_RESOURCE_TYPE_BUFFER,                       ///< Buffer
    ANF_RESOURCE_TYPE_TEXTURE1D,                    ///< Texture 1D
    ANF_RESOURCE_TYPE_TEXTURE2D,                    ///< Texture 2D
    ANF_RESOURCE_TYPE_TEXTURE_CUBE,                 ///< Texture cubemap
    ANF_RESOURCE_TYPE_TEXTURE3D,                    ///< Texture 3D
};

/*******************************************************************************************************************************
*   @brief
*       Technique ID
*******************************************************************************************************************************/
enum AnfTechniqueId : uint32_t
{
    ANF_TECHNIQUE_ID_SR_TEMPORAL           = 0,        ///< ANF Temporal Super Resolution
    ANF_TECHNIQUE_ID_FG_TEMPORAL           = 1         ///< ANF Temporal Frame Generation
};

/*******************************************************************************************************************************
*   @brief
*       ANF return result
*******************************************************************************************************************************/
enum AnfResult : uint32_t
{
    ANF_RESULT_SUCCESS                         = 0,        ///< Success
    ANF_RESULT_ERROR                           = 1,        ///< Error: error type is unspecified
    ANF_RESULT_ERROR_UNKNOWN_STYPE             = 2,        ///< Error: unknown structure type encountered.
    ANF_RESULT_ERROR_CLIENT_API                = 3,        ///< Error in the client API (Vulkan, GLES, etc.)
    ANF_RESULT_ERROR_MEMORY                    = 4,        ///< Error: memory allocation failure
    ANF_RESULT_ERROR_INVALID_PARAMETER         = 5,        ///< Error: invalid parameter
    ANF_RESULT_NOT_SUPPORTED                   = 6,        ///< Something is not supported
};

/*******************************************************************************************************************************
*   @brief
*       Client API
*******************************************************************************************************************************/
enum AnfClientApi : uint32_t
{
    ANF_CLIENT_API_VULKAN              = 0,            ///< Vulkan
};

/*******************************************************************************************************************************
*   @brief
*       Log level
*******************************************************************************************************************************/
enum AnfLogLevel : uint32_t
{
    ANF_LOG_LEVEL_NONE             = 0,           ///< None; used only for specifying log level at create time
    ANF_LOG_LEVEL_ERROR            = 1,           ///< Error
    ANF_LOG_LEVEL_WARNING          = 2,           ///< Warning
    ANF_LOG_LEVEL_INFO             = 3,           ///< Info
};

/*******************************************************************************************************************************
*   @brief
*       Version information
*******************************************************************************************************************************/
struct AnfVersion
{
    uint32_t            major;              ///< Major version
    uint32_t            minor;              ///< Minor version
    uint32_t            build;              ///< Build version
};

/*******************************************************************************************************************************
*   @brief
*       Log message callback function
*******************************************************************************************************************************/
typedef void (*AnfLogMessageCb)(
    AnfLogLevel     logLevel,                   ///< Log level passed to the callback; will never be ANF_LOG_LEVEL_NONE
    const char*     pMessage);                  ///< Message passed to the callback

typedef AnfFlags32 AnfInstanceCreateFlags;    ///< ANF instance create flags bitfield

/*******************************************************************************************************************************
*   @brief
*       ANF application info
*******************************************************************************************************************************/
struct AnfApplicationInfo
{
    AnfStructHeader         header;             ///< Standard struct header

    const char*             pApplicationName;   ///< NULL or string containing the name of the application
    uint32_t                applicationVersion; ///< Version of the application
    const char*             pEngineName;        ///< NULL or string containing the name of the engine the application uses
    uint32_t                engineVersion;      ///< Version of the engine the application uses
};

/*******************************************************************************************************************************
*   @brief
*       ANF instance create flags
*******************************************************************************************************************************/
enum AnfInstanceCreateFlag : AnfInstanceCreateFlags
{
    ANF_INSTANCE_CREATE_FLAG_IS_SYSTEM            = (1 << 0),   ///< Indicates if ANF is being used in a system service
    ANF_INSTANCE_CREATE_FLAG_ENABLE_DEBUG_OVERLAY = (1 << 1),  ///< Enable the debug visualization overlay.
};

/*******************************************************************************************************************************
*   @brief
*       ANF instance create info
*******************************************************************************************************************************/
struct AnfInstanceCreateInfo
{
    AnfStructHeader             header;                 ///< Standard struct header
    AnfLogMessageCb             logMessageCallback;     ///< Log message callback
    AnfLogLevel                 logLevel;               ///< Write all logs for this level and below; use
                                                         ///< ANF_LOG_LEVEL_NONE to turn off all logging.
    AnfInstanceCreateFlags      flags;                  ///< Instance creation flags
    AnfClientApi                clientApi;              ///< Client API which this instance will be used with.
                                                         ///< Determines which client API structs will be expected elsewhere.
    const AnfApplicationInfo*   pApplicationInfo;       ///< NULL or pointer to the application info
};

/*******************************************************************************************************************************
*   @brief
*       Resource label - indicates how a resource is used in the technique
*******************************************************************************************************************************/
enum AnfResourceLabel : uint32_t
{
    ANF_RESOURCE_LABEL_DEPTH                     = 0,        ///< Depth buffer
    ANF_RESOURCE_LABEL_MOTION_VECTORS            = 1,        ///< Motion vectors
    ANF_RESOURCE_LABEL_INPUT_COLOR               = 2,        ///< Input color
    ANF_RESOURCE_LABEL_OUTPUT_COLOR              = 3,        ///< Output color
};

/*******************************************************************************************************************************
*   @brief
*       Common structure for technique requirements
*******************************************************************************************************************************/
struct AnfTechniqueRequirements
{
    AnfStructHeader                 header;                         ///< Standard struct header

    AnfVersion                      version;                        ///< Technique implementation version

    const AnfResourceLabel*         pRequiredResources;             ///< Array of required resource labels of size
                                                                     ///< numRequiredResources
    uint32_t                        numRequiredResources;           ///< Number of required resource labels

    const AnfResourceLabel*         pOptionalResources;             ///< Array of optional resource labels of size
                                                                     ///< numOptionalResources
    uint32_t                        numOptionalResources;           ///< Number of optional resource labels

    const AnfStructHeader*          pClientApiRequirements;         ///< Requirements specific to the particular client API
                                                                     ///< as determined by AnfInstance

    const AnfStructHeader*          pTechniqueGroupRequirements;    ///< Pointer to struct of requirements for the techniquue
                                                                     ///< group that this technique belongs to.
};

/*******************************************************************************************************************************
*   @brief
*       Requirements for a given resource
*******************************************************************************************************************************/
struct AnfResourceRequirements
{
    AnfStructHeader                 header;                    ///< Standard struct header

    AnfResourceType                 resourceType;              ///< Resource type

    const AnfFormat*                pSupportedFormats;         ///< Array of supported formats for this resource
    uint32_t                        numSupportedFormats;       ///< Number of supported formats for this resource

    const AnfStructHeader*          pClientApiRequirements;    ///< Pointer to struct filling out additional requirements for a
                                                                ///< particular client API
};

/*******************************************************************************************************************************
*   @brief
*       2D (width, height)
*******************************************************************************************************************************/
struct AnfDim2D
{
    uint32_t    width;      ///< Width
    uint32_t    height;     ///< Height
};

/*******************************************************************************************************************************
*   @brief
*       2D floating point coordinate
*******************************************************************************************************************************/
struct AnfFloat2
{
    float   x;          ///< X coordinate
    float   y;          ///< Y coordinate
};

/*******************************************************************************************************************************
*   @brief
*       Debug visualization overlay mode. Selects what the overlay composites onto the technique output.
*       Technique-agnostic (currently only supports SR).
*******************************************************************************************************************************/
enum AnfDebugOverlayMode : uint32_t
{
    // Per-mode knobs are noted below; hudCorner/hudFlipY apply to every mode.
    ANF_DEBUG_OVERLAY_MODE_NONE              = 0,  ///< No overlay drawn this dispatch.
    ANF_DEBUG_OVERLAY_MODE_INPUT_COLOR       = 1,  ///< Bilinear-upsampled jittered input color, replacing the output (technique still runs). Knobs: none.
    ANF_DEBUG_OVERLAY_MODE_MV_HEATMAP        = 2,  ///< HSV heatmap of the motion-vector field blended over the output. Knobs: mvHeatmapScale, showRawMv, mvValueScale.
    ANF_DEBUG_OVERLAY_MODE_DEPTH_VIS         = 3,  ///< Depth buffer color ramp. Shows depth discontinuities. Knobs: depthInvert, depthScale.
    ANF_DEBUG_OVERLAY_MODE_WARP_PREDICT      = 4,  ///< Previous output warped by MVs. Shows warp geometry. Knobs: mvValueScale.
    ANF_DEBUG_OVERLAY_MODE_REPROJECT_ERROR   = 5,  ///< Warp-vs-current error heat map: blue=good MVs, red=bad. Knobs: mvValueScale.
    ANF_DEBUG_OVERLAY_MODE_JITTER_PLOT       = 6,  ///< Scatter plot of recent jitter offsets. Knobs: none.
    ANF_DEBUG_OVERLAY_MODE_JITTER_ACCUMULATE = 7,  ///< Phase-based jitter accumulation; shows sub-pixel coverage correctness. Knobs: accumulateJitter, jitterAccumAlpha, jitterScale (sign only).
    ANF_DEBUG_OVERLAY_MODE_SR_JITTER_SCALE   = 8,  ///< Output with jitter scaled by jitterScale before neural dispatch. Knobs: jitterScale (value).
};

/*******************************************************************************************************************************
*   @brief
*       HUD corner position for the debug overlay.
*******************************************************************************************************************************/
enum AnfDebugOverlayHudCorner : uint32_t
{
    ANF_DEBUG_OVERLAY_HUD_CORNER_TOP_LEFT     = 0,
    ANF_DEBUG_OVERLAY_HUD_CORNER_TOP_RIGHT    = 1,
    ANF_DEBUG_OVERLAY_HUD_CORNER_BOTTOM_LEFT  = 2,
    ANF_DEBUG_OVERLAY_HUD_CORNER_BOTTOM_RIGHT = 3,
};

/*******************************************************************************************************************************
*   @brief
*       Debug visualization overlay configuration. Optional; chain onto a technique dispatch struct's
*       pNext (e.g. AnfSRDispatch, AnfFGDispatch). Settings persist across dispatches: if present, it replaces
*       the current settings, absent keeps the last-applied ones; mode = ANF_DEBUG_OVERLAY_MODE_NONE
*       stops drawing. Ignored unless the overlay was enabled at create time via ANF_INSTANCE_CREATE_FLAG_ENABLE_DEBUG_OVERLAY.
*******************************************************************************************************************************/
struct AnfDebugOverlayConfig
{
    AnfStructHeader          header;             ///< Standard struct header

    AnfDebugOverlayMode      mode;               ///< NONE = stop drawing; 1-8 = visualization mode
    float                    mvHeatmapScale;     ///< MV_HEATMAP color saturation scale
    AnfBool                  showRawMv;          ///< MV_HEATMAP: raw direction-colored channels instead of blended heatmap
    AnfFloat2                mvValueScale;       ///< Per-axis multiplier on raw MV (x,y). Modes 2/4/5.
    AnfFloat2                jitterScale;        ///< Jitter sign/scale (x,y). Modes 7 (sign) and 8 (value).
    float                    jitterAccumAlpha;   ///< JITTER_ACCUMULATE blend alpha (0..1)
    AnfBool                  accumulateJitter;   ///< JITTER_ACCUMULATE enable
    AnfDebugOverlayHudCorner hudCorner;        ///< HUD corner position (TOP_LEFT default)
    AnfBool                  hudFlipY;           ///< Flip HUD text for Y-inverted framebuffers
    AnfBool                  depthInvert;        ///< DEPTH_VIS: invert depth before colorizing
    uint32_t                 depthScale;         ///< DEPTH_VIS: integer depth multiplier (1-99)
};

/*******************************************************************************************************************************
*   @brief
*       Buffer resource size and layout information
*******************************************************************************************************************************/
struct AnfResourceBufferDims
{
    uint32_t    sizeInBytes;            ///< The size of the buffer resource in bytes.
    uint32_t    strideInBytes;          ///< The stride of the buffer resource in bytes.
    uint32_t    alignmentInBytes;       ///< The alignment of the buffer resource in bytes.
};

/*******************************************************************************************************************************
*   @brief
*       Texture resource size information
*******************************************************************************************************************************/
struct AnfResourceTextureDims
{
    uint32_t width;                 ///< Texture width
    uint32_t height;                ///< Texture height

    union
    {
        uint32_t depth;             ///< 3D texture depth
        uint32_t numArrayLayers;    ///< Number of array layers
    };

    uint32_t mipCount;              ///< Number of mips, or 0 if doing a full mip chain
};

/*******************************************************************************************************************************
*   @brief
*       Resource description
*******************************************************************************************************************************/
struct AnfResourceDesc
{
    AnfStructHeader                 header;                 ///< Standard struct header

    AnfResourceType                 type;                   ///< The type of the resource.
    AnfFormat                       format;                 ///< The resource format.
    uint32_t                        layout;                 ///< If a texture, the layout.
                                                             ///< For Vulkan this is interpretted as VkImageLayout

    union
    {
        AnfResourceBufferDims       bufferDims;             ///< If a buffer, the size info
        AnfResourceTextureDims      textureDims;            ///< If a texture, the size info
    };

    AnfClientResource               resource;               ///< Handle for the resource; one of: VkBuffer, VkImage
};

/*******************************************************************************************************************************
*   @brief
*       Description of a resource when used as a parameter.
*******************************************************************************************************************************/
struct AnfResourceParamDesc
{
    AnfStructHeader          header;             ///< Standard struct header

    AnfResourceLabel         resourceLabel;      ///< Indicates how this resource will be used.
    AnfResourceDesc          resourceDesc;       ///< Resource description
};

typedef AnfFlags64 AnfTechniqueCreateFlags;   ///< Technique create flags bitfield; empty for now

/*******************************************************************************************************************************
*   @brief
*       Technique create flags
*******************************************************************************************************************************/
enum AnfTechniqueCreateBit : AnfTechniqueCreateFlags
{
    ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE = (1 << 0),   ///< Technique will dispatch in immediate mode instead of
                                                                ///  writing inline to command lists. If set, then for
                                                                ///  later dispatches the command list handle will be
                                                                ///  ignored.
                                                                ///
                                                                ///  NOTE - This is the only mode currently supported for
                                                                ///         ANF_TECHNIQUE_ID_FG_TEMPORAL.
};

/*******************************************************************************************************************************
*   @brief
*       Common create info struct for techniques.
*
*       This contains create info common to all technique groups.
*******************************************************************************************************************************/
struct AnfTechniqueCreateInfo
{
    AnfStructHeader             header;                     ///< Standard struct header

    AnfTechniqueId              techniqueId;                ///< Technique ID of technique to create
    AnfTechniqueCreateFlags     flags;                      ///< Common technique create flags
    uint32_t                    maxInFlight;                ///< Maximum number of dispatches allowed to be in progress at once

    const AnfStructHeader*      pClientApiCreateInfo;       ///< Pointer to technique create info that is specific to the client
                                                             ///< API indicated in "clientApi" used for the instance.
                                                             ///< For Vulkan, this struct must be AnfTechniqueVulkanCreateInfo

    const AnfStructHeader*      pTechniqueGroupCreateInfo;  ///< Create info specific to the technique group
};

/*******************************************************************************************************************************
*   @brief
*       Common dispatch info info struct for techniques.
*
*       This contains create info common to all technique groups.
*******************************************************************************************************************************/
struct AnfTechniqueDispatchInfo
{
    AnfStructHeader                 header;                         ///< Standard struct header

    AnfClientCommandList             commandList;                    ///< "Command list" to record this techniques commands
                                                                     ///< into. The type of handle passed differs depending on
                                                                     ///< the client API. For Vulkan, should be a VkCommandBuffer
                                                                     ///< If in immediate dispatch mode, this should be NULL.

    const AnfResourceParamDesc*     pResources;                     ///< Resources for the dispatch
    uint32_t                        numResources;                   ///< Num resources

    AnfBool                         reset;                          ///< When ANF_TRUE, this indicates that the technique state
                                                                     ///< can be reset.

    const AnfStructHeader*          pTechniqueGroupDispatchInfo;    ///< Info specific to this technique group
};

/*******************************************************************************************************************************
*   @brief
*       Sync objects for the technique dispatch to wait on and signal when using ANF_TECHNIQUE_CREATE_FLAG_IMMEDIATE_DISPATCH
*
*       This struct extends AnfTechniqueDispatchInfo
*******************************************************************************************************************************/
struct AnfTechniqueDispatchImmediateInfo
{
    AnfStructHeader                 header;                         ///< Standard struct header

    // For dispatch immediate mode, the array of sync objects that the dispatch should wait on before executing
    const AnfClientSyncObject*      pWaitSyncObjects;
    uint32_t                        numWaitSyncObjects;

    // For dispatch immediate mode, the array of sync objects that the dispatch should signal after executing
    const AnfClientSyncObject*      pSignalSyncObjects;
    uint32_t                        numSignalSyncObjects;
};

/*******************************************************************************************************************************
*   @brief
*       Model weights info for using fine tuned weights
*
*       This struct extends AnfTechniqueCreateInfo (should be in the pNext pointer of AnfTechniqueCreateInfo)
*******************************************************************************************************************************/
struct AnfModelWeightsInfo
{
    AnfStructHeader             header;                     ///< Standard struct header

    const char*                 pWeightsPath;               ///< Path to updated weights on device
};

#endif // ANF_TYPES_H