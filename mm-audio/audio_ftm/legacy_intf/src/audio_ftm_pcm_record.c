#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_pcm_record.c
  @brief  AUDIO FTM PCM record driver
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_pcm_record.c#2 $
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

#include "audio_ftm_pcm_record.h"
#include "audio_ftm_util_fifo.h"
#include "audio_ftm_hw_drv.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/
#ifdef ANDROID
/* definitions for Android logging */
#include <utils/Log.h>
#include <cutils/properties.h>
#else /* ANDROID */
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#define ALOGI(...)      fprintf(stdout, __VA_ARGS__)
#define ALOGE(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGV(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGD(...)      fprintf(stderr, __VA_ARGS__)
#endif /* ANDROID */

#define MAX_AUD_FTM_CLIENT   1
#define AUD_FTM_FIFO_BUF_SIZE   2048

/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

struct Aud_FTM_ClientCtxt_S;

typedef struct Aud_FTM_DrvCtxt_S{
   Aud_FTM_DevCtxt_T            *pDevCtxt;
   struct Aud_FTM_ClientCtxt_S  *apClientCtxt[MAX_AUD_FTM_CLIENT];

     DALBOOL                      bOpened;
     DAL_ATOMIC                   dwOpenCount;

     pthread_t                    hRecord_thread;
     DALBOOL                      bStart;

   DALSYSSyncHandle             hClientSync;

     AUD_FTM_PCM_REC_PARAM_T      client_param;       /* parameters passed from client */
     uint32                       tx_blk_convey_samples;  /* sample count */
} Aud_FTM_DrvCtxt_T, *pAud_FTM_DrvCtxt_T;

/// Structure which describes the client context
typedef struct Aud_FTM_ClientCtxt_S{
  Aud_FTM_DrvCtxt_T     *pDrvCtxt;

    uint32 dwAccessCode;                        ///< access mode for the opened device
    uint32 dwShareMode;                        ///< share mode for the opened device
  uint32 dwClientID;             // client sequence number of LPA driver
} Aud_FTM_ClientCtxt_T, *pAud_FTM_ClientCtxt_T;


struct PCM_WAVE_HEADER {
    uint32 riff_id;
    uint32 riff_sz;
    uint32 riff_fmt;
    uint32 fmt_id;
    uint32 fmt_sz;
    uint16 audio_format;
    uint16 num_channels;
    uint32 sample_rate;
    uint32 byte_rate;          /* sample_rate * num_channels * bps / 8 */
    uint16 block_align;      /* num_channels * bps / 8 */
    uint16 bits_per_sample;
    uint32 data_id;
    uint32 data_sz;
};

/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1

/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/

static DALBOOL    g_bDriverInitialized=FALSE;   //  whether it has been inited or not
static uint16 *pAud_ftm_tx_buf=NULL;

/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static void aud_ftm_record_pcm( void * pCtxt )
{
  int32  size, total, rd_len;
    struct PCM_WAVE_HEADER  hdr;
    Aud_FTM_DevCtxt_T * pDevCtxt;
    int32  fd;
    AUDIO_FTM_STS_T ret;

    Aud_FTM_DrvCtxt_T * pDrvCtxt = (Aud_FTM_DrvCtxt_T *)pCtxt;
    pDevCtxt=pDrvCtxt->pDevCtxt;

#ifdef MSM8960_ALSA
    char temp[20];
    strlcpy(temp, "hw:0,0", sizeof(temp));
    if (pDevCtxt->capturedevice >= 0) {
        snprintf(temp, sizeof(temp), "hw:0,%d", pDevCtxt->capturedevice);
        printf("\n%s: device name %s",__func__, temp);
    }
    audio_ftm_hw_rec_wav(0, temp, 48000, 1, pDrvCtxt->client_param.pFileName, &pDrvCtxt->bStart);
#else
    hdr.riff_id = ID_RIFF;
    hdr.riff_sz = 0;
    hdr.riff_fmt = ID_WAVE;
    hdr.fmt_id = ID_FMT;
    hdr.fmt_sz = 16;
    hdr.audio_format = FORMAT_PCM;
    hdr.num_channels = pDevCtxt->numChannels;
    hdr.sample_rate = pDevCtxt->sampleRate;
    hdr.byte_rate = hdr.sample_rate * hdr.num_channels * sizeof(int16);
    hdr.block_align = hdr.num_channels * sizeof(int16);
    hdr.bits_per_sample = 16;
    hdr.data_id = ID_DATA;
    hdr.data_sz = 0;

    fd = open(pDrvCtxt->client_param.pFileName, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        DALSYS_Log_Err("cannot open/create PCM record file\n");
        goto ftm_record_err;
    }

    write(fd, &hdr, sizeof(hdr));
    total=0;
    size=pDrvCtxt->tx_blk_convey_samples * sizeof(int16);

  while(pDrvCtxt->bStart != TRUE )
            DALSYS_Delay(10);

  do
      {
      if( pDrvCtxt->bStart != TRUE ) break;

      ret=audio_ftm_hw_read(pDevCtxt,pAud_ftm_tx_buf,pDrvCtxt->tx_blk_convey_samples,&rd_len);
            if(ret != AUDIO_FTM_SUCCESS)
            {
              DALSYS_Log_Err("PCM Read has problem\n");
                break;
            }

         if (write(fd, pAud_ftm_tx_buf, size) != (ssize_t)size)
         {
            DALSYS_Log_Err("Failed to save PCM data to file\n");
            break;
         }

         total += size;

      }while(1);

    /* update lengths in header */
    hdr.data_sz = total;
    hdr.riff_sz = total + 8 + 16 + 8;
    lseek(fd, 0, SEEK_SET);
    write(fd, &hdr, sizeof(hdr));
    close(fd);

ftm_record_err:
#endif

    pthread_exit(0);
}


