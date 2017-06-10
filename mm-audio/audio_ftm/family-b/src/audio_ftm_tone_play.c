#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_tone_play.c
  @brief  AUDIO FTM Tone play driver
====================================================================================================
Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_tone_play.c#2 $
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

#include "audio_ftm_tone_play.h"
#include "audio_ftm_util_fifo.h"
#include "audio_ftm_hw_drv.h"
#include "audio_ftm_dtmf_gen.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/

#define MAX_AUD_FTM_CLIENT        1
//#define AUD_FTM_CLIENT_BUF_SIZE   4096
//#define AUD_FTM_FIFO_BUF_SIZE     4096*2
#define AUD_FTM_WORKLOOP_PRIORITY    99
#define AUD_FTM_TONE_PLAY_TOTAL_EVENTS  2
#define  AUD_FTM_TONE_PLAY_FIFO_FACTOR   4    /* how may times of dev_buf for FIFO circular buf */

/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

struct Aud_FTM_ClientCtxt_S;

typedef struct Aud_FTM_DrvCtxt_S{
   Aud_FTM_DevCtxt_T            *pDevCtxt;
   struct Aud_FTM_ClientCtxt_S  *apClientCtxt[MAX_AUD_FTM_CLIENT];

     DALBOOL                      bOpened;
     DAL_ATOMIC                   dwOpenCount;
     DALBOOL                      bStart;

   DALSYSSyncHandle             hClientSync;
   DALSYSEventHandle            hReq_Data_Event;    /*notify that more data is demanded*/

   AUD_FTM_CIRCULAR_BUF_T       fifo_data_buf;             /* circular buffer for source data */

     AUD_FTM_TONE_PLAY_PARAM_T    client_param;       /* parameters passed from client */
     uint32                       nRx_blk_convey_samples;  /* how many samples transferred */

   DTMFGenStruct                dtmfGen;
   uint16                       nDTMF_Gain;
     pthread_t                    producer_thread;
     pthread_t                    consumer_thread;

} Aud_FTM_DrvCtxt_T, *pAud_FTM_DrvCtxt_T;

/// Structure which describes the client context
typedef struct Aud_FTM_ClientCtxt_S{
  Aud_FTM_DrvCtxt_T     *pDrvCtxt;

    uint32 dwAccessCode;                        ///< access mode for the opened device
    uint32 dwShareMode;                        ///< share mode for the opened device
  uint32 dwClientID;             // client sequence number of LPA driver
} Aud_FTM_ClientCtxt_T, *pAud_FTM_ClientCtxt_T;


/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/



/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/

static DALBOOL    g_bDriverInitialized=FALSE;   //  whether it has been inited or not

static int16 vol_map[]={0,0x1000,0x2000,0x3000,0x4000,0x5000,0x6000,0x7000,0x7fff};

static int16  * pTmpBuf=NULL;

static uint16 *pAud_ftm_rx_buf=NULL;

/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static void aud_ftm_tone_generator(void *pDrvCtxt);
static void aud_ftm_play_tone( void * pDrvCtxt );

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/

static void aud_ftm_tone_generator(void *pCtxt)
{
  int32  size, capacity;
  uint16  i, channels;
    int16   sample;
    Aud_FTM_DrvCtxt_T * pDrvCtxt = (Aud_FTM_DrvCtxt_T *)pCtxt;

    channels=pDrvCtxt->client_param.channel;

  do
      {
          DALSYS_EventWaitTimeout(pDrvCtxt->hReq_Data_Event, 500);   /* 500ms timeout */
          if( pDrvCtxt->bStart != TRUE ) break;

          /* determine if needing fillful new data */
          size=cbuf_data_size(&(pDrvCtxt->fifo_data_buf));
          capacity=cbuf_capacity(&(pDrvCtxt->fifo_data_buf));

        /* generate data */
        DALSYS_memset(pTmpBuf,0,capacity-size-4);

        for(i=0; i<((capacity-size-4)/sizeof(int16)); i++)
        {
          sample=audio_ftm_dtmf_tone_sample_gen(&(pDrvCtxt->dtmfGen) ,pDrvCtxt->nDTMF_Gain);
          *(pTmpBuf+i)=sample;
                if(channels == AUDIO_FTM_CHN_2)
                {
                   i++;
                   *(pTmpBuf+i)=sample;
                }
        }

        aud_ftm_cbuf_write (&(pDrvCtxt->fifo_data_buf),(uint8 *)pTmpBuf,(int32)(capacity-size-4));

      }while(1);

    pthread_exit(0);
}

