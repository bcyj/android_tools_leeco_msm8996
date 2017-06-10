#ifndef AUDIO_FTM_DTMF_GEN_H
#define AUDIO_FTM_DTMF_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file audio_ftm_dtmf_gen.h
  @brief  DTMF Generator API
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_dtmf_gen.h#1 $
$DateTime: 2011/04/05 20:05:46 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      05/30/2010                    File creation.



====================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "DALSYS_common.h"

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/




/*==================================================================================================
                                            MACROS
==================================================================================================*/

/*==================================================================================================
                                             ENUMS
==================================================================================================*/



/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef int Flag;

typedef struct {
   int16 increment;
   int16 tableIndex;
}DTMFToneGenParamsStruct;

typedef struct {
   DTMFToneGenParamsStruct tone1;
   DTMFToneGenParamsStruct tone2;
}DTMFGenStruct;

/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/*
  dtmfFreq1: Input - low frequency, such as 300 (Hz)
  dtmfFreq2: Input - high frequency, such as 500 (Hz)
  samplingfreq: input - sampling rate, such as 8000 (Hz)

*/
void audio_ftm_dtmf_tone_init
(
      DTMFGenStruct *dtmfGen,
      int32 dtmfFreq1,
      int32 dtmfFreq2,
      int16 samplingfreq
);

/*
  dtmfGain: Input - Gain, range 0-0x7fff

  return:   generated DTMF sample output
*/

int16  audio_ftm_dtmf_tone_sample_gen
(
        DTMFGenStruct *dtmfGen,
        uint16 dtmfGain
)	;

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_DTMF_GEN_H */

