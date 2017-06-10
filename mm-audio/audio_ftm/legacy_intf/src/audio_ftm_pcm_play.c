/*==========================================================================

                     FTM Audio PCM Play support

Description
  File adds support for Wave file playback support

===========================================================================*/

/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
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
#ifdef __cplusplus
extern "C" {
#endif


/*==============================================================================
                                        INCLUDE FILES
==============================================================================*/

#include "audio_ftm_pcm_play.h"
#include "audio_ftm_util_fifo.h"
#include "audio_ftm_hw_drv.h"

/*==============================================================================
                                     LOCAL CONSTANTS
==============================================================================*/

#define MAX_AUD_FTM_CLIENT   1
#define AUD_FTM_FIFO_BUF_SIZE   2048

/*==============================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==============================================================================*/

struct Aud_FTM_ClientCtxt_S;

typedef struct Aud_FTM_DrvCtxt_S{
   Aud_FTM_DevCtxt_T            *pDevCtxt;
   struct Aud_FTM_ClientCtxt_S  *apClientCtxt[MAX_AUD_FTM_CLIENT];
   DALBOOL                      bOpened;
   DAL_ATOMIC                   dwOpenCount;
   pthread_t                    hPlay_thread;
   DALBOOL                      bStart;
   DALSYSSyncHandle             hClientSync;
   AUD_FTM_PCM_PLAY_PARAM_T      client_param;
   uint32                       rx_buffer_sz;
} Aud_FTM_DrvCtxt_T, *pAud_FTM_DrvCtxt_T;

typedef struct Aud_FTM_ClientCtxt_S{
   Aud_FTM_DrvCtxt_T     *pDrvCtxt;
   uint32 dwAccessCode;
   uint32 dwShareMode;
   uint32 dwClientID;
} Aud_FTM_ClientCtxt_T, *pAud_FTM_ClientCtxt_T;

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

/*==============================================================================
                                        LOCAL MACROS
==============================================================================*/

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1

/*=============================================================================
                                      LOCAL VARIABLES
==============================================================================*/

static DALBOOL    g_bDriverInitialized=FALSE;
static uint16 *pAud_ftm_rx_buf=NULL;

/*==============================================================================
                                     GLOBAL VARIABLES
==============================================================================*/


/*==============================================================================
                                 LOCAL FUNCTION PROTOTYPES
==============================================================================*/


/*==============================================================================
                                     LOCAL FUNCTIONS
==============================================================================*/
static void aud_ftm_play_pcm( void * pCtxt )
{
    int32  size, total, rd_len;
    Aud_FTM_DevCtxt_T * pDevCtxt;
    int fd;
    AUDIO_FTM_STS_T ret;
    int num_of_samples = 0, temp = 0;

    if (!pCtxt) {
        DALSYS_Log_Err("Invalid params to thread\n");
        goto ftm_play_err;
    }

    Aud_FTM_DrvCtxt_T *pDrvCtxt = (Aud_FTM_DrvCtxt_T *)pCtxt;
    pDevCtxt=pDrvCtxt->pDevCtxt;

    if (!pDevCtxt || !pAud_ftm_rx_buf){
        DALSYS_Log_Err("\nInvalid context %p Buffer pointer %p",
        pDevCtxt, pAud_ftm_rx_buf);
        goto ftm_play_err;
    }
    fd = open(pDrvCtxt->client_param.pFileName,O_RDWR, 0666);
    if (fd < 0) {
       DALSYS_Log_Err("cannot open PCM play file\n");
       goto ftm_play_err;
    }

    while(pDrvCtxt->bStart != TRUE ) DALSYS_Delay(10);
    DALSYS_Log_Info("\n Start Playback");
    do {
           if( pDrvCtxt->bStart != TRUE ) break;
           num_of_samples = read(fd, pAud_ftm_rx_buf, pDrvCtxt->rx_buffer_sz);
           if (!num_of_samples) {
              DALSYS_Log_Err("EOF file reached\n");
              break;
           }
           temp = (pDevCtxt->bitWidth/8) * (pDevCtxt->numChannels);
           if (!temp) {
               DALSYS_Log_Err("Error invalid bitWidth %d numChannels %d\n",
               (int)pDevCtxt->bitWidth, (int)pDevCtxt->numChannels);
               goto ftm_play_err;
           }
           num_of_samples = num_of_samples/temp;
           //DALSYS_Log_Info("\n %s Push samples %d", __func__, num_of_samples);
           /*Read data from the file and push for write*/
           ret = audio_ftm_hw_write(pDevCtxt, pAud_ftm_rx_buf, num_of_samples);
           if(ret != AUDIO_FTM_SUCCESS) {
              DALSYS_Log_Err("PCM write has problem\n");
              break;
           }
    } while(1);
    DALSYS_Log_Info("\n Done with file");

ftm_play_err:
    if (fd > 0) {
        DALSYS_Log_Info("\n Close the file");
        close(fd);
    }
    pthread_exit(0);
}

static AUDIO_FTM_STS_T
audio_ftm_pcm_parse_header(char *filename, struct wav_header *phdr)
{
    int fd, format;
    struct wav_header hdr;
    AUDIO_FTM_STS_T status = AUDIO_FTM_ERROR;
    if (!filename || !phdr) {
        DALSYS_Log_Err("%s: Invalid param\n", __func__);
        return status;
    }
    fd = open(filename,O_RDWR, 0666);
    if (fd < 0) {
        DALSYS_Log_Err("%s: File not found err %s\n",
        __func__, strerror(errno));
        return status;
    }
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        DALSYS_Log_Err("%s: Read failed\n", __func__);
        goto error;
    }

    if (hdr.bits_per_sample == 16)
        format = SNDRV_PCM_FORMAT_S16_LE;
    else if(hdr.bits_per_sample == 24) {
        format = SNDRV_PCM_FORMAT_S24_LE;
        DALSYS_Log_Err("%s: 24 bit not supported\n", __func__);
        goto error;
    }
    if (hdr.bits_per_sample > 16) {
        if (hdr.data_id != 0x61746164) {
            DALSYS_Log_Err("scan the rest of file for DATA ID \n");
            int temp = 0;
            /* parse the file until 'data' chunk is found */
            while (temp != 0x61746164) {
                if(read(fd, &temp, 4) != 4) {
                    DALSYS_Log_Err("DATA ID not found in header0x%x\n", temp);
                    goto error;
                }
                DALSYS_Log_Err("temp 0x%x\n", temp);
            }
            hdr.data_id = 0x61746164;
            read(fd, &hdr.data_sz, 4);
        }
        DALSYS_Log_Err("data_id 0x%x data_sz = 0x%x\n",
         hdr.data_id,hdr.data_sz);
    }

    if ((hdr.riff_id != ID_RIFF) ||
        (hdr.riff_fmt != ID_WAVE) ||
        (hdr.fmt_id != ID_FMT)) {
        DALSYS_Log_Err("'%s' is not a riff/wave file\n", filename);
        goto error;
    }

    DALSYS_Log_Err("hdr.fmt_sz %d hdr.bits_per_sample = %d"
                    "hdr.audio_format = %d\n",
                    hdr.fmt_sz, hdr.bits_per_sample, hdr.audio_format);

    if ((hdr.audio_format != FORMAT_PCM) ||
        (hdr.fmt_sz != 16 && hdr.fmt_sz != 40)) {
        DALSYS_Log_Err("'%s' is not pcm format\n", filename);
        goto error;
    }
    if ((hdr.bits_per_sample != 16 )) {
        DALSYS_Log_Err("'%s' is not 16 bits per sample\n", filename);
        goto error;
    }
    memcpy(phdr,&hdr,sizeof(hdr));
    status = AUDIO_FTM_SUCCESS;
error:
    if (fd > 0)
      close(fd);
    return status;
}


