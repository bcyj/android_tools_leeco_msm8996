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
/*
 *  D E C L A R A T I O N S
 */
/* Define INPUT_DUMP_ENABLE to 1 if input dump is needed */
//#define INPUT_DUMP_ENABLE 1
#if INPUT_DUMP_ENABLE
#define CREATE_INPUT_DUMP(m_filename)                   \
{                                                       \
    FILE *fp = fopen(m_filename, "wb");                 \
    if(fp != NULL)                                      \
        fclose(fp);                                     \
}
#define DUMP_INPUT(m_filename, m_buf, m_size)           \
{                                                       \
    ITTIAM_ERROR("Dumping %d bytes to %s\n", m_size,    \
                 m_filename);                           \
    FILE *fp = fopen(m_filename, "ab");                 \
    if(fp != NULL && m_buf != NULL)                     \
    {                                                   \
        fwrite(m_buf, 1, m_size, fp);                   \
        fclose(fp);                                     \
    }                                                   \
}
#else
#define CREATE_INPUT_DUMP(m_filename)
#define DUMP_INPUT(m_filename, m_buf, m_size)
#endif

#define GETTIME(timer) gettimeofday(timer,NULL);
#define ELAPSEDTIME(start, end, timetaken) \
           timetaken = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);


#define NUM_IN_BUFFERS      4       // Input Buffers
#define NUM_OUT_BUFFERS     33       // Output Buffers
#define MAX_WIDTH           4096
#define MAX_HEIGHT          2160

#define MIN_WIDTH           16
#define MIN_HEIGHT          16


/* Padding values will be used only when buffer sharing is enabled */
#define AVC_PADDING_WIDTH       48
#define AVC_PADDING_HEIGHT      80
#define AVC_MAX_WIDTH           1920
#define AVC_MAX_HEIGHT          1088


#define HEVC_PADDING_WIDTH       160
#define HEVC_PADDING_HEIGHT      160
#define HEVC_MAX_WIDTH           4096
#define HEVC_MAX_HEIGHT          2160

#define VP9_PADDING_WIDTH       160
#define VP9_PADDING_HEIGHT      160
#define VP9_MAX_WIDTH           4096
#define VP9_MAX_HEIGHT          2160

#define MPEG2_MAX_WIDTH          1920
#define MPEG2_MAX_HEIGHT         1088

#define MPEG4_MAX_WIDTH          1920
#define MPEG4_MAX_HEIGHT         1088


#define NUM_OUT_HELD            0
#define MAX_TIMESTAMP_CNT       64
#define CUSTOM_BUFFERFLAG_OWNED 0x00000100

#define MAX_COLOR_FMTS          6

#define IOMX_ThumbnailMode                                      (OMX_IndexVendorStartUnused + 0x103)
#define IOMX_GoogleAndroidIndexGetAndroidNativeBufferUsage      (OMX_IndexVendorStartUnused + 0x104)
#define IOMX_GoogleAndroidIndexUseAndroidNativeBuffer           (OMX_IndexVendorStartUnused + 0x105)
#define IOMX_GoogleAndroidIndexUseAndroidNativeBuffer2          (OMX_IndexVendorStartUnused + 0x107)
#define IOMX_GoogleAndroidIndexEnableAndroidNativeBuffer        (OMX_IndexVendorStartUnused + 0x106)
#define IOMX_GoogleAndroidIndexEnableAdaptivePlayback           (OMX_IndexVendorStartUnused + 0x108)
#define IOMX_GoogleAndroidIndexDescribeColorFormat              (OMX_IndexVendorStartUnused + 0x109)


/*
 *     D E F I N I T I O N S
 */
/*
 * Private data of the component.
 * It includes input and output port related information (Port definition, format),
 *   buffer related information, specifications for the video decoder, the command and data pipes
 *   and the BufferList structure for storing input and output buffers.
 */

/* Processor */
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
#define VP9_MAX_NUM_CORES   4

