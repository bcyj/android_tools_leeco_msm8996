/*==============================================================================
*       WFDMMSinkRenderer.cpp
*
*  DESCRIPTION:
*       Does AV SYnc and render.
*
*
*  Copyright (c) 2013-14 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/25/2013         SK            InitialDraft
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
#include <cutils/properties.h>
#include "WFDMMLogs.h"
#include "WFDMMSinkRenderer.h"
#include "WFDMMThreads.h"
#include "WFDMMSourceMutex.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "qmmList.h"
#include "MMTimer.h"
#include "AudioTrack.h"
#include "gralloc_priv.h"
#include "qdMetaData.h"
#ifndef USE_OMX_AAC_CODEC
#include "WFDMMSinkAudioDecode.h"
#endif
#define MAX_CONSEQ_VIDEO_DROP 2
#define MAX_CONSEQ_AUDIO_DROP 5
#define MAX_CONSEQ_VIDEO_FRAMES_COUNT 30
#define DEFAULT_AUDIO_TRACK_LATENCY 90000

#include "WFDMMSinkStatistics.h"
#define AUDIO_RENDER_TOLERANCE 2000 //us
#define VIDEO_RENDER_TOLERANCE 2000 //us

#define AUDIO_AVSYNC_DROP_WINDOW_AUDIO_ONLY -400000

#define INITIAL_AUDIO_DROP_WINDOW 700000 //700ms
using namespace android;

#define CRITICAL_SECT_ENTER if(mhCritSect)                                     \
                                  MM_CriticalSection_Enter(mhCritSect);        \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE if(mhCritSect)                                     \
                                  MM_CriticalSection_Leave(mhCritSect);        \
/*      END CRITICAL_SECT_LEAVE    */

#define NOTIFYERROR  mpFnHandler((void*)(int64)mClientData,                    \
                                 mnModuleId,WFDMMSINK_ERROR,                   \
                                 OMX_ErrorUndefined,                           \
                                 0 );                                          \
/*NOTIFYERROR*/

#define WFD_DUMP_PCM
#define WFD_DUMP_AAC

/*------------------------------------------------------------------------------
 State definitions
--------------------------------------------------------------------------------
*/
#define DEINIT  0
#define INIT    1
#define OPENED  2
#define CLOSING 4
#define CLOSED  3

#define ISSTATEDEINIT  (state(0, true) == DEINIT)
#define ISSTATEINIT    (state(0, true) == INIT)
#define ISSTATEOPENED  (state(0, true) == OPENED)
#define ISSTATECLOSED  (state(0, true) == CLOSED)
#define ISSTATECLOSING (state(0, true) == CLOSING)

#define ISERROR         (mnErr == -1)

#define SETSTATECLOSING (state(CLOSING, false))
#define SETSTATEINIT    (state(INIT   , false))
#define SETSTATEDEINIT  (state(DEINIT , false))
#define SETSTATEOPENED  (state(OPENED , false))
#define SETSTATECLOSED  (state(CLOSED , false))


/*==============================================================================

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
*//*==========================================================================*/
WFDMMSinkRenderer::WFDMMSinkRenderer(int moduleId,
                                     WFDMMSinkEBDType       pFnEBD,
                                     WFDMMSinkHandlerFnType   pFnHandler,
                                     void* clientData)
{

    InitData();
    /*--------------------------------------------------------------------------

    ----------------------------------------------------------------------------
    */
    mpFnHandler = pFnHandler;
    mpFnEBD     = pFnEBD;
    mnModuleId  = moduleId;
    mClientData = clientData;

    mhCritSect = NULL;
#ifndef USE_OMX_AAC_CODEC
    mpAudioDecoder = NULL;
#endif
    int nRet = MM_CriticalSection_Create(&mhCritSect);
    if(nRet != 0)
    {
        mhCritSect = NULL;
        WFDMMLOGE("Error in Critical Section Create");
    }

    SETSTATEDEINIT;

}

/*==============================================================================

         FUNCTION:          ~WFDMMSinkRenderer

         DESCRIPTION:
*//**       @brief          Destructor for renderer class
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
WFDMMSinkRenderer::~WFDMMSinkRenderer()
{
    WFDMMLOGH("WFDMMSinkRenderer Destructor");
    deinitialize();
}

/*==============================================================================

         FUNCTION:          InitData

         DESCRIPTION:
*//**       @brief          Initializes class variables
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkRenderer::InitData()
{
    mbFirstFrame = true;
    mAudioQ  = NULL;
    mFrameInfoFreeQ = NULL;
    mpVideoThread = NULL;
    mpAudioThread = NULL;
    mpFpAAC = NULL;
    mbDumpAAC = false;
    mpFpWAV = NULL;
    mbDumpWav = false;
    mpGraphicPollThread = NULL;
    mhCritSect = NULL;
    mpWindow = NULL;
    mpAudioTrack = NULL;
    mbMediaTimeSet = false;
    mbAudioTimeReset = true;
    mbVideoTimeReset = true;
    mnNumVidQedBufs = 0;
    mnNumVidDQedBufs = 0;
    mbAudioCodecHdrSet = false;
    memset(&mCfg, 0, sizeof(mCfg));
    memset(&mPictInfo, 0, sizeof(mPictInfo));
    memset(&msFrameInfo, 0, sizeof(msFrameInfo));
    meState = DEINIT;
    mClientData = 0;
    mnModuleId = 0;
    mpFnEBD = NULL;
    mpFnHandler = NULL;
    mnBaseMediaTime = 0;
    mbMediaTimeSet = false;
    mnErr = 0;
    mnVideoMaxConsecDrop = MAX_CONSEQ_VIDEO_DROP;
    mnVideoDropCount = 0;
    mbLookForIDR = 0;
    mbFlushInProgress = false;
    mnFlushTimeStamp = 0;
    mbPaused = false;
    mnAudioLatencyCheckCount = 0;
    mnFirstAudioFrameTimeStamp= 0;
    mnAddAudioTrackLatency = 0;
    mnAudioOffset = -1;
    return;
}

/*==============================================================================

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
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::Configure(rendererConfigType *pCfg)
{
    CHECK_ARGS_RET_OMX_ERR_3(pCfg, mpFnHandler, mpFnEBD);

    memcpy(&mCfg, pCfg, sizeof(rendererConfigType));


    if(pCfg->bHasVideo)
    {
        mpWindow = pCfg->pWindow;

        if(mpWindow == NULL)
        {
            return OMX_ErrorBadParameter;
        }
    }

    if(createResources() != OMX_ErrorNone)
    {
        WFDMMLOGE("Renderer Create Resources Failed");
        return OMX_ErrorInsufficientResources;
    }
    SETSTATEINIT;
    return OMX_ErrorNone;
}

/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::AllocateBuffers()
{
    OMX_ERRORTYPE eErr = OMX_ErrorNone;

    /*--------------------------------------------------------------------------
     WFDMMSinkRenderer does not allocates Buffers. Revisit
    ----------------------------------------------------------------------------
    */
    return eErr;
}

/*==============================================================================

         FUNCTION:          DeliverInput

         DESCRIPTION:
*//**       @brief          Audio and Video Buffers are received through this
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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::DeliverInput(int trackId,
                                             OMX_BUFFERHEADERTYPE *pBuffer)
{
    /*--------------------------------------------------------------------------
     If we havent started and buffers are received with nFilledLen 0, thse
     buffers has to be cancelled back to Native Window
     (ISSTATEINIT && pFrameInfo->pBuffer->nFilledLen == 0) this condition is
     for this purpose
    ----------------------------------------------------------------------------
    */
    if (ISSTATECLOSING || ISSTATEDEINIT || ISSTATECLOSED)
    {
        /*----------------------------------------------------------------------
         We can allow buffer flow in INIT and OPENED
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("Output Delivered when state not in play");
        return OMX_ErrorInvalidState;
    }
    WFDMMLOGM("WFDMMSinkRenderer DeliverInput");

    if (!pBuffer)
    {
        WFDMMLOGE("Invalid Arguments");
        return OMX_ErrorBadParameter;
    }

    /*--------------------------------------------------------------------------
     Check if Video or Audio is advertized during start up. Reject any samples
     from unsupported track
    ----------------------------------------------------------------------------
    */
    if((trackId == SINK_VIDEO_TRACK_ID && !mCfg.bHasVideo) ||
       (trackId == SINK_AUDIO_TRACK_ID && !mCfg.bHasAudio))
    {
        WFDMMLOGE("Sample for track when track not advertized");
        return OMX_ErrorBadParameter;
    }


    if( trackId == SINK_VIDEO_TRACK_ID && !mbFirstFrame &&
        !pBuffer->nFilledLen)
    {
        WFDMMLOGE("Not queing 0 sized frames to renderer");
        return OMX_ErrorBadParameter;
    }

    /*--------------------------------------------------------------------------
        Check if timebase has changed and needs reset
    ----------------------------------------------------------------------------
    */
    if(trackId == SINK_AUDIO_TRACK_ID &&
      mbAudioTimeReset && pBuffer->nFilledLen)
    {
        WFDMMLOGH("Audio Time Reset. Waiting");
        if((pBuffer->nFlags & OMX_BUFFERFLAG_STARTTIME) == 0)
        {
            WFDMMLOGE("Renderer waiting for Audio STARTTIME");
            return OMX_ErrorInvalidState;
        }
        else
        {
            mbAudioTimeReset = false;
        }
    }

    if(trackId == SINK_VIDEO_TRACK_ID &&
      mbVideoTimeReset && pBuffer->nFilledLen)
    {
        if((pBuffer->nFlags & OMX_BUFFERFLAG_STARTTIME) == 0)
        {
            WFDMMLOGE("Renderer Waiting for Video STARTTIME");
            return OMX_ErrorInvalidState;
        }
        else
        {
            mbVideoTimeReset = false;
        }
    }
#ifndef USE_AUDIO_TUNNEL_MODE
#ifndef USE_OMX_AAC_CODEC
    /*--------------------------------------------------------------------------
     Decode frame it is AAC. In place decoding is done. The time taken for
     decoding will be considered in AV Sync
    ----------------------------------------------------------------------------
    */
    if(trackId == SINK_AUDIO_TRACK_ID &&
       mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_AAC)
    {
#ifdef WFD_DUMP_AAC
        if(mpFpAAC)
        {
            fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, mpFpAAC);
        }
#endif
        OMX_ERRORTYPE eErr = mpAudioDecoder->Decode(pBuffer);
        if(eErr != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to decode AAC frame");
          //  NOTIFYERROR;
            return OMX_ErrorDynamicResourcesUnavailable;
        }
    }
