#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_dtmf_basic_op.c
  @brief  Utility: basic arithmetic operators
====================================================================================================
Copyright (c) 2010 - 2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/qnx/ftm_drv_lib/rel/1.0/src/audio_ftm_dtmf_basic_op.c#4 $
$DateTime: 2011/05/02 20:06:41 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      05/30/2010                    File creation.


====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/

#include "audio_ftm_dtmf_basic_op.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/



/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/



/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/



/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/

int32 audio_ftm_dtmf_add32 (int32 v1, int32 v2)
{
    int64 acc;

    acc=(int64)v1 + (int64)v2;

    if(acc > (int64)0x7fffffff ) return (int32)0x7fffffff;
    if(acc < ~(int64)0x7fffffff) return (int32)0x80000000;

    return (int32)acc;
}

/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

int16 audio_ftm_dtmf_add16 (int16 v1, int16 v2)
{
    int16 var_out;
    int32 acc;

    acc = (int32) v1 + (int32)v2;

    if (acc > (int32)0x7fff)  return (int16)0x7fff;
    if (acc < ~(int32)0x00007fff) return (int16)0x8000;

		return (int16)acc;
}

int32 audio_ftm_dtmf_multiply16 (int16 v1, int16 v2)
{
    int64 prod;

    prod=(int32) v1 *(int32) v2;

    prod *=2;

    if(prod > (int64)0x7fffffff ) return (int32)0x7fffffff;
    if(prod < ~(int64)0x7fffffff) return (int32)0x80000000;

    return (int32)prod;

}

int16 audio_ftm_dtmf_multiply16_round (int16 v1, int16 v2)
{
    int16 prod16;
    int32 acc;

    acc = (int32)(v1*v2) + (int32) 0x00004000L;
    prod16= (int16)(acc >> 15);

    if (prod16 > (int16)0x7fff)  return (int16)0x7fff;
    if (prod16 < ~(int16)0x7fff) return (int16)0x8000;

		return (int16)prod16;

}

int16 audio_ftm_dtmf_mac16_round (int32 v3, int16 v1, int16 v2)
{
    int16 out;
    int32 acc;
    int32 prod;

    prod = audio_ftm_dtmf_multiply16 (v1, v2);
    acc = audio_ftm_dtmf_add32 (v3, prod);
    acc = audio_ftm_dtmf_add32 (acc, (int32) 0x00008000L);
    out =  (int16) (acc >> 16);

    return (out);
}


#ifdef __cplusplus
}
#endif

