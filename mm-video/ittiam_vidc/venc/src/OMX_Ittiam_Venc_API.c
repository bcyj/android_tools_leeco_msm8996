/*
 * Copyright (c) 2005 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 */
/******************************************************************************
*
*                                 OMX Ittiam
*
*                     ITTIAM SYSTEMS PVT LTD, BANGALORE
*                             COPYRIGHT(C) 2011
*
*  This program is proprietary to ittiam systems pvt. ltd.,and is protected
*  under indian copyright act as an unpublished work.its use and disclosure
*  is limited by the terms and conditions of a license agreement.it may not
*  be copied or otherwise  reproduced or disclosed  to persons  outside the
*  licensee's   organization  except  in  accordance   with  the  terms and
*  conditions  of such  an agreement. all copies and reproductions shall be
*  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
*  its entirety.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>
#include <OMX_QCOMExtns.h>

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include "ittiam_datatypes.h"
#include "iv.h"
#include "ive.h"
#include "ih264_cxa8.h"

#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Venc.h>

#include<utils/Log.h>

#define ComponentVersion   "ITTIAM_OMX_VENC_03_00_SEP_12_2013"
OMX_U32 align16(OMX_U32 a)
{
  OMX_U32 b;
  if(a & 0xF)
  {
    b = a + (16 - (a & 0xF));
  }
  else
  {
    b = a;
  }
  return b;
}

OMX_U32 align4096(OMX_U32 a)
{
  OMX_U32 b;
  if(a & 4095)
  {
    b = a + (4096 - (a & 4095));
  }
  else
  {
    b = a;
  }
  return b;
}

OMX_U32 align2048(OMX_U32 a)
{
  OMX_U32 b;
  if(a & 2047)
  {
    b = a + (2048 - (a & 2047));
  }
  else
  {
    b = a;
  }
  return b;
}



/*
 *     F U N C T I O N S
 */

/*****************************************************************************/