#endif
#endif

    frameInfoType *pFrameInfo = NULL;

    accessFrameInfoMeta(&pFrameInfo, popfront);

    if (!pFrameInfo)
    {
        WFDMMLOGE("Failed to get frameInfo");
        NOTIFYERROR;
        return OMX_ErrorInsufficientResources;
    }

    pFrameInfo->pBuffer = pBuffer;


    if(!Scheduler(trackId, pFrameInfo))
    {
        accessFrameInfoMeta(&pFrameInfo, pushrear);
        NOTIFYERROR;
        WFDMMLOGE1("Failed to Schedule track = %d", trackId);
        return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}
/*==============================================================================

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
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::createResources()
{

    if(!createThreadsAndQueues())
    {
        WFDMMLOGE("Renderer Failed to create Threads and Queues");
        return OMX_ErrorInsufficientResources;
    }
    if(mCfg.bHasAudio)
    {
        if(!createAudioResources())
        {
            WFDMMLOGE("Renderer failed to create audio resources");
            NOTIFYERROR;
            return OMX_ErrorInsufficientResources;
        }
#ifdef USE_OMX_AAC_CODEC
        mnAudioDropBytes = mCfg.sAudioInfo.nSampleRate *
                           mCfg.sAudioInfo.nChannels * 2;
#endif
    }

    if(mCfg.bHasVideo)
    {
        if(mpWindow == NULL)
        {
            WFDMMLOGE("No valid native window to render");
            return OMX_ErrorInsufficientResources;
        }
        mnVideoDropFrames = mCfg.sVideoInfo.nFrameRate;
    }

    prepareDumpFiles();

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          createAudioTrack

         DESCRIPTION:
*//**       @brief          creates Audio Track
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true or false
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/

bool WFDMMSinkRenderer::createAudioTrack()
{
    if(mpAudioTrack != NULL)
    {
        WFDMMLOGD("Destory Previous Audio Track");
        mpAudioTrack->stop();
    }
    int AudioFmt = AUDIO_FORMAT_PCM_16_BIT;
    int nFlags   = AUDIO_OUTPUT_FLAG_NONE;

    if(mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_LPCM)
    {
        AudioFmt = AUDIO_FORMAT_PCM_16_BIT;
    }
    else if(mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_AAC)
    {
#ifdef USE_AUDIO_TUNNEL_MODE
        AudioFmt = AUDIO_FORMAT_AAC;
        nFlags = AUDIO_OUTPUT_FLAG_TUNNEL|AUDIO_OUTPUT_FLAG_DIRECT;
#else //USE_AUDIO_TUNNEL_MODE
        AudioFmt = AUDIO_FORMAT_PCM_16_BIT;
#ifdef USE_OMX_AAC_CODEC
        WFDMMLOGE("Can't support non LPCM in Renderer");
        return false;
#endif//USE_OMX_AAC_CODEC
#endif//USE_AUDIO_TUNNEL_MODE
    }
    else if(mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_DOLBY_DIGITAL)
    {
        AudioFmt = AUDIO_FORMAT_AC3;
#ifdef USE_AUDIO_TUNNEL_MODE
        nFlags = AUDIO_OUTPUT_FLAG_TUNNEL|AUDIO_OUTPUT_FLAG_DIRECT;
#endif
    }

#ifdef USE_AUDIO_TUNNEL_MODE
    if(nFlags & AUDIO_OUTPUT_FLAG_TUNNEL)
    {
        WFDMMLOGH("Tunnel mode audio Configure audiosession");
        mnAudioSessId = AudioSystem::newAudioSessionId();
        AudioSystem::acquireAudioSessionId(mnAudioSessId,getpid());
    }
#endif
    nFlags |= AUDIO_OUTPUT_FLAG_FAST;

    WFDMMLOGD2("SinkRenderer Audio SampleRate %d Channels %d",
               mCfg.sAudioInfo.nSampleRate, mCfg.sAudioInfo.nChannels);
    mpAudioTrack = MM_New_Args(android::AudioTrack, (
                          AUDIO_STREAM_MUSIC,
                          mCfg.sAudioInfo.nSampleRate,
                          (audio_format_t)AudioFmt,
                          audio_channel_out_mask_from_count
                                     (mCfg.sAudioInfo.nChannels),
                          (int)0,
                         (audio_output_flags_t) nFlags,
                          AudioTrackCb, this, 0, mnAudioSessId,
                           android::AudioTrack::TRANSFER_SYNC));

    if(mpAudioTrack == NULL)
    {
        WFDMMLOGE("Failed to create Audio Track");
        return false;
    }

    status_t nErr = mpAudioTrack->initCheck();

    if(nErr != 0)
    {
        WFDMMLOGE("Failed to Init Audio track");
        return false;
    }

    mpAudioTrack->start();
    mnAudioLatency = mpAudioTrack->latency() * 1000;
    WFDMMLOGH1("AudioTrack STarted with latency %ld",mnAudioLatency);

    return true;
}
/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::createAudioResources()
{

    if(createAudioTrack() == false)
    {
        WFDMMLOGE("Failed to create Audio Track");
        return false;
    }
#ifndef USE_AUDIO_TUNNEL_MODE
#ifndef USE_OMX_AAC_CODEC
    /*--------------------------------------------------------------------------
      Create A decoder if the audio codec needs One. Currently only AAC needs
      a decoder. Make sure to update this comment if you are adding AC3
    ----------------------------------------------------------------------------
    */

    if(mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_AAC)
    {
        mpAudioDecoder = WFDMMSinkAudioDecode::create(WFD_AUDIO_AAC);
        if(!mpAudioDecoder)
        {
            WFDMMLOGE("Failed to create Audio Decoder");
            return false;
        }
        audioConfigType sAudioCfg;
        sAudioCfg.eAudioType     = mCfg.sAudioInfo.eAudioFmt;
        sAudioCfg.nBitsPerSample = 16;
        sAudioCfg.nChannels      = mCfg.sAudioInfo.nChannels;
        sAudioCfg.nSampleRate    = mCfg.sAudioInfo.nSampleRate;

        OMX_ERRORTYPE eRet = mpAudioDecoder->Configure(&sAudioCfg);

        if(eRet != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to configure Audio decoder");
            return false;
        }
        WFDMMLOGH("AudioDecoder Created");
    }
#endif
#endif
    return true;
}

/*==============================================================================

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
*//*==========================================================================*/
bool WFDMMSinkRenderer::createThreadsAndQueues()
{
    /*--------------------------------------------------------------------------
      Create threads if session has audio and video
    ----------------------------------------------------------------------------
    */
    if(mCfg.bHasVideo)
    {
        mpVideoThread = MM_New_Args(WFDMMThreads, (1));

        if(!mpVideoThread)
        {
            WFDMMLOGE("Failed to create Video Thread");
            return false;
        }

        mpVideoThread->Start(VideoThreadEntry, -2,
                             32768,this, "WFDSinkVideorenderer");

        mpGraphicPollThread = MM_New_Args(WFDMMThreads, (1));

        if(!mpGraphicPollThread)
        {
            WFDMMLOGE("Failed to create Graphic Thread");
            return false;
        }

        mpGraphicPollThread->Start(GraphicThreadEntry, -2,
                             32768,this, "WFDSinkGraphicrenderer");

    }


    if(mCfg.bHasAudio)
    {
        mpAudioThread = MM_New_Args(WFDMMThreads, (1));

        if(!mpAudioThread)
        {
            WFDMMLOGE("Failed to create Audio Thread");
            return false;
        }

        mpAudioThread->Start(AudioThreadEntry, -2,
                             65536*2, this, "WFDSinkAudioRenderer");

    }

    if(mCfg.bHasAudio)
    {
        mAudioQ = MM_New_Args(SignalQueue, (100, sizeof(int*)));
        if(!mAudioQ)
        {
            return false;
        }
    }

    if(mCfg.bHasVideo)
    {
        (void)qmm_ListInit(&mVideoPendingQ);
        (void)qmm_ListInit(&mVideoRenderQ);
    }


    mFrameInfoFreeQ = MM_New_Args(SignalQueue,
                                  (RENDERERMAXQSIZES, sizeof(int*)));
    /*--------------------------------------------------------------------------
     Frame Info Q is used to keep a list of frameInfo structures. These are
     meta data associated with each buffer that is going to be scheduled for
     rendering
    ----------------------------------------------------------------------------
    */
    if(!mFrameInfoFreeQ)
    {
        WFDMMLOGE("Failed to allocate the FrameInfoQ");
        return false;
    }

    /*--------------------------------------------------------------------------
     Populate the FreeQ with all the frameInfo structure objects that we have
    ----------------------------------------------------------------------------
    */

    for(int i = 0; i < RENDERERMAXQSIZES; i++)
    {
        uint8 *pFrameInfoPtr = (uint8*)(msFrameInfo + i);
        mFrameInfoFreeQ->Push(&pFrameInfoPtr, sizeof(int*));
    }
    return true;
}

