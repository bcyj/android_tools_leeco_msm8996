#ifndef _WFD_MM_VIDEO_ENCODE_H_
#define _WFD_MM_VIDEO_ENCODE_H_

/*==============================================================================
*       WFDMMSourceVideoEncode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSourceVideoEncode
*
*
*  Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
12/27/2013                    InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/

#include "OMX_Core.h"
#include "WFDMMSource.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMCriticalSection.h"
#include "WFDMMSourceVideoCapture.h"
#include <gui/IGraphicBufferProducer.h>
#include "WFDMMSourceStatistics.h"

class WFDMMSourceVideoEncode{

public:
    typedef void (*FrameDeliveryFnType) (OMX_BUFFERHEADERTYPE* pBufferHdr, void *);

    WFDMMSourceVideoEncode();

    ~WFDMMSourceVideoEncode();

   enum CodecProfileType {
        MPEG4ProfileSimple,
        MPEG4ProfileAdvancedSimple,
        H263ProfileBaseline,
        AVCProfileBaseline,
        AVCProfileHigh,
        AVCProfileMain,
        VP8ProfileMain,
   };

    struct CmdType
    {
        OMX_EVENTTYPE    eEvent;
        OMX_COMMANDTYPE eCmd;
        OMX_U32         sCmdData;
        OMX_ERRORTYPE    eResult;
    };

    OMX_ERRORTYPE configure(
         VideoEncStaticConfigType* pConfig,
         eventHandlerType pFnCallback,
         FrameDeliveryFnType pFrameDelivery,
         OMX_BOOL bHDCPEnabled,
         OMX_U32 nModuleId,
         void* pAppData);

    OMX_ERRORTYPE Pause();

    OMX_ERRORTYPE Resume();

    OMX_S32 GetNumInpBuf() const {return m_nNumInputBuffers;}

    OMX_S32 GetNumOutBuf() const {return m_nNumOutputBuffers;}

    OMX_S32 GetInpBufSize() const {return m_nInputBufferSize;}

    OMX_S32 GetOutBufSize() const {return m_nOutputBufferSize;}

    static OMX_ERRORTYPE EventCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);

    OMX_ERRORTYPE DeliverInput(
          OMX_BUFFERHEADERTYPE* pBufferHdr);

    static OMX_ERRORTYPE EmptyDoneCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr);

    static OMX_ERRORTYPE FillDoneCallback(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr);

    OMX_ERRORTYPE SetFreeBuffer(
        OMX_BUFFERHEADERTYPE* pBufferHdr);

    OMX_ERRORTYPE QInputBuffer(
        OMX_BUFFERHEADERTYPE** ppBufferHdr);

    OMX_ERRORTYPE RequestIntraVOP();

    OMX_ERRORTYPE ChangeBitrate(OMX_S32 nBitrate);

    OMX_BUFFERHEADERTYPE** GetInputBuffHdrs()const;

    OMX_BUFFERHEADERTYPE** GetOutputBuffHdrs()const;

    android::IGraphicBufferProducer* getSurface() const
    {
        return m_hVideoCapture ? m_hVideoCapture->getSurface():NULL;
    }

    OMX_ERRORTYPE Start();

    OMX_ERRORTYPE Stop();

private:
    OMX_ERRORTYPE createResources();

    OMX_ERRORTYPE initData();

    OMX_ERRORTYPE GoToExecutingState();

    OMX_ERRORTYPE GoToIdleState();

    OMX_ERRORTYPE GoToLoadedState();

    OMX_ERRORTYPE AllocateBuffers();

    OMX_ERRORTYPE DeAllocateBuffers();

    OMX_ERRORTYPE createCaptureSource();

    OMX_ERRORTYPE ReleaseResources();

    OMX_ERRORTYPE SetState
    (
        OMX_STATETYPE eState,
        OMX_BOOL bSynchronous
    );

    OMX_ERRORTYPE WaitState
    (
        OMX_STATETYPE eState
    );

    OMX_ERRORTYPE finish();

    OMX_ERRORTYPE destroy();

    void dumpVencStats() const;

    static void captureDataCb(OMX_PTR pClientData,
                              OMX_BUFFERHEADERTYPE *pBuffer);

    static void captureEventsCb(OMX_PTR pClientData,
                                OMX_U32 nEvent,
                                OMX_U32 nEvtData);
    int state(int state, bool get_true_set_false);

    OMX_HANDLETYPE              m_hEncoder;
    OMX_STATETYPE               m_eState;
    OMX_U32                     m_nModuleId;
    OMX_PTR                     m_pAppData;
    OMX_VIDEO_CODINGTYPE        m_eCodec;
    CodecProfileType            m_eProfType;
    OMX_BOOL                    m_bCABAC;
    OMX_BUFFERHEADERTYPE**      m_pInputBuffers;
    OMX_S32                     m_nNumInputBuffers;
    OMX_BUFFERHEADERTYPE**      m_pOutputBuffers;
    OMX_S32                     m_nNumOutputBuffers;
    OMX_S32                     m_nInputBufferSize;
    OMX_S32                     m_nOutputBufferSize;
    OMX_S32                     m_nBitrateFactor;
    OMX_U32                     m_nInputPortIndex;
    OMX_U32                     m_nOutputPortIndex;
    OMX_BOOL                    m_bInputPortFound;
    OMX_BOOL                    m_bOutputPortFound;
    SignalQueue*                m_pCommandQ;
    SignalQueue*                m_pInpBufQ;
    SignalQueue*                m_pOutBufQ;
    eventHandlerType            m_pFnEventCallback;
    FrameDeliveryFnType         m_pFnFrameDeliver;
    MM_HANDLE                   m_hCritSect;
    MM_HANDLE                   m_hInputQLock;
    MM_HANDLE                   m_hOutputQLock;
    VideoEncStaticConfigType    m_sConfig;
    WFDMMSourceVideoCapture    *m_hVideoCapture;
    int                         m_nState;
    OMX_BOOL                    m_bHDCPEnabled;
    OMX_BOOL                    m_bEnableProfiling;

    struct wfd_venc_stats
    {
        OMX_U64     nETB;
        OMX_U64     nEBD;
        OMX_U64     nFTB;
        OMX_U64     nFBD;
        OMX_TICKS   nETBDiff;
        OMX_TICKS   nMaxETBTime;
        OMX_TICKS   nEBDDiff;
        OMX_TICKS   nMaxEBDTime;
        OMX_TICKS   nFTBDiff;
        OMX_TICKS   nMaxFTBTime;
        OMX_TICKS   nFBDDiff;
        OMX_TICKS   nMaxFBDTime;
    };

    wfd_venc_stats              mWfdVencStats;
    WFDMMSourceStatistics*      mWFDMMSrcStats;
    #ifdef ENABLE_H264_DUMP
        FILE*    H264_dump_file;
        OMX_BOOL m_bEnableH264Dump;
    #endif /* ENABLE_H264_DUMP */
};
#endif //_WFD_MM_VIDEO_ENCODE_H_
