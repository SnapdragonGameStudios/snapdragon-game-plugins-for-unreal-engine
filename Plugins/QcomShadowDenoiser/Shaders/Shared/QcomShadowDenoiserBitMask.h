//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERBITMASK_H_
#define _QCOMSHADOWDENOISERBITMASK_H_

#if FEATURE_LEVEL == FEATURE_LEVEL_ES3_1
#define GetGlobalSampler(Filter,WrapMode) \
	View.Shared##Filter##WrapMode##Sampler
#define GlobalBilinearClampedSampler GetGlobalSampler(Bilinear, Clamped)
#endif

//
// Reduce input shadow buffer so each 8x8 screen block is represented by a single 64bit texel with one bit per pixel (set if 'shadowed').
//

// Define INPUT_RT_SHADOW_BUFFER to be the texture/sampler for the noisy ray traced shadow image (Red channel= shadowed or lit, 0.0 or 1.0)

// Gather an 8x8 area of InputShadowTexture and pack into 2x 32bit uint.
uint2 LoadBitMask8x8(float2 TopLeftUV)
{
	uint16_t2 p0;
	uint16_t2 p1;
	{
		// top 8x4 block
		uint4 g00 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(0,0) ) );
		uint p00 = g00.w | (g00.z<<1) | (g00.x << 8) | (g00.y << 9);
		uint4 g20 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(2,0) ) );
		uint p20 = g20.w | (g20.z<<1) | (g20.x << 8) | (g20.y << 9);
		uint4 g40 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(4,0) ) );
		uint p40 = g40.w | (g40.z<<1) | (g40.x << 8) | (g40.y << 9);
		uint4 g60 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(6,0) ) );
		uint p60 = g60.w | (g60.z<<1) | (g60.x << 8) | (g60.y << 9);
		uint4 g02 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(0,2) ) );
		uint p02 = g02.w | (g02.z<<1) | (g02.x << 8) | (g02.y << 9);
		uint4 g22 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(2,2) ) );
		uint p22 = g22.w | (g22.z<<1) | (g22.x << 8) | (g22.y << 9);
		uint4 g42 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(4,2) ) );
		uint p42 = g42.w | (g42.z<<1) | (g42.x << 8) | (g42.y << 9);
		uint4 g62 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(6,2) ) );
		uint p62 = g62.w | (g62.z<<1) | (g62.x << 8) | (g62.y << 9);
		p0 = uint16_t2( p00 | (p20<<2) | (p40<<4) | (p60<<6), p02 | (p22<<2) | (p42<<4) | (p62<<6) );
	}
	{
		// bottom 8x4 block
		uint4 g00 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(0,4) ) );
		uint p00 = g00.w | (g00.z<<1) | (g00.x << 8) | (g00.y << 9);
		uint4 g20 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(2,4) ) );
		uint p20 = g20.w | (g20.z<<1) | (g20.x << 8) | (g20.y << 9);
		uint4 g40 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(4,4) ) );
		uint p40 = g40.w | (g40.z<<1) | (g40.x << 8) | (g40.y << 9);
		uint4 g60 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(6,4) ) );
		uint p60 = g60.w | (g60.z<<1) | (g60.x << 8) | (g60.y << 9);
		uint4 g02 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(0,6) ) );
		uint p02 = g02.w | (g02.z<<1) | (g02.x << 8) | (g02.y << 9);
		uint4 g22 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(2,6) ) );
		uint p22 = g22.w | (g22.z<<1) | (g22.x << 8) | (g22.y << 9);
		uint4 g42 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(4,6) ) );
		uint p42 = g42.w | (g42.z<<1) | (g42.x << 8) | (g42.y << 9);
		uint4 g62 = uint4( textureGatherOffset( INPUT_RT_SHADOW_BUFFER, TopLeftUV, uint16_t2(6,6) ) );
		uint p62 = g62.w | (g62.z<<1) | (g62.x << 8) | (g62.y << 9);
		p1 = uint16_t2( p00 | (p20<<2) | (p40<<4) | (p60<<6), p02 | (p22<<2) | (p42<<4) | (p62<<6) );
	}
	return uint2(uint(p0.x) | (uint(p0.y)<<16), uint(p1.x) | (uint(p1.y)<<16));
}

#endif // _QCOMSHADOWDENOISERBITMASK_H_
