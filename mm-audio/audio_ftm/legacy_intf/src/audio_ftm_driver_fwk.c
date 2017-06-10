#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_driver_fwk.c
  @brief  AUDIO FTM driver framework
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_driver_fwk.c#1 $
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
#include "audio_ftm_driver.h"

/******** individual driver module API ***********/
#include "audio_ftm_tone_play.h"
#include "audio_ftm_afe_loopback.h"
#include "audio_ftm_pcm_loopback.h"
#include "audio_ftm_pcm_record.h"
#include "audio_ftm_pcm_play.h"

#define ARRAYLEN(a)	(sizeof(a)/sizeof(a[0]))
/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

typedef struct
{
  AUDIO_FTM_METHOD_ENUM_T  method_id;
  AUDIO_FTM_FncPtrTbl_T    intf;
}AUDIO_FTM_INTF_OBJ_T;

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/

static AUDIO_FTM_METHOD_ENUM_T nCurrent_ftm_method=AUDIO_FTM_METHOD_TONE_PLAY;
static AUDIO_FTM_FncPtrTbl_T   *g_pFncPtrTbl=NULL;

static AUDIO_FTM_INTF_OBJ_T g_IntfaceTable[]=
{
  {
    AUDIO_FTM_METHOD_TONE_PLAY,     {
                                      audio_ftm_toneplay_attach,
                                      audio_ftm_toneplay_open,
                                      audio_ftm_toneplay_close,
                                      audio_ftm_toneplay_read,
                                      audio_ftm_toneplay_write,
                                      audio_ftm_toneplay_iocontrol,
                                      audio_ftm_toneplay_detach
                                    }
  },
  {
    AUDIO_FTM_METHOD_PCM_LOOPBACK,  {
                                      audio_ftm_pcm_lp_attach,
                                      audio_ftm_pcm_lp_open,
                                      audio_ftm_pcm_lp_close,
                                      audio_ftm_pcm_lp_read,
                                      audio_ftm_pcm_lp_write,
                                      audio_ftm_pcm_lp_iocontrol,
                                      audio_ftm_pcm_lp_detach
                                    }

  },
  {
    AUDIO_FTM_METHOD_AFE_LOOPBACK,  {
                                      audio_ftm_afe_lp_attach,
                                      audio_ftm_afe_lp_open,
                                      audio_ftm_afe_lp_close,
                                      audio_ftm_afe_lp_read,
                                      audio_ftm_afe_lp_write,
                                      audio_ftm_afe_lp_iocontrol,
                                      audio_ftm_afe_lp_detach
                                    }
  },
  {
    AUDIO_FTM_METHOD_PCM_CAPTURE,  {
                                      audio_ftm_pcm_rec_attach,
                                      audio_ftm_pcm_rec_open,
                                      audio_ftm_pcm_rec_close,
                                      audio_ftm_pcm_rec_read,
                                      audio_ftm_pcm_rec_write,
                                      audio_ftm_pcm_rec_iocontrol,
                                      audio_ftm_pcm_rec_detach
                                    }
  },
  {
    AUDIO_FTM_METHOD_PCM_PLAY,  {
                                      audio_ftm_pcm_play_attach,
                                      audio_ftm_pcm_play_open,
                                      audio_ftm_pcm_play_close,
                                      audio_ftm_pcm_play_read,
                                      audio_ftm_pcm_play_write,
                                      audio_ftm_pcm_play_iocontrol,
                                      audio_ftm_pcm_play_detach
                                    }
  }
};


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

  @param nMethod      : Input - FTM test method enumeration
         pDriverHdl   : Output - a driver handle will be returned if attach success

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
Audio_FTM_Attach
(
    AUDIO_FTM_METHOD_ENUM_T nMethod,
    void *pParam,
    AUDIO_FTM_DRIVER_HANDLE_T * pDriverHdl
)
{
  uint8 i;
  AUDIO_FTM_STS_T ret;

  nCurrent_ftm_method=nMethod;

  if(g_pFncPtrTbl != NULL)
    return AUDIO_FTM_ERROR;

  for(i=0; i<AUDIO_FTM_METHOD_MAX;i++)
  {
    if(i<ARRAYLEN(g_IntfaceTable))
      if(nCurrent_ftm_method == g_IntfaceTable[i].method_id)
      break;
  }

	if(i >= AUDIO_FTM_METHOD_MAX)
		return AUDIO_FTM_ERR_INVALID_PARAM;

  g_pFncPtrTbl=&g_IntfaceTable[i].intf;
  ret=g_pFncPtrTbl->Attach(pParam,pDriverHdl);
  return ret;
}

