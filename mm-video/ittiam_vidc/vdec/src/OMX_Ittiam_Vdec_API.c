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
extern "C"
{
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

#include "datatypedef.h"
#include "iv.h"
#include "ivd.h"
#ifdef ENABLE_MPEG4
#include "imp4d_cxa8.h"
#endif
#ifdef ENABLE_MPEG2
#include "imp2d_cxa8.h"
#endif
#ifdef ENABLE_H264
#include "ih264d_cxa8.h"
#endif
#ifdef ENABLE_HEVC
#include "ihevcd_cxa.h"
#endif

#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include<utils/Log.h>

#define ComponentVersion   "ITTIAM_OMX_VDEC_03_02_SEP_12_2013"

#define OUTPUT_BUFFER_LOG 0

#if OUTPUT_BUFFER_LOG
FILE *outputBufferFile1;
char outputfilename [] = "/data/output.yuv";
#endif

//#define DISABLE_LOW_COMPLEXITY
//#define SUSPEND_RESUME_HACK

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

OMX_U32 align32(OMX_U32 a)
{
    OMX_U32 b;
    if(a & 31)
    {
        b = a + (32 - (a & 31));
    }
    else
    {
        b = a;
    }
    return b;
}

OMX_U32 align64(OMX_U32 a)
{
    OMX_U32 b;
    if(a & 63)
    {
        b = a + (64 - (a & 63));
    }
    else
    {
        b = a;
    }
    return b;
}

OMX_U32 align128(OMX_U32 a)
{
    OMX_U32 b;
    if(a & 127)
    {
        b = a + (128 - (a & 127));
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

OMX_U32 align8192(OMX_U32 a)
{
    OMX_U32 b;
    if(a & 8191)
    {
        b = a + (8192 - (a & 8191));
    }
    else
    {
        b = a;
    }
    return b;
}


WORD32 get_num_disp_buffers(UWORD32 width, UWORD32 height);
void ittiam_video_release_display_frame(VIDDECTYPE* pVidDec,
                                        UWORD32 disp_buf_id);
void video_skipb_frames(VIDDECTYPE* pVidDec);
void video_skip_none(VIDDECTYPE* pVidDec);
void video_skip_pb_frames(VIDDECTYPE* pVidDec);
void avc_set_deblocking_lvl(VIDDECTYPE* pVidDec, UWORD32 level);
void mpeg4_enable_qpel(VIDDECTYPE* pVidDec);
void mpeg4_disable_qpel(VIDDECTYPE* pVidDec);

/*
 *     F U N C T I O N S
 */

/*****************************************************************************/

OMX_ERRORTYPE SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_COMMANDTYPE Cmd,
                OMX_IN OMX_U32 nParam1,
                OMX_IN OMX_PTR pCmdData)
{
    VIDDECTYPE *pVidDec;
    ThrCmdType eCmd;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, 1, 1);
    if (Cmd == OMX_CommandMarkBuffer)
    OMX_CONF_CHECK_CMD(pCmdData, 1, 1);

    if (pVidDec->state == OMX_StateInvalid)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInvalidState);

    switch (Cmd)
    {
        case OMX_CommandStateSet:
        pthread_mutex_lock(&pVidDec->pipes_mutex);
        pVidDec->cmd_pending = 1;
        pthread_mutex_unlock(&pVidDec->pipes_mutex);
        eCmd = SetState;
        if(pVidDec->share_disp_buf && pVidDec->state == OMX_StateExecuting && nParam1 == OMX_StateIdle)
        {
            pthread_mutex_lock(&pVidDec->codec_mutex);
            pthread_cond_signal(&pVidDec->codec_signal);
            pthread_mutex_unlock(&pVidDec->codec_mutex);
        }
        break;
        case OMX_CommandFlush:
        eCmd = Flush;
        pthread_mutex_lock(&pVidDec->pipes_mutex);
        pVidDec->cmd_pending = 1;
        pthread_mutex_unlock(&pVidDec->pipes_mutex);
        if(pVidDec->share_disp_buf)
        {
            /* In case codec was waiting on O/P bufs, send a dummy signal */
            pthread_mutex_lock(&pVidDec->codec_mutex);
            pthread_cond_signal(&pVidDec->codec_signal);
            pthread_mutex_unlock(&pVidDec->codec_mutex);
        }

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

    pthread_mutex_lock(&pVidDec->pipes_mutex);
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));

    // In case of MarkBuf, the pCmdData parameter is used to carry the data.
    // In other cases, the nParam1 parameter carries the data.
    if(eCmd == MarkBuf)
    write(pVidDec->cmddatapipe[1], &pCmdData, sizeof(OMX_PTR));
    else
    write(pVidDec->cmddatapipe[1], &nParam1, sizeof(nParam1));
    pthread_mutex_unlock(&pVidDec->pipes_mutex);

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE GetState(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STATETYPE* pState)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pState, 1);

    *pState = pVidDec->state;

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                OMX_IN OMX_PTR pAppData)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pCallbacks, pAppData);

    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );

    pVidDec->pCallbacks = pCallbacks;
    pVidDec->pAppData = pAppData;

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_INDEXTYPE nParamIndex,
                OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, ComponentParameterStructure, 1);

    if (pVidDec->state == OMX_StateInvalid)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    switch (nParamIndex)
    {
        // Gets OMX_PORT_PARAM_TYPE structure
        case OMX_IndexParamVideoInit:
        memcpy(ComponentParameterStructure, &pVidDec->sPortParam, sizeof
                        (OMX_PORT_PARAM_TYPE));
        break;
        // Gets OMX_PARAM_PORTDEFINITIONTYPE structure
        case OMX_IndexParamPortDefinition:
        if (((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex
                        == pVidDec->sInPortDef.nPortIndex)
        {
            ITTIAM_DEBUG("GetParameter Input port definition Size = %d ", (WORD32)pVidDec->sInPortDef.nBufferSize );

            memcpy(ComponentParameterStructure, &pVidDec->sInPortDef, sizeof
                            (OMX_PARAM_PORTDEFINITIONTYPE));
        }
        else if (((OMX_PARAM_PORTDEFINITIONTYPE *)
                                        (ComponentParameterStructure))->nPortIndex ==
                        pVidDec->sOutPortDef.nPortIndex)
        memcpy(ComponentParameterStructure, &pVidDec->sOutPortDef, sizeof
                        (OMX_PARAM_PORTDEFINITIONTYPE));
        else
        eError = OMX_ErrorBadPortIndex;
        break;
        // Gets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
        case OMX_IndexParamVideoPortFormat:
        if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex
                        == pVidDec->sInPortFormat.nPortIndex)
        {
            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
                                            (ComponentParameterStructure))->nIndex >
                            pVidDec->sInPortFormat.nIndex)
            eError = OMX_ErrorNoMore;
            else
            memcpy(ComponentParameterStructure, &pVidDec->sInPortFormat, sizeof
                            (OMX_VIDEO_PARAM_PORTFORMATTYPE));
        }
        else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
                                        (ComponentParameterStructure))->nPortIndex ==
                        pVidDec->sOutPortFormat.nPortIndex)
        {
            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
                                            (ComponentParameterStructure))->nIndex >
                            pVidDec->sOutPortFormat.nIndex)
            eError = OMX_ErrorNoMore;
            else
            memcpy(ComponentParameterStructure, &pVidDec->sOutPortFormat, sizeof
                            (OMX_VIDEO_PARAM_PORTFORMATTYPE));
        }
        else
        eError = OMX_ErrorBadPortIndex;
        break;
        // Gets OMX_PRIORITYMGMTTYPE structure
        case OMX_IndexParamPriorityMgmt:
        memcpy(ComponentParameterStructure, &pVidDec->sPriorityMgmt, sizeof
                        (OMX_PRIORITYMGMTTYPE));
        break;
        // Gets OMX_VIDEO_PARAM_MPEG4TYPE structure
        case OMX_IndexParamVideoMpeg4:
        if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingMPEG4)
        {
            if (((OMX_VIDEO_PARAM_MPEG4TYPE *)(ComponentParameterStructure))->nPortIndex
                            == pVidDec->sMpeg4.nPortIndex)
            memcpy(ComponentParameterStructure, &pVidDec->sMpeg4, sizeof
                            (OMX_VIDEO_PARAM_MPEG4TYPE));
            else
            eError = OMX_ErrorBadPortIndex;
        }
        else
        {
            eError = OMX_ErrorUnsupportedIndex;
        }

        break;
        // Gets OMX_VIDEO_PARAM_AVCTYPE structure
        case OMX_IndexParamVideoAvc:
        if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)
        {
            if (((OMX_VIDEO_PARAM_AVCTYPE *)(ComponentParameterStructure))->nPortIndex
                            == pVidDec->sH264.nPortIndex)
            memcpy(ComponentParameterStructure, &pVidDec->sH264, sizeof
                            (OMX_VIDEO_PARAM_AVCTYPE));
            else
            eError = OMX_ErrorBadPortIndex;
        }
        else
        {
            eError = OMX_ErrorUnsupportedIndex;
        }

        break;

        case OMX_IndexParamStandardComponentRole:
        memcpy(ComponentParameterStructure, &pVidDec->componentRole, sizeof(OMX_PARAM_COMPONENTROLETYPE));
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
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_INDEXTYPE nIndex,
                OMX_IN OMX_PTR ComponentParameterStructure)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    ITTIAM_DEBUG("SetParameter hComponent = %x nIndex = %x ComponentParameterStructure = %x",
                    (WORD32)hComponent,(WORD32) nIndex, (WORD32)ComponentParameterStructure);

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, ComponentParameterStructure, 1);

    ITTIAM_DEBUG("SetParameter state = %x ", pVidDec->state );

    if (pVidDec->state != OMX_StateLoaded)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    switch ((WORD32)nIndex)
    {
        // Sets OMX_PORT_PARAM_TYPE structure
        case OMX_IndexParamVideoInit:
            memcpy(&pVidDec->sPortParam, ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
            break;

        case OMX_IndexParamVideoProfileLevelCurrent:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *param;
            param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)ComponentParameterStructure;
            pVidDec->nProfile = param->eProfile;
            pVidDec->nLevel = param->eLevel;
            ITTIAM_DEBUG("SetParameter OMX_IndexParamVideoProfileLevelCurrent profile = %d  level = %d", pVidDec->nProfile, pVidDec->nLevel );
            break;
        }
        // Sets OMX_PARAM_PORTDEFINITIONTYPE structure
        case OMX_IndexParamPortDefinition:
        if (((OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure))->nPortIndex
                        == pVidDec->sInPortDef.nPortIndex)
        {
            memcpy(&pVidDec->sInPortDef, ComponentParameterStructure, sizeof
                            (OMX_PARAM_PORTDEFINITIONTYPE));
            ITTIAM_DEBUG("SetParameter Input port definition Size = %d ", (WORD32)pVidDec->sInPortDef.nBufferSize );
        }
        else if (((OMX_PARAM_PORTDEFINITIONTYPE *)
                                        (ComponentParameterStructure))->nPortIndex ==
                        pVidDec->sOutPortDef.nPortIndex)
        {
            memcpy(&pVidDec->sOutPortDef, ComponentParameterStructure, sizeof
                            (OMX_PARAM_PORTDEFINITIONTYPE));

            pVidDec->sOutPortDef.format.video.nStride = align16(pVidDec->sOutPortDef.format.video.nFrameWidth) + pVidDec->width_padding;
            ITTIAM_LOG("nStride %d nFrameWidth %d\n", (WORD32)pVidDec->sOutPortDef.format.video.nStride, (WORD32)pVidDec->sOutPortDef.format.video.nFrameWidth);
#ifndef ICS
            if(pVidDec->mProcessorType == SAMSUNG_GENERIC)
            {
                pVidDec->sOutPortDef.format.video.nSliceHeight = align16(pVidDec->sOutPortDef.format.video.nFrameHeight) + (pVidDec->height_padding/2);
            }
            else
            {
                pVidDec->sOutPortDef.format.video.nSliceHeight = (pVidDec->sOutPortDef.format.video.nFrameHeight) + (pVidDec->height_padding/2);
            }
            pVidDec->offset = ((pVidDec->y_height_padding/2) * (pVidDec->sOutPortDef.format.video.nFrameWidth + pVidDec->width_padding))
            + (pVidDec->width_padding/2);
#else
            if(!(pVidDec->ThumbnailMode))
            {
                if(pVidDec->mProcessorType == SAMSUNG_GENERIC)
                {
                    pVidDec->sOutPortDef.format.video.nSliceHeight = align16(pVidDec->sOutPortDef.format.video.nFrameHeight) + (pVidDec->y_height_padding);
                }
                else
                {
                    pVidDec->sOutPortDef.format.video.nSliceHeight = (pVidDec->sOutPortDef.format.video.nFrameHeight) + (pVidDec->y_height_padding);
                }
            }
            else
            {
                pVidDec->sOutPortDef.format.video.nSliceHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
            }

            pVidDec->offset = 0;
#endif
                    // Store the crop values
                    pVidDec->sCropRect.nWidth = pVidDec->sOutPortDef.format.video.nFrameWidth;
                    pVidDec->sCropRect.nHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
                    // Frame width and height values should be aligned.so assigning stride and slice height values
                    pVidDec->sOutPortDef.format.video.nFrameWidth = pVidDec->sOutPortDef.format.video.nStride;
                    pVidDec->sOutPortDef.format.video.nFrameHeight = pVidDec->sOutPortDef.format.video.nSliceHeight;

                    UWORD32 VideoArea = pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nFrameHeight;

            if(0 == pVidDec->share_disp_buf)
            {

                WORD32 y_buf_size;
                WORD32 y_buf_size_align;
                if(!(pVidDec->ThumbnailMode))
                {
                    pVidDec->sOutPortDef.format.video.nStride = align32(pVidDec->sOutPortDef.format.video.nStride);
                    if(pVidDec->mProcessorType != SAMSUNG_GENERIC)
                    {
                        if((pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m) ||
                                        (pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar))
                        {
                            pVidDec->sOutPortDef.format.video.nStride = align128(pVidDec->sOutPortDef.format.video.nStride);
                            pVidDec->sOutPortDef.format.video.nSliceHeight = align32(pVidDec->sOutPortDef.format.video.nSliceHeight);
                        }
                    }
                }
            }

            if( (pVidDec->sOutPortDef.format.video.nFrameWidth < MIN_WIDTH) || (pVidDec->sOutPortDef.format.video.nFrameHeight < MIN_HEIGHT)
                            || (VideoArea > (pVidDec->max_width * pVidDec->max_height)))
            {
                ITTIAM_ERROR("Unsupported resolution width %d: height %d",
                                (WORD32)pVidDec->sOutPortDef.format.video.nFrameWidth,
                                (WORD32)pVidDec->sOutPortDef.format.video.nFrameHeight);
                eError = OMX_ErrorUnsupportedSetting;
                return eError;
            }

            if(pVidDec->share_disp_buf)
            pVidDec->stride = 0;
            else
            pVidDec->stride = pVidDec->sOutPortDef.format.video.nStride;
            ITTIAM_LOG("New stride %d slice height %d\n", (WORD32)pVidDec->sOutPortDef.format.video.nStride, (WORD32)pVidDec->sOutPortDef.format.video.nSliceHeight);

            if(pVidDec->sOutPortDef.nBufferCountMin == pVidDec->sOutPortDef.nBufferCountActual)
            {
                if((1 == pVidDec->share_disp_buf) && ((pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) || (pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)))
                {
                    pVidDec->sOutPortDef.nBufferCountMin = get_num_disp_buffers(
                                    pVidDec->sOutPortDef.format.video.nFrameWidth,
                                    pVidDec->sOutPortDef.format.video.nFrameHeight);
                }
                else
                {
                    pVidDec->sOutPortDef.nBufferCountMin = 11;
                    /* When dimensions are greater than 1280x720, PMEM can not allocate 11 buffers in the following platforms */
                    /* This will have negative impact in terms of performance for handling peak bitrates */
                    if((QCOM_7X27A == pVidDec->mProcessorType) ||
                        (QCOM_8X25 == pVidDec->mProcessorType) ||
                        (QCOM_8X25Q == pVidDec->mProcessorType))
                    {
                        if(pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nFrameHeight > 1280 * 720)
                            pVidDec->sOutPortDef.nBufferCountMin = 5;

                    }
                }

                pVidDec->sOutPortDef.nBufferCountMin -= pVidDec->minUndequeuedBufs; // For ICS, display allocates 2 extra buffers

#ifdef SUSPEND_RESUME_HACK
                pVidDec->sOutPortDef.nBufferCountMin -= 1;
#endif
                if((pVidDec->ThumbnailMode == 1) && (0 == pVidDec->share_disp_buf))
                {
                    pVidDec->sOutPortDef.nBufferCountMin = 1;
                }


                pVidDec->sOutPortDef.nBufferCountActual = pVidDec->sOutPortDef.nBufferCountMin;

                ITTIAM_DEBUG("Recalculated Buffer count = %d", (WORD32)pVidDec->sOutPortDef.nBufferCountActual);
            }
            else
            {
                ITTIAM_DEBUG("New Buffer count(from the client) = %d", (WORD32)pVidDec->sOutPortDef.nBufferCountActual);
            }
            pVidDec->sOutPortDef.nBufferSize = (pVidDec->sOutPortDef.format.video.nStride) *
            (pVidDec->sOutPortDef.format.video.nSliceHeight +
                            pVidDec->sOutPortDef.format.video.nSliceHeight / 2);

            if(pVidDec->mProcessorType == SAMSUNG_GENERIC)
            {
                pVidDec->sOutPortDef.nBufferSize = pVidDec->sOutPortDef.nBufferSize;
            }
            else
            {
                pVidDec->sOutPortDef.nBufferSize = align8192(pVidDec->sOutPortDef.nBufferSize);
            }

        }
        else
        eError = OMX_ErrorBadPortIndex;
        break;
        // Sets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
        case OMX_IndexParamVideoPortFormat:
        if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nPortIndex
                        == pVidDec->sInPortFormat.nPortIndex)
        {
            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure))->nIndex
                            > pVidDec->sInPortFormat.nIndex)
            eError = OMX_ErrorNoMore;
            else
            memcpy(&pVidDec->sInPortFormat, ComponentParameterStructure, sizeof
                            (OMX_VIDEO_PARAM_PORTFORMATTYPE));
        }
        else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
                                        (ComponentParameterStructure))->nPortIndex ==
                        pVidDec->sOutPortFormat.nPortIndex)
        {
            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE *)
                                            (ComponentParameterStructure))->nIndex >
                            pVidDec->sOutPortFormat.nIndex)
            eError = OMX_ErrorNoMore;
            else
            memcpy(&pVidDec->sOutPortFormat, ComponentParameterStructure, sizeof
                            (OMX_VIDEO_PARAM_PORTFORMATTYPE));
        }
        else
        eError = OMX_ErrorBadPortIndex;
        break;
        // Sets OMX_PRIORITYMGMTTYPE structure
        case OMX_IndexParamPriorityMgmt:
        memcpy(&pVidDec->sPriorityMgmt, ComponentParameterStructure, sizeof
                        (OMX_PRIORITYMGMTTYPE));
        break;
        // Sets OMX_VIDEO_PARAM_MPEG4TYPE structure
        case OMX_IndexParamVideoMpeg4:
        if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingMPEG4)
        {
            if (((OMX_VIDEO_PARAM_MPEG4TYPE *)(ComponentParameterStructure))->nPortIndex
                            == pVidDec->sMpeg4.nPortIndex)
            memcpy(&pVidDec->sMpeg4, ComponentParameterStructure, sizeof
                            (OMX_VIDEO_PARAM_MPEG4TYPE));
            else
            eError = OMX_ErrorBadPortIndex;
        }
        else
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
        case OMX_IndexParamVideoAvc:
        if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)
        {
            if (((OMX_VIDEO_PARAM_AVCTYPE *)(ComponentParameterStructure))->nPortIndex
                            == pVidDec->sH264.nPortIndex)
            memcpy(&pVidDec->sH264, ComponentParameterStructure, sizeof
                            (OMX_VIDEO_PARAM_AVCTYPE));
            else
            eError = OMX_ErrorBadPortIndex;
        }
        else
        {
            eError = OMX_ErrorUnsupportedIndex;
        }
        break;
        case OMX_IndexParamStandardComponentRole:
        memcpy(&pVidDec->componentRole, ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE));
        char* cTempRole = NULL;
        cTempRole = (char*)pVidDec->componentRole.cRole;

        if(!strcmp(cTempRole,"video_decoder.avc"))
        {
            OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingAVC;
            pVidDec->sInPortDef.format.video.cMIMEType = "H264";
            pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
            pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
            OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sH264, OMX_VIDEO_PARAM_AVCTYPE);
            pVidDec->sH264.nPortIndex = 0x0;
            pVidDec->width_padding = 0;
            pVidDec->height_padding = 0;
            pVidDec->y_height_padding = 0;
            pVidDec->uv_height_padding = 0;
