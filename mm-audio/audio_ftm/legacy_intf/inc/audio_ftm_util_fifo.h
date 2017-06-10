#ifndef AUDIO_FTM_UTIL_FIFO_H
#define AUDIO_FTM_UTIL_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file aud_ftm_util_fifo.h
  @brief  Audio FTM Hardware Driver API
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_util_fifo.h#1 $
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
typedef struct
{
    int32   rd_pointer;
    int32   wr_pointer;
    int32   size;
    uint8   *buffer;
    DALSYSSyncHandle  hSync;
} AUD_FTM_CIRCULAR_BUF_T;

/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
int32 cbuf_data_size(AUD_FTM_CIRCULAR_BUF_T *rb);

int32 cbuf_capacity(AUD_FTM_CIRCULAR_BUF_T *rb);

DALBOOL aud_ftm_cbuf_init(AUD_FTM_CIRCULAR_BUF_T *cbuf, uint16 sz);

int32 aud_ftm_cbuf_write (AUD_FTM_CIRCULAR_BUF_T *cbuf, uint8 * buf, int32 len);

int32 aud_ftm_cbuf_read (AUD_FTM_CIRCULAR_BUF_T *cbuf, uint8 * buf, int max);

DALBOOL aud_ftm_cbuf_deinit(AUD_FTM_CIRCULAR_BUF_T *cbuf);
void aud_ftm_cbuf_clear (AUD_FTM_CIRCULAR_BUF_T *cbuf);


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_UTIL_FIFO_H */