/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
  @brief FTM Driver Attach API

  This API can be used to attach the FTM driver.

  @param  pParam:  Input - pointer to parameter
          pDriverHdl   : Output - a driver handle will be returned if attach success

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
audio_ftm_pcm_rec_attach
(
    void *pParam,                                 /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *pDriverHdl        /* Output: driver handle */
)
{

  /************* Context creation ***********/

  Aud_FTM_DevCtxt_T *pDeviceCtxt;
  Aud_FTM_DrvCtxt_T *pDriverCtxt;

    Aud_FTM_HW_INIT_PARAM_T hw_param;
    AUDIO_FTM_STS_T sts;

    if (g_bDriverInitialized)
  {
        return  AUDIO_FTM_ERROR;    //  already inited,
  }

    /****************** create context object ******************/

  if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DevCtxt_T), (void **)&pDeviceCtxt))
    {
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (pDeviceCtxt == NULL)  return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
  DALSYS_memset(pDeviceCtxt,0,sizeof(Aud_FTM_DevCtxt_T));

  if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DrvCtxt_T), (void **)&pDriverCtxt))
    {
                DALSYS_Free(pDeviceCtxt);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (pDriverCtxt == NULL)
        {
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
        NULL))
    {
        return AUDIO_FTM_ERROR;
    }

    /*************  hardware initialization ***********/

   DALSYS_memcpy((uint8*)&(pDriverCtxt->client_param),
                  (uint8*)pParam, sizeof(AUD_FTM_PCM_REC_PARAM_T));

   hw_param.inpath=pDriverCtxt->client_param.path.inpath;
   hw_param.outpath=AUDIO_FTM_OUT_INVALID;

   hw_param.rate=(hw_param.inpath == AUDIO_FTM_IN_FM)? AUDIO_FTM_PCM_RATE_48K : AUDIO_FTM_PCM_RATE_8K;
   hw_param.width=AUDIO_FTM_BIT_WIDTH_16;
   hw_param.channel=pDriverCtxt->client_param.channel;
   hw_param.gain=pDriverCtxt->client_param.gain;
     hw_param.bLoopbackCase=FALSE;

   sts=aud_ftm_hw_init(pDeviceCtxt, &hw_param);
   if(sts != AUDIO_FTM_SUCCESS)
        return AUDIO_FTM_ERROR;

    /***** end of Init *****/

        pDriverCtxt->bOpened=FALSE;
      g_bDriverInitialized=TRUE;
    *pDriverHdl=(AUDIO_FTM_DRIVER_HANDLE_T)pDriverCtxt;

      return AUDIO_FTM_SUCCESS;
}