#ifdef ENABLE_H264
            pVidDec->iVdec_cxa8_api_function = ih264d_cxa8_api_function;
#endif
        }
        if(!strcmp(cTempRole,"video_decoder.mpeg4"))
        {
            OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingMPEG4;
            pVidDec->sInPortDef.format.video.cMIMEType = "MPEG4";
            pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
            pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
            OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sMpeg4, OMX_VIDEO_PARAM_MPEG4TYPE);
            pVidDec->sMpeg4.nPortIndex = 0x0;
            pVidDec->width_padding = 0;
            pVidDec->height_padding = 0;
            pVidDec->y_height_padding = 0;
            pVidDec->uv_height_padding = 0;
#ifdef ENABLE_MPEG4
            pVidDec->iVdec_cxa8_api_function = imp4d_cxa8_api_function;
#endif
        }
        if(!strcmp(cTempRole,"video_decoder.hevc"))
        {
            OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingHEVC;
            pVidDec->sInPortDef.format.video.cMIMEType = "HEVC";
            pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
            pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
            OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sMpeg4, OMX_VIDEO_PARAM_MPEG4TYPE);
            pVidDec->sMpeg4.nPortIndex = 0x0;
            pVidDec->width_padding = 0;
            pVidDec->height_padding = 0;
            pVidDec->y_height_padding = 0;
            pVidDec->uv_height_padding = 0;