OMX_ERRORTYPE SendCommand(OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData)
{
    VIDENCTYPE *pVidEnc;
    ThrCmdType eCmd;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, 1, 1);
    if (Cmd == OMX_CommandMarkBuffer)
       OMX_CONF_CHECK_CMD(pCmdData, 1, 1);

    if (pVidEnc->state == OMX_StateInvalid)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInvalidState);

    switch (Cmd){
       case OMX_CommandStateSet:
            pthread_mutex_lock(&pVidEnc->pipes_mutex);
            pVidEnc->cmd_pending = 1;
            pthread_mutex_unlock(&pVidEnc->pipes_mutex);
           eCmd = SetState;
           if(pVidEnc->state == OMX_StateExecuting && nParam1 == OMX_StateIdle)
           {
                pthread_mutex_lock(&pVidEnc->codec_mutex);
                pthread_cond_signal(&pVidEnc->codec_signal);
                pthread_mutex_unlock(&pVidEnc->codec_mutex);
           }
           break;
       case OMX_CommandFlush:
           eCmd = Flush;
            pthread_mutex_lock(&pVidEnc->pipes_mutex);
            pVidEnc->cmd_pending = 1;
            pthread_mutex_unlock(&pVidEnc->pipes_mutex);

            /* In case codec was waiting on O/P bufs, send a dummy signal */
            pthread_mutex_lock(&pVidEnc->codec_mutex);
            pthread_cond_signal(&pVidEnc->codec_signal);
            pthread_mutex_unlock(&pVidEnc->codec_mutex);

           if ((WORD32)nParam1 > 1 && (WORD32)nParam1 != -1)
              OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);
           break;
       case OMX_CommandPortDisable:
           eCmd = DisablePort;
           break;
       case OMX_CommandPortEnable:
           eCmd = EnablePort;
           break;
       case OMX_CommandMarkBuffer:
           eCmd = MarkBuf;
           if (nParam1 > 0)
                  OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);
               break;
       default:
               break;
       }

    pthread_mutex_lock(&pVidEnc->pipes_mutex);
    write(pVidEnc->cmdpipe[1], &eCmd, sizeof(eCmd));

    // In case of MarkBuf, the pCmdData parameter is used to carry the data.
    // In other cases, the nParam1 parameter carries the data.
    if(eCmd == MarkBuf)
        write(pVidEnc->cmddatapipe[1], &pCmdData, sizeof(OMX_PTR));
    else
        write(pVidEnc->cmddatapipe[1], &nParam1, sizeof(nParam1));
    pthread_mutex_unlock(&pVidEnc->pipes_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE GetState(OMX_IN  OMX_HANDLETYPE hComponent,
             OMX_OUT OMX_STATETYPE* pState)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pState, 1);

    *pState = pVidEnc->state;

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE SetCallbacks(OMX_IN  OMX_HANDLETYPE hComponent,
                 OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
                 OMX_IN  OMX_PTR pAppData)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;


    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pCallbacks, pAppData);

    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidEnc->state );

    pVidEnc->pCallbacks = pCallbacks;
    pVidEnc->pAppData = pAppData;

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE GetParameter(OMX_IN  OMX_HANDLETYPE hComponent,
                 OMX_IN  OMX_INDEXTYPE nParamIndex,
                 OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, ComponentParameterStructure, 1);

    if (pVidEnc->state == OMX_StateInvalid)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

	switch (nParamIndex){
	// Gets OMX_PORT_PARAM_TYPE structure
	case OMX_IndexParamVideoInit:
		memcpy(ComponentParameterStructure, &pVidEnc->sPortParam, sizeof
				(OMX_PORT_PARAM_TYPE));
		break;
		// Gets OMX_PARAM_PORTDEFINITIONTYPE structure
	case OMX_IndexParamPortDefinition:
		if (((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex
				== pVidEnc->sInPortDef.nPortIndex)
		{
			ITTIAM_DEBUG("%s, Input port definition Size = %d ", __FUNCTION__,(WORD32)pVidEnc->sInPortDef.nBufferSize );

			memcpy(ComponentParameterStructure, &pVidEnc->sInPortDef, sizeof
					(OMX_PARAM_PORTDEFINITIONTYPE));
			if (pVidEnc->mUseLifeEffects) {
				((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatAndroidOpaque;
			}
		}
		else if (((OMX_PARAM_PORTDEFINITIONTYPE *)
				(ComponentParameterStructure))->nPortIndex ==
						pVidEnc->sOutPortDef.nPortIndex)
			memcpy(ComponentParameterStructure, &pVidEnc->sOutPortDef, sizeof
					(OMX_PARAM_PORTDEFINITIONTYPE));
		else
			eError = OMX_ErrorBadPortIndex;
		break;
		// Gets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
	case OMX_IndexParamVideoPortFormat:
		if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex
				== pVidEnc->sInPortFormat.nPortIndex)
		{
			int index = ((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex;
			if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
					(ComponentParameterStructure))->nIndex > 2)
			{
				ITTIAM_DEBUG("pVidEnc->sInPortFormat.nIndex = %d ", (WORD32)pVidEnc->sInPortFormat.nIndex );
				eError = OMX_ErrorNoMore;
			}
			else
			{
				memcpy(ComponentParameterStructure, &pVidEnc->sInPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
				if (index == 0)
				{
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex = index;
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
				}
				if (index == 1)
				{
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex = index;
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->eColorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420PackedSemiPlanar32m4ka;
				}
				if (index == 2)
				{
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->eColorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatAndroidOpaque;
					pVidEnc->mUseLifeEffects = 1;
					((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex = index;
				}
			}
		}
		else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
				(ComponentParameterStructure))->nPortIndex ==
						pVidEnc->sOutPortFormat.nPortIndex){
			if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
					(ComponentParameterStructure))->nIndex >
			pVidEnc->sOutPortFormat.nIndex)
				eError = OMX_ErrorNoMore;
			else
				memcpy(ComponentParameterStructure, &pVidEnc->sOutPortFormat, sizeof
						(OMX_VIDEO_PARAM_PORTFORMATTYPE));
		}
		else
			eError = OMX_ErrorBadPortIndex;
		break;
		// Gets OMX_PRIORITYMGMTTYPE structure
	case OMX_IndexParamPriorityMgmt:
		memcpy(ComponentParameterStructure, &pVidEnc->sPriorityMgmt, sizeof
				(OMX_PRIORITYMGMTTYPE));
		break;
		// Gets OMX_VIDEO_PARAM_AVCTYPE structure
	case OMX_IndexParamVideoAvc:
		if (((OMX_VIDEO_PARAM_AVCTYPE *)(ComponentParameterStructure))->nPortIndex
				== pVidEnc->sH264.nPortIndex)
		{
			memcpy(ComponentParameterStructure, &pVidEnc->sH264, sizeof
					(OMX_VIDEO_PARAM_AVCTYPE));
		}
		else
		{
			eError = OMX_ErrorBadPortIndex;
		}

           break;
        case OMX_IndexParamVideoProfileLevelQuerySupported:
            if (((OMX_VIDEO_PARAM_PROFILELEVELTYPE *)(ComponentParameterStructure))->nPortIndex
                                                                   == pVidEnc->sInProfile.nPortIndex)
                {
                memcpy(ComponentParameterStructure, &pVidEnc->sInProfile, sizeof
                                                                   (OMX_VIDEO_PARAM_PROFILELEVELTYPE));
                }
            else
                {
                    memcpy(ComponentParameterStructure, &pVidEnc->sOutProfile, sizeof
                                                                   (OMX_VIDEO_PARAM_PROFILELEVELTYPE));
                }
            break;

        case OMX_IndexParamVideoBitrate:
            if (((OMX_VIDEO_PARAM_BITRATETYPE *)(ComponentParameterStructure))->nPortIndex
                                                                   == pVidEnc->sBitRateType.nPortIndex)
                {
                memcpy(ComponentParameterStructure, &pVidEnc->sBitRateType, sizeof
                                                                   (OMX_VIDEO_PARAM_BITRATETYPE));
                }
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        case OMX_IndexParamStandardComponentRole:
            memcpy(ComponentParameterStructure, &pVidEnc->componentRole, sizeof(OMX_PARAM_COMPONENTROLETYPE));
            break;

       default:
           eError = OMX_ErrorUnsupportedIndex;
           break;
       }

OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE SetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR ComponentParameterStructure)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

	ITTIAM_DEBUG("%s hComponent = %x nIndex = %x ComponentParameterStructure = %x", __FUNCTION__,
			(WORD32)hComponent, (WORD32)nIndex, (WORD32)ComponentParameterStructure);

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, ComponentParameterStructure, 1);

	ITTIAM_DEBUG("%s state = %x ", __FUNCTION__,pVidEnc->state);

    if (pVidEnc->state != OMX_StateLoaded)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

	switch (nIndex){
	// Sets OMX_PORT_PARAM_TYPE structure
	case OMX_IndexParamVideoInit:
		memcpy(&pVidEnc->sPortParam, ComponentParameterStructure, sizeof
				(OMX_PORT_PARAM_TYPE));
		break;
		// Sets OMX_PARAM_PORTDEFINITIONTYPE structure
	case OMX_IndexParamPortDefinition:
	{
		if (((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex
				== pVidEnc->sInPortDef.nPortIndex)
		{
			memcpy(&pVidEnc->sInPortDef, ComponentParameterStructure, sizeof
					(OMX_PARAM_PORTDEFINITIONTYPE));
			if (pVidEnc->sInPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatAndroidOpaque) {
				pVidEnc->sInPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
				pVidEnc->mUseLifeEffects = 1;
			}
                   ITTIAM_DEBUG("%s Input port definition Size = %d ", __FUNCTION__,(WORD32)pVidEnc->sInPortDef.nBufferSize );
                   ITTIAM_DEBUG("resolution width %d: height %d",
                                (WORD32)pVidEnc->sInPortDef.format.video.nFrameWidth,
                                (WORD32)pVidEnc->sInPortDef.format.video.nFrameHeight);

		}
		else if (((OMX_PARAM_PORTDEFINITIONTYPE *)
				(ComponentParameterStructure))->nPortIndex ==
						pVidEnc->sOutPortDef.nPortIndex)
		{
			memcpy(&pVidEnc->sOutPortDef, ComponentParameterStructure, sizeof
					(OMX_PARAM_PORTDEFINITIONTYPE));
			ITTIAM_DEBUG("%s Output port definition Size = %d ", __FUNCTION__,(WORD32)pVidEnc->sOutPortDef.nBufferSize );
			ITTIAM_DEBUG("resolution width %d: height %d",
					(WORD32)pVidEnc->sOutPortDef.format.video.nFrameWidth,
					(WORD32)pVidEnc->sOutPortDef.format.video.nFrameHeight);
		}
		else
			eError = OMX_ErrorBadPortIndex;
		break;
	}
	// Sets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
	case OMX_IndexParamVideoPortFormat:
		if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex
				== pVidEnc->sInPortFormat.nPortIndex){
			if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex > 2)
				eError = OMX_ErrorNoMore;
			else
			{
				memcpy(&pVidEnc->sInPortFormat, ComponentParameterStructure, sizeof
						(OMX_VIDEO_PARAM_PORTFORMATTYPE));
				if (pVidEnc->sInPortFormat.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatAndroidOpaque) {
					pVidEnc->sInPortFormat.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
					pVidEnc->mUseLifeEffects = 1;
				}
			}
		}
		else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
				(ComponentParameterStructure))->nPortIndex ==
						pVidEnc->sOutPortFormat.nPortIndex){
			if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
					(ComponentParameterStructure))->nIndex >
			pVidEnc->sOutPortFormat.nIndex)
				eError = OMX_ErrorNoMore;
			else
				memcpy(&pVidEnc->sOutPortFormat, ComponentParameterStructure, sizeof
						(OMX_VIDEO_PARAM_PORTFORMATTYPE));
		}
		else
			eError = OMX_ErrorBadPortIndex;
		break;
		// Sets OMX_PRIORITYMGMTTYPE structure
	case OMX_IndexParamPriorityMgmt:
		memcpy(&pVidEnc->sPriorityMgmt, ComponentParameterStructure, sizeof
				(OMX_PRIORITYMGMTTYPE));
		break;
	case OMX_IndexParamVideoAvc:

        if (((OMX_VIDEO_PARAM_AVCTYPE *)(ComponentParameterStructure))->nPortIndex
                                                         == pVidEnc->sH264.nPortIndex)
        {   memcpy(&pVidEnc->sH264, ComponentParameterStructure, sizeof
                                                           (OMX_VIDEO_PARAM_AVCTYPE));
                                                           }
        else
        {
            eError = OMX_ErrorBadPortIndex;
            }
        break;
        case OMX_IndexParamVideoBitrate:
            if (((OMX_VIDEO_PARAM_BITRATETYPE *)(ComponentParameterStructure))->nPortIndex
                                                                   == pVidEnc->sBitRateType.nPortIndex)
                {
                memcpy( &pVidEnc->sBitRateType,ComponentParameterStructure, sizeof
                                                                   (OMX_VIDEO_PARAM_BITRATETYPE));
                }
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        case OMX_IndexParamStandardComponentRole:
            memcpy(&pVidEnc->componentRole, ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE));
            break;

       default:
           eError = OMX_ErrorUnsupportedIndex;
           break;
       }

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
              OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
              OMX_IN OMX_U32 nPortIndex,
              OMX_IN OMX_PTR pAppPrivate,
              OMX_IN OMX_U32 nSizeBytes,
              OMX_IN OMX_U8* pBuffer)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_U32 nIndex = 0x0;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, ppBufferHdr, pBuffer);

    if (nPortIndex == pVidEnc->sInPortDef.nPortIndex)
       pPortDef = &pVidEnc->sInPortDef;
    else if (nPortIndex == pVidEnc->sOutPortDef.nPortIndex)
            pPortDef = &pVidEnc->sOutPortDef;
         else
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    if (!pPortDef->bEnabled)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == pVidEnc->sInPortDef.nPortIndex){
       ListAllocate(pVidEnc->sInBufList, nIndex);
       if (pVidEnc->sInBufList.pBufHdr[nIndex] == NULL){
          pVidEnc->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE)) ;
          if (!pVidEnc->sInBufList.pBufHdr[nIndex])
             OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
          OMX_CONF_INIT_STRUCT_PTR (pVidEnc->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
          }
