#ifndef __WFD_MM_SINK_RENDERER_H__
#define __WFD_MM_SINK_RENDERER_H__
/*==============================================================================
*       WFDMMSinkRenderer.h
*
*  DESCRIPTION:
*       Class declaration for WFDMM Sink MediaSource. Connects RTP decoder and
*       parser and provides Audio and Video samples to
*       the media framework.
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
#ifdef WFD_ICS
#include "surfaceflinger/Surface.h"
#else
#include <Surface.h>
#include <android/native_window.h>
#endif
//#include "media/mediaplayer.h"
//#include "AEEStd.h"
#include "WFDMMSinkCommon.h"
#include "OMX_Core.h"
#include "MMCriticalSection.h"
#include "binder/ProcessState.h"
#include "wdsm_mm_interface.h"

#include "qmmList.h"

//using namespace android;
#define RENDERERMAXQSIZES 200
#define AUDIO_AV_SYNC_DROP_WINDOW -200000
#define VIDEO_AV_SYNC_DROP_WINDOW -200000

typedef enum action
{
    pushrear,
    popfront,
    poprear,
    pushfront,
    peekfront,
    peekrear,
    size,
    search
}actionType;

typedef struct rendererConfig
{
    bool   bHasVideo;
    bool   bHasAudio;
    bool   bAVSyncMode;
    bool   bAudioTrackLatencyMode;

    struct audioInfo
    {
        WFD_audio_type eAudioFmt;
        uint32 nSampleRate;
        uint32 nChannels;
        uint32 nFrameLength;
    }sAudioInfo;

    struct videoInfo
    {
        uint32 nFrameWidth;
        uint32 nFrameHeight;
        uint32 nFrameRate;
        int32  nFrameDropMode;
    }sVideoInfo;
    uint64          nDecoderLatency;
    int32 naudioAVSyncDropWindow;
    int32 nvideoAVSyncDropWindow;
    ANativeWindow  *pWindow;
}rendererConfigType;

class  WFDMMThreads;
class  SignalQueue;
namespace android {
    class  AudioTrack;
}
#ifndef USE_OMX_AAC_CODEC
class WFDMMSinkAudioDecode;
#endif

typedef struct frameInfo
{
    QMM_ListLinkType      pLink;
    OMX_BUFFERHEADERTYPE *pBuffer;  //Pointer of the buffer
    uint64                nTSSched;  //Time at which the frame to be scheduled
    uint64                nTSArrival; // Arrival time of the buffer
}frameInfoType;

class WFDMMSinkRenderer
{
public:

    /*==========================================================================

             FUNCTION:         WFDMMSinkRenderer

             DESCRIPTION:
    *//**       @brief         WFDMMSinkRenderer contructor
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
     WFDMMSinkRenderer(int moduleId,
                       WFDMMSinkEBDType       pFnEBD,
                       WFDMMSinkHandlerFnType pFnHandler,
                       void* clientData);


    /*==========================================================================

             FUNCTION:         WFDMMSinkRenderer

             DESCRIPTION:
    *//**       @brief         WFDMMSinkRenderer contructor
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
     ~WFDMMSinkRenderer();


    /*==========================================================================

             FUNCTION:         Configure

             DESCRIPTION:
    *//**       @brief         Configures the renderer
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         pCfg        - Configuration Parameters

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_ERRORTYPE
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    OMX_ERRORTYPE  Configure(rendererConfigType *pCfg);

    /*==========================================================================

             FUNCTION:          DeliverInput

             DESCRIPTION:
    *//**       @brief          Audio and Video Buffers are received through
                                this
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          trackId - Audio/Video track identifier
                                pBuffer - Buffer Header

    *//*     RETURN VALUE:
    *//**       @return
                                OMX_ErrorType
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    OMX_ERRORTYPE  DeliverInput(int trackId,
                                OMX_BUFFERHEADERTYPE *pBuffer);

    OMX_ERRORTYPE  Stop();

    OMX_ERRORTYPE  Start();

    void SetMediaBaseTime(uint64 nTime);

    void pauseRendering(void);

    void restartRendering(bool flush);

    void setFlushTimeStamp(uint64 nTS);

    void setDecoderLatency(uint64 nLatencyInms);

private:

    /*==========================================================================

             FUNCTION:         createResources

             DESCRIPTION:
    *//**       @brief         creates resources for WFDMMSinkRenderer
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_ERRORTYPE
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    OMX_ERRORTYPE  createResources();

    /*==========================================================================

             FUNCTION:          AllocateBuffers

             DESCRIPTION:
    *//**       @brief          Allocates Required Buffers
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                None
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    OMX_ERRORTYPE  AllocateBuffers();

    /*==========================================================================

             FUNCTION:          createAudioResources

             DESCRIPTION:
    *//**       @brief          creates Audio Resources. Hmm
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                true or false
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    bool  createAudioResources();

    /*==========================================================================

             FUNCTION:         createThreadsAndQueues

             DESCRIPTION:
    *//**       @brief         creates threads and queues needed for the module
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               bool
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    bool  createThreadsAndQueues();

    /*==========================================================================

             FUNCTION:          Scheduler

             DESCRIPTION:
    *//**       @brief          Schedules Audio and Video Frames for rendering
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                None
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    bool  Scheduler(int trackId, frameInfoType *pFrameInfo);

    /*==========================================================================

             FUNCTION:          PushVideo4Render

             DESCRIPTION:
    *//**       @brief          Pushes video frames for rendering
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          pFrameInfo - Current frame to be rendered

    *//*     RETURN VALUE:
    *//**       @return
                                true or false
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    bool  PushVideo4Render(frameInfoType *pFrameInfo);

    /*==========================================================================

             FUNCTION:          PushAudio4Render

             DESCRIPTION:
    *//**       @brief          Render the latest audio frame
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                true or false
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    bool  PushAudio4Render(frameInfoType *pFrameInfo);

    /*==========================================================================

             FUNCTION:          RenderVideo

             DESCRIPTION:
    *//**       @brief          Render the video. Push to Native window
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                None
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    bool  RenderVideo(frameInfoType *pFrameInfo);

    /*==========================================================================
             FUNCTION:         SearchBuffer
             DESCRIPTION:
    *//**       @brief         Search for a node in the packet queue
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         pH - pointer the the list handle
                               pFrameInfo - pointer to hold the packet
                                            once found
                               seqNum - sequence number of the packet being
                                             searched
    *//*     RETURN VALUE:
    *//**       @return
                               0 -success
                               -1 failure
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    bool  SearchBuffer(QMM_ListHandleType *pH,
                                        frameInfoType **pFrameInfo,
                                        long bufferId);

    /*==========================================================================
             FUNCTION:         ComparePayloadParam
             DESCRIPTION:
    *//**       @brief         Compare a mamber in a node against a specified
                               value
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         pElement - pointer the the list Element
                               pCmpValue - pointer to value to be compared
    *//*     RETURN VALUE:
    *//**       @return
                               QMM_ListErrorType
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    static QMM_ListErrorType  ComparePayloadParam
    (
       void *pElement,
       void *pCmpValue
    );

    /*==========================================================================

             FUNCTION:         deinitialize

             DESCRIPTION:
    *//**       @brief         deallocated all resources
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               OMX_ERRORTYPE
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    OMX_ERRORTYPE  deinitialize();


    /*==========================================================================

             FUNCTION:         VideoThreadEntry

             DESCRIPTION:
    *//**       @brief         Static entry function called from WFDMMThread
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    static void  VideoThreadEntry(void* pThis,
                                  unsigned int nSignal);


    /*==========================================================================

             FUNCTION:         VideoThread

             DESCRIPTION:
    *//**       @brief         Video Thread for fetching samples from parser
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    void  VideoThread(unsigned int nSignal);

    /*==========================================================================

             FUNCTION:         AudioThreadEntry

             DESCRIPTION:
    *//**       @brief         Static entry function called from WFDMMThread
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*========================================================================*/
    static void  AudioThreadEntry(void* pThis,
                                  unsigned int nSignal);
    /*==========================================================================

             FUNCTION:         AudioThread

             DESCRIPTION:
    *//**       @brief         Audio Thread for fetching samples from parser
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    void  AudioThread(unsigned int nSignal);

    /*==========================================================================

             FUNCTION:         VideoThreadEntry

             DESCRIPTION:
    *//**       @brief         Static entry function called from WFDMMThread
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    static void  GraphicThreadEntry(void* pThis,
                                    unsigned int nSignal);


    /*==========================================================================

             FUNCTION:         GraphicThread

             DESCRIPTION:
    *//**       @brief         Polls for rendered buffers
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         None

    *//*     RETURN VALUE:
    *//**       @return
                               None
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    void  GraphicThread(unsigned int nSignal);

    /*==========================================================================
             FUNCTION:         state
             DESCRIPTION:
    *//**       @brief         Updates or looks at current state
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         state   - target State if set
                               get_true_set_false - get or set state
    *//*     RETURN VALUE:
    *//**       @return
                               current state
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    int  state(int state, bool get_true_set_false);

    /*==========================================================================
             FUNCTION:         AccessPendingQ
             DESCRIPTION:
    *//**       @brief         Access pending Queue
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         pPkt    - pointer to hold the packet
                               bAction - action to be performed on the Queue
    *//*     RETURN VALUE:
    *//**       @return
                               0 - success
                               -1 - failure
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    int  AccessPendingQ(frameInfoType **pPkt,
                        actionType bAction);

    /*==========================================================================
             FUNCTION:         AccessRenderQ
             DESCRIPTION:
    *//**       @brief         Access buffers Queued for rendering
    *//**
    @par     DEPENDENCIES:
                               None
    *//*
             PARAMETERS:
    *//**       @param         pPkt    - pointer to hold the packet
                               bAction - action to be performed on the Queue
    *//*     RETURN VALUE:
    *//**       @return
                               0 - success
                               -1 - failure
    @par     SIDE EFFECTS:
                               None
    *//*======================================================================*/
    int  AccessRenderQ(frameInfoType **pPkt, uint64 handle,
                                           actionType bAction);

    /*==========================================================================

             FUNCTION:         getCurrentTimeUs

             DESCRIPTION:
    *//**       @brief         returns the current system clock in microsecond
                               precision
    *//**
    @par     DEPENDENCIES:
                                None
    *//*
             PARAMETERS:
    *//**       @param          None

    *//*     RETURN VALUE:
    *//**       @return
                                Current time in Us
    @par     SIFE EFFECTS:
                                None
    *//*======================================================================*/
    uint64  getCurrentTimeUs();

    void    InitData();

    bool    anyChangeInCrop(pictureInfoType *);//TBD

    bool    accessFrameInfoMeta(frameInfoType **pFrameInfo,
                             actionType action);

    bool    GenerateAACHeaderFromADTS(uint8 *pAdts,      uint32 nAdtsLen,
                                   uint8 *pAacHeader, uint32 *nAACHdrLen);

    static void  AudioTrackCb(int  nEvent,
                                      void *pCookie,
                                      void *pInfo);
    void prepareDumpFiles();

    void AddAudioTrackLatency();

    bool createAudioTrack();

    void ProcessAudioTrackCb(int nEvent, void* pInfo);

    uint64             mnBaseMediaTime;
    frameInfoType      msFrameInfo[RENDERERMAXQSIZES];
    pictureInfoType    mPictInfo;
    SignalQueue       *mFrameInfoFreeQ;
    SignalQueue       *mAudioQ;
    QMM_ListHandleType mVideoRenderQ;
    QMM_ListHandleType mVideoPendingQ;
    WFDMMThreads      *mpVideoThread;
    WFDMMThreads      *mpAudioThread;
    WFDMMThreads      *mpGraphicPollThread;
    MM_HANDLE          mhCritSect;
