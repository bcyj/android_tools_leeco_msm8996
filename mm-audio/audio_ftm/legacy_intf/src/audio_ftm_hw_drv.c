#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_hw_drv.c
  @brief  This driver is OS and Platform dependant.
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_hw_drv.c#2 $
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

#include "audio_ftm_hw_drv.h"
#include "control.h"       /* under vendor\qcom\proprietary\mm-audio\audio-alsa\inc  */

#include <stdio.h>
#include "msm_audio.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/



/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/

#define ALSA_PCM_PLAY_DEVICE  "/dev/msm_pcm_out"
#define ALSA_PCM_REC_DEVICE   "/dev/msm_pcm_in"

#define ON  1
#define OFF 0

/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/

static DALBOOL    g_bDriverInitialized=FALSE;   //  whether it has been inited or not

static Aud_FTM_Input_Dev_Item_T  tx_device_tbl[]=
{
  {AUDIO_FTM_IN_HANDSET_MIC1,"handset_tx"},
  {AUDIO_FTM_IN_HANDSET_MIC2,"ftm_handset_tx2"},
  {AUDIO_FTM_IN_HEADSET_MIC,"headset_mono_tx"},
  {AUDIO_FTM_IN_SPEAKER_MIC,"speaker_mono_tx"},
  {AUDIO_FTM_IN_BT,"bt_sco_tx"},
  {AUDIO_FTM_IN_LINEIN_L,"ftm_linein_l_tx"},
  {AUDIO_FTM_IN_LINEIN_R,"ftm_linein_r_tx"},
  {AUDIO_FTM_IN_FM,"fm_stereo_tx"},
  {AUDIO_FTM_IN_DMIC1,"ftm_dmic1_left_tx"},
  {AUDIO_FTM_IN_DMIC2,"ftm_dmic1_right_tx"},
  {AUDIO_FTM_IN_DMIC3,"ftm_dmic2_left_tx"},
  {AUDIO_FTM_IN_DMIC4,"ftm_dmic2_right_tx"},
  {AUDIO_FTM_IN_AUXIN,"ftm_handset_mic1_aux_in"},
  {AUDIO_FTM_IN_DMIC1_DMIC2,"ftm_dmic1_l_and_r_tx"},
  {AUDIO_FTM_IN_DMIC3_DMIC4,"ftm_dmic2_l_and_r_tx"},
  {AUDIO_FTM_IN_MIC1_MIC2,"handset_mic1_handset_mic2"},
  {AUDIO_FTM_IN_MIC1_AUXIN,"handset_mic1_aux_in"},
  {AUDIO_FTM_IN_HANDSET_MIC_ADIE_LP,"ftm_handset_mic_adie_lp_tx"},
  {AUDIO_FTM_IN_HEADSET_MIC_ADIE_LP,"ftm_headset_mic_adie_lp_tx"}
};

static Aud_FTM_Output_Dev_Item_T  rx_device_tbl[]=
{
  {AUDIO_FTM_OUT_HANDSET,"handset_rx"},
  {AUDIO_FTM_OUT_HEADSET_L,"ftm_headset_mono_l_rx"},
  {AUDIO_FTM_OUT_HEADSET_R,"ftm_headset_mono_r_rx"},
  {AUDIO_FTM_OUT_HEADSET_STEREO_L_R,"headset_stereo_rx"},
  {AUDIO_FTM_OUT_HEADSET_MONO_L_R,"ftm_headset_mono_rx"},
  {AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R,"ftm_headset_mono_diff_rx"},
  {AUDIO_FTM_OUT_SPEAKER1_L,"ftm_spkr_l_rx"},
  {AUDIO_FTM_OUT_SPEAKER1_R,"ftm_spkr_r_rx"},
  {AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R,"speaker_stereo_rx"},
  {AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,"ftm_spkr_mono_rx"},
  {AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,"ftm_spkr_mono_diff_rx"},
  {AUDIO_FTM_OUT_AUXOUT,"ftm_aux_out_rx"},
  {AUDIO_FTM_OUT_BT,"bt_sco_rx"},
  {AUDIO_FTM_OUT_FM,"fm_stereo_rx"},
  {AUDIO_FTM_OUT_MI2S_SD0,"mi2s_sd0_rx"},
  {AUDIO_FTM_OUT_MI2S_SD1,"mi2s_sd1_rx"},
  {AUDIO_FTM_OUT_MI2S_SD2,"mi2s_sd2_rx"},
  {AUDIO_FTM_OUT_MI2S_SD3,"mi2s_sd3_rx"},
  {AUDIO_FTM_OUT_HANDSET_ADIE_LP,"ftm_handset_adie_lp_rx"},
  {AUDIO_FTM_OUT_HEADSET_L_ADIE_LP,"ftm_headset_l_adie_lp_rx"},
  {AUDIO_FTM_OUT_HEADSET_R_ADIE_LP,"ftm_headset_r_adie_lp_rx"},
  {AUDIO_FTM_OUT_SPK_L_ADIE_LP,"ftm_spk_l_adie_lp_rx"},
  {AUDIO_FTM_OUT_SPK_R_ADIE_LP,"ftm_spk_r_adie_lp_rx"}
};

