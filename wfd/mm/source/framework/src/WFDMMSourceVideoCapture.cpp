/*==============================================================================
*       WFDMMSourceVideoCapture.cpp
*
*  DESCRIPTION:
*       Captures frames and delivers to encoder component
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
12/27/2013                       InitialDraft
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
#include "MMTimer.h"
#include "common_log.h"
#include "MMMemory.h"
#include "WFDMMLogs.h"
#include "WFDMMThreads.h"
#include <cstdlib>
#include <errno.h>
#include <threads.h>
#include "WFDMMSourceVideoCapture.h"
#include "WFDMMSourceMutex.h"
#include "WFDMMSourceSignalQueue.h"
#include <media/stagefright/MetaData.h>
#include "WFDUtils.h"
#include "display_config.h"
#include "gralloc_priv.h"
#include <cutils/properties.h>

using namespace android;

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WFDMMSRCVCAP"
#endif

#define WFD_MM_SOURCE_THREAD_STACK_SIZE 16384

#define VIDEO_CAPTURE_START_EVENT  0
#define VIDEO_CAPTURE_STOP_EVENT   1
#define VIDEO_CAPTURE_MAX_EVENT    2

#define VC_DEFAULT_FRAMERATE       30

#define VC_INVALID_OP              0
#define VC_READ_FRAME_REGENERATED  1
#define VC_READ_NEW_FRAME          2
#define VC_READ_NO_FRAME_UPDATE    3
#define VC_WRITE_SUCCESS           4
#define VC_MIN_RETRANSMIT_COUNT    15

enum debugVar
{
    SMS_NUM_BUFF,
};

enum key {
    FRAME_NO_KEY = 'frNo',
    FRAME_READ_TIME_KEY = 'rdTm',
};


/*------------------------------------------------------------------------------
 State definitions
--------------------------------------------------------------------------------
*/
#define DEINIT  0
#define INIT    1
#define OPENED  2
#define CLOSING 4
#define CLOSED  3


/*------------------------------------------------------------------------------
 Macros to Update states
--------------------------------------------------------------------------------
*/
#define ISSTATEDEINIT  (state(0, true) == DEINIT)
#define ISSTATEINIT    (state(0, true) == INIT)
#define ISSTATEOPENED  (state(0, true) == OPENED)
#define ISSTATECLOSED  (state(0, true) == CLOSED)
#define ISSTATECLOSING (state(0, true) == CLOSING)

#define SETSTATECLOSING (state(CLOSING, false))
#define SETSTATEINIT    (state(INIT   , false))
#define SETSTATEDEINIT  (state(DEINIT , false))
#define SETSTATEOPENED  (state(OPENED , false))
#define SETSTATECLOSED  (state(CLOSED , false))

/*------------------------------------------------------------------------------
 Critical Sections
--------------------------------------------------------------------------------
*/
#define WFD_VCAP_CRITICAL_SECT_ENTER(critSect) if(critSect)                    \
                                      MM_CriticalSection_Enter(critSect);
    /*      END CRITICAL_SECT_ENTER    */

#define WFD_VCAP_CRITICAL_SECT_LEAVE(critSect) if(critSect)                    \
                                      MM_CriticalSection_Leave(critSect);
    /*      END CRITICAL_SECT_LEAVE    */

/*------------------------------------------------------------------------------
//This macro provides a single point exit from function on
//encountering any error
--------------------------------------------------------------------------------
*/
#define WFD_OMX_ERROR_CHECK(result,error) ({                                   \
        if(result!= OMX_ErrorNone)                                             \
        {                                                                      \
            WFDMMLOGE(error);                                                  \
            goto EXIT;                                                         \
        }                                                                      \
    })

/*=============================================================================

         FUNCTION:          fillStatInfo

         DESCRIPTION:
*//**       @brief        Fill in profiling data to the OMX_BUFFERHEADER we
                          are delivering to encoder
*//**
@par     DEPENDENCIES:
                          None
*//*
         PARAMETERS:
*//**       @param[in]    pBufferHdr   The buffer header to be sent to encoder

*           @param[in]    nTime        The time of delivering frame to encoder

*           @param[in]    pMediaBuffer The MediaBuffer that has frame info

*//*     RETURN VALUE:
*//**       @return
                          result OMX_ErrorNone if no error, else appropriate error

@par     SIFE EFFECTS:
                          None
*//*=========================================================================*/

static OMX_ERRORTYPE inline fillStatInfo(
    OMX_BUFFERHEADERTYPE *pBufferHdr,
    OMX_TICKS nTime,
    MediaBuffer* pMediaBuffer = NULL)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if(pBufferHdr)
    {
        buff_hdr_extra_info* tempExtra = static_cast<buff_hdr_extra_info*>
                            (pBufferHdr->pPlatformPrivate);
        if(tempExtra)
        {
            tempExtra->nEncDelTime = nTime;
            if(pMediaBuffer && pMediaBuffer->meta_data()!= NULL)
            {
                int64_t frameNo = -1;

                if(pMediaBuffer->meta_data()->findInt64(FRAME_NO_KEY,&frameNo))
                {
                    WFDMMLOGE1("Sending frame number %lld",frameNo);
                    tempExtra->nFrameNo = frameNo;
                }
                else
                {
                    WFDMMLOGE("Unable to query frameNo in MediaBuffer metadata!");
                }
            }
        }
    }

    return result;
}

/*=============================================================================

         FUNCTION:          WFDMMSourceVideoCapture

         DESCRIPTION:
*//**       @brief          CTOR for WFDMMSourceVideoCapture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]    pEnc   Handle to encoder component

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
WFDMMSourceVideoCapture::WFDMMSourceVideoCapture()
{
    WFDMMLOGE("WFDMMSourceVideoCapture ctor");
    initData();
}

/*=============================================================================

         FUNCTION:          ~WFDMMSourceVideoCapture

         DESCRIPTION:
*//**       @brief          DTOR for WFDMMSourceVideoCapture
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
*//*=========================================================================*/
WFDMMSourceVideoCapture::~WFDMMSourceVideoCapture()
{
    WFDMMLOGH("WFDMMSourceVideoCapture dtor");
    releaseResources();
    WFDMMLOGH("Done with ~WFDMMSourceVideoCapture");
}

