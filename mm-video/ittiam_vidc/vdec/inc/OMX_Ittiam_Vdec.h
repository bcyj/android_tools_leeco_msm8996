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

#ifndef OMX_ITTIAM_VDEC_H
#define OMX_ITTIAM_VDEC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifndef ISExynos
#include <OMX_QCOMExtns.h>
#endif
/*
 *  D E C L A R A T I O N S
 */

#define NUM_IN_BUFFERS      4       // Input Buffers
#define NUM_OUT_BUFFERS     33       // Output Buffers
#define MAX_WIDTH           1920
#define MAX_HEIGHT          1088

#define MIN_WIDTH           48
#define MIN_HEIGHT          32

/* Setting SHARE_DISP_BUF to zero completely disabled shared display mode */
/* Setting it to 1, enables sharing only for 7x27a and that too
only for resolutions above 848x480 */
/* Currently resolution dependent checks and platform dependent checks are not present */
#define SHARE_DISP_BUF  1

/* Padding values will be used only when buffer sharing is enabled */
#define AVC_WIDTH_PADDING       48
#define AVC_Y_HEIGHT_PADDING    80
#define AVC_UV_HEIGHT_PADDING   40
#define AVC_HEIGHT_PADDING      (AVC_Y_HEIGHT_PADDING + AVC_UV_HEIGHT_PADDING)

#define OMX_VIDEO_CodingHEVC 8//test
#define HEVC_WIDTH_PADDING       160
#define HEVC_Y_HEIGHT_PADDING    160
#define HEVC_UV_HEIGHT_PADDING   80
#define HEVC_HEIGHT_PADDING      (HEVC_Y_HEIGHT_PADDING + HEVC_UV_HEIGHT_PADDING)



#define NUM_OUT_HELD    0
#define STAMP_IDX 32
#define CUSTOM_BUFFERFLAG_OWNED 0x00000100


#define CUSTOM_COLOR_FormatYVU420SemiPlanar 0x7FA30C00
#define CUSTOM_COLOR_FormatYUV420SemiPlanar 0x109

#define CUSTOM_OMX_IndexConfigLCLevel   (OMX_IndexVendorStartUnused + 0x1)
#define CUSTOM_OMX_ThumbnailMode    (OMX_IndexVendorStartUnused + 0x3)
#ifdef ISExynos
#define OMX_VIDEO_CodingHEVC    (OMX_VIDEO_CodingVendorStartUnused + 0x1)
#define OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage   (OMX_IndexVendorStartUnused + 0x4)
#define OMX_GoogleAndroidIndexUseAndroidNativeBuffer    (OMX_IndexVendorStartUnused + 0x5)
#endif
#define OMX_GoogleAndroidIndexEnableAndroidNativeBuffer (OMX_IndexVendorStartUnused + 0x6)


/*
 *     D E F I N I T I O N S
 */
/*
 * Private data of the component.
 * It includes input and output port related information (Port definition, format),
 *   buffer related information, specifications for the video decoder, the command and data pipes
 *   and the BufferList structure for storing input and output buffers.
 */

typedef enum PROCESSORTYPE{
    GENERIC = 0x0,
    QCOM_GENERIC = 0x100,
    QCOM_7X27A,
    QCOM_8X25,
    QCOM_8X25Q,
    QCOM_8X10,
    QCOM_8X12,
    QCOM_8960,
    QCOM_APQ8064,
    QCOM_MPQ8064,
    QCOM_8974,
    SAMSUNG_GENERIC = 0x200,
    TI_GENERIC = 0x300,

}PROCESSORTYPE;

/* Maximum number of cores supported by each codec */
#define H264_MAX_NUM_CORES  3
#define HEVC_MAX_NUM_CORES  4
#define MPEG4_MAX_NUM_CORES 3
#define MPEG2_MAX_NUM_CORES 2

typedef struct QOMX_CROP_RECT{
        OMX_U32 nWidth;
        OMX_U32 nHeight;
}QOMX_CROP_RECT;

