#ifndef _WFD_MM_VIDEO_CAPTURE_H_
#define _WFD_MM_VIDEO_CAPTURE_H_

/*==============================================================================
*       WFDMMSourceVideoCapture.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSourceVideoCapture
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
#include "MMSignal.h"
#include "WFDMMSource.h"
#include "MMCriticalSection.h"
#include <gui/IGraphicBufferProducer.h>
#include <media/stagefright/SurfaceMediaSource.h>
#include <media/stagefright/MediaBuffer.h>


//#define WFD_DBG_ENABLE_YUV_READ

#define VIDEO_CAPTURE_NUM_BUFF     2

typedef struct captureFrameInfo
{
    uint8  *pBuffer;
    bool    isRead;
    bool    isValid;
}captureFrameInfoType;

typedef struct videoCaptureConfig
{
    OMX_U32 nFrameRate;
}videoCaptureConfigType;

typedef void (*VidCaptureDataCallbackFnType)
                    (OMX_PTR pClientData, OMX_BUFFERHEADERTYPE *pBufferHdr);

typedef void (*VidCaptureEventCallbackFnType)
                    (OMX_PTR pClientData, OMX_U32 nEvent, OMX_U32 nEvtData);


class WFDMMThreads;

class WFDMMSourceVideoCapture{

public:

    WFDMMSourceVideoCapture();

    ~WFDMMSourceVideoCapture();

    OMX_ERRORTYPE configure
    (
        VideoEncStaticConfigType*        pConfig,
        VidCaptureEventCallbackFnType    pFnEvt,
        VidCaptureDataCallbackFnType     pFnData,
        OMX_U32                          nModuleId,
        OMX_PTR                          pAppData
    );

    OMX_ERRORTYPE startCapture();

    OMX_ERRORTYPE stopCapture();

    OMX_ERRORTYPE pauseCapture();

    OMX_ERRORTYPE resumeCapture();

    android::IGraphicBufferProducer *getSurface()
     const { return m_pBuffProducer.get();}

    OMX_ERRORTYPE SetFreeBuffer
    (
        OMX_BUFFERHEADERTYPE *pBuffer,
        OMX_BOOL              bForce = OMX_FALSE
    );


private:

    MM_HANDLE                         m_hVideoCaptureTimer;
    WFDMMThreads                     *m_hVideoCaptureThread;
    captureFrameInfoType              m_sLatestFrameQ[VIDEO_CAPTURE_NUM_BUFF];
    OMX_U64                           m_nReadIdx;
    OMX_U64                           m_nWriteIdx;
    OMX_TICKS                         m_nPrevBufferTS;
    MM_HANDLE                         m_hQCritSect;
    MM_HANDLE                         m_hStateCritSect;
    VideoEncStaticConfigType          m_sConfig;
    OMX_U32                           m_nModuleId;
    OMX_PTR                           m_pAppData;
    android::sp<android::SurfaceMediaSource>   m_pSurfaceMediaSrc;
    android::sp<android::IGraphicBufferProducer>          m_pBuffProducer;
    OMX_U32                           m_nTimerCtr;
    OMX_BOOL                          m_bFirstFrameFetched;
    OMX_BOOL                          m_bMissedFrame;
    OMX_BOOL                          m_bPaused;
    OMX_U32                           m_nMinRetransmitCnt;
    android::MediaBuffer             *m_pLastXmitBuffer;
    SignalQueue                      *m_pVideoInputQ;
    VidCaptureDataCallbackFnType      m_pFnData;
    VidCaptureEventCallbackFnType     m_pFnEvent;
    OMX_S32                           m_nState;
    OMX_BOOL                          m_bHDCPEnabled;

    struct wfd_vcap_stats
    {
        int nAcquiredBuff;
        int nRelasedBuff;
        OMX_TICKS prevKickTime;
        OMX_TICKS maxReadTime;
        int64_t   frameNo;
    };

    wfd_vcap_stats                    mWfdVcapStats;

    OMX_ERRORTYPE initData();

    OMX_ERRORTYPE createResources();

    void          releaseResources();

    void          releaseMediaBuffers();

    OMX_ERRORTYPE createSurface(int width, int height);

    static void   captureThreadEntry(void* pThreadData, unsigned int signal);

    OMX_ERRORTYPE captureThread(int nSignal);

    static void   inputDeliveryTimerCb(void *pPtr);
    void          inputDeliveryHandler();

    int AccessLatestBufQ
    (
        bool bRead,
        bool bForce,
        captureFrameInfoType* pBufferInfo
    );
    int state(int state, bool get_true_set_false);
};
#endif //_WFD_MM_VIDEO_CAPTURE_H_