#ifdef ENABLE_HEVC
            pVidDec->iVdec_cxa8_api_function = ihevcd_cxa_api_function;
#endif
            if(pVidDec->share_disp_buf)
            {
                pVidDec->width_padding = HEVC_WIDTH_PADDING;
                pVidDec->height_padding = HEVC_HEIGHT_PADDING;
                pVidDec->y_height_padding = HEVC_Y_HEIGHT_PADDING;
                pVidDec->uv_height_padding = HEVC_UV_HEIGHT_PADDING;
            }
        }
        break;

        case CUSTOM_OMX_ThumbnailMode:
        pVidDec->ThumbnailMode = 1;
        pVidDec->share_disp_buf = 0;
        pVidDec->width_padding = 0;
        pVidDec->height_padding = 0;
        pVidDec->y_height_padding = 0;
        pVidDec->uv_height_padding = 0;
        pVidDec->max_width = 1920;
        pVidDec->max_height = 1080;
        pVidDec->sOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
        pVidDec->sOutPortFormat.eColorFormat = OMX_COLOR_FormatYUV420Planar;
        ITTIAM_LOG("In ThumbnailMode");
        break;
        case (OMX_INDEXTYPE)OMX_QcomIndexParamVideoDivx:
        ITTIAM_LOG("Divx Codecs");
        break;
        case OMX_GoogleAndroidIndexEnableAndroidNativeBuffer:
        ITTIAM_LOG("This should work");
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
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_U32 nIndex = 0x0;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, ppBufferHdr, pBuffer);

    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
    pPortDef = &pVidDec->sInPortDef;
    else if (nPortIndex == pVidDec->sOutPortDef.nPortIndex)
    pPortDef = &pVidDec->sOutPortDef;
    else
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    if (!pPortDef->bEnabled)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pVidDec->sOutPortDef.format.video.eColorFormat != OMX_COLOR_FormatYUV420SemiPlanar)
    {
        if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
        {
            ITTIAM_DEBUG("<<VG>>Improper Buffer Size nSizeBytes %d nBufferSize %d",nSizeBytes,pPortDef->nBufferSize);
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);
        }
    }

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
    {
        ITTIAM_DEBUG("<<VG>>Input Use Buffer index");
        ListAllocate(pVidDec->sInBufList, nIndex);
        if (pVidDec->sInBufList.pBufHdr[nIndex] == NULL)
        {
            pVidDec->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
            OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!pVidDec->sInBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
            OMX_CONF_INIT_STRUCT_PTR (pVidDec->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
        }
        pVidDec->sInBufList.pBufHdr[nIndex]->pBuffer = pBuffer;
        pVidDec->sInBufList.bBufOwner[nIndex] = OMX_FALSE;
        LoadBufferHeader(pVidDec->sInBufList, pVidDec->sInBufList.pBufHdr[nIndex], pAppPrivate,
                        nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Input Use Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidDec->sInPortDef.bPopulated, (WORD32)pVidDec->sInBufList.nAllocSize);
    }
    else
    {
        ITTIAM_DEBUG("<<VG>>Output Use Buffer index");
        ListAllocate(pVidDec->sOutBufList, nIndex);
        if (pVidDec->sOutBufList.pBufHdr[nIndex] == NULL)
        {
            pVidDec->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
            OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!pVidDec->sOutBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
            OMX_CONF_INIT_STRUCT_PTR (pVidDec->sOutBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
        }
        pVidDec->sOutBufList.pBufHdr[nIndex]->pBuffer = pBuffer;
        pVidDec->sOutBufList.bBufOwner[nIndex] = OMX_FALSE;

        LoadBufferHeader(pVidDec->sOutBufList, pVidDec->sOutBufList.pBufHdr[nIndex],
                        pAppPrivate, nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Output Use Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidDec->sOutPortDef.bPopulated, (WORD32)pVidDec->sOutBufList.nAllocSize);
    }

    pthread_mutex_lock(&pVidDec->signal_mutex);
    if((pVidDec->sInPortDef.bPopulated == pVidDec->sInPortDef.bEnabled) &&
                    (pVidDec->sOutPortDef.bPopulated == pVidDec->sOutPortDef.bEnabled))
    {
        pthread_cond_signal(&pVidDec->buffers_signal);
        pVidDec->bufferallocationpending = 0;
        pVidDec->PortReconfiguration = 0;
        data_sync_barrier();
    }
    pthread_mutex_unlock(&pVidDec->signal_mutex);

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
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S8 nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, ppBufferHdr, 1);

    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
    pPortDef = &pVidDec->sInPortDef;
    else
    {
        if (nPortIndex == pVidDec->sOutPortDef.nPortIndex)
        pPortDef = &pVidDec->sOutPortDef;
        else
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);
    }

    if (!pPortDef->bEnabled)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
    {
        ListAllocate(pVidDec->sInBufList, nIndex);
        if (pVidDec->sInBufList.pBufHdr[nIndex] == NULL)
        {
            pVidDec->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
            OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!pVidDec->sInBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
            OMX_CONF_INIT_STRUCT_PTR (pVidDec->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
        }
        pVidDec->sInBufList.pBufHdr[nIndex]->pBuffer = (OMX_U8*)
        OMX_OSAL_Malloc(nSizeBytes);
        if (!pVidDec->sInBufList.pBufHdr[nIndex]->pBuffer)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

        pVidDec->sInBufList.bBufOwner[nIndex] = OMX_TRUE;
        LoadBufferHeader(pVidDec->sInBufList, pVidDec->sInBufList.pBufHdr[nIndex], pAppPrivate,
                        nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);

        ITTIAM_DEBUG("Input Allocate Buffer index = %d, bPopulated = %d nAllocSize = %d Buffer =%x", (WORD32)nPortIndex, (WORD32)pVidDec->sInPortDef.bPopulated, (WORD32)pVidDec->sInBufList.nAllocSize,
                        (WORD32)pVidDec->sInBufList.pBufHdr[nIndex]->pBuffer);

    }
    else
    {
        ListAllocate(pVidDec->sOutBufList, nIndex);
        if (pVidDec->sOutBufList.pBufHdr[nIndex] == NULL)
        {
            pVidDec->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
            OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!pVidDec->sOutBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
            OMX_CONF_INIT_STRUCT_PTR(pVidDec->sOutBufList.pBufHdr[nIndex],OMX_BUFFERHEADERTYPE);
        }
        pVidDec->sOutBufList.pBufHdr[nIndex]->pBuffer = (OMX_U8*)
        OMX_OSAL_Malloc(nSizeBytes);
        if (!pVidDec->sOutBufList.pBufHdr[nIndex]->pBuffer)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

        pVidDec->sOutBufList.bBufOwner[nIndex] = OMX_TRUE;
        LoadBufferHeader(pVidDec->sOutBufList, pVidDec->sOutBufList.pBufHdr[nIndex],
                        pAppPrivate, nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Output Allocate Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidDec->sOutPortDef.bPopulated, (WORD32)pVidDec->sOutBufList.nAllocSize);
    }

    pthread_mutex_lock(&pVidDec->signal_mutex);
    if((pVidDec->sInPortDef.bPopulated == pVidDec->sInPortDef.bEnabled) &&
                    (pVidDec->sOutPortDef.bPopulated == pVidDec->sOutPortDef.bEnabled))
    {
        pVidDec->bufferallocationpending = 0;
        pVidDec->PortReconfiguration = 0;
        data_sync_barrier();
        pthread_cond_signal(&pVidDec->buffers_signal);
    }
    pthread_mutex_unlock(&pVidDec->signal_mutex);

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_U32 nIndex;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    if(pVidDec->bufferallocationpending)
    {
        pthread_mutex_lock(&pVidDec->signal_mutex);
        pVidDec->state = OMX_StateInvalid;
        pthread_cond_signal(&pVidDec->buffers_signal);
        pthread_mutex_unlock(&pVidDec->signal_mutex);
    }
    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
    {
        pPortDef = &pVidDec->sInPortDef;
        ListFreeBuffer(pVidDec->sInBufList, pBufferHdr, pPortDef, nIndex)
        ITTIAM_DEBUG("Buffer freed Port index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pPortDef->bPopulated, (WORD32)pVidDec->sInBufList.nAllocSize);
    }
    else if (nPortIndex == pVidDec->sOutPortDef.nPortIndex)
    {
        pPortDef = &pVidDec->sOutPortDef;
        ListFreeBuffer(pVidDec->sOutBufList, pBufferHdr, pPortDef, nIndex)
        ITTIAM_DEBUG("Buffer freed Port index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pPortDef->bPopulated, (WORD32)pVidDec->sOutBufList.nAllocSize);
    }
    else
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    if (pPortDef->bEnabled && pVidDec->state != OMX_StateIdle && pVidDec->state != OMX_StateInvalid && pVidDec->state != OMX_StateExecuting)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    pthread_mutex_lock(&pVidDec->signal_mutex);
    if (!pVidDec->sInBufList.nAllocSize && !pVidDec->sOutBufList.nAllocSize)
    {
        pthread_cond_signal(&pVidDec->buffers_signal);
    }
    if (pVidDec->PortReconfiguration && !pVidDec->sOutBufList.nAllocSize)
    {
        pthread_cond_signal(&pVidDec->buffers_signal);
    }
    pthread_mutex_unlock(&pVidDec->signal_mutex);

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDDECTYPE *pVidDec;
    ThrCmdType eCmd = EmptyBuf;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    if (!pVidDec->sInPortDef.bEnabled)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pBufferHdr->nInputPortIndex != 0x0 || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);

    if (pVidDec->state != OMX_StateExecuting && pVidDec->state != OMX_StatePause)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidDec->pipes_mutex);
    pVidDec->NumETB++;
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidDec->cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));
    data_sync_barrier();
    pthread_mutex_unlock(&pVidDec->pipes_mutex);

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    VIDDECTYPE *pVidDec;
    ThrCmdType eCmd = FillBuf;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
