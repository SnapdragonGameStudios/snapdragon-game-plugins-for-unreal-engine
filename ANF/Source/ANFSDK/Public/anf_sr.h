//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

/**
********************************************************************************************************************************
* @file  anf_sr.h
* @brief ANF Super Resolution Header
********************************************************************************************************************************
*/

#ifndef ANF_SR_H
#define ANF_SR_H

#include "anf_types.h"

/*******************************************************************************************************************************
*   @brief
*       Super resolution quality mode
*******************************************************************************************************************************/
enum AnfSRQualityMode : uint32_t
{
    ANF_SR_QUALITY_MODE_PERFORMANCE                = 0,    ///< Per-dimension super resolution ratio of 2.0x.
};

/*******************************************************************************************************************************
*   @brief
*       Super resolution technique group create info
*******************************************************************************************************************************/
struct AnfSRCreateInfo
{
    AnfStructHeader             header;             ///< Standard struct header

    AnfDim2D                    inputSize;          ///< The size of the input resources.
    AnfDim2D                    outputSize;         ///< The size of the output resource.
};

/*******************************************************************************************************************************
*   @brief
*       Super resolution dispatch information
*******************************************************************************************************************************/
struct AnfSRDispatch
{
    AnfStructHeader                  header;                     ///< Standard struct header

    AnfFloat2                        jitterOffset;               ///< The subpixel jitter offset applied to the camera. Values
                                                                  ///< are in range [-0.5, 0.5] in x and y dimensions.
};

/*******************************************************************************************************************************
*   @brief
*       Super resolution technique requirements
*******************************************************************************************************************************/
struct AnfSRRequirements
{
    AnfStructHeader                  header;                     ///< Standard struct header

    uint32_t                         supportedQualityModes;      ///< Bitfield of supported quality modes
};

#endif // ANF_SR_H