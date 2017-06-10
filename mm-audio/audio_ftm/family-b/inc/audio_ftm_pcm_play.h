/*==========================================================================

                     Header file for FTM Audio PCM Play support

Description
  Header file exposes the interface for FTM pcm playback

===========================================================================*/
/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 */

/*
 * Copyright 2010, The Android Open-Source Project
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIO_FTM_PCM_PLAY_H
#define AUDIO_FTM_PCM_PLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


/*==============================================================================
                                         INCLUDE FILES
==============================================================================*/

#include "DALSYS_common.h"
#include "audio_ftm_driver_fwk.h"

/*==============================================================================
                                           CONSTANTS
==============================================================================*/



/*==============================================================================
                                            MACROS
==============================================================================*/

/*==============================================================================
                                             ENUMS
==============================================================================*/


/*==============================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==============================================================================*/


/*==============================================================================
                                 GLOBAL VARIABLE DECLARATION
==============================================================================*/


/*==============================================================================
                                     FUNCTION PROTOTYPES
==============================================================================*/

AUDIO_FTM_STS_T
audio_ftm_pcm_play_attach
(
    void *,                             /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *        /* Output: driver handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_open
(
    AUDIO_FTM_DRIVER_HANDLE_T,         /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T AccessMode,  /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T  ShareMode,   /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T *        /* Output: client handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_close
(
    AUDIO_FTM_CLIENT_HANDLE_T         /* Input: client handle */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_read
(
    AUDIO_FTM_CLIENT_HANDLE_T,        /* Input: client handle */
    void *,                           /* Input: buffer pointer for reading  */
    uint32 nBufSize,                  /* Input: Read buffer size */
    uint32 *pCount                    /* Output: return the actual read bytes */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_write
(
    AUDIO_FTM_CLIENT_HANDLE_T,  /* Input: client handle */
    void *,     /* Input: buffer pointer containing data for writing */
    uint32 nBufSize                 /* Input: Write buffer size */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_iocontrol
(
    AUDIO_FTM_CLIENT_HANDLE_T,        /* Input: client handle */
    uint32 dwIOCode,                  /* Input: IOControl command */
    uint8 * pBufIn,            /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,                   /* Input: parameter length */
    uint8 * pBufOut,                  /* Output: buffer pointer for outputs */
    uint32 dwLenOut,                  /* Input: expected output length */
    uint32 *pActualOutLen             /* Output: actual output length */
);

AUDIO_FTM_STS_T
audio_ftm_pcm_play_detach
(
    AUDIO_FTM_DRIVER_HANDLE_T
);

/*============================================================================*/
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_PCM_PLAY_H */