#if OUTPUT_BUFFER_LOG
   int y_size = 0;
   int c_offset = 0;
   unsigned char *buf_addr = NULL;

   y_size =  1280*720;
   c_offset= (y_size + 4095) & (~(4095));
#endif

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);
#if OUTPUT_BUFFER_LOG
   buf_addr = (unsigned char *)pBufferHdr->pBuffer;
#endif

    if (!pVidDec->sOutPortDef.bEnabled)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);

    if (pVidDec->state != OMX_StateExecuting && pVidDec->state != OMX_StatePause)
    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    //ITTIAM_DEBUG("FTB Buffer %x pBufferHdr %x", (WORD32)pBufferHdr->pBuffer + pVidDec->offset, (WORD32)pBufferHdr);
    if(1 == pVidDec->share_disp_buf)
    {
        pthread_mutex_lock(&pVidDec->codec_mutex);

        pBufferHdr->nFlags = pBufferHdr->nFlags | CUSTOM_BUFFERFLAG_OWNED;
        //ITTIAM_DEBUG("pVidDec->sInPortFormat.eCompressionFormat %x", pVidDec->sInPortFormat.eCompressionFormat);

        //if((pVidDec->hdr_decode_done == 1) && ((pVidDec->wait_for_op_buffers != 1) || (pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)))

        if(pVidDec->hdr_decode_done == 1)
        {
            {
                UWORD32 i;
                for(i = 0; i < NUM_OUT_BUFFERS; i++)
                {
                    if((pBufferHdr->pBuffer/* + pVidDec->offset*/) == pVidDec->disp_buf_id_mapping[i])
                    {
                        ITTIAM_DEBUG("Calling ittiam_video_release_display_frame %d", i);
                        ittiam_video_release_display_frame(pVidDec, i);

                        break;
                    }
                }
            }

            pthread_cond_signal(&pVidDec->codec_signal);
        }

        pthread_mutex_unlock(&pVidDec->codec_mutex);
    }
    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidDec->pipes_mutex);
    pVidDec->NumFTB++;
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidDec->cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));
#if OUTPUT_BUFFER_LOG

    if(outputBufferFile1)
   {
      fwrite((const char *)buf_addr, y_size, 1, outputBufferFile1);
      fwrite((const char *)(buf_addr + c_offset), (y_size>>1), 1, outputBufferFile1);
   }