//ICS changes
        if (!(pVidEnc->meta_mode_enable))
        {
            pVidEnc->sInBufList.pBufHdr[nIndex]->pBuffer = pBuffer;
        }
//ICS changes

       pVidEnc->sInBufList.bBufOwner[nIndex] = OMX_FALSE;
       LoadBufferHeader(pVidEnc->sInBufList, pVidEnc->sInBufList.pBufHdr[nIndex], pAppPrivate,
                                            nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
    ITTIAM_DEBUG("Input Use Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidEnc->sInPortDef.bPopulated, (WORD32)pVidEnc->sInBufList.nAllocSize);
    }else{
       ListAllocate(pVidEnc->sOutBufList,  nIndex);
       if (pVidEnc->sOutBufList.pBufHdr[nIndex] == NULL){
          pVidEnc->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                  OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
          if (!pVidEnc->sOutBufList.pBufHdr[nIndex])
              OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
          OMX_CONF_INIT_STRUCT_PTR (pVidEnc->sOutBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
          }
       pVidEnc->sOutBufList.pBufHdr[nIndex]->pBuffer = pBuffer;
       pVidEnc->sOutBufList.bBufOwner[nIndex] = OMX_FALSE;

       LoadBufferHeader(pVidEnc->sOutBufList, pVidEnc->sOutBufList.pBufHdr[nIndex],
                                   pAppPrivate, nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Output Use Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidEnc->sOutPortDef.bPopulated, (WORD32)pVidEnc->sOutBufList.nAllocSize);
       }

    pthread_mutex_lock(&pVidEnc->signal_mutex);
    if((pVidEnc->sInPortDef.bPopulated == pVidEnc->sInPortDef.bEnabled) &&
       (pVidEnc->sOutPortDef.bPopulated == pVidEnc->sOutPortDef.bEnabled)){
        pthread_cond_signal(&pVidEnc->buffers_signal);
    }
    pthread_mutex_unlock(&pVidEnc->signal_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                   OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                   OMX_IN OMX_U32 nPortIndex,
                   OMX_IN OMX_PTR pAppPrivate,
                   OMX_IN OMX_U32 nSizeBytes)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S8 nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, ppBufferHdr, 1);

    if (nPortIndex == pVidEnc->sInPortDef.nPortIndex)
       pPortDef = &pVidEnc->sInPortDef;
    else{
       if (nPortIndex == pVidEnc->sOutPortDef.nPortIndex)
      pPortDef = &pVidEnc->sOutPortDef;
       else
          OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);
       }

    if (!pPortDef->bEnabled)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == pVidEnc->sInPortDef.nPortIndex){
       ListAllocate(pVidEnc->sInBufList,  nIndex);
       if (pVidEnc->sInBufList.pBufHdr[nIndex] == NULL){
          pVidEnc->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE)) ;
          if (!pVidEnc->sInBufList.pBufHdr[nIndex])
             OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
          OMX_CONF_INIT_STRUCT_PTR (pVidEnc->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
          }
       pVidEnc->sInBufList.pBufHdr[nIndex]->pBuffer = (OMX_U8*)
                                                           OMX_OSAL_Malloc(nSizeBytes);

       if (!pVidEnc->sInBufList.pBufHdr[nIndex]->pBuffer)
          OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

       pVidEnc->sInBufList.bBufOwner[nIndex] = OMX_TRUE;
       LoadBufferHeader(pVidEnc->sInBufList, pVidEnc->sInBufList.pBufHdr[nIndex], pAppPrivate,
                                            nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);

    ITTIAM_DEBUG("Input Allocate Buffer index = %d, bPopulated = %d nAllocSize = %d size %d", (WORD32)nPortIndex, (WORD32)pVidEnc->sInPortDef.bPopulated, (WORD32)pVidEnc->sInBufList.nAllocSize,(WORD32)pVidEnc->sInPortDef.nBufferSize);

      }
    else{
       ListAllocate(pVidEnc->sOutBufList,  nIndex);
       if (pVidEnc->sOutBufList.pBufHdr[nIndex] == NULL){
          pVidEnc->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE)) ;
          if (!pVidEnc->sOutBufList.pBufHdr[nIndex])
             OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
          OMX_CONF_INIT_STRUCT_PTR(pVidEnc->sOutBufList.pBufHdr[nIndex],OMX_BUFFERHEADERTYPE);
          }
       pVidEnc->sOutBufList.pBufHdr[nIndex]->pBuffer = (OMX_U8*)
                                                            OMX_OSAL_Malloc(sizeof(nSizeBytes));
       if (!pVidEnc->sOutBufList.pBufHdr[nIndex]->pBuffer)
          OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

       pVidEnc->sOutBufList.bBufOwner[nIndex] = OMX_TRUE;
       LoadBufferHeader(pVidEnc->sOutBufList, pVidEnc->sOutBufList.pBufHdr[nIndex],
                                   pAppPrivate, nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Output Allocate Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidEnc->sOutPortDef.bPopulated, (WORD32)pVidEnc->sOutBufList.nAllocSize);
       }


    pthread_mutex_lock(&pVidEnc->signal_mutex);
    if((pVidEnc->sInPortDef.bPopulated == pVidEnc->sInPortDef.bEnabled) &&
       (pVidEnc->sOutPortDef.bPopulated == pVidEnc->sOutPortDef.bEnabled)){
        pthread_cond_signal(&pVidEnc->buffers_signal);
    }
    pthread_mutex_unlock(&pVidEnc->signal_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE FreeBuffer(OMX_IN  OMX_HANDLETYPE hComponent,
               OMX_IN  OMX_U32 nPortIndex,
               OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_U32 nIndex;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == pVidEnc->sInPortDef.nPortIndex){
       pPortDef = &pVidEnc->sInPortDef;
       ListFreeBuffer(pVidEnc->sInBufList, pBufferHdr, pPortDef, nIndex)
    ITTIAM_DEBUG("Buffer freed Port index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pPortDef->bPopulated, (WORD32)pVidEnc->sInBufList.nAllocSize);
       }
    else if (nPortIndex == pVidEnc->sOutPortDef.nPortIndex){
        pPortDef = &pVidEnc->sOutPortDef;
            ListFreeBuffer(pVidEnc->sOutBufList, pBufferHdr, pPortDef, nIndex)
    ITTIAM_DEBUG("Buffer freed Port index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pPortDef->bPopulated, (WORD32)pVidEnc->sOutBufList.nAllocSize);
            }
         else
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);


    if (pPortDef->bEnabled && pVidEnc->state != OMX_StateIdle && pVidEnc->state != OMX_StateInvalid)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    pthread_mutex_lock(&pVidEnc->signal_mutex);
    if (!pVidEnc->sInBufList.nAllocSize && !pVidEnc->sOutBufList.nAllocSize){
        pthread_cond_signal(&pVidEnc->buffers_signal);
    }
    pthread_mutex_unlock(&pVidEnc->signal_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}



/*****************************************************************************/
OMX_ERRORTYPE EmptyThisBuffer(OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDENCTYPE *pVidEnc;
    ThrCmdType eCmd = EmptyBuf;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    if (!pVidEnc->sInPortDef.bEnabled)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);

    if (pVidEnc->state != OMX_StateExecuting && pVidEnc->state != OMX_StatePause)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

     /*
      pthread_mutex_lock(&pVidEnc->codec_mutex);
      ITTIAM_DEBUG("Ittiam: ETB Buffer %x", pBufferHdr->pBuffer + pVidEnc->offset);
      pthread_mutex_unlock(&pVidEnc->codec_mutex);
    */

	// Put the command and data in the pipe
	pthread_mutex_lock(&pVidEnc->pipes_mutex);
	pVidEnc->NumETB++;
    //ALOGD("NumETB %d ",pVidEnc->NumETB);
	write(pVidEnc->cmdpipe[1], &eCmd, sizeof(eCmd));
	write(pVidEnc->cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));
	pthread_mutex_unlock(&pVidEnc->pipes_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE FillThisBuffer(OMX_IN  OMX_HANDLETYPE hComponent,
                   OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDENCTYPE *pVidEnc;
    ThrCmdType eCmd = FillBuf;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    if (!pVidEnc->sOutPortDef.bEnabled)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);

    if (pVidEnc->state != OMX_StateExecuting && pVidEnc->state != OMX_StatePause)
       OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

	pthread_mutex_lock(&pVidEnc->codec_mutex);
  //  ITTIAM_DEBUG("FTB Buffer %x", pBufferHdr->pBuffer + pVidEnc->offset);
	pthread_mutex_unlock(&pVidEnc->codec_mutex);

	// Put the command and data in the pipe
	pthread_mutex_lock(&pVidEnc->pipes_mutex);
	pVidEnc->NumFTB++;
    //ALOGD("NumFTB %d",pVidEnc->NumFTB);
	write(pVidEnc->cmdpipe[1], &eCmd, sizeof(eCmd));
	write(pVidEnc->cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));
	pthread_mutex_unlock(&pVidEnc->pipes_mutex);

OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE ComponentDeInit(OMX_IN  OMX_HANDLETYPE hComponent)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType eCmd = StopThread;
    OMX_U32 nIndex = 0;

	pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
	ITTIAM_DEBUG("Ittiam Encoder De-init");
	// In case the client crashes, check for nAllocSize parameter.
	// If this is greater than zero, there are elements in the list that are not free'd.
	// In that case, free the elements.
	ITTIAM_DEBUG("pVidEnc->sInBufList.nAllocSize = %d",(WORD32)pVidEnc->sInBufList.nAllocSize );
	if ((WORD32)pVidEnc->sInBufList.nAllocSize > 0)
		ListFreeAllBuffers(pVidEnc->sInBufList, nIndex)
		ITTIAM_DEBUG("pVidEnc->sOutBufList.nAllocSize = %d",(WORD32)pVidEnc->sOutBufList.nAllocSize );
	if ((WORD32)pVidEnc->sOutBufList.nAllocSize > 0)
		ListFreeAllBuffers(pVidEnc->sOutBufList, nIndex)

    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidEnc->pipes_mutex);
    write(pVidEnc->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidEnc->cmddatapipe[1], &eCmd, sizeof(eCmd));
    pthread_mutex_unlock(&pVidEnc->pipes_mutex);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(pVidEnc->thread_id, (void*)&eError);

    // close the pipe handles
    close(pVidEnc->cmdpipe[0]);
    close(pVidEnc->cmdpipe[1]);
    close(pVidEnc->cmddatapipe[0]);
    close(pVidEnc->cmddatapipe[1]);

    pthread_mutex_destroy(&pVidEnc->pipes_mutex);
    pthread_mutex_destroy(&pVidEnc->signal_mutex);
    pthread_mutex_destroy(&pVidEnc->codec_mutex);

    pthread_cond_destroy(&pVidEnc->buffers_signal);
    pthread_cond_destroy(&pVidEnc->codec_signal);

    OMX_OSAL_Free(pVidEnc);
    return eError;
}