/*=============================================================================

         FUNCTION:          initData

         DESCRIPTION:
*//**       @brief          initializes data members of WFDMMSourceVideoCapture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::initData()
{
    OMX_ERRORTYPE result       = OMX_ErrorNone;
    m_hVideoCaptureTimer       = NULL;
    m_hVideoCaptureThread      = NULL;
    m_pLastXmitBuffer          = NULL;
    m_nReadIdx                 = VIDEO_CAPTURE_NUM_BUFF -1;
    m_nWriteIdx                = 0;
    m_hQCritSect               = NULL;
    m_hStateCritSect           = NULL;
    m_nModuleId                = 0;
    m_pAppData                 = NULL;
    m_nTimerCtr                = 0;
    m_bFirstFrameFetched       = OMX_FALSE;
    m_bMissedFrame             = OMX_FALSE;
    m_bPaused                  = OMX_FALSE;
    m_nPrevBufferTS            = 0;
    memset(&m_sConfig, 0, sizeof(m_sConfig));
    m_pLastXmitBuffer          = NULL;
    m_pVideoInputQ             = NULL;
    m_pFnData                  = NULL;
    m_nMinRetransmitCnt        = VC_MIN_RETRANSMIT_COUNT;
    m_pFnEvent                 = NULL;
    memset(&mWfdVcapStats, 0, sizeof(mWfdVcapStats));

    for(int i = 0; i < VIDEO_CAPTURE_NUM_BUFF ; i++)
    {
        m_sLatestFrameQ[i].pBuffer = NULL;
        m_sLatestFrameQ[i].isRead  = false;
        m_sLatestFrameQ[i].isValid = false;
    }

    SETSTATEDEINIT;

    return result;
}


/*==============================================================================

         FUNCTION:          configure

         DESCRIPTION:
*//**       @brief          Configures capture copmponent
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pConfig      The input configuration
                                         for WFD session

            @param[in]      pFnCallback  The callback for reporting
                                         events to controller module

            @param[in]      nModuleId    Module Id assigned by client

            @param[in]      pAppData     The appdata for use in callbacks

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::configure
(
    VideoEncStaticConfigType*        pConfig,
    VidCaptureEventCallbackFnType    pFnEvt,
    VidCaptureDataCallbackFnType     pFnData,
    OMX_U32                          nModuleId,
    OMX_PTR                          pAppData
)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(!pConfig || !pFnEvt || !pAppData)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,
        "WFDMMSourceVideoCapture::Invalid data to configure capture module");
    }

    memcpy(&m_sConfig, pConfig, sizeof(m_sConfig));

    if(m_sConfig.nFramerate == 0)
    {
        m_sConfig.nFramerate = VC_DEFAULT_FRAMERATE;
    }

    m_pFnEvent         = pFnEvt;
    m_pFnData          = pFnData;
    m_nModuleId        = nModuleId;
    m_pAppData         = pAppData;
    result             = createResources();

    WFD_OMX_ERROR_CHECK(result,
                        "WFDMMSourceVideoCapture::createResources failed");

    SETSTATEINIT;
EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            createResources

         DESCRIPTION:
*//**       @brief           create threads & queues for WFDMMSourceVideoCapture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::createResources()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int nRet = 0;


    /*--------------------------------------------------------------------------
     Create a thread to capture frames from SurfaceMediaSource
    ----------------------------------------------------------------------------
    */
    m_hVideoCaptureThread =
        MM_New_Args(WFDMMThreads, (VIDEO_CAPTURE_MAX_EVENT));


    if(!m_hVideoCaptureThread)
    {
        result = OMX_ErrorInsufficientResources;

        WFD_OMX_ERROR_CHECK(result,
              "WFDMMSourceVideoCapture:: m_hVideoCaptureThread create failed");
    }


    m_hVideoCaptureThread->Start(captureThreadEntry,
                                  -2, 32768,this, "CaptureThread");

    if(MM_CriticalSection_Create(&m_hQCritSect))
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
              "WFDMMSourceVideoCapture:: crit sect creation failed");
    }

    if(MM_CriticalSection_Create(&m_hStateCritSect))
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
              "WFDMMSourceVideoCapture:: crit sect creation failed");
    }

    m_pVideoInputQ  = MM_New_Args(SignalQueue, (100, sizeof(int*)));

    if(!m_pVideoInputQ)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
              "WFDMMSourceVideoCapture:: failed to create Video Q");
    }

EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            releaseResources

         DESCRIPTION:
*//**       @brief           create threads & queues for WFDMMSourceVideoCapture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
void WFDMMSourceVideoCapture::releaseResources()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(m_hVideoCaptureTimer)
    {
        MM_Timer_Release(m_hVideoCaptureTimer);
        m_hVideoCaptureTimer = NULL;
    }

    releaseMediaBuffers();

    if(m_hVideoCaptureThread)
    {
        MM_Delete(m_hVideoCaptureThread);
        m_hVideoCaptureThread = NULL;
    }

    if(m_pVideoInputQ)
    {
        MM_Delete(m_pVideoInputQ);
        m_pVideoInputQ = NULL;
    }

    if(m_hQCritSect)
    {
        MM_CriticalSection_Release(m_hQCritSect);
        m_hQCritSect = NULL;
    }

    if(m_hStateCritSect)
    {
        MM_CriticalSection_Release(m_hStateCritSect);
        m_hStateCritSect = NULL;
    }

}

