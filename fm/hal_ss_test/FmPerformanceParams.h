/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#ifndef __FM_PERFORMANCE_PARAMS_H__
#define __FM_PERFORMANCE_PARAMS_H__

#include "Qualcomm_FM_Const.h"

#define MIN_BLEND_SINRHI -128
#define MAX_BLEND_SINRHI  127
#define MIN_BLEND_RMSSIHI -128
#define MAX_BLEND_RMSSIHI  127

class FmPerformanceParams
{
      private:
      public:
          signed char SetAfRmssiTh(UINT fd, unsigned short th);
          signed char SetAfRmssiSamplesCnt(UINT fd, unsigned char cnt);
          signed char SetGoodChannelRmssiTh(UINT fd, signed char th);
          signed char SetSrchAlgoType(UINT fd, unsigned char algo);
          signed char SetSinrFirstStage(UINT fd, signed char th);
          signed char SetRmssiFirstStage(UINT fd, signed char th);
          signed char SetCf0Th12(UINT fd, int th);
          signed char SetSinrSamplesCnt(UINT fd, unsigned char cnt);
          signed char SetIntfLowTh(UINT fd, unsigned char th);
          signed char SetIntfHighTh(UINT fd, unsigned char th);
          signed char SetSinrFinalStage(UINT fd, signed char th);
          signed char SetHybridSrchList(UINT fd, unsigned int *freqs, signed char *sinrs, unsigned int n);
          signed char SetBlendSinr(UINT fd, signed char bsinr);
          signed char SetBlendRmssi(UINT fd, signed char brmssi);

          signed char GetAfRmssiTh(UINT fd, unsigned short &th);
          signed char GetAfRmssiSamplesCnt(UINT fd, unsigned char &cnt);
          signed char GetGoodChannelRmssiTh(UINT fd, signed char &th);
          signed char GetSrchAlgoType(UINT fd, unsigned char &algo);
          signed char GetSinrFirstStage(UINT fd, signed char &th);
          signed char GetRmssiFirstStage(UINT fd, signed char &th);
          signed char GetCf0Th12(UINT fd, int &th);
          signed char GetSinrSamplesCnt(UINT fd, unsigned char &cnt);
          signed char GetIntfLowTh(UINT fd, unsigned char &th);
          signed char GetIntfHighTh(UINT fd, unsigned char &th);
          signed char GetIntfDet(UINT fd, unsigned char &th);
          signed char GetSinrFinalStage(UINT fd, signed char &th);
          signed char GetBlendSinr(UINT fd, signed char &bsinr);
          signed char GetBlendRmssi(UINT fd, signed char &brmssi);
};

#endif //__FM_PERFORMANCE_PARAMS_H__
