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

#ifndef OMX_ITTIAM_VENC_H
#define OMX_ITTIAM_VENC_H

#include "ih264_cxa8.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *  D E C L A R A T I O N S
 */

#define NUM_IN_BUFFERS      32      // Input Buffers
#define NUM_OUT_BUFFERS     6       // Output Buffers
#define MAX_WIDTH           1280
#define MAX_HEIGHT          720
#define MAX_MEM_REC         300

#define MIN_WIDTH           48
#define MIN_HEIGHT          32
#define MAX_NUM_IO_BUFS     3

#define NUM_OUT_HELD    0

#define CUSTOM_OMX_IndexConfigLCLevel   (OMX_IndexVendorStartUnused + 0x1)

/*
 *     D E F I N I T I O N S
 */
/*
 * Private data of the component.
 * It includes input and output port related information (Port definition, format),
 *   buffer related information, specifications for the video decoder, the command and data pipes
 *   and the BufferList structure for storing input and output buffers.
 */
typedef struct
{
    UWORD8   *u1_mem_ptr;
    UWORD8   *u1_align_mem_ptr;
    UWORD32  u4_memsize;
    UWORD32  u4_alignment;

} mem_record;
 typedef struct
{
    UWORD32     u4_mem_cnt;
    mem_record  memRec[MAX_MEM_REC];
} mem_mgr;

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

typedef struct VIDENCTYPE {
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
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   sInProfile;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   sOutProfile;
    OMX_VIDEO_PARAM_BITRATETYPE   sBitRateType;
    OMX_COLOR_FORMATTYPE sInColorFormat;
    pthread_t thread_id;
    OMX_U32 datapipe[2];
    WORD32 cmdpipe[2];
    WORD32 cmddatapipe[2];
    ThrCmdType eTCmd;
    BufferList sInBufList;
    BufferList sOutBufList;
    pthread_mutex_t pipes_mutex;
    pthread_mutex_t signal_mutex;
    pthread_cond_t buffers_signal;
    OMX_PARAM_COMPONENTROLETYPE componentRole;
    OMX_U32 FramesEncoded;
    OMX_U32 NumETB;
    OMX_U32 NumEBD;
    OMX_U32 NumFTB;
    OMX_U32 NumFBD;
    OMX_U32 num_op_buffers_held;
    OMX_BUFFERHEADERTYPE *pBufHeld[NUM_OUT_HELD];
    UWORD32 u4_min_num_out_bufs;
    UWORD32 u4_min_out_buf_size;
    UWORD32 height_padding;
    UWORD32 y_height_padding;
    UWORD32 uv_height_padding;
    UWORD32 offset;
    UWORD32 initdone;
    UWORD32 highqualityencode;
    UWORD32 quadcore;
    UWORD32 meta_mode_enable;
    UWORD32 mUseLifeEffects;
    UWORD32 buffersize;
    UWORD32 BufferUnmaprequired;
    OMX_U8* input_buffer;
    OMX_U8* temp_buffer;
    UWORD32 u4_alt_ref_frame;
    void *mem;
    UWORD32 input_color_format;
    /* Ittiam decoder specific data */

    // Main API function pointer
    IV_API_CALL_STATUS_T (*iVenc_cxa8_api_function)(iv_obj_t *ps_handle, void *pv_api_ip,void *pv_api_op);
    iv_mem_rec_t        *pv_mem_rec_location;
    iv_obj_t *g_ENCHDL;
    UWORD32 u4_num_mem_recs;
    UWORD32 u4_ip_frm_ts;
    UWORD32 hdr_encode_done;
    UWORD32 data_generation_started;
    pthread_mutex_t codec_mutex;
    pthread_cond_t codec_signal;
    UWORD32 cmd_pending;
    UWORD32 wait_for_op_buffers;
    UWORD32 LowComplexity;
    void    *disp_buf_id_mapping[NUM_OUT_BUFFERS];
    void    *inp_buf_id_mapping[NUM_IN_BUFFERS];
    void    *inp_y_id_mapping[NUM_IN_BUFFERS];
    UWORD32 seek_to_I_frame;

} VIDENCTYPE;



iv_obj_t  *enc_handle;




OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN  OMX_HANDLETYPE hComponent, OMX_IN OMX_VIDEO_CODINGTYPE VidFormat,
                                    OMX_IN  OMX_COLOR_FORMATTYPE ColorFormat,OMX_IN OMX_U32 high_quality_encoder,OMX_IN OMX_U32 quadcore,  OMX_IN OMX_U32 alt_ref_flag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_ITTIAM_VENC_H */

