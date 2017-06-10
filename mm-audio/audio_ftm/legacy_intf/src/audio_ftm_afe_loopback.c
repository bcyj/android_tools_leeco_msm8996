#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_afe_loopback.c
  @brief  AUDIO FTM Tone play driver
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_afe_loopback.c#2 $
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

#include "audio_ftm_afe_loopback.h"
#include "audio_ftm_util_fifo.h"
#include "audio_ftm_hw_drv.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/

#define MAX_AUD_FTM_CLIENT   1

/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

struct Aud_FTM_ClientCtxt_S;

typedef struct Aud_FTM_DrvCtxt_S{
   Aud_FTM_DevCtxt_T            *pDevCtxt;
   struct Aud_FTM_ClientCtxt_S  *apClientCtxt[MAX_AUD_FTM_CLIENT];

     DALBOOL                 bOpened;
     DAL_ATOMIC                  dwOpenCount;

   DALSYSSyncHandle             hClientSync;

     AUD_FTM_AFE_LP_PARAM_T       client_param;       /* parameters passed from client */
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

/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


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
audio_ftm_afe_lp_attach
(
    void *pParam,                                 /* Input: parameters  */
    AUDIO_FTM_DRIVER_HANDLE_T *pDriverHdl        /* Output: driver handle */
)
{

  /************* Context creation ***********/

  Aud_FTM_DevCtxt_T *pDeviceCtxt;
  Aud_FTM_DrvCtxt_T *pDriverCtxt;
  AUD_FTM_AFE_LP_PARAM_T     *afe_lp_param;

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
    afe_lp_param = (AUD_FTM_AFE_LP_PARAM_T *)pParam;
    pDeviceCtxt->capturedevice = afe_lp_param->capturedevice;
    pDeviceCtxt->playbackdevice = afe_lp_param->playbackdevice;
  /************* Create Critical Section ***********/

    if(DAL_SUCCESS != DALSYS_SyncCreate(DALSYS_SYNC_ATTR_RESOURCE,
        &(pDriverCtxt->hClientSync),
        NULL))
    {
        return AUDIO_FTM_ERROR;
    }

    /*************  hardware initialization ***********/

   DALSYS_memcpy((uint8*)&(pDriverCtxt->client_param),
                  (uint8*)pParam, sizeof(AUD_FTM_AFE_LP_PARAM_T));

   hw_param.inpath=pDriverCtxt->client_param.path.inpath;
   hw_param.outpath=pDriverCtxt->client_param.path.outpath;
   hw_param.rate=AUDIO_FTM_PCM_RATE_8K;
   hw_param.width=AUDIO_FTM_BIT_WIDTH_16;
   hw_param.channel=AUDIO_FTM_CHN_1;
   hw_param.gain=pDriverCtxt->client_param.gain;
   hw_param.bLoopbackCase=TRUE;
     hw_param.m_loopback_type=AUDIO_FTM_AFE_LOOPBACK;

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
audio_ftm_afe_lp_open
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

  pOpenContext->dwClientID= DALSYS_atomic_read(&pDrvCxt->dwOpenCount);

  pDrvCxt->apClientCtxt[pOpenContext->dwClientID]=pOpenContext;

  /***** open HW ****/
  res=audio_ftm_hw_open(pDrvCxt->pDevCtxt);
  if(res != AUDIO_FTM_SUCCESS)
  {
    DALSYS_SyncLeave(pDrvCxt->hClientSync);
    return AUDIO_FTM_ERROR;
  }

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
audio_ftm_afe_lp_close
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
      pDrvCtx->bOpened=FALSE;

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
audio_ftm_afe_lp_read
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
audio_ftm_afe_lp_write
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
audio_ftm_afe_lp_iocontrol
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
       ret=AUDIO_FTM_ERR_INVALID_PARAM;
             break;
        }
        vol=*(uint16 *)pBufIn;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_CHG_VOL,(uint8 *) &vol, sizeof(vol), NULL, 0, NULL);
  }
    break;

    case IOCTL_AUDIO_FTM_START:
    {
        AFEDevAudIfDirType dir;
        dir=AUDIO_IF_SOURCE;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_START, &dir, sizeof(dir), NULL, 0, NULL);
  }
    break;

    case IOCTL_AUDIO_FTM_STOP:
    {
        AFEDevAudIfDirType dir;
        dir=AUDIO_IF_SOURCE;
        audio_ftm_hw_iocontrol(pDevCtxt, IOCTL_AUDIO_FTM_STOP, &dir, sizeof(dir), NULL, 0, NULL);
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
audio_ftm_afe_lp_detach
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
