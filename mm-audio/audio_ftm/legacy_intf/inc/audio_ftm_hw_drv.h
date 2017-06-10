#ifndef AUDIO_FTM_HW_DRV_H
#define AUDIO_FTM_HW_DRV_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file audio_ftm_hw_drv.h
  @brief  Audio FTM OS dependent Layer API
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_hw_drv.h#1 $
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
#ifdef MSM8960_ALSA
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sound/asound.h>
#include <alsa_audio.h>
#endif

#include "audio_ftm_driver.h"
#include "audio_ftm_driver_fwk.h"

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/


/*==================================================================================================
                                            MACROS
==================================================================================================*/

#define DIR_RX 1
#define DIR_TX 2


#define AUDIO_FTM_TC_TX_HANDSET_MIC1				1
#define AUDIO_FTM_TC_TX_HANDSET_MIC2				2
#define AUDIO_FTM_TC_RX_HANDSET_EARPHONE			3
#define AUDIO_FTM_TC_TX_HEADSET_MIC				4
#define AUDIO_FTM_TC_RX_HEADSET_SPKR_MONO			5
#define AUDIO_FTM_TC_RX_HEADSET_SPKR_STEREO			6
#define AUDIO_FTM_TC_RX_HEADSET_SPKR_MONO_DIFF			7
#define AUDIO_FTM_TC_RX_HEADSET_SPKR_LEFT			8
#define AUDIO_FTM_TC_RX_HEADSET_SPKR_RIGHT			9
#define AUDIO_FTM_TC_TX_SPKR_PHONE_MIC				10
#define AUDIO_FTM_TC_RX_SPKR_PHONE_STEREO			11
#define AUDIO_FTM_TC_RX_SPKR_PHONE_MONO				12
#define AUDIO_FTM_TC_RX_SPKR_PHONE_LEFT				13
#define AUDIO_FTM_TC_RX_SPKR_PHONE_RIGHT			14
#define AUDIO_FTM_TC_RX_LINEOUT_RIGHT_DIFF			15
#define AUDIO_FTM_TC_RX_LINEOUT_LEFT_DIFF			16
#define AUDIO_FTM_TC_TX_LINEIN_LEFT				17
#define AUDIO_FTM_TC_TX_LINEIN_RIGHT				18
#define AUDIO_FTM_TC_TX_AUDIO_INPUT3				19
#define AUDIO_FTM_TC_TX_AUDIO_INPUT4				20
#define AUDIO_FTM_TC_RX_LINEOUT5				21
#define AUDIO_FTM_TC_TX_DMIC1_LEFT				22
#define AUDIO_FTM_TC_TX_DMIC1_RIGHT				23
#define AUDIO_FTM_TC_TX_DMIC1_LEFT_RIGHT			24
#define AUDIO_FTM_TC_TX_DMIC2_LEFT				25
#define AUDIO_FTM_TC_TX_DMIC2_RIGHT				26
#define AUDIO_FTM_TC_TX_DMIC2_LEFT_RIGHT			27
#define AUDIO_FTM_TC_TX_ANALOG_INPUT1_ANALOG_INPUT3		28
#define AUDIO_FTM_TC_TX_ANALOG_INPUT1_ANALOG_INPUT2		29
#define AUDIO_FTM_TC_TX_FM_STEREO_INPUT				30
#define AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT			31
#define AUDIO_FTM_TC_TX_BT_SCO					32
#define AUDIO_FTM_TC_RX_BT_SCO					33
#define AUDIO_FTM_TC_LB_AFE_HANDSET_MIC_HANDSET_EARPHONE	34
#define AUDIO_FTM_TC_LB_AFE_HEADSET_MIC_HEADSET_L_R		35
#define AUDIO_FTM_TC_LB_AFE_SPKR_PHONE_MIC_SPKR_PHONE_L_R	36
#define AUDIO_FTM_TC_LB_AFE_HEADSET_MIC_HEADSET_L_R_VOL_0	37
#define AUDIO_FTM_TC_LB_AFE_HEADSET_MIC_HEADSET_L_R_VOL_25	38
#define AUDIO_FTM_TC_LB_AFE_HEADSET_MIC_HEADSET_L_R_VOL_75	39
#define AUDIO_FTM_TC_LB_ADIE_HANDSET_MIC_HANDSET_EARPHONE	40
#define AUDIO_FTM_TC_LB_ADIE_HEADSET_MIC_HEADSET_L		41
#define AUDIO_FTM_TC_LB_ADIE_HEADSET_MIC_HEADSET_R		42
#define AUDIO_FTM_TC_LB_ADIE_SPKR_PHONE_MIC_SPKR_PHONE_L	43
#define AUDIO_FTM_TC_LB_ADIE_SPKR_PHONE_MIC_SPKR_PHONE_R	44
#define AUDIO_FTM_TC_NUM_DEVS					45