/*==============================================================================

         FUNCTION:          accessFrameInfoMeta

         DESCRIPTION:
*//**       @brief          Objects of frameInfo are used to store Audio and
                            video buffers in queues before being rendered.
                            This function is used to either get an empty
                            frameinfo object or to push a frame info object back
                            to queue once its use is complete.
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::accessFrameInfoMeta(frameInfoType **pFrameInfo,
                                            actionType action)
{
    CRITICAL_SECT_ENTER
    /*--------------------------------------------------------------------------
     Note that this Q supports only push and pop. Therefore poprear and popfront
     maps to pop and pushrear and pushfront maps to push
    ----------------------------------------------------------------------------
    */
    if(mFrameInfoFreeQ && pFrameInfo)
    {
        if(action == poprear || action == popfront)
        {
            *pFrameInfo = NULL;

            mFrameInfoFreeQ->Pop(pFrameInfo, sizeof(pFrameInfo), 0);

            if(*pFrameInfo)
            {
                /*--------------------------------------------------------------
                 Found a free frameinfo object
                ----------------------------------------------------------------
                */
                CRITICAL_SECT_LEAVE
                return true;
            }
            WFDMMLOGE("FrameInfo Popped is NULL");
        }
        else if(action == pushrear || action == pushfront)
        {
            if(!(*pFrameInfo))
            {
                WFDMMLOGE("Invalid FrameInfo Pushed");
                CRITICAL_SECT_LEAVE;
                return false;
            }
            OMX_ERRORTYPE eErr =
                mFrameInfoFreeQ->Push(pFrameInfo, sizeof(pFrameInfo));

            if(eErr == OMX_ErrorNone)
            {
                /*--------------------------------------------------------------
                 Push free frameinfo object
                ----------------------------------------------------------------
                */
                CRITICAL_SECT_LEAVE
                return true;
            }
        }
    }
    WFDMMLOGE("Failed to operate on FrameInfo Q");
    CRITICAL_SECT_LEAVE
    return false;
}


/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::Scheduler(int trackId, frameInfoType *pFrameInfo)
{

    /*--------------------------------------------------------------------------
     TODO - do any adjustments based on different propagation delays here.
     For this delays of audio and video paths has to be measured.
    ----------------------------------------------------------------------------
    */
    pFrameInfo->nTSArrival = getCurrentTimeUs();;
    pFrameInfo->nTSSched   = pFrameInfo->pBuffer->nTimeStamp;

    if(trackId == SINK_VIDEO_TRACK_ID)
    {
        if(mbFirstFrame && !pFrameInfo->pBuffer->nFilledLen)
        {
            WFDMMLOGH("Cancel Initial frames to Native Window");
            /*------------------------------------------------------------------
              Cancel Initial Buffers. Don't know why, but native window
              doesnt work without this and no documentation provided.
            --------------------------------------------------------------------
            */
            pictureInfoType *pPictInfo =
                           (pictureInfoType *)pFrameInfo->pBuffer->pAppPrivate;

            GraphicBuffer *pGraphicBuffer = (GraphicBuffer *)
                                        pPictInfo->pGraphicBuffer;

            int nErr = mpWindow->cancelBuffer(
                          mpWindow.get(),
                          pGraphicBuffer->getNativeBuffer(),
                         -1);
            if (nErr != 0)
            {
                WFDMMLOGE("Failed to cancel graphic buffer");
                return false;
            }
            mnNumVidQedBufs++;
            AccessRenderQ(&pFrameInfo, (uint32)0, pushrear);
            return true;
        }
        /*----------------------------------------------------------------------
          At this time we should have received Base time, if not take it here.
        ------------------------------------------------------------------------
        */
        if(!mbMediaTimeSet)
        {
            mnBaseMediaTime = 0;
            mnBaseMediaTime = getCurrentTimeUs();
            mbMediaTimeSet  = true;
        }
        WFDMMLOGH("Push Video to Pending Q");
        AccessPendingQ(&pFrameInfo, pushrear);
        return true;
    }
    else
    {
        /*----------------------------------------------------------------------
          At this time we should have received Base time, if not take it here.
        ------------------------------------------------------------------------
        */
        if(!mbMediaTimeSet)
        {
            mnBaseMediaTime = 0;
            mnBaseMediaTime = getCurrentTimeUs();
            mbMediaTimeSet  = true;
        }
        WFDMMLOGH("Push Audio to Pending Q");
        if(mAudioQ)
        {
            mAudioQ->Push(&pFrameInfo, sizeof(&pFrameInfo));
        }
        return true;
    }

    return false;

}

/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::PushVideo4Render(frameInfoType *pFrameInfo)
{

    if(pFrameInfo != NULL)
    {
#ifdef DROP_INITIAL_VIDEO_FRAMES
        if(mnVideoDropFrames)
        {
            mnVideoDropFrames--;

            if(mnVideoDropFrames % 3)
            {
                pFrameInfo->pBuffer->nFilledLen = 0;
                mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                        pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushfront);
                return true;
            }
        }
#endif

        if(mCfg.sVideoInfo.nFrameDropMode == FRAME_DROP_DROP_TILL_IDR)
        {
            if(pFrameInfo->pBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
            {
                mbLookForIDR = false;
                WFDMMLOGE("Renderer Found IDR, stop frame drop");
            }
            if(pFrameInfo->pBuffer->nFlags & OMX_BUFFERFLAG_DATACORRUPT)
            {
                mbLookForIDR = true;
                WFDMMLOGE("Renderer Drop Frame, Corrupt Data");
            }
            if(mbLookForIDR)
            {
                WFDMMLOGE("Renderer Drop Frame. Wait for IDR");
                pFrameInfo->pBuffer->nFilledLen = 0;
                mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                    pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushfront);
                return true;
            }
        }

        /*------------------------------------------------------------------
          First push to render Q to avoid any race condition. Then send to
          actualRenders
        --------------------------------------------------------------------
        */
        AccessRenderQ(&pFrameInfo, (uint32)0, pushrear);

        bool bRet = false;

        WFDMMLOGM("Render Video");
        bRet = RenderVideo(pFrameInfo);

        if(!bRet)
        {
            WFDMMLOGE("Failed to Render video track");
            pFrameInfo = NULL;
            AccessRenderQ(&pFrameInfo, (uint32)0, poprear);
            if(pFrameInfo)
            {
                mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                        pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushfront);
            }
            return false;
        }
    }
    return true;
}

/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::PushAudio4Render(frameInfoType *pFrameInfo)
{
    /*--------------------------------------------------------------------------
      See if there are video or audio frames pending for rendering
    ----------------------------------------------------------------------------
    */
    WFDMMLOGD("Push Audio For Rendering");

    if(mbRestartAudioTrack)
    {
        WFDMMLOGH("Restart Audio Track");
        mbRestartAudioTrack = false;
        if(createAudioTrack() == false)
        {

            mpFnEBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID,
                 pFrameInfo->pBuffer);
            accessFrameInfoMeta(&pFrameInfo, pushfront);
            NOTIFYERROR;
        }
    }

#ifdef USE_AUDIO_TUNNEL_MODE
    if(!mbAudioCodecHdrSet && mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_AAC)
    {
        uint8 sAACHeader[2];
        uint32 nSize = 2;

        bool bRet = GenerateAACHeaderFromADTS((uint8*)pFrameInfo->pBuffer->pBuffer,
                                  (uint32)pFrameInfo->pBuffer->nFilledLen,
                                  (uint8*)sAACHeader,
                                  &nSize);

        if(nSize >= 2 && bRet == true)
        {
            mpAudioTrack->write(sAACHeader,nSize);
        }
        mbAudioCodecHdrSet = true;
    }
#endif

    if(pFrameInfo != NULL)
    {
        WFDMMLOGD("Push Audio For Rendering to audio Track");
        int nBytesWritten = 0;

        uint8* pData = pFrameInfo->pBuffer->pBuffer +
                         pFrameInfo->pBuffer->nOffset;
        uint32 nDataSize = pFrameInfo->pBuffer->nFilledLen;

#ifdef USE_AUDIO_TUNNEL_MODE
        if(mCfg.sAudioInfo.eAudioFmt == WFD_AUDIO_AAC)
        {
            if(nDataSize <= 7)
            {
                pFrameInfo->pBuffer->nFilledLen = 0;
                mpFnEBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID,
                         pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushfront);
                return true;
            }
            pData += 7;
            nDataSize -= 7;
        }