/*=============================================================================

         FUNCTION:           debug_param

         DESCRIPTION:
*//**       @brief           Helper to query a property
*//**
@par     DEPENDENCIES:
                             None
*//*
         PARAMETERS:
*//*         @param[in]      type type of debugVariable

*//*     RETURN VALUE:
*//**       @return
                             value of the property
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

static int debug_param(debugVar type)
{
    int ret = -1;
    char szTemp[PROPERTY_VALUE_MAX];
    switch(type)
    {
        case SMS_NUM_BUFF:
            ret = property_get("debug.wfd.smsnum",szTemp,NULL);
            if (ret <= 0 )
            {
                WFDMMLOGE2("Failed to read prop %d %s value", ret, szTemp);
                return -1;
            }
            break;
        default:
            WFDMMLOGE1("!!!Invalid type %d for debug_param!", type);
            return -1;
    }
    ret = atoi(szTemp);
    WFDMMLOGE1("debug_param returning %d",ret);
    return ret;
}

/*=============================================================================

         FUNCTION:            createSurface

         DESCRIPTION:
*//**       @brief           Create surface for using SurfaceMediaSource APIs
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in] width width of surface to be created

     *         @param[in] height height of surface to be created

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or None
@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::createSurface(int width, int height)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    m_pSurfaceMediaSrc   = MM_New_Args(SurfaceMediaSource,
                                       (width, height));
    m_pBuffProducer = ((m_pSurfaceMediaSrc->getProducer()));
    if(m_pBuffProducer == NULL)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,
                            "WFDMMSourceVideoCapture::Unable to get BufferQ");
    }
    int32_t consumerUsage;
EXIT:
    return result;
}

/*==============================================================================

         FUNCTION:            AccessLatestBufQ

         DESCRIPTION:
*//**       @brief           Helper method to update read/write index in FrameQ
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in] read If a frame has been read of the FrameQ or not

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
int WFDMMSourceVideoCapture::AccessLatestBufQ
(
    bool bRead,
    bool bForce,
    captureFrameInfoType* pBufferInfo
)
{
    WFD_VCAP_CRITICAL_SECT_ENTER(m_hQCritSect);

    int nRetVal = VC_INVALID_OP;

    if(bRead)
    {
        OMX_U64 nReadIdx = (m_nReadIdx + 1) % VIDEO_CAPTURE_NUM_BUFF;

        /*----------------------------------------------------------------------
         Check if the next buffer position has been updated with a new frame.
         Else keep sending the current frame
        ------------------------------------------------------------------------
        */
        if(m_sLatestFrameQ[nReadIdx].isValid)
        {
            if(!m_sLatestFrameQ[nReadIdx].isRead)
            {
                /*--------------------------------------------------------------
                 There is an unread frame available. Read that and update index
                ----------------------------------------------------------------
                */
                m_sLatestFrameQ[nReadIdx].isRead = true;

                if(pBufferInfo)
                {
                    memcpy(pBufferInfo, &m_sLatestFrameQ[nReadIdx],
                           sizeof(captureFrameInfoType));
                }
                m_nReadIdx = nReadIdx;
                nRetVal =  VC_READ_NEW_FRAME;
            }
            else
            {
                /*--------------------------------------------------------------
                 If there is no latest frame, simply return unless the caller
                 forces a buffer to be read. in that case regeneration will
                 happen.
                ----------------------------------------------------------------
                */
                if(bForce)
                {
                    WFDMMLOGH("Frame Regeneration");
                    if(pBufferInfo)
                    {
                        memcpy(pBufferInfo, &m_sLatestFrameQ[m_nReadIdx],
                               sizeof(captureFrameInfoType));
                    }
                    nRetVal = VC_READ_FRAME_REGENERATED;

                    ((MediaBuffer*)
                       (m_sLatestFrameQ[m_nReadIdx].pBuffer))->add_ref();
                    mWfdVcapStats.nAcquiredBuff++;
                }
                else
                {
                    nRetVal = VC_READ_NO_FRAME_UPDATE;
                }
            }
        }
    }
    else
    {
        if(m_sLatestFrameQ[m_nWriteIdx].isValid &&
           !m_sLatestFrameQ[m_nWriteIdx].isRead)
        {
            /*------------------------------------------------------------------
             Buffer is being overwritten without consumption. Release it
            --------------------------------------------------------------------
            */
            MediaBuffer *pMediaBuffer =
                          (MediaBuffer*)m_sLatestFrameQ[m_nWriteIdx].pBuffer;
            if(pMediaBuffer)
            {
                pMediaBuffer->release();
                mWfdVcapStats.nRelasedBuff++;
            }
            /*-----------------------------------------------------------------
             Latch on RdIdx to the current WriteIdx to avoid out of sequence
             frames
            -------------------------------------------------------------------
            */
            m_nReadIdx = m_nWriteIdx;
        }

        /*----------------------------------------------------------------------
         Update new MediaBuffer information in the queue
        ------------------------------------------------------------------------
        */

        if(pBufferInfo && pBufferInfo->pBuffer)
        {
            m_sLatestFrameQ[m_nWriteIdx].pBuffer = pBufferInfo->pBuffer;
            m_sLatestFrameQ[m_nWriteIdx].isRead  = false;
            m_sLatestFrameQ[m_nWriteIdx].isValid = true;

            MediaBuffer *pMediaBuffer =
                          (MediaBuffer*)(pBufferInfo->pBuffer);
            int64_t frameNo;
            if(pMediaBuffer->meta_data()->findInt64(FRAME_NO_KEY,&frameNo))
            {
                WFDMMLOGE3("Pushing frame number %lld rdIdx = %lld wtIdx = %lld",
                    frameNo, m_nReadIdx, m_nWriteIdx);
            }
            else
            {
                WFDMMLOGE("Unable to query frameNo in MediaBuffer metadata!");
            }
        }
        m_nWriteIdx= (m_nWriteIdx + 1) % VIDEO_CAPTURE_NUM_BUFF;

        nRetVal = VC_WRITE_SUCCESS;
    }

    WFD_VCAP_CRITICAL_SECT_LEAVE(m_hQCritSect);
    return nRetVal;
}

/*==============================================================================

         FUNCTION:            startCapture

         DESCRIPTION:
*//**       @brief           start capturing frames
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or none
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::startCapture()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int nRet = 0;
    sp<MetaData> params;
    int numSMSBuff = 5; //default

    WFDMMLOGH("StartCapture");

    /*--------------------------------------------------------------------------
    // Creating surface based on negotiated height and width which will be
    // retained throughout the session
    ----------------------------------------------------------------------------
    */
    result = createSurface((int)m_sConfig.nFrameWidth,(int)m_sConfig.nFrameHeight);

    WFD_OMX_ERROR_CHECK(result,
                        "WFDMMSourceVideoCapture::Failed to get surface");

    WFDMMLOGH("Surface Created");

    SETSTATEOPENED;

    params = new MetaData();

    if(params == NULL  || m_pSurfaceMediaSrc == NULL)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"SurfacemediaSource or MetaData not allocated!");
    }

    nRet = debug_param(SMS_NUM_BUFF);

    if(nRet > 0) // Don't allow less than 1
    {
        numSMSBuff = nRet;
    }

    params->setInt32(android::kKeyNumBuffers,numSMSBuff);
    WFDMMLOGE1("Starting SMS with %d number of Buffers", numSMSBuff);

    m_pSurfaceMediaSrc->start(params.get());

    /*--------------------------------------------------------------------------
     Create timer to send frames periodically
    ----------------------------------------------------------------------------
    */
    nRet = MM_Timer_CreateEx(1, inputDeliveryTimerCb, this, &m_hVideoCaptureTimer);

    if(nRet == 0 && m_hVideoCaptureTimer)
    {

        nRet = MM_Timer_StartEx( m_hVideoCaptureTimer, 0,
                        (int)((uint64)500000000/ m_sConfig.nFramerate));

        if(nRet != 0)
        {

            /*------------------------------------------------------------------
             Timer creation failed! Release timer
            --------------------------------------------------------------------
            */
            (void)MM_Timer_Release( m_hVideoCaptureTimer);
            m_hVideoCaptureTimer = NULL;
            result = OMX_ErrorInsufficientResources;
            WFD_OMX_ERROR_CHECK(result,
                  "WFDMMSourceVideoCapture:: failed to create Timer");
        }
    }


    m_hVideoCaptureThread->SetSignal(VIDEO_CAPTURE_START_EVENT);


EXIT:
    return result;
}

/*==============================================================================

         FUNCTION:            stopCapture

         DESCRIPTION:
*//**       @brief           stop capturing frames
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or none
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::stopCapture()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    m_bPaused = OMX_FALSE;

    SETSTATECLOSING;

    WFDMMLOGD("Waiting for StopCapture");
    while(m_pVideoInputQ && m_pVideoInputQ->GetSize())
    {
        /*----------------------------------------------------------------------
         Wait until all buffers are returned to
        ------------------------------------------------------------------------
        */
        MM_Timer_Sleep(2);
    }

    releaseMediaBuffers();

    WFDMMLOGD1("Diff b/w acq and released is :%d",
        mWfdVcapStats.nAcquiredBuff - mWfdVcapStats.nRelasedBuff);

    /*-------------------------------------------------------------------------
    Unblock SMS read, let it return EOS, we will exit the capture thread soon
    ---------------------------------------------------------------------------
    */

    if(m_pSurfaceMediaSrc.get() != NULL)
    {
        m_pSurfaceMediaSrc->stop();
    }

    if(m_hVideoCaptureThread)
    {
        m_hVideoCaptureThread->SetSignal(VIDEO_CAPTURE_STOP_EVENT);
    }


    if(m_hVideoCaptureTimer)
    {
        (void)MM_Timer_Release( m_hVideoCaptureTimer);
        m_hVideoCaptureTimer = NULL;
    }

    SETSTATECLOSED;

    return result;
}