/* Type of output buffer - Local for s/w rendering or native for h/w rendering */
typedef enum
{
    BUFFER_TYPE_LOCAL       = 0x0,
    BUFFER_TYPE_NATIVEBUF2  = 0x10,
    BUFFER_TYPE_NATIVEBUF   = 0x20,
}bufferTypeT;

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
    pthread_mutex_t signal_flush_mutex;
    pthread_cond_t buffers_flush_signal;
    OMX_PARAM_COMPONENTROLETYPE componentRole;
    OMX_U32 NumETB;
    OMX_U32 NumEBD;
    OMX_U32 NumFTB;
    OMX_U32 NumFBD;

    OMX_BUFFERHEADERTYPE *pBufHeld[NUM_OUT_HELD];
    UWORD32 padWidth;
    UWORD32 padHeight;

    UWORD32 initDone;
    UWORD32 bufferAllocationPending;
    UWORD32 flushInProgress;
    UWORD32 ThumbnailMode;
    UWORD32 PortReconfiguration;
    UWORD32 reInitPending;
    UWORD32 reInitWidth;
    UWORD32 reInitHeight;
    UWORD32 nBufferCountActual;
    OMX_S64 nTimeStamp[MAX_TIMESTAMP_CNT];
    UWORD32 isTimeStampValid[MAX_TIMESTAMP_CNT];
    PROCESSORTYPE mProcessorType;

    /* Ittiam decoder specific data */
    // Main API function pointer
    IV_API_CALL_STATUS_T (*iVdec_cxa8_api_function)(iv_obj_t *ps_handle, void *pv_api_ip,void *pv_api_op);
    iv_mem_rec_t        *pMemRecs;
    iv_obj_t *ps_codec;
    UWORD32 numMemRecs;

    ivd_out_bufdesc_t s_disp_buffers[NUM_OUT_BUFFERS];
    UWORD32 hdrDecodeDone;
    UWORD32 cmd_pending;
    void    *disp_buf_id_mapping[NUM_OUT_BUFFERS];

    UWORD32 shareDispBuf;
    UWORD32 numCores;
    UWORD32 maxWidth;
    UWORD32 maxHeight;
    UWORD32 minUndequeuedBufs;
    UWORD32 stride;
    UWORD32 disableInterlaced;
    struct timeval prevStopTime;
    WORD32 nProfile;
    WORD32 nLevel;
    IV_COLOR_FORMAT_T e_output_format;
    WORD32 swrender;
    OMX_U32 nDispWidth;
    OMX_U32 nDispHeight;
    OMX_U32 mDRCError;
    OMX_U32 mUnsupportedReslnError;
    bufferTypeT bufferType;
    OMX_VIDEO_CODINGTYPE VidFormat;
    UWORD8 *pFlushOutBuf;
    UWORD32 codecBufCnt;
    UWORD32 initHeight;
    UWORD32 initWidth;
    UWORD32 isInFlush;
    UWORD32 receivedEOS;
    UWORD32 numProfiles;
    UWORD32 numLevels;
    const UWORD32 *pProfiles;
    const UWORD32 *pLevels;
    OMX_S64 prevTimeStamp;
    UWORD32 ignoreInitialBPics;
} VIDDECTYPE;

#define MIN(a, b)       ((a) < (b)) ? (a) : (b)
#define MAX(a, b)       ((a) > (b)) ? (a) : (b)
#define ALIGN4096(x)    ((((x) + 4095) >> 12) << 12)
#define ALIGN128(x)     ((((x) + 127) >> 7) << 7)
#define ALIGN64(x)      ((((x) + 63) >> 6) << 6)
#define ALIGN32(x)      ((((x) + 31) >> 5) << 5)
#define ALIGN16(x)      ((((x) + 15) >> 4) << 4)
#define ALIGN8(x)       ((((x) + 7) >> 3) << 3)

#define IV_ISFATALERROR(x)         (((x) >> IVD_FATALERROR) & 0x1)



static __inline void data_sync_barrier(void)
{
    __asm__(" dsb \n\t");
    return;
}
void ComponentReset(VIDDECTYPE *pVidDec);
OMX_API OMX_ERRORTYPE ComponentInit(OMX_IN  OMX_HANDLETYPE hComponent,
                                    OMX_IN  OMX_VIDEO_CODINGTYPE VidFormat,
                                    OMX_IN  OMX_COLOR_FORMATTYPE ColorFormat,
                                    OMX_IN  OMX_U32 maxWidth,
                                    OMX_IN  OMX_U32 maxHeight,
                                    OMX_IN  OMX_U32 shareDispBuf,
                                    OMX_IN  OMX_U32 numCores,
                                    OMX_IN  OMX_U32 disableInterlaced,
                                    OMX_IN  OMX_U32 minUndequeuedBufs,
                                    OMX_IN  PROCESSORTYPE processorType,
                                    OMX_IN  OMX_U32 swrender);
OMX_API OMX_ERRORTYPE GetParameter(OMX_IN  OMX_HANDLETYPE hComponent,
                 OMX_IN  OMX_INDEXTYPE nParamIndex,
                 OMX_INOUT OMX_PTR ComponentParameterStructure);
OMX_API OMX_ERRORTYPE SetParameter(OMX_IN  OMX_HANDLETYPE hComponent, OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR ComponentParameterStructure);
OMX_API OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr);
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

