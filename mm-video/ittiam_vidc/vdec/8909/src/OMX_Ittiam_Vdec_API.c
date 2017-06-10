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
#include <OMX_VideoExt.h>
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
#include "imp4d_cxa8.h"


#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include <system/graphics.h>
#include<utils/Log.h>

#define ComponentVersion   COMP_VERSION


//#define SUSPEND_RESUME_HACK


void ittiam_video_release_display_frame(VIDDECTYPE* pVidDec,
                                        UWORD32 disp_buf_id);
void video_skipb_frames(VIDDECTYPE* pVidDec);
void video_skip_none(VIDDECTYPE* pVidDec);
void video_skip_pb_frames(VIDDECTYPE* pVidDec);
void avc_set_degrade(VIDDECTYPE* pVidDec, UWORD32 type, UWORD32 pics);
void mpeg4_enable_qpel(VIDDECTYPE* pVidDec);
void mpeg4_disable_qpel(VIDDECTYPE* pVidDec);

/*
 *     F U N C T I O N S
 */

/*****************************************************************************/
static const UWORD32 gMPEG4Profiles[] =
{
    OMX_VIDEO_MPEG4ProfileSimple,
    OMX_VIDEO_MPEG4ProfileAdvancedSimple
};

static const UWORD32 gMPEG4Levels[] =
{
    OMX_VIDEO_MPEG4Level0,
    OMX_VIDEO_MPEG4Level0b,
    OMX_VIDEO_MPEG4Level1,
    OMX_VIDEO_MPEG4Level2,
    OMX_VIDEO_MPEG4Level3,
    OMX_VIDEO_MPEG4Level4,
    OMX_VIDEO_MPEG4Level4a,
    OMX_VIDEO_MPEG4Level5,
};


static const UWORD32 gNumMPEG4Profiles = sizeof(gMPEG4Profiles)/sizeof(gMPEG4Profiles[0]);
static const UWORD32 gNumMPEG4Levels = sizeof(gMPEG4Levels)/sizeof(gMPEG4Levels[0]);


static const UWORD32 gH263Profiles[] =
{
    OMX_VIDEO_H263ProfileBaseline,
};

static const UWORD32 gH263Levels[] =
{
     OMX_VIDEO_H263Level10 ,
     OMX_VIDEO_H263Level20 ,
     OMX_VIDEO_H263Level30 ,
     OMX_VIDEO_H263Level40 ,
     OMX_VIDEO_H263Level45 ,
};


static const UWORD32 gNumH263Profiles = sizeof(gH263Profiles)/sizeof(gH263Profiles[0]);
static const UWORD32 gNumH263Levels = sizeof(gH263Levels)/sizeof(gH263Levels[0]);