#endif
        if(pFrameInfo->pBuffer->nFilledLen)
        {
#ifdef USE_OMX_AAC_CODEC
            /*------------------------------------------------------------------
             OMX AAC cocec intriduces an initial latency. We need to drop some
             amount of data to catchup.
            --------------------------------------------------------------------
            */
            if(mnAudioDropBytes)
            {
                if(pFrameInfo->pBuffer->nFilledLen < mnAudioDropBytes)
                {
                    mnAudioDropBytes -= pFrameInfo->pBuffer->nFilledLen;
                }
                else
                {
                    mnAudioDropBytes = 0;
                }
                WFDMMLOGH("Renderer: Drop Initial Audio");
            }
            else
            {
#endif

#ifdef WFD_DUMP_PCM
                if(mpFpWAV)
                {
                    fwrite(pData, 1, nDataSize, mpFpWAV);
                }

#endif
                nBytesWritten = mpAudioTrack->write(pData,nDataSize);
#ifdef USE_OMX_AAC_CODEC
            }
#endif
            WFDMMLOGD1("PCM Write NumBytes Written = %d", nBytesWritten);
            pFrameInfo->pBuffer->nFilledLen = 0;
            /* Debug Code , can be enabled if needed */
            //mnAudioLatency = mpAudioTrack->latency();
            //WFDMMLOGM1("mpAudioTrack->latency() = %ld ms",mnAudioLatency);
        }

        mpFnEBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID,
                 pFrameInfo->pBuffer);
        accessFrameInfoMeta(&pFrameInfo, pushfront);
    }

    return true;
}


/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkRenderer::RenderVideo(frameInfoType *pFrameInfo)
{
    if(!pFrameInfo || !pFrameInfo->pBuffer ||
                 !pFrameInfo->pBuffer->pAppPrivate)
    {
        WFDMMLOGE("Invalid FrameBuffer passed");
        return false;
    }

    pictureInfoType *pPictInfo = (pictureInfoType *)
                                  pFrameInfo->pBuffer->pAppPrivate;

    GraphicBuffer *pGraphicBuffer = (GraphicBuffer *)pPictInfo->pGraphicBuffer;
    ANativeWindowBuffer * pNativeBuffer = pGraphicBuffer->getNativeBuffer();
    BufferDim_t bufferDim = {(int32_t)pPictInfo->rect.nWidth,
                             (int32_t)pPictInfo->rect.nHeight};
    int nRets = setMetaData((private_handle_t*)pNativeBuffer->handle,
                            UPDATE_BUFFER_GEOMETRY, &bufferDim);
    if(nRets != 0)
    {
         WFDMMLOGE("Failed to set Buffer geometry");
         return false;
    }

    if(mbFirstFrame || anyChangeInCrop(pPictInfo))
    {
        /*----------------------------------------------------------------------
         Set the crop and reset dimensions if video resolution changes in
         runtime - read smoothstreaming.
        ------------------------------------------------------------------------
        */
        memcpy(&mPictInfo, pPictInfo, sizeof(mPictInfo));
        android_native_rect_t sCrop;

        sCrop.left     = pPictInfo->rect.nLeft;
        sCrop.top      = pPictInfo->rect.nTop;
        sCrop.right    = pPictInfo->rect.nLeft + pPictInfo->rect.nWidth;
        sCrop.bottom   = pPictInfo->rect.nTop + pPictInfo->rect.nHeight;

        WFDMMLOGH4("WFDMMRenderer SetCrop %d %d %d %d",
                   sCrop.left, sCrop.top, sCrop.right, sCrop.bottom);

        int nRet = 0;
/*
        nRet = mpWindow.get()->perform(mpWindow.get(),
               NATIVE_WINDOW_UPDATE_BUFFERS_GEOMETRY,
               pPictInfo->nWidth,
               pPictInfo->nHeight,
               pPictInfo->eColorFmt);
*/

        nRet = native_window_set_crop(mpWindow.get(),
                                          &sCrop);
        if(nRet != 0)
        {
            WFDMMLOGE("Failed to set crop on native window");
        }
        mbFirstFrame = false;
    }

    WFDMMLOGD("QueuBuffer to Native window");
    int nRet = mpWindow->queueBuffer(mpWindow.get(),
                                 pGraphicBuffer->getNativeBuffer(), -1);

    if(nRet != 0)
    {
        WFDMMLOGE("Failed to queueBuffer to NativeWindow");
        return false;
    }

    mnNumVidQedBufs++;

    return true;
}


/*==============================================================================
         FUNCTION:         SearchBuffer
         DESCRIPTION:
*//**       @brief         Search for a node in the packet queue
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pH - pointer the the list handle
                           pFrameInfo - pointer to hold the packet once found
                           seqNum - sequence number of the packet being searched
*//*     RETURN VALUE:
*//**       @return
                           0 -success
                           -1 failure
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkRenderer::SearchBuffer(QMM_ListHandleType *pH,
                                    frameInfoType **pFrameInfo,
                                    long bufferId)
{
    if(!pH || !pFrameInfo)
    {
        WFDMMLOGE("Invalid Arguments");
        return false;
    }

    QMM_ListErrorType eError;

    WFDMMLOGH1("Searching for Buffer Id = %ld", bufferId);

    //Look for a valid Packet in History with matching seqNo
    eError = qmm_ListSearch(pH, ComparePayloadParam, &bufferId,
                            (QMM_ListLinkType **)pFrameInfo);

    if(eError == QMM_LIST_ERROR_PRESENT && *pFrameInfo)
    {
        (void)qmm_ListPopElement(pH, (QMM_ListLinkType *)(*pFrameInfo));
    }
    else
    {
        return false;
    }

    return true;
}

/*==============================================================================
         FUNCTION:         ComparePayloadParam
         DESCRIPTION:
*//**       @brief         Compare a mamber in a node against a specified value
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
*//*==========================================================================*/
QMM_ListErrorType WFDMMSinkRenderer::ComparePayloadParam
(
   void *pElement,
   void *pCmpValue
)
{
    if(!pElement || !pCmpValue)
    {
        return QMM_LIST_ERROR_BAD_PARM;
    }

    frameInfoType *pPkt = (frameInfoType *)pElement;

    pictureInfoType *pPictInfo = (pictureInfoType *)
                                 (pPkt->pBuffer->pAppPrivate);

    if(!pPictInfo || !pPictInfo->pGraphicBuffer)
    {
        return QMM_LIST_ERROR_NOT_PRESENT;
    }
    GraphicBuffer *pGraphicBuffer = (GraphicBuffer *)pPictInfo->pGraphicBuffer;

    if(*(reinterpret_cast<long*>(pCmpValue)) ==
       reinterpret_cast<long>(pGraphicBuffer->handle))
    {
        //Found what we are looking for
        WFDMMLOGH1("Buffer Found %lu", (long)pCmpValue);
        return QMM_LIST_ERROR_PRESENT;
    }

    return QMM_LIST_ERROR_NOT_PRESENT;
}


/*==============================================================================

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
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::deinitialize()
{

    if(!ISSTATECLOSED || !ISSTATEINIT)
    {
        SETSTATECLOSING;
    }

    WFDMMLOGH("Wait for all buffers to be returned");

    int nPendingQLen;
    int nRenderQLen;
    int nAudioQLen;
    while (1)
    {
        nPendingQLen = AccessPendingQ(NULL, size);
        nRenderQLen  = AccessRenderQ(NULL, (uint32)0, size);
        nAudioQLen   = mAudioQ ? mAudioQ->GetSize() : 0;

        WFDMMLOGH1("QueueLength at exit Audio- %d", nAudioQLen);
        WFDMMLOGH1("QueueLength at exit VideoRender- %d", nRenderQLen);
        WFDMMLOGH1("QueueLength at exit VIdeoPending- %d", nPendingQLen);

        if((!nPendingQLen || nPendingQLen == -1) &&
           (!nRenderQLen  || nRenderQLen  == -1) &&
           (!nAudioQLen))
        {
            WFDMMLOGH("All Buffers returned or freed");
            break;
        }
        MM_Timer_Sleep(5);
    }

    WFDMMLOGH("Done waiting for all buffers to be returned");

    if(ISSTATECLOSING)
    {
        SETSTATECLOSED;
    }
#ifndef USE_OMX_AAC_CODEC
    if(mpAudioDecoder)
    {
        MM_Delete(mpAudioDecoder);
        mpAudioDecoder = NULL;
    }
#endif

    /*--------------------------------------------------------------------------
      Close Threads and signal queues
    ----------------------------------------------------------------------------
    */
    if(mpAudioThread)
    {
        MM_Delete(mpAudioThread);
        mpAudioThread = NULL;
    }

    if(mpVideoThread)
    {
        MM_Delete(mpVideoThread);
        mpVideoThread = NULL;
    }

    if (mpGraphicPollThread)
    {
        MM_Delete(mpGraphicPollThread);
        mpGraphicPollThread = NULL;
    }

    if(mpAudioTrack != NULL)
    {
        mpAudioTrack->stop();
    }

    if(mAudioQ)
    {
        MM_Delete(mAudioQ);
        mAudioQ = NULL;
    }

    if(mFrameInfoFreeQ)
    {
        MM_Delete(mFrameInfoFreeQ);
        mFrameInfoFreeQ = NULL;
    }

    if (mpFpAAC)
    {
        fclose(mpFpAAC);
        mpFpAAC = NULL;
    }

    if(mpFpWAV)
    {
        fclose(mpFpWAV);
        mpFpWAV = NULL;
    }

    qmm_ListDeInit(&mVideoRenderQ);
    qmm_ListDeInit(&mVideoPendingQ);


    return OMX_ErrorNone;

}


