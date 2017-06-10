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
//extern "C" {
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


#include <wchar.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "datatypedef.h"
#include "iv.h"
#include "ivd.h"

#include "imp4d_cxa8.h"

#include <OMX_Ittiam.h>
#include <OMX_Ittiam_Vdec.h>

#include<utils/Log.h>
#include <system/graphics.h>


IV_API_CALL_STATUS_T ittiam_video_decoder_init(VIDDECTYPE* pVidDec);

void ittiam_video_decoder_process(VIDDECTYPE* pVidDec,
                                  OMX_BUFFERHEADERTYPE *pInBufHdr,
                                  OMX_BUFFERHEADERTYPE *pOutBufHdr);
void ittiam_video_decoder_reset(VIDDECTYPE* pVidDec);
IV_API_CALL_STATUS_T ittiam_video_set_params(VIDDECTYPE* pVidDec, IVD_VIDEO_DECODE_MODE_T e_mode);


void ittiam_video_pad_output_buffer(VIDDECTYPE *pVidDec, UWORD8 *pu1_out_buf)
{

    UWORD8 *pu1_buf = pu1_out_buf;
    WORD32 diff = pVidDec->sOutPortDef.format.video.nFrameWidth - pVidDec->nDispWidth;
    UWORD32 i, j;
    for(i = 0; i < pVidDec->sOutPortDef.format.video.nFrameHeight; i++)
    {
        memset(pu1_buf + pVidDec->nDispWidth, 0, diff);
        pu1_buf += pVidDec->sOutPortDef.format.video.nFrameWidth;
    }

    UWORD32 stride = pVidDec->sOutPortDef.format.video.nFrameWidth;
    UWORD32 height = pVidDec->sOutPortDef.format.video.nFrameHeight;


    switch((WORD32)pVidDec->sOutPortDef.format.video.eColorFormat)
    {
        /* YUV/YVU 420 Planar */
        case IOMX_COLOR_FormatYVU420Planar:
        case OMX_COLOR_FormatYUV420Planar:
            pu1_buf = pu1_out_buf;
            pu1_buf += stride * height;

            for(i = 0; i < height / 2; i++)
            {
                memset(pu1_buf + pVidDec->nDispWidth / 2, 0, diff / 2);
                pu1_buf += stride / 2;
            }

            pu1_buf = pu1_out_buf;
            pu1_buf += stride * height;
            pu1_buf += stride * height / 4;


            for(i = 0; i < height / 2; i++)
            {
                memset(pu1_buf + pVidDec->nDispWidth / 2, 0, diff / 2);
                pu1_buf += stride / 2;
            }

            break;

        /* YUV 420 Semi Planar */
        case OMX_COLOR_FormatYUV420Flexible:
        case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m:
        case IOMX_COLOR_FormatYVU420SemiPlanar:
        case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
            pu1_buf = pu1_out_buf;
            pu1_buf += stride * height;

            for(i = 0; i < height / 2; i++)
            {
                memset(pu1_buf + pVidDec->nDispWidth, 0, diff);
                pu1_buf += stride;
            }


            break;

        default:
            ITTIAM_ERROR("Unknown Color Format %8x", pVidDec->sOutPortDef.format.video.eColorFormat);
            break;
    }
    return;
}

static WORD32 get_min_timestamp_idx(OMX_S64 *p_nTimeStamp, UWORD32 *p_isTimeStampValid)
{
    OMX_S64 minTimeStamp;
    WORD32 i;
    WORD32 idx;
    minTimeStamp = LLONG_MAX;
    idx = -1;
    for(i = 0; i < MAX_TIMESTAMP_CNT; i++)
    {
        if(p_isTimeStampValid[i])
        {
            if(p_nTimeStamp[i] < minTimeStamp)
            {
                minTimeStamp = p_nTimeStamp[i];
                idx = i;
            }
        }
    }
    return idx;
}

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

static void handle_port_settings_changed(VIDDECTYPE* pVidDec, WORD32 width, WORD32 height, OMX_BUFFERHEADERTYPE *pOutBufHdr)
{
    ITTIAM_LOG("Need port reconfiguration width old %d new %d ",
          (UWORD32)pVidDec->nDispWidth, (UWORD32)width);
    ITTIAM_LOG("Need port reconfiguration height old %d new %d ",
          (UWORD32)pVidDec->nDispHeight, (UWORD32)height);
    pVidDec->nDispWidth = width;
    pVidDec->nDispHeight = height;
    /* H/w rendering using GraphicBufferMapper for OMX.google.android.index.useAndroidNativeBuffer2
    requires chroma stride to be aligned 16, hence aligning luma stride to 32 for 420P */
    pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN32(pVidDec->nDispWidth + pVidDec->padWidth);

    pVidDec->sOutPortDef.format.video.nFrameHeight = (pVidDec->nDispHeight) + (pVidDec->padHeight);
    if(IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m == pVidDec->sOutPortDef.format.video.eColorFormat)
    {
        pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN128(pVidDec->nDispWidth + pVidDec->padWidth);
        pVidDec->sOutPortDef.format.video.nFrameHeight = ALIGN32(pVidDec->sOutPortDef.format.video.nFrameHeight);
    }
    pVidDec->sOutPortDef.format.video.nStride = pVidDec->sOutPortDef.format.video.nFrameWidth;
    pVidDec->sOutPortDef.format.video.nSliceHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
    pVidDec->stride = pVidDec->sOutPortDef.format.video.nStride;
    pVidDec->sOutPortDef.nBufferSize = (pVidDec->sOutPortDef.format.video.nFrameWidth) *
        (pVidDec->sOutPortDef.format.video.nFrameHeight +
                        ((pVidDec->sOutPortDef.format.video.nFrameHeight + 1) / 2));


    {
        pVidDec->nBufferCountActual =  pVidDec->sOutPortDef.nBufferCountActual;
        if((1 == pVidDec->shareDispBuf) && ((pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) || (pVidDec->sInPortFormat.eCompressionFormat == OMX_VIDEO_CodingHEVC)))
            pVidDec->sOutPortDef.nBufferCountMin = get_num_disp_buffers(pVidDec->nDispWidth,pVidDec->nDispHeight,pVidDec->sInPortFormat.eCompressionFormat);
        else
            pVidDec->sOutPortDef.nBufferCountMin = 11;
        pVidDec->sOutPortDef.nBufferCountMin -= pVidDec->minUndequeuedBufs;

        pVidDec->sOutPortDef.nBufferCountActual = pVidDec->sOutPortDef.nBufferCountMin;
    }

    pVidDec->sOutPortDef.nBufferSize = ALIGN4096(pVidDec->sOutPortDef.nBufferSize);
    pVidDec->PortReconfiguration = 1;
    pVidDec->hdrDecodeDone = 0;


    ITTIAM_LOG("Calling OMX_EventPortSettingsChanged handler");
    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                      pVidDec->pAppData,
                                      OMX_EventPortSettingsChanged,
                                      0x1, 0, NULL);
    if(pOutBufHdr)
    {
        pOutBufHdr->nFilledLen = 0;
        pOutBufHdr->nTimeStamp = 0;
        pOutBufHdr->nFlags = pOutBufHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);
        FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pOutBufHdr);
    }

    return;
}




void ittiam_video_release_display_frame(VIDDECTYPE* pVidDec,
                                        UWORD32 disp_buf_id)
{
    ivd_rel_display_frame_ip_t s_video_rel_disp_ip;
    ivd_rel_display_frame_op_t s_video_rel_disp_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_video_rel_disp_ip.e_cmd = IVD_CMD_REL_DISPLAY_FRAME;
    s_video_rel_disp_ip.u4_size = sizeof(ivd_rel_display_frame_ip_t);
    s_video_rel_disp_op.u4_size = sizeof(ivd_rel_display_frame_op_t);
    s_video_rel_disp_ip.u4_disp_buf_id = disp_buf_id;

    ITTIAM_DEBUG("Releasing buffer with id %d\n", disp_buf_id);
    e_dec_status = pVidDec->iVdec_cxa8_api_function(
                    pVidDec->ps_codec, (void *)&s_video_rel_disp_ip,
                    (void *)&s_video_rel_disp_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_ERROR("Error in release display frame %d error code %x", e_dec_status,
              s_video_rel_disp_op.u4_error_code);
    }

}

WORD32 ittiam_video_release_display_frames(VIDDECTYPE *pVidDec)
{
    UWORD32 i;
    OMX_BUFFERHEADERTYPE* pBufferHdr;

    if(0 == pVidDec->shareDispBuf)
        return 0;

    if(0 == pVidDec->hdrDecodeDone)
        return 0;
    /* If there are any buffers in the fillThisBuffer list,
       then release them to the codec */
    while(pVidDec->sOutBufList.nSizeOfList > 0)
    {
        ListGetEntry(pVidDec->sOutBufList, pBufferHdr);

        for(i = 0; i < NUM_OUT_BUFFERS; i++)
        {
            if(pBufferHdr->pOutputPortPrivate == pVidDec->disp_buf_id_mapping[i])
            {
                ittiam_video_release_display_frame(pVidDec, i);
                pVidDec->codecBufCnt++;
                break;
            }
        }
    }
    return 0;
}
IV_API_CALL_STATUS_T ittiam_video_set_flush_mode(VIDDECTYPE* pVidDec)
{
    ivd_ctl_flush_ip_t s_video_flush_ip;
    ivd_ctl_flush_op_t s_video_flush_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_video_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_video_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
    s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
    s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);
    ITTIAM_LOG("Setting decoder in flush mode");
    /*****************************************************************************/
    /*   API Call: Video Flush                                                  */
    /*****************************************************************************/
    e_dec_status = pVidDec->iVdec_cxa8_api_function(
                    pVidDec->ps_codec, (void *)&s_video_flush_ip,
                    (void *)&s_video_flush_op);
    if(e_dec_status != IV_SUCCESS)
    {
        ITTIAM_ERROR("Error in video Flush e_dec_status = %d u4_error_code = %d\n",
              e_dec_status, s_video_flush_op.u4_error_code);
    }
    pVidDec->isInFlush = 1;
    return e_dec_status;

}
void ittiam_video_log_codec_version(VIDDECTYPE* pVidDec)
{
    ivd_ctl_getversioninfo_ip_t s_ctl_dec_ip;
    ivd_ctl_getversioninfo_op_t s_ctl_dec_op;
    UWORD8 au1_buf[512];
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_GETVERSION;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_getversioninfo_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_getversioninfo_op_t);
    s_ctl_dec_ip.pv_version_buffer = au1_buf;
    s_ctl_dec_ip.u4_version_buffer_size = sizeof(au1_buf);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                     (void *)&(s_ctl_dec_ip),
                                     (void *)&(s_ctl_dec_op));

    if(e_dec_status != IV_SUCCESS)
    {
        ITTIAM_ERROR("Error in Getting Version number e_dec_status = %d u4_error_code = %x\n",
              e_dec_status, s_ctl_dec_op.u4_error_code);
    }
    else
    {
        ITTIAM_LOG("Ittiam Decoder Version number: %s\n",
              (char *)s_ctl_dec_ip.pv_version_buffer);
    }
    return;
}