#endif

    data_sync_barrier();
    pthread_mutex_unlock(&pVidDec->pipes_mutex);

    OMX_CONF_CMD_BAIL:
    return eError;
}

/*****************************************************************************/
OMX_ERRORTYPE ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType eCmd = StopThread;
    OMX_U32 nIndex = 0;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    ITTIAM_DEBUG("pVidDec->sInBufList.nAllocSize = %d",(WORD32)pVidDec->sInBufList.nAllocSize );
    if (pVidDec->sInBufList.nAllocSize > 0)
    ListFreeAllBuffers(pVidDec->sInBufList, nIndex)
    ITTIAM_DEBUG("pVidDec->sOutBufList.nAllocSize = %d",(WORD32)pVidDec->sOutBufList.nAllocSize );
    if (pVidDec->sOutBufList.nAllocSize > 0)
    ListFreeAllBuffers(pVidDec->sOutBufList, nIndex)

    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidDec->pipes_mutex);
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidDec->cmddatapipe[1], &eCmd, sizeof(eCmd));
    pthread_mutex_unlock(&pVidDec->pipes_mutex);

    pVidDec->exitPending = 1;
    // Wait for thread to exit so we can get the status into "error"
    pthread_join(pVidDec->thread_id, (void*)&eError);

    // close the pipe handles
    close(pVidDec->cmdpipe[0]);
    close(pVidDec->cmdpipe[1]);
    close(pVidDec->cmddatapipe[0]);
    close(pVidDec->cmddatapipe[1]);

    pthread_mutex_destroy(&pVidDec->pipes_mutex);
    pthread_mutex_destroy(&pVidDec->signal_mutex);
    pthread_mutex_destroy(&pVidDec->codec_mutex);

    pthread_cond_destroy(&pVidDec->buffers_signal);
    pthread_cond_destroy(&pVidDec->codec_signal);

#if OUTPUT_BUFFER_LOG
  fclose (outputBufferFile1);
#endif

    OMX_OSAL_Free(pVidDec);
    ITTIAM_LOG("Decoder instance deleted");
    return eError;
}