/*==============================================================================

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
*//*==========================================================================*/
void WFDMMSinkRenderer::VideoThreadEntry(void *pThis,
                                         unsigned int nSignal)
{
    CHECK_ARGS_RET_VOID_1(pThis);

    WFDMMSinkRenderer *pMe = (WFDMMSinkRenderer*)pThis;

    if(pMe)
    {
        pMe->VideoThread(nSignal);
    }
}


/*==============================================================================

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
*//*==========================================================================*/
void WFDMMSinkRenderer::VideoThread(unsigned int nSignal)
{
    (void) nSignal;
    WFDMMLOGH("WFDMMSinkRenderer::VideoThread");
    while(ISSTATEOPENED || ISSTATECLOSING)
    {
        if(mbPaused && !ISSTATECLOSING)
        {
            /*------------------------------------------------------------------
             Rendering has been paused. Hold on to the buffers and continue.
            --------------------------------------------------------------------
            */
            MM_Timer_Sleep(10);
            continue;
        }
        frameInfoType *pFrameInfo = NULL;
        uint32 pendingQsize = AccessPendingQ(NULL, size);
        WFDMMSinkStatistics* pStatInst =
                            WFDMMSinkStatistics::CreateInstance();

        if(pendingQsize)
        {
            if(AccessPendingQ(&pFrameInfo, peekfront) == QMM_LIST_ERROR_NONE &&
               pFrameInfo != NULL)
            {
                if(mCfg.bHasAudio)
                {
                    AddAudioTrackLatency();
                }

                uint64 timedecider = pFrameInfo->pBuffer->nTimeStamp -
                                   getCurrentTimeUs() + mCfg.nDecoderLatency +
                                   mnAddAudioTrackLatency;

                if(((int64)timedecider <= (int64)mCfg.nvideoAVSyncDropWindow &&
                  mnVideoDropCount < mnVideoMaxConsecDrop &&
                  mCfg.bAVSyncMode) ||
                  ISSTATECLOSING    ||
                 (mbFlushInProgress
                && (uint64)pFrameInfo->pBuffer->nTimeStamp < mnFlushTimeStamp))
                {
                    /*----------------------------------------------------------
                      In this case the frame is late. Therefor we need to drop
                      it if it exceeds the time window and take the statistics.
                    ------------------------------------------------------------
                    */
                    mnVideoDropCount++;

                    if(pStatInst && !ISSTATECLOSING && !mbFlushInProgress)
                    {
                        rendererStatsType nRendStatObj;

                        nRendStatObj.nIsVideoFrame = 1;
                        nRendStatObj.nIsLate = 1;
                        nRendStatObj.nTimeDecider = (int64)timedecider;
                        nRendStatObj.nSessionTime =
                            getCurrentTimeUs() - mnBaseMediaTime;
                        nRendStatObj.nArrivalTime = pFrameInfo->nTSArrival;

                        pStatInst->UpdateDropsOrLateby(nRendStatObj);
                     }

                    WFDMMLOGE4(
                       "Renderer drop Videoframe TS = %llu, lateby = %lld flushing  %d %lld",
                       pFrameInfo->pBuffer->nTimeStamp,
                       (int64)timedecider, mbFlushInProgress,
                               mnFlushTimeStamp);

                    AccessPendingQ(&pFrameInfo, popfront);

                    if(pFrameInfo)
                    {
                        pFrameInfo->pBuffer->nFilledLen = 0;
                        mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                            pFrameInfo->pBuffer);
                        accessFrameInfoMeta(&pFrameInfo, pushfront);
                    }
                    if(AccessPendingQ(NULL, size) == 0)
                    {
                        MM_Timer_Sleep(2);
                    }
                }
                else if((int64)timedecider <= (int64)VIDEO_RENDER_TOLERANCE ||
                  !mCfg.bAVSyncMode)
                {
                    if(mbFlushInProgress &&
                       ((uint64)pFrameInfo->pBuffer->nTimeStamp
                                                           >= mnFlushTimeStamp))
                    {
                        WFDMMLOGH("Renderer Flush complete");
                        mbFlushInProgress = false;
                    }
                    WFDMMLOGM("PushVideoFrame to render");
                    mnVideoDropCount = 0;
                    /*----------------------------------------------------------
                      Update the frame lateby statistics here
                    ------------------------------------------------------------
                    */

                    if(pStatInst)
                    {
                        rendererStatsType nRendStatObj;

                        nRendStatObj.nIsVideoFrame = 1;
                        nRendStatObj.nIsLate = 0;
                        nRendStatObj.nTimeDecider = (int64)timedecider;
                        nRendStatObj.nSessionTime =
                            getCurrentTimeUs() - mnBaseMediaTime;
                        nRendStatObj.nArrivalTime = pFrameInfo->nTSArrival;

                        pStatInst->UpdateDropsOrLateby(nRendStatObj);
                    }

                    WFDMMLOGM("PushVideoFrame to render");
                    (void)AccessPendingQ(&pFrameInfo, popfront);
                    if(!PushVideo4Render(pFrameInfo))
                    {
                        WFDMMLOGE("Failed to render a video frame");
                    }
                    if(AccessPendingQ(NULL, size) == 0)
                    {
                        MM_Timer_Sleep(2);
                    }
                }
                else
                {
                    WFDMMLOGH("Push back video frame not time yet");
                    MM_Timer_Sleep(2);
                }
            }
            else
            {
                MM_Timer_Sleep(2);
            }
        }
        else
        {
            MM_Timer_Sleep(2);
        }
    }
    WFDMMLOGD("Renderer Video Thread Ended");
}

/*==============================================================================

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
*//*==========================================================================*/
void WFDMMSinkRenderer::AudioThreadEntry(void* pThis,
                                         unsigned int nSignal)
{
    CHECK_ARGS_RET_VOID_1(pThis);

    WFDMMSinkRenderer *pMe = (WFDMMSinkRenderer*)pThis;

    if(pMe)
    {
        pMe->AudioThread(nSignal);
    }
}