void ittiam_video_set_decode_args(VIDDECTYPE* pVidDec,
                                ivd_video_decode_ip_t *ps_video_decode_ip,
                                ivd_video_decode_op_t *ps_video_decode_op,
                                  OMX_BUFFERHEADERTYPE *pInBufHdr,
                                  OMX_BUFFERHEADERTYPE *pOutBufHdr,
                                  WORD32 timeStampIdx)
{


    ps_video_decode_ip->u4_size = sizeof(ivd_video_decode_ip_t);
    ps_video_decode_op->u4_size = sizeof(ivd_video_decode_op_t);

    ps_video_decode_ip->e_cmd = IVD_CMD_VIDEO_DECODE;


    ps_video_decode_ip->pv_stream_buffer = NULL;
    ps_video_decode_ip->u4_num_Bytes = 0;
    ps_video_decode_ip->u4_ts = 0;
    if(pInBufHdr)
    {
        ps_video_decode_ip->u4_ts = timeStampIdx;
        ps_video_decode_ip->pv_stream_buffer = pInBufHdr->pBuffer + pInBufHdr->nOffset;
        ps_video_decode_ip->u4_num_Bytes = pInBufHdr->nFilledLen - pInBufHdr->nOffset;
    }

    if(0 == pVidDec->shareDispBuf)
    {

        ps_video_decode_ip->s_out_buffer.pu1_bufs[0] = pVidDec->pFlushOutBuf;
        if(pOutBufHdr)
            ps_video_decode_ip->s_out_buffer.pu1_bufs[0] = pOutBufHdr->pOutputPortPrivate;

        {
            WORD32 stride = pVidDec->sOutPortDef.format.video.nFrameWidth;
            WORD32 height = pVidDec->sOutPortDef.format.video.nFrameHeight;

            ps_video_decode_ip->s_out_buffer.u4_min_out_buf_size[0] = stride * height;
            ps_video_decode_ip->s_out_buffer.u4_min_out_buf_size[1] = (stride * height) / 4;
            ps_video_decode_ip->s_out_buffer.u4_min_out_buf_size[2] = (stride * height) / 4;


            ps_video_decode_ip->s_out_buffer.pu1_bufs[1] = ps_video_decode_ip->s_out_buffer.pu1_bufs[0] + (stride * height);
            ps_video_decode_ip->s_out_buffer.pu1_bufs[2] = ps_video_decode_ip->s_out_buffer.pu1_bufs[1] + ((stride * height) / 4);
            ps_video_decode_ip->s_out_buffer.u4_num_bufs = 3;

            /* In case of Hardware rendering using HAL_PIXEL_FORMAT_YV12, swap U and V pointers */
            if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420Planar )
            {
                UWORD8 *pu1_buf = ps_video_decode_ip->s_out_buffer.pu1_bufs[1];
                ps_video_decode_ip->s_out_buffer.pu1_bufs[1] = ps_video_decode_ip->s_out_buffer.pu1_bufs[2];
                ps_video_decode_ip->s_out_buffer.pu1_bufs[2] = pu1_buf;
            }
        }
    }
    return;
}

void ittiam_video_set_display_frame(VIDDECTYPE* pVidDec)
{
    ivd_ctl_getbufinfo_ip_t s_ctl_dec_ip;
    ivd_ctl_getbufinfo_op_t s_ctl_dec_op;
    WORD32 outlen = 0;
    UWORD32 num_display_buf;
    UWORD32 i;
    IV_API_CALL_STATUS_T e_dec_status;

    if(0 == pVidDec->shareDispBuf)
        return;

    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_GETBUFINFO;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_getbufinfo_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_getbufinfo_op_t);
    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(e_dec_status != IV_SUCCESS)
    {
        ITTIAM_ERROR("Error in Get Buf Info");
    }

    ITTIAM_DEBUG("u4_num_disp_bufs = %d", s_ctl_dec_op.u4_num_disp_bufs);
    num_display_buf = s_ctl_dec_op.u4_num_disp_bufs < pVidDec->sOutPortDef.nBufferCountActual ?
                                    s_ctl_dec_op.u4_num_disp_bufs :
                                    pVidDec->sOutPortDef.nBufferCountActual;

    num_display_buf = pVidDec->sOutPortDef.nBufferCountActual;

    for(i = 0; i < num_display_buf;)
    {
        pVidDec->s_disp_buffers[i].u4_min_out_buf_size[0] =
                        s_ctl_dec_op.u4_min_out_buf_size[0];
        pVidDec->s_disp_buffers[i].u4_min_out_buf_size[1] =
                        s_ctl_dec_op.u4_min_out_buf_size[1];
        pVidDec->s_disp_buffers[i].u4_min_out_buf_size[2] =
                        s_ctl_dec_op.u4_min_out_buf_size[2];

        outlen = s_ctl_dec_op.u4_min_out_buf_size[0];

        if(s_ctl_dec_op.u4_min_num_out_bufs > 1)
            outlen += s_ctl_dec_op.u4_min_out_buf_size[1];

        if(s_ctl_dec_op.u4_min_num_out_bufs > 2)
            outlen += s_ctl_dec_op.u4_min_out_buf_size[2];


        pVidDec->s_disp_buffers[i].pu1_bufs[0] =
                        pVidDec->sOutBufList.pBufHdr[i]->pOutputPortPrivate;

        ITTIAM_DEBUG("Set Display Frame buffer %d -- %x", i, (WORD32)pVidDec->sOutBufList.pBufHdr[i]->pOutputPortPrivate);

        if(pVidDec->s_disp_buffers[i].pu1_bufs[0] == NULL)
        {
            ITTIAM_LOG("\nAllocation failure\n");
            return;
        }

        {

            if(s_ctl_dec_op.u4_min_num_out_bufs > 1)
                pVidDec->s_disp_buffers[i].pu1_bufs[1] = pVidDec->s_disp_buffers[i].pu1_bufs[0] + (s_ctl_dec_op.u4_min_out_buf_size[0]);

            if(s_ctl_dec_op.u4_min_num_out_bufs > 2)
                pVidDec->s_disp_buffers[i].pu1_bufs[2] = pVidDec->s_disp_buffers[i].pu1_bufs[1] + (s_ctl_dec_op.u4_min_out_buf_size[1]);

            pVidDec->s_disp_buffers[i].u4_num_bufs = s_ctl_dec_op.u4_min_num_out_bufs;

            pVidDec->disp_buf_id_mapping[i] = pVidDec->s_disp_buffers[i].pu1_bufs[0];

            /* In case of Hardware rendering using HAL_PIXEL_FORMAT_YV12, swap U and V pointers */
            if(pVidDec->sOutPortDef.format.video.eColorFormat == (OMX_COLOR_FORMATTYPE)IOMX_COLOR_FormatYVU420Planar )
            {
                UWORD8 *pu1_buf = pVidDec->s_disp_buffers[i].pu1_bufs[1];
                pVidDec->s_disp_buffers[i].pu1_bufs[1] = pVidDec->s_disp_buffers[i].pu1_bufs[2];
                pVidDec->s_disp_buffers[i].pu1_bufs[2] = pu1_buf;

            }

            i++;
        }
    }


    {
        ivd_set_display_frame_ip_t s_set_display_frame_ip;
        ivd_set_display_frame_op_t s_set_display_frame_op;

        s_set_display_frame_ip.e_cmd = IVD_CMD_SET_DISPLAY_FRAME;
        s_set_display_frame_ip.u4_size = sizeof(ivd_set_display_frame_ip_t);
        s_set_display_frame_op.u4_size = sizeof(ivd_set_display_frame_op_t);

        s_set_display_frame_ip.num_disp_bufs = num_display_buf;

        ITTIAM_DEBUG("s_set_display_frame_ip.num_disp_bufs = %d",
                     s_set_display_frame_ip.num_disp_bufs);

        memcpy(&(s_set_display_frame_ip.s_disp_buffer),
               &(pVidDec->s_disp_buffers),
               s_set_display_frame_ip.num_disp_bufs * sizeof(ivd_out_bufdesc_t));

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->ps_codec, (void *)&s_set_display_frame_ip,
                        (void *)&s_set_display_frame_op);

        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in Set display frame %d error code %x", e_dec_status,
                  s_set_display_frame_op.u4_error_code);
        }

    }

}

void mpeg4_disable_qpel(VIDDECTYPE* pVidDec)
{
    imp4d_cxa8_ctl_disable_qpel_ip_t s_ip;
    imp4d_cxa8_ctl_disable_qpel_op_t s_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_ip.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_ip_t);
    s_op.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_op_t);
    s_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_DISABLE_QPEL;
    s_ip.u4_disable_qpel_level = 1; /* Disable Qpel in B frames */

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ip,
                                                    (void *)&s_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for mpeg4_disable_qpel Failed\n");
    }

}

void mpeg4_enable_qpel(VIDDECTYPE* pVidDec)
{
    imp4d_cxa8_ctl_disable_qpel_ip_t s_ip;
    imp4d_cxa8_ctl_disable_qpel_op_t s_op;
    IV_API_CALL_STATUS_T e_dec_status;
    s_ip.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_ip_t);
    s_op.u4_size = sizeof(imp4d_cxa8_ctl_disable_qpel_op_t);
    s_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_DISABLE_QPEL;
    s_ip.u4_disable_qpel_level = 0; /* Enable Qpel */

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ip,
                                                    (void *)&s_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for mpeg4_enable_qpel Failed\n");
    }
}
void video_skipb_frames(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_B;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skipb_frames Failed\n");
    }
}

