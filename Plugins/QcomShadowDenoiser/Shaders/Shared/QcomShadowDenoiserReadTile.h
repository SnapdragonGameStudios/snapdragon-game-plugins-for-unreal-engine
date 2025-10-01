//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================
#ifndef _QCOMSHADOWDENOISERREADTILE_H_
#define _QCOMSHADOWDENOISERREADTILE_H_

// Define INPUT_TILE_CLASSIFICATION_BUFFER to be tile classification texture

// Return true if fully lit or is fully shadowed, return false if block partially lit and partially shadowed
// Output tile lit amount (0.0 if fully shadowed, 1.0 if fully lit, undefined if tile is not fully lit or fully shadowed)
bool GetTileEarlyOut(uint2 InTileLoc, out float OutLight)
{
	int TileClassification = texelFetch(INPUT_TILE_CLASSIFICATION_BUFFER, int2(InTileLoc), 0).x;
	if (TileClassification > 32 || TileClassification < -32)
	{
		OutLight = (TileClassification<0) ? 0.0 : 1.0;
		return true;
	}
	return false;
}

#endif // _QCOMSHADOWDENOISERREADTILE_H_ 
