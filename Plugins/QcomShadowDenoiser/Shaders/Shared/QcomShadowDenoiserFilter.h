//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERFILTER_H_
#define _QCOMSHADOWDENOISERFILTER_H_

// Define INPUT_SHADOW_BUFFER to be the texture we want to filter.  R = shadow mean (0.0 - 1.0), G = shadow variance.
// Define ATROUS_FILTER_STEPSIZE to be the a-trous step size for this pass of the filter (ie 1,2,4 etc).
// Define INPUT_TILE_CLASSIFICATION_BUFFER to be tile classification texture.


// Do the A-trous filter.
// Returns filtered shadow mean and variance
float2 FilterAtrous(uint2 ScreenLoc)
{
	// Get the average variance (around center)
	const float FilterWeights[3] = {0.25, 0.125, 0.0625};

	float AverageVariance = 0.0;
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-1, -1)).g * FilterWeights[abs(-1)+abs(-1)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 0, -1)).g * FilterWeights[abs( 0)+abs(-1)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 1, -1)).g * FilterWeights[abs( 1)+abs(-1)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-1,  0)).g * FilterWeights[abs(-1)+abs( 0)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 0,  0)).g * FilterWeights[abs( 0)+abs( 0)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 1,  0)).g * FilterWeights[abs( 1)+abs( 0)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-1,  1)).g * FilterWeights[abs(-1)+abs( 1)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 0,  1)).g * FilterWeights[abs( 0)+abs( 1)];
	AverageVariance += texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( 1,  1)).g * FilterWeights[abs( 1)+abs( 1)];

	float Deviation = sqrt(0.0001 + max(0.0,AverageVariance));

	//
	// Sample mask
	float2 CenterMask = texelFetch(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0).rg;

	// Sample mask neighbor values
	float2 NeighborInputs[8];
	NeighborInputs[0] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(      0,-ATROUS_FILTER_STEPSIZE)).rg;
	NeighborInputs[1] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-ATROUS_FILTER_STEPSIZE,      0)).rg;
	NeighborInputs[2] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( ATROUS_FILTER_STEPSIZE,      0)).rg;
	NeighborInputs[3] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(      0, ATROUS_FILTER_STEPSIZE)).rg;
	NeighborInputs[4] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-ATROUS_FILTER_STEPSIZE,-ATROUS_FILTER_STEPSIZE)).rg;
	NeighborInputs[5] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( ATROUS_FILTER_STEPSIZE,-ATROUS_FILTER_STEPSIZE)).rg;
	NeighborInputs[6] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2(-ATROUS_FILTER_STEPSIZE, ATROUS_FILTER_STEPSIZE)).rg;
	NeighborInputs[7] = texelFetchOffset(INPUT_SHADOW_BUFFER, int2(ScreenLoc), 0, int2( ATROUS_FILTER_STEPSIZE, ATROUS_FILTER_STEPSIZE)).rg;

	// Weights from neighbors
	float4 VarianceWeights[2];
	//  NSEW neighbors
	float4 NeighborValues = float4(NeighborInputs[0].x, NeighborInputs[1].x, NeighborInputs[2].x, NeighborInputs[3].x);
	VarianceWeights[0] = exp(-abs( (NeighborValues-CenterMask.x)/Deviation ));
	//  Diagonal neighbors
	NeighborValues = float4(NeighborInputs[4].x, NeighborInputs[5].x, NeighborInputs[6].x, NeighborInputs[7].x);
	VarianceWeights[1] = exp(-abs( (NeighborValues-CenterMask.x)/Deviation ));

#if 0
	//
	// Sample depth
	float CenterDepth = CalcSceneDepthOffset(ScreenLoc, uint2(0,0));

	// Sample depth neighbor values.
	float4 NeighborDepth;
	NeighborDepth.x = CalcSceneDepthOffset(ScreenLoc, int2(      0,-FilterOffset));
	NeighborDepth.y = CalcSceneDepthOffset(ScreenLoc, int2(-FilterOffset,      0));
	NeighborDepth.z = CalcSceneDepthOffset(ScreenLoc, int2( FilterOffset,      0));
	NeighborDepth.w = CalcSceneDepthOffset(ScreenLoc, int2(      0, FilterOffset));

	// Weights from depth
	float4 DepthWeights = exp(-abs(NeighborDepth - CenterDepth) * kDepthSensitivity);
#else
	float4 DepthWeights[2];
	DepthWeights[0] = float4(1.0,1.0,1.0,1.0);
	DepthWeights[1] = float4(1.0,1.0,1.0,1.0);
#endif
	// Combine weighting

	float4 NeighborWeights[2];
	NeighborWeights[0] = VarianceWeights[0] * DepthWeights[0] * 0.125;
	NeighborWeights[1] = VarianceWeights[1] * DepthWeights[1] * 0.0625;

	// Calculate the weighted filter (for both the mean [x] and the variance [y])
	float Weight = 0.25;
	float WeightAccum = Weight;
	float2 FilterAccum = CenterMask * float2(Weight, Weight*Weight);
	for(int j=0;j<2;++j)
	{
		for(int i=0;i<4;++i)
		{
			Weight = NeighborWeights[j][i];
			WeightAccum += Weight;
			FilterAccum += NeighborInputs[i+j*4] * float2(Weight, Weight*Weight);
		}
	}

	return FilterAccum / float2(WeightAccum, WeightAccum*WeightAccum);
}


#ifdef INPUT_TILE_CLASSIFICATION_BUFFER

// Include code for GetTileEarlyOut()
#include "QcomShadowDenoiserReadTile.h"

// Do the A-trous filter with tile optimization (early out fully shadowed or lit pixels).
// Returns filtered shadow mean and variance
float2 FilterAtrous(uint2 ScreenLoc, uint2 TileLoc)
{
    // If this screen pixel's tile is fully shadowed or fully lit (which accounts for the maximum filter width) then bypass the reproject
    float EarlyOutShadow;
    if (GetTileEarlyOut(TileLoc, EarlyOutShadow))
    {
        return float2(EarlyOutShadow, 0.0);
    }

    return FilterAtrous(ScreenLoc);
}

#endif // INPUT_TILE_CLASSIFICATION_BUFFER

#endif // _QCOMSHADOWDENOISERFILTER_H_