void video_skip_none(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skip_none Failed\n");
    }
}

void video_skip_pb_frames(VIDDECTYPE* pVidDec)
{
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_PB;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for video_skip_pb_frames Failed\n");
    }
}


IV_API_CALL_STATUS_T iv_mpeg2d_set_numcores(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status = IV_FAIL;
    return e_dec_status;
}

IV_API_CALL_STATUS_T iv_h264d_set_numcores(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status = IV_FAIL;
    return e_dec_status;
}

IV_API_CALL_STATUS_T iv_hevcd_set_numcores(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status = IV_FAIL;
    return e_dec_status;
}

IV_API_CALL_STATUS_T iv_mpeg4d_set_numcores(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status = IV_FAIL;
    imp4d_cxa8_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
    imp4d_cxa8_ctl_set_num_cores_op_t s_ctl_set_cores_op;

    s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_set_cores_ip.e_sub_cmd = IMP4D_CXA8_CMD_CTL_SET_NUM_CORES;
    s_ctl_set_cores_ip.u4_num_cores = MIN(pVidDec->numCores,
                                          MPEG4_MAX_NUM_CORES);
    s_ctl_set_cores_ip.u4_size = sizeof(imp4d_cxa8_ctl_set_num_cores_ip_t);
    s_ctl_set_cores_op.u4_size = sizeof(imp4d_cxa8_ctl_set_num_cores_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(
                    pVidDec->ps_codec, (void *)&s_ctl_set_cores_ip,
                    (void *)&s_ctl_set_cores_op);

    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_ERROR("Error in Setting number of cores\n");
        return (e_dec_status);
    }
    return (e_dec_status);
}

void ittiam_video_decoder_deinit(VIDDECTYPE* pVidDec)
{

    iv_retrieve_mem_rec_ip_t s_clear_dec_ip;
    iv_retrieve_mem_rec_op_t s_clear_dec_op;
    UWORD32 u4_num_mem_recs;
    s_clear_dec_ip.pv_mem_rec_location = pVidDec->pMemRecs;

    if((NULL == pVidDec->ps_codec) || (NULL == pVidDec->pMemRecs))
        return;

    s_clear_dec_ip.e_cmd = IV_CMD_RETRIEVE_MEMREC;
    s_clear_dec_ip.u4_size = sizeof(iv_retrieve_mem_rec_ip_t);
    s_clear_dec_op.u4_size = sizeof(iv_retrieve_mem_rec_op_t);

    pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec, (void *)&s_clear_dec_ip,
                                     (void *)&s_clear_dec_op);

    {
        iv_mem_rec_t *ps_mem_rec;
        UWORD16 u2_i;

        u4_num_mem_recs = s_clear_dec_op.u4_num_mem_rec_filled;

        ps_mem_rec = s_clear_dec_ip.pv_mem_rec_location;

        for(u2_i = 0; u2_i < u4_num_mem_recs; u2_i++)
        {
            if(ps_mem_rec->pv_base)
            {
                OMX_OSAL_Free(ps_mem_rec->pv_base);
                ps_mem_rec->pv_base = NULL;
            }
            ps_mem_rec++;
        }
        if(pVidDec->pMemRecs)
        {
            OMX_OSAL_Free(pVidDec->pMemRecs);
            pVidDec->pMemRecs = NULL;
        }
    }
    ITTIAM_DEBUG("Codec de-initialization complete");

}

void ittiam_video_decoder_flush(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_ctl_flush_ip_t s_video_flush_ip;
    ivd_ctl_flush_op_t s_video_flush_op;
    int32_t width, height, stride;
    void *dummy_out_buf;


    ITTIAM_LOG("Flush called");



    /*****************************************************************************/
    /*   API Call: Video Flush                                                  */
    /*****************************************************************************/
    e_dec_status = ittiam_video_set_flush_mode(pVidDec);

    if(e_dec_status != IV_SUCCESS)
        return;

    while(IV_SUCCESS == e_dec_status)
    {
        ivd_video_decode_ip_t s_video_decode_ip;
        ivd_video_decode_op_t s_video_decode_op;
        ittiam_video_set_decode_args(pVidDec, &s_video_decode_ip, &s_video_decode_op,
                                     NULL, NULL, 0);

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->ps_codec, (void *)&s_video_decode_ip,
                        (void *)&s_video_decode_op);
    }
    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));
    pVidDec->prevTimeStamp = -1;
    pVidDec->ignoreInitialBPics = 1;
    return;
}