/*==============================================================================
                                       GLOBAL FUNCTIONS
==============================================================================*/

/*==============================================================================
  @brief FTM Driver Attach API

  This API can be used to attach the FTM driver.

  @param  pParam:  Input - pointer to parameter
          pDriverHdl   : Output - a driver handle will
                         be returned if attach success

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
                       error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_attach(
    void *pParam,                                 /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *pDriverHdl        /* Output: driver handle */
)
{

  /************* Context creation ***********/

    Aud_FTM_DevCtxt_T *pDeviceCtxt;
    Aud_FTM_DrvCtxt_T *pDriverCtxt;
    Aud_FTM_HW_INIT_PARAM_T hw_param;
    AUDIO_FTM_STS_T sts = AUDIO_FTM_ERROR;
    if (g_bDriverInitialized) {
        DALSYS_Log_Err("%s g_bDriverInitialized %d", __func__, __LINE__);
        return  AUDIO_FTM_ERROR;    //  already inited,
  }

  /****************** create context object ******************/

    if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DevCtxt_T),
      (void **)&pDeviceCtxt)) {
        DALSYS_Log_Err("%s DALSYS_Malloc failed %d", __func__, __LINE__);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (!pDeviceCtxt)  return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    DALSYS_memset(pDeviceCtxt,0,sizeof(Aud_FTM_DevCtxt_T));

    if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DrvCtxt_T),
     (void **)&pDriverCtxt)) {
      DALSYS_Log_Err("%s DALSYS_Malloc failed %d", __func__, __LINE__);
      DALSYS_Free(pDeviceCtxt);
      return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (!pDriverCtxt) {
       DALSYS_Free(pDeviceCtxt);
       return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }
    DALSYS_memset(pDriverCtxt,0,sizeof(Aud_FTM_DrvCtxt_T));
    DALSYS_atomic_init(&pDriverCtxt->dwOpenCount);
    pDriverCtxt->pDevCtxt=pDeviceCtxt;
    pDeviceCtxt->pDrvCtxt=pDriverCtxt;

  /************* Create Critical Section ***********/

    if(DAL_SUCCESS != DALSYS_SyncCreate(DALSYS_SYNC_ATTR_RESOURCE,
        &(pDriverCtxt->hClientSync),
        NULL)) {
        DALSYS_Log_Err("%s DALSYS_SyncCreate failed %d", __func__, __LINE__);
        goto error;
    }

    /*************  hardware initialization ***********/

   DALSYS_memcpy((uint8*)&(pDriverCtxt->client_param),
    (uint8*)pParam, sizeof(AUD_FTM_PCM_PLAY_PARAM_T));

   hw_param.inpath=AUDIO_FTM_IN_INVALID;
   hw_param.outpath=pDriverCtxt->client_param.path.outpath;

   struct wav_header hdr;
   if (audio_ftm_pcm_parse_header(pDriverCtxt->client_param.pFileName, &hdr)
       != AUDIO_FTM_SUCCESS) {
       DALSYS_Log_Err("%s parse failed %d", __func__, __LINE__);
       goto error;
   } else {
       DALSYS_Log_Info("\n %s width %d channels %d sample rate %d",
       __func__, hdr.bits_per_sample, hdr.num_channels, hdr.sample_rate);
   }
   hw_param.rate= hdr.sample_rate;
   hw_param.width= hdr.bits_per_sample;
   hw_param.channel= hdr.num_channels;
   hw_param.gain= pDriverCtxt->client_param.gain;
   hw_param.bLoopbackCase = FALSE;
   sts=aud_ftm_hw_init(pDeviceCtxt, &hw_param);
   if(sts != AUDIO_FTM_SUCCESS) {
     DALSYS_Log_Err("%s aud_ftm_hw_init failed %d", __func__, __LINE__);
     goto error;
   }

    /***** end of Init *****/

    pDriverCtxt->bOpened=FALSE;
    g_bDriverInitialized=TRUE;
    *pDriverHdl=(AUDIO_FTM_DRIVER_HANDLE_T)pDriverCtxt;
    return AUDIO_FTM_SUCCESS;
error:
    DALSYS_Free(pDeviceCtxt);
    DALSYS_Free(pDriverCtxt);
    return sts;
}