char * convert_vidformat_to_mime(OMX_VIDEO_CODINGTYPE VidFormat)
{
    if(VidFormat == OMX_VIDEO_CodingMPEG4)
        return "MPEG4";
    else if(VidFormat == OMX_VIDEO_CodingAVC)
        return "H264";
    else if(VidFormat == OMX_VIDEO_CodingHEVC)
        return "HEVC";
    else if(VidFormat == OMX_VIDEO_CodingH263)
        return "H263";
    else if(VidFormat == (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx)
        return "DIVX";
    else if(VidFormat == OMX_VIDEO_CodingMPEG2)
        return "MPEG2";
    else
        return NULL;
}

OMX_ERRORTYPE ComponentRoleEnum(
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_U8 *cRole,
                OMX_IN OMX_U32 nIndex)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(nIndex == 0)
    {
        if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingMPEG4)
        {
            strlcpy((char *)cRole, "video_decoder.mpeg4",OMX_MAX_STRINGNAME_SIZE);
        }
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingAVC)
        {
            strlcpy((char *)cRole, "video_decoder.avc",OMX_MAX_STRINGNAME_SIZE);
        }
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingHEVC)
        {
            strlcpy((char *)cRole, "video_decoder.hevc",OMX_MAX_STRINGNAME_SIZE);
        }
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingH263)
        {
            strlcpy((char *)cRole, "video_decoder.h263",OMX_MAX_STRINGNAME_SIZE);
        }
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx)
        {
            strlcpy((char *)cRole, "video_decoder.divx",OMX_MAX_STRINGNAME_SIZE);
        }
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingMPEG2)
        {
            strlcpy((char *)cRole, "video_decoder.mpeg2",OMX_MAX_STRINGNAME_SIZE);
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
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_INDEXTYPE nIndex,
                OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CONFIG_RECTTYPE *rect;

    ITTIAM_DEBUG("GetConfig hComponent = %x nIndex = %x ComponentParameterStructure = %x",
                    (WORD32)hComponent, (WORD32)nIndex, (WORD32)pComponentConfigStructure);
    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pComponentConfigStructure, 1);

    if(pVidDec->state == OMX_StateInvalid)
    {
        ITTIAM_DEBUG("GetConfig in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch(nIndex)
    {
        case OMX_IndexConfigCommonOutputCrop:
        rect = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
        if (rect->nPortIndex == 1)
        {
            rect->nLeft = pVidDec->width_padding/2;
            rect->nTop = pVidDec->y_height_padding/2;
               rect->nWidth = pVidDec->sCropRect.nWidth;
               rect->nHeight = pVidDec->sCropRect.nHeight;
        }
        else
        {
            ITTIAM_DEBUG("get_config: Bad port index %d queried on only o/p port\n",
                            (WORD32)rect->nPortIndex);
            eError = OMX_ErrorBadPortIndex;
        }
        break;

        default:
        eError = OMX_ErrorUnsupportedIndex;
        break;
    }

    OMX_CONF_CMD_BAIL:
    return eError;

}

OMX_ERRORTYPE SetConfig(
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_INDEXTYPE nIndex,
                OMX_IN OMX_PTR pComponentConfigStructure)
{
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 LC_level;

    ITTIAM_DEBUG("SetConfig hComponent = %x nIndex = %x ComponentParameterStructure = %x",
                    (WORD32)hComponent,(WORD32) nIndex, (WORD32)pComponentConfigStructure);

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pComponentConfigStructure, 1);

    switch ((WORD32)nIndex)
    {
        case CUSTOM_OMX_IndexConfigLCLevel:

            LC_level = *((OMX_U32 *)pComponentConfigStructure);
#ifdef DISABLE_LOW_COMPLEXITY
        LC_level = 0;
#endif
        /* If low complexity mode is not supported,
         then ignore this SetConfig */
        if(LC_level > pVidDec->low_complexity_level_max)
        {
            LC_level = pVidDec->low_complexity_level_max;
        }
        ITTIAM_DEBUG("LC_level = %d",(WORD32)LC_level);

        /* In case of ICS, ignore level 3, since in ICS seek to I is done by default,
         if the delay is too high */
//#ifdef ICS
        if(LC_level > 2)
            LC_level = 2;
//#endif
        if(pVidDec->LowComplexity == 1)
        {
            pthread_mutex_lock(&pVidDec->codec_mutex);
            if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingMPEG4)
            {
                if(LC_level == 3)
                {
                    pVidDec->seek_to_I_frame = 1;
                    video_skip_pb_frames(pVidDec);
                }
                else
                {
                    if(LC_level == 2)
                    {
                        ITTIAM_DEBUG("MPEG4 Low complexity mode level 2");
                        video_skipb_frames(pVidDec);
                        mpeg4_disable_qpel(pVidDec);

                    }
                    else if(LC_level == 1)
                    {
                        ITTIAM_DEBUG("MPEG4 Low complexity mode level 1");
                        video_skip_none(pVidDec);
                        mpeg4_disable_qpel(pVidDec);

                    }
                    else if(LC_level == 0)
                    {
                        ITTIAM_DEBUG("MPEG4 Normal mode");
                        video_skip_none(pVidDec);
                        mpeg4_enable_qpel(pVidDec);
                    }
                }
            }
            else if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)
            {
                ITTIAM_DEBUG("H264Low complexity mode level %d", (WORD32)LC_level);
                if(LC_level == 3)
                {
                    pVidDec->seek_to_I_frame = 1;
                    video_skip_pb_frames(pVidDec);
                }
                else
                {
                    avc_set_deblocking_lvl(pVidDec, LC_level);
                    if(LC_level >= 1)
                    {
                        video_skipb_frames(pVidDec);
                    }
                    else
                    {
                        video_skip_none(pVidDec);
                    }
                }

            }
            else if(pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)
            {
                ITTIAM_DEBUG("HEVCLow complexity mode level %d : unsupported", (WORD32)LC_level);
            }
            pthread_mutex_unlock(&pVidDec->codec_mutex);
        }
        break;

        case CUSTOM_OMX_ThumbnailMode:
            ITTIAM_LOG("Thumbnail mode information recieved");
            pVidDec->ThumbnailMode = 1;
            pVidDec->share_disp_buf = 0;
            pVidDec->sOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
            pVidDec->sOutPortFormat.eColorFormat = OMX_COLOR_FormatYUV420Planar;

            break;
        default:
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

    OMX_CONF_CMD_BAIL:
    return eError;
}

