#ifndef AUDIO_FTM_DRIVER_H
#define AUDIO_FTM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file audio_ftm_driver.h
  @brief  Audio FTM Driver API
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_driver.h#2 $
$DateTime: 2011/04/07 20:45:53 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      05/30/2010                    File creation.



====================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
/* common Linux headers for sub-driver to include */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

/* audio FTM common types */
#include "DALSYS_common.h"

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/

/*------ IOCTLs -----*/
#define IOCTL_AUDIO_FTM_CHG_VOL         0x1000
#define IOCTL_AUDIO_FTM_START           0x1001
#define IOCTL_AUDIO_FTM_STOP            0x1002
#define IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE    0x1003   /* inquiry the device buffer size after configuring the sampling rate and chnnels */
#define IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE    0x1004   /* inquiry the device buffer size after configuring the sampling rate and chnnels */

/*==================================================================================================
                                            MACROS
==================================================================================================*/

/*--------- Handle Types  ---------*/
typedef void  * AUDIO_FTM_DRIVER_HANDLE_T;
typedef void  * AUDIO_FTM_CLIENT_HANDLE_T;


/*==================================================================================================
                                             ENUMS
==================================================================================================*/

typedef enum
{
  AUD_FTM_ACCESS_MODE_RD,
  AUD_FTM_ACCESS_MODE_WR,
  AUD_FTM_ACCESS_MODE_RD_WR,
  AUD_FTM_ACCESS_MODE_CTRL,
  AUD_FTM_ACCESS_MODE_MAX
}AUD_FTM_ACCESS_MODE_T;

typedef enum
{
  AUD_FTM_SHAREMODE_EXCLU,   /* exclusive mode */
  AUD_FTM_SHAREMODE_SHARE    /* share mode */
}AUD_FTM_SHARE_MODE_T;

/*-------- Error Codes -----------*/
typedef enum
{
  AUDIO_FTM_SUCCESS,
  AUDIO_FTM_ERROR,
  AUDIO_FTM_ERR_INVALID_PARAM,
  AUDIO_FTM_ERR_MEM_ALLOC_FAIL,
  AUDIO_FTM_ERR_WR_BUF_FULL,
  AUDIO_FTM_ERR_RD_BUF_EMPTY ,
  AUDIO_FTM_ERR_TIMEOUT ,
} AUDIO_FTM_STS_T;

/*-------- TEST METHODS -----------*/
typedef enum
{
  AUDIO_FTM_METHOD_TONE_PLAY,
  AUDIO_FTM_METHOD_PCM_LOOPBACK,
  AUDIO_FTM_METHOD_AFE_LOOPBACK,
  AUDIO_FTM_METHOD_PCM_CAPTURE,
  AUDIO_FTM_METHOD_PCM_PLAY,
  AUDIO_FTM_METHOD_MAX
} AUDIO_FTM_METHOD_ENUM_T;

/*------- INPUT PATH ----------*/
#define  AUDIO_FTM_IN_INVALID -1

typedef enum
{
  AUDIO_FTM_IN_HANDSET_MIC1=0,
  AUDIO_FTM_IN_HANDSET_MIC2,
  AUDIO_FTM_IN_HEADSET_MIC,
  AUDIO_FTM_IN_SPEAKER_MIC,
  AUDIO_FTM_IN_BT,
  AUDIO_FTM_IN_LINEIN_L,
  AUDIO_FTM_IN_LINEIN_R,
  AUDIO_FTM_IN_FM,
  AUDIO_FTM_IN_DMIC1,
  AUDIO_FTM_IN_DMIC2,
  AUDIO_FTM_IN_DMIC3,
  AUDIO_FTM_IN_DMIC4,
  AUDIO_FTM_IN_AUXIN,
  AUDIO_FTM_IN_DMIC1_DMIC2,
  AUDIO_FTM_IN_DMIC3_DMIC4,
  AUDIO_FTM_IN_MIC1_MIC2,
  AUDIO_FTM_IN_MIC1_AUXIN,
  AUDIO_FTM_IN_MIC1_BT,
  AUDIO_FTM_IN_ULTRASOUND_SENSOR,
  AUDIO_FTM_IN_HANDSET_MIC_ADIE_LP,
  AUDIO_FTM_IN_HEADSET_MIC_ADIE_LP,
  AUDIO_FTM_IN_PATH_MAX
}AUDIO_FTM_INPUT_PATH_ENUM_T;