/*==============================================================================
  @brief FTM driver open

  This API can be used to open the FTM driver.

  @param DriverHdl:   Input -   handle created in Audio_FTM_Attach
         AccessMode:  Input -   Read/Write/RD_WR
         ShareMode:   Input -   Exclusive or Share
         pClientHdl:  Output -  point to the returned Open handle

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
                       error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_open
(
    AUDIO_FTM_DRIVER_HANDLE_T pDriverContext ,  /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T AccessMode,           /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T  ShareMode,            /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T * pClientContext  /* Output: client handle */
)
{
    Aud_FTM_DrvCtxt_T * pDrvCxt = (Aud_FTM_DrvCtxt_T *)pDriverContext;
    Aud_FTM_ClientCtxt_T * pOpenContext = NULL;
    AUDIO_FTM_STS_T res;
    uint32 dev_buf_size;

    if (!pDrvCxt) {
        DALSYS_Log_Err("%s Invalid params %d", __func__, __LINE__);
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    DALSYS_SyncEnter(pDrvCxt->hClientSync);
    /* ShareMode = 0: exclusive mode;
       FILE_SHARE_READ: read only; =FILE_SHARE_WRITE: write only */
    /* LPA driver only works at Exclusive Mode */

    if((ShareMode == AUD_FTM_SHAREMODE_EXCLU ) &&
       (DALSYS_atomic_read(&pDrvCxt->dwOpenCount) != 0)) {
      DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERROR;
    }

    if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_ClientCtxt_T),
    (void **)&pOpenContext)) {
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (!pOpenContext) {
      DALSYS_SyncLeave(pDrvCxt->hClientSync);
      return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }
    DALSYS_memset(pOpenContext,0,sizeof(Aud_FTM_ClientCtxt_T));

    // Store device settings for future use
    pOpenContext->pDrvCtxt = pDrvCxt;
    pOpenContext->dwAccessCode = AccessMode;
    pOpenContext->dwShareMode = ShareMode;

    pOpenContext->dwClientID=DALSYS_atomic_read(&pDrvCxt->dwOpenCount);
    pDrvCxt->apClientCtxt[pOpenContext->dwClientID]=pOpenContext;

#ifdef MSM8960_ALSA
    pDrvCxt->pDevCtxt->read_write_flag = PCM_OUT;
#endif
	pDrvCxt->pDevCtxt->playbackdevice =
	pDrvCxt->pDevCtxt->pDrvCtxt->client_param.device;
    /***** open HW ****/
    res=audio_ftm_hw_open(pDrvCxt->pDevCtxt);
    if(res != AUDIO_FTM_SUCCESS) {
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERROR;
    }

    audio_ftm_hw_iocontrol(pDrvCxt->pDevCtxt, IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE,
    NULL, 0, &dev_buf_size, sizeof(dev_buf_size), NULL);
    pDrvCxt->rx_buffer_sz=dev_buf_size;

    if(DAL_SUCCESS != DALSYS_Malloc(dev_buf_size,
    (void **)&pAud_ftm_rx_buf)) {
         DALSYS_SyncLeave(pDrvCxt->hClientSync);
         DALSYS_Log_Err(" pAud_ftm_rx_buf allocation fail\n");
         return DAL_ERROR;
    }
    DALSYS_memset(pAud_ftm_rx_buf,0,dev_buf_size);

    // Increase opened device counter
    AddRef(&pDrvCxt->dwOpenCount);
    pDrvCxt->bOpened=TRUE;
    *pClientContext=  (AUDIO_FTM_CLIENT_HANDLE_T)pOpenContext;
    DALSYS_SyncLeave(pDrvCxt->hClientSync);
    return AUDIO_FTM_SUCCESS;
}

