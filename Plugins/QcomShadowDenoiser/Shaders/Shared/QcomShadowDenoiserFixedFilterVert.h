//============================================================================================================
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _QCOMSHADOWDENOISERFILTERFIXEDVERT_H_
#define _QCOMSHADOWDENOISERFILTERFIXEDVERT_H_

// Define INPUT_HORIZONTALLYFILTERED_BUFFER to be the output of the filed horizontal filter pass

#if FEATURE_LEVEL == FEATURE_LEVEL_ES3_1
#define GetGlobalSampler(Filter,WrapMode) \
	View.Shared##Filter##WrapMode##Sampler
#define GlobalBilinearClampedSampler GetGlobalSampler(Bilinear, Clamped)
#endif

#define cFilterRadius (8)	// radius, not width.  Does not include the center pixel
static_const float16_t cFilterWeights[cFilterRadius] = { float16_t(0.963640444),
												  float16_t(0.862303357),
												  float16_t(0.716531311),
												  float16_t(0.552892001),
												  float16_t(0.39616443),
												  float16_t(0.263597138),
												  float16_t(0.162868066),
												  float16_t(0.0934461102) };
static_const float16_t cFilterScale = float16_t( 1.0 / 9.02288571 );

//
// Vertically filter the input (radius cFilterRadius) for two adjacent pixels
//
float16_t2 FilterVertical2(float2 ScreenCoord)
{
    // Read center pixel
    float16_t2 FilteredSum = float16_t2( textureGatherOffset(INPUT_HORIZONTALLYFILTERED_BUFFER, ScreenCoord, int16_t2(0,0)).wz );

	// Accumulate the left and right sides of the filter at the same time
    for(int i = 0; i < cFilterRadius; i+=2)
    {
        float16_t4 LeftRightGathers = float16_t4( textureGatherOffset(INPUT_HORIZONTALLYFILTERED_BUFFER, ScreenCoord, int16_t2(0,i+1)).wxzy );
        LeftRightGathers += float16_t4( textureGatherOffset(INPUT_HORIZONTALLYFILTERED_BUFFER, ScreenCoord, int16_t2(0,-i-2)).xwyz );
        // LeftRightGathers in order:   x z
        //                              y w
        float16_t2 Weight01 = float16_t2( cFilterWeights[i], cFilterWeights[i+1]);
        LeftRightGathers *= Weight01.xyxy;
        FilteredSum += LeftRightGathers.xz;
        FilteredSum += LeftRightGathers.yw;
    }

    return FilteredSum * cFilterScale;
}

#endif // _QCOMSHADOWDENOISERFILTERFIXEDVERT_H_