/*------- OUTPUT PATH ----------*/
#define AUDIO_FTM_OUT_INVALID -1

typedef enum
{
  AUDIO_FTM_OUT_HANDSET=50,
  AUDIO_FTM_OUT_HEADSET_L,
  AUDIO_FTM_OUT_HEADSET_R,
  AUDIO_FTM_OUT_HEADSET_STEREO_L_R,
  AUDIO_FTM_OUT_HEADSET_MONO_L_R,
  AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R,
  AUDIO_FTM_OUT_SPEAKER1_L,
  AUDIO_FTM_OUT_SPEAKER1_R,
  AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R,
  AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,
  AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,
  AUDIO_FTM_OUT_AUXOUT,
  AUDIO_FTM_OUT_BT,
  AUDIO_FTM_OUT_FM,
  AUDIO_FTM_OUT_MI2S_SD0,
  AUDIO_FTM_OUT_MI2S_SD1,
  AUDIO_FTM_OUT_MI2S_SD2,
  AUDIO_FTM_OUT_MI2S_SD3,
  AUDIO_FTM_OUT_HANDSET_BT,
  AUDIO_FTM_OUT_HANDSET_ADIE_LP,
  AUDIO_FTM_OUT_HEADSET_L_ADIE_LP,
  AUDIO_FTM_OUT_HEADSET_R_ADIE_LP,
  AUDIO_FTM_OUT_SPK_L_ADIE_LP,
  AUDIO_FTM_OUT_SPK_R_ADIE_LP,
  AUDIO_FTM_OUT_PATH_MAX
}AUDIO_FTM_OUTPUT_PATH_ENUM_T;

typedef enum
{
  AUDIO_FTM_PCM_RATE_8K=8000,
  AUDIO_FTM_PCM_RATE_11K=11025,
  AUDIO_FTM_PCM_RATE_12K=12000,
  AUDIO_FTM_PCM_RATE_16K=16000,
  AUDIO_FTM_PCM_RATE_22K=22050,
  AUDIO_FTM_PCM_RATE_24K=24000,
  AUDIO_FTM_PCM_RATE_32K=32000,
  AUDIO_FTM_PCM_RATE_44K=44100,
  AUDIO_FTM_PCM_RATE_48K=48000
}AUDIO_FTM_PCM_RATE_T;

typedef enum
{
  AUDIO_FTM_CHN_1=1,
  AUDIO_FTM_CHN_2=2,
  AUDIO_FTM_CHN_4=4,
  AUDIO_FTM_CHN_6=6,
  AUDIO_FTM_CHN_8=8
}AUDIO_FTM_CHN_T;

typedef enum
{
  AUDIO_FTM_BIT_WIDTH_8=8,
  AUDIO_FTM_BIT_WIDTH_16=16,
  AUDIO_FTM_BIT_WIDTH_32=32
}AUDIO_FTM_BIT_WIDTH_T;

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*---- INPUT/OUTPUT PARIING ----*/
typedef struct
{
  AUDIO_FTM_INPUT_PATH_ENUM_T  inpath;
  AUDIO_FTM_OUTPUT_PATH_ENUM_T outpath;
} AUDIO_FTM_PATH_PAIR_T;

/*---- Tone Play Parameter Structure ----*/
typedef struct
{
   AUDIO_FTM_PATH_PAIR_T  path;
   uint16                 gain;      /* Volume gain */
   uint16                 dtmf_hi;   /* High freq in Hz  */
   uint16                 dtmf_low;  /* Low freq in Hz   */
   uint16                 channel;
   int                    device;
}AUD_FTM_TONE_PLAY_PARAM_T;

/*---- PCM_LOOPBACK Parameter Structure ----*/
typedef struct
{
   AUDIO_FTM_PATH_PAIR_T  path;
   uint16                 gain;       /* Volume gain */
   int                    device;
}AUD_FTM_PCM_LP_PARAM_T;