/*==============================================================================

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
*//*==========================================================================*/
void WFDMMSinkRenderer::AudioThread(unsigned int nSignal)
{
    (void) nSignal;
    WFDMMLOGH("WFDMMSinkRenderer AudioThread");
    int nDropCount = 0;
    uint64 timedecider;

    if(!mCfg.bHasVideo)
    {
        WFDMMLOGD("No Video Relax AV Sync window");
        mCfg.naudioAVSyncDropWindow = AUDIO_AVSYNC_DROP_WINDOW_AUDIO_ONLY;

    }

    while(ISSTATEOPENED || ISSTATECLOSING)
    {
        if(mbPaused && !ISSTATECLOSING)
        {
            /*------------------------------------------------------------------
             Rendering has been paused. Hold on to the buffers and continue.
            --------------------------------------------------------------------
            */
            MM_Timer_Sleep(10);
            continue;
        }
        frameInfoType *pFrameInfo = NULL;
        mAudioQ->Peek(&pFrameInfo,
                     sizeof(&pFrameInfo));

        WFDMMSinkStatistics* pStatInst =
                                 WFDMMSinkStatistics::CreateInstance();

        if(pFrameInfo)
        {
           WFDMMLOGD("Got a Pending Audio Frame");
           if(mCfg.nDecoderLatency <= mnAudioLatency)
           {
              timedecider = pFrameInfo->pBuffer->nTimeStamp -
                                     getCurrentTimeUs() +
                                     (mnAudioOffset != -1 ? mnAudioOffset : 0);
           }
           else
           {
              timedecider = pFrameInfo->pBuffer->nTimeStamp -
                                     getCurrentTimeUs() + mCfg.nDecoderLatency +
                                     (mnAudioOffset != -1 ? mnAudioOffset : 0) -
                                     mnAudioLatency;
            }
            bool bAVSyncDrop = false;

            /*With PCR change timestamp value will be incremental rather than getting reset
              to 0*/
            if(mnFirstAudioFrameTimeStamp == 0)
                mnFirstAudioFrameTimeStamp = pFrameInfo->pBuffer->nTimeStamp;

            int deltaAudioTimeStamp = pFrameInfo->pBuffer->nTimeStamp - mnFirstAudioFrameTimeStamp;
            WFDMMLOGE1("deltaAudioTimeStamp = %d",deltaAudioTimeStamp);
            if(deltaAudioTimeStamp >= 0 &&
               deltaAudioTimeStamp <= INITIAL_AUDIO_DROP_WINDOW)
            {
                bAVSyncDrop = true;
            }
            else if (nDropCount)
            {
                nDropCount--;
                bAVSyncDrop = true;
            }
            if((( (int64)timedecider <= (int64)mCfg.naudioAVSyncDropWindow)&&
                  mCfg.bAVSyncMode) || ISSTATECLOSING ||
                 (mbFlushInProgress
              && (uint64)pFrameInfo->pBuffer->nTimeStamp <= mnFlushTimeStamp)||
                 bAVSyncDrop)
            {
                /*-------------------------------------------------------------
                  In this case the frame is late. Therefor we need to drop
                  it if it exceeds the time window
                ----------------------------------------------------------------
                */

                if(!bAVSyncDrop && !mbFlushInProgress
                    && !ISSTATECLOSING)
                    nDropCount = MAX_CONSEQ_AUDIO_DROP;

                if(pStatInst)
                {
                    rendererStatsType nRendStatObj;

                    nRendStatObj.nIsVideoFrame = 0;
                    nRendStatObj.nIsLate = 1;
                    nRendStatObj.nTimeDecider = timedecider;
                    nRendStatObj.nSessionTime =
                        getCurrentTimeUs() - mnBaseMediaTime;
                    nRendStatObj.nArrivalTime = pFrameInfo->nTSArrival;

                    pStatInst->UpdateDropsOrLateby(nRendStatObj);
                }

                WFDMMLOGH2(
                   "Renderer drop Audioframe TS = %llu, lateby = %lld",
                   pFrameInfo->pBuffer->nTimeStamp,
                   (int64)timedecider);
                pFrameInfo->pBuffer->nFilledLen = 0;
                mAudioQ->Pop(&pFrameInfo,
                     sizeof(&pFrameInfo),
                     0);
                mpFnEBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID,
                        pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushfront);

                if(mAudioQ->GetSize() == 0)
                {
                    MM_Timer_Sleep(2);
                }
            }
            else if((int64)timedecider <= AUDIO_RENDER_TOLERANCE ||
              !mCfg.bAVSyncMode)
            {
                if(mbFlushInProgress &&
                   ((uint64)pFrameInfo->pBuffer->nTimeStamp >= mnFlushTimeStamp))
                {
                    WFDMMLOGH("Renderer Flush complete");
                    mbFlushInProgress = false;
                }
                if(pStatInst)
                {
                    rendererStatsType nRendStatObj;

                    nRendStatObj.nIsVideoFrame = 0;
                    nRendStatObj.nIsLate = 0;
                    nRendStatObj.nTimeDecider = timedecider;
                    nRendStatObj.nSessionTime =
                        getCurrentTimeUs() - mnBaseMediaTime;
                    nRendStatObj.nArrivalTime = pFrameInfo->nTSArrival;

                    pStatInst->UpdateDropsOrLateby(nRendStatObj);
                }

                mAudioQ->Pop(&pFrameInfo,
                     sizeof(&pFrameInfo),
                     0);
                if(mnAudioOffset == -1)
                {
                    mnAudioOffset = (getCurrentTimeUs()- pFrameInfo->pBuffer->nTimeStamp); //in us
                    if(mnAudioOffset < 0)
                    {
                        mnAudioOffset = 0;
                    }
                }
                if(!PushAudio4Render(pFrameInfo))
                {
                    WFDMMLOGE("Failed to render a Audio frame");
                }
                if(mAudioQ->GetSize() == 0)
                {
                    MM_Timer_Sleep(2);
                }
            }
            else
            {
                WFDMMLOGD("Pending AudioFrame not ready for render");
                MM_Timer_Sleep(2);
            }
        }
        else
        {
            MM_Timer_Sleep(2);
        }
    }
    WFDMMLOGD("Renderer Audio Thread ended");
}

/*==============================================================================

         FUNCTION:         GraphicThreadEntry

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
*//*==========================================================================*/
void WFDMMSinkRenderer::GraphicThreadEntry(void* pThis,
                                           unsigned int nSignal)
{
    CHECK_ARGS_RET_VOID_1(pThis);

    WFDMMSinkRenderer *pMe = (WFDMMSinkRenderer*)pThis;

    pMe->GraphicThread(nSignal);
}


/*==============================================================================

         FUNCTION:         GraphicThread

         DESCRIPTION:
*//**       @brief         Thread for getting frames back from native window
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
*//*==========================================================================*/
void WFDMMSinkRenderer::GraphicThread(unsigned int nSignal)
{
    (void)nSignal;
    ANativeWindowBuffer *pBuf;
    frameInfoType       *pFrameInfo;

    WFDMMLOGD("graphic Thread Started");

    while((ISSTATEOPENED || ISSTATECLOSING) && !ISERROR)
    {
        int nRet = 0;

        pFrameInfo = NULL;

        if(mnNumVidQedBufs - mnNumVidDQedBufs > 2)
        {
            nRet = native_window_dequeue_buffer_and_wait(mpWindow.get(), &pBuf);
            if (nRet != 0)
            {
                WFDMMLOGH("failed to dequeue Buffer");
                MM_Timer_Sleep(2);
                continue;
            }
            else
            {
                AccessRenderQ(&pFrameInfo, (uint64)pBuf->handle, search);

                if (pFrameInfo)
                {
                    mnNumVidDQedBufs++;
                    WFDMMLOGH("Found a match for dequeued buffer");

                    pFrameInfo->pBuffer->nFilledLen = 0;
                    mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                            pFrameInfo->pBuffer);
                    accessFrameInfoMeta(&pFrameInfo, pushrear);
                    pFrameInfo = NULL;

                }
                else
                {
                    WFDMMLOGE("Failed to find a buffer match after dequeue");
                }
            }
        }
        else
        {
            MM_Timer_Sleep(2);
        }
    }
    /*--------------------------------------------------------------------------
      If we encounter an error in dequeue we are not going to try again
      Lets flush all pending buffers in Render Q
    ----------------------------------------------------------------------------
    */
    if(ISERROR)
    {
        uint32 nSize = AccessRenderQ(NULL, 0, size);
        frameInfoType *pFrameInfo = NULL;

        for(uint32 i = 0; i < nSize; i++)
        {
            AccessRenderQ(&pFrameInfo, 0, popfront);
            if(pFrameInfo)
            {
                pFrameInfo->pBuffer->nFilledLen = 0;
                mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                        pFrameInfo->pBuffer);
                accessFrameInfoMeta(&pFrameInfo, pushrear);
                pFrameInfo = NULL;
            }
        }
    }
    WFDMMLOGD("graphic Thread Ended");
}

/*==============================================================================
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
*//*==========================================================================*/

int WFDMMSinkRenderer::state(int state, bool get_true_set_false)
{
    CRITICAL_SECT_ENTER;

    if(get_true_set_false == true)
    {
        /*----------------------------------------------------------------------
         Means get
        ------------------------------------------------------------------------
        */

    }
    else
    {
        meState = state;
    }

    CRITICAL_SECT_LEAVE;

    return meState;
}

/*==============================================================================
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
*//*==========================================================================*/
int WFDMMSinkRenderer::AccessPendingQ(frameInfoType **pPkt,
                                       actionType bAction)
{
    if(pPkt == NULL && bAction != size)
    {
        return -1;
    }

    if(!mCfg.bHasVideo)
    {
        if(pPkt)
        {
            *pPkt = NULL;
        }
        return 0;
    }

    CRITICAL_SECT_ENTER;
    QMM_ListErrorType eListError = QMM_LIST_ERROR_NONE;
    QMM_ListSizeType nQlen =0;

    eListError = qmm_ListSize(&mVideoPendingQ, &nQlen);

    if(bAction == size)
    {
        if(eListError == QMM_LIST_ERROR_NONE)
        {
            CRITICAL_SECT_LEAVE
            return nQlen;
        }
        else
        {
            CRITICAL_SECT_LEAVE
            return -1;
        }
    }

    if(bAction == pushrear || bAction == pushfront)
    {
        WFDMMLOGL("Push to pending");
        if(*pPkt)
        {

            if(bAction == pushrear)
            {
                eListError =
                     qmm_ListPushRear(&mVideoPendingQ, &((*pPkt)->pLink));
            }
            else
            {
                eListError =
                     qmm_ListPushFront(&mVideoPendingQ, &((*pPkt)->pLink));
            }

            if(eListError != QMM_LIST_ERROR_NONE)
            {
                WFDMMLOGE("Failed to enqueue");
            }
        }
    }
    else if(bAction == popfront || bAction == poprear || bAction == peekfront
            || bAction == peekrear)
    {
        WFDMMLOGL1("POP from pending %d", nQlen);

        if(!nQlen)
        {
            *pPkt = NULL;
            CRITICAL_SECT_LEAVE;
            return 0;
        }

        if(bAction == popfront)
        {
            eListError = qmm_ListPopFront(&mVideoPendingQ,
                                          (QMM_ListLinkType **)pPkt);
        }
        else if(bAction == peekfront)
        {
            eListError = qmm_ListPeekFront(&mVideoPendingQ,
                                          (QMM_ListLinkType **)pPkt);
        }
        else if(bAction == peekrear)
        {
            eListError = qmm_ListPeekRear(&mVideoPendingQ,
                                          (QMM_ListLinkType **)pPkt);
        }
        else
        {
            eListError = qmm_ListPopRear(&mVideoPendingQ,
                                          (QMM_ListLinkType **)pPkt);
        }

        if(eListError != QMM_LIST_ERROR_NONE ||  !(*pPkt))
        {
            WFDMMLOGE("Failed to Pop");
        }
    }
    CRITICAL_SECT_LEAVE;
    return 0;
}