/*==============================================================================

         FUNCTION:         pauseCapture

         DESCRIPTION:
*//**       @brief         pause capture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ERRORTYPE
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::pauseCapture()
{
    WFDMMLOGH("Calling pause on display");
    OMX_ERRORTYPE result = OMX_ErrorNone;

    m_bPaused = OMX_TRUE;

    SETSTATECLOSING;
    if(m_hVideoCaptureTimer)
    {
        MM_Timer_Stop(m_hVideoCaptureTimer);
    }
    qdutils::setSecondaryDisplayStatus(qdutils::DISPLAY_VIRTUAL, qdutils::EXTERNAL_PAUSE);

    WFDMMLOGD("Waiting for pauseCapture");
    while(m_pVideoInputQ && m_pVideoInputQ->GetSize())
    {
        /*----------------------------------------------------------------------
         Return all buffers to encode module
        ------------------------------------------------------------------------
        */
        OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
        WFDMMLOGD1("pauseCapture:: Queue Size is %ld",
                m_pVideoInputQ->GetSize());
        m_pVideoInputQ->Pop(&pBufferHdr, sizeof(&pBufferHdr), 0);
        if(pBufferHdr)
        {
            pBufferHdr->nFilledLen = 0;
            pBufferHdr->pOutputPortPrivate = NULL;
            m_pFnData(m_pAppData, pBufferHdr);
        }
    }

    releaseMediaBuffers();

    WFDMMLOGD1("pauseCapture:: Diff b/w acq and released is :%d",
        mWfdVcapStats.nAcquiredBuff - mWfdVcapStats.nRelasedBuff);

    SETSTATECLOSED;

    return result;
}

/*==============================================================================

         FUNCTION:            resumeCapture

         DESCRIPTION:
*//**       @brief           resume capture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ERRORTYPE
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::resumeCapture()
{
    WFDMMLOGH("Calling resume on display");

    SETSTATEOPENED;
    qdutils::setSecondaryDisplayStatus(qdutils::DISPLAY_VIRTUAL, qdutils::EXTERNAL_RESUME);

    /*----------------------------------------------------------------------
     Reinitialize all counters/varibles used by timer BEFORE resuming it
    ------------------------------------------------------------------------
    */

    m_nTimerCtr            = 0;
    m_bFirstFrameFetched   = OMX_FALSE;
    m_bMissedFrame         = OMX_FALSE;
    m_nReadIdx             = VIDEO_CAPTURE_NUM_BUFF -1;
    m_nWriteIdx            = 0;
    mWfdVcapStats.frameNo  = 0;

    if(m_hVideoCaptureTimer)
    {
        MM_Timer_StartEx( m_hVideoCaptureTimer, 0,
                        (int)((uint64)500000000/ m_sConfig.nFramerate));
    }

    m_bPaused = OMX_FALSE;

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:            CaptureThreadEntry

         DESCRIPTION:
*//**       @brief           entry point for capture thread
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSourceVideoCapture::captureThreadEntry
(
    void *pThreadData,
    unsigned int signal
)
{
    if (pThreadData)
    {
        WFDMMLOGE("WFDMMSourceVideoCapture::CaptureThreadEntry");

        ((WFDMMSourceVideoCapture*)pThreadData)->captureThread(signal);
    }
    else
    {
        WFDMMLOGE(
        "WFDMMSourceVideoCapture::Null data passed in CaptureThreadEntry");
    }
    return;
}

/*==============================================================================

         FUNCTION:            CaptureThread

         DESCRIPTION:
*//**       @brief           main body for Capture Thread execution
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone  or error
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::captureThread(int nSignal)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    size_t     readBytes = 0;
    int         duration = 0;
    int              ret = 0;
    OMX_TICKS beforeRead = 0;
    OMX_TICKS afterRead  = 0;
    OMX_TICKS diffRead   = 0;

    WFDMMLOGE("WFDMMSourceVideoCapture::CaptureThread");

    /*--------------------------------------------------------------------------
    //If ever rechristening of this thread is implemented bear in mind that
    //there is an upper limit to the name (as of now 16)
    ----------------------------------------------------------------------------
    */
    ret = pthread_setname_np(pthread_self(),"WFD_VCAP_Thread");
    if(ret)
    {
        WFDMMLOGE2("Failed to set thread name due to %d %s",
                    ret, strerror(errno));
        //No need to panic
    }

    if(nSignal == VIDEO_CAPTURE_START_EVENT)
    {
        WFDMMLOGE("VIDEO_CAPTURE_START");
        captureFrameInfoType sCaptureBuffer;
        MediaBuffer          *pMediaBuffer = NULL;

        sCaptureBuffer.pBuffer = (uint8*)pMediaBuffer;
        sCaptureBuffer.isRead  = false;
        sCaptureBuffer.isValid = true;

        while(1)
        {
            GetCurTime(beforeRead);
            int ret = m_pSurfaceMediaSrc->read(&pMediaBuffer);
            GetCurTime(afterRead);
            diffRead = afterRead - beforeRead;
            if(diffRead > mWfdVcapStats.maxReadTime)
            {
                mWfdVcapStats.maxReadTime = diffRead;
            }
            WFDMMLOGE3("m_pSurfaceMediaSrc->read returned %d %p in %lld",
                ret, pMediaBuffer, diffRead);
            if(pMediaBuffer && ret == 0)
            {
                if(m_bPaused || !ISSTATEOPENED)
                {
                    pMediaBuffer->release();
                }
                else
                {
                    WFDMMLOGM1("refcount = %d",pMediaBuffer->refcount());
                    WFDMMLOGM1("size = %d",pMediaBuffer->size());
                    WFDMMLOGM2("rangeOffset = %d range_length = %d",
                    pMediaBuffer->range_offset(),pMediaBuffer->range_length());
                    int64_t time;

                    /*----------------------------------------------------------
                    First read off all meta data that framework might have
                    populated, then populate it with all the meta data that we
                    require to suit our purpose
                    ------------------------------------------------------------
                    */

                    pMediaBuffer->meta_data()->findInt64(android::kKeyTime,
                                                                    &time);
                    mWfdVcapStats.frameNo++;
                    pMediaBuffer->meta_data()->setInt64(FRAME_NO_KEY,
                                                    mWfdVcapStats.frameNo);
                    pMediaBuffer->meta_data()->setInt64(FRAME_READ_TIME_KEY,
                                            static_cast<int64_t>(afterRead));
                    WFDMMLOGM3("time = %lld, frame number = %lld at %lld",
                              time, mWfdVcapStats.frameNo, afterRead);
                    sCaptureBuffer.pBuffer = (uint8*)pMediaBuffer;
                    mWfdVcapStats.nAcquiredBuff++;
                    ret = AccessLatestBufQ(false, false, &sCaptureBuffer);
                }
            }
            else
            {
                /*------------------------------------------------------------
                SurfaceMediaSource returning an error usually indicates that
                the producer has disconnected from its BufferQ (when surface
                has been removed), which will normally occur on teardown. SMS
                is returning EOS probably, so allow a small sleep for context
                switching
                ------------------------------------------------------------
                */
                MM_Timer_Sleep(5);
            }
            if(!ISSTATEOPENED && !m_bPaused)
            {
                break;//out of while loop for SMS-> read
            }
        }
    }
    else if (nSignal == VIDEO_CAPTURE_STOP_EVENT)
    {
        WFDMMLOGE("VIDEO_CAPTURE_STOP_EVENT");
        if(m_pSurfaceMediaSrc.get() != NULL)
        {
            m_pSurfaceMediaSrc->stop();
        }
        WFDMMLOGD("StopCapture Done");
    }

