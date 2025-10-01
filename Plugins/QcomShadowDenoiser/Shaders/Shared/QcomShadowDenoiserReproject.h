//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERREPROJECT_H_
#define _QCOMSHADOWDENOISERREPROJECT_H_

// temporal clamping/rejection mode
#define cRejectNone					(0)
#define cRejectUseDepthMinMax		(1)
#define cRejectUseFilter			(2)

// Define INPUT_RT_SHADOW_BUFFER to be the input shadow texture (output of ray tracing pass). x=shadowed or lit, y=shadow distance to light
// Define INPUT_FILTERED_SHADOW_HISTORY_BUFFER to be the shadow buffer written by the previous frame's filter pass
// Define INPUT_TILE_MINMAX_DISTANCE_BUFFER to be the minmax shadow distance texture
// Define INPUT_MINMAX_HISTORY_BUFFER to be the minmax buffer written by the previous frame.
// Define INPUT_MOMENTS_HISTORY_BUFFER to be the moments buffer written by the previous frame.
// Define INPUT_FIXED_FILTER_BUFFER to be the output of the fixed filter stage(s).
// Define INPUT_TILE_CLASSIFICATION_BUFFER to be tile classification texture (Optional)

// Implement "void ReadReprojectedFrameData(float2 ViewportUV, out HistoryLocData ReprojectedFrameData)"
//struct DepthPositionData
//{
//	float	CurrentLinearDepth;						// Depth sampled for the current frame.
//	float2	ReprojectedUV;							// Current position reprojected into the previous frame's view.
//	float	ReprojectedLinearDepth;					// Current depth reprojected into the previous frame's view
//	float	HistoryLinearDepth;						// Depth sampled at ReprojectedUV for the previous frame.
//};

// Tuning values that can be overriden by application
#ifndef TUNING_REPROJECT_DEPTH_SENSITIVITY
// Sensitivity of depth difference calculation.  Larger is more sensitive (reprojected pixels less likely to be accepted).
#define TUNING_REPROJECT_DEPTH_SENSITIVITY (0.1)
#endif // TUNING_REPROJECT_DEPTH_SENSITIVITY

#ifndef TUNING_HISTORY_CERTAINTY_THRESHOLD
// Certainty (0-1) under which we discard accumulated temporal data and fallback to spatial filters.
#define TUNING_HISTORY_CERTAINTY_THRESHOLD 0.05
#endif // TUNING_HISTORY_CERTAINTY_THRESHOLD

#ifndef TUNING_MINIMUM_UNREPROJECTED_SHADOW
// How much of the un-reprojected (noisy) shadow from the current frame to use when we have full confidence of the temporal value (smaller values make smoother shadows but slower response to input changes in cases where scene changes are not detected by the Rejection mode)
#define TUNING_MINIMUM_UNREPROJECTED_SHADOW 0.1
#endif // TUNING_MINIMUM_UNREPROJECTED_SHADOW


// Structure output by Reproject()
struct ReprojectOutputs
{
	float4	Moments;								// Output of temporal reprojection
													// x = Mean shadow (1=fully lit, 0=fully shadowed)
													// y = Variance
													// z = Sample count
													// w = Mean distance
	float2	Shadow;									// Output of temporal filter to be further filtered by the A-Trous passes
													// x = Mean shadow (0-1)
													// y = variance
	float2	DistanceMinMax;							// x = min shadow distance
													// y = max shadow distance
	bool	DistanceMinMaxValid;					// True if the DistanceMinMax value is valid (and should be stored), false if the DistanceMinMax is not being used
};

#ifdef INPUT_TILE_CLASSIFICATION_BUFFER
// Include code for GetTileEarlyOut()
#include "QcomShadowDenoiserReadTile.h"
#endif // INPUT_TILE_CLASSIFICATION_BUFFER

