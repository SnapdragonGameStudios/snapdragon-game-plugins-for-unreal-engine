//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERTILECLASSIFICATION_H_
#define _QCOMSHADOWDENOISERTILECLASSIFICATION_H_

//
// Determine tile level (8x8) attributes - used to optimize denoising of fully lit or fully shadowed tiles (8x8 pixels).
//

// Inputs:  image with bitmask representing the 'shadowed' pixels in an 8x8 area (top 8x8 in x channel, bottom 8x4 in y channel)
//			output tile data

// Define INPUT_SHADOWMASK_BUFFER to be the input bits for the current 8x8 block (RG32 uint format).
// Define INPUT_HISTORY_CLASSIFICATION_BUFFER to be the previouus frame's classification buffer.


int TileClassification(uint2 TileLoc)
{
	uint FullyLit = 0;
	uint FullyShadowed = 0;

	uint2 Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(-1,-1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(0,-1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(1,-1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;

	Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(-1,0)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(0,0)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(1,0)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;

	Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(-1,1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(0,1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;
    Shadowed8x8 = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, int2(TileLoc), 0, int2(1,1)).xy;
    FullyShadowed |= ~(Shadowed8x8.x & Shadowed8x8.y);
	FullyLit |= Shadowed8x8.x | Shadowed8x8.y;

	int BlockType = 0;	// Default to 'partially' lit/shadowed.  Fully Lit is positive, fully shadowed is negative
	if (FullyLit==0)
	{
		// No set bits in the 'Shadowed' input
		BlockType = -1;	// Fully lit
	}
	else if (FullyShadowed==0)
	{
		// No clear bits in the 'shadowed' input
		BlockType = 1;	// Fully shadowed
	}

	// Use the history to keep track of how many frames this tile has been fully lit or fully shadowed for.
	int OldBlockType = texelFetch(INPUT_HISTORY_CLASSIFICATION_BUFFER, int2(TileLoc), 0).x;
	if ((OldBlockType * BlockType)>0)
	{
		// Block is still fully lit or fully unlit, accumulate count of frames it has had this value for!
		BlockType += OldBlockType;
		BlockType = clamp(BlockType, -127, 127);
	}
	return BlockType;
}

#endif // _QCOMSHADOWDENOISERTILECLASSIFICATION_H_