/*==============================================================================
  @brief FTM driver Close

  This API can be used to Close the FTM driver.

  @param OpenHdl:   Input -   handle created in Audio_FTM_Open

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
                        error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_close
(
    AUDIO_FTM_CLIENT_HANDLE_T  pOpenContext       /* Input: client handle */
)
{
    Aud_FTM_ClientCtxt_T   *pExplicitOpenContext;
    Aud_FTM_DrvCtxt_T      *pDrvCtx;
    uint32                  nClient_id;
    uint32                  i;
    AUDIO_FTM_STS_T res;

    if (!pOpenContext) {
        DALSYS_Log_Err("%s Invalid params %d", __func__, __LINE__);
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    pExplicitOpenContext = (Aud_FTM_ClientCtxt_T *)pOpenContext;
    pDrvCtx = pExplicitOpenContext->pDrvCtxt;
    if (!pDrvCtx) {
        DALSYS_Log_Err("%s Invalid params %d", __func__, __LINE__);
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    res=audio_ftm_hw_close(pDrvCtx->pDevCtxt);
    if(res != AUDIO_FTM_SUCCESS) return AUDIO_FTM_ERROR;
    nClient_id=pExplicitOpenContext->dwClientID;
    DALSYS_Free(pExplicitOpenContext); pExplicitOpenContext=NULL;
    DALSYS_SyncEnter(pDrvCtx->hClientSync);

    for(i=nClient_id; i<DALSYS_atomic_read(&pDrvCtx->dwOpenCount)-1; i++) {
        pDrvCtx->apClientCtxt[i]=pDrvCtx->apClientCtxt[i+1];
        pDrvCtx->apClientCtxt[i]->dwClientID--;
    }
    pDrvCtx->apClientCtxt[i]=NULL;

    // Decrease opened device counter
    Release(&pDrvCtx->dwOpenCount);

    if(!DALSYS_atomic_read(&pDrvCtx->dwOpenCount)) {
       pDrvCtx->bOpened=FALSE;
       DALSYS_Free(pAud_ftm_rx_buf); pAud_ftm_rx_buf=NULL;
    }
    DALSYS_SyncLeave(pDrvCtx->hClientSync);
    return AUDIO_FTM_SUCCESS;
}

/*==============================================================================
  @brief FTM driver Read

  This API can be used to Read data from FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to buffer which stores data
                               read from driver
         nBufSize     Input -   Read buffer size
         pCount:      Output -  return the actual read bytes

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
                      error code otherwise
=============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_read
(
    AUDIO_FTM_CLIENT_HANDLE_T pDriverContext,/* Input: client handle */
    void *pBuf,             /* Input: buffer pointer for reading  */
    uint32 nBufSize,        /* Input: Read buffer size */
    uint32 *pCount          /* Output: return the actual read bytes */
)
{
  return AUDIO_FTM_SUCCESS;
}

/*==============================================================================
  @brief FTM driver Write

  This API can be used to Write data to FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to data buffer which will be written
                               to driver
         nCount:      Input -   Bytes count

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
                       error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_write
(
    AUDIO_FTM_CLIENT_HANDLE_T pDriverContext,   /* Input: client handle */
    void *pBuf,    /* Input: buffer pointer containing data for writing */
    uint32 nBufSize /* Input: Write buffer size */
)
{
  return AUDIO_FTM_SUCCESS;

}

