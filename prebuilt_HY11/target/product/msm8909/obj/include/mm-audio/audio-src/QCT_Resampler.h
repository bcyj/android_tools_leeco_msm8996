#ifndef ANDROID_QCT_RESAMPLER_API_H
#define ANDROID_QCT_RESAMPLER_API_H
/*============================================================================
 Copyright (c) 2012 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdint.h>
#include <sys/types.h>

namespace android {

class QCT_Resampler {

public:
	static size_t	MemAlloc(int bitDepth, int inChannelCount, int32_t inSampleRate, int32_t sampleRate);
	static void		Init(int16_t *pState, int32_t inChannelCount, int32_t inSampleRate, int32_t mSampleRate);
	static void		Resample90dB(int16_t* pState, int16_t* in, int32_t* out, size_t inFrameCount, size_t outFrameCount);
	static size_t	GetNumInSamp(int16_t* pState, size_t outFrameCount);
};
};
#endif  //ANDROID_QCT_RESAMPLER_API_H