static char * pRxDevName, *pTxDevName;


/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================
  @brief convert path to device name ALSA expects

  @param iPath:   Input -   AUDIO_FTM_INPUT_PATH_ENUM_T

  @return return: device name string
===============================================================================*/
static char * get_tx_device_name(AUDIO_FTM_INPUT_PATH_ENUM_T iPath)
{
  int16 i,size;

  size=sizeof(tx_device_tbl)/sizeof(Aud_FTM_Input_Dev_Item_T);
  for(i=0; i<size; i++)
  {
    if(iPath == tx_device_tbl[i].id)
      break;
  }
  if( i >= size )  return NULL;
  return tx_device_tbl[i].dev_name;
}

/*=============================================================================
  @brief convert path to device name ALSA expects

  @param iPath:   Input -   AUDIO_FTM_OUTPUT_PATH_ENUM_T

  @return return: device name string
===============================================================================*/
static char * get_rx_device_name(AUDIO_FTM_OUTPUT_PATH_ENUM_T oPath)
{
  int16 i,size;

  size=sizeof(rx_device_tbl)/sizeof(Aud_FTM_Output_Dev_Item_T);
  for(i=0; i<size; i++)
  {
    if(oPath == rx_device_tbl[i].id)
      break;
  }
  if( i >= size )  return NULL;
  return rx_device_tbl[i].dev_name;
}