void FlushOutputBuffers(VIDDECTYPE* pVidDec)
{
    UWORD32 i;
    pthread_mutex_lock(&pVidDec->signal_mutex);
    pVidDec->flushInProgress = 1;
    data_sync_barrier();
    pthread_mutex_unlock(&pVidDec->signal_mutex);
    for(i = 0; i < pVidDec->nBufferCountActual; i++)
    {
        if(pVidDec->sOutBufList.pBufHdr[i] != NULL)
        {
            if(pVidDec->sOutBufList.pBufHdr[i]->nFlags & CUSTOM_BUFFERFLAG_OWNED)
            {
                pVidDec->sOutBufList.pBufHdr[i]->nFlags =
                                pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                                & (~CUSTOM_BUFFERFLAG_OWNED);
                pVidDec->codecBufCnt--;
                pVidDec->sOutBufList.pBufHdr[i]->nFilledLen = 0;
                FillBufferDone(
                                pVidDec->hSelf, pVidDec->pAppData,
                                pVidDec->sOutBufList.pBufHdr[i]);

            }
        }
        else{
            ITTIAM_ERROR("Trying to call Fill Bufferdone on a freed buffer, Somthing went wrong");
        }
    }
    while(pVidDec->sOutBufList.nSizeOfList)
    {
        OMX_BUFFERHEADERTYPE  *pBufferHdr;
        ListGetEntry(pVidDec->sOutBufList, pBufferHdr);
    }
    pVidDec->codecBufCnt = 0;
    pthread_mutex_lock(&pVidDec->signal_flush_mutex);
    pVidDec->flushInProgress = 0;
    data_sync_barrier();
    pthread_cond_signal(&pVidDec->buffers_flush_signal);
    pthread_mutex_unlock(&pVidDec->signal_flush_mutex);
}
IV_API_CALL_STATUS_T ittiam_video_decoder_init(VIDDECTYPE* pVidDec)
{

    IV_API_CALL_STATUS_T e_dec_status;
    UWORD32 u4_num_mem_recs;
    UWORD32 u4_num_reorder_frames;
    UWORD32 u4_num_ref_frames;

    /* Map OMX eColorFormat to Ittiam Color Format enum */
    switch((WORD32)pVidDec->sOutPortDef.format.video.eColorFormat)
    {
        /* YUV/YVU 420 Planar */
        case IOMX_COLOR_FormatYVU420Planar:
        case OMX_COLOR_FormatYUV420Planar:
            pVidDec->e_output_format = IV_YUV_420P;
            ITTIAM_DEBUG("Codec Color Format IV_YUV_420P");
            break;

        /* YUV 420 Semi Planar */
        case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
        case IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case OMX_COLOR_FormatYUV420Flexible:
            pVidDec->e_output_format = IV_YUV_420SP_UV;
            ITTIAM_DEBUG("Codec Color Format IV_YUV_420SP_UV");
            break;

        /* YVU 420 Semi Planar */
        case IOMX_COLOR_FormatYVU420SemiPlanar:
        case OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
            pVidDec->e_output_format = IV_YUV_420SP_VU;
            ITTIAM_DEBUG("Codec Color Format IV_YUV_420SP_VU");
            break;

        default:
            ITTIAM_ERROR("Unknown Color Format %8x", pVidDec->sOutPortDef.format.video.eColorFormat);
            return OMX_ErrorUndefined;
    }

    {
        iv_num_mem_rec_ip_t s_no_of_mem_rec_query_ip;
        iv_num_mem_rec_op_t s_no_of_mem_rec_query_op;

        //s_no_of_mem_rec_query_ip.u4_size = sizeof(iv_num_mem_rec_ip_t);
        s_no_of_mem_rec_query_ip.u4_size = sizeof(s_no_of_mem_rec_query_ip);
        s_no_of_mem_rec_query_op.u4_size = sizeof(s_no_of_mem_rec_query_op);
        s_no_of_mem_rec_query_ip.e_cmd = IV_CMD_GET_NUM_MEM_REC;

        /*****************************************************************************/
        /*   API Call: Get Number of Mem Records                                     */
        /*****************************************************************************/
        ITTIAM_DEBUG("Get Number of Mem Records");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->ps_codec, (void*)&s_no_of_mem_rec_query_ip,
                        (void*)&s_no_of_mem_rec_query_op);
        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in get mem records");
            return (e_dec_status);
        }

        u4_num_mem_recs = s_no_of_mem_rec_query_op.u4_num_mem_rec;
    }

    pVidDec->pMemRecs = (iv_mem_rec_t*)OMX_OSAL_Malloc(u4_num_mem_recs * sizeof(iv_mem_rec_t));
    if(pVidDec->pMemRecs == NULL)
    {
        ITTIAM_LOG("\nAllocation failure\n");
        e_dec_status = IV_FAIL;
        return (e_dec_status);
    }


    {

            /* Initialize number of ref and reorder modes (for H264 and HEVC). In case of thumb set it to 1 ref and 0 reorder */
            u4_num_reorder_frames = 16;
            u4_num_ref_frames = 16;

            if(pVidDec->ThumbnailMode)
            {
                u4_num_reorder_frames = 0;
                u4_num_ref_frames = 1;
            }

    }

    {
        UWORD32 i;
        iv_fill_mem_rec_ip_t s_fill_mem_rec_ip;
        iv_fill_mem_rec_op_t s_fill_mem_rec_op;
        imp4d_cxa8_fill_mem_rec_ip_t s_mp4d_fill_mem_rec_ip_t;
        iv_mem_rec_t *ps_mem_rec;
        iv_fill_mem_rec_ip_t *s_fill_mem_rec_ip_ptr;

        switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
        {
        case OMX_VIDEO_CodingMPEG4:
        case OMX_VIDEO_CodingH263:
        case QOMX_VIDEO_CodingDivx:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_mp4d_fill_mem_rec_ip_t;
                s_mp4d_fill_mem_rec_ip_t.s_ivd_fill_mem_rec_ip_t.u4_size = sizeof(imp4d_cxa8_fill_mem_rec_ip_t);
                s_mp4d_fill_mem_rec_ip_t.u4_share_disp_buf = pVidDec->shareDispBuf;
                s_mp4d_fill_mem_rec_ip_t.e_output_format = pVidDec->e_output_format;


            }
            break;
        default:
            {
                s_fill_mem_rec_ip_ptr = (iv_fill_mem_rec_ip_t *)&s_fill_mem_rec_ip;
                s_fill_mem_rec_ip.u4_size = sizeof(iv_fill_mem_rec_ip_t);
            }
            break;
        }

        s_fill_mem_rec_ip_ptr->e_cmd = IV_CMD_FILL_NUM_MEM_REC;
        s_fill_mem_rec_ip_ptr->pv_mem_rec_location = pVidDec->pMemRecs;
        s_fill_mem_rec_ip_ptr->u4_max_frm_wd = pVidDec->initWidth;

        s_fill_mem_rec_ip_ptr->u4_max_frm_ht = pVidDec->initHeight;
        s_fill_mem_rec_op.u4_size = sizeof(iv_fill_mem_rec_op_t);

        ps_mem_rec = pVidDec->pMemRecs;
        for(i = 0; i < u4_num_mem_recs; i++)
            ps_mem_rec[i].u4_size = sizeof(iv_mem_rec_t);

        /*****************************************************************************/
        /*   API Call: Fill Mem Records                                              */
        /*****************************************************************************/

        ITTIAM_DEBUG("Fill Mem Records");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->ps_codec, (void *)s_fill_mem_rec_ip_ptr,
                        (void *)&s_fill_mem_rec_op);

        u4_num_mem_recs = s_fill_mem_rec_op.u4_num_mem_rec_filled;

        if(IV_SUCCESS != e_dec_status)
        {
            ITTIAM_ERROR("Error in fill mem records");
            return (e_dec_status);
        }

        ps_mem_rec = pVidDec->pMemRecs;

        for(i = 0; i < u4_num_mem_recs; i++)
        {

            ps_mem_rec->pv_base = OMX_OSAL_Memalign(ps_mem_rec->u4_mem_alignment,
                                           ps_mem_rec->u4_mem_size);
            if(ps_mem_rec->pv_base == NULL)
            {
                ITTIAM_LOG("\nAllocation failure for size %d alignment %d\n", ps_mem_rec->u4_mem_size, ps_mem_rec->u4_mem_alignment);
                e_dec_status = IV_FAIL;
                return (e_dec_status);

            }

            ps_mem_rec++;
        }
    }
    {
        ivd_init_ip_t s_init_ip;
        ivd_init_op_t s_init_op;
        imp4d_cxa8_init_ip_t s_mp4d_init_ip_t;

        ivd_init_ip_t *s_init_ip_ptr;

        void *dec_fxns = (void *)pVidDec->iVdec_cxa8_api_function;
        iv_mem_rec_t *mem_tab;

        mem_tab = (iv_mem_rec_t*)pVidDec->pMemRecs;

        switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
        {
        case OMX_VIDEO_CodingMPEG4:
        case OMX_VIDEO_CodingH263:
        case QOMX_VIDEO_CodingDivx:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_mp4d_init_ip_t;
                s_mp4d_init_ip_t.s_ivd_init_ip_t.u4_size =
                                sizeof(imp4d_cxa8_init_ip_t);
                s_mp4d_init_ip_t.u4_share_disp_buf = pVidDec->shareDispBuf;
            }
            break;
        default:
            {
                s_init_ip_ptr = (ivd_init_ip_t *)&s_init_ip;
                s_init_ip.u4_size = sizeof(ivd_init_ip_t);
            }
            break;
        }

        s_init_ip_ptr->e_cmd = (IVD_API_COMMAND_TYPE_T)IV_CMD_INIT;
        s_init_ip_ptr->pv_mem_rec_location = mem_tab;
        s_init_ip_ptr->u4_frm_max_wd = pVidDec->initWidth;
        s_init_ip_ptr->u4_frm_max_ht = pVidDec->initHeight;

        s_init_op.u4_size = sizeof(ivd_init_op_t);

        s_init_ip_ptr->u4_num_mem_rec = u4_num_mem_recs;
        s_init_ip_ptr->e_output_format = pVidDec->e_output_format;

        pVidDec->ps_codec = (iv_obj_t*)mem_tab[0].pv_base;
        pVidDec->ps_codec->pv_fxns = dec_fxns;
        pVidDec->ps_codec->u4_size = sizeof(iv_obj_t);
        /*****************************************************************************/
        /*   API Call: Initialize the Decoder                                        */
        /*****************************************************************************/

        ITTIAM_DEBUG("Initialize the Decoder");
        e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                        (void *)s_init_ip_ptr,
                                                        (void *)&s_init_op);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in Init %x", s_init_op.u4_error_code);
            return (e_dec_status);
        }
    }
    switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
    {
        case OMX_VIDEO_CodingAVC:
            e_dec_status = iv_h264d_set_numcores(pVidDec);
            if(IV_SUCCESS != e_dec_status)
                return e_dec_status;
            break;
        case OMX_VIDEO_CodingHEVC:
            e_dec_status = iv_hevcd_set_numcores(pVidDec);
            if(IV_SUCCESS != e_dec_status)
                return e_dec_status;
            break;
        case OMX_VIDEO_CodingVP9:
            // Number of cores already set during create for VP9 decoder.
            break;
        case OMX_VIDEO_CodingMPEG4:
        case OMX_VIDEO_CodingH263:
        case QOMX_VIDEO_CodingDivx:
            e_dec_status = iv_mpeg4d_set_numcores(pVidDec);
            if(IV_SUCCESS != e_dec_status)
                return e_dec_status;
            break;
        case OMX_VIDEO_CodingMPEG2:
            e_dec_status = iv_mpeg2d_set_numcores(pVidDec);
            if(IV_SUCCESS != e_dec_status)
                return e_dec_status;
            break;
        default:
            break;
    }
    /*****************************************************************************/
    /*   API Call: Set the run time (dynamics) parameters before the Process call*/
    /*****************************************************************************/
    ITTIAM_DEBUG("Set the run time (dynamics) parameters ");
    e_dec_status = ittiam_video_set_params(pVidDec, IVD_DECODE_HEADER);
    if(e_dec_status != IV_SUCCESS)
        return (e_dec_status);

    pVidDec->ignoreInitialBPics = 1;
    pVidDec->prevTimeStamp = -1;
    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));

    /* Get the codec version */
    ittiam_video_log_codec_version(pVidDec);

    return IV_SUCCESS;

}

void ittiam_video_get_frame_dimensions(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T ret;
    switch((int)pVidDec->sInPortDef.format.video.eCompressionFormat)
    {

    default:
        break;
    }
    return;
}
//DRC Changes
/*****************************************************************************/
/*  Function Name : ittiam_video_decoder_reset                             */
/*  Description   : Called from the component thread,                                                           */
/*                           If Dynamic resolution changes occur                                                  */
/*                        while the component is in the executing state. */
/*  Inputs        :                                                          */
/*  Globals       :                                                          */
/*  Processing    :                                                          */
/*  Outputs       :                                                          */
/*  Returns       :                                                          */
/*  Issues        :                                                          */
/*  Revision History:                                                        */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         09 03 2010   Ittiam          Draft                                */
/*****************************************************************************/
void ittiam_video_decoder_reset(VIDDECTYPE* pVidDec)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_ctl_reset_ip_t s_ctl_reset_ip_t;
    ivd_ctl_reset_op_t s_ctl_reset_op_t;
    s_ctl_reset_ip_t.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_reset_ip_t.e_sub_cmd = IVD_CMD_CTL_RESET;
    s_ctl_reset_ip_t.u4_size = sizeof(ivd_ctl_reset_ip_t);
    s_ctl_reset_op_t.u4_size = sizeof(ivd_ctl_reset_op_t);

    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_reset_ip_t,
                                                    (void *)&s_ctl_reset_op_t);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_DEBUG("Error: Control Call for codec reset failed\n");
    }
}

IV_API_CALL_STATUS_T ittiam_video_set_params(VIDDECTYPE* pVidDec, IVD_VIDEO_DECODE_MODE_T e_mode)
{
    IV_API_CALL_STATUS_T e_dec_status;
    ivd_ctl_set_config_ip_t s_ctl_dec_ip;
    ivd_ctl_set_config_op_t s_ctl_dec_op;

    s_ctl_dec_ip.u4_disp_wd = pVidDec->stride;
    s_ctl_dec_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_dec_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_dec_ip.e_vid_dec_mode = e_mode;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_set_config_op_t);


    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_ctl_dec_ip,
                                                    (void *)&s_ctl_dec_op);
    if(IV_SUCCESS != e_dec_status)
    {
        ITTIAM_ERROR("Error in Setting the run time (dynamics) parameters e_dec_status = %d u4_error_code = %x\n",
          e_dec_status, s_ctl_dec_op.u4_error_code);
    }

    return e_dec_status;
}

