//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERFIXEDFILTERHORIZ_H_
#define _QCOMSHADOWDENOISERFIXEDFILTERHORIZ_H_

// Define INPUT_SHADOWMASK_BUFFER to be the texture containing the 2x32bit texture representing the shadowed/lit bits for each 8x8 pixel tile
// Define INPUT_FILTERLOOKUP_BUFFER to be the 512x2 sized filter lookup texture (lookup 9bits of 'shadow/lit' to determine a filtered output value).

//
// Horizontally filter for the given ScreenLoc (and for ScreenLoc + (0,4), ie 4 pixels below
// because the InputShadowBitBuffer covers an 8x8 block, top 8x4 in x, bottom 8x4 in y).
//
float16_t2 FilterHorizontal(uint2 ScreenLoc, uint16_t2 TileLoc)
{
	uint RowShift    = (ScreenLoc.y & 0x3) * 8;

	uint2 PriorByte   = (TileLoc.x>0) ? (texelFetchOffset(INPUT_SHADOWMASK_BUFFER, TileLoc, 0, int16_t2(-1,0)).xy >> RowShift) : uint2(0,0);
	uint2 CurrentByte = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, TileLoc, 0, int16_t2(0,0)).xy >> RowShift;
	uint2 NextByte    = texelFetchOffset(INPUT_SHADOWMASK_BUFFER, TileLoc, 0, int16_t2(1,0)).xy >> RowShift;

    // Construct a single uint with the lowest 17bits containing the valies (boolean) to be filtered horizontally.

	PriorByte &= 0xff;
	CurrentByte &= 0xff;
	NextByte &= 0xff;
	uint2 FilterRegion = PriorByte | (CurrentByte<<8) | (NextByte<<16);

	uint BitLoc = ScreenLoc.x & 0x7;	// Bit index of the center pixel
	FilterRegion >>= BitLoc;

	uint16_t2 LeftBits  = uint16_t2(FilterRegion)    & uint16_t(0x1ff);
	uint16_t2 RightBits = uint16_t2(FilterRegion>>8) & uint16_t(0x1ff);

    // First 9 bits up to and including the center pixel
    float16_t2 FilteredSum;
	FilteredSum.x = float16_t(texelFetch(INPUT_FILTERLOOKUP_BUFFER, uint16_t2(LeftBits.x, 0), 0).x);	// top row
	FilteredSum.y  = float16_t( texelFetch(INPUT_FILTERLOOKUP_BUFFER, uint16_t2(LeftBits.y,  0), 0 ).x );	// bottom (+4 pixels) row
    // Second 9 bits from (including) the center pixel
    FilteredSum.x += float16_t( texelFetch(INPUT_FILTERLOOKUP_BUFFER, uint16_t2(RightBits.x, 1), 0 ).x );	// top row
    FilteredSum.y += float16_t( texelFetch(INPUT_FILTERLOOKUP_BUFFER, uint16_t2(RightBits.y, 1), 0 ).x );	// bottom (+4 pixels) row

    return FilteredSum;
}

#endif // _QCOMSHADOWDENOISERFIXEDFILTERHORIZ_H_