/*=============================================================================
  @brief FTM driver IOControl

  This API can be used to set/read parameters or commands to/from FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         dwIOCode     Input -   IOControl command
         pBufIn       Input -   buffer pointer for input parameters
         dwLenIn      Input -   Input parameter length
         pBufOut      Input -   buffer pointer for outputs
         dwLenOut     Input -   expected output length
         pActualOutLen Output - actual output length

   @return return code, AUDIO_FTM_SUCCESS on successful completion,
                       error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_iocontrol
(
    AUDIO_FTM_CLIENT_HANDLE_T pOpenContext,   /* Input: client handle */
    uint32 dwCode,                            /* Input: IOControl command */
    uint8 * pBufIn,    /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,    /* Input: parameter length */
    uint8 * pBufOut,   /* Output: buffer pointer for outputs */
    uint32 dwLenOut,   /* Input: expected output length */
    uint32 *pActualOutLen  /* Output: actual output length */
)
{
    Aud_FTM_DrvCtxt_T * pDrvCtxt;
    Aud_FTM_DevCtxt_T * pDevCtxt;
    AUDIO_FTM_STS_T ret;

    // The device has to be open
    if (!pOpenContext)
        return AUDIO_FTM_ERR_INVALID_PARAM;

    pDrvCtxt = ((Aud_FTM_ClientCtxt_T *)pOpenContext)->pDrvCtxt;
    pDevCtxt=pDrvCtxt->pDevCtxt;

    ret=AUDIO_FTM_SUCCESS;

    switch(dwCode) {
    case IOCTL_AUDIO_FTM_CHG_VOL:
    {
      uint16 vol;
      if (!pBufIn) {
           DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_CHG_VOL failed\n");
           ret=AUDIO_FTM_ERR_INVALID_PARAM;
           break;
        }
        vol=*(uint16 *)pBufIn;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_CHG_VOL,
        (uint8 *)&vol, sizeof(vol), NULL, 0, NULL);
    }
    break;
    case IOCTL_AUDIO_FTM_START:
    {
      AFEDevAudIfDirType dir;
        int res;
        dir=AUDIO_IF_SINK;
        /* create Playback thread */
        res=pthread_create(&(pDrvCtxt->hPlay_thread), NULL,
            aud_ftm_play_pcm,(void *)pDrvCtxt);
        if(res !=0 ) {
          DALSYS_Log_Err("Failed to create PCM Record thread\n");
            return AUDIO_FTM_ERROR;
        }
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_START,
            (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);
        pDrvCtxt->bStart=TRUE;
    }
    break;
    case IOCTL_AUDIO_FTM_STOP:
    {
      int res;
      AFEDevAudIfDirType dir;
      dir=AUDIO_IF_SINK;
      pDrvCtxt->bStart=FALSE;
       DALSYS_Delay(100);   /* delay 100ms to consume all remained data */
       res=pthread_join(pDrvCtxt->hPlay_thread, NULL);
        if(res !=0) {
            DALSYS_Log_Err("Failed to join PCM record thread\n");
            return AUDIO_FTM_ERROR;
        }
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_STOP,
           (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);
    }
    break;

    default:
        ret=AUDIO_FTM_ERR_INVALID_PARAM;
    }
    return ret;
}

