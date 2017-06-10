#ifndef AUDIO_FTM_PCM_LOOPBACK_H
#define AUDIO_FTM_PCM_LOOPBACK_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file audio_ftm_pcm_loopback.h
  @brief  Audio FTM PCM Loopback Driver API
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_pcm_loopback.h#1 $
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
#include "audio_ftm_driver_fwk.h"

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


/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_attach
(
    void *,                             /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *        /* Output: driver handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_open
(
    AUDIO_FTM_DRIVER_HANDLE_T,         /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T AccessMode,  /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T  ShareMode,   /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T *        /* Output: client handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_close
(
    AUDIO_FTM_CLIENT_HANDLE_T         /* Input: client handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_read
(
    AUDIO_FTM_CLIENT_HANDLE_T,        /* Input: client handle */
    void *,                           /* Input: buffer pointer for reading  */
    uint32 nBufSize,                  /* Input: Read buffer size */
    uint32 *pCount                    /* Output: return the actual read bytes */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_write
(
    AUDIO_FTM_CLIENT_HANDLE_T,        /* Input: client handle */
    void *,                           /* Input: buffer pointer containing data for writing */
    uint32 nBufSize                  /* Input: Write buffer size */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_iocontrol
(
    AUDIO_FTM_CLIENT_HANDLE_T,        /* Input: client handle */
    uint32 dwIOCode,                  /* Input: IOControl command */
    uint8 * pBufIn,                   /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,                   /* Input: parameter length */
    uint8 * pBufOut,                  /* Output: buffer pointer for outputs */
    uint32 dwLenOut,                  /* Input: expected output length */
    uint32 *pActualOutLen             /* Output: actual output length */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_lp_detach
(
    AUDIO_FTM_DRIVER_HANDLE_T
);

/*=================================================================================================*/
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_PCM_LOOPBACK_H */