//DRC Changes
void ittiam_video_decoder_process(VIDDECTYPE* pVidDec,
                                  OMX_BUFFERHEADERTYPE *pInBufHdr,
                                  OMX_BUFFERHEADERTYPE *pOutBufHdr)
{
    IV_API_CALL_STATUS_T e_dec_status;

    ivd_video_decode_ip_t s_video_decode_ip;
    ivd_video_decode_op_t s_video_decode_op;
    UWORD32 u4_op_len_produced;

    WORD32 header_decode_error = 0;
    UWORD32 i;
    UWORD32 timeStampIdx;
    UWORD32 hdr_decode_done;
    struct timeval startTime, stopTime;
    WORD32 timeTaken, timeDelay;



    if((pInBufHdr) && (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS))
    {
        ITTIAM_DEBUG("EOS seen on input");
        pVidDec->receivedEOS = 1;
        if (pInBufHdr->nFilledLen == 0)
        {
            ittiam_video_set_flush_mode(pVidDec);
            pInBufHdr = NULL;
        }
    }

    hdr_decode_done = 0;
    timeStampIdx = 0;

    if(pInBufHdr && (0 == pVidDec->isInFlush))
    {

        ITTIAM_DEBUG("Input Timestamp %lld nFilledLen %d nOffset %d nFlags 0x%x",
                     pInBufHdr->nTimeStamp,
                    (WORD32)pInBufHdr->nFilledLen,
                    (WORD32)pInBufHdr->nOffset,
                    (WORD32)pInBufHdr->nFlags);

        for(timeStampIdx = 0; timeStampIdx < MAX_TIMESTAMP_CNT; timeStampIdx++)
        {
            if(pVidDec->isTimeStampValid[timeStampIdx] == 0)
            {
                break;
            }
        }
        if(timeStampIdx == MAX_TIMESTAMP_CNT)
        {
            ITTIAM_ERROR("Unable to find free slot to hold timestamp overwriting last entry");
            timeStampIdx = MAX_TIMESTAMP_CNT - 1;
        }
        pVidDec->isTimeStampValid[timeStampIdx] = 1;
        pVidDec->nTimeStamp[timeStampIdx] = pInBufHdr->nTimeStamp;
    }

    if(pVidDec->hdrDecodeDone == 0)
    {
        ITTIAM_DEBUG("Setting codec in Header Decode mode with stride %d", pVidDec->stride);
        ittiam_video_set_params(pVidDec, IVD_DECODE_HEADER);
    }

    ittiam_video_set_decode_args(pVidDec, &s_video_decode_ip, &s_video_decode_op,
                                 pInBufHdr, pOutBufHdr, timeStampIdx);

    /*****************************************************************************/
    /*   API Call: Video Decode                                                  */
    /*****************************************************************************/
    //If input dump is enabled, dump input bitstream
    DUMP_INPUT(INPUT_DUMP_PATH, s_video_decode_ip.pv_stream_buffer, s_video_decode_ip.u4_num_Bytes);

    reprocess:
    s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);
    GETTIME(&startTime);
    e_dec_status = pVidDec->iVdec_cxa8_api_function(pVidDec->ps_codec,
                                                    (void *)&s_video_decode_ip,
                                                    (void *)&s_video_decode_op);
    GETTIME(&stopTime);



    ELAPSEDTIME(startTime, stopTime, timeTaken);
    ELAPSEDTIME(pVidDec->prevStopTime, startTime, timeDelay);

    if(pVidDec->ignoreInitialBPics && s_video_decode_op.u4_output_present)
    {
        if(IV_B_FRAME == s_video_decode_op.e_pic_type)
        {
            ITTIAM_LOG("Ignoring B picture to handle open GOP cases");
            s_video_decode_op.u4_output_present = 0;
            s_video_decode_op.u4_frame_decoded_flag = 0;
        }
        else
        {
            pVidDec->ignoreInitialBPics = 0;
        }
    }
    /* In case of shared mode, if the decoder is waiting for buffers, then return to processing after acquiring buffer */
    if((pVidDec->shareDispBuf) &&
       ((s_video_decode_op.u4_error_code & 0xFF) == IVD_DEC_REF_BUF_NULL))
    {
        if(pVidDec->cmd_pending == 1)
            return;

        ittiam_video_release_display_frames(pVidDec);
        goto reprocess;
    }

    if((pVidDec->hdrDecodeDone == 0) && (e_dec_status != IV_SUCCESS))
    {
        /* If stream width * height is greater than maxWd * maxHt, then
           recreate the codec instance */
        if((s_video_decode_op.u4_pic_wd * s_video_decode_op.u4_pic_ht) >
           (pVidDec->initWidth * pVidDec->initHeight))
        {
            ITTIAM_LOG("Stream width and height are greater than create width and height");
            pVidDec->initWidth = s_video_decode_op.u4_pic_wd;
            pVidDec->initHeight = s_video_decode_op.u4_pic_ht;
            ittiam_video_decoder_deinit(pVidDec);
            pVidDec->initDone = 0;
            pVidDec->stride = ALIGN32(pVidDec->initWidth + pVidDec->padWidth);
            ITTIAM_LOG("Reinitializing the codec instance");
            ITTIAM_LOG("New Width %d",s_video_decode_op.u4_pic_wd);
            ITTIAM_LOG("New Height %d",s_video_decode_op.u4_pic_ht);
            e_dec_status = ittiam_video_decoder_init(pVidDec);
            if(IV_SUCCESS != e_dec_status)
            {
                pVidDec->pCallbacks->EventHandler(
                                pVidDec->hSelf,
                                pVidDec->pAppData,
                                OMX_EventError,
                                OMX_ErrorInvalidState,
                                OMX_StateInvalid,
                                NULL);
            }
            else
            {
                WORD32 width = s_video_decode_op.u4_pic_wd;
                WORD32 height = s_video_decode_op.u4_pic_ht;
                pVidDec->initDone = 1;
                if(pVidDec->shareDispBuf)
                {
                    pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN32(s_video_decode_op.u4_pic_wd + pVidDec->padWidth);

                    pVidDec->sOutPortDef.format.video.nStride = pVidDec->sOutPortDef.format.video.nFrameWidth;
                    pVidDec->stride = pVidDec->sOutPortDef.format.video.nFrameWidth;
                    pVidDec->sOutPortDef.format.video.nFrameHeight = (s_video_decode_op.u4_pic_ht) + (pVidDec->padHeight);
                    if(IOMXQ_COLOR_FormatYUV420PackedSemiPlanar32m == pVidDec->sOutPortDef.format.video.eColorFormat)
                    {
                        pVidDec->sOutPortDef.format.video.nFrameWidth = ALIGN128(pVidDec->nDispWidth + pVidDec->padWidth);
                        pVidDec->sOutPortDef.format.video.nFrameHeight = ALIGN32(pVidDec->sOutPortDef.format.video.nFrameHeight);
                    }

                    pVidDec->sOutPortDef.format.video.nSliceHeight = pVidDec->sOutPortDef.format.video.nFrameHeight;
                    pVidDec->sOutPortDef.nBufferSize = (pVidDec->sOutPortDef.format.video.nFrameWidth) *
                    (pVidDec->sOutPortDef.format.video.nFrameHeight +
                                    ((pVidDec->sOutPortDef.format.video.nFrameHeight + 1) / 2));

                    pVidDec->sOutPortDef.nBufferSize = ALIGN4096(pVidDec->sOutPortDef.nBufferSize);

                    pVidDec->PortReconfiguration = 1;
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventPortSettingsChanged,
                                                      0x1, 0, NULL);
                    FlushOutputBuffers(pVidDec);
                    pVidDec->isInFlush = 0;
                    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));
                    pVidDec->prevTimeStamp = -1;
                    pVidDec->ignoreInitialBPics = 1;
                    return;
                }


                handle_port_settings_changed(pVidDec, width, height, pOutBufHdr);
                if(pOutBufHdr)
                    pOutBufHdr = NULL;
                return;

            }

        }
    }
    ITTIAM_STATISTICS("TimeTaken(us) = %-6d DelayBetweenCalls = %-6d BytesConsumed = %-6d", timeTaken, timeDelay, s_video_decode_op.u4_num_bytes_consumed);


    if(pVidDec->disableInterlaced)
    {
        if((e_dec_status == IV_SUCCESS) && (pVidDec->hdrDecodeDone == 0))
        {
            if(0 == s_video_decode_op.u4_progressive_frame_flag)
            {
                ITTIAM_ERROR("Found interlaced stream which is not supported");
                e_dec_status = IV_FAIL;
                s_video_decode_op.u4_error_code = 1 << IVD_FATALERROR;
                s_video_decode_op.u4_error_code |= 1 << IVD_UNSUPPORTEDINPUT;
                s_video_decode_op.u4_pic_wd = 0;
                s_video_decode_op.u4_pic_ht = 0;
            }
        }
    }

    /* In case of thumbnail mode, call flush and get the first decoded frame out, if frame is decoded and not returned */
    if((pVidDec->ThumbnailMode == 1) &&
        (s_video_decode_op.u4_output_present == 0) &&
        (s_video_decode_op.u4_frame_decoded_flag == 1))
    {

        e_dec_status = ittiam_video_set_flush_mode(pVidDec);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_ERROR("Error in setting the decoder in flush mode");
        }

        e_dec_status = pVidDec->iVdec_cxa8_api_function(
                        pVidDec->ps_codec, (void *)&s_video_decode_ip,
                        (void *)&s_video_decode_op);
        if(e_dec_status != IV_SUCCESS)
        {
            ITTIAM_LOG("Video Decode  in flush failed e_dec_status=%d u4_error_code =%d",
                  e_dec_status, s_video_decode_op.u4_error_code);
            memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
            memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));
            pVidDec->prevTimeStamp = -1;
            pVidDec->ignoreInitialBPics = 1;
            //Prevent the process function to be called again
            pthread_mutex_lock(&pVidDec->signal_mutex);
            pVidDec->cmd_pending = 1;
            pthread_mutex_unlock(&pVidDec->signal_mutex);

            return;

        }
        else
        {
            ITTIAM_LOG("Video Decode  in flush output present %d",
                  s_video_decode_op.u4_output_present);

        }
    }


    if(pInBufHdr)
        pInBufHdr->nOffset += s_video_decode_op.u4_num_bytes_consumed;
    if(1 != s_video_decode_op.u4_frame_decoded_flag)
    {
        /* If the input did not contain picture data, then ignore the associated timestamp */
        pVidDec->isTimeStampValid[timeStampIdx] = 0;
    }
    if(s_video_decode_op.u4_pic_wd > 0 && s_video_decode_op.u4_pic_ht > 0)
        hdr_decode_done = 1;



    {
        //MediaBuffer *mbuf = drainOutputBuffer();
        UWORD32 width, height;
        OMX_BUFFERHEADERTYPE *pBufferHdr;
        width = s_video_decode_op.u4_pic_wd;
        height = s_video_decode_op.u4_pic_ht;
        pVidDec->prevStopTime = stopTime;
        {
            if(((s_video_decode_op.u4_error_code & 0xFF) == pVidDec->mDRCError) ||
               ((s_video_decode_op.u4_error_code & 0xFF) == pVidDec->mUnsupportedReslnError))
            {
                ittiam_video_set_flush_mode(pVidDec);

                //ittiam_video_decoder_reset(pVidDec);
                //ittiam_video_set_params(pVidDec, IVD_DECODE_HEADER);
                //pVidDec->hdrDecodeDone = 0;
                if((s_video_decode_op.u4_error_code & 0xFF) == pVidDec->mUnsupportedReslnError)
                {

                    pVidDec->reInitPending = 1;
                    pVidDec->reInitWidth = s_video_decode_op.u4_pic_wd;
                    pVidDec->reInitHeight = s_video_decode_op.u4_pic_ht;
                    ITTIAM_ERROR("Reinit Pending with width %d height %d", pVidDec->reInitWidth, pVidDec->reInitHeight);
                }

                // if(pInBufHdr)
                    // pInBufHdr->nOffset = 0; //In DRC same buffer needs to be passed again.
                if(pInBufHdr)
                    pInBufHdr->nOffset = 0; //In DRC same buffer needs to be passed again.

                goto reprocess;

            }

            if( ((width > 0) || (height > 0)) &&
                ((width != pVidDec->nDispWidth) || (height != pVidDec->nDispHeight)))
            {

                handle_port_settings_changed(pVidDec, width, height, pOutBufHdr);
                if(pOutBufHdr)
                    pOutBufHdr = NULL;
                if(pInBufHdr)
                    pInBufHdr->nOffset = 0; //In DRC same buffer needs to be passed again.
                return;

            }
        }

        if(pVidDec->hdrDecodeDone == 0 && hdr_decode_done == 1)
        {
            ittiam_video_get_frame_dimensions(pVidDec);

            if(pVidDec->shareDispBuf)
                ittiam_video_set_display_frame(pVidDec);

            /* Set the decoder in frame decode mode */
            e_dec_status = ittiam_video_set_params(pVidDec, IVD_DECODE_FRAME);



            pVidDec->hdrDecodeDone = 1;
            data_sync_barrier();
            if((0 == pVidDec->shareDispBuf) && (pOutBufHdr))

            {
                pOutBufHdr->nTimeStamp = 0;

                pOutBufHdr->nFilledLen = 0;
                pVidDec->NumFBD++;

                pOutBufHdr->nFlags = pOutBufHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);

                FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pOutBufHdr);
                pOutBufHdr = NULL;
            }

            return;
        }

        if(1 == pVidDec->shareDispBuf)
        {
            pBufferHdr = NULL;
            if(s_video_decode_op.u4_output_present == 1)
            {
                pBufferHdr = pVidDec->sOutBufList.pBufHdr[s_video_decode_op.u4_disp_buf_id];
                if(!(pBufferHdr->nFlags & CUSTOM_BUFFERFLAG_OWNED))
                {
                    ITTIAM_ERROR("Codec returing a buffer it does not own");
                    return;
                }
                pVidDec->codecBufCnt--;


            }
            else if(pVidDec->isInFlush)
            {
                for(i = 0; i < pVidDec->sOutPortDef.nBufferCountActual; i++)
                {
                    if(pVidDec->sOutBufList.pBufHdr[i]->nFlags
                                    & CUSTOM_BUFFERFLAG_OWNED)
                    {
                        pBufferHdr = pVidDec->sOutBufList.pBufHdr[i];
                        break;
                    }
                }
            }
        }
        else
        {
            pBufferHdr = pOutBufHdr;
            pOutBufHdr = NULL;
        }
        if(s_video_decode_op.u4_output_present == 0)
        {
            if(s_video_decode_op.u4_error_code)
            {
                ITTIAM_DEBUG("Video Decode  get output failed e_dec_status=%d u4_error_code =%x", e_dec_status, s_video_decode_op.u4_error_code);

                if(pVidDec->reInitPending)
                {
                    pVidDec->initWidth = pVidDec->reInitWidth;
                    pVidDec->initHeight = pVidDec->reInitHeight;
                    ittiam_video_decoder_deinit(pVidDec);
                    pVidDec->initDone = 0;
                    pVidDec->hdrDecodeDone = 0;

                    pVidDec->stride = ALIGN32(pVidDec->reInitWidth + pVidDec->padWidth);
                    ITTIAM_LOG("Reinitializing the codec instance stride %d", pVidDec->stride);
                    ITTIAM_DEBUG("New Width %d",pVidDec->initWidth);
                    ITTIAM_DEBUG("New Height %d",pVidDec->initHeight);

                    e_dec_status = ittiam_video_decoder_init(pVidDec);
                    pVidDec->reInitPending = 0;
                    if(IV_SUCCESS != e_dec_status)
                    {
                        pVidDec->pCallbacks->EventHandler(
                                        pVidDec->hSelf,
                                        pVidDec->pAppData,
                                        OMX_EventError,
                                        OMX_ErrorInvalidState,
                                        OMX_StateInvalid,
                                        NULL);
                    }
                    else
                    {
                        pVidDec->initDone = 1;
                        if(pBufferHdr)
                        {
                            pBufferHdr->nFlags = pBufferHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);
                            FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pBufferHdr);
                            //return;
                        }

                        return;

                    }
                }
                if(pVidDec->shareDispBuf)
                {
                    ittiam_video_decoder_reset(pVidDec);
                    pVidDec->stride = 0;
                    ittiam_video_set_params(pVidDec, IVD_DECODE_HEADER);
                }
            }


            if(pBufferHdr)
                pBufferHdr->nFilledLen = 0;
            /* If EOS was recieved on input port and decoder is in flush
               and there is no output from the codec, then signal EOS on output port */
            if (pBufferHdr && pVidDec->receivedEOS && pVidDec->isInFlush)
            {
                ITTIAM_LOG("Setting EOS on output");
                pBufferHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                pVidDec->receivedEOS = 0;
                pVidDec->prevTimeStamp = -1;
                pVidDec->ignoreInitialBPics = 1;
                memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));
                //Prevent the process function to be called again
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 1;
                pthread_mutex_unlock(&pVidDec->signal_mutex);

            }

            /* If in flush mode and no output is returned by the codec,
             * then come out of flush mode */
            pVidDec->isInFlush = 0;

        }
        else if(pBufferHdr)
        {
            WORD32 idx = s_video_decode_op.u4_ts;

            pBufferHdr->nTimeStamp = pVidDec->nTimeStamp[idx];
            {
                WORD32 min_idx;
                min_idx = get_min_timestamp_idx(pVidDec->nTimeStamp,  pVidDec->isTimeStampValid);
                if((min_idx >= 0) && (pVidDec->nTimeStamp[min_idx] < pBufferHdr->nTimeStamp))
                {
                    idx = min_idx;
                    pBufferHdr->nTimeStamp = pVidDec->nTimeStamp[idx];
                }
            }
            pVidDec->isTimeStampValid[idx] = 0;
            pBufferHdr->nFilledLen = pVidDec->sOutPortDef.nBufferSize;

            ITTIAM_DEBUG("TIMESTAMP IDX inp_ts_idx %d out_ts_idx %d pVidDec->nTimeStamp[out_ts_idx] %lld, pBufferHdr->nTimeStamp %lld\n",
                         s_video_decode_ip.u4_ts, s_video_decode_op.u4_ts,
                         pVidDec->nTimeStamp[s_video_decode_op.u4_ts],
                         pBufferHdr->nTimeStamp);

        }

        /* If input EOS is seen and decoder is not in flush mode,
         * set the decoder in flush mode.
         * There can be a case where EOS is sent along with last picture data
         * In that case, only after decoding that input data, decoder has to be
         * put in flush. This case is handled here  */

        if (pVidDec->receivedEOS && !pVidDec->isInFlush)
        {
            ittiam_video_set_flush_mode(pVidDec);
        }

        if(e_dec_status != IV_SUCCESS)
        {
            if(pOutBufHdr)
                pOutBufHdr->nFilledLen = 0;

            if(pInBufHdr)
            {
                /* In case decoder is in error and has not consumed any input bytes,
                    then ignore them */
                if(s_video_decode_op.u4_num_bytes_consumed)
                    pInBufHdr->nOffset += s_video_decode_op.u4_num_bytes_consumed;
                else
                    pInBufHdr->nOffset = pInBufHdr->nFilledLen;
            }

            if(IV_ISFATALERROR(s_video_decode_op.u4_error_code))
            {
                ITTIAM_ERROR("Decoder in error u4_error_code = %x. Stopping decoding.\n", s_video_decode_op.u4_error_code);
                header_decode_error = 1;
                pVidDec->receivedEOS = 1;

                if(pBufferHdr)
                {
                    FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pBufferHdr);
                }

                pVidDec->pCallbacks->EventHandler(
                                pVidDec->hSelf,
                                pVidDec->pAppData,
                                OMX_EventError,
                                OMX_ErrorInvalidState,
                                OMX_StateInvalid,
                                NULL);



                //Prevent the process function to be called again
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 1;
                pthread_mutex_unlock(&pVidDec->signal_mutex);

                return;

            }
        }

        if(pBufferHdr)
        {
            pBufferHdr->nFlags = pBufferHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);
            FillBufferDone(pVidDec->hSelf, pVidDec->pAppData, pBufferHdr);
            return;
        }


    }
    if((0 == pVidDec->shareDispBuf) && (pOutBufHdr))
    {
        pOutBufHdr->nTimeStamp = 0;
        pOutBufHdr->nFilledLen = 0;
        pVidDec->NumFBD++;
        pOutBufHdr->nFlags = pOutBufHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);

        ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD, __LINE__);
        ITTIAM_DEBUG(" Calling FillBufferDone bufferhdr %p line %d timestamp %lld",
                     pOutBufHdr, __LINE__, pOutBufHdr->nTimeStamp);
        FillBufferDone(pVidDec->hSelf, pVidDec->pAppData,
                                            pOutBufHdr);
        pOutBufHdr = NULL;
    }
    return;

}

