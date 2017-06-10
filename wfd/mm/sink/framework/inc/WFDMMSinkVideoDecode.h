#ifndef __WFD_MM_SINK_VIDEO_DECODE_H__
#define __WFD_MM_SINK_VIDEO_DECODE_H__

/*==============================================================================
*       WFDMMSinkVideoDecode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSinkVideoDecode.
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/28/2013         SK            InitialDraft
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
//#include "AEEStd.h"
#include "OMX_Core.h"
#include "OMX_IVCommon.h"
#include "MMCriticalSection.h"
#include "WFDMMSinkCommon.h"
#include "filesource.h"
#include "GraphicBuffer.h"
#include "HardwareAPI.h"
#include "qmmList.h"
#include "wdsm_mm_interface.h"
#include "WFDMMSinkStatistics.h"
typedef struct videoConfig
{
    WFD_video_type             eVideoType;
    uint32                     nFrameWidth;
    uint32                     nFrameHeight;
    uint32                     nFps;
    bool                       bSecure;
    ANativeWindow             *pWindow;

}videoConfigType;

class  SignalQueue;

class WFDMMSinkVideoDecode
{
public:

    uint64 getCurrentTimeUs();
    WFDMMSinkVideoDecode(int moduleId,
                         WFDMMSinkEBDType       pFnEBD,
                         WFDMMSinkFBDType       pFnFBD,
                         WFDMMSinkHandlerFnType pFnHandler,
                         void* clientData);
    ~WFDMMSinkVideoDecode();

    OMX_ERRORTYPE  Configure(videoConfigType *pCfg);

    OMX_ERRORTYPE  DeliverInput
    (
        OMX_BUFFERHEADERTYPE *pBuffer
    );

    OMX_ERRORTYPE  SetFreeBuffer
    (
        OMX_BUFFERHEADERTYPE *pBuffer
    );

    OMX_ERRORTYPE  Stop();

    OMX_ERRORTYPE  Start();

private:
    OMX_ERRORTYPE  AllocateVideoBuffers();
    OMX_ERRORTYPE  allocateVideoInputBuffers();

    OMX_ERRORTYPE  allocateVideoOutputBuffers();
    OMX_ERRORTYPE  createResources();
    OMX_ERRORTYPE  destroyResources();

    bool  destroyVideoResources();

    bool  freeVideoBuffers();

    OMX_ERRORTYPE  freeVideoInputBuffers();

    OMX_ERRORTYPE  freeVideoOutputBuffers();

    bool  destroyThreadsAndQueues();

    bool  createThreadsAndQueues();

    OMX_ERRORTYPE  deinitialize();

    void  PrerequisitesforStop();

    void  InitData();

    int  state
    (
       int state,
       bool get_true_set_false
    );

    static OMX_ERRORTYPE  EventCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);


    static OMX_ERRORTYPE  EmptyDoneCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE  FillDoneCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE  SetState
    (
        OMX_STATETYPE eState,
        OMX_BOOL bSynchronous
    );

    OMX_ERRORTYPE  WaitState
    (
        OMX_STATETYPE eState
    );

    bool createVideoResources();
    bool setupPictureInfo(OMX_BUFFERHEADERTYPE *pBuffer);
    bool HandlePortReconfig();

    videoConfigType        mCfg;
    WFDMMSinkHandlerFnType mpFnHandler;
    WFDMMSinkFBDType       mpFnFBD;
    WFDMMSinkEBDType       mpFnEBD;
    SignalQueue *mOutputBufCollectorQ;
    SignalQueue *mVideoOutputQ;
    SignalQueue *mVideoInputQ;
    SignalQueue *mGraphicBufQ;
    SignalQueue *mSignalQ;
    android::sp<ANativeWindow>  mpWindow;
    uint32 mnOutputPortIndex;
    uint32 mnInputPortIndex;
    uint32 mnNumInputBuffers;
    uint32 mnNumOutputBuffers;
    uint32 mnNumDequeuedBufs;
    OMX_COLOR_FORMATTYPE meColorFmt;
    OMX_HANDLETYPE mhVdec;
    OMX_VIDEO_CODINGTYPE meCompressionFmt;
    uint32 mnModuleId;
    void* mClientData;
    uint32 mUsage;
    MM_HANDLE mhCritSect;
    int32  meState;
    bool   mbInputPortFound;
    bool   mbOutputPortFound;
    WFDMMSinkStatistics *m_pStatInst;
};
#endif /*__WFD_MM_SINK_VIDEO_DECODE_H__*/