/*==============================================================================
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
*//*==========================================================================*/
int WFDMMSinkRenderer::AccessRenderQ(frameInfoType **pPkt, uint64 handle,
                                       actionType bAction)
{
    if(pPkt == NULL && bAction != size)
    {
        return -1;
    }

    if(!mCfg.bHasVideo)
    {
        if(pPkt)
        {
            *pPkt = NULL;
        }
        return 0;
    }

    CRITICAL_SECT_ENTER;
    QMM_ListErrorType eListError = QMM_LIST_ERROR_NONE;
    QMM_ListSizeType nQlen =0;

    eListError = qmm_ListSize(&mVideoRenderQ, &nQlen);

    if(bAction == size)
    {
        if(eListError == QMM_LIST_ERROR_NONE)
        {
            CRITICAL_SECT_LEAVE
            return nQlen;
        }
        else
        {
            CRITICAL_SECT_LEAVE
            return -1;
        }
    }

    if(bAction == pushrear || bAction == pushfront)
    {
        WFDMMLOGL("Push to FreeQ");
        if(*pPkt)
        {
            if(bAction == pushrear)
            {
                eListError =
                     qmm_ListPushRear(&mVideoRenderQ, &((*pPkt)->pLink));
            }
            else
            {
                eListError =
                      qmm_ListPushFront(&mVideoRenderQ, &((*pPkt)->pLink));
            }

        }
    }
    else if(bAction == popfront || bAction == poprear)
    {
        WFDMMLOGL1("POP from pending %d", nQlen);

        if(!nQlen)
        {
            *pPkt = NULL;
            CRITICAL_SECT_LEAVE;
            return 0;
        }

        if(bAction == popfront)
        {
            eListError = qmm_ListPopFront(&mVideoRenderQ,
                                          (QMM_ListLinkType **)pPkt);
        }
        else
        {
            eListError = qmm_ListPopRear(&mVideoRenderQ,
                                          (QMM_ListLinkType **)pPkt);
        }

    }
    else if(bAction == search)
    {
        SearchBuffer(&mVideoRenderQ, pPkt,handle);
    }
    CRITICAL_SECT_LEAVE;
    return 0;
}

/*==============================================================================

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
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
uint64 WFDMMSinkRenderer::getCurrentTimeUs()
{
    struct timespec sTime;
    clock_gettime(CLOCK_MONOTONIC, &sTime);

    uint64 currTS = ((OMX_U64)sTime.tv_sec * 1000000/*us*/)
                          + ((OMX_U64)sTime.tv_nsec / 1000);
    return currTS - mnBaseMediaTime;

}

/*==============================================================================

         FUNCTION:

         DESCRIPTION:
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::Stop()
{

    SETSTATECLOSING;

    uint32 nAudPendingSize = mAudioQ ? mAudioQ->GetSize() : 0;
    uint32 nVidPendingSize = AccessPendingQ(NULL, size);
    uint32 nVidRenderSize = AccessRenderQ(NULL, (uint32)0, size);

    /*--------------------------------------------------------------------------
      Wait for all buffers to be returned
    ----------------------------------------------------------------------------
    */
    WFDMMLOGH("Waiting for Renderer buffers to be returned");

    unsigned long nStartMs = 0;
    unsigned long nCurrMs = 0;
    bool bIsStartTimeSet = false;

    #define DRAW_CYCLE_MS 17
    #define WAIT_MARGIN 3

    uint32 nWaitMs = (nVidRenderSize + nVidPendingSize + WAIT_MARGIN)
                                        * DRAW_CYCLE_MS;

    while(nAudPendingSize || nVidPendingSize ||
           nVidRenderSize > NUM_BUFS_HELD_IN_NATIVE_WINDOW)
    {
        MM_Timer_Sleep(2);
        nAudPendingSize = mAudioQ? mAudioQ->GetSize(): 0;
        nVidPendingSize = AccessPendingQ(NULL, size);
        nVidRenderSize = AccessRenderQ(NULL, (uint32)0, size);
        WFDMMLOGH3("Aud Pending = %d, Vid Pending = %d, Vid Render = %d",
                   nAudPendingSize, nVidPendingSize, nVidRenderSize);
        if(nVidPendingSize== 0 && nAudPendingSize == 0)
        {
            if(!bIsStartTimeSet)
            {
                MM_Time_GetTime(&nStartMs);
                bIsStartTimeSet = true;
            }
            MM_Time_GetTime(&nCurrMs);
            if((unsigned)(nCurrMs - nStartMs) > nWaitMs)
            {
                break;
            }
        }
    }

    frameInfoType *pPkt;

    /*--------------------------------------------------------------------------
     Remove all buffers that are stuck in Render Q
    ----------------------------------------------------------------------------
    */
    for(uint32 i = 0; i < nVidRenderSize; i++)
    {
        WFDMMLOGH("Free Buffer stuck in Render Q");
        AccessRenderQ(&pPkt, 0, popfront);
        if(pPkt)
        {
            if((nVidRenderSize - i) > NUM_BUFS_HELD_IN_NATIVE_WINDOW)
            {
                pPkt->pBuffer->nFilledLen = 0;
                mpFnEBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID,
                   pPkt->pBuffer);
            }
            accessFrameInfoMeta(&pPkt, pushrear);
        }
    }

    WFDMMLOGH("Done waiting for all Renderer Buffers to be returned");

    /*--------------------------------------------------------------------------
     Now we have returned all buffers. We can move to INIT.
    ----------------------------------------------------------------------------
    */
    SETSTATECLOSED;

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          Start

         DESCRIPTION:
*//**       @brief          Make Renderer ready for buffer flow
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkRenderer::Start()
{
    if(!ISSTATEINIT)
    {
        WFDMMLOGE("Renderer not in Init when started");
        return OMX_ErrorInvalidState;
    }
    WFDMMLOGH("WFDMMSinkRenderer Start");

    mbFirstFrame = true;

    SETSTATEOPENED;

    if(mpAudioThread)
    {
        mpAudioThread->SetSignal(0);
    }
    if(mpVideoThread)
    {
        mpVideoThread->SetSignal(0);
    }
    if(mpGraphicPollThread)
    {
        mpGraphicPollThread->SetSignal(0);
    }


    return OMX_ErrorNone;
}
/*==============================================================================

         FUNCTION:

         DESCRIPTION:
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkRenderer::SetMediaBaseTime(uint64 nTime)
{
    WFDMMLOGH("Renderer Setting Media Base Time");
    /*-------------------------------------------------------------------------
      Renderer shall not receive any new frames when Media Base Time is being
      set. Renderer will flush out its existing data and only then will honor
      the new base time
    ---------------------------------------------------------------------------
    */
    mbAudioTimeReset = true;
    mbVideoTimeReset = true;
    mnFlushTimeStamp = (uint64)-1;
    mbFlushInProgress = true;

    unsigned long nStartMs = 0;
    unsigned long nCurrMs = 0;
    MM_Time_GetTime(&nStartMs);

    while((mAudioQ && mAudioQ->GetSize()) || AccessPendingQ(NULL, size))
    {
        MM_Time_GetTime(&nCurrMs);
        if((nCurrMs - nStartMs) > 1000)//wait for a second)
        {
            break;
        }
        MM_Timer_Sleep(5);
    }

    mnFlushTimeStamp = 0;
    mbFlushInProgress = false;

    mbMediaTimeSet = true;
    mnBaseMediaTime = nTime;
}

/*==============================================================================

         FUNCTION:          GenerateAACHeaderFromADTS

         DESCRIPTION:
*//**       @brief          Generate ADIF AAC header from ADTS header
                            information
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
*//*==========================================================================*/
bool WFDMMSinkRenderer::GenerateAACHeaderFromADTS(uint8* pAdts, uint32 nAdtsLen,
                                          uint8 *pAacHeader, uint32 *nAACHdrLen)
{
    if(!pAdts || nAdtsLen < 7 || !pAacHeader || (*nAACHdrLen) < 2)
    {
        WFDMMLOGE("GenerateAACHeader Invalid Args");
        return false;
    }

    uint8 nAudioObjectType = ((pAdts [2] >> 6) & 0x03) + 1;
    uint8 nSampFreqIndex   = ((pAdts [2] >> 2) & 0x0F);
    uint8 nChannelConfig   = ((pAdts [2] << 2) & 0x04)
                           | ((pAdts [3] >> 6) & 0x03);


    pAacHeader [0]
            = (OMX_U8)((nAudioObjectType << 3)
               | ((nSampFreqIndex & 0x0E) >> 1));
    pAacHeader [1]
            = (OMX_U8)(((nSampFreqIndex & 0x01) << 7)
               | (nChannelConfig << 3));

    *nAACHdrLen = 2;

    return true;
}

/*==============================================================================

         FUNCTION:          AudioTrackCb

         DESCRIPTION:
*//**       @brief          Any event for Audio track is handled here
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
*//*==========================================================================*/
void  WFDMMSinkRenderer::AudioTrackCb(int  nEvent,
                                      void *pCookie,
                                      void *pInfo)
{
    (void) pCookie;
    (void) pInfo;
    WFDMMLOGD1("AudioTrack CallBack event = %d", nEvent);

    WFDMMSinkRenderer  *pMe = (WFDMMSinkRenderer*)pCookie;

    if(pMe)
    {
        pMe->ProcessAudioTrackCb(nEvent,pInfo);
    }

    return;
}