/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
  @brief FTM Driver HW driver init

  @param pCtxt        : Input - Device context passed from MDD driver
         pInitParam   : Input - parameters for initialization

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T aud_ftm_hw_init
(
    Aud_FTM_DevCtxt_T  *pCtxt,
    Aud_FTM_HW_INIT_PARAM_T *pInitParam
)
{
     int32 alsa_ctl;
   AUDIO_FTM_STS_T res;
     static DALBOOL bIsDBInitialized;
     char *pDevName;

   bIsDBInitialized = FALSE;
   res= AUDIO_FTM_SUCCESS;

   if(g_bDriverInitialized != TRUE)
   {
         alsa_ctl = msm_mixer_open("/dev/snd/controlC0", 0);
     if (alsa_ctl < 0)
     {
             DALSYS_Log_Err("Fail to open ALSA MIXER\n");
             return AUDIO_FTM_ERROR;
     }
         g_bDriverInitialized=TRUE;
   }

   pRxDevName=get_rx_device_name(pInitParam->outpath);
     if(pRxDevName == NULL)
          pCtxt->rx_dev_id=-ENODEV;
     else
     pCtxt->rx_dev_id=msm_get_device(pRxDevName);

   pTxDevName=get_tx_device_name(pInitParam->inpath);
     if(pTxDevName == NULL)
          pCtxt->tx_dev_id=-ENODEV;
     else
     pCtxt->tx_dev_id=msm_get_device(pTxDevName);

   pCtxt->sampleRate=pInitParam->rate;
     pCtxt->bitWidth=pInitParam->width;
     pCtxt->numChannels=pInitParam->channel;
     pCtxt->gain=pInitParam->gain;
     pCtxt->rx_buf_size=0;
   pCtxt->tx_buf_size=0;
     pCtxt->bLoopbackCase=pInitParam->bLoopbackCase;
     pCtxt->m_loopback_type=pInitParam->m_loopback_type;
   if(pCtxt->bLoopbackCase == TRUE)
   {
     if( (pCtxt->rx_dev_id < 0) || (pCtxt->tx_dev_id < 0) )
             return AUDIO_FTM_ERROR;
   }

   if((pInitParam->inpath !=AUDIO_FTM_IN_INVALID) && (pCtxt->tx_dev_id < 0))
             return AUDIO_FTM_ERROR;

   if((pInitParam->outpath !=AUDIO_FTM_OUT_INVALID) && (pCtxt->rx_dev_id < 0))
             return AUDIO_FTM_ERROR;

     pCtxt->m_state=AUDIO_FTM_HW_DRV_INITIALIZED;

   return AUDIO_FTM_SUCCESS;

}

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
)
{
    AUDIO_FTM_STS_T  ret;
    int dev_fd;
    uint16  session_id;
    struct msm_audio_config config;


    if( pDevCtxt->m_state != AUDIO_FTM_HW_DRV_INITIALIZED)
    {
      DALSYS_Log_Err("Fail: Open must be run after initialization only\n");
    return AUDIO_FTM_ERROR;
    }

    if((pDevCtxt->rx_dev_id != -ENODEV) && (pDevCtxt->bLoopbackCase != TRUE))
    {
        dev_fd = open(ALSA_PCM_PLAY_DEVICE, O_WRONLY);

        if (dev_fd < 0) {
            DALSYS_Log_Err("Failed to open Rx device\n");
            return AUDIO_FTM_ERROR;
        }
        pDevCtxt->rx_fd=dev_fd;

        if (ioctl(dev_fd, AUDIO_GET_SESSION_ID, &session_id))
        {
            DALSYS_Log_Err("could not get decoder session id\n");
            close(dev_fd);
            return AUDIO_FTM_ERROR;
        }

        pDevCtxt->rx_session_id=session_id;

      if (msm_en_device(pDevCtxt->rx_dev_id, ON) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be enabled\n", pRxDevName);
          return AUDIO_FTM_ERROR;
      }

     /* connect the HW device with decoding session */
    if (msm_route_stream(DIR_RX, session_id, pDevCtxt->rx_dev_id, ON) < 0)
        {
            DALSYS_Log_Err("could not route stream to Device %s\n",pRxDevName);

      if (msm_en_device(pDevCtxt->rx_dev_id, OFF) < 0)
      {
        DALSYS_Log_Err("Failed to disable device %s\n",pRxDevName);
                return AUDIO_FTM_ERROR;
      }
        }

        if (ioctl(dev_fd, AUDIO_GET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not get device config\n");
            return AUDIO_FTM_ERROR;
        }

        config.channel_count = pDevCtxt->numChannels;
        config.sample_rate = pDevCtxt->sampleRate;
        config.bits=pDevCtxt->bitWidth;

        if (ioctl(dev_fd, AUDIO_SET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not set config\n");
            return AUDIO_FTM_ERROR;
        }

    if(ioctl(dev_fd, AUDIO_GET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not get config\n");
            return AUDIO_FTM_ERROR;
        }

        pDevCtxt->rx_buf_size=config.buffer_size;
 }

  if((pDevCtxt->tx_dev_id != -ENODEV) && (pDevCtxt->bLoopbackCase != TRUE))
  {
        dev_fd = open(ALSA_PCM_REC_DEVICE, O_RDONLY);

        if (dev_fd < 0) {
            DALSYS_Log_Err("Failed to open Tx device\n");
            return AUDIO_FTM_ERROR;
        }
        pDevCtxt->tx_fd=dev_fd;

        if (ioctl(dev_fd, AUDIO_GET_SESSION_ID, &session_id))
        {
            DALSYS_Log_Err("could not get decoder session id\n");
            close(dev_fd);
            return AUDIO_FTM_ERROR;
        }

        pDevCtxt->tx_session_id=session_id;

      if (msm_en_device(pDevCtxt->tx_dev_id, ON) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be enabled\n", pTxDevName);
          return AUDIO_FTM_ERROR;
      }
     /* connect the HW device with encoding session */
    if (msm_route_stream(DIR_TX, session_id, pDevCtxt->tx_dev_id, ON) < 0)
        {
            DALSYS_Log_Err("could not route stream to Device %s\n",pTxDevName);

      if (msm_en_device(pDevCtxt->tx_dev_id, OFF) < 0)
      {
        DALSYS_Log_Err("Failed to disable device %s\n",pTxDevName);
                return AUDIO_FTM_ERROR;
      }
        }

        if (ioctl(dev_fd, AUDIO_GET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not get device config\n");
            return AUDIO_FTM_ERROR;
        }

        config.channel_count = pDevCtxt->numChannels;
        config.sample_rate = pDevCtxt->sampleRate;
        config.bits=pDevCtxt->bitWidth;

        if (ioctl(dev_fd, AUDIO_SET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not set config\n");
            return AUDIO_FTM_ERROR;
        }

    if(ioctl(dev_fd, AUDIO_GET_CONFIG, &config))
        {
            DALSYS_Log_Err("could not get config\n");
            return AUDIO_FTM_ERROR;
        }

        pDevCtxt->tx_buf_size=config.buffer_size;
  }

    if(pDevCtxt->bLoopbackCase == TRUE)
    {
    if((pDevCtxt->tx_dev_id == -ENODEV) || (pDevCtxt->rx_dev_id == -ENODEV))
    {
            DALSYS_Log_Err("At least one device ID is invalid for loopback\n");
            return AUDIO_FTM_ERROR;
    }

      if (msm_en_device(pDevCtxt->rx_dev_id, ON) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be enabled\n", pRxDevName);
          return AUDIO_FTM_ERROR;
      }

      if (msm_en_device(pDevCtxt->tx_dev_id, ON) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be enabled\n", pTxDevName);
          return AUDIO_FTM_ERROR;
      }
    }

    pDevCtxt->m_state=AUDIO_FTM_HW_DRV_OPENED;

    return AUDIO_FTM_SUCCESS;
}

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
)
{
    AUDIO_FTM_STS_T  ret;

    ret = AUDIO_FTM_SUCCESS;

  if(pDevCtxt == NULL)  return AUDIO_FTM_ERR_INVALID_PARAM;

  /* Loopback */
  if(pDevCtxt->bLoopbackCase == TRUE)
  {
    if(pDevCtxt->tx_dev_id == -ENODEV)
    {
            DALSYS_Log_Err("Tx device ID is invalid, cannot close\n");
            ret=AUDIO_FTM_ERROR;
      }
        else
        {
          if (msm_en_device(pDevCtxt->tx_dev_id, OFF) < 0)
          {
            DALSYS_Log_Err("Device %s cannot be disabled\n", pTxDevName);
              ret=AUDIO_FTM_ERROR;
          }
        }

    if(pDevCtxt->rx_dev_id == -ENODEV)
    {
            DALSYS_Log_Err("Rx device ID is invalid, cannot close\n");
            ret=AUDIO_FTM_ERROR;
      }
        else
        {
          if (msm_en_device(pDevCtxt->rx_dev_id, OFF) < 0)
          {
            DALSYS_Log_Err("Device %s cannot be disabled\n", pRxDevName);
              ret=AUDIO_FTM_ERROR;
          }
        }
  }

  /* Rx */
  if((pDevCtxt->rx_dev_id != -ENODEV) && (pDevCtxt->bLoopbackCase != TRUE))
  {
     /* disconnect the HW device with decoding session */
    if (msm_route_stream(DIR_RX, pDevCtxt->rx_session_id, pDevCtxt->rx_dev_id, OFF) < 0)
        {
            DALSYS_Log_Err("could not de-route stream to Device %s\n",pRxDevName);
            ret=AUDIO_FTM_ERROR;
        }

      if (msm_en_device(pDevCtxt->rx_dev_id, OFF) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be disabled\n", pRxDevName);
            ret=AUDIO_FTM_ERROR;
      }

        close(pDevCtxt->rx_fd );
  }

  /* Tx */
  if((pDevCtxt->tx_dev_id != -ENODEV) && (pDevCtxt->bLoopbackCase != TRUE))
  {
     /* disconnect the HW device with decoding session */
    if (msm_route_stream(DIR_TX, pDevCtxt->tx_session_id, pDevCtxt->tx_dev_id, OFF) < 0)
        {
            DALSYS_Log_Err("could not de-route stream to Device %s\n",pTxDevName);
            ret=AUDIO_FTM_ERROR;
        }

      if (msm_en_device(pDevCtxt->tx_dev_id, OFF) < 0)
      {
        DALSYS_Log_Err("Device %s cannot be disabled\n", pTxDevName);
            ret=AUDIO_FTM_ERROR;
      }

        close(pDevCtxt->tx_fd );
  }

    pDevCtxt->m_state=AUDIO_FTM_HW_DRV_CLOSED;

  return ret;
}

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
    void * pBuf,                      /* Input: buffer pointer for reading  */
    uint32 nSamples,                  /* Input: Read buffer size */
    uint32 *pCount                    /* Output: return the actual read bytes */
)
{
   uint32 len, size;

     size=nSamples*(pDevCtxt->bitWidth/AUDIO_FTM_BIT_WIDTH_8);
     len=(uint32)read(pDevCtxt->tx_fd, pBuf, size);
   *pCount=len;

   if (len != size)
        {
            DALSYS_Log_Err("Read less bytes than required\n");
      return AUDIO_FTM_ERROR;
        }

    return AUDIO_FTM_SUCCESS;
}

/*==================================================================================================
  @brief FTM driver Write

  This API can be used to Write data to FTM HW driver

  @param pDevCtxt:    Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to data buffer which will be written to driver
         nBufSize:    Input -   Samples

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_hw_write
(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    void *pBuf,                       /* Input: buffer pointer containing data for writing */
    uint32 nSamples                   /* Input: Samples */
)
{
   uint32  len;

     len=nSamples*(pDevCtxt->bitWidth/AUDIO_FTM_BIT_WIDTH_8);
   if (write(pDevCtxt->rx_fd, pBuf, len) != len)
     {
                DALSYS_Log_Err("write return not equal to size\n");
                return AUDIO_FTM_ERROR;
   }

   return AUDIO_FTM_SUCCESS;
}

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
)
{
  AUDIO_FTM_STS_T   ret=AUDIO_FTM_SUCCESS;

    if((pDevCtxt->m_state == AUDIO_FTM_HW_DRV_OPENED) ||
        (pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STARTED) ||
        (pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STOPPED))
    {

            switch(dwIOCode)
          {
            case IOCTL_AUDIO_FTM_CHG_VOL:
              {
                        uint32 vol;
                        uint16 active_session_id;

                        if(pBufIn == NULL)
                        {
              DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_CHG_VOL failed\n");
                ret=AUDIO_FTM_ERROR;
                            break;
                        }

                        vol=*(uint32 *)pBufIn;

                /* Non Loopback case */
                        if(pDevCtxt->bLoopbackCase != TRUE)
                        {

                          if(pDevCtxt->rx_dev_id != -ENODEV)
                          {
                                if(msm_set_volume(pDevCtxt->rx_session_id,vol))
                                {
                                  DALSYS_Log_Err("Failed on Rx session volume setup\n");
                        ret=AUDIO_FTM_ERROR;
                                }
                          }

                        if(pDevCtxt->tx_dev_id != -ENODEV)
                          {
                                if(msm_set_volume(pDevCtxt->tx_session_id,vol))
                                {
                                  DALSYS_Log_Err("Failed on Tx session volume setup\n");
                        ret=AUDIO_FTM_ERROR;
                                }
                          }
                        }

                /* Loopback case */
                if(pDevCtxt->bLoopbackCase == TRUE)
                {
                  if(msm_set_device_volume(pDevCtxt->rx_dev_id,vol))
                  {
                                  DALSYS_Log_Err("Failed on Rx device volume setup\n");
                        ret=AUDIO_FTM_ERROR;
                  }

                            if(msm_set_device_volume(pDevCtxt->tx_dev_id,vol))
                            {
                                  DALSYS_Log_Err("Failed on Tx device volume setup\n");
                        ret=AUDIO_FTM_ERROR;
                            }
                }

                }
              break;

            case IOCTL_AUDIO_FTM_START:
              {
              AFEDevAudIfDirType dir;

                        if(pBufIn == NULL)
                        {
              DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_START failed\n");
                ret=AUDIO_FTM_ERROR;
                            break;
                        }

                        dir= *(AFEDevAudIfDirType*)pBufIn;

                if((pDevCtxt->bLoopbackCase != TRUE)    && (dir == AUDIO_IF_SINK))
                {
                  if(pDevCtxt->rx_dev_id != -ENODEV)
                  {
                    if (ioctl(pDevCtxt->rx_fd, AUDIO_START, 0) < 0)
                    {
                        DALSYS_Log_Err("Rx stream cannot be started\n");
                        ret=AUDIO_FTM_ERROR;
                    }
                }
                }

                if((pDevCtxt->bLoopbackCase != TRUE)    && (dir == AUDIO_IF_SOURCE))
                {
                if(pDevCtxt->tx_dev_id != -ENODEV)
                {
                    if (ioctl(pDevCtxt->tx_fd, AUDIO_START, 0) < 0)
                    {
                        DALSYS_Log_Err("Tx stream cannot be started\n");
                        ret=AUDIO_FTM_ERROR;
                    }
                }
                }

                if((pDevCtxt->bLoopbackCase == TRUE) && (pDevCtxt->tx_dev_id != -ENODEV) && (pDevCtxt->rx_dev_id != -ENODEV))
                {
                  if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
                  {
                        if(msm_snd_dev_loopback(pDevCtxt->rx_dev_id, pDevCtxt->tx_dev_id, ON))
                                {
                     DALSYS_Log_Err("Failed to enable loopback\n");
                                     ret=AUDIO_FTM_ERROR;
                                }
                  }
                }

                  if(ret == AUDIO_FTM_SUCCESS)
                            pDevCtxt->m_state=AUDIO_FTM_HW_DRV_STARTED;
              }
              break;

            case IOCTL_AUDIO_FTM_STOP:
              {
              AFEDevAudIfDirType dir;

                        if(pBufIn == NULL)
                        {
              DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_STOP failed\n");
                ret=AUDIO_FTM_ERROR;
                            break;
                        }

                        dir= *(AFEDevAudIfDirType*)pBufIn;

                if((pDevCtxt->bLoopbackCase != TRUE)    && (dir == AUDIO_IF_SINK))
                {
              if(pDevCtxt->rx_dev_id != -ENODEV)
              {
                                if (ioctl(pDevCtxt->rx_fd, AUDIO_STOP, 0) < 0)
                                {
                                    DALSYS_Log_Err("Rx stream cannot be stopped\n");
                                    ret=AUDIO_FTM_ERROR;
                                }
              }
                }

                if((pDevCtxt->bLoopbackCase != TRUE)    && (dir == AUDIO_IF_SOURCE))
                {
              if(pDevCtxt->tx_dev_id != -ENODEV)
              {
                                if (ioctl(pDevCtxt->tx_fd, AUDIO_STOP, 0) < 0)
                                {
                                    DALSYS_Log_Err("Tx stream cannot be stopped\n");
                                    ret=AUDIO_FTM_ERROR;
                                }
              }
                }

                if((pDevCtxt->bLoopbackCase == TRUE) && (pDevCtxt->tx_dev_id != -ENODEV) && (pDevCtxt->rx_dev_id != -ENODEV))
                {

                    if(msm_snd_dev_loopback(pDevCtxt->rx_dev_id, pDevCtxt->tx_dev_id, OFF))
                            {
                 DALSYS_Log_Err("Failed to disable loopback\n");
                                 ret=AUDIO_FTM_ERROR;
                            }
                }

                  if(ret == AUDIO_FTM_SUCCESS)
                            pDevCtxt->m_state=AUDIO_FTM_HW_DRV_STOPPED;
              }
              break;

                    case IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE:
                     {
                        if(pBufOut == NULL)
                        {
              DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE failed\n");
                ret=AUDIO_FTM_ERROR;
                            break;
                        }
                       *((uint32 *)pBufOut)=pDevCtxt->rx_buf_size;
                   }
                    break;

                    case IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE:
                     {
                        if(pBufOut == NULL)
                        {
              DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE failed\n");
                ret=AUDIO_FTM_ERROR;
                            break;
                        }
                       *((uint32 *)pBufOut)=pDevCtxt->tx_buf_size;
                   }
                    break;

              default:

                     DALSYS_Log_Err("this operation is not supportted\n");
           ret= AUDIO_FTM_ERROR;
              break;
          }
        }
      else
        {
            DALSYS_Log_Err("this operation cannot be done when the driver is not in active state\n");
      ret= AUDIO_FTM_ERROR;
        }

  return ret;
}

/*==================================================================================================
  @brief FTM Driver HW driver deinit

  @param pDevCtxt:    Input -   handle to device context

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T aud_ftm_hw_deinit
(
    Aud_FTM_DevCtxt_T  *pDevCtxt
)
{
    int alsa_ctl;

    if (g_bDriverInitialized == TRUE)
    {
        alsa_ctl = msm_mixer_close();
        if (alsa_ctl < 0)
            DALSYS_Log_Err("Fail to close ALSA MIXER\n");
    }


  g_bDriverInitialized=FALSE;

  pDevCtxt->m_state=AUDIO_FTM_HW_DRV_UN_INITIALIZED;

  return  AUDIO_FTM_SUCCESS;
}

/*================================================================================================*/


#ifdef __cplusplus
}
#endif