#ifndef USE_OMX_AAC_CODEC
    WFDMMSinkAudioDecode *mpAudioDecoder;
#endif
    android::sp<ANativeWindow>  mpWindow;
    android::sp<android::AudioTrack>  mpAudioTrack;
    rendererConfigType mCfg;
    int                meState;
    void*              mClientData;
    int                mnModuleId;
    int                mnErr;
    int                mnAudioSessId;
    uint64             mnNumVidQedBufs;
    uint64             mnNumVidDQedBufs;
    uint64             mnFlushTimeStamp;
    uint32             mnVideoDropFrames;
    uint32             mnAudioDropBytes;
    uint32             mnVideoMaxConsecDrop;
    uint32             mnVideoDropCount;
    uint32             mnAudioLatencyCheckCount;
    uint32             mnAddAudioTrackLatency;
    FILE              *mpFpAAC;
    FILE              *mpFpWAV;
    uint32             mnAudioLatency;
    WFDMMSinkEBDType       mpFnEBD;
    WFDMMSinkHandlerFnType   mpFnHandler;
    bool               mbFirstFrame;
    bool               mbMediaTimeSet;
    bool               mbAudioCodecHdrSet;
    bool               mbDumpWav;
    bool               mbDumpAAC;
    bool               mbLookForIDR;
    bool               mbPaused;
    bool               mbFlushInProgress;
    bool               mbRestartAudioTrack;
    int32              mnAudioOffset;
    uint64             mnFirstAudioFrameTimeStamp;
    bool               mbAudioTimeReset;
    bool               mbVideoTimeReset;
};

#endif /*__WFD_MM_SINK_RENDERER_H__*/