OMX_ERRORTYPE GetExtensionIndex(
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    VIDDECTYPE *pVidDec;

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if(pVidDec->state == OMX_StateInvalid)
    {
        ITTIAM_DEBUG("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.getAndroidNativeBufferUsage", sizeof("OMX.google.android.index.getAndroidNativeBufferUsage") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.useAndroidNativeBuffer2", sizeof("OMX.google.android.index.useAndroidNativeBuffer2") - 1))
    {
        *pIndexType = OMX_ErrorNotImplemented;
        return OMX_ErrorNotImplemented;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.useAndroidNativeBuffer", sizeof("OMX.google.android.index.useAndroidNativeBuffer") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexUseAndroidNativeBuffer;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.enableAndroidNativeBuffers", sizeof("OMX.google.android.index.enableAndroidNativeBuffers") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeBuffer;
    }
    else if(!strncmp(cParameterName,"OMX.ITTIAM.index.LClevel", sizeof("OMX.ITTIAM.index.LClevel") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)CUSTOM_OMX_IndexConfigLCLevel;
    }
    else if(!strncmp(cParameterName,"OMX.QCOM.index.param.video.SyncFrameDecodingMode", sizeof("OMX.QCOM.index.param.video.SyncFrameDecodingMode") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)CUSTOM_OMX_ThumbnailMode;
    }
    else if(!strncmp(cParameterName,"OMX.ITTIAM.ThumbnailMode", sizeof("OMX.ITTIAM.ThumbnailMode") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)CUSTOM_OMX_ThumbnailMode;
    }
    else
    {
        ITTIAM_DEBUG("Extension: %s not implemented\n", cParameterName);
        return OMX_ErrorNotImplemented;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE StubbedComponentTunnelRequest(
                OMX_IN OMX_HANDLETYPE hComp,
                OMX_IN OMX_U32 nPort,
                OMX_IN OMX_HANDLETYPE hTunneledComp,
                OMX_IN OMX_U32 nTunneledPort,
                OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE StubbedGetComponentVersion(
                OMX_IN OMX_HANDLETYPE hComponent,
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
OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_VIDEO_CODINGTYPE VidFormat,
                OMX_IN OMX_COLOR_FORMATTYPE ColorFormat,
                OMX_IN OMX_U32 max_width,
                OMX_IN OMX_U32 max_height,
                OMX_IN OMX_U32 share_disp_buf,
                OMX_IN OMX_U32 num_cores,
                OMX_IN OMX_U32 low_complexity_level_max,
                OMX_IN OMX_U32 disable_interlaced,
                OMX_IN OMX_U32 minUndequeuedBufs,
                OMX_IN OMX_U32 IsQcomCore,
                OMX_IN PROCESSORTYPE processor_type)
{
    OMX_COMPONENTTYPE *pComp;
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 err;
    OMX_U32 nIndex;

    pComp = (OMX_COMPONENTTYPE *)hComponent;

    ITTIAM_LOG("Ittiam OMX Video Decoder Colorformat in CI() %d",ColorFormat);
    // Create private data
    pVidDec = (VIDDECTYPE *)OMX_OSAL_Malloc(sizeof(VIDDECTYPE));
    if(pVidDec == NULL)
    {
          ITTIAM_DEBUG("Memory allocation failed for VidDec in ComponentInit( ) \n");
          return OMX_ErrorInsufficientResources;
    }
    memset(pVidDec, 0x0, sizeof(VIDDECTYPE));

    pVidDec->share_disp_buf = share_disp_buf;
    pVidDec->num_cores = num_cores;
    pVidDec->disable_interlaced = disable_interlaced;
    pVidDec->low_complexity_level_max = low_complexity_level_max;
    pVidDec->max_width = max_width;
    pVidDec->max_height = max_height;
    pVidDec->minUndequeuedBufs = minUndequeuedBufs;
    ITTIAM_LOG("Ittiam OMX Video Decoder Init Version %s",ComponentVersion);

    pComp->pComponentPrivate = (OMX_PTR)pVidDec;
    pVidDec->state = OMX_StateLoaded;
    pVidDec->hSelf = hComponent;
    pVidDec->initdone = 0;
    pVidDec->ThumbnailMode = 0;

    pVidDec->mProcessorType = processor_type;
    pVidDec->IsQcomCore = IsQcomCore;
    pVidDec->PortReconfiguration = 0;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );

    // Fill in function pointers
    pComp->SetCallbacks = SetCallbacks;
    pComp->GetComponentVersion = StubbedGetComponentVersion;
    pComp->SendCommand = SendCommand;
    pComp->GetParameter = GetParameter;
    pComp->SetParameter = SetParameter;
    pComp->GetConfig = GetConfig;
    pComp->SetConfig = SetConfig;
    pComp->GetExtensionIndex = GetExtensionIndex;
    pComp->GetState = GetState;
    pComp->ComponentTunnelRequest = StubbedComponentTunnelRequest;
    pComp->UseBuffer = UseBuffer;
    pComp->AllocateBuffer = AllocateBuffer;
    pComp->FreeBuffer = FreeBuffer;
    pComp->EmptyThisBuffer = EmptyThisBuffer;
    pComp->FillThisBuffer = FillThisBuffer;
    pComp->ComponentDeInit = ComponentDeInit;
    pComp->UseEGLImage = StubbedUseEGLImage;
    pComp->ComponentRoleEnum = ComponentRoleEnum;

#if OUTPUT_BUFFER_LOG
     outputBufferFile1 = fopen (outputfilename, "ab");
#endif

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sPortParam, OMX_PORT_PARAM_TYPE);
    pVidDec->sPortParam.nPorts = 0x2;
    pVidDec->sPortParam.nStartPortNumber = 0x0;
    pVidDec->bufferallocationpending = 0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    pVidDec->sInPortDef.nPortIndex = 0x0;
    pVidDec->sInPortDef.bEnabled = OMX_TRUE;
    pVidDec->sInPortDef.bPopulated = OMX_FALSE;
    pVidDec->sInPortDef.eDomain = OMX_PortDomainVideo;

    pVidDec->sInPortDef.format.video.nFrameWidth = pVidDec->max_width;
    pVidDec->sInPortDef.format.video.nFrameHeight = pVidDec->max_height;
    pVidDec->sInPortDef.eDir = OMX_DirInput;
    pVidDec->sInPortDef.nBufferCountMin = NUM_IN_BUFFERS;
    pVidDec->sInPortDef.nBufferCountActual = NUM_IN_BUFFERS;
    pVidDec->sInPortDef.nBufferSize = (OMX_U32)(1024*1024);// JK TBD
    if(IsQcomCore)
    {
        pVidDec->sInPortDef.format.video.cMIMEType = convert_vidformat_to_mime(VidFormat);
        pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
    }
    pVidDec->sInPortDef.format.video.eColorFormat = OMX_VIDEO_CodingUnused;

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    pVidDec->sOutPortDef.nPortIndex = 0x1;
    pVidDec->sOutPortDef.bEnabled = OMX_TRUE;
    pVidDec->sOutPortDef.bPopulated = OMX_FALSE;
    pVidDec->sOutPortDef.eDomain = OMX_PortDomainVideo;
    pVidDec->sOutPortDef.format.video.cMIMEType = "YUV420";
    pVidDec->sOutPortDef.format.video.nFrameWidth = pVidDec->max_width;
    pVidDec->sOutPortDef.format.video.nFrameHeight = pVidDec->max_height;
    pVidDec->sOutPortDef.format.video.nStride = pVidDec->max_width;
    pVidDec->sOutPortDef.format.video.nSliceHeight = pVidDec->max_height;
    pVidDec->sOutPortDef.eDir = OMX_DirOutput;
    pVidDec->sOutPortDef.nBufferCountMin = NUM_OUT_BUFFERS;
    pVidDec->sOutPortDef.nBufferCountActual = NUM_OUT_BUFFERS;

    pVidDec->sOutPortDef.format.video.eColorFormat = ColorFormat;

    ITTIAM_LOG("Ittiam OMX Video Decoder Colorformat in CI()2 %d",ColorFormat);

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sInPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pVidDec->sInPortFormat.nPortIndex = 0x0;
    pVidDec->sInPortFormat.nIndex = 0x0;
    if(IsQcomCore)
    {
        pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
    }
    pVidDec->sInPortFormat.eColorFormat = OMX_VIDEO_CodingUnused;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sOutPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pVidDec->sOutPortFormat.nPortIndex = 0x1;
    pVidDec->sOutPortFormat.nIndex = 0x0;
    pVidDec->sOutPortFormat.eColorFormat = ColorFormat;

    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sPriorityMgmt, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sInBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pVidDec->sInBufSupplier.nPortIndex = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sOutBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    pVidDec->sOutBufSupplier.nPortIndex = 0x1;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );

    if(IsQcomCore)
    {
        if(VidFormat == OMX_VIDEO_CodingMPEG4)
        {
            OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sMpeg4, OMX_VIDEO_PARAM_MPEG4TYPE);
            pVidDec->sMpeg4.nPortIndex = 0x0;
            // In MPEG4 padding is 0
        }
        else if(VidFormat == OMX_VIDEO_CodingAVC)
        {
            OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sH264, OMX_VIDEO_PARAM_AVCTYPE);
            pVidDec->sH264.nPortIndex = 0x0;
            if(1 == pVidDec->share_disp_buf)
            {
                pVidDec->width_padding = AVC_WIDTH_PADDING;
                pVidDec->height_padding = AVC_HEIGHT_PADDING;
                pVidDec->y_height_padding = AVC_Y_HEIGHT_PADDING;
                pVidDec->uv_height_padding = AVC_UV_HEIGHT_PADDING;
            }
            else
            {
                pVidDec->width_padding = 0;
                pVidDec->height_padding = 0;
                pVidDec->y_height_padding = 0;
                pVidDec->uv_height_padding = 0;
            }
        }
        else if(VidFormat == OMX_VIDEO_CodingHEVC)
        {
            ITTIAM_LOG("This is OMX_VIDEO_CodingHEVC");
            if(1 == pVidDec->share_disp_buf)
            {
                pVidDec->width_padding = HEVC_WIDTH_PADDING;
                pVidDec->height_padding = HEVC_HEIGHT_PADDING;
                pVidDec->y_height_padding = HEVC_Y_HEIGHT_PADDING;
                pVidDec->uv_height_padding = HEVC_UV_HEIGHT_PADDING;
            }
            else
            {
                pVidDec->width_padding = 0;
                pVidDec->height_padding = 0;
                pVidDec->y_height_padding = 0;
                pVidDec->uv_height_padding = 0;
            }
        }
        else if(VidFormat == OMX_VIDEO_CodingH263)
        {
            ITTIAM_LOG("This is H263");
        }
        else if(VidFormat == (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx)
        {
            ITTIAM_LOG("This is Divx");
        }
        else if(VidFormat == OMX_VIDEO_CodingMPEG2)
        {
            ITTIAM_LOG("This is MPEG2");
        }
        else
        {
            ITTIAM_ERROR("Unknown Codec");
            return OMX_ErrorUndefined;
        }

            ITTIAM_ERROR("Color Format %8x", ColorFormat);
        if((ColorFormat != (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar) &&
            (ColorFormat != (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar) &&
            (ColorFormat != (OMX_COLOR_FORMATTYPE)CUSTOM_COLOR_FormatYUV420SemiPlanar) &&
            (ColorFormat != (OMX_COLOR_FORMATTYPE)CUSTOM_COLOR_FormatYVU420SemiPlanar))
        {
            ITTIAM_ERROR("Unknown Color Format %8x", ColorFormat);
            return OMX_ErrorUndefined;
        }
    }
    if((VidFormat == OMX_VIDEO_CodingMPEG4) &&
      ((pVidDec->sOutPortDef.format.video.nFrameWidth * pVidDec->sOutPortDef.format.video.nFrameHeight ) >= (1920*1080))&&
      (pVidDec->num_cores==2))
    {
        char propertyValue[64] = {0};
        int mpeg4Reject1080p;
        propertyValue[0] = '\0';
        property_get("ittiam.vdec.mpeg4.1080p.reject", propertyValue, "0");
        mpeg4Reject1080p = atoi(propertyValue);
        ITTIAM_LOG("mpeg4 1080p reject property value is %d",mpeg4Reject1080p);
        if(mpeg4Reject1080p)
        {
           ITTIAM_ERROR("Rejected MPEG4,1080p for 8x10 1.0");
           eError = OMX_ErrorUndefined;
           goto EXIT;
        }
    }

    pVidDec->sOutPortDef.nBufferSize = (pVidDec->sOutPortDef.format.video.nFrameWidth + pVidDec->width_padding) *
    ((pVidDec->sOutPortDef.format.video.nFrameHeight + pVidDec->height_padding) +
                    (pVidDec->sOutPortDef.format.video.nFrameHeight / 2 + pVidDec->height_padding / 2));
    // Initialize the input buffer list
    memset(&(pVidDec->sInBufList), 0x0, sizeof(BufferList));

    pVidDec->sInBufList.pBufHdr = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sInPortDef.nBufferCountActual);
    if(pVidDec->sInBufList.pBufHdr == NULL)
    {
          ITTIAM_DEBUG("Unable to allocate memory for sInBufList of decoder\n");
          return OMX_ErrorInsufficientResources;
    }
    for (nIndex = 0; nIndex < pVidDec->sInPortDef.nBufferCountActual; nIndex++)
    {
        pVidDec->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
        OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        OMX_CONF_INIT_STRUCT_PTR (pVidDec->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    pVidDec->sInBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sInPortDef.nBufferCountActual);

    pVidDec->sInBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                    pVidDec->sInPortDef.nBufferCountActual);
    pVidDec->sInBufList.nSizeOfList = 0;
    pVidDec->sInBufList.nAllocSize = 0;
    pVidDec->sInBufList.nListEnd = -1;
    pVidDec->sInBufList.nWritePos = -1;
    pVidDec->sInBufList.nReadPos = -1;
    pVidDec->sInBufList.eDir = OMX_DirInput;
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );

    // Initialize the output buffer list
    memset(&(pVidDec->sOutBufList), 0x0, sizeof(BufferList));
    pVidDec->sOutBufList.pBufHdr = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sOutPortDef.nBufferCountActual);
    if(pVidDec->sOutBufList.pBufHdr == NULL)
    {
          ITTIAM_DEBUG("Unable to allocate memory for OutBufList of decoder \n");
          return OMX_ErrorInsufficientResources;
    }

    for (nIndex = 0; nIndex < pVidDec->sOutPortDef.nBufferCountActual; nIndex++)
    {
        pVidDec->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
        OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        OMX_CONF_INIT_STRUCT_PTR (pVidDec->sOutBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
    }
    pVidDec->sOutBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sOutPortDef.nBufferCountActual);

    pVidDec->sOutBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                    pVidDec->sOutPortDef.nBufferCountActual);
    pVidDec->sOutBufList.nSizeOfList = 0;
    pVidDec->sOutBufList.nAllocSize = 0;
    pVidDec->sOutBufList.nListEnd = -1;
    pVidDec->sOutBufList.nWritePos = -1;
    pVidDec->sOutBufList.nReadPos = -1;
    pVidDec->sOutBufList.eDir = OMX_DirOutput;

    pVidDec->LowComplexity = 1;
    pVidDec->NumETB = 0;
    pVidDec->NumEBD = 0;
    pVidDec->NumFTB = 0;
    pVidDec->NumFBD = 0;

    // Create the pipe used to send commands to the thread
    err = pipe(pVidDec->cmdpipe);
    if (err)
    {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the pipe used to send command data to the thread
    err = pipe(pVidDec->cmddatapipe);
    if (err)
    {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // One pipe is thread safe. But since two pipes are used here,
    // a mutex lock is required to prevent any race condition

    pthread_mutex_init(&pVidDec->pipes_mutex, NULL);
    pthread_mutex_init(&pVidDec->signal_mutex, NULL);
    pthread_mutex_init(&pVidDec->codec_mutex, NULL);

    pthread_cond_init(&pVidDec->buffers_signal, NULL);
    pthread_cond_init(&pVidDec->codec_signal, NULL);

    // Create the component thread
    err = pthread_create(&pVidDec->thread_id, NULL, ComponentThread, pVidDec);
    if( err || !pVidDec->thread_id )
    {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    if(IsQcomCore)
    {
        // Ittiam specific initializations
        switch((WORD32)VidFormat)
        {
#ifdef ENABLE_MPEG4
            case OMX_VIDEO_CodingMPEG4:
            case OMX_VIDEO_CodingH263:
            case QOMX_VIDEO_CodingDivx:
            {
                pVidDec->iVdec_cxa8_api_function = imp4d_cxa8_api_function;
                break;
            }
#endif
#ifdef ENABLE_MPEG2
            case OMX_VIDEO_CodingMPEG2:
            {
                pVidDec->iVdec_cxa8_api_function = imp2d_cxa8_api_function;
                break;
            }
#endif
#ifdef ENABLE_H264
            case OMX_VIDEO_CodingAVC:
            {
                pVidDec->iVdec_cxa8_api_function = ih264d_cxa8_api_function;
                break;
            }
#endif
#ifdef ENABLE_HEVC
            case OMX_VIDEO_CodingHEVC:
            {
                pVidDec->iVdec_cxa8_api_function = ihevcd_cxa_api_function;
                break;
            }
#endif
            default:
            ITTIAM_DEBUG("%s Unsupported VidFormat = %x ", __FUNCTION__, VidFormat );
            eError = OMX_ErrorUndefined;
            goto EXIT;
            break;
        }
    }
    EXIT:
    ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );
    return eError;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