#define PATH_NULL	0
#define PATH_RX		1
#define PATH_TX		2
#define PATH_AFE_LB	3
#define PATH_ADIE_LB	4
#define PATH_FM		5

/*
To get the latest device numbers, cat /proc/asound/pcm on target.
*/

#define AUDIO_FTM_ALSA_DEVICE_MM1		"hw:0,0"
#define AUDIO_FTM_ALSA_DEVICE_BT_SCO_RX		"hw:0,9"
#define AUDIO_FTM_ALSA_DEVICE_BT_SCO_TX		"hw:0,10"
#define AUDIO_FTM_ALSA_DEVICE_FM_RX		"hw:0,11"
#define AUDIO_FTM_ALSA_DEVICE_FM_TX		"hw:0,12"

struct ftm_tc_device {
	int (*enable)(struct mixer *, int);
	int (*set_volume)(struct mixer *, int);
	int path_type;
	int tc_vol;
};

extern int g_curr_rx_device;
extern int g_curr_tx_device;
extern int g_curr_afe_lb_device;
extern int g_curr_adie_lb_device;
extern int g_curr_fm_device;
extern int g_curr_device;
extern const char *g_curr_alsa_device_name;
extern struct ftm_tc_device ftm_tc_devices[];

int audio_ftm_fm_hostless_en(int enable);
void audio_ftm_fm_device_set(int capture , int playback);

/*==================================================================================================
                                             ENUMS
==================================================================================================*/
typedef enum
{
  AUDIO_FTM_HW_DRV_UN_INITIALIZED,
	AUDIO_FTM_HW_DRV_INITIALIZED,
	AUDIO_FTM_HW_DRV_OPENED,
	AUDIO_FTM_HW_DRV_STARTED,
	AUDIO_FTM_HW_DRV_STOPPED,
	AUDIO_FTM_HW_DRV_CLOSED
} AUDIO_FTM_HW_DRV_STATE_T;

typedef enum
{
  AUDIO_FTM_AFE_LOOPBACK,
	AUDIO_FTM_ADIE_LOOPBACK
} AUDIO_FTM_LOOPBACK_T;


/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
struct Aud_FTM_DrvCtxt_S;

typedef struct _Aud_FTM_DevCtxt
{
   AUDIO_FTM_HW_DRV_STATE_T m_state;
   struct Aud_FTM_DrvCtxt_S      *pDrvCtxt;

   struct pcm			*pcm;
   int		read_write_flag;

   int32                      rx_dev_id;      /* HW Rx device ID */
   int32                      tx_dev_id;      /* HW Tx device ID */
   AUDIO_FTM_PCM_RATE_T       sampleRate;
   int32                      numChannels;
   int32                      bitWidth;
	 uint16                     gain;
	 int32                      rx_fd;         /* file handle for PCM decoding */
	 int32                      tx_fd;         /* file handle for PCM encoding */
	 uint16                     tx_session_id;  /* PCM Record session */
	 uint16                     rx_session_id;  /* PCM playback session */
	 uint32                     rx_buf_size;
	 uint32                     tx_buf_size;
	 DALBOOL                    bLoopbackCase;
	 AUDIO_FTM_LOOPBACK_T       m_loopback_type;
	 int   capturedevice;
	 int   playbackdevice;
} Aud_FTM_DevCtxt_T;