static void aud_ftm_play_tone( void * pCtxt )
{
  int32  size, capacity;
    Aud_FTM_DrvCtxt_T * pDrvCtxt = (Aud_FTM_DrvCtxt_T *)pCtxt;

  do
      {
          aud_ftm_cbuf_read(&(pDrvCtxt->fifo_data_buf),(uint8 *)pAud_ftm_rx_buf,
                             (pDrvCtxt->nRx_blk_convey_samples)*sizeof(int16) );

          audio_ftm_hw_write( pDrvCtxt->pDevCtxt,(void *)pAud_ftm_rx_buf,
            (uint32)pDrvCtxt->nRx_blk_convey_samples);

          /* determine if needing fillful new data */
          size=cbuf_data_size(&(pDrvCtxt->fifo_data_buf));
          capacity=cbuf_capacity(&(pDrvCtxt->fifo_data_buf));

      if( pDrvCtxt->bStart != TRUE ) break;

          if(size <= (capacity/2))
          {
            DALSYS_EventCtrl(pDrvCtxt->hReq_Data_Event, 0,0,NULL,0);
          }

      }while(1);

    pthread_exit(0);
}


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
audio_ftm_toneplay_attach
(
    void *pParam,                                 /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *pDriverHdl        /* Output: driver handle */
)
{
  /************* Context creation ***********/

  Aud_FTM_DevCtxt_T *pDeviceCtxt;
  Aud_FTM_DrvCtxt_T *pDriverCtxt;

    Aud_FTM_HW_INIT_PARAM_T hw_param;
    int16 freq1,freq2, sampling_freq;

    AUDIO_FTM_STS_T sts;

    if (g_bDriverInitialized)
  {
      DALSYS_Log_Err("%s: error: already initialized\n", __func__);
        return  AUDIO_FTM_ERROR;    //  already inited,
  }

  DALSYS_Log_Info("Attach TonePlay Driver\n");

    /****************** create context object ******************/

  if((DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DevCtxt_T), (void **)&pDeviceCtxt)) ||
        (pDeviceCtxt == NULL))
    {
      DALSYS_Log_Err("Memory allocation fail\n");
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

  DALSYS_memset(pDeviceCtxt,0,sizeof(Aud_FTM_DevCtxt_T));

  if((DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_DrvCtxt_T), (void **)&pDriverCtxt)) ||
        (pDriverCtxt == NULL))
    {
      DALSYS_Log_Err("Memory allocation fail\n");
                DALSYS_Free(pDeviceCtxt);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

  DALSYS_memset(pDriverCtxt,0,sizeof(Aud_FTM_DevCtxt_T));

    DALSYS_atomic_init(&pDriverCtxt->dwOpenCount);
    pDriverCtxt->pDevCtxt=pDeviceCtxt;
    pDeviceCtxt->pDrvCtxt=pDriverCtxt;

  /************* Create Critical Section ***********/

    if(DAL_SUCCESS != DALSYS_SyncCreate(DALSYS_SYNC_ATTR_RESOURCE,
        &(pDriverCtxt->hClientSync),
        NULL))
    {
      DALSYS_Log_Err("hClientSync creation fail\n");
        return AUDIO_FTM_ERROR;
    }

   if(DAL_SUCCESS != DALSYS_EventCreate(DALSYS_EVENT_ATTR_WORKLOOP_EVENT,       //Not used for work loops
                              &(pDriverCtxt->hReq_Data_Event),          //Return event handle
                              NULL))
   {
    DALSYS_Log_Err("%s: failed to create event\n", __func__);
    DALSYS_SyncLeave(pDriverCtxt->hClientSync);
    DALSYS_Free(pDriverCtxt->hClientSync);
    return AUDIO_FTM_ERROR;
   }

    /*************  hardware initialization ***********/

   DALSYS_memcpy((uint8*)&(pDriverCtxt->client_param),
                  (uint8*)pParam, sizeof(AUD_FTM_TONE_PLAY_PARAM_T));

   hw_param.inpath=AUDIO_FTM_IN_INVALID;
   hw_param.outpath=pDriverCtxt->client_param.path.outpath;
   hw_param.rate=AUDIO_FTM_PCM_RATE_8K;
   hw_param.width=AUDIO_FTM_BIT_WIDTH_16;
   hw_param.channel=AUDIO_FTM_CHN_1;
   hw_param.gain=pDriverCtxt->client_param.gain;
     hw_param.bLoopbackCase=FALSE;

   sts=aud_ftm_hw_init(pDeviceCtxt, &hw_param);
   if(sts != AUDIO_FTM_SUCCESS) {
      DALSYS_Log_Err("%s: failed to init hw\n", __func__);
        return AUDIO_FTM_ERROR;
   }

      /* Tone generator init */

    freq1=((AUD_FTM_TONE_PLAY_PARAM_T*)pParam)->dtmf_low;
    freq2=((AUD_FTM_TONE_PLAY_PARAM_T*)pParam)->dtmf_hi;
    sampling_freq=8000;

    audio_ftm_dtmf_tone_init(&(pDriverCtxt->dtmfGen),freq1,freq2,sampling_freq);
    pDriverCtxt->nDTMF_Gain = vol_map[3];

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
audio_ftm_toneplay_open
(
    AUDIO_FTM_DRIVER_HANDLE_T pDriverContext ,  /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T AccessMode,           /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T  ShareMode,            /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T * pClientContext  /* Output: client handle */
)
{
    Aud_FTM_DrvCtxt_T * pDrvCxt = (Aud_FTM_DrvCtxt_T *)pDriverContext;
    Aud_FTM_ClientCtxt_T * pOpenContext = NULL;
    AUDIO_FTM_STS_T  res;
    int16 i;
    uint32 dev_buf_size, fifo_buf_size;

    if (pDrvCxt == NULL)
    {
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        return AUDIO_FTM_ERR_INVALID_PARAM;
    }

    DALSYS_SyncEnter(pDrvCxt->hClientSync);

    if((ShareMode == AUD_FTM_SHAREMODE_EXCLU ) && (DALSYS_atomic_read(&pDrvCxt->dwOpenCount) != 0))
        /* ShareMode = 0: exclusive mode; =FILE_SHARE_READ: read only; =FILE_SHARE_WRITE: write only */
    {
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        return AUDIO_FTM_ERROR;
    }

  if(DAL_SUCCESS != DALSYS_Malloc(sizeof(Aud_FTM_ClientCtxt_T), (void **)&pOpenContext))
    {
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
        DALSYS_SyncLeave(pDrvCxt->hClientSync);
        return AUDIO_FTM_ERR_MEM_ALLOC_FAIL;
    }

    if (pOpenContext == NULL)
    {
      DALSYS_SyncLeave(pDrvCxt->hClientSync);
        DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
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

  /***** open HW *****/
  DALSYS_Log_Info("%s: open hw\n",__func__);
  pDrvCxt->pDevCtxt->playbackdevice =
  pDrvCxt->pDevCtxt->pDrvCtxt->client_param.device;
  res=audio_ftm_hw_open(pDrvCxt->pDevCtxt);
  if(res != AUDIO_FTM_SUCCESS)
  {
    DALSYS_Log_Err("%s: failed %d\n",__func__,__LINE__);
    DALSYS_SyncLeave(pDrvCxt->hClientSync);
    return AUDIO_FTM_ERROR;
  }

    audio_ftm_hw_iocontrol(pDrvCxt->pDevCtxt, IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE, NULL, 0, &dev_buf_size, sizeof(dev_buf_size), NULL);
    pDrvCxt->nRx_blk_convey_samples=dev_buf_size/sizeof(int16);

  /********* init circular buf ********/
    fifo_buf_size=dev_buf_size*AUD_FTM_TONE_PLAY_FIFO_FACTOR+4;    // reserve 4 bytes

  DALSYS_Log_Info("%s: init buf\n",__func__);
  aud_ftm_cbuf_init(&(pDrvCxt->fifo_data_buf), fifo_buf_size);

  if(DAL_SUCCESS != DALSYS_Malloc(dev_buf_size*AUD_FTM_TONE_PLAY_FIFO_FACTOR, (void **)&pTmpBuf))  /* buffer for DTMF data producing */
  {
          DALSYS_SyncLeave(pDrvCxt->hClientSync);
      DALSYS_Log_Err(" pTmpBuf allocation fail\n");
      return DAL_ERROR;
    }
  DALSYS_memset(pTmpBuf,0,dev_buf_size*AUD_FTM_TONE_PLAY_FIFO_FACTOR);

  if(DAL_SUCCESS != DALSYS_Malloc(dev_buf_size, (void **)&pAud_ftm_rx_buf))  /* buffer for fetch data from FIFO and pass to low layer driver for consuming */
  {
          DALSYS_SyncLeave(pDrvCxt->hClientSync);
      DALSYS_Log_Err(" pAud_ftm_rx_buf allocation fail\n");
      return DAL_ERROR;
    }
  DALSYS_memset(pAud_ftm_rx_buf,0,dev_buf_size);

  /***** prefill the circular buffer with DTMF data *****/

  for(i=0; i<((dev_buf_size*AUD_FTM_TONE_PLAY_FIFO_FACTOR)/sizeof(int16)); i++)
  {
    *(pTmpBuf+i)=audio_ftm_dtmf_tone_sample_gen(&(pDrvCxt->dtmfGen),pDrvCxt->nDTMF_Gain);
  }
  aud_ftm_cbuf_write (&(pDrvCxt->fifo_data_buf),(uint8 *)pTmpBuf,(int32)(dev_buf_size*AUD_FTM_TONE_PLAY_FIFO_FACTOR));

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
audio_ftm_toneplay_close
(
    AUDIO_FTM_CLIENT_HANDLE_T  pOpenContext       /* Input: client handle */
)
{
    Aud_FTM_ClientCtxt_T   *pExplicitOpenContext;
    Aud_FTM_DrvCtxt_T      *pDrvCtx;
    uint32                  nClient_id;
    uint32                  i;
    AUDIO_FTM_STS_T         res;

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

  aud_ftm_cbuf_clear(&(pDrvCtx->fifo_data_buf));

  for(i=nClient_id; i< DALSYS_atomic_read(&pDrvCtx->dwOpenCount)-1; i++)
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
    aud_ftm_cbuf_deinit(&(pDrvCtx->fifo_data_buf));
    DALSYS_Free(pTmpBuf); pTmpBuf=NULL;
    DALSYS_Free(pAud_ftm_rx_buf); pAud_ftm_rx_buf=NULL;
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
audio_ftm_toneplay_read
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
audio_ftm_toneplay_write
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
audio_ftm_toneplay_iocontrol
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
      int16  vol_max;

        if (pBufIn == NULL)
        {
       ret=AUDIO_FTM_ERR_INVALID_PARAM;
             break;
        }
        vol=*(uint16 *)pBufIn;

    /* change volume in HW */
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_CHG_VOL, (uint8 *)&vol, sizeof(vol), NULL, 0, NULL);
  }
    break;

    case IOCTL_AUDIO_FTM_START:
    {
      AFEDevAudIfDirType dir;
        int res;
        dir=AUDIO_IF_SINK;

        pDrvCtxt->bStart=TRUE;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_START, (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);

    /* create DTMF data producer thread */
    res=pthread_create(&(pDrvCtxt->producer_thread), NULL, aud_ftm_tone_generator,(void *)pDrvCtxt);
        if(res !=0 )
        {
          DALSYS_Log_Err("Failed to create Tone Generator thread\n");
            return AUDIO_FTM_ERROR;
        }

        /* create consumer thread */
    res=pthread_create(&(pDrvCtxt->consumer_thread), NULL, aud_ftm_play_tone,(void *)pDrvCtxt);
        if(res !=0 )
        {
          DALSYS_Log_Err("Failed to create Tone Play thread\n");
            return AUDIO_FTM_ERROR;
        }

  }
    break;

    case IOCTL_AUDIO_FTM_STOP:
    {
        int res;
      AFEDevAudIfDirType dir;
        dir=AUDIO_IF_SINK;

      pDrvCtxt->bStart=FALSE;
        DALSYS_Delay(100);   /* delay 100ms to consume all remained data */
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_STOP, (uint8 *)&dir, sizeof(dir), NULL, 0, NULL);

    res=pthread_join(pDrvCtxt->producer_thread, NULL);
        if(res !=0)
        {
          DALSYS_Log_Err("Failed to join DTMF generator thread\n");
            return AUDIO_FTM_ERROR;
        }
   res=pthread_join(pDrvCtxt->consumer_thread, NULL);
        if(res !=0)
        {
          DALSYS_Log_Err("Failed to join Tone Play thread\n");
            return AUDIO_FTM_ERROR;
        }

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
audio_ftm_toneplay_detach
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
  pDrvContext->bStart=FALSE;

  aud_ftm_hw_deinit(pDevContext);

  DALSYS_DestroySyncObject(pDrvContext->hClientSync);
  DALSYS_DestroyEventObject(pDrvContext->hReq_Data_Event);
  DALSYS_Free(pDevContext); pDevContext=NULL;
  DALSYS_Free(pDrvContext); pDrvContext=NULL;

    g_bDriverInitialized=FALSE;

    return AUDIO_FTM_SUCCESS;

}

/*================================================================================================*/


#ifdef __cplusplus
}
#endif