/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
void* ComponentThread(void* pThreadData)
{
    int i, fd1;
    fd_set rfds;
    WORD32 cmddata;
    ThrCmdType cmd;

    // Variables related to decoder buffer handling
    OMX_BUFFERHEADERTYPE *pInBufHdr = NULL;
    OMX_BUFFERHEADERTYPE *pOutBufHdr = NULL;
    OMX_MARKTYPE *pMarkBuf = NULL;

    // Variables related to decoder timeouts
    struct timespec abstime;
    int ret_val;
    int nTimeout;

    // Recover the pointer to my component specific data
    VIDDECTYPE* pVidDec = (VIDDECTYPE*)pThreadData;

    while(1)
    {
        fd1 = pVidDec->cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);

        // Check for new command
        i = select(pVidDec->cmdpipe[0] + 1, &rfds, NULL, NULL, NULL);

        if(FD_ISSET(pVidDec->cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            pthread_mutex_lock(&pVidDec->pipes_mutex);
            read(pVidDec->cmdpipe[0], &cmd, sizeof(cmd));
            read(pVidDec->cmddatapipe[0], &cmddata, sizeof(cmddata));
            pthread_mutex_unlock(&pVidDec->pipes_mutex);

            // State transition command
            if(cmd == SetState)
            {
                ITTIAM_DEBUG("SetState Command");
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 0;
                pthread_mutex_unlock(&pVidDec->signal_mutex);

                // If the parameter states a transition to the same state
                //   raise a same state transition error.
                if(pVidDec->state == (OMX_STATETYPE)(cmddata))
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventError,
                                                      OMX_ErrorSameState, 0,
                                                      NULL);
                else
                {
                    // transitions/callbacks made based on state transition table
                    // cmddata contains the target state
                    switch((OMX_STATETYPE)(cmddata))
                    {
                        case OMX_StateInvalid:
                            pVidDec->state = OMX_StateInvalid;
                            data_sync_barrier();
                            pVidDec->pCallbacks->EventHandler(
                                            pVidDec->hSelf, pVidDec->pAppData,
                                            OMX_EventError,
                                            OMX_ErrorInvalidState, 0, NULL);
                            pVidDec->pCallbacks->EventHandler(
                                            pVidDec->hSelf, pVidDec->pAppData,
                                            OMX_EventCmdComplete,
                                            OMX_CommandStateSet, pVidDec->state,
                                            NULL);
                            break;
                        case OMX_StateLoaded:
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state
                                                            == OMX_StateWaitForResources)
                            {
                                ret_val = 0;

                                pthread_mutex_lock(&pVidDec->signal_mutex);

                                // Transition happens only when the ports are unpopulated
                                if((pVidDec->sInBufList.nAllocSize > 0)
                                                || pVidDec->sOutBufList.nAllocSize
                                                                > 0)
                                {

                                    //gettimeofday(&abstime,NULL);
                                    clock_gettime(CLOCK_REALTIME, &abstime);
                                    abstime.tv_sec += OMX_TIMEOUT_SEC;
                                    ITTIAM_DEBUG("Waiting for buffers to be freed during setState to Loaded");
                                    ret_val = pthread_cond_timedwait(
                                                    &pVidDec->buffers_signal,
                                                    &pVidDec->signal_mutex,
                                                    &abstime);

                                }
                                pthread_mutex_unlock(&pVidDec->signal_mutex);

                                if(ret_val == 0)
                                {
                                    // DeInitialize the decoder when moving from Idle to Loaded
                                    if(pVidDec->state == OMX_StateIdle
                                                    && pVidDec->initDone == 1)
                                    {

                                        ittiam_video_decoder_deinit(pVidDec);
                                        ComponentReset(pVidDec);
                                        pVidDec->initDone = 0;

                                    }

                                    pVidDec->state = OMX_StateLoaded;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pVidDec->state, NULL);

                                }
                                else
                                {
                                    // Transition to loaded failed
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventError,
                                                    OMX_ErrorTimeout, 0, NULL);

                                }
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateIdle:

                            if(pVidDec->state == OMX_StateInvalid)
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            else
                            {
                                // Return buffers if currently in pause and executing
                                if(pVidDec->state == OMX_StatePause
                                                || pVidDec->state
                                                                == OMX_StateExecuting)
                                {
                                    if(pInBufHdr)
                                    {
                                        // Return input buffer to client for refill
                                        pVidDec->pCallbacks->EmptyBufferDone(
                                                        pVidDec->hSelf,
                                                        pVidDec->pAppData,
                                                        pInBufHdr);
                                        pInBufHdr = NULL;
                                    }
                                    ittiam_video_decoder_flush(pVidDec);
                                    ListFlushEntries(pVidDec->sInBufList,
                                                     pVidDec);

                                    ITTIAM_DEBUG("Calling ListFlush or FlushOut");
                                    if(1 == pVidDec->shareDispBuf)
                                        FlushOutputBuffers(pVidDec);
                                    else
                                        ListFlushEntries(pVidDec->sOutBufList,
                                                         pVidDec);
                                    pVidDec->isInFlush = 0;
                                    pVidDec->prevTimeStamp = -1;
                                    pVidDec->ignoreInitialBPics = 1;
                                    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                                    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));

                                    ITTIAM_DEBUG("Calling ListFlush or FlushOut done");

                                }

                                ret_val = 0;
                                ITTIAM_DEBUG("pthread_mutex_lock for signal");
                                pthread_mutex_lock(&pVidDec->signal_mutex);

                                // All "enabled" Ports have to be "populated", before transition completes
                                if((pVidDec->sInPortDef.bEnabled
                                                != pVidDec->sInPortDef.bPopulated)
                                                || (pVidDec->sOutPortDef.bEnabled
                                                                != pVidDec->sOutPortDef.bPopulated))
                                {
                                    pVidDec->bufferAllocationPending = 1;
                                    data_sync_barrier();
                                    ITTIAM_DEBUG("Buffer Allocation is pending ");
                                    if(pVidDec->state == OMX_StateLoaded)
                                    {
                                       ret_val = pthread_cond_wait(
                                                    &pVidDec->buffers_signal,
                                                    &pVidDec->signal_mutex);
                                    }

                                }
                                ITTIAM_DEBUG("pthread_mutex_unlock for signal");
                                pthread_mutex_unlock(&pVidDec->signal_mutex);

                                if(pVidDec->bufferAllocationPending)
                                {
                                    pVidDec->state = OMX_StateInvalid;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventError,
                                                    OMX_ErrorInvalidState, 0,
                                                    NULL);
                                    ITTIAM_LOG("Invalid state");
                                }
                                else
                                {
                                    // Initialize the decoder when moving from Loaded to Idle
                                    if(pVidDec->state == OMX_StateLoaded)
                                    {
                                        IV_API_CALL_STATUS_T e_dec_status;
                                        e_dec_status = ittiam_video_decoder_init(
                                                                        pVidDec);
                                        if(IV_SUCCESS != e_dec_status)
                                        {
                                            pVidDec->pCallbacks->EventHandler(
                                                            pVidDec->hSelf,
                                                            pVidDec->pAppData,
                                                            OMX_EventError,
                                                            OMX_ErrorInvalidState,
                                                            OMX_StateInvalid,
                                                            NULL);
                                        }
                                        else
                                        {
                                            pVidDec->initDone = 1;
                                        }
                                    }

                                    ITTIAM_DEBUG("Sending OMX_EventCmdComplete for Idle");
                                    pVidDec->state = OMX_StateIdle;
                                    data_sync_barrier();
                                    pVidDec->pCallbacks->EventHandler(
                                                    pVidDec->hSelf,
                                                    pVidDec->pAppData,
                                                    OMX_EventCmdComplete,
                                                    OMX_CommandStateSet,
                                                    pVidDec->state, NULL);


                                }

                            }
                            break;
                        case OMX_StateExecuting:
                            // Transition can only happen from pause or idle state
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state == OMX_StatePause)
                            {

                                // Return buffers if currently in pause
                                if(pVidDec->state == OMX_StatePause)
                                {
                                    ListFlushEntries(pVidDec->sInBufList,
                                                     pVidDec);

                                    ittiam_video_decoder_flush(pVidDec);

                                    if(1 == pVidDec->shareDispBuf)
                                        FlushOutputBuffers(pVidDec);
                                    else
                                        ListFlushEntries(pVidDec->sOutBufList,
                                                         pVidDec);
                                    pVidDec->isInFlush = 0;
                                    pVidDec->prevTimeStamp = -1;
                                    pVidDec->ignoreInitialBPics = 1;
                                    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                                    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));

                                }

                                pVidDec->state = OMX_StateExecuting;
                                data_sync_barrier();
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);

                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StatePause:
                            // Transition can only happen from idle or executing state
                            if(pVidDec->state == OMX_StateIdle
                                            || pVidDec->state
                                                            == OMX_StateExecuting)
                            {

                                pVidDec->state = OMX_StatePause;
                                data_sync_barrier();
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateWaitForResources:
                            if(pVidDec->state == OMX_StateLoaded)
                            {
                                pVidDec->state = OMX_StateWaitForResources;
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandStateSet,
                                                pVidDec->state, NULL);
                            }
                            else
                                pVidDec->pCallbacks->EventHandler(
                                                pVidDec->hSelf,
                                                pVidDec->pAppData,
                                                OMX_EventError,
                                                OMX_ErrorIncorrectStateTransition,
                                                0, NULL);
                            break;
                        case OMX_StateKhronosExtensions:
                        case OMX_StateVendorStartUnused:
                        case OMX_StateMax:
                            /* Not handled */
                            break;
                    }
                }
            }
            else if(cmd == DisablePort)
            {
                ITTIAM_DEBUG("DisablePort Command");
                // Disable Port(s)
                // cmddata contains the port index to be stopped.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be stopped.
                if(cmddata == 0x0 || cmddata == -1)
                {
                    // Return all input buffers
                    if(pInBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                             pVidDec->pAppData,
                                                             pInBufHdr);
                        pInBufHdr = NULL;
                    }
                    ListFlushEntries(pVidDec->sInBufList, pVidDec)
                    // Disable port
                    pVidDec->sInPortDef.bEnabled = OMX_FALSE;
                    data_sync_barrier();
                }
                if(cmddata == 0x1 || cmddata == -1)
                {
                    ittiam_video_decoder_flush(pVidDec);
                    // Return all output buffers
                    if(1 == pVidDec->shareDispBuf)
                    {
                        FlushOutputBuffers(pVidDec);
                    }
                    else
                    {
                        ListFlushEntries(pVidDec->sOutBufList, pVidDec);
                    }
                    pVidDec->isInFlush = 0;
                    pVidDec->ignoreInitialBPics = 1;
                    pVidDec->prevTimeStamp = -1;
                    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));

                    // Disable port
                    pVidDec->sOutPortDef.bEnabled = OMX_FALSE;
                    data_sync_barrier();
                    if(pVidDec->PortReconfiguration)
                    {
                        ittiam_video_decoder_reset(pVidDec);
                    }
                }
                // Wait for all buffers to be freed
                pthread_mutex_lock(&pVidDec->signal_mutex);
                ret_val = 0;
                // Transition happens only when the ports are unpopulated
                if(pVidDec->PortReconfiguration
                                && (pVidDec->sOutBufList.nAllocSize > 0))
                {

                    //gettimeofday(&abstime,NULL);
                    clock_gettime(CLOCK_REALTIME, &abstime);
                    abstime.tv_sec += OMX_TIMEOUT_SEC;
                    ITTIAM_DEBUG("Waiting for buffers to be freed during port reconfiguration");
                    ret_val = pthread_cond_timedwait(&pVidDec->buffers_signal,
                                                     &pVidDec->signal_mutex,
                                                     &abstime);

                }
                pthread_mutex_unlock(&pVidDec->signal_mutex);

                if(ret_val == 0)
                {
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandPortDisable,
                                                      0x1, NULL);
                }
                else
                {
                    ITTIAM_LOG("Something bad happend ");
                }
            }
            else if(cmd == EnablePort)
            {
                ITTIAM_DEBUG("EnablePort Command");
                // Enable Port(s)
                // cmddata contains the port index to be restarted.
                // It is assumed that 0 is input and 1 is output port for this component.
                // The cmddata value -1 means both input and output ports will be restarted.

                if(cmddata == 0x0 || cmddata == -1)
                    pVidDec->sInPortDef.bEnabled = OMX_TRUE;

                if(cmddata == 0x1 || cmddata == -1)
                    pVidDec->sOutPortDef.bEnabled = OMX_TRUE;

                // Wait for port to be populated
                nTimeout = 0;
                while(1)
                {
                    // Return cmdcomplete event if input port populated
                    if(cmddata == 0x0
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || pVidDec->sInPortDef.bPopulated))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x0, NULL);
                        break;
                    }
                    // Return cmdcomplete event if output port populated
                    if(cmddata == 0x1
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || pVidDec->sOutPortDef.bPopulated))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x1, NULL);
                        break;
                    }
                    // Return cmdcomplete event if input and output ports populated
                    if(cmddata == -1
                                    && (pVidDec->state == OMX_StateLoaded
                                                    || (pVidDec->sInPortDef.bPopulated
                                                                    && pVidDec->sOutPortDef.bPopulated)))
                    {
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x0, NULL);
                        pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                          pVidDec->pAppData,
                                                          OMX_EventCmdComplete,
                                                          OMX_CommandPortEnable,
                                                          0x1, NULL);
                        break;
                    }

                    if(nTimeout++ > OMX_MAX_TIMEOUTS)
                    {
                        pVidDec->pCallbacks->EventHandler(
                                        pVidDec->hSelf,
                                        pVidDec->pAppData,
                                        OMX_EventError,
                                        OMX_ErrorPortUnresponsiveDuringAllocation,
                                        0, NULL);
                        break;
                    }

                    millisleep(OMX_TIMEOUT_MSEC);

                }
            }
            else if(cmd == Flush)
            {
                ITTIAM_DEBUG("Flush Command: Port %d", cmddata);
                pthread_mutex_lock(&pVidDec->signal_mutex);
                pVidDec->cmd_pending = 0;
                pthread_mutex_unlock(&pVidDec->signal_mutex);
                // Flush port(s)
                // cmddata contains the port index to be flushed.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be flushed.
                if(cmddata == 0x0 || cmddata == -1)
                {
                    // Return all input buffers and send cmdcomplete
                    if(pInBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                             pVidDec->pAppData,
                                                             pInBufHdr);
                        pInBufHdr = NULL;
                    }
                    ListFlushEntries(pVidDec->sInBufList, pVidDec)
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandFlush, 0x0,
                                                      NULL);
                }
                if(cmddata == 0x1 || cmddata == -1)
                {
                    // Flush the ittiam decoder
                    ittiam_video_decoder_flush(pVidDec);

                    if(pOutBufHdr)
                    {
                        // Return input buffer to client for refill
                        pVidDec->NumFBD++;
                        //ITTIAM_DEBUG("NumFBD %d LINE %d", (WORD32)pVidDec->NumFBD, __LINE__);
                        pOutBufHdr->nFlags = pOutBufHdr->nFlags & (~CUSTOM_BUFFERFLAG_OWNED);

                        FillBufferDone(pVidDec->hSelf,
                                                            pVidDec->pAppData,
                                                            pOutBufHdr);
                        pOutBufHdr = NULL;
                    }
                    if(1 == pVidDec->shareDispBuf)
                    {
                        // Return all output buffers and send cmdcomplete
                        FlushOutputBuffers(pVidDec);
                    }
                    else
                    {
                        ITTIAM_DEBUG("Calling list flush entries %d ",
                                     __LINE__);
                        ListFlushEntries(pVidDec->sOutBufList, pVidDec);
                    }
                    pVidDec->isInFlush = 0;
                    pVidDec->prevTimeStamp = -1;
                    pVidDec->ignoreInitialBPics = 1;
                    memset(pVidDec->nTimeStamp, 0, sizeof(pVidDec->nTimeStamp));
                    memset(pVidDec->isTimeStampValid, 0, sizeof(pVidDec->isTimeStampValid));

                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventCmdComplete,
                                                      OMX_CommandFlush, 0x1,
                                                      NULL);
                }
            }
            else if(cmd == StopThread)
            {
                ITTIAM_LOG("StopThread Command");
                // Kill thread
                goto EXIT;
            }
            else if(cmd == FillBuf)
            {
                ITTIAM_DEBUG("FillBuf  = %p ",
                             ((OMX_BUFFERHEADERTYPE*)cmddata));
                // Fill buffer
                ListSetEntry(pVidDec->sOutBufList,
                             ((OMX_BUFFERHEADERTYPE*)cmddata))
            }
            else if(cmd == EmptyBuf)
            {

                // Empty buffer
                ListSetEntry(pVidDec->sInBufList,
                             ((OMX_BUFFERHEADERTYPE *)cmddata))
                // Mark current buffer if there is outstanding command
                if(pMarkBuf)
                {
                    ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent =
                                    pMarkBuf->hMarkTargetComponent;
                    ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData =
                                    pMarkBuf->pMarkData;
                    pMarkBuf = NULL;
                }
            }
            else if(cmd == MarkBuf)
            {
                ITTIAM_DEBUG("MarkBuf Command");

                if(!pMarkBuf)
                    pMarkBuf = (OMX_MARKTYPE *)(cmddata);
            }
        }
        // Buffer processing
        // Only happens when the component is in executing state.
        //Even in shared mode, ensure there is atleast one buffer that can be released
        while(pVidDec->state == OMX_StateExecuting
                        && pVidDec->sInPortDef.bEnabled
                        && pVidDec->sOutPortDef.bEnabled
                        && ((pVidDec->sInBufList.nSizeOfList > 0) || pInBufHdr || pVidDec->isInFlush)
                        && ((pVidDec->sOutBufList.nSizeOfList > 0) || pOutBufHdr || (pVidDec->codecBufCnt > pVidDec->sOutPortDef.nBufferCountActual / 2))
                        && (pVidDec->cmd_pending == 0)
                        && (pVidDec->PortReconfiguration == 0))
        {

            if((NULL == pInBufHdr) && (0 == pVidDec->isInFlush))
            {
                ListGetEntry(pVidDec->sInBufList, pInBufHdr)
                pInBufHdr->nOffset = 0;
            }
            // If there is no output buffer, get one from list for non-shared use case
            //In shared mode, release maximum of 1 buffer
            if(1 == pVidDec->shareDispBuf)
            {
                ittiam_video_release_display_frames(pVidDec);
                pOutBufHdr = NULL;
            }
            else if (!pOutBufHdr)
                ListGetEntry(pVidDec->sOutBufList, pOutBufHdr)

            // Check for EOS flag
            if(pInBufHdr)
            {

                // Check for mark buffers
                if(pInBufHdr->pMarkData)
                {
                    // Copy mark to output buffer header
                    if(pOutBufHdr)
                    {
                        pOutBufHdr->pMarkData = pInBufHdr->pMarkData;
                        // Copy handle to output buffer header
                        pOutBufHdr->hMarkTargetComponent =
                                        pInBufHdr->hMarkTargetComponent;
                    }
                }
                // Trigger event handler
                if(pInBufHdr->hMarkTargetComponent == pVidDec->hSelf
                                && pInBufHdr->pMarkData)
                    pVidDec->pCallbacks->EventHandler(pVidDec->hSelf,
                                                      pVidDec->pAppData,
                                                      OMX_EventMark, 0, 0,
                                                      pInBufHdr->pMarkData);

            }

            // Decode frame
            ittiam_video_decoder_process(pVidDec, pInBufHdr, pOutBufHdr);

            /* If more than 4 bytes are remaining in the current bufer,
                then do not call EmptyBufferDone  */
            if(pInBufHdr && ((pInBufHdr->nOffset + 4) >= pInBufHdr->nFilledLen))
            {
                // Return input buffer to client for refill
                pVidDec->NumEBD++;
                pVidDec->pCallbacks->EmptyBufferDone(pVidDec->hSelf,
                                                     pVidDec->pAppData,
                                                     pInBufHdr);
                pInBufHdr = NULL;
            }
            if( pOutBufHdr && ((0 == pVidDec->shareDispBuf) || (pOutBufHdr->nFilledLen != 0)))
            {
                pOutBufHdr = NULL;
            }
        }

    }
    EXIT:
    return (void*)OMX_ErrorNone;
}


#ifdef __cplusplus
//}
#endif /* __cplusplus */