/*==============================================================================

         FUNCTION:          ProcessAudioTrackCb

         DESCRIPTION:
*//**       @brief          Any event for Audio track is processed here
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
*//*==========================================================================*/
void  WFDMMSinkRenderer::ProcessAudioTrackCb(int  nEvent,
                                      void *pInfo)
{
    UNUSED(pInfo);
    WFDMMLOGD1("Process AudioTrack CallBack event = %d", nEvent);

    if(nEvent == android::AudioTrack::EVENT_NEW_IAUDIOTRACK)
    {
        mbRestartAudioTrack = true;
    }

    return;
}

/*==============================================================================

         FUNCTION:          anyChangeInCrop

         DESCRIPTION:
*//**       @brief          Looks if any picture crop settings changed. Usually
                            happens when video dimension changes
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
*//*==========================================================================*/
bool WFDMMSinkRenderer::anyChangeInCrop(pictureInfoType *pPictInfo)
{
    if(pPictInfo)
    {
        if(mPictInfo.rect.nHeight != pPictInfo->rect.nHeight ||
           mPictInfo.rect.nLeft   != pPictInfo->rect.nLeft   ||
           mPictInfo.rect.nTop    != pPictInfo->rect.nTop    ||
           mPictInfo.rect.nWidth  != pPictInfo->rect.nWidth  ||
           mPictInfo.nHeight      != pPictInfo->nHeight      ||
           mPictInfo.nWidth       != pPictInfo->nWidth)
        {
            WFDMMLOGH("Crop Info Changed");
            return true;
        }
    }
    return false;
}


/*==============================================================================

         FUNCTION:          prepareDumpFiles

         DESCRIPTION:
*//**       @brief          prepare the Dump files
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
*//*==========================================================================*/
void WFDMMSinkRenderer::prepareDumpFiles()
{
    int nRet = 0;
    char szTemp[PROPERTY_VALUE_MAX] = {(char)0};
#ifdef WFD_DUMP_PCM
    nRet = property_get("persist.debug.wfd.dumpwav",szTemp,NULL);
    if (nRet <= 0 )
    {
        WFDMMLOGE2("Failed to read prop %d %s value PCM", nRet, szTemp);
    }

    if(strcmp(szTemp,"1")==0)
    {
        WFDMMLOGH("Renderer Dump enabled for PCM");
        mbDumpWav = true;
    }


    if(mbDumpWav)
    {
        mpFpWAV= fopen("/data/media/wfdsinkaudiodump.wav","wb");
        if(!mpFpWAV)
        {
            WFDMMLOGE("Renderer::audiodump fopen failed");
            mbDumpWav = OMX_FALSE;
        }
        else
        {
           #define ID_RIFF 0x46464952
           #define ID_WAVE 0x45564157
           #define ID_FMT  0x20746d66
           #define ID_DATA 0x61746164
            struct WavHeader {
                uint32_t riff_id;
                uint32_t riff_sz;
                uint32_t riff_fmt;
                uint32_t fmt_id;
                uint32_t fmt_sz;
                uint16_t audio_format;
                uint16_t num_channels;
                uint32_t sample_rate;
                uint32_t byte_rate;
                uint16_t block_align;
                uint16_t bits_per_sample;
                uint32_t data_id;
                uint32_t data_sz;
            }sHdr;

            sHdr.riff_id = ID_RIFF;
            sHdr.riff_fmt = ID_WAVE;
            sHdr.fmt_id = ID_FMT;
            sHdr.fmt_sz = 16;
            sHdr.audio_format = 1;
            sHdr.num_channels = mCfg.sAudioInfo.nChannels;
            sHdr.sample_rate = 48000;
            sHdr.bits_per_sample = 16;
            sHdr.byte_rate = (48000 * mCfg.sAudioInfo.nChannels
                                      * mCfg.sAudioInfo.nSampleRate) / 8;
            sHdr.block_align = (sHdr.bits_per_sample *
                                 mCfg.sAudioInfo.nSampleRate) / 8;
            sHdr.data_id = ID_DATA;
            sHdr.data_sz = 2147483648LL;
            sHdr.riff_sz = sHdr.data_sz + 44 - 8;
            fwrite(&sHdr,1, sizeof(sHdr),mpFpWAV);
            WFDMMLOGE("Renderer::Writing  wav header");
        }
    }
#endif
#ifdef WFD_DUMP_AAC
    memset(szTemp, 0, sizeof(szTemp));
    nRet = property_get("persist.debug.wfd.dumpaac",szTemp,NULL);
    if (nRet <= 0 )
    {
        WFDMMLOGE2("Failed to read prop %d %s value AAC", nRet, szTemp);
    }

    if(strcmp(szTemp,"1")==0)
    {
        WFDMMLOGH("Renderer Dump Enabled for AAC");
        mbDumpAAC = true;
    }

    if(mbDumpAAC)
    {
        mpFpAAC = fopen("/data/media/wfdsinkaacdump.aac","wb");
        if(!mpFpAAC)
        {
            WFDMMLOGE("Renderer:aacdump fopen failed");
            mbDumpAAC = OMX_FALSE;
        }
    }
#endif
}

/*==============================================================================

         FUNCTION:          pauseRendering

         DESCRIPTION:
*//**       @brief          pauses rendering and holds on to the buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkRenderer::pauseRendering(void)
{
    mbPaused = true;
}

/*==============================================================================

         FUNCTION:          pauseRendering

         DESCRIPTION:
*//**       @brief          pauses rendering and holds on to the buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkRenderer::restartRendering(bool bFlush)
{
    if(mbPaused && bFlush)
    {
        mnFlushTimeStamp = (uint64)-1;
        mbFlushInProgress = true;

        unsigned long nStartMs = 0;
        unsigned long nCurrMs = 0;
        MM_Time_GetTime(&nStartMs);

        while((mAudioQ && mAudioQ->GetSize()) || AccessPendingQ(NULL, size))
        {
            MM_Time_GetTime(&nCurrMs);
            if((nCurrMs - nStartMs) > 1000)//wait for a second)
            {
                break;
            }
            MM_Timer_Sleep(5);
        }

        mnFlushTimeStamp = 0;
        mbFlushInProgress = false;

    }
    mbPaused = false;
}

/*==============================================================================

         FUNCTION:          setFlushTimeStamp

         DESCRIPTION:
*//**       @brief          sets a timestamp upto which data has to be flushed
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkRenderer::setFlushTimeStamp(uint64 nTS)
{
    mnFlushTimeStamp = nTS;
    mbFlushInProgress = true;
}

/*==============================================================================

         FUNCTION:          setDecoderLatency

         DESCRIPTION:
*//**       @brief          This simulates decoder latency by delaying the
                            rendering of frames to control smoothness of
                            playback
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
*//*==========================================================================*/
void WFDMMSinkRenderer::setDecoderLatency(uint64 nLatencyInms)
{
    WFDMMLOGH("Renderer Setting decode latency");
    CRITICAL_SECT_ENTER;
    mCfg.nDecoderLatency = nLatencyInms;
    CRITICAL_SECT_LEAVE;
}

/*==============================================================================

         FUNCTION:          AddAudioTrackLatency

         DESCRIPTION:
*//**       @brief          add the audiotrack latency to video thhread
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
*//*==========================================================================*/
void WFDMMSinkRenderer::AddAudioTrackLatency()
{
    uint32 mnAudioLatencyNew;
    mnAudioLatencyCheckCount++;
    if(mCfg.bAudioTrackLatencyMode == 0)
    {
        if(mnAudioLatency > DEFAULT_AUDIO_TRACK_LATENCY)
        {
            /*------------------------------------------------------------------
              case when BT is connected and wfd started.
              ------------------------------------------------------------------
            */
             mnAddAudioTrackLatency = mnAudioLatency;
        }
        if(mnAudioLatencyCheckCount == MAX_CONSEQ_VIDEO_FRAMES_COUNT)
        {
             mnAudioLatencyNew = mpAudioTrack->latency() * 1000;
             if(mnAudioLatencyNew > DEFAULT_AUDIO_TRACK_LATENCY)
             {
                 /*-------------------------------------------------------------
                   BT is connected after the wfd started
                   -------------------------------------------------------------
                 */
                 mnAddAudioTrackLatency = mnAudioLatencyNew;
             }
             else
             {
                 /*-------------------------------------------------------------
                   Don't consider the audio track latency
                   -------------------------------------------------------------
                 */
                 mnAddAudioTrackLatency = 0;
             }
             mnAudioLatencyCheckCount = 0;
             mnAudioLatency = mnAudioLatencyNew;
        }
    }
    else if(mCfg.bAudioTrackLatencyMode == 1)
    {
        mnAddAudioTrackLatency = mnAudioLatency;
        /*----------------------------------------------------------------------
          If flag is set we add audio track latency to renderer and in case if
          BT is  connected ,we add the new latency to renderer to improve the AV
          sync
         -----------------------------------------------------------------------
        */
        if(mnAudioLatencyCheckCount == MAX_CONSEQ_VIDEO_FRAMES_COUNT)
        {
            mnAudioLatencyNew = mpAudioTrack->latency() * 1000;
            if(mnAudioLatencyNew > DEFAULT_AUDIO_TRACK_LATENCY)
            {
                mnAddAudioTrackLatency = mnAudioLatencyNew;
            }
            else
            {
                mnAddAudioTrackLatency = mnAudioLatency;
            }
            mnAudioLatencyCheckCount = 0;
            mnAudioLatency = mnAudioLatencyNew;
         }
    }
}
