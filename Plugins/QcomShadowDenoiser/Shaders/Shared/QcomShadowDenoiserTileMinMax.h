//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERTILEMINMAX_H_
#define _QCOMSHADOWDENOISERTILEMINMAX_H_

#if FEATURE_LEVEL == FEATURE_LEVEL_ES3_1
#define GetGlobalSampler(Filter,WrapMode) \
	View.Shared##Filter##WrapMode##Sampler
#define GlobalBilinearClampedSampler GetGlobalSampler(Bilinear, Clamped)
#endif
//
// Reduce input shadow buffer so each 8x8 screen block has a min/max value (for pixels with a distance in G and negative distances denoting 'no' pixel).
// Does the 'min' calculation using unsigned integer comparision so any negative distances are treated as very large positive values and essentially ignored.
// The Ray Tracing pass (which generates the input to this shader) is expected to write a negative distance value for any 'lit' (ray miss) pixels.
//

// Define INPUT_RT_SHADOW_BUFFER to be the texture/sampler for the noisy ray traced shadow image (Red channel = shadowed or lit, 0.0 or 1.0, Green channel = shadow distance to light)

float2 TileMinMax(float2 InputUV)
{
	half4 g00 = half4( textureGatherGreen(		 INPUT_RT_SHADOW_BUFFER, InputUV ) );
	half4 g20 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(2,0) ) );
	half4 g40 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(4,0) ) );
	half4 g60 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(6,0) ) );

	half4 g02 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(0,2) ) );
	half4 g22 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(2,2) ) );
	half4 g42 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(4,2) ) );
	half4 g62 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(6,2) ) );

	half4 g04 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(0,4) ) );
	half4 g24 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(2,4) ) );
	half4 g44 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(4,4) ) );
	half4 g64 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(6,4) ) );

	half4 g06 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(0,6) ) );
	half4 g26 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(2,6) ) );
	half4 g46 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(4,6) ) );
	half4 g66 = half4( textureGatherGreenOffset( INPUT_RT_SHADOW_BUFFER, InputUV, int16_t2(6,6) ) );

	half4 gmax0 = max( max(g00,g20), max(g40,g60) );
	half4 gmax2 = max( max(g02,g22), max(g42,g62) );
	half4 gmax4 = max( max(g04,g24), max(g44,g64) );
	half4 gmax6 = max( max(g06,g26), max(g46,g66) );
	half4 gmax = max( max(gmax0, gmax2), max(gmax4, gmax6));
	half fmax = max( max(gmax.x, gmax.y), max(gmax.z, gmax.w) );

	uint4 ug00 = asuint(float4(g00));
	uint4 ug20 = asuint(float4(g20));
	uint4 ug40 = asuint(float4(g40));
	uint4 ug60 = asuint(float4(g60));
	uint4 ug02 = asuint(float4(g02));
	uint4 ug22 = asuint(float4(g22));
	uint4 ug42 = asuint(float4(g42));
	uint4 ug62 = asuint(float4(g62));
	uint4 ug04 = asuint(float4(g04));
	uint4 ug24 = asuint(float4(g24));
	uint4 ug44 = asuint(float4(g44));
	uint4 ug64 = asuint(float4(g64));
	uint4 ug06 = asuint(float4(g06));
	uint4 ug26 = asuint(float4(g26));
	uint4 ug46 = asuint(float4(g46));
	uint4 ug66 = asuint(float4(g66));

	uint4 ugmin0 = min( min(ug00,ug20), min(ug40,ug60) );
	uint4 ugmin2 = min( min(ug02,ug22), min(ug42,ug62) );
	uint4 ugmin4 = min( min(ug04,ug24), min(ug44,ug64) );
	uint4 ugmin6 = min( min(ug06,ug26), min(ug46,ug66) );
	uint4 ugmin = min( min(ugmin0, ugmin2), min(ugmin4, ugmin6));
	float fmin = asfloat( min( min(ugmin.x, ugmin.y), min(ugmin.z, ugmin.w) ) );

    return float2( min(fmin, float(fmax)), float(fmax) );
}

#endif // _QCOMSHADOWDENOISERTILEMINMAX_H_