/*==============================================================================
  @brief FTM driver Detach

  This API can be used to Detach FTM driver, it will release all resournces

  @param DriverHdl:     Input -   handle created in Audio_FTM_Attach

  @return return code, AUDIO_FTM_SUCCESS on successful completion,
  error code otherwise
==============================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_play_detach
(
    AUDIO_FTM_DRIVER_HANDLE_T pDriverContext
)
{

  Aud_FTM_DevCtxt_T* pDevContext;
    Aud_FTM_DrvCtxt_T* pDrvContext;

    pDrvContext = (Aud_FTM_DrvCtxt_T *)pDriverContext;
    if(!pDrvContext)
    {
        return AUDIO_FTM_ERROR;
    }
    pDevContext= pDrvContext->pDevCtxt;
    if(!pDevContext)
    {
        return AUDIO_FTM_ERROR;
    }
    aud_ftm_hw_deinit(pDevContext);
    DALSYS_DestroySyncObject(pDrvContext->hClientSync);
    DALSYS_Free(pDevContext); pDevContext=NULL;
    DALSYS_Free(pDrvContext); pDrvContext=NULL;
    g_bDriverInitialized=FALSE;
    return AUDIO_FTM_SUCCESS;
}

/*============================================================================*/
#ifdef __cplusplus
}
#endif