/*==================================================================================================
  @brief FTM driver open

  This API can be used to open the FTM driver.

  @param DriverHdl:   Input -   handle created in Audio_FTM_Attach
         AccessMode:  Input -   Read / Write / Rd_Wr
         ShareMode:   Input -   Exclusive or Share
         pClientHdl:  Output -  point to the returned Open handle

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
Audio_FTM_Open
(
    AUDIO_FTM_DRIVER_HANDLE_T DriverHdl,  /* Input: driver handle */
    AUD_FTM_ACCESS_MODE_T     AccessMode, /* Input: Access mode */
    AUD_FTM_SHARE_MODE_T      ShareMode,  /* Input: Share mode */
    AUDIO_FTM_CLIENT_HANDLE_T *pClientHdl /* Output: client handle */
)
{
  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  return g_pFncPtrTbl->Open(DriverHdl,AccessMode,ShareMode,pClientHdl);
}


/*==================================================================================================
  @brief FTM driver Close

  This API can be used to Close the FTM driver.

  @param OpenHdl:   Input -   handle created in Audio_FTM_Open

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
Audio_FTM_Close
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl        /* Input: client handle */
)
{
  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  return g_pFncPtrTbl->Close(OpenHdl);
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
Audio_FTM_Read
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    void * pBuf,
    uint32 nBufSize,
    uint32 *pCount

)
{
  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  return g_pFncPtrTbl->Read(OpenHdl,pBuf,nBufSize,pCount);
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
Audio_FTM_Write
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    void * pBuf,
    uint32 nCount
)
{
  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  return g_pFncPtrTbl->Write(OpenHdl,pBuf,nCount);
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
Audio_FTM_IOControl
(
    AUDIO_FTM_CLIENT_HANDLE_T OpenHdl,
    uint32 dwIOCode,
    uint8 * pBufIn,
    uint32  dwLenIn,
    uint8 * pBufOut,
    uint32  dwLenOut,
    uint32 *pActualOutLen
)
{
  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  return g_pFncPtrTbl->IOControl(OpenHdl,dwIOCode,pBufIn,dwLenIn,pBufOut,dwLenOut,pActualOutLen);
}

/*==================================================================================================
  @brief FTM driver Detach

  This API can be used to Detach FTM driver, it will release all resournces

  @param DriverHdl:     Input -   handle created in Audio_FTM_Attach

  @return return code, AUDIO_FTM_SUCCESS on successful completion, error code otherwise
==================================================================================================*/

AUDIO_FTM_STS_T
Audio_FTM_Detach
(
    AUDIO_FTM_DRIVER_HANDLE_T DriverHdl
)
{
  AUDIO_FTM_STS_T res;

  if(g_pFncPtrTbl == NULL)
    return AUDIO_FTM_ERROR;
  res=g_pFncPtrTbl->Detach(DriverHdl);
  if(res != AUDIO_FTM_SUCCESS)
    return AUDIO_FTM_ERROR;
  g_pFncPtrTbl=NULL;

  /* TODO - has to be supported - only for bringup*/
  //DALSYS_DeInitMod();
	return res;
}

/*================================================================================================*/

#ifdef __cplusplus
}
#endif