static WORD32 get_num_disp_buffers(UWORD32 width, UWORD32 height, OMX_VIDEO_CODINGTYPE codec)
{

    UWORD32 i4_size = 0;
    UWORD32 num_disp_bufs;

    if(codec == OMX_VIDEO_CodingAVC)
    {
        UWORD32 ui16_frmWidthInMbs = width / 16;
        UWORD32 ui16_frmHeightInMbs = height / 16;
        UWORD32 max_luma_samples;

        if ((width * height) > (1920 * 1088)) {
            /* Level 5.1 */
            max_luma_samples = 9437184;
        } else if ((width * height) > (1280 * 720)) {
            /* Level 4.0 */
            max_luma_samples = 2097152;
        } else if ((width * height) > (720 * 480)) {
            /* Level 3.1 */
            max_luma_samples = 921600;
        } else if ((width * height) > (640 * 360)) {
            /* Level 3.0 */
            max_luma_samples = 414720;
        } else if ((width * height) > (352 * 288)) {
            /* Level 2.1 */
            max_luma_samples = 202752;
        } else {
            /* Level 2.0 */
            max_luma_samples = 101376;
        }

        i4_size = max_luma_samples / (ui16_frmWidthInMbs * (ui16_frmHeightInMbs));

        num_disp_bufs = i4_size / 384;
        num_disp_bufs += (num_disp_bufs + 1);

        num_disp_bufs = MIN(num_disp_bufs, 32);
        return (num_disp_bufs);
    }
    else if(codec == OMX_VIDEO_CodingHEVC)
    {
        UWORD32 max_luma_samples;
        UWORD32 luma_samples;

        if ((width * height) > (1920 * 1088)) {
            /* Level 5.0 */
            max_luma_samples = 8912896;
        } else if ((width * height) > (1280 * 720)) {
            /* Level 4.0 */
            max_luma_samples = 2228224;
        } else if ((width * height) > (960 * 540)) {
            /* Level 3.1 */
            max_luma_samples = 983040;
        } else if ((width * height) > (640 * 360)) {
            /* Level 3.0 */
            max_luma_samples = 552960;
        } else if ((width * height) > (352 * 288)) {
            /* Level 2.1 */
            max_luma_samples = 245760;
        } else {
            /* Level 2.0 */
            max_luma_samples = 122880;
        }

        luma_samples = width * height;
        if(luma_samples <= (max_luma_samples >> 2))
        {
            num_disp_bufs = 16;
        }
        else if(luma_samples <= (max_luma_samples >> 1))
        {
            num_disp_bufs = 12;
        }
        else if(luma_samples <= ((3 * max_luma_samples) >> 2))
        {
            num_disp_bufs = 8;
        }
        else
        {
            num_disp_bufs = 6;
        }
        num_disp_bufs = num_disp_bufs * 2 + 1;
        num_disp_bufs = MIN(num_disp_bufs, 32);
        return (num_disp_bufs);

    }
    return 11;

}


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
        break;
        case OMX_CommandFlush:
        eCmd = Flush;
        pthread_mutex_lock(&pVidDec->pipes_mutex);
        pVidDec->cmd_pending = 1;
        pthread_mutex_unlock(&pVidDec->pipes_mutex);

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
    {
        ITTIAM_DEBUG("Invalid state");
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);
    }

    switch (nParamIndex)
    {
        // Gets OMX_PORT_PARAM_TYPE structure
        case OMX_IndexParamVideoInit:
        ITTIAM_DEBUG("GetParameter OMX_IndexParamVideoInit");
        memcpy(ComponentParameterStructure, &pVidDec->sPortParam, sizeof
                        (OMX_PORT_PARAM_TYPE));
        break;
        // Gets OMX_PARAM_PORTDEFINITIONTYPE structure
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE  *portFmt = (OMX_PARAM_PORTDEFINITIONTYPE *)(ComponentParameterStructure);
            ITTIAM_DEBUG("GetParameter OMX_IndexParamPortDefinition");

            if (portFmt->nPortIndex == pVidDec->sInPortDef.nPortIndex)
            {
                ITTIAM_DEBUG("GetParameter Input port definition Size = %d ", (WORD32)pVidDec->sInPortDef.nBufferSize );

                memcpy(ComponentParameterStructure, &pVidDec->sInPortDef, sizeof
                                (OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (portFmt->nPortIndex == pVidDec->sOutPortDef.nPortIndex)
            {
                ITTIAM_DEBUG("GetParameter Output port definition Size = %d ", (WORD32)pVidDec->sOutPortDef.nBufferSize );
                memcpy(ComponentParameterStructure, &pVidDec->sOutPortDef, sizeof
                            (OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else
            {
                ITTIAM_ERROR("GetParameter OMX_IndexParamPortDefinition Invalid nPortIndex = %d ", (WORD32)portFmt->nPortIndex );
                eError = OMX_ErrorBadPortIndex;
            }
            break;

        }
        // Gets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
        case OMX_IndexParamVideoPortFormat:
        {
            ITTIAM_DEBUG("GetParameter OMX_IndexParamVideoPortFormat");
            OMX_VIDEO_PARAM_PORTFORMATTYPE *portFmt = ((OMX_VIDEO_PARAM_PORTFORMATTYPE *)(ComponentParameterStructure));
            if (portFmt->nPortIndex == pVidDec->sInPortFormat.nPortIndex)
            {
                if (portFmt->nIndex > pVidDec->sInPortFormat.nIndex)
                {
                    ITTIAM_ERROR("Invalid output param->nIndex %d out->nIndex %d",
                                (WORD32)portFmt->nIndex, (WORD32)pVidDec->sInPortFormat.nIndex);
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(portFmt, &pVidDec->sInPortFormat, sizeof
                                (OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else if (portFmt->nPortIndex == pVidDec->sOutPortFormat.nPortIndex)
            {
                if (portFmt->nIndex >= MAX_COLOR_FMTS)
                {
                    ITTIAM_ERROR("GetParameter OMX_IndexParamVideoPortFormat Invalid portFmt->nIndex %d out->nIndex %d",
                                 (WORD32)portFmt->nIndex, (WORD32)pVidDec->sOutPortFormat.nIndex);
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    switch(portFmt->nIndex)
                    {
                        case 0:
#ifdef USE_YUV420SP
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420SemiPlanar;
#else
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420Planar;
#endif
                            break;
                        case 2:
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m;
                            break;
                        case 3:
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_QCOM_COLOR_FormatYVU420SemiPlanar;
                            break;
                        case 4:
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
                            break;
                        case 5:
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Flexible;
                            break;
                        case 1:
                        default:
                            pVidDec->sOutPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                            break;
                    }

                    pVidDec->sOutPortFormat.nIndex = portFmt->nIndex;
                    pVidDec->sOutPortFormat.eColorFormat = pVidDec->sOutPortDef.format.video.eColorFormat;
                    ITTIAM_DEBUG("GetParameter OMX_IndexParamVideoPortFormat output eColorFormat 0x%x", pVidDec->sOutPortDef.format.video.eColorFormat);
                    memcpy(ComponentParameterStructure, &pVidDec->sOutPortFormat, sizeof
                                (OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                ITTIAM_ERROR("Invalid nPortIndex %d", (WORD32)portFmt->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }


           break;
        }

        // Gets OMX_PRIORITYMGMTTYPE structure
        case OMX_IndexParamPriorityMgmt:
        ITTIAM_DEBUG("GetParameter OMX_IndexParamPriorityMgmt");
        memcpy(ComponentParameterStructure, &pVidDec->sPriorityMgmt, sizeof
                        (OMX_PRIORITYMGMTTYPE));
        break;


        case OMX_IndexParamStandardComponentRole:
        ITTIAM_DEBUG("GetParameter OMX_IndexParamStandardComponentRole");
        memcpy(ComponentParameterStructure, &pVidDec->componentRole, sizeof(OMX_PARAM_COMPONENTROLETYPE));
        break;

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel =
                  (OMX_VIDEO_PARAM_PROFILELEVELTYPE *) ComponentParameterStructure;
            WORD32 idx;

            if (profileLevel->nPortIndex != pVidDec->sInPortDef.nPortIndex)
            {
                ITTIAM_ERROR("Invalid port index: %d", (WORD32)profileLevel->nPortIndex);
                return OMX_ErrorUnsupportedIndex;
            }

            if (profileLevel->nProfileIndex >= pVidDec->numProfiles * pVidDec->numLevels)
            {
                ITTIAM_ERROR("Invalid profile index: %d", (WORD32)profileLevel->nProfileIndex);
                return OMX_ErrorNoMore;
            }
            idx = profileLevel->nProfileIndex / pVidDec->numLevels;
            profileLevel->eProfile = pVidDec->pProfiles[idx];

            idx = profileLevel->nProfileIndex % pVidDec->numLevels;
            profileLevel->eLevel   = pVidDec->pLevels[idx];
            return OMX_ErrorNone;
        }

        default:
        ITTIAM_ERROR("GetParameter Unsupported Index 0x%x", nParamIndex);
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

    if (pVidDec->state == OMX_StateInvalid)
    {
        ITTIAM_DEBUG("SetParameter Invalid state = %x ", pVidDec->state );
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);
    }

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
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE * portFmt =(OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
            if (portFmt->nPortIndex == pVidDec->sInPortDef.nPortIndex)
            {
                memcpy(&pVidDec->sInPortDef, ComponentParameterStructure, sizeof
                                (OMX_PARAM_PORTDEFINITIONTYPE));
                ITTIAM_DEBUG("SetParameter Input port definition Size = %d ", (WORD32)pVidDec->sInPortDef.nBufferSize );
            }
            else if (portFmt->nPortIndex == pVidDec->sOutPortDef.nPortIndex)
            {
                OMX_BOOL bEnabled;
                bEnabled = pVidDec->sOutPortDef.bEnabled;
                memcpy(&pVidDec->sOutPortDef, ComponentParameterStructure, sizeof
                                (OMX_PARAM_PORTDEFINITIONTYPE));
                pVidDec->sOutPortDef.bEnabled = bEnabled;

                if(0 == pVidDec->nDispWidth || 0 == pVidDec->nDispHeight)
                {
                    pVidDec->nDispWidth = pVidDec->sOutPortDef.format.video.nFrameWidth;
                    pVidDec->nDispHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
                    /* If initWidth and initHeight are initialized for adaptive playback,
                       with max width and max height, then do not initialize here */
                    if(0 == pVidDec->initWidth)
                        pVidDec->initWidth = pVidDec->sOutPortDef.format.video.nFrameWidth;
                    if(0 == pVidDec->initHeight)
                        pVidDec->initHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
                    UWORD32 VideoArea = pVidDec->nDispWidth * pVidDec->nDispHeight;

                    if( (pVidDec->nDispWidth < MIN_WIDTH) || (pVidDec->nDispHeight < MIN_HEIGHT)
                                    || (VideoArea > (pVidDec->maxWidth * pVidDec->maxHeight)) )
                    {
                        ITTIAM_ERROR("Unsupported resolution width %d: height %d",
                                        (WORD32)pVidDec->nDispWidth,
                                        (WORD32)pVidDec->nDispHeight);
                        eError = OMX_ErrorUnsupportedSetting;
                        return eError;
                    }

                    if((1 == pVidDec->shareDispBuf) && ((pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) || (pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)))
                    {
                        pVidDec->sOutPortDef.nBufferCountMin = get_num_disp_buffers(
                                        pVidDec->nDispWidth,
                                        pVidDec->nDispHeight,
                                        pVidDec->sInPortFormat.eCompressionFormat);
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
                            if(pVidDec->nDispWidth * pVidDec->nDispHeight > 1280 * 720)
                                pVidDec->sOutPortDef.nBufferCountMin = 5;

                        }
                    }

                    pVidDec->sOutPortDef.nBufferCountMin -= pVidDec->minUndequeuedBufs;

    #ifdef SUSPEND_RESUME_HACK
                    pVidDec->sOutPortDef.nBufferCountMin -= 1;
    #endif
                    if((pVidDec->ThumbnailMode == 1) && (0 == pVidDec->shareDispBuf))
                    {
                        pVidDec->sOutPortDef.nBufferCountMin = 1;
                    }


                    pVidDec->sOutPortDef.nBufferCountActual = pVidDec->sOutPortDef.nBufferCountMin;
                    pVidDec->nBufferCountActual =  pVidDec->sOutPortDef.nBufferCountActual;

                    ITTIAM_DEBUG("Recalculated Buffer count = %d", (WORD32)pVidDec->sOutPortDef.nBufferCountActual);
                    /* H/w rendering using GraphicBufferMapper for OMX.google.android.index.useAndroidNativeBuffer2
                    requires chroma stride to be aligned 16, hence aligning luma stride to 32 for 420P */
                    pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN32(pVidDec->nDispWidth + pVidDec->padWidth);

                    pVidDec->sOutPortDef.format.video.nFrameHeight = pVidDec->nDispHeight + pVidDec->padHeight;
                    if(IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m == pVidDec->sOutPortDef.format.video.eColorFormat)
                    {
                        pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN128(pVidDec->nDispWidth + pVidDec->padWidth);
                        pVidDec->sOutPortDef.format.video.nFrameHeight = ALIGN32(pVidDec->sOutPortDef.format.video.nFrameHeight);
                    }

                }

                pVidDec->sOutPortDef.format.video.nStride = pVidDec->sOutPortDef.format.video.nFrameWidth;
                pVidDec->sOutPortDef.format.video.nSliceHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
                pVidDec->stride = pVidDec->sOutPortDef.format.video.nFrameWidth;
                pVidDec->sOutPortDef.nBufferSize = (pVidDec->sOutPortDef.format.video.nFrameWidth) *
                (pVidDec->sOutPortDef.format.video.nFrameHeight +
                                ((pVidDec->sOutPortDef.format.video.nFrameHeight + 1) / 2));

                pVidDec->sOutPortDef.nBufferSize = ALIGN4096(pVidDec->sOutPortDef.nBufferSize);
            }
            else
            {
                ITTIAM_ERROR("SetParameter OMX_IndexParamPortDefinition Invalid nPortIndex %d", (WORD32)portFmt->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }
        // Sets OMX_VIDEO_PARAM_PORTFORMATTYPE structure
        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *portFmt = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
            if (portFmt->nPortIndex == pVidDec->sInPortFormat.nPortIndex)
            {
                if (portFmt->nIndex > pVidDec->sInPortFormat.nIndex)
                {
                    ITTIAM_ERROR("Invalid input param->nIndex %d out->nIndex %d",
                                 (WORD32)portFmt->nIndex, (WORD32)pVidDec->sInPortFormat.nIndex);
                    eError = OMX_ErrorNoMore;
                }
                else
                    memcpy(&pVidDec->sInPortFormat, ComponentParameterStructure, sizeof
                                (OMX_VIDEO_PARAM_PORTFORMATTYPE));
            }
            else if (portFmt->nPortIndex == pVidDec->sOutPortFormat.nPortIndex)
            {
                if (portFmt->nIndex >= MAX_COLOR_FMTS)
                {
                    ITTIAM_ERROR("Invalid output param->nIndex %d out->nIndex %d",
                                 (WORD32)portFmt->nIndex, (WORD32)pVidDec->sOutPortFormat.nIndex);
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&pVidDec->sOutPortFormat, ComponentParameterStructure, sizeof
                                (OMX_VIDEO_PARAM_PORTFORMATTYPE));
                    if(OMX_COLOR_FormatYUV420Flexible == pVidDec->sOutPortFormat.eColorFormat)
                        pVidDec->sOutPortFormat.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                    pVidDec->sOutPortDef.format.video.eColorFormat = pVidDec->sOutPortFormat.eColorFormat;
                    ITTIAM_LOG("SetParameter OMX_IndexParamVideoPortFormat pVidDec->sOutPortDef.format.video.eColorFormat = 0x%x",
                                pVidDec->sOutPortDef.format.video.eColorFormat );

                }
            }
            else
            {
                ITTIAM_ERROR("SetParameter OMX_IndexParamVideoPortFormat Invalid nPortIndex %d", (WORD32)portFmt->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }
            // Sets OMX_PRIORITYMGMTTYPE structure
        case OMX_IndexParamPriorityMgmt:
            memcpy(&pVidDec->sPriorityMgmt, ComponentParameterStructure, sizeof
                            (OMX_PRIORITYMGMTTYPE));
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
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;


                if(pVidDec->shareDispBuf)
                {
                    pVidDec->padWidth = AVC_PADDING_WIDTH;
                    pVidDec->padHeight = AVC_PADDING_HEIGHT;
                }
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, AVC_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, AVC_MAX_HEIGHT);

            }
            if(!strcmp(cTempRole,"video_decoder.mpeg4"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingMPEG4;
                pVidDec->sInPortDef.format.video.cMIMEType = "MPEG4";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);


            }
            if(!strcmp(cTempRole,"video_decoder.h263"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingH263;
                pVidDec->sInPortDef.format.video.cMIMEType = "H263";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);
            }
            if(!strcmp(cTempRole,"video_decoder.divx"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = QOMX_VIDEO_CodingDivx;
                pVidDec->sInPortDef.format.video.cMIMEType = "DIVX";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);


            }
            if(!strcmp(cTempRole,"video_decoder.divx4"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = QOMX_VIDEO_CodingDivx;
                pVidDec->sInPortDef.format.video.cMIMEType = "DIVX";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);


            }

            if(!strcmp(cTempRole,"video_decoder.mpeg2"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingMPEG2;
                pVidDec->sInPortDef.format.video.cMIMEType = "MPEG2";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;
                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG2_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG2_MAX_HEIGHT);


            }
            if(!strcmp(cTempRole,"video_decoder.hevc"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingHEVC;
                pVidDec->sInPortDef.format.video.cMIMEType = "HEVC";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;

                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;


                if(pVidDec->shareDispBuf)
                {
                    pVidDec->padWidth = HEVC_PADDING_WIDTH;
                    pVidDec->padHeight = HEVC_PADDING_HEIGHT;
                }
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, HEVC_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, HEVC_MAX_HEIGHT);

            }
            if(!strcmp(cTempRole,"video_decoder.vp9"))
            {
                OMX_VIDEO_CODINGTYPE VidFormat = OMX_VIDEO_CodingVP9;
                pVidDec->sInPortDef.format.video.cMIMEType = "VP9";
                pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
                pVidDec->sInPortFormat.eCompressionFormat = VidFormat;

                pVidDec->padWidth = 0;
                pVidDec->padHeight = 0;


                if(pVidDec->shareDispBuf)
                {
                    pVidDec->padWidth = VP9_PADDING_WIDTH;
                    pVidDec->padHeight = VP9_PADDING_HEIGHT;
                }
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, VP9_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, VP9_MAX_HEIGHT);

            }
            break;

        case IOMX_ThumbnailMode:
            pVidDec->ThumbnailMode = 1;
            pVidDec->shareDispBuf = 0;
            pVidDec->padWidth = 0;
            pVidDec->padHeight = 0;


            pVidDec->sOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
            pVidDec->sOutPortFormat.eColorFormat = OMX_COLOR_FormatYUV420Planar;

            ITTIAM_LOG("In ThumbnailMode");
            break;

        case (OMX_INDEXTYPE)OMX_QcomIndexParamVideoDivx:
            ITTIAM_LOG("Divx Codecs");
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
    {
        pPortDef = &pVidDec->sInPortDef;
    }
    else if (nPortIndex == pVidDec->sOutPortDef.nPortIndex)
    {
        pPortDef = &pVidDec->sOutPortDef;
    }
    else
    {
        ITTIAM_ERROR("UseBuffer Invalid portIndex %d", (WORD32)nPortIndex);
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);
    }

    if (!pPortDef->bEnabled)
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (nSizeBytes < pPortDef->nBufferSize || pPortDef->bPopulated)
    {
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);
    }

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
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
        pVidDec->sInBufList.pBufHdr[nIndex]->pBuffer = pBuffer;
        pVidDec->sInBufList.bBufOwner[nIndex] = OMX_FALSE;
        LoadBufferHeader(pVidDec->sInBufList, pVidDec->sInBufList.pBufHdr[nIndex], pAppPrivate,
                        nSizeBytes, nPortIndex, *ppBufferHdr, pPortDef);
        ITTIAM_DEBUG("Input Use Buffer index = %d, bPopulated = %d nAllocSize = %d", (WORD32)nPortIndex, (WORD32)pVidDec->sInPortDef.bPopulated, (WORD32)pVidDec->sInBufList.nAllocSize);
    }
    else
    {
        ListAllocate(pVidDec->sOutBufList, nIndex);
        if (pVidDec->sOutBufList.pBufHdr[nIndex] == NULL)
        {
            pVidDec->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
            OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!pVidDec->sOutBufList.pBufHdr[nIndex])
            {
                ITTIAM_ERROR("UseBuffer Allocation failure for size %d", sizeof(OMX_BUFFERHEADERTYPE));
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);
            }

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
        pVidDec->bufferAllocationPending = 0;
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

    if (nSizeBytes < pPortDef->nBufferSize || pPortDef->bPopulated)
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
        pVidDec->bufferAllocationPending = 0;
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

    pthread_mutex_lock(&pVidDec->signal_mutex);
    if(pVidDec->bufferAllocationPending)
    {

        pVidDec->state = OMX_StateInvalid;
        data_sync_barrier();
        ITTIAM_DEBUG("Waiting for buffers to be allocated");
        pthread_cond_signal(&pVidDec->buffers_signal);

    }
    pthread_mutex_unlock(&pVidDec->signal_mutex);

    pthread_mutex_lock(&pVidDec->signal_flush_mutex);
    if(pVidDec->flushInProgress)
    {
        ITTIAM_DEBUG("Waiting for buffers to be flushed");
        int ret_val = pthread_cond_wait(&pVidDec->buffers_flush_signal,&pVidDec->signal_flush_mutex);
    }
    pthread_mutex_unlock(&pVidDec->signal_flush_mutex);

    if (nPortIndex == pVidDec->sInPortDef.nPortIndex)
        pPortDef = &pVidDec->sInPortDef;
    else if(nPortIndex == pVidDec->sOutPortDef.nPortIndex)
        pPortDef = &pVidDec->sOutPortDef;
    else
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadParameter);

    if (pPortDef->bEnabled && pVidDec->state != OMX_StateIdle && pVidDec->state != OMX_StateInvalid && pVidDec->state != OMX_StateExecuting)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

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

    pVidDec = (VIDDECTYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    OMX_CONF_CHECK_CMD(pVidDec, pBufferHdr, 1);
    OMX_CONF_CHK_VERSION(pBufferHdr, OMX_BUFFERHEADERTYPE, eError);

    if (!pVidDec->sOutPortDef.bEnabled)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorBadPortIndex);

    if (pVidDec->state != OMX_StateExecuting && pVidDec->state != OMX_StatePause)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorIncorrectStateOperation);


    pBufferHdr->nFlags = pBufferHdr->nFlags | CUSTOM_BUFFERFLAG_OWNED;

    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidDec->pipes_mutex);
    pVidDec->NumFTB++;
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidDec->cmddatapipe[1], &pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE*));
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

    pthread_mutex_lock(&pVidDec->signal_mutex);
    if((pVidDec->sInPortDef.bEnabled != pVidDec->sInPortDef.bPopulated) ||
                        (pVidDec->sOutPortDef.bEnabled != pVidDec->sOutPortDef.bPopulated) )
    {
        pVidDec->state = OMX_StateInvalid;
        data_sync_barrier();
        pthread_cond_signal(&pVidDec->buffers_signal);
    }
    pthread_mutex_unlock(&pVidDec->signal_mutex);

    // Put the command and data in the pipe
    pthread_mutex_lock(&pVidDec->pipes_mutex);
    write(pVidDec->cmdpipe[1], &eCmd, sizeof(eCmd));
    write(pVidDec->cmddatapipe[1], &eCmd, sizeof(eCmd));
    pthread_mutex_unlock(&pVidDec->pipes_mutex);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(pVidDec->thread_id, (void*)&eError);

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    ITTIAM_DEBUG("pVidDec->sInBufList.nAllocSize = %d",(WORD32)pVidDec->sInBufList.nAllocSize );
    if (pVidDec->sInBufList.nAllocSize > 0)
        ListFreeAllBuffers(pVidDec->sInBufList, nIndex)


    if(pVidDec->sInBufList.pBufHdr)
    {
        for (nIndex = 0; nIndex < NUM_IN_BUFFERS; nIndex++)
        {
            if(pVidDec->sInBufList.pBufHdr[nIndex])
            {
                OMX_OSAL_Free(pVidDec->sInBufList.pBufHdr[nIndex]);
                pVidDec->sInBufList.pBufHdr[nIndex] = NULL;
            }
        }

        OMX_OSAL_Free(pVidDec->sInBufList.pBufHdr);
        pVidDec->sInBufList.pBufHdr = NULL;
    }
    if(pVidDec->sInBufList.pBufHdr_dyn)
    {
        OMX_OSAL_Free(pVidDec->sInBufList.pBufHdr_dyn);
        pVidDec->sInBufList.pBufHdr_dyn = NULL;
    }
    if(pVidDec->sInBufList.bBufOwner)
    {
        OMX_OSAL_Free(pVidDec->sInBufList.bBufOwner);
        pVidDec->sInBufList.bBufOwner = NULL;
    }

    ITTIAM_DEBUG("pVidDec->sOutBufList.nAllocSize = %d",(WORD32)pVidDec->sOutBufList.nAllocSize );
    if (pVidDec->sOutBufList.nAllocSize > 0)
        ListFreeAllBuffers(pVidDec->sOutBufList, nIndex)

    if(pVidDec->sOutBufList.pBufHdr)
    {
        for (nIndex = 0; nIndex < NUM_OUT_BUFFERS; nIndex++)
        {
            if(pVidDec->sOutBufList.pBufHdr[nIndex])
            {
                OMX_OSAL_Free(pVidDec->sOutBufList.pBufHdr[nIndex]);
                pVidDec->sOutBufList.pBufHdr[nIndex] = NULL;
            }
        }

        OMX_OSAL_Free(pVidDec->sOutBufList.pBufHdr);
        pVidDec->sOutBufList.pBufHdr = NULL;
    }
    if(pVidDec->sOutBufList.pBufHdr_dyn)
    {
        OMX_OSAL_Free(pVidDec->sOutBufList.pBufHdr_dyn);
        pVidDec->sOutBufList.pBufHdr_dyn = NULL;
    }
    if(pVidDec->sOutBufList.bBufOwner)
    {
        OMX_OSAL_Free(pVidDec->sOutBufList.bBufOwner);
        pVidDec->sOutBufList.bBufOwner = NULL;
    }

    OMX_OSAL_Free(pVidDec->pFlushOutBuf);

    // close the pipe handles
    close(pVidDec->cmdpipe[0]);
    close(pVidDec->cmdpipe[1]);
    close(pVidDec->cmddatapipe[0]);
    close(pVidDec->cmddatapipe[1]);

    pthread_mutex_destroy(&pVidDec->pipes_mutex);
    pthread_mutex_destroy(&pVidDec->signal_mutex);


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
    else if(VidFormat == OMX_VIDEO_CodingVP9)
        return "VP9";
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
        else if(pVidDec->sInPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingVP9)
        {
            strlcpy((char *)cRole, "video_decoder.vp9",OMX_MAX_STRINGNAME_SIZE);
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
        {
            OMX_CONFIG_RECTTYPE *rect;
            rect = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
            if (rect->nPortIndex == 1)
            {
                rect->nLeft = pVidDec->padWidth/2;
                rect->nTop = pVidDec->padHeight/2;
                rect->nWidth = pVidDec->nDispWidth;
                rect->nHeight = pVidDec->nDispHeight;
                ITTIAM_DEBUG("OMX_IndexConfigCommonOutputCrop left %d right %d width %d height %d", (WORD32)rect->nLeft, (WORD32)rect->nTop, (WORD32)rect->nWidth, (WORD32)rect->nHeight);
            }
            else
            {
                ITTIAM_DEBUG("get_config: Bad port index %d queried on only o/p port\n",
                                (WORD32)rect->nPortIndex);
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }
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

        case IOMX_ThumbnailMode:
            ITTIAM_LOG("Thumbnail mode information recieved");
            pVidDec->ThumbnailMode = 1;
            pVidDec->shareDispBuf = 0;
            pVidDec->padWidth = 0;
            pVidDec->padHeight = 0;


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
    ITTIAM_DEBUG("Calling GetExtensionIndex for %s", cParameterName);


    if(pVidDec->state == OMX_StateInvalid)
    {
        ITTIAM_DEBUG("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.getAndroidNativeBufferUsage", sizeof("OMX.google.android.index.getAndroidNativeBufferUsage") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexGetAndroidNativeBufferUsage;
        pVidDec->bufferType = BUFFER_TYPE_NATIVEBUF2;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.useAndroidNativeBuffer2", sizeof("OMX.google.android.index.useAndroidNativeBuffer2") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexUseAndroidNativeBuffer2;
        pVidDec->bufferType = BUFFER_TYPE_NATIVEBUF2;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.useAndroidNativeBuffer", sizeof("OMX.google.android.index.useAndroidNativeBuffer") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexUseAndroidNativeBuffer;
        pVidDec->bufferType = BUFFER_TYPE_NATIVEBUF;
    }
    else if(!strncmp(cParameterName,"OMX.google.android.index.enableAndroidNativeBuffers", sizeof("OMX.google.android.index.enableAndroidNativeBuffers") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexEnableAndroidNativeBuffer;
        pVidDec->bufferType = BUFFER_TYPE_NATIVEBUF2;
    }
    else if(!strncmp(cParameterName,"OMX.QCOM.index.param.video.SyncFrameDecodingMode", sizeof("OMX.QCOM.index.param.video.SyncFrameDecodingMode") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_ThumbnailMode;
    }
    else if(!strncmp(cParameterName,"OMX.ittiam.ThumbnailMode", sizeof("OMX.ittiam.ThumbnailMode") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_ThumbnailMode;
    }
#ifdef ENABLE_ADAPTIVE_PLAYBACK
    else if(!strncmp(cParameterName,"OMX.google.android.index.prepareForAdaptivePlayback", sizeof("OMX.google.android.index.prepareForAdaptivePlayback") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexEnableAdaptivePlayback;
    }
#endif
#ifdef ENABLE_DESCRIBE_COLOR_FORMAT
    else if(!strncmp(cParameterName,"OMX.google.android.index.describeColorFormat", sizeof("OMX.google.android.index.describeColorFormat") - 1))
    {
        *pIndexType = (OMX_INDEXTYPE)IOMX_GoogleAndroidIndexDescribeColorFormat;
    }
#endif
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

void ComponentReset(VIDDECTYPE *pVidDec)
{
    pVidDec->isInFlush = 0;
    pVidDec->receivedEOS = 0;
    pVidDec->ThumbnailMode = 0;
    pVidDec->bufferType = BUFFER_TYPE_LOCAL;
    pVidDec->PortReconfiguration = 0;
    pVidDec->reInitPending = 0;
    pVidDec->reInitWidth = 0;
    pVidDec->reInitHeight = 0;
    pVidDec->nBufferCountActual = 0;
    pVidDec->flushInProgress = 0;
    pVidDec->codecBufCnt = 0;
    pVidDec->bufferAllocationPending = 0;
    pVidDec->hdrDecodeDone = 0;

    pVidDec->NumETB = 0;
    pVidDec->NumEBD = 0;
    pVidDec->NumFTB = 0;
    pVidDec->NumFBD = 0;

    pVidDec->initWidth = 0;
    pVidDec->initHeight = 0;
    pVidDec->nDispWidth = 0;
    pVidDec->nDispHeight = 0;
    pVidDec->initDone = 0;
    pVidDec->prevTimeStamp = -1;


}
/*****************************************************************************/
OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_VIDEO_CODINGTYPE VidFormat,
                OMX_IN OMX_COLOR_FORMATTYPE ColorFormat,
                OMX_IN OMX_U32 maxWidth,
                OMX_IN OMX_U32 maxHeight,
                OMX_IN OMX_U32 share_disp_buf,
                OMX_IN OMX_U32 numCores,
                OMX_IN OMX_U32 disableInterlaced,
                OMX_IN OMX_U32 minUndequeuedBufs,
                OMX_IN PROCESSORTYPE processorType,
                OMX_IN OMX_U32 swrender)
{
    OMX_COMPONENTTYPE *pComp;
    VIDDECTYPE *pVidDec;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 err;
    OMX_U32 nIndex;

    pComp = (OMX_COMPONENTTYPE *)hComponent;

    // Create private data
    pVidDec = (VIDDECTYPE *)OMX_OSAL_Malloc(sizeof(VIDDECTYPE));
    if (!pVidDec)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    memset(pVidDec, 0x0, sizeof(VIDDECTYPE));
    pVidDec->swrender = swrender;
    pVidDec->shareDispBuf = share_disp_buf;
    pVidDec->numCores = numCores;
    pVidDec->disableInterlaced = disableInterlaced;
    pVidDec->maxWidth = maxWidth;
    pVidDec->maxHeight = maxHeight;
    pVidDec->minUndequeuedBufs = minUndequeuedBufs;
    pVidDec->VidFormat = VidFormat;
    pVidDec->mProcessorType = processorType;
    GETTIME(&pVidDec->prevStopTime);

    ITTIAM_LOG("Ittiam OMX Video Decoder Version %s",ComponentVersion);
    ITTIAM_LOG("Color Format 0x%x", ColorFormat);
    ITTIAM_LOG("Number of Cores %d", (WORD32)numCores);
    pComp->pComponentPrivate = (OMX_PTR)pVidDec;
    pVidDec->state = OMX_StateLoaded;
    pVidDec->hSelf = hComponent;
    pVidDec->ignoreInitialBPics = 1;
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

    {
        switch((WORD32)VidFormat)
        {
            case OMX_VIDEO_CodingMPEG4:
            case QOMX_VIDEO_CodingDivx:
            {
                pVidDec->iVdec_cxa8_api_function = imp4d_cxa8_api_function;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);
                pVidDec->mDRCError = IVD_RES_CHANGED;
                pVidDec->mUnsupportedReslnError = IVD_STREAM_WIDTH_HEIGHT_NOT_SUPPORTED;
                pVidDec->pProfiles = gMPEG4Profiles;
                pVidDec->pLevels = gMPEG4Levels;
                pVidDec->numProfiles = gNumMPEG4Profiles;
                pVidDec->numLevels = gNumMPEG4Levels;
                pVidDec->shareDispBuf = 0;
                break;
            }
            case OMX_VIDEO_CodingH263:
            {
                pVidDec->iVdec_cxa8_api_function = imp4d_cxa8_api_function;
                pVidDec->maxWidth  = MIN(pVidDec->maxWidth, MPEG4_MAX_WIDTH);
                pVidDec->maxHeight = MIN(pVidDec->maxHeight, MPEG4_MAX_HEIGHT);
                pVidDec->mDRCError = IVD_RES_CHANGED;
                pVidDec->mUnsupportedReslnError = IVD_STREAM_WIDTH_HEIGHT_NOT_SUPPORTED;
                pVidDec->pProfiles = gH263Profiles;
                pVidDec->pLevels = gH263Levels;
                pVidDec->numProfiles = gNumH263Profiles;
                pVidDec->numLevels = gNumH263Levels;
                pVidDec->shareDispBuf = 0;
                break;
            }

            default:
                ITTIAM_DEBUG("%s Unsuppoted VidFormat = %x ", __FUNCTION__, VidFormat );
                eError = OMX_ErrorUndefined;
                goto EXIT;
            break;
        }
    }

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sPortParam, OMX_PORT_PARAM_TYPE);
    pVidDec->sPortParam.nPorts = 0x2;
    pVidDec->sPortParam.nStartPortNumber = 0x0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    pVidDec->sInPortDef.nPortIndex = 0x0;
    pVidDec->sInPortDef.bEnabled = OMX_TRUE;
    pVidDec->sInPortDef.bPopulated = OMX_FALSE;
    pVidDec->sInPortDef.eDomain = OMX_PortDomainVideo;

    pVidDec->sInPortDef.format.video.nFrameWidth = 176;
    pVidDec->sInPortDef.format.video.nFrameHeight = 144;


    pVidDec->sInPortDef.eDir = OMX_DirInput;
    pVidDec->sInPortDef.nBufferCountMin = NUM_IN_BUFFERS;
    pVidDec->sInPortDef.nBufferCountActual = NUM_IN_BUFFERS;
    pVidDec->sInPortDef.nBufferSize = (OMX_U32)(1024*1024);// JK TBD
    pVidDec->sInPortDef.format.video.cMIMEType = convert_vidformat_to_mime(VidFormat);
    pVidDec->sInPortDef.format.video.eCompressionFormat = VidFormat;
    pVidDec->sInPortDef.format.video.eColorFormat = OMX_VIDEO_CodingUnused;

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    pVidDec->sOutPortDef.nPortIndex = 0x1;
    pVidDec->sOutPortDef.bEnabled = OMX_TRUE;
    pVidDec->sOutPortDef.bPopulated = OMX_FALSE;
    pVidDec->sOutPortDef.eDomain = OMX_PortDomainVideo;
    pVidDec->sOutPortDef.format.video.cMIMEType = "YUV420";
    pVidDec->sOutPortDef.format.video.nFrameWidth = 176;
    pVidDec->sOutPortDef.format.video.nFrameHeight = 144;
    pVidDec->sOutPortDef.eDir = OMX_DirOutput;
    pVidDec->sOutPortDef.nBufferCountMin = NUM_OUT_BUFFERS;
    pVidDec->sOutPortDef.nBufferCountActual = NUM_OUT_BUFFERS;

    pVidDec->sOutPortDef.format.video.eColorFormat = ColorFormat;
    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&pVidDec->sInPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    pVidDec->sInPortFormat.nPortIndex = 0x0;
    pVidDec->sInPortFormat.nIndex = 0x0;
    pVidDec->sInPortFormat.eCompressionFormat = VidFormat;

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



    pVidDec->sOutPortDef.nBufferSize = (pVidDec->nDispWidth + pVidDec->padWidth) *
    ((pVidDec->nDispHeight + pVidDec->padHeight) +
                    (((pVidDec->nDispHeight + 1) / 2) + pVidDec->padHeight / 2));
    // Initialize the input buffer list
    memset(&(pVidDec->sInBufList), 0x0, sizeof(BufferList));

    pVidDec->sInBufList.pBufHdr = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sInPortDef.nBufferCountActual);
    if (!pVidDec->sInBufList.pBufHdr)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    for (nIndex = 0; nIndex < pVidDec->sInPortDef.nBufferCountActual; nIndex++)
    {
        pVidDec->sInBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
        OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        if (!pVidDec->sInBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

        OMX_CONF_INIT_STRUCT_PTR (pVidDec->sInBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    pVidDec->sInBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sInPortDef.nBufferCountActual);
    if (!pVidDec->sInBufList.pBufHdr_dyn)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    pVidDec->sInBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                    pVidDec->sInPortDef.nBufferCountActual);

    if (!pVidDec->sInBufList.bBufOwner)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

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
    if (!pVidDec->sOutBufList.pBufHdr)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    for (nIndex = 0; nIndex < pVidDec->sOutPortDef.nBufferCountActual; nIndex++)
    {
        pVidDec->sOutBufList.pBufHdr[nIndex] = (OMX_BUFFERHEADERTYPE*)
        OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
        if (!pVidDec->sOutBufList.pBufHdr[nIndex])
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

        OMX_CONF_INIT_STRUCT_PTR (pVidDec->sOutBufList.pBufHdr[nIndex], OMX_BUFFERHEADERTYPE);
    }
    pVidDec->sOutBufList.pBufHdr_dyn = (OMX_BUFFERHEADERTYPE**)
    OMX_OSAL_Malloc (sizeof(OMX_BUFFERHEADERTYPE*) *
                    pVidDec->sOutPortDef.nBufferCountActual);
    if (!pVidDec->sOutBufList.pBufHdr_dyn)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    pVidDec->sOutBufList.bBufOwner = (OMX_BOOL*) OMX_OSAL_Malloc (sizeof(OMX_BOOL) *
                    pVidDec->sOutPortDef.nBufferCountActual);
    if (!pVidDec->sOutBufList.bBufOwner)
        OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorInsufficientResources);

    pVidDec->sOutBufList.nSizeOfList = 0;
    pVidDec->sOutBufList.nAllocSize = 0;
    pVidDec->sOutBufList.nListEnd = -1;
    pVidDec->sOutBufList.nWritePos = -1;
    pVidDec->sOutBufList.nReadPos = -1;
    pVidDec->sOutBufList.eDir = OMX_DirOutput;



    //If input dump is enabled, then open create an empty file
    CREATE_INPUT_DUMP(INPUT_DUMP_PATH);

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

    pVidDec->pFlushOutBuf = OMX_OSAL_Malloc((3 * pVidDec->maxWidth * pVidDec->maxHeight) / 2);
    if (!pVidDec->pFlushOutBuf)
    {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // One pipe is thread safe. But since two pipes are used here,
    // a mutex lock is required to prevent any race condition

    pthread_mutex_init(&pVidDec->pipes_mutex, NULL);
    pthread_mutex_init(&pVidDec->signal_mutex, NULL);

    pthread_cond_init(&pVidDec->buffers_signal, NULL);

    pthread_mutex_init(&pVidDec->signal_flush_mutex, NULL);
    pthread_cond_init(&pVidDec->buffers_flush_signal, NULL);

    // Create the component thread
    err = pthread_create(&pVidDec->thread_id, NULL, ComponentThread, pVidDec);
    if( err || !pVidDec->thread_id )
    {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    ComponentReset(pVidDec);


    EXIT:
    OMX_CONF_CMD_BAIL:
    if(pVidDec)
        ITTIAM_DEBUG("%s state = %x ", __FUNCTION__, pVidDec->state );
    return eError;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