EXIT:
    return result;
}

/*==============================================================================

         FUNCTION:          SetFreeBuffer

         DESCRIPTION:
*//**       @brief          Provides a free o/p buffer so that it can be
                            queue back to OMX Decoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoCapture::SetFreeBuffer
(
    OMX_BUFFERHEADERTYPE *pBuffer,
    OMX_BOOL              bForce
)
{
    OMX_ERRORTYPE eErr = OMX_ErrorUndefined;

    if (!pBuffer)
    {
        WFDMMLOGE("Invalid Buffer Pointer in SetFreeBuffer");
        return OMX_ErrorBadParameter;
    }

    if(ISSTATEINIT || ISSTATEOPENED || bForce)
    {
        /*----------------------------------------------------------------------
          This module is the rightful owner of the buffer at death ot stop of
          the module. So hold the buffer in the collectorQ
        ------------------------------------------------------------------------
        */
        if(pBuffer->pOutputPortPrivate)
        {
            WFDMMLOGH2("SetFreeBuffer Release = %p with ref count %d",
                pBuffer->pOutputPortPrivate,
                ((MediaBuffer*)pBuffer->pOutputPortPrivate)->refcount());
            ((MediaBuffer*)pBuffer->pOutputPortPrivate)->release();
            mWfdVcapStats.nRelasedBuff++;
        }
        WFDMMLOGE("Push frame to Input Q");
        m_pVideoInputQ->Push(&pBuffer, sizeof(&pBuffer));
        return OMX_ErrorNone;
    }
    else
    {
        /*----------------------------------------------------------------------
         Buffer provided in an invalid state return to caller
        ------------------------------------------------------------------------
        */
        if(pBuffer->pOutputPortPrivate)
        {
            WFDMMLOGH2("SetFreeBuffer Release = %p with ref count %d",
                pBuffer->pOutputPortPrivate,
                ((MediaBuffer*)pBuffer->pOutputPortPrivate)->refcount());
            ((MediaBuffer*)pBuffer->pOutputPortPrivate)->release();
            mWfdVcapStats.nRelasedBuff++;
            pBuffer->pOutputPortPrivate = NULL;
        }
        return OMX_ErrorInvalidState;
    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          releaseMediaBuffers

         DESCRIPTION:
*//**       @brief          Helper method to release all instances of
                                    MediaBuffers held by this module outside
                                    the Video InputQ

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

void WFDMMSourceVideoCapture::releaseMediaBuffers()
{
    /*--------------------------------------------------------------------------
     If the most recent transmitted buffer is being held, release it
    ----------------------------------------------------------------------------
    */
    if(m_pLastXmitBuffer)
    {
      //  int refCount = m_pLastXmitBuffer->refcount();
      //  WFDMMLOGD1("m_pLastXmitBuffer has ref count %d", refCount);
      //  for(int i =1; i< refCount; i++)
      //  {
            //Call release on all refernces since at this point there is
            //probably no one to call release on these lingering references
            m_pLastXmitBuffer->release();
            mWfdVcapStats.nRelasedBuff++;
      //  }
        m_pLastXmitBuffer = NULL;
    }

    /*--------------------------------------------------------------------------
     Release any mediabuffer references held in any queue
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < VIDEO_CAPTURE_NUM_BUFF; i++)
    {
        if(m_sLatestFrameQ[i].isValid && !m_sLatestFrameQ[i].isRead)
        {
         //   int refCount = ((MediaBuffer*)m_sLatestFrameQ[i].pBuffer)->refcount();
         //   WFDMMLOGD2("m_sLatestFrameQ[%d].pBuffer has ref count %d",
         //       i, refCount);
         //   for(int j =1; i< refCount; i++)
         //   {
                //Call release on all refernces since at this point there is
                //probably no one to call release on these lingering references
                ((MediaBuffer*)m_sLatestFrameQ[i].pBuffer)->release();
                mWfdVcapStats.nRelasedBuff++;
         //   }
         //   mWfdVcapStats.nRelasedBuff++;
            m_sLatestFrameQ[i].isValid = false;
        }
    }
}


/*==============================================================================

         FUNCTION:          inputDeliveryTimerCb

         DESCRIPTION:
*//**       @brief          Timer callback
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          pPtr - pointer to object

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSourceVideoCapture::inputDeliveryTimerCb(void *pPtr)
{
    WFDMMSourceVideoCapture *pMe = (WFDMMSourceVideoCapture *)pPtr;

    if(pMe)
    {
        pMe->inputDeliveryHandler();
    }
}

/*==============================================================================

         FUNCTION:          inputDeliveryHandler

         DESCRIPTION:
*//**       @brief          Timer handler for reading of frames off the FrameQ
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSourceVideoCapture::inputDeliveryHandler()
{

    OMX_TICKS timerKickTime = 0;
    GetCurTime(timerKickTime);
    WFDMMLOGD1("Timer kicked in with diff of %lld!",
        timerKickTime - mWfdVcapStats.prevKickTime);
    mWfdVcapStats.prevKickTime = timerKickTime;

    OMX_U32                 nFramesRead = 0;
    OMX_U32                     nFrames = 0;
    OMX_BUFFERHEADERTYPE *pBufferHdr[2] = {NULL};
    bool                       isNew[2] = {false};


    if(!m_pVideoInputQ)
    {
        WFDMMLOGE("Invalid Queues etc.");
        return;
    }

    if(!ISSTATEOPENED)
    {
        WFDMMLOGE("Capture Timer fired in Invalid State");

        if(ISSTATECLOSING || ISSTATECLOSED)
        {
            /*------------------------------------------------------------------
             If the state is closing do encoder a favor by returning all
             buffers
            --------------------------------------------------------------------
            */
            WFDMMLOGD1("Queue Size is %ld", m_pVideoInputQ->GetSize());
            while(m_pVideoInputQ->GetSize())
            {
                WFDMMLOGD1("Queue Size is %ld", m_pVideoInputQ->GetSize());
                pBufferHdr[0] = NULL;
                m_pVideoInputQ->Pop(&pBufferHdr[0], sizeof(&pBufferHdr[0]), 0);
                if(pBufferHdr[0])
                {
                    pBufferHdr[0]->nFilledLen = 0;
                    pBufferHdr[0]->pOutputPortPrivate = NULL;
                    m_pFnData(m_pAppData, pBufferHdr[0]);
                }
            }
        }
        return;
    }

    /*--------------------------------------------------------------------------
     Increment a counter. We run the timer at twice the framerate. Story about
     we use odd occurence of timer or even shall be told later in the function
    ----------------------------------------------------------------------------
    */
    m_nTimerCtr++;

    OMX_TICKS nCurrTime = 0;
    /*--------------------------------------------------------------------------
     Read the time to be sent with frame if any
    ----------------------------------------------------------------------------
    */

    GetCurTime(nCurrTime);

    /*--------------------------------------------------------------------------
     If it is a full frame timer expiry we decide to send a frame
    ----------------------------------------------------------------------------
    */
    if(m_nTimerCtr % 2 == 0 || !m_bFirstFrameFetched)
    {
        nFrames++;
    }

    /*--------------------------------------------------------------------------
      If we had missed a frame in the previous full frame interval we will
      try to send two frames here
    ----------------------------------------------------------------------------
    */
    if(m_bMissedFrame)
    {
        nFrames++;
    }


    for(OMX_U32 i = 0; i < nFrames; i++)
    {
        m_pVideoInputQ->Pop(&(pBufferHdr[i]), sizeof(pBufferHdr[i]), 0);

        if(pBufferHdr[i])
        {
            WFDMMLOGH1("FRC Found Buf Hdr Fetch Data %p", pBufferHdr[i]);
            captureFrameInfoType sFrameInfo;

            memset(&sFrameInfo, 0 , sizeof(sFrameInfo));

            int nRet = AccessLatestBufQ(true, false, &sFrameInfo);
#if 0
           if(nRet == VC_INVALID_OP)
            {
                /*--------------------------------------------------------------
                 If the read returns an invalid operation (most probable) in
                 the initial stages of session then return back the buffer to
                 VideoInputQ to avert the scenario of exhausting the buffers
                ----------------------------------------------------------------
                */
                m_pVideoInputQ->Push(&pBufferHdr[i], sizeof(&pBufferHdr[i]));
            }
#endif
#if 0
            if(nRet == VC_READ_NO_FRAME_UPDATE && m_bFirstFrameFetched)
            {
                /*--------------------------------------------------------------
                 If no frame update has happened and we know that frames
                 have already arrived repeat the previous frame
                 if a full frame duration has elapsed since the last frame
                 update
                ----------------------------------------------------------------
                */
                if(m_nTimerCtr %2 == 0 && i == 0 && m_bMissedFrame)
                {
                    nRet = AccessLatestBufQ(true, true, &sFrameInfo);
                }
            }
#endif
            if(nRet == VC_READ_NEW_FRAME || nRet == VC_READ_FRAME_REGENERATED)
            {
                nFramesRead++;
                if(!m_bFirstFrameFetched)
                {
                    /*----------------------------------------------------------
                      Decide to pick at odd number or even number of callbacks
                      based on when the frame arrives for first time
                    ------------------------------------------------------------
                    */
                    m_bFirstFrameFetched = OMX_TRUE;
                    m_nTimerCtr = 0;
                }
                pBufferHdr[i]->pOutputPortPrivate = (OMX_PTR)sFrameInfo.pBuffer;

                if(nRet == VC_READ_NEW_FRAME)
                {
                    isNew[i] = true;
                }
            }
            else
            {
                WFDMMLOGE1("No frame push Buffer Back because %d",nRet);
                m_pVideoInputQ->Push(&pBufferHdr[i], sizeof(&pBufferHdr[i]));
                break;
            }
        }
        else if(i == 0)
        {
            WFDMMLOGE("!!! Error timed out POP");
            return;
        }
    }

    /*--------------------------------------------------------------------------
    Now lets walk through what we received from above loop and see what all
    conditions from below are occuring and make some decisions accordingly

    No retransmission

    TimerExpiry: -|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
    Expected DQ:  x-------x-------x-------x-------x-------x-------x-------x
    DQed frame :  x-------x-------x-------x-------x-------x-------x-------x
    FramesTSms:   0------33------66-----100-----133----166------200-----233

    In this example we can see even if there is a jitter there is no frame
    regeneration even if no frame arrived on timer expiry.

    Retransmission

    Case 1: Occassional Single frame drops
    TimerExpiry: -|---|---|---|---|---|---|---|---|---|---|---|---|---|---|--
    Expected DQ:  x-------x-------x-------x-------x-------x-------x-------x--
    DQed frame :  x-------x-------x-------o-------x-------x-------o------ x--
    FramesTSms:   0------33-------66------------100&133--166-----------200&233
    Red indicates regenerated frames


    Case 1: Continuous frame drops
    TimerExpiry: -|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---
    Expected DQ:  x-------x-------x-------x-------x-------x-------x-------x----
    DQed frame :  x-------x-------x-------o-------o-------o-------o-------x----
    FramesTSms:   0------33------66------------- 100-----133-----166----200233

     Verbosity of below code is for understanding puporpose.
    -------------------------------------------------------------------------------
    */

    if(nFramesRead)
    {
        if(pBufferHdr[0])
        {
            WFDMMLOGH2("FRC Buffer Header 0 %p BUffer0 = %p",
                pBufferHdr[0],pBufferHdr[0]->pOutputPortPrivate);
        }
        if(pBufferHdr[1])
        {
            WFDMMLOGH2("FRC Buffer Header 1 %p BUffer1 = %p",
                pBufferHdr[1],pBufferHdr[1]->pOutputPortPrivate);
        }
    }
    if(m_pLastXmitBuffer)
    {
        WFDMMLOGH1("FRC Prev = %p", m_pLastXmitBuffer);
    }


    if(!m_bFirstFrameFetched)
    {
        WFDMMLOGE("FRC: Before Start");
    }
    else if(m_sConfig.bEnableFrameSkip)
    {
        WFDMMLOGE("FRC: Frame Skip enabled");
        if(nFramesRead == 1)
        {
            /*------------------------------------------------------------------
             No frame drop case.
            --------------------------------------------------------------------
            */
            pBufferHdr[0]->nTimeStamp = nCurrTime;
            if(m_pLastXmitBuffer == NULL)
            {
                m_pLastXmitBuffer =
                       (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
                m_pLastXmitBuffer->add_ref();
                mWfdVcapStats.nAcquiredBuff++;
            }
            else if(m_pLastXmitBuffer)
            {
                m_pLastXmitBuffer->release();
                mWfdVcapStats.nRelasedBuff++;
                m_pLastXmitBuffer =
                     (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
                m_pLastXmitBuffer->add_ref();
                mWfdVcapStats.nAcquiredBuff++;
            }
            if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
            {
                MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                          pOutputPortPrivate;
                if((char*)pMediaBuffer->data())
                {
                    memcpy(pBufferHdr[0]->pBuffer,
                          (char*)pMediaBuffer->data() +
                           pMediaBuffer->range_offset(),
                           pMediaBuffer->range_length());

                    pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                    fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                }
            }
            m_nPrevBufferTS = nCurrTime;
            m_pFnData(m_pAppData, pBufferHdr[0]);
            m_nMinRetransmitCnt = VC_MIN_RETRANSMIT_COUNT;
        }
        else if(!m_sConfig.nFrameSkipInterval && !m_nMinRetransmitCnt)
        {
            WFDMMLOGE("FRC: Infinite Frame Skipping. No retransmission");
        }
        else
        {
            if(((uint32)nCurrTime - (uint32)m_nPrevBufferTS) >
                m_sConfig.nFrameSkipInterval || m_nMinRetransmitCnt)
            {
                WFDMMLOGH("FRC : Frame skipping Time to retransmit");
                WFDMMLOGE("FRC: Case 1 in Frame Skipping");
                pBufferHdr[0] = 0;
                m_pVideoInputQ->Pop(&(pBufferHdr[0]), sizeof(pBufferHdr[0]), 0);

                if(!pBufferHdr[0])
                {
                    WFDMMLOGD("No free buffer return");
                    return;
                }

                m_nPrevBufferTS = nCurrTime;
                /*--------------------------------------------------------------
                 No frame received this time. Retransmit previous frame
                ----------------------------------------------------------------
                */
                pBufferHdr[0]->nTimeStamp = nCurrTime;
                pBufferHdr[0]->pOutputPortPrivate = (OMX_PTR)m_pLastXmitBuffer;
                if (m_pLastXmitBuffer)
                    m_pLastXmitBuffer->add_ref();
                mWfdVcapStats.nAcquiredBuff++;

                if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
                {
                    MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                             pOutputPortPrivate;
                    if((char*)pMediaBuffer->data())
                    {
                        memcpy(pBufferHdr[0]->pBuffer,
                              (char*)pMediaBuffer->data() +
                              pMediaBuffer->range_offset(),
                              pMediaBuffer->range_length());

                        pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();
                        fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                    }
                }
                m_pFnData(m_pAppData, pBufferHdr[0]);
                if(m_nMinRetransmitCnt)
                {
                    m_nMinRetransmitCnt--;
                }
            }
        }
    }
    else if(!m_bMissedFrame && nFramesRead == 0 && m_nTimerCtr % 2 == 0)
    {
        /*----------------------------------------------------------------------
         Full frame period expired without a frame, wait until next frame update
        ------------------------------------------------------------------------
        */
        m_bMissedFrame = OMX_TRUE;
    }
    else if(!m_bMissedFrame && nFramesRead == 1)
    {
        /*----------------------------------------------------------------------
         No frame drop case.
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("FRC: Normal Buffer Case");
        pBufferHdr[0]->nTimeStamp = nCurrTime;
        if(m_pLastXmitBuffer == NULL)
        {
            m_pLastXmitBuffer = (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
            m_pLastXmitBuffer->add_ref();
            mWfdVcapStats.nAcquiredBuff++;
        }
        else if(m_pLastXmitBuffer)
        {
            m_pLastXmitBuffer->release();
            mWfdVcapStats.nRelasedBuff++;
            m_pLastXmitBuffer =
                 (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
            m_pLastXmitBuffer->add_ref();
            mWfdVcapStats.nAcquiredBuff++;
        }
        if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
        {
            MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                      pOutputPortPrivate;
            if((char*)pMediaBuffer->data())
            {
                memcpy(pBufferHdr[0]->pBuffer,
                      (char*)pMediaBuffer->data() +
                       pMediaBuffer->range_offset(),
                       pMediaBuffer->range_length());

                pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
            }
        }
        m_pFnData(m_pAppData, pBufferHdr[0]);
    }

    else if(m_bMissedFrame && nFramesRead == 0 && m_nTimerCtr %2 == 0)
    {
        WFDMMLOGE("FRC: Case 1");
        pBufferHdr[0] = 0;
        m_pVideoInputQ->Pop(&(pBufferHdr[0]), sizeof(pBufferHdr[0]), 0);

        if(!pBufferHdr[0])
        {
            WFDMMLOGD("No free buffer return");
            return;
        }

        /*----------------------------------------------------------------------
         No frame received this time. Retransmit previous frame
        ------------------------------------------------------------------------
        */
        pBufferHdr[0]->nTimeStamp = nCurrTime -
                             (uint64)1000000/m_sConfig.nFramerate;
        pBufferHdr[0]->pOutputPortPrivate = (OMX_PTR)m_pLastXmitBuffer;

        if (m_pLastXmitBuffer)
            m_pLastXmitBuffer->add_ref();
        mWfdVcapStats.nAcquiredBuff++;

        if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
        {
            MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                     pOutputPortPrivate;
            if((char*)pMediaBuffer->data())
            {
                memcpy(pBufferHdr[0]->pBuffer,
                      (char*)pMediaBuffer->data() +
                      pMediaBuffer->range_offset(),
                      pMediaBuffer->range_length());

                pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
            }
        }
        m_pFnData(m_pAppData, pBufferHdr[0]);
    }

    else if(m_bMissedFrame && nFramesRead == 2)
    {
        WFDMMLOGE("FRC: Case 2");
        m_bMissedFrame = OMX_FALSE;
        if(pBufferHdr[0])
        {
            pBufferHdr[0]->nTimeStamp = nCurrTime -
                                        (uint64)1000000/m_sConfig.nFramerate;
            if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
            {
                MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                          pOutputPortPrivate;
                if((char*)pMediaBuffer->data())
                {
                    memcpy(pBufferHdr[0]->pBuffer,
                          (char*)pMediaBuffer->data() +
                          pMediaBuffer->range_offset(),
                          pMediaBuffer->range_length());

                    pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                    fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                }
            }
            m_pFnData(m_pAppData, pBufferHdr[0]);
        }

        if(pBufferHdr[1])
        {
            pBufferHdr[1]->nTimeStamp = nCurrTime;

            if(pBufferHdr[1]->pBuffer && pBufferHdr[1]->pOutputPortPrivate)
            {
                MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[1]->
                                                          pOutputPortPrivate;
                if((char*)pMediaBuffer->data())
                {
                    memcpy(pBufferHdr[1]->pBuffer,
                          (char*)pMediaBuffer->data() +
                          pMediaBuffer->range_offset(),
                          pMediaBuffer->range_length());

                    pBufferHdr[1]->nFilledLen =  pMediaBuffer->size();//range_length();
                    fillStatInfo(pBufferHdr[1],nCurrTime,pMediaBuffer);
                }
            }
            if(m_pLastXmitBuffer)
            {
                m_pLastXmitBuffer->release();
                mWfdVcapStats.nRelasedBuff++;
                m_pLastXmitBuffer =
                     (MediaBuffer *)pBufferHdr[1]->pOutputPortPrivate;
                m_pLastXmitBuffer->add_ref();
                mWfdVcapStats.nAcquiredBuff++;
            }
            m_pFnData(m_pAppData, pBufferHdr[1]);
        }
    }

    /*--------------------------------------------------------------------------
     Here we check the case where a frame was not ready at the frame duration,
     but it arrived during the next half frame timer expiry.
    ----------------------------------------------------------------------------
    */
    else if(m_bMissedFrame && nFramesRead == 1 && m_nTimerCtr % 2)
    {
        WFDMMLOGE("FRC: Case 3");
        m_bMissedFrame = OMX_FALSE;
        pBufferHdr[0]->nTimeStamp = nCurrTime -
                             (uint64)500000/m_sConfig.nFramerate;

        if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
        {
            MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                      pOutputPortPrivate;
            if((char*)pMediaBuffer->data())
            {
                memcpy(pBufferHdr[0]->pBuffer,
                      (char*)pMediaBuffer->data() +
                       pMediaBuffer->range_offset(),
                       pMediaBuffer->range_length());

                pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
            }
        }
        if(m_pLastXmitBuffer)
        {
            m_pLastXmitBuffer->release();
            mWfdVcapStats.nRelasedBuff++;
            m_pLastXmitBuffer =
                 (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
            m_pLastXmitBuffer->add_ref();
            mWfdVcapStats.nAcquiredBuff++;
        }
        m_pFnData(m_pAppData, pBufferHdr[0]);

    }
    /*--------------------------------------------------------------------------
     Here we check the condition where a frame wasnt available in a timer expiry
     at frame duration and the next half farme duration but arrived at next
     full frame duration. If the frame is a new one then we can assume that a
     frame for previous frame duration was dropped and a new frame came.
     In that case we need to send an older frame and then send the new frame
    ----------------------------------------------------------------------------
    */
    else if(m_bMissedFrame && nFramesRead == 1 && m_nTimerCtr % 2 == 0)
    {
        WFDMMLOGE("FRC: Case 4");
        m_bMissedFrame = OMX_FALSE;
     //   if(isNew[0] == true)
        {
            /*------------------------------------------------------------------
             A new frame has arrived after a previous frame drop
            --------------------------------------------------------------------
            */
            pBufferHdr[1] = NULL;
            m_pVideoInputQ->Pop((&pBufferHdr[1]), sizeof(pBufferHdr[1]), 0);

            if(pBufferHdr[1] == NULL)
            {
                /*--------------------------------------------------------------
                 No new buffer available, send the new frame with current
                 timestamp
                ----------------------------------------------------------------
                */
                pBufferHdr[0]->nTimeStamp = nCurrTime;
                if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
                {
                    MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                             pOutputPortPrivate;
                    if((char*)pMediaBuffer->data())
                    {
                        memcpy(pBufferHdr[0]->pBuffer,
                              (char*)pMediaBuffer->data() +
                              pMediaBuffer->range_offset(),
                              pMediaBuffer->range_length());

                        pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                        fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                    }
                }
                if(m_pLastXmitBuffer)
                {
                    m_pLastXmitBuffer->release();
                    mWfdVcapStats.nRelasedBuff++;
                    m_pLastXmitBuffer =
                         (MediaBuffer *)pBufferHdr[0]->pOutputPortPrivate;
                    m_pLastXmitBuffer->add_ref();
                    mWfdVcapStats.nAcquiredBuff++;
                }
                m_pFnData(m_pAppData, pBufferHdr[0]);
            }
            else
            {
                OMX_PTR pTemp = pBufferHdr[0]->pOutputPortPrivate;

                pBufferHdr[0]->nTimeStamp = nCurrTime -
                                     (uint64)1000000/m_sConfig.nFramerate;
                pBufferHdr[0]->pOutputPortPrivate = m_pLastXmitBuffer;
                if (m_pLastXmitBuffer)
                    m_pLastXmitBuffer->add_ref();
                mWfdVcapStats.nAcquiredBuff++;

                if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
                {
                    MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                             pOutputPortPrivate;
                    if((char*)pMediaBuffer->data())
                    {
                        memcpy(pBufferHdr[0]->pBuffer,
                              (char*)pMediaBuffer->data() +
                              pMediaBuffer->range_offset(),
                              pMediaBuffer->range_length());

                        pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                        fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                    }
                }
                m_pFnData(m_pAppData, pBufferHdr[0]);


                pBufferHdr[1]->nTimeStamp = nCurrTime;
                pBufferHdr[1]->pOutputPortPrivate = pTemp;

                if(pBufferHdr[1]->pBuffer && pBufferHdr[1]->pOutputPortPrivate)
                {
                    MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[1]->
                                                              pOutputPortPrivate;
                    if((char*)pMediaBuffer->data())
                    {
                        memcpy(pBufferHdr[1]->pBuffer,
                              (char*)pMediaBuffer->data() +
                              pMediaBuffer->range_offset(),
                              pMediaBuffer->range_length());

                        pBufferHdr[1]->nFilledLen =  pMediaBuffer->size();//range_length();
                        fillStatInfo(pBufferHdr[1],nCurrTime,pMediaBuffer);
                    }
                }
                if(m_pLastXmitBuffer)
                {
                    m_pLastXmitBuffer->release();
                    mWfdVcapStats.nRelasedBuff++;
                    m_pLastXmitBuffer =
                         (MediaBuffer *)pBufferHdr[1]->pOutputPortPrivate;
                    m_pLastXmitBuffer->add_ref();
                    mWfdVcapStats.nAcquiredBuff++;
                }
                m_pFnData(m_pAppData, pBufferHdr[1]);

            }
        }
#if 0
        else
        {
            /*------------------------------------------------------------------
             Retransmission case
            --------------------------------------------------------------------
            */
            pBufferHdr[0]->nTimeStamp = nCurrTime -
                                 (uint64)1000000/m_sConfig.nFramerate;

            if(pBufferHdr[0]->pBuffer && pBufferHdr[0]->pOutputPortPrivate)
            {
                MediaBuffer *pMediaBuffer = (MediaBuffer*)pBufferHdr[0]->
                                                          pOutputPortPrivate;
                if((char*)pMediaBuffer->data())
                {
                    memcpy(pBufferHdr[0]->pBuffer,
                          (char*)pMediaBuffer->data() +
                          pMediaBuffer->range_offset(),
                          pMediaBuffer->range_length());

                    pBufferHdr[0]->nFilledLen =  pMediaBuffer->size();//range_length();
                    fillStatInfo(pBufferHdr[0],nCurrTime,pMediaBuffer);
                }
            }
            m_pFnData(m_pAppData, pBufferHdr[0]);
        }
#endif
    }

}


/*==============================================================================

         FUNCTION:          state

         DESCRIPTION:
*//**       @brief          sets or gets the state. This makes state transitions
                            thread safe
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            int state when get else a Dont Care
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
int WFDMMSourceVideoCapture::state(int state, bool get_true_set_false)
{
    WFD_VCAP_CRITICAL_SECT_ENTER(m_hStateCritSect);

    if(get_true_set_false == true)
    {
        /*----------------------------------------------------------------------
          Just return the current state
        ------------------------------------------------------------------------
        */

    }
    else
    {
        m_nState = state;
        WFDMMLOGE1("WFDMMSourceVideoCapture Moved to state %d",state);
    }

    WFD_VCAP_CRITICAL_SECT_LEAVE(m_hStateCritSect);

    return (int)m_nState;
}