OMX_ERRORTYPE ComponentRoleEnum(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_U8 *cRole,
    OMX_IN OMX_U32 nIndex)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(nIndex == 0){
        if(pVidEnc->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingAVC)
        {
        strlcpy((char *)cRole, "video_encoder.avc",OMX_MAX_STRINGNAME_SIZE);
        }
        else
        {
            eError = OMX_ErrorInvalidComponentName;
        }
    }
    else
    {
        eError = OMX_ErrorNoMore;
    }
    return eError;

}


OMX_ERRORTYPE GetConfig(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_RECTTYPE *rect;

	ITTIAM_DEBUG("%s hComponent = %x nIndex = %x ComponentParameterStructure = %x",__FUNCTION__,
			(WORD32)hComponent, (WORD32)nIndex, (WORD32)pComponentConfigStructure);
	pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
	OMX_CONF_CHECK_CMD(pVidEnc, pComponentConfigStructure, 1);

	if(pVidEnc->state == OMX_StateInvalid)
	{
		ITTIAM_DEBUG("%s in Invalid State",__FUNCTION__);
		return OMX_ErrorInvalidState;
	}

    switch(nIndex) {
       case OMX_IndexConfigCommonOutputCrop:
          break;

       default:
           eError = OMX_ErrorUnsupportedIndex;
           break;
     }

OMX_CONF_CMD_BAIL:
    return eError;

}