/*==================================================================================================
  @brief FTM driver open

  This API can be used to open the FTM driver.

  @param DriverHdl:   Input -   handle created in Audio_FTM_Attach
         AccessMode:  Input -   Read/Write/RD_WR
         ShareMode:   Input -   Exclusive or Share
         pClientHdl:  Output -  point to the returned Open handle

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_open
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

    if (pDrvCxt == NULL)
    {
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    DALSYS_SyncEnter(pDrvCxt->hClientSync);

    if((ShareMode == AUD_FTM_SHAREMODE_EXCLU ) && (DALSYS_atomic_read(&pDrvCxt->dwOpenCount) != 0))
        /* ShareMode = 0: exclusive mode; =FILE_SHARE_READ: read only; =FILE_SHARE_WRITE: write only */
        /* LPA driver only works at Exclusive Mode */
    {
      DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERROR;
    }

  if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_ClientCtxt_T), (void **)&pOpenContext))
    {
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (pOpenContext == NULL)
    {
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
  pDrvCxt->pDevCtxt->read_write_flag = PCM_IN;
#endif

  /***** open HW ****/
  res=audio_ftm_hw_open(pDrvCxt->pDevCtxt);
  if(res != AUDIO_FTM_SUCCESS)
  {
    DALSYS_SyncLeave(pDrvCxt->hClientSync);
    return AUDIO_FTM_ERROR;
  }

    audio_ftm_hw_iocontrol(pDrvCxt->pDevCtxt, IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE, NULL, 0, &dev_buf_size, sizeof(dev_buf_size), NULL);
    pDrvCxt->tx_blk_convey_samples=dev_buf_size/sizeof(int16);

  if(DAL_SUCCESS != DALSYS_Malloc(dev_buf_size, (void **)&pAud_ftm_tx_buf))  /* buffer for reading data from low layer */
  {
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
      DALSYS_Log_Err(" pAud_ftm_tx_buf allocation fail\n");
      return DAL_ERROR;
    }
  DALSYS_memset(pAud_ftm_tx_buf,0,dev_buf_size);

    // Increase opened device counter
  AddRef(&pDrvCxt->dwOpenCount);
    pDrvCxt->bOpened=TRUE;

    DALSYS_SyncLeave(pDrvCxt->hClientSync);

    *pClientContext=  (AUDIO_FTM_CLIENT_HANDLE_T)pOpenContext;
      return AUDIO_FTM_SUCCESS;
}

/*==================================================================================================
  @brief FTM driver Close

  This API can be used to Close the FTM driver.

  @param OpenHdl:   Input -   handle created in Audio_FTM_Open

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_close
(
    AUDIO_FTM_CLIENT_HANDLE_T  pOpenContext       /* Input: client handle */
)
{
    Aud_FTM_ClientCtxt_T   *pExplicitOpenContext;
    Aud_FTM_DrvCtxt_T      *pDrvCtx;
    uint32                  nClient_id;
    uint32                  i;
    AUDIO_FTM_STS_T res;

    if (pOpenContext == NULL)
    {
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    pExplicitOpenContext = (Aud_FTM_ClientCtxt_T *)pOpenContext;

    pDrvCtx = pExplicitOpenContext->pDrvCtxt;

  res=audio_ftm_hw_close(pDrvCtx->pDevCtxt);
  if(res != AUDIO_FTM_SUCCESS) return AUDIO_FTM_ERROR;

    nClient_id=pExplicitOpenContext->dwClientID;

  DALSYS_Free(pExplicitOpenContext); pExplicitOpenContext=NULL;

  DALSYS_SyncEnter(pDrvCtx->hClientSync);

  for(i=nClient_id; i<DALSYS_atomic_read(&pDrvCtx->dwOpenCount)-1; i++)
  {
    pDrvCtx->apClientCtxt[i]=pDrvCtx->apClientCtxt[i+1];
      pDrvCtx->apClientCtxt[i]->dwClientID--;
  }
  pDrvCtx->apClientCtxt[i]=NULL;

    // Decrease opened device counter
  Release(&pDrvCtx->dwOpenCount);

    if(DALSYS_atomic_read(&pDrvCtx->dwOpenCount) == 0)
    {
      pDrvCtx->bOpened=FALSE;
    DALSYS_Free(pAud_ftm_tx_buf); pAud_ftm_tx_buf=NULL;
    }

  DALSYS_SyncLeave(pDrvCtx->hClientSync);

    return AUDIO_FTM_SUCCESS;

}

/*==================================================================================================
  @brief FTM driver Read

  This API can be used to Read data from FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to buffer which stores data read from driver
         nBufSize     Input -   Read buffer size
         pCount:      Output -  return the actual read bytes

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_read
(
    AUDIO_FTM_CLIENT_HANDLE_T pDriverContext,   /* Input: client handle */
    void *pBuf,                                 /* Input: buffer pointer for reading  */
    uint32 nBufSize,                            /* Input: Read buffer size */
    uint32 *pCount                              /* Output: return the actual read bytes */
)
{
  return AUDIO_FTM_SUCCESS;
}