typedef struct VIDDECTYPE {
    OMX_STATETYPE state;
    OMX_CALLBACKTYPE *pCallbacks;
    OMX_PTR pAppData;
    OMX_HANDLETYPE hSelf;
    OMX_PORT_PARAM_TYPE sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE sInPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE sOutPortDef;
    OMX_VIDEO_PARAM_PORTFORMATTYPE sInPortFormat;
    OMX_VIDEO_PARAM_PORTFORMATTYPE sOutPortFormat;
    OMX_PRIORITYMGMTTYPE sPriorityMgmt;
    OMX_PARAM_BUFFERSUPPLIERTYPE sInBufSupplier;
    OMX_PARAM_BUFFERSUPPLIERTYPE sOutBufSupplier;
    OMX_VIDEO_PARAM_MPEG4TYPE sMpeg4;
    OMX_VIDEO_PARAM_AVCTYPE   sH264;
    pthread_t thread_id;
    WORD32 datapipe[2];
    WORD32 cmdpipe[2];
    WORD32 cmddatapipe[2];
    ThrCmdType eTCmd;
    BufferList sInBufList;
    BufferList sOutBufList;
    pthread_mutex_t pipes_mutex;
    pthread_mutex_t signal_mutex;
    pthread_cond_t buffers_signal;
    OMX_PARAM_COMPONENTROLETYPE componentRole;
    OMX_U32 NumETB;
    OMX_U32 NumEBD;
    OMX_U32 NumFTB;
    OMX_U32 NumFBD;
    OMX_U32 num_op_buffers_held;
    OMX_BUFFERHEADERTYPE *pBufHeld[NUM_OUT_HELD];
    UWORD32 width_padding;
    UWORD32 height_padding;
    UWORD32 y_height_padding;
    UWORD32 uv_height_padding;
    UWORD32 offset;
    UWORD32 initdone;
    UWORD32 bufferallocationpending;
    UWORD32 exitPending;
    UWORD32 ThumbnailMode;
    UWORD32 IsExynos;
    UWORD32 IsQcomCore;
    UWORD32 PortReconfiguration;
    OMX_S64 nTimeStamp[STAMP_IDX];
    UWORD32 CheckForValidity[STAMP_IDX];
    PROCESSORTYPE mProcessorType;

    /* Ittiam decoder specific data */

    // Main API function pointer
    IV_API_CALL_STATUS_T (*iVdec_cxa8_api_function)(iv_obj_t *ps_handle, void *pv_api_ip,void *pv_api_op);
    iv_mem_rec_t        *pv_mem_rec_location;
    iv_obj_t *g_DECHDL;
    UWORD32 u4_num_mem_recs;
    ivd_out_bufdesc_t *s_out_buffer;
    UWORD32 u4_ip_frm_ts;
    ivd_out_bufdesc_t s_disp_buffers[NUM_OUT_BUFFERS];
    UWORD32 hdr_decode_done;
    pthread_mutex_t codec_mutex;
    pthread_cond_t codec_signal;
    UWORD32 cmd_pending;
    UWORD32 wait_for_op_buffers;
    UWORD32 LowComplexity;
    void    *disp_buf_id_mapping[NUM_OUT_BUFFERS];
    void    *inp_y_id_mapping[NUM_OUT_BUFFERS];
    void    *inp_uv_id_mapping[NUM_OUT_BUFFERS];
    UWORD32 seek_to_I_frame;
    UWORD32 share_disp_buf;
    UWORD32 num_cores;
    UWORD32 low_complexity_level_max;
    UWORD32 max_width;
    UWORD32 max_height;
    UWORD32 minUndequeuedBufs;
    UWORD32 stride;
    UWORD32 disable_interlaced;
    long long prev_stop_ts;
    WORD32 nProfile;
    WORD32 nLevel;
    IV_COLOR_FORMAT_T e_output_format;
    QOMX_CROP_RECT sCropRect;
} VIDDECTYPE;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

static __inline void data_sync_barrier(void)
{

__asm__(" dsb \n\t");

    return;
}

OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN  OMX_HANDLETYPE hComponent,
                                    OMX_IN  OMX_VIDEO_CODINGTYPE VidFormat,
                                    OMX_IN  OMX_COLOR_FORMATTYPE ColorFormat,
                                    OMX_IN  OMX_U32 max_width,
                                    OMX_IN  OMX_U32 max_height,
                                    OMX_IN  OMX_U32 share_disp_buf,
                                    OMX_IN  OMX_U32 num_cores,
                                    OMX_IN  OMX_U32 low_complexity_level_max,
                                    OMX_IN  OMX_U32 disable_interlaced,
                                    OMX_IN  OMX_U32 minUndequeuedBufs,
                                    OMX_IN  OMX_U32 IsQcomCore,
                                    OMX_IN  PROCESSORTYPE processor_type);
OMX_API OMX_ERRORTYPE GetParameter(OMX_IN  OMX_HANDLETYPE hComponent,
                 OMX_IN  OMX_INDEXTYPE nParamIndex,
                 OMX_INOUT OMX_PTR ComponentParameterStructure);
OMX_API OMX_ERRORTYPE SetParameter(OMX_IN  OMX_HANDLETYPE hComponent, OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR ComponentParameterStructure);
OMX_API OMX_ERRORTYPE UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
              OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
              OMX_IN OMX_U32 nPortIndex,
              OMX_IN OMX_PTR pAppPrivate,
              OMX_IN OMX_U32 nSizeBytes,
              OMX_IN OMX_U8* pBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_ITTIAM_VDEC_H */