OMX_ERRORTYPE SetConfig(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR pComponentConfigStructure)
{
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 LC_level;

	ITTIAM_DEBUG("%s hComponent = %x nIndex = %x ComponentParameterStructure = %x",__FUNCTION__,
			(WORD32)hComponent, (WORD32)nIndex, (WORD32)pComponentConfigStructure);

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidEnc, pComponentConfigStructure, 1);

    switch (nIndex){
       default:
           eError = OMX_ErrorUnsupportedIndex;
           break;
       }

OMX_CONF_CMD_BAIL:
    return eError;
}

OMX_ERRORTYPE GetExtensionIndex(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_STRING cParameterName,
        OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    VIDENCTYPE *pVidEnc;

    pVidEnc = (VIDENCTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

	if(pVidEnc->state == OMX_StateInvalid)
	{
		ITTIAM_ERROR("Get Extension Index in Invalid State\n");
		return OMX_ErrorInvalidState;
	}
	else if(!strncmp(cParameterName,"OMX.ITTIAM.index.LClevel", sizeof("OMX.ITTIAM.index.LClevel") - 1)) {
		*pIndexType = (OMX_INDEXTYPE)CUSTOM_OMX_IndexConfigLCLevel;
	}
	else {
		ITTIAM_ERROR("Extension: %s not implemented\n", cParameterName);
		return OMX_ErrorNotImplemented;
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE StubbedComponentTunnelRequest(
    OMX_IN  OMX_HANDLETYPE hComp,
    OMX_IN  OMX_U32 nPort,
    OMX_IN  OMX_HANDLETYPE hTunneledComp,
    OMX_IN  OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE StubbedGetComponentVersion(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STRING cComponentName,
        OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
        OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
        OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE StubbedUseEGLImage(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN void* eglImage)
{
    return OMX_ErrorNotImplemented;
}

/*****************************************************************************/
OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN  OMX_HANDLETYPE hComponent, OMX_IN OMX_VIDEO_CODINGTYPE VidFormat,
                                    OMX_IN  OMX_COLOR_FORMATTYPE ColorFormat,OMX_IN OMX_U32 high_quality_encoder,OMX_IN OMX_U32 quadcore, OMX_IN OMX_U32 alt_ref_flag)
{
    OMX_COMPONENTTYPE *pComp;
    VIDENCTYPE *pVidEnc;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 err;
    OMX_U32 nIndex;

	pComp = (OMX_COMPONENTTYPE *)hComponent;
	ITTIAM_LOG("OMX VERSION :: %s",ComponentVersion);
	// Create private data
	pVidEnc = (VIDENCTYPE *)OMX_OSAL_Malloc(sizeof(VIDENCTYPE));
    if(pVidEnc == NULL)
    {
          ITTIAM_DEBUG("Memory allocation failed for VidEnc in ComponentInit( ) \n");
          return OMX_ErrorInsufficientResources;
    }
	memset(pVidEnc, 0x0, sizeof(VIDENCTYPE));

    pComp->pComponentPrivate = (OMX_PTR)pVidEnc;
    pVidEnc->state = OMX_StateLoaded;
    pVidEnc->hSelf = hComponent;
    pVidEnc->initdone = 0;
    pVidEnc->FramesEncoded = 0;
    pVidEnc->meta_mode_enable = 0;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidEnc->state );

    // Fill in function pointers
    pComp->SetCallbacks             =   SetCallbacks;
    pComp->GetComponentVersion      =   StubbedGetComponentVersion;
    pComp->SendCommand              =   SendCommand;
    pComp->GetParameter             =   GetParameter;
    pComp->SetParameter             =   SetParameter;
    pComp->GetConfig                =   GetConfig;
    pComp->SetConfig                =   SetConfig;
    pComp->GetExtensionIndex        =   GetExtensionIndex;
    pComp->GetState                 =   GetState;
    pComp->ComponentTunnelRequest   =   StubbedComponentTunnelRequest;
    pComp->UseBuffer                =   UseBuffer;
    pComp->AllocateBuffer           =   AllocateBuffer;
    pComp->FreeBuffer               =   FreeBuffer;
    pComp->EmptyThisBuffer          =   EmptyThisBuffer;
    pComp->FillThisBuffer           =   FillThisBuffer;
    pComp->ComponentDeInit          =   ComponentDeInit;
    pComp->UseEGLImage              =   StubbedUseEGLImage;
    pComp->ComponentRoleEnum        =   ComponentRoleEnum;

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sPortParam, OMX_PORT_PARAM_TYPE);
    pVidEnc->sPortParam.nPorts = 0x2;
    pVidEnc->sPortParam.nStartPortNumber = 0x0;

	// Initialize the video parameters for input port
	OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
	pVidEnc->sInPortDef.nPortIndex = 0x0;
	pVidEnc->sInPortDef.bEnabled = OMX_TRUE;
	pVidEnc->sInPortDef.bPopulated = OMX_FALSE;
	pVidEnc->sInPortDef.eDomain = OMX_PortDomainVideo;
	pVidEnc->sInPortDef.format.video.cMIMEType = "yuv";
	pVidEnc->sInPortDef.format.video.nFrameWidth = MAX_WIDTH;
	pVidEnc->sInPortDef.format.video.nFrameHeight = MAX_HEIGHT;
	pVidEnc->sInPortDef.format.video.nStride = -1;
	pVidEnc->sInPortDef.format.video.nSliceHeight = -1;
	pVidEnc->sInPortDef.format.video.nBitrate = 64000;
	pVidEnc->sInPortDef.format.video.xFramerate = 15 << 16;
	pVidEnc->sInPortDef.eDir = OMX_DirInput;
	pVidEnc->sInPortDef.nBufferCountMin = NUM_IN_BUFFERS;
	pVidEnc->sInPortDef.nBufferCountActual = NUM_IN_BUFFERS;
	pVidEnc->sInPortDef.nBufferSize =  (OMX_U32)((MAX_WIDTH*MAX_HEIGHT*3) >> 1); // JK TBD
	pVidEnc->sInPortDef.format.video.eCompressionFormat =  OMX_VIDEO_CodingUnused;
	pVidEnc->sInPortDef.format.video.eColorFormat =  ColorFormat;

       // Hack!!
	// If the input color format is chosen other than OMX_COLOR_FormatYUV420SemiPlanar
	// the omx build crashes at run time with error unsupported color format 21.
	// On 8x25, 8x25q, 7x27 devices the input color format is IOMX_COLOR_FormatYVU420SemiPlanar.
	// So we are maintaining a copy of the actual input color format in the variable sInColorFormat for
	// later uses and let Getparameter function changes the input format to OMX_COLOR_FormatYUV420SemiPlanar.
      pVidEnc->sInColorFormat = ColorFormat;

       // Initialize the video parameters for output port
	OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
	pVidEnc->sOutPortDef.nPortIndex = 0x1;
	pVidEnc->sOutPortDef.bEnabled = OMX_TRUE;
	pVidEnc->sOutPortDef.bPopulated = OMX_FALSE;
      pVidEnc->sOutPortDef.eDomain = OMX_PortDomainVideo;
	pVidEnc->sOutPortDef.format.video.cMIMEType = "264";
	pVidEnc->sOutPortDef.format.video.nFrameWidth = MAX_WIDTH;
	pVidEnc->sOutPortDef.format.video.nFrameHeight = MAX_HEIGHT;
	pVidEnc->sOutPortDef.format.video.nStride      = -1;
	pVidEnc->sOutPortDef.format.video.nSliceHeight = -1;
	pVidEnc->sOutPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
	pVidEnc->sOutPortDef.format.video.nBitrate = 64000;
	pVidEnc->sOutPortDef.format.video.xFramerate = 15 << 16;
	pVidEnc->sOutPortDef.eDir = OMX_DirOutput;
	pVidEnc->sOutPortDef.nBufferCountMin = NUM_OUT_BUFFERS;
	pVidEnc->sOutPortDef.nBufferCountActual = NUM_OUT_BUFFERS;
	pVidEnc->sOutPortDef.nBufferSize =  (OMX_U32)((MAX_WIDTH*MAX_HEIGHT*3) >> 2);
	pVidEnc->sOutPortDef.format.video.eColorFormat =  OMX_VIDEO_CodingUnused;

	// Initialize the video compression format for input port
	OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sInPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
	pVidEnc->sInPortFormat.nPortIndex = 0x0;
	pVidEnc->sInPortFormat.nIndex = 0x2;
	pVidEnc->sInPortFormat.eColorFormat = ColorFormat;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sOutPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pVidEnc->sOutPortFormat.nPortIndex = 0x1;
    pVidEnc->sOutPortFormat.nIndex = 0x0;
    pVidEnc->sOutPortFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sH264, OMX_VIDEO_PARAM_AVCTYPE);
    pVidEnc->sH264.nPortIndex = 0x1;
    pVidEnc->sH264.eProfile = OMX_VIDEO_AVCProfileBaseline;
    pVidEnc->sH264.eLevel = OMX_VIDEO_AVCLevel1;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sInProfile, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
    pVidEnc->sInProfile.nPortIndex = 0x0;
    pVidEnc->sInProfile.eLevel = OMX_VIDEO_AVCLevel1;
    pVidEnc->sInProfile.eProfile = OMX_VIDEO_AVCProfileBaseline;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sOutProfile, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
    pVidEnc->sOutProfile.nPortIndex = 0x1;
    pVidEnc->sOutProfile.eLevel = OMX_VIDEO_AVCLevel1;
    pVidEnc->sOutProfile.eProfile = OMX_VIDEO_AVCProfileBaseline;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sPriorityMgmt, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sBitRateType, OMX_VIDEO_PARAM_BITRATETYPE);
    pVidEnc->sBitRateType.nPortIndex = 0x1;
    pVidEnc->sBitRateType.eControlRate = OMX_Video_ControlRateConstant;
    pVidEnc->sBitRateType.nTargetBitrate = 64000;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sInBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pVidEnc->sInBufSupplier.nPortIndex = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&pVidEnc->sOutBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pVidEnc->sOutBufSupplier.nPortIndex = 0x1;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidEnc->state );


	if((ColorFormat != OMX_COLOR_FormatYUV420Planar) &&
			(ColorFormat != OMX_COLOR_FormatYUV420SemiPlanar) &&
			(ColorFormat != (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar) &&
			(ColorFormat != (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420PackedSemiPlanar32m4ka))
	{
		return OMX_ErrorUndefined;
	}

    // Initialize the input buffer list
    memset(&(pVidEnc->sInBufList), 0x0, sizeof(BufferList));
    pVidEnc->sInBufList.pBufHdr = (OMX_BUFFERHEADERTYPE**)
                               OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                                                       pVidEnc->sInPortDef.nBufferCountActual);
    if(pVidEnc->sInBufList.pBufHdr == NULL)
    {
          ITTIAM_DEBUG("Unable to allocate memory for sInBufList\n");
          return OMX_ErrorInsufficientResources;
    }

    for (nIndex = 0; nIndex < pVidEnc->sInPortDef.nBufferCountActual; nIndex++) {
        pVidEnc->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                 OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        OMX_CONF_INIT_STRUCT_PTR (pVidEnc->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
        }

    pVidEnc->sInBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
                               OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                                                       pVidEnc->sInPortDef.nBufferCountActual);

    pVidEnc->sInBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                                                       pVidEnc->sInPortDef.nBufferCountActual);
    pVidEnc->sInBufList.nSizeOfList = 0;
    pVidEnc->sInBufList.nAllocSize = 0;
    pVidEnc->sInBufList.nListEnd = -1;
    pVidEnc->sInBufList.nWritePos = -1;
    pVidEnc->sInBufList.nReadPos = -1;
    pVidEnc->sInBufList.eDir = OMX_DirInput;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidEnc->state );

    // Initialize the output buffer list
    memset(&(pVidEnc->sOutBufList), 0x0, sizeof(BufferList));
    pVidEnc->sOutBufList.pBufHdr = (OMX_BUFFERHEADERTYPE**)
                                         OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                                                      pVidEnc->sOutPortDef.nBufferCountActual);
    if(pVidEnc->sOutBufList.pBufHdr == NULL)
    {
          ITTIAM_DEBUG("Unable to allocate memory for OutBufList of encoder \n");
          return OMX_ErrorInsufficientResources;
    }

    for (nIndex = 0; nIndex < pVidEnc->sOutPortDef.nBufferCountActual; nIndex++) {
        pVidEnc->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
                                                 OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        OMX_CONF_INIT_STRUCT_PTR (pVidEnc->sOutBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
        }
    pVidEnc->sOutBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
                                         OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                                                      pVidEnc->sOutPortDef.nBufferCountActual);

    pVidEnc->sOutBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                                                      pVidEnc->sOutPortDef.nBufferCountActual);
    pVidEnc->sOutBufList.nSizeOfList = 0;
    pVidEnc->sOutBufList.nAllocSize = 0;
    pVidEnc->sOutBufList.nListEnd = -1;
    pVidEnc->sOutBufList.nWritePos = -1;
    pVidEnc->sOutBufList.nReadPos = -1;
    pVidEnc->sOutBufList.eDir = OMX_DirOutput;

    pVidEnc->LowComplexity = 1;
    pVidEnc->NumETB = 0;
    pVidEnc->NumEBD = 0;
    pVidEnc->BufferUnmaprequired = 0;
    pVidEnc->data_generation_started = 0;
    pVidEnc->hdr_encode_done = 0;
    pVidEnc->highqualityencode = high_quality_encoder;
    pVidEnc->quadcore = quadcore;
    pVidEnc->mUseLifeEffects = 0;
    pVidEnc->u4_alt_ref_frame = alt_ref_flag;

    // Create the pipe used to send commands to the thread
    err = pipe(pVidEnc->cmdpipe);
    if (err){
       eError = OMX_ErrorInsufficientResources;
       goto EXIT;
       }

    // Create the pipe used to send command data to the thread
    err = pipe(pVidEnc->cmddatapipe);
    if (err){
       eError = OMX_ErrorInsufficientResources;
       goto EXIT;
       }

    // One pipe is thread safe. But since two pipes are used here,
    // a mutex lock is required to prevent any race condition

    pthread_mutex_init(&pVidEnc->pipes_mutex, NULL);
    pthread_mutex_init(&pVidEnc->signal_mutex, NULL);
    pthread_mutex_init(&pVidEnc->codec_mutex, NULL);

    pthread_cond_init(&pVidEnc->buffers_signal, NULL);
    pthread_cond_init(&pVidEnc->codec_signal, NULL);

	// Create the component thread
	err = pthread_create(&pVidEnc->thread_id, NULL, ComponentThread, pVidEnc);

	if( err || !pVidEnc->thread_id ) {
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}


EXIT:
      ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidEnc->state );
      return eError;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