/*==================================================================================================
  @brief FTM driver Write

  This API can be used to Write data to FTM driver

  @param OpenHdl:     Input -   handle created in Audio_FTM_Open
         pBuf         Input -   point to data buffer which will be written to driver
         nCount:      Input -   Bytes count

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_write
(
    AUDIO_FTM_CLIENT_HANDLE_T pDriverContext,   /* Input: client handle */
    void *pBuf,                                 /* Input: buffer pointer containing data for writing */
    uint32 nBufSize                            /* Input: Write buffer size */
)
{
  return AUDIO_FTM_SUCCESS;

}

/*==================================================================================================
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
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_iocontrol
(
    AUDIO_FTM_CLIENT_HANDLE_T pOpenContext,   /* Input: client handle */
    uint32 dwCode,                            /* Input: IOControl command */
    uint8 * pBufIn,                           /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,                           /* Input: parameter length */
    uint8 * pBufOut,                          /* Output: buffer pointer for outputs */
    uint32 dwLenOut,                          /* Input: expected output length */
    uint32 *pActualOutLen                     /* Output: actual output length */
)
{
    Aud_FTM_DrvCtxt_T * pDrvCtxt;
    Aud_FTM_DevCtxt_T * pDevCtxt;
    AUDIO_FTM_STS_T ret;

    // The device has to be open
    if (pOpenContext == NULL)
        return AUDIO_FTM_ERR_INVALID_PARAM;

    pDrvCtxt = ((Aud_FTM_ClientCtxt_T *)pOpenContext)->pDrvCtxt;
    pDevCtxt=pDrvCtxt->pDevCtxt;

    ret=AUDIO_FTM_SUCCESS;

    switch(dwCode)
    {

    case IOCTL_AUDIO_FTM_CHG_VOL:
    {
      uint16 vol;
        if (pBufIn == NULL)
        {
           DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_CHG_VOL failed\n");
       ret=AUDIO_FTM_ERR_INVALID_PARAM;
             break;
        }
        vol=*(uint16 *)pBufIn;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_CHG_VOL, (uint8 *)&vol, sizeof(vol), NULL, 0, NULL);
  }
    break;

    case IOCTL_AUDIO_FTM_START:
    {
      AFEDevAudIfDirType dir;
        int res;

        dir=AUDIO_IF_SOURCE;
        pDrvCtxt->pDevCtxt->capturedevice =
        pDrvCtxt->pDevCtxt->pDrvCtxt->client_param.device;
        /* create record thread */
    res=pthread_create(&(pDrvCtxt->hRecord_thread), NULL, aud_ftm_record_pcm,(void *)pDrvCtxt);
        if(res !=0 )
        {
          DALSYS_Log_Err("Failed to create PCM Record thread\n");
            return AUDIO_FTM_ERROR;
        }

        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_START, (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);
    pDrvCtxt->bStart=TRUE;
    sleep(1);

  }
    break;

    case IOCTL_AUDIO_FTM_STOP:
    {
        int res;
      AFEDevAudIfDirType dir;
        dir=AUDIO_IF_SOURCE;

      pDrvCtxt->bStart=FALSE;
        DALSYS_Delay(100);   /* delay 100ms to consume all remained data */
    res=pthread_join(pDrvCtxt->hRecord_thread, NULL);
        if(res !=0)
        {
          DALSYS_Log_Err("Failed to join PCM record thread\n");
            return AUDIO_FTM_ERROR;
        }
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_STOP, (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);
  }
    break;

    default:
        ret=AUDIO_FTM_ERR_INVALID_PARAM;
    }

    return ret;
}

/*==================================================================================================
  @brief FTM driver Detach

  This API can be used to Detach FTM driver, it will release all resournces

  @param DriverHdl:     Input -   handle created in Audio_FTM_Attach

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/
AUDIO_FTM_STS_T
audio_ftm_pcm_rec_detach
(
    AUDIO_FTM_DRIVER_HANDLE_T pDriverContext
)
{

  Aud_FTM_DevCtxt_T* pDevContext;
    Aud_FTM_DrvCtxt_T* pDrvContext;

    pDrvContext = (Aud_FTM_DrvCtxt_T *)pDriverContext;
    if(pDrvContext == NULL)
    {
        return AUDIO_FTM_ERROR;
    }
  pDevContext= pDrvContext->pDevCtxt;
    if(pDevContext == NULL)
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

/*================================================================================================*/


#ifdef __cplusplus
}
#endif