typedef struct
{
   AUDIO_FTM_INPUT_PATH_ENUM_T  inpath;
   AUDIO_FTM_OUTPUT_PATH_ENUM_T outpath;
   AUDIO_FTM_PCM_RATE_T     rate;
   AUDIO_FTM_BIT_WIDTH_T    width;
   AUDIO_FTM_CHN_T          channel;
   uint16                   gain;
	 DALBOOL                  bLoopbackCase;
	 AUDIO_FTM_LOOPBACK_T     m_loopback_type;
}Aud_FTM_HW_INIT_PARAM_T;

typedef struct _Aud_FTM_Input_Dev_Item
{
  AUDIO_FTM_INPUT_PATH_ENUM_T id;
	char *dev_name;
}Aud_FTM_Input_Dev_Item_T;

typedef struct _Aud_FTM_Output_Dev_Item
{
  AUDIO_FTM_OUTPUT_PATH_ENUM_T id;
	char *dev_name;
}Aud_FTM_Output_Dev_Item_T;


/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
  @brief FTM Driver HW driver init

  @param pCtxt        : Input - Device context passed from MDD driver
         pInitParam   : Input - parameters for initialization

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T  aud_ftm_hw_init
(
				Aud_FTM_DevCtxt_T  *pCtxt,
				Aud_FTM_HW_INIT_PARAM_T   *pInitParam
);

/*==================================================================================================
  @brief FTM HW driver open

  This API can be used to open the FTM HW driver.

  @param pDevCtxt:    Input -   handle to device context
  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
audio_ftm_hw_open
(
    Aud_FTM_DevCtxt_T  *pDevCtxt      /* Input: handle to hw driver */
);


/*==================================================================================================
  @brief FTM HW driver Close

  This API can be used to Close the FTM HW driver.

  @param pDevCtxt:   Input -   handle to device context

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
audio_ftm_hw_close
(
    Aud_FTM_DevCtxt_T  *pDevCtxt      /* Input: handle to hw driver */
);

/*==================================================================================================
  @brief FTM HW driver Read

  This API can be used to Read data from FTM HW driver

  @param pDevCtxt:    Input -   handle to device context
         pBuf         Input -   point to buffer which stores data read from driver
         nBufSize     Input -   Read buffer size
         pCount:      Output -  return the actual read bytes

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_hw_read
(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    void * pBuf,																							  /* Input: buffer pointer for reading  */
    uint32 nSamples,                                      /* Input: how many samples will be read */
    uint32 *pCount                                        /* Output: return the actual read bytes */
);


/*==================================================================================================
  @brief FTM driver Write

  This API can be used to Write data to FTM HW driver

  @param pDevCtxt:    Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to data buffer which will be written to driver
         nBufSize:    Input -   Write buffer size

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_hw_write
(
    Aud_FTM_DevCtxt_T  *pDevCtxt,    /* Input: handle to hw driver */
    void *pBuf,                       /* Input: buffer pointer containing data for writing */
    uint32 nSamples                  /* Input: Write nSamples sample data */
);

/*==================================================================================================
  @brief FTM HW driver IOControl

  This API can be used to set/read parameters or commands to/from FTM HW driver

  @param pDevCtxt:    Input -   handle to device context
         dwIOCode     Input -   IOControl command
         pBufIn       Input -   buffer pointer for input parameters
         dwLenIn      Input -   Input parameter length
         pBufOut      Input -   buffer pointer for outputs
         dwLenOut     Input -   expected output length
         pActualOutLen Output - actual output length

   @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
audio_ftm_hw_iocontrol
(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    uint32 dwIOCode,                  /* Input: IOControl command */
    uint8 * pBufIn,                   /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,                   /* Input: parameter length */
    uint8 * pBufOut,                  /* Output: buffer pointer for outputs */
    uint32 dwLenOut,                  /* Input: expected output length */
    uint32 *pActualOutLen             /* Output: actual output length */
);


/*==================================================================================================
  @brief FTM Driver HW driver deinit

  @param pDevCtxt:    Input -   handle to device context

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T aud_ftm_hw_deinit
(
    Aud_FTM_DevCtxt_T  *pDevCtxt
);


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* AUDIO_FTM_HW_DRV_H */