void Reproject(uint2 ScreenLoc, uint2 TileLoc, int TemporalRejectMode, out ReprojectOutputs Output)
{
	// Get the min/max shadow distance for this tile
	float2 TileMinMax = texelFetch(INPUT_TILE_MINMAX_DISTANCE_BUFFER, int2(TileLoc), 0).xy;

#ifdef INPUT_TILE_CLASSIFICATION_BUFFER
	// If this screen pixel's tile is fully shadowed or fully lit (which accounts for the maximum filter width) then bypass the reproject
	float EarlyOutShadow;
	if (GetTileEarlyOut(TileLoc, EarlyOutShadow))
	{
		Output.Moments = float4(0.0, 0.0, 0.0, 0.0);
		Output.Shadow = float2(EarlyOutShadow, 0.0/*variance*/);
		Output.DistanceMinMax = TileMinMax;
		return;
	}
#endif // INPUT_TILE_CLASSIFICATION_BUFFER

	DepthPositionData DepthPositionData;
	bool IsPredictionOnScreen = ReadAndReprojectPositionData(ScreenLoc, DepthPositionData);

	float2 NoisyShadow = texelFetch(INPUT_RT_SHADOW_BUFFER, int2(ScreenLoc), 0).xy;

	float OutputShadow = NoisyShadow.x;		// Default to 'noisy' input shadow
	float Mean = NoisyShadow.x;				// Default to 'noisy' input shadow
	float Variance = 16.0;					// and assume the variance is huge!

	float HistoryCertainty = 0.0;			// Assume we are uncertain of the history!
	float SampleCount = 1.0;

	if (IsPredictionOnScreen)
	{
		// Determine if the history depth at the predicted position (last frame's depth) is consistant with our current frame depth 
		//TODO: do we need to include jitter, ie GetHistoryScreenPositionIncludingTAAJitter)
		//TODO: we should consider normal (or neighboring depth values) to adjust sensitivity based on surface normal
		HistoryCertainty = exp(-abs(DepthPositionData.HistoryLinearDepth - DepthPositionData.ReprojectedLinearDepth) * TUNING_REPROJECT_DEPTH_SENSITIVITY);

		if (HistoryCertainty < TUNING_HISTORY_CERTAINTY_THRESHOLD)
			IsPredictionOnScreen = false;
	}

	float2 DistanceMinMax = float2(-1.0/*min*/, -1.0/*max*/);
	if (IsPredictionOnScreen)
	{
		if (TemporalRejectMode == cRejectUseDepthMinMax)
		{
			DistanceMinMax = TileMinMax;

			if (DistanceMinMax.x >= 0.0)
			{
				float2 HistoryDistanceMinMax = textureLod(INPUT_MINMAX_HISTORY_BUFFER, DepthPositionData.ReprojectedUV, 0).xy;

				float DistanceUnder = HistoryDistanceMinMax.x - DistanceMinMax.x;	// positive if under history
				float DistanceOver = DistanceMinMax.y - HistoryDistanceMinMax.y;	// positive if over history

				if (DistanceUnder * DistanceOver < -1.0)
				{
					// Reprojected sample's shadow depth either above or below the tile's min/max distance
					// Further from the current min/max = more uncertainty
					HistoryCertainty *= exp(-max(DistanceUnder, DistanceOver) / 20.0);
				}
			}
			else
			{
				// No min/max depth around this sample in the current frame.  
				HistoryCertainty = 0.0;
			}
			if (HistoryCertainty < TUNING_HISTORY_CERTAINTY_THRESHOLD)
				IsPredictionOnScreen = false;
		}
	}

	if (IsPredictionOnScreen)
	{
		// Start with spatially filtered Shadow (mean and moment) of the previous frame
		float2 HistoryFilteredShadow = textureLod(INPUT_FILTERED_SHADOW_HISTORY_BUFFER, DepthPositionData.ReprojectedUV, 0).xy;

		// Start with moments for the previous frame.
		float3 Moments = textureLod(INPUT_MOMENTS_HISTORY_BUFFER, DepthPositionData.ReprojectedUV, 0).xyz;

		// Scale down the number of samples if the reprojection is uncertain
		Moments.z *= HistoryCertainty;

		//
		// Calculate the new mean and variance (adding the contribution of the current sample)
		//
		Mean = Moments.x;
		float Moment2Sum = Moments.y * Moments.z;
		SampleCount = max(0, Moments.z) + 1;	// we are adding one more sample

		float DeltaMean = (NoisyShadow.x - Mean);
		Mean += (DeltaMean / SampleCount);
		Moment2Sum += DeltaMean * (NoisyShadow.x - Mean/*updated mean*/);
		Variance = Moment2Sum / SampleCount;

		if (TemporalRejectMode == cRejectUseFilter)
		{
			//
			// Clamp the reprojected shadow mean/variance using the estimated shadow mean/variance for the region around this pixel
			//
			float FixedFilterMean = texelFetch(INPUT_FIXED_FILTER_BUFFER, int2(ScreenLoc), 0).x;
			float FixedFilterVariance = max(0.0, FixedFilterMean - FixedFilterMean * FixedFilterMean);
			float FixedFilterDeviation = sqrt(FixedFilterVariance);
			float2 FixedFilterMinMax = float2(FixedFilterMean - 0.5 * FixedFilterDeviation, FixedFilterMean + 0.5 * FixedFilterDeviation);
			float UnclampedReprojectedShadow = HistoryFilteredShadow.x;
			HistoryFilteredShadow.x = clamp(HistoryFilteredShadow.x, FixedFilterMinMax.x, FixedFilterMinMax.y);

			// Reduce the number of temporal samples based on how far our mean deviates from the fixed filter
			const float DampeningFactor = 20.0f;
			const float Uncertainty = (UnclampedReprojectedShadow - FixedFilterMean) / max(0.5f * FixedFilterDeviation, 0.001f);
			const float Dampening = exp(-Uncertainty * Uncertainty / DampeningFactor);
			SampleCount *= Dampening;

			// When we only have a few temporal samples increase the variance so we get more spatial filtered data
			if (SampleCount < 10.0f)
			{
				const float UncertaintyMultiplier = max(10.0f - SampleCount, 1.0f);
				Variance = max(FixedFilterVariance, Variance) * UncertaintyMultiplier;
			}
		}

		// Blend the history (reprojected) shadow with the current value.
		// We want some amount of 'new' sample (and exclusively new sample if we believe the reprojected value is occluded/invalid)
		float TemporalBlend = max(0.0, HistoryCertainty - TUNING_MINIMUM_UNREPROJECTED_SHADOW);
		OutputShadow = lerp(NoisyShadow.x, HistoryFilteredShadow.x, TemporalBlend);

		// Use the 'worse' of temporal and reprojected variance (and boost the reprojected variance if we are somewhat uncertain of its value)
		Variance = max(Variance, HistoryFilteredShadow.y / (TemporalBlend + TUNING_MINIMUM_UNREPROJECTED_SHADOW));
	}

	//
	// Populate the output data
	//
	Output.Moments = float4(Mean, Variance, SampleCount, HistoryCertainty/*debug*/);

	// Boost the variance to be fed into the A-Trous filters based on the history certainty (certainty of 1.0 == 0.0 boost)
	float VarianceUncertaintyBoost = 1.0 / max(0.1, HistoryCertainty) - 1.0;
	Output.Shadow = float2(OutputShadow, Variance + VarianceUncertaintyBoost);

	Output.DistanceMinMaxValid = TemporalRejectMode == cRejectUseDepthMinMax;
	Output.DistanceMinMax = DistanceMinMax;
}

#endif // _QCOMSHADOWDENOISERREPROJECT_H_