/*---- AFE_LOOPBACK Parameter Structure ----*/
typedef struct
{
   AUDIO_FTM_PATH_PAIR_T  path;
   uint16                 gain;       /* Volume gain */
   uint16                 channel;
   int                    playbackdevice;
   int                    capturedevice;
}AUD_FTM_AFE_LP_PARAM_T;

/*---- PCM CAPTURE Parameter Structure ----*/
typedef struct
{
   AUDIO_FTM_PATH_PAIR_T  path;
   uint16                 gain;        /* Volume gain */
   uint32                 size;        /* total bytes for record */
   void                  *pData_buf;   /* buf to save captured data, allocate/free by client */
   char                  *pFileName;   /* Driver save data to flash file, client get data from it*/
   uint16                channel;
   int                    device;
}AUD_FTM_PCM_REC_PARAM_T;

typedef struct
{
   AUDIO_FTM_PATH_PAIR_T  path;
   uint16                 gain;        /* Volume gain */
   char                  *pFileName;   /* File to be played*/
   uint16                channel;
   int                    device;
}AUD_FTM_PCM_PLAY_PARAM_T;

/* callback function for PCM playback
pBuf - In:  client need to put data into this buffer
size - In:  byte length requested for data filling

Return:  actually len filled in
*/
typedef uint32
(*Audio_FTM_PCM_PLAY_CALLBACK_FncPtr)
(
    uint16 *pBuf,
    uint32 size
);


/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/*
  @brief FTM Driver Attach API

  This API can be used to attach the FTM driver.

  @param nMethod      : Input - FTM test method enumeration
         pDriverHdl   : Output - a driver handle will be returned if attach success

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Attach
(
    AUDIO_FTM_METHOD_ENUM_T nMethod,
    void *,
    AUDIO_FTM_DRIVER_HANDLE_T * pDriverHdl
);

/*
  @brief FTM driver open

  This API can be used to open the FTM driver.

  @param DriverHdl:   Input -   handle created in Audio_FTM_Attach
         AccessMode:  Input -   Read / Write / Rd_Wr
         ShareMode:   Input -   Exclusive or Share
         pClientHdl:  Output -  point to the returned Open handle

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Open
(
    AUDIO_FTM_DRIVER_HANDLE_T DriverHdl,  /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T     AccessMode, /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T      ShareMode,  /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T *pClientHdl /* Output: client handle */
);


/*
  @brief FTM driver Close

  This API can be used to Close the FTM driver.

  @param OpenHdl:   Input -   handle created in Audio_FTM_Open

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Close
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl        /* Input: client handle */
);

/*
  @brief FTM driver Read

  This API can be used to Read data from FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to buffer which stores data read from driver
         nBufSize     Input -   Read buffer size
         pCount:      Output -  return the actual read bytes

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Read
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    void * pBuf,
    uint32 nBufSize,
    uint32 *pCount

);

/*
  @brief FTM driver Write

  This API can be used to Write data to FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to data buffer which will be written to driver
         nCount:      Input -   Bytes count

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Write
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    void * pBuf,
    uint32 nCount
);


/*
  @brief FTM driver IOControl

  This API can be used to set/read parameters or commands to/from FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         dwIOCode     Input -   IOControl command
         pBufIn       Input -   buffer pointer for input parameters
         dwLenIn      Input -   Input parameter length
         pBufOut      Input -   buffer pointer for outputs
         dwLenOut     Input -   expected output length
         pActualOutLen Output - actual output length

   @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_IOControl
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    uint32 dwIOCode,
    uint8 * pBufIn,
    uint32  dwLenIn,
    uint8 * pBufOut,
    uint32  dwLenOut,
    uint32 *pActualOutLen
);

/*
  @brief FTM driver Detach

  This API can be used to Detach FTM driver, it will release all resournces

  @param DriverHdl:     Input -   handle created in Audio_FTM_Attach

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
*/

AUDIO_FTM_STS_T
Audio_FTM_Detach
(
    AUDIO_FTM_DRIVER_HANDLE_T DriverHdl
);


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_DRIVER_H */
