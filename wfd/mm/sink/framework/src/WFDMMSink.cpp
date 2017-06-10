/*==============================================================================
*       WFDMMSink.cpp
*
*  DESCRIPTION:
*       WFDMMSink framework for multimedia in WFD Sink
*
*
*  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
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
#include "MMDebugMsg.h"
#include "MMThread.h"
#include "MMTimer.h"
#include "MMMalloc.h"
#include "MMCriticalSection.h"
#include "MMDebugMsg.h"
#include "MMSignal.h"
#include "WFDMMSink.h"
#include "WFDMMSinkCommon.h"
#include "MMMemory.h"
#include "WFDMMSinkMediaSource.h"
#include "WFDMMSinkVideoDecode.h"
#include "RTPStreamPort.h"
#ifdef USE_OMX_AAC_CODEC
#include "WFDMMSinkAudioDecode.h"
#endif
#include "WFDMMSinkRenderer.h"
#include "WFDMMSinkStatistics.h"
#include "OMX_Core.h"
#include "WFDMMLogs.h"
#include "WFDMMThreads.h"
#include "HDCPManager.h"
#include "wfd_cfg_parser.h"

#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include "MMFile.h"

#include <binder/ProcessState.h>
#ifdef WFD_ICS
#include <surfaceflinger/Surface.h>
#else
#include <Surface.h>
#endif

#include "display_config.h"

#define SINK_MODULE_MEDIASOURCE       1001
#define SINK_MODULE_VIDEO_DECODER     2002
#define SINK_MODULE_AUDIO_DECODER     3003
#define SINK_MODULE_RENDERER          4004
#define SINK_MODULE_HDCP              5005
#define SINK_MODULE_SELF              6006

#define IDR_MIN_INTERVAL              500

#define DEINIT 0
#define INIT   1
#define PLAY   2
#define STOP   3
#define STREAM_PLAY 4
#define STREAM_PAUSE 5
#define STREAM_FLUSH 6
#define STREAM_SET_VOLUME 7
#define STREAM_SET_DECODER_LATENCY 8
#define STATE_CONTROL_MAX 9

extern int MM_Memory_InitializeCheckPoint( void );
extern int MM_Memory_ReleaseCheckPoint( void );

/* =============================================================================

                              DATA DECLARATIONS

================================================================================
*/
/* -----------------------------------------------------------------------------
** Constant / Define Declarations
** -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
** Type Declarations
** -------------------------------------------------------------------------- */
//This threshhold should be tuned\r
//it depends on Video frame size as well, bit rate,packetization scheme\r
#define DEFINE_THRESHHOLD_NBYTES_DROP 7*188*15 // 7Ts packets in 15 frames
#define DEFINE_TIMER_INTERVAL 1000
/* -----------------------------------------------------------------------------
** Global Constant Data Declarations
** -------------------------------------------------------------------------- */

/* =============================================================================
*                       Local Function Definitions
* =========================================================================== */

/* -----------------------------------------------------------------------------
** Local Data Declarations
** -------------------------------------------------------------------------- */
WFDMMSink        *WFDMMSink::m_pMe = NULL;
unsigned          WFDMMSink::m_nRefCnt = 0;
/* =============================================================================
**                          Macro Definitions
** ========================================================================== */

// MM related error checks
#define SET_MM_ERROR_FAIL        (m_nMMStatus = WFD_STATUS_FAIL)
#define SET_MM_ERROR_BADPARAM    (m_nMMStatus = WFD_STATUS_BADPARAM)
#define SET_MM_ERROR_UNSUPPORTED (m_nMMStatus = WFD_STATUS_NOTSUPPORTED)
#define SET_MM_ERROR_RESOURCE    (m_nMMStatus = WFD_STATUS_MEMORYFAIL)
#define RESET_MM_ERROR           (m_nMMStatus = WFD_STATUS_SUCCESS)

// DRM Related Error Checks
#define SET_DRM_ERROR_FAIL        (m_nDrmStatus = WFD_STATUS_FAIL)
#define SET_DRM_ERROR_BADPARAM    (m_nDrmStatus = WFD_STATUS_BADPARAM)
#define SET_DRM_ERROR_UNSUPPORTED (m_nDrmStatus = WFD_STATUS_NOTSUPPORTED)
#define SET_DRM_ERROR_RESOURCE    (m_nDrmStatus = WFD_STATUS_MEMORYFAIL)
#define RESET_DRM_ERROR           (m_nDrmStatus = WFD_STATUS_SUCCESS)

// MM State management
#define RESET_MM_STATE            (m_eMMState = WFD_SINK_MM_STATE_ZERO)
#define SET_MM_STATE_DEINIT       (m_eMMState = WFD_SINK_MM_STATE_DEINIT)
#define SET_MM_STATE_PLAY         (m_eMMState = WFD_SINK_MM_STATE_PLAY)
#define SET_MM_STATE_PAUSE        (m_eMMState = WFD_SINK_MM_STATE_PAUSE)
#define SET_MM_STATE_INIT         (m_eMMState = WFD_SINK_MM_STATE_INIT)

#define IS_MM_STATE_PLAY          (m_eMMState == WFD_SINK_MM_STATE_PLAY)
#define IS_MM_STATE_INIT          (m_eMMState == WFD_SINK_MM_STATE_INIT)
#define IS_MM_STATE_PAUSE         (m_eMMState == WFD_SINK_MM_STATE_PAUSE)
#define IS_MM_STATE_DEINIT        (m_eMMState == WFD_SINK_MM_STATE_DEINIT)

#define ISMMTEARDOWNNEEDED        (m_eState == MMWFDSINK_STATE_INIT ||         \
                                   m_eState == MMWFDSINK_STATE_PLAY ||         \
                                   m_eState == MMWFDSINK_STATE_PAUSE)          \
/*      END ISMMTEARDOWNNEEDED     */

// Drm State management
#define RESET_DRM_STATE           (m_eDrmState = WFD_SINK_DRM_STATE_ZERO)
#define SET_DRM_STATE_INIT        (m_eDrmState = WFD_SINK_DRM_STATE_INIT)
#define SET_DRM_STATE_ACQUIRING   (m_eDrmState = WFD_SINK_DRM_STATE_ACQUIRING)
#define SET_DRM_STATE_ACQUIRED    (m_eDrmState = WFD_SINK_DRM_STATE_ACQUIRED)

#define IS_DRM_STATE_INIT         (m_eDrmState == WFD_SINK_DRM_STATE_INIT)
#define IS_DRM_STATE_ACQUIRING    (m_eDrmState == WFD_SINK_DRM_STATE_ACQUIRING)
#define IS_DRM_STATE_ACQUIRED     (m_eDrmState == WFD_SINK_DRM_STATE_ACQUIRED)
#define IS_DRM_STATE_ZERO         (m_eDrmState == WFD_SINK_DRM_STATE_ZERO)
#define IS_DRM_TEARDOWN_NEEDED    (m_eDrmState != WFD_SINK_DRM_STATE_ZERO)

#define CRITICAL_SECT_ENTER if(m_hCritSect)                                    \
                                  MM_CriticalSection_Enter(m_hCritSect);       \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE if(m_hCritSect)                                    \
                                  MM_CriticalSection_Leave(m_hCritSect);       \
/*      END CRITICAL_SECT_LEAVE    */


#define CRITICAL_SECT_ENTER_FBD if(m_hCritSectFBD)                             \
                                  MM_CriticalSection_Enter(m_hCritSectFBD);    \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE_FBD if(m_hCritSectFBD)                             \
                                  MM_CriticalSection_Leave(m_hCritSectFBD);    \
/*      END CRITICAL_SECT_LEAVE    */


#define CRITICAL_SECT_ENTER_EBD if(m_hCritSectEBD)                             \
                                  MM_CriticalSection_Enter(m_hCritSectEBD);    \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE_EBD if(m_hCritSectEBD)                             \
                                  MM_CriticalSection_Leave(m_hCritSectEBD);    \
/*      END CRITICAL_SECT_LEAVE    */

#define CRITICAL_SECT_ENTER_EVT if(m_hCritSectEvtHdlr)                         \
                                  MM_CriticalSection_Enter(m_hCritSectEvtHdlr);\
/*      END CRITICAL_SECT_ENTER_EVT   */

#define CRITICAL_SECT_LEAVE_EVT if(m_hCritSectEvtHdlr)                         \
                                  MM_CriticalSection_Leave(m_hCritSectEvtHdlr);\
/*      END CRITICAL_SECT_LEAVE_EVT    */


#define RETURNUNSUPPORTED  {return WFD_STATUS_NOTSUPPORTED; }
#define RETURNNORESOURCES  {return WFD_STATUS_MEMORYFAIL;   }
#define RETURNSUCCESS      {return WFD_STATUS_SUCCESS;      }
#define RETURNFAIL         {return WFD_STATUS_FAIL;         }
#define RETURNBADPARAM     {return WFD_STATUS_BADPARAM;     }
#define RETURNSUCCESS      {return WFD_STATUS_SUCCESS;      }
#define RETURNBADSTATE     {return WFD_STATUS_RUNTIME_ERROR;}


// Memory Alloc Free Routines with Null checks
#define SINK_FREEIF(x)      if(x){MM_Free(x); x = NULL;}
#define SINK_MALLOC(x)      MM_Malloc(x)

#define CHECK_ERROR_STATUS if(m_nMMStatus != WFD_STATUS_SUCCESS){              \
                               WFDMMLOGE("Failed. Status Fail");               \
                               return m_nMMStatus;                             \
                           }                                                   \
/* END  CHECK_ERROR_STATUS  */

#define CHECK_ERROR_STATUS_CS if(m_nMMStatus != WFD_STATUS_SUCCESS){           \
                               WFDMMLOGE("Failed. Status Fail");               \
                               CRITICAL_SECT_LEAVE;                            \
                               return m_nMMStatus;                             \
                           }                                                   \
/* END  CHECK_ERROR_STATUS  */

#define CHECK_NULL_ERROR(x, y) if(x == NULL) {                                 \
        WFDMMLOGE(y);                                                          \
        return WFD_STATUS_FAIL;                                                \
}                                                                              \
/* END  CHECK_NULL_ERROR   */

#define DRM_HDCP_MIME_TYPE  "video/hdcp"

/* =============================================================================
**                            Function Definitions
** ========================================================================== */

/*==============================================================================

         FUNCTION:         WFDMMSink::CreateInstance

         DESCRIPTION:
*//**       @brief         This is the public API to create the single WFD Sink
                           instance.
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

*//*==========================================================================*/
WFDMMSink* WFDMMSink::CreateInstance()
{
    if(!m_pMe)
    {
        WFDMMLOGE("WFDMMSink::CreateInstance");
        m_pMe = MM_New(WFDMMSink);
        ++m_nRefCnt;
    }
    WFDMMLOGH1("WFDMMSink: Ref count = %u",m_nRefCnt);
    return m_pMe;
}

/*==============================================================================

         FUNCTION:         WFDMMSink::DeleteInstance

         DESCRIPTION:
*//**       @brief         This is the public API to delete the WFD Sink
                           instance.
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

*//*==========================================================================*/
void WFDMMSink::DeleteInstance()
{
    WFDMMLOGH1("Delete WFDMMSink, Instance count %d", (int)m_nRefCnt);

    if(m_pMe)
    {
        MM_Delete(m_pMe);
        m_pMe = NULL;
        --m_nRefCnt;
    }

    return;
}

/*==============================================================================

         FUNCTION:         WFDMMSink

         DESCRIPTION:
*//**       @brief         This is the WFDMMSink class constructor-initializes
                           the class members.
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

*//*==========================================================================*/
WFDMMSink::WFDMMSink()
{
    WFDMMLOGH("WFDMMSink constructor");
    MM_Memory_InitializeCheckPoint( );

    int nRet = 0;

    FILE *pDumpFile = fopen("/data/media/rtpdump.ts", "rb");
    if(pDumpFile != NULL)
    {
        fclose(pDumpFile);
        pDumpFile = NULL;
        if(MM_File_Delete("/data/media/rtpdump.ts") != 0)
            WFDMMLOGE("Caution!!Dump file not deleted.Previous dumps present");
    }

    RESET_MM_ERROR;
    RESET_DRM_ERROR;

    RESET_MM_STATE;
    RESET_DRM_STATE;

    nRet = MM_CriticalSection_Create(&m_hCritSect);
    if(nRet != 0)
    {
        m_hCritSect = NULL;
        WFDMMLOGE("Error in Critical Section Create");
    }

    nRet = MM_CriticalSection_Create(&m_hCritSectEBD);
    if(nRet != 0)
    {
        m_hCritSectEBD = NULL;
        WFDMMLOGE("Error in Critical Section Create for EBD");
    }


    nRet = MM_CriticalSection_Create(&m_hCritSectFBD);
    if(nRet != 0)
    {
        m_hCritSectFBD = NULL;
        WFDMMLOGE("Error in Critical Section Create for FBD");
    }

    nRet = MM_CriticalSection_Create(&m_hCritSectEvtHdlr);
    if(nRet != 0)
    {
        m_hCritSectEvtHdlr = NULL;
        WFDMMLOGE("Error in Critical Section Create for EvtHdlr");
    }

    m_nAudioSampleRate = 0;
    m_nAudioChannels = 0;
    m_pVideoSurface = NULL;
    m_pWindow = NULL;
    m_pHDCPManager = NULL;
    m_pAudioDecode = NULL;
    m_pVideoDecode = NULL;
    m_pMediaSrc    = NULL;
    m_pRenderer    = NULL;
    m_pSinkStatistics = NULL;
    m_bMMTearingDown = OMX_FALSE;
    m_nTimeLastIdrReq = 0;
    m_hIDRDelayTimer = NULL;
    m_bPendingIDRReq = false;
    m_bIDRTimerPending = false;
    m_nDecoderLatency = 0;
    m_nFlushTimestamp = 0;
    m_nActualBaseTime = 0;
    m_pRTPStreamPort = NULL;
    m_bUpdatedSurface = false;
    m_bStreamPaused = false;
    m_pNegotiatedCap = NULL;
    m_nFrameCntDownStartTime = 0;
    m_bNotifyFirstFrameInfo = true;
    m_pCmdThread = MM_New_Args(WFDMMThreads,
                               ((unsigned int)STATE_CONTROL_MAX));

    if(!m_pCmdThread)
    {
        WFDMMLOGE("Failed to create Command Thread");
    }

    if(m_pCmdThread && m_pCmdThread->Start(CmdThreadEntry, -2, 32768 * 4,
                           this, "SinkCmdThread"))
    {
        WFDMMLOGE("Failed to start command thread");
    }

    createSinkStatistics();

    SET_MM_STATE_DEINIT;
}

/*==============================================================================

         FUNCTION:          CmdThreadEntry

         DESCRIPTION:
*//**       @brief          Entry function for command thread
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
void WFDMMSink::CmdThreadEntry(void* pHandle,unsigned int nCmd)
{
    WFDMMSink *pMe = (WFDMMSink*)pHandle;

    if(!pMe)
    {
        WFDMMLOGE("Invalid Handle received");
        return;
    }

    return pMe->CmdThread(nCmd);
}

/*==============================================================================

         FUNCTION:          CmdThread

         DESCRIPTION:
*//**       @brief          Cmd thread which process commands
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
void WFDMMSink::CmdThread(int nCmd)
{
    OMX_ERRORTYPE eErr = OMX_ErrorNone;

    switch(nCmd)
    {
    case INIT:
        /*----------------------------------------------------------------------
          This is expected to come after SETUP. At this point we will allocate
          all resources needed by different modules and make them ready to start
          the session.
        ------------------------------------------------------------------------
        */
        if(IS_MM_STATE_INIT)
        {
            WFDMMLOGE("INIT when alreay in INIT");
            break;
        }
        if(IS_MM_STATE_PLAY)
        {
            WFDMMLOGE("INIT request in PLAY State");
            break;
        }

        /*----------------------------------------------------------------------
         Create the Surfacetextureclient here for this part of the session.
        ------------------------------------------------------------------------
        */
        if( m_pNegotiatedCap->pSurface != NULL )
        {
            WFDMMLOGH1("WFDMMSink: received surface from upper layer%p",
                       m_pNegotiatedCap->pSurface);
            m_pVideoSurface = (Surface*)m_pNegotiatedCap->pSurface;

            /*---------------------------------------------------------------------
                 Having a native window is a prerequisite for calling ::play
            -----------------------------------------------------------------------
            */
            if(m_pVideoSurface != NULL)
            {
#ifdef MR2
                m_pWindow       = MM_New_Args(Surface,
                                    (m_pVideoSurface->getIGraphicBufferProducer()));
#else
                m_pWindow       = MM_New_Args(SurfaceTextureClient,
                                    (m_pVideoSurface->getSurfaceTexture()));
#endif
                if(m_pWindow == NULL)
                {
                    WFDMMLOGE("WFDMMSink Failed to create Native Window");
                    SET_MM_ERROR_FAIL;
                    WFDMMLOGE("MediaSource failed to start");
                    break;
                }
            }
        }
        else
        {
            WFDMMLOGE("WFDMMSink Surface is Null");
            SET_MM_ERROR_FAIL;
            return;
        }

        WFDMMLOGH("Notify Display HAL to Disable Dynamic refresh rate");
        qdutils::configureDynRefreshRate(qdutils::DISABLE_METADATA_DYN_REFRESH_RATE, 0);
        /*----------------------------------------------------------------------
         Not if the native window is created proceed with creating resources
         for each module
        ------------------------------------------------------------------------
        */

        if(!createDataSource  () ||
           !createMediaSource () ||
           !createVideoDecoder() ||
           !createAudioDecoder() ||
           !createRenderer    ())
        {
            WFDMMLOGE("Failed to create MM components");
            if(!destroyMMComponents())
            {
                WFDMMLOGE("Failed to destroy MM components");
            }
            SET_MM_ERROR_FAIL;
            break;
        }

        WFDMMLOGH("WFDMMSINK Moves to INIT");
        SET_MM_STATE_INIT;

        break;


    case DEINIT:
        if(IS_MM_STATE_DEINIT)
        {
            WFDMMLOGE("DEINIT when already in DEINIT");
            if(!destroyMMComponents())
            {
                WFDMMLOGE("Failed to destroy MM components");
            }
            else
            {
                WFDMMLOGE("Success to destroy MM components");
            }
            break;
        }

        if(IS_MM_STATE_PLAY)
        {
            SET_MM_STATE_INIT;
            WFDMMLOGE("State In Play, Go to Init first");

            /*-----------------------------------------------------------------
                CHanged state to INIT, which means buffer flow will stop and
                buffers will be returned to final resting places of buffers.
                Calling renderer stop will make sure renderer doesnt hold any
                buffers.
                Call EBD and FDB once to make sure no buffers are midway
                while we are changing the state.
            -------------------------------------------------------------------
            */
            processEBD(SINK_MODULE_SELF, 0, NULL);
            processFBD(SINK_MODULE_SELF, 0, NULL);

            if(m_pRenderer != NULL)
            {
                m_pRenderer->Stop();
            }

            if(m_pVideoDecode != NULL)
            {
                m_pVideoDecode->Stop();
            }
#ifdef USE_OMX_AAC_CODEC
            if(m_pAudioDecode != NULL)
            {
                m_pAudioDecode->Stop();
            }
#endif
            if(m_pMediaSrc != NULL)
            {
                m_pMediaSrc->Stop();
            }

        }

        WFDMMLOGH("Notify Display HAL to Enable Dynamic refresh rate");
        qdutils::configureDynRefreshRate(qdutils::ENABLE_METADATA_DYN_REFRESH_RATE, 0);
        if(!destroyMMComponents())
        {
            WFDMMLOGE("Failed to destroy MM components");
        }

        if(m_pVideoSurface != NULL)
        {
            if(m_pWindow != NULL)
            {
              //  native_window_api_disconnect(m_pWindow.get(),
              //                            NATIVE_WINDOW_API_MEDIA);
                m_pWindow.clear();
                m_pWindow = NULL;
            }
            m_pVideoSurface.clear();
            m_pVideoSurface = NULL;
        }

        WFDMMLOGH("WFD Sink moves to DEINIT");
        SET_MM_STATE_DEINIT;
        break;

    case PLAY:
        WFDMMLOGH("Play Command Received");

        /*----------------------------------------------------------------------
         We receive this command after RTSP PLAY. By this time INIT must
         have completed
        ------------------------------------------------------------------------
        */

        /*----------------------------------------------------------------------
         There are a few things to do here.
         1. Make sure the state is in INIT
        ------------------------------------------------------------------------
        */
        if(IS_MM_STATE_PLAY)
        {
            WFDMMLOGE("PLAY when already in PLAY state");
            break;
        }
        if(!IS_MM_STATE_INIT)
        {
            WFDMMLOGE("Called play when not in INIT");
            return;
        }

        /*----------------------------------------------------------------------
        2. Resources are already created in INIT. Now start the modules to
        indicate the session is about to start. In this step, the buffers are
        to the rightful owners at the start of session,
         - All Video O/P buffers must be with decoder.
         - All Audio O/P buffers must be with audio decoder
         - All Video and Audio i/p buffers must be with MediaSource.
         - Renderer holds no buffers
        ------------------------------------------------------------------------
        */
        m_bNotifyFirstFrameInfo = true;

        if(m_bStreamPaused)
        {
            m_bStreamPaused = false;
        }

        if(m_pMediaSrc != NULL)
        {
            eErr = m_pMediaSrc->Start();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("MediaSource failed to start");
                break;
            }
        }

        if(m_pVideoDecode != NULL)
        {
            eErr = m_pVideoDecode->Start();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Video Decode failed to start");
                break;
            }
        }

        if(m_pRenderer != NULL)
        {
            eErr = m_pRenderer->Start();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Video Decode failed to start");
                break;
            }
        }
#ifdef USE_OMX_AAC_CODEC
        if(m_pAudioDecode != NULL)
        {
            eErr = m_pAudioDecode->Start();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Audio Decode failed to start");
                break;
            }
        }
#endif
        WFDMMLOGH("WFD Sink MM moves to PLAY");
        SET_MM_STATE_PLAY;
        break;

    case STOP:
        WFDMMLOGH("Stop Command Received");
        /*----------------------------------------------------------------------
         We receive this command after RTSP TEARDOWN or Pause.
        ------------------------------------------------------------------------
        */

        /*----------------------------------------------------------------------
         There are a few things to do here.
         1. Make sure the state is in PLAY.
         2. Each component has its role to play to ensure the buffer flow has
         been stopped properly.
            VideoDecode module must let go off all input buffers and must
            be returned to the MediaSource
            VideoDecode must collect all output buffers back from Renderer
            Renderer must let go of all audio and video buffers.
            Audio Decoder wherever applicable must hold on to all audio output
            buffers.
            MediaSource must collect all VideoDecoder input buffers and audio
            input Buffers.
        3. Once this is done we can move the state to INIT. Resources will
           remain allocated.
        ------------------------------------------------------------------------
        */
        if(!IS_MM_STATE_PLAY)
        {
            WFDMMLOGE("Stop when not in PLAY. Nothing to do");
            break;
        }


        if(m_pMediaSrc != NULL)
        {
            eErr = m_pMediaSrc->Stop();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("MediaSource failed to stop");
            }
        }

        if(m_pVideoDecode != NULL)
        {
            eErr = m_pVideoDecode->Stop();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Video Decode failed to Stop");
            }
        }

        if(m_pRenderer != NULL)
        {
            eErr = m_pRenderer->Stop();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Video Decode failed to Stop.. ");
            }
        }
#ifdef USE_OMX_AAC_CODEC
        if(m_pAudioDecode != NULL)
        {
            eErr = m_pAudioDecode->Stop();
            if(eErr != OMX_ErrorNone)
            {
                SET_MM_ERROR_FAIL;
                WFDMMLOGE("Audio Decode failed to Stop.. ");
            }
        }
#endif
        /*----------------------------------------------------------------------
           Delete any timers etc.
        ------------------------------------------------------------------------
        */
        if(m_hIDRDelayTimer)
        {
            MM_Timer_Release(m_hIDRDelayTimer);
            m_hIDRDelayTimer = NULL;
        }
        WFDMMLOGH("WFD Sink MM moves to INIT");
        SET_MM_STATE_INIT;
        break;
    case STREAM_PLAY:
        if(IS_MM_STATE_PLAY && m_bStreamPaused)
        {
            WFDMMLOGH("STREAM_PLAY: Notify Display HAL to Disable Dynamic refresh rate");
            qdutils::configureDynRefreshRate(qdutils::DISABLE_METADATA_DYN_REFRESH_RATE, 0);
            if(m_pRenderer)
            {
                m_pRenderer->restartRendering(m_bFlushAll);
            }

            if(m_pMediaSrc)
            {
                m_pMediaSrc->streamPlay(m_bFlushAll);
            }

            m_bStreamPaused = false;
        }
        break;
    case STREAM_PAUSE:
        if(IS_MM_STATE_PLAY && !m_bStreamPaused)
        {
            WFDMMLOGH("STREAM_PAUSE: Notify Display HAL to Enable Dynamic refresh rate");
            qdutils::configureDynRefreshRate(qdutils::ENABLE_METADATA_DYN_REFRESH_RATE, 0);
            if(m_pMediaSrc)
            {
                m_pMediaSrc->streamPause();
            }
            if(m_pRenderer)
            {
                m_pRenderer->pauseRendering();
            }
            m_bStreamPaused = true;
        }
        break;
    case STREAM_FLUSH:
        if(IS_MM_STATE_PLAY)
        {
            if(m_pMediaSrc)
            {
                m_pMediaSrc->setFlushTimeStamp(m_nFlushTimestamp);
            }

            if(m_pRenderer)
            {
                m_pRenderer->setFlushTimeStamp(m_nFlushTimestamp);
            }
        }
        break;
    case STREAM_SET_DECODER_LATENCY:
        if(m_pRenderer)
        {
            m_pRenderer->setDecoderLatency(m_nDecoderLatency);
        }
        break;
    default:
        break;
    }
    return;
}



/*==============================================================================

         FUNCTION:         ~WFDMMSink

         DESCRIPTION:
*//**       @brief         This is the WFDMMSink class destructor
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

*//*==========================================================================*/
WFDMMSink::~WFDMMSink()
{
    WFDMMLOGE("WFDMMSink destructor");

    //Teardown HDCP and MM sessions
    (void)teardownHDCPSession();
    (void)teardownMMSession();

    if(!destroySinkStatistics())
        WFDMMLOGE("Failed to destroy Sink Statistics instance");

    if(m_hCritSect)
    {
        MM_CriticalSection_Release(m_hCritSect);
        m_hCritSect = NULL;
    }

    if(m_hCritSectEBD)
    {
        MM_CriticalSection_Release(m_hCritSectEBD);
        m_hCritSectEBD = NULL;
    }
    if(m_hCritSectFBD)
    {
        MM_CriticalSection_Release(m_hCritSectFBD);
        m_hCritSectFBD = NULL;
    }

    if(m_hCritSectEvtHdlr)
    {
        MM_CriticalSection_Release(m_hCritSectEvtHdlr);
        m_hCritSectEvtHdlr = NULL;
    }
    if(m_pCmdThread)
    {
        MM_Delete(m_pCmdThread);
        m_pCmdThread = NULL;
    }
    MM_Memory_ReleaseCheckPoint();
}

/*==============================================================================

         FUNCTION:         setupHDCPSession

         DESCRIPTION:
*//**       @brief         sets up HDCP connection unless already connected to
                           same IP and port
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pIpAddr - IP address of the peer
                           port    - port number to listen for incoming
                                     connection

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::setupHDCPSession
(
    char *pIpAddr,
    unsigned short port
)
{
    WFDMMLOGH("WFDMMSink::setupHDCPSession");
    CRITICAL_SECT_ENTER;

    if(!IS_DRM_STATE_ZERO)
    {
        WFDMMLOGE("WFDMMSink Not in a state to Setup HDCP");
        CRITICAL_SECT_LEAVE;
        RETURNBADSTATE;
    }

    int uniqueId = 0;
    if(m_pHDCPManager == NULL)
    {
        m_pHDCPManager = MM_New(HDCPManager);

        if(!m_pHDCPManager)
        {
            WFDMMLOGE("WFDMMSink: creating HDCP client Failed");
            SET_DRM_ERROR_RESOURCE;
            CRITICAL_SECT_LEAVE;
            return WFD_STATUS_FAIL;

        }

        SET_DRM_STATE_INIT;

        if(m_pHDCPManager && m_pHDCPManager->getHDCPManagerState()
            == HDCP_STATE_DEINIT)
        {
            HDCPStatusType eStatus =  m_pHDCPManager->initializeHDCPManager();

            if(eStatus != HDCP_SUCCESS)
            {
                /*--------------------------------------------------------------
                  MM session can still proceed. Just set DRM state to fail
                ----------------------------------------------------------------
                */
                WFDMMLOGE("Failed to initialize HDCP session");
                SET_DRM_ERROR_FAIL;
                CRITICAL_SECT_LEAVE;
                return WFD_STATUS_FAIL;
            }
            else
            {
                eStatus = m_pHDCPManager->setupHDCPSession(HDCP_MODE_RX,
                                                   pIpAddr,port);
                if(eStatus != HDCP_SUCCESS)
                {
                    WFDMMLOGE("Failed to setup HDCP session");
                    SET_DRM_ERROR_FAIL;
                    CRITICAL_SECT_LEAVE;
                    return WFD_STATUS_FAIL;
                }
            }

        }
    }
    SET_DRM_STATE_ACQUIRING;

    CRITICAL_SECT_LEAVE;
    WFDMMLOGH("WFDMMSink::setupHDCPSession.... Done");

    return WFD_STATUS_SUCCESS;
}

/*==============================================================================

         FUNCTION:         getHDCPStatus

         DESCRIPTION:
*//**       @brief         gets currents status of HDCP connection
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pIpAddr - ip address of peer
                           port - port number to listen


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::getHDCPStatus
(
    char *pIpAddr,
    unsigned short port
)
{
    (void) pIpAddr;
    (void) port;
    CRITICAL_SECT_ENTER;
    if(m_pHDCPManager != NULL)
    {
        if(m_pHDCPManager->getHDCPManagerState() == HDCP_STATE_CONNECTED)
        {
            WFDMMLOGH("Acquired HDCP session");
            SET_DRM_STATE_ACQUIRED;
        }

        CRITICAL_SECT_LEAVE;
        return WFD_STATUS_SUCCESS;
    }
    CRITICAL_SECT_LEAVE;
    return WFD_STATUS_FAIL;
}

/*==============================================================================

         FUNCTION:         teardownHDCPSession

         DESCRIPTION:
*//**       @brief         tears down HDCP session with peer with the ip
                           specified
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pIpAddr - ip address of peer
                           port - port number to listen


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::teardownHDCPSession
(
)
{
    WFDMMLOGH("WFDMMSink Teardown HDCP Session");
    CRITICAL_SECT_ENTER;
    if(!IS_DRM_TEARDOWN_NEEDED)
    {
        WFDMMLOGE("DRM HDCP not initialized. No teadown needed");
        CRITICAL_SECT_LEAVE;
        RETURNSUCCESS;
    }


    if(m_pHDCPManager)
    {
        m_pHDCPManager->teardownHDCPSession();
        m_pHDCPManager->deinitializeHDCPManager();
        MM_Delete(m_pHDCPManager);
        m_pHDCPManager = NULL;
    }

    RESET_DRM_ERROR;
    RESET_DRM_STATE;

    WFDMMLOGH("WFDMMSink Teardown HDCP Session... Done");
    CRITICAL_SECT_LEAVE;
    RETURNSUCCESS;
}

/*==============================================================================

         FUNCTION:         setupMMSession

         DESCRIPTION:
*//**       @brief         This is the WFDMMSink class constructor-initializes
                           the class members.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         WFD_device_t  -  WFD Device Type
                           WFD_MM_capability_t  - WFD Negotiated Capability
                           WFD_MM_callbacks_t - WFD Callback


*//*     RETURN VALUE:
*//**       @return
                           WFD_STATUS_SUCCESS or other error codes

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::setupMMSession
(
    WFD_device_t             eWFDDeviceType,
    WFD_MM_capability_t      *pNegotiatedCap,
    WFD_MM_callbacks_t       *pCallback
)
{
    WFDMMLOGH("WFDMMSink::setupMMSession");

    CRITICAL_SECT_ENTER;

    if( (eWFDDeviceType != WFD_DEVICE_PRIMARY_SINK &&
         eWFDDeviceType != WFD_DEVICE_SECONDARY_SINK ) ||
        ( pNegotiatedCap == NULL) ||
        (pNegotiatedCap->video_method == WFD_VIDEO_H264 &&
         pNegotiatedCap->video_config.video_config.num_h264_profiles == 0))
    {
        WFDMMLOGE("WFDMMSink::WFDMMSink bad parameters");
        CRITICAL_SECT_LEAVE;
        RETURNBADPARAM;
    }

    /**---------------------------------------------------------------------------
    Validate Codecs.
    ---------------
    This is the only place where we should be checking for
    the codecs that we support as of now. This will reduce the effort to
    add support for more codecs later. Check for TODOs elsewhere too.
    ------------------------------------------------------------------------------
    */
    if(pNegotiatedCap->video_method == WFD_VIDEO_3D
#ifndef USE_AUDIO_TUNNEL_MODE
       || pNegotiatedCap->audio_method == WFD_AUDIO_DOLBY_DIGITAL
#endif
    )
    {
        WFDMMLOGE("WFDMMSink::WFDMMSink Unsupported Codecs");
        SET_MM_ERROR_UNSUPPORTED;
        CRITICAL_SECT_LEAVE;
        RETURNUNSUPPORTED;
    }

    m_nTimeLastIdrReq = 0;

    WFDMMLOGE("Sanity check done");

    if(pCallback)
    {
        sCallBacks.av_format_change_cb = pCallback->av_format_change_cb;
        sCallBacks.capability_cb       = pCallback->capability_cb;
        sCallBacks.idr_cb              = pCallback->idr_cb;
        sCallBacks.update_event_cb     = pCallback->update_event_cb;
    }

    /**---------------------------------------------------------------------------
    Create a local copy of the negotiated capability
    ------------------------------------------------------------------------------
    */
    m_pNegotiatedCap = (WFD_MM_capability_t *)
        SINK_MALLOC( sizeof(WFD_MM_capability_t) );

    if (m_pNegotiatedCap)
    {
        m_pNegotiatedCap->video_config.video_config.h264_codec =
          (WFD_h264_codec_config_t*)SINK_MALLOC(sizeof(WFD_h264_codec_config_t) *
          pNegotiatedCap->video_config.video_config.num_h264_profiles);
    }


    if(m_pNegotiatedCap &&
        m_pNegotiatedCap->video_config.video_config.h264_codec)
    {
        uint32 *pTemPtr = (uint32*)m_pNegotiatedCap->video_config.
                                                     video_config.h264_codec;
        memcpy(m_pNegotiatedCap, pNegotiatedCap,
            sizeof(WFD_MM_capability_t));
        m_pNegotiatedCap->video_config.video_config.h264_codec =
            (WFD_h264_codec_config_t*)pTemPtr;

        memcpy(m_pNegotiatedCap->video_config.video_config.h264_codec,
            pNegotiatedCap->video_config.video_config.h264_codec,
            sizeof(WFD_h264_codec_config_t));

    }
    else
    {
        WFDMMLOGE("WFDMMSink::WFDMMSink memory allocation failed");
        SET_MM_ERROR_BADPARAM;
        CRITICAL_SECT_LEAVE;
        RETURNBADPARAM;
    }

    /**---------------------------------------------------------------------------
    If content protection is enabled check for HDCP status
    ------------------------------------------------------------------------------
    */
    if(pNegotiatedCap->content_protection_config.content_protection_ake_port)
    {
        if(IS_DRM_STATE_ACQUIRING)
        {
            unsigned long nTime = 0;
            unsigned long nStartTime = 0;
            MM_Time_GetTime(&nStartTime);
            nTime = nStartTime;

            /**-----------------------------------------------------------------
              If HDCP Rights are being acquired, then wait for upto <9 seconds
            --------------------------------------------------------------------
            */
            while(!IS_DRM_STATE_ACQUIRED && nTime - nStartTime < 8000)
            {
                getHDCPStatus(NULL,
                              pNegotiatedCap->content_protection_config.
                              content_protection_ake_port);
                if(!IS_DRM_STATE_ACQUIRED)
                {
                    WFDMMLOGE("WFDMMSink SetupMMSession No HDCP Rights.. Fail");
                }
                MM_Timer_Sleep(100);
                MM_Time_GetTime(&nTime);
            }
        }
    }

    if(!IS_DRM_STATE_ACQUIRED)
    {
        teardownHDCPSession();
    }
    else if(m_pHDCPManager)
    {
        char *msg = (char*)SINK_MALLOC(sizeof(int)+1);
        if(msg != NULL)
        {
         snprintf(msg,sizeof(int),"%d",m_pNegotiatedCap->audio_method);
         WFDMMLOGE1("WFD prepared Audio Play %s",msg);
         m_pHDCPManager->constructCodecAndStreamType(SINK_AUDIO_TRACK_ID,msg);
         memset(msg,0,sizeof(int)+1);
         snprintf(msg,sizeof(int),"%d",m_pNegotiatedCap->video_method);
         WFDMMLOGE1("WFD prepared Video Play %s",msg);
         m_pHDCPManager->constructCodecAndStreamType(SINK_VIDEO_TRACK_ID,msg);
        }
    }

    WFDMMLOGH("WFDMMSink::setupMMSession... Done");

    CRITICAL_SECT_LEAVE;

    RETURNSUCCESS;
}

/*==============================================================================

         FUNCTION:          createDataSource

         DESCRIPTION:
*//**       @brief           Creates the RTP Stream port module
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
bool WFDMMSink::createDataSource()
{

    if(!m_pNegotiatedCap)
    {
        return false;
    }

    if(m_pNegotiatedCap->transport_capability_config.eRtpPortType ==
                                                                   RTP_PORT_TCP)
    {
        if(m_pRTPStreamPort)
        {
            WFDMMLOGE("Using the existing streamport in case of TCP.");
            m_pRTPStreamPort->updateRTPPortVars();
            return true;
        }
    }
    else
    {
        if(m_pRTPStreamPort)
        {
            WFDMMLOGE("Destroy streamport in case of UDP");
            destroyDataSource();
        }
    }

    mediaSourceConfigType mCfg;

    mCfg.rtpPort = m_pNegotiatedCap->
                         transport_capability_config.port1_id;
    mCfg.rtcpPortLocal = mCfg.rtpPort + 1;

    mCfg.bIsTCP   =
          (m_pNegotiatedCap->transport_capability_config.eRtpPortType
                     == RTP_PORT_TCP) ? true: false;

    mCfg.nRtpSock = m_pNegotiatedCap->transport_capability_config.rtpSock;
    mCfg.nRtcpSock = m_pNegotiatedCap->transport_capability_config.rtcpSock;

    if(mCfg.rtpPort == 0)
    {
        /*----------------------------------------------------------------------
          Invalid Port Number
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("Invalid RTP Port number");
        return false;
    }
    WFDMMLOGH1("Media Source RTP Port Num %u", mCfg.rtpPort);
    WFDMMLOGH2("Media Source RTP Socket Pair %d %d", mCfg.nRtpSock,
                mCfg.nRtcpSock);

    /*--------------------------------------------------------------------------
     Create a socket pair to send to RTP module
    ----------------------------------------------------------------------------
    */

    m_pRTPStreamPort = MM_New_Args(RTPStreamPort,
                   (mCfg.rtpPort, mCfg.bIsTCP, mCfg.nRtpSock, mCfg.nRtcpSock));

   if(!m_pRTPStreamPort)
    {
        WFDMMLOGE("Cant create RTP Decoder");
        return false;
    }

    return true;
}

/*==============================================================================

         FUNCTION:          createMediaSource

         DESCRIPTION:
*//**       @brief           Creates the Media  Source Module
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
bool WFDMMSink::createMediaSource()
{
    if(!m_pNegotiatedCap)
    {
        return false;
    }

    mediaSourceConfigType sConfig;

    sConfig.eAudioFmt = m_pNegotiatedCap->audio_method;

    sConfig.bHasAudio = m_pNegotiatedCap->audio_method == WFD_AUDIO_LPCM ||
                        m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC ?
                        true: false;
    sConfig.bHasVideo = m_pNegotiatedCap->video_method == WFD_VIDEO_H264 ?
                        true: false;
    sConfig.nPeerIP   = m_pNegotiatedCap->peer_ip_addrs.ipv4_addr1;

    sConfig.rtpPort = m_pNegotiatedCap->
                         transport_capability_config.port1_id;
    sConfig.rtcpPortLocal = sConfig.rtpPort + 1;

    sConfig.bSecure   = IS_DRM_STATE_ACQUIRED ? true : false;

    sConfig.pFnDecrypt = IS_DRM_STATE_ACQUIRED ? decryptCb : NULL;

    sConfig.rtcpPortRemote =
                   m_pNegotiatedCap->transport_capability_config.port1_rtcp_id;
  //  sConfig.  = IS_DRM_STATE_ACQUIRED ? m_pNegotiatedCap->
  //                 content_protection_config.content_protection_ake_port : 0;
    sConfig.bIsTCP   =
          (m_pNegotiatedCap->transport_capability_config.eRtpPortType
                     == RTP_PORT_TCP) ? true: false;

    sConfig.nRtpSock = m_pNegotiatedCap->transport_capability_config.rtpSock;
    sConfig.nRtcpSock = m_pNegotiatedCap->transport_capability_config.rtcpSock;

    int nFrameDropMode = 0;
    getCfgItem(VIDEO_PKTLOSS_FRAME_DROP_MODE_KEY, &nFrameDropMode);

    sConfig.nFrameDropMode = nFrameDropMode;

    sConfig.pRTPStreamPort = m_pRTPStreamPort;

    m_pMediaSrc = MM_New_Args(WFDMMSinkMediaSource,
                             (SINK_MODULE_MEDIASOURCE,
                              eventHandlerCb, FBD,avInfoCb,this));

    if(!m_pMediaSrc)
    {
        WFDMMLOGE("Failed to create Media Source");
        return false;
    }

    if(m_pMediaSrc->Configure(&sConfig) != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to configure Media Source");
        return false;
    }
    return true;
}

/*==============================================================================

         FUNCTION:           createAudioDecoder

         DESCRIPTION:        creates AudioDecoder Instance. Place holder for now
*//**       @brief
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
bool WFDMMSink::createAudioDecoder()
{
#ifndef USE_OMX_AAC_CODEC
    return true;
#else

    if(m_pNegotiatedCap->audio_method == WFD_AUDIO_LPCM ||
       m_pNegotiatedCap->audio_method == WFD_AUDIO_INVALID ||
       m_pNegotiatedCap->audio_method == WFD_AUDIO_UNK)
    {
        return true;
    }

    audioConfigType sConfig;

    if(m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC)
    {
        getAACAudioParams(m_pNegotiatedCap->audio_config.aac_codec.
                          supported_modes_bitmap);
    }
    else if(m_pNegotiatedCap->audio_method == WFD_AUDIO_DOLBY_DIGITAL)
    {
        getAC3AudioParams(m_pNegotiatedCap->audio_config.dolby_digital_codec
                          .supported_modes_bitmap);
    }

    sConfig.eAudioType   = m_pNegotiatedCap->audio_method;
    sConfig.nChannels    = m_nAudioChannels? m_nAudioChannels : 2;
    sConfig.nSampleRate  = m_nAudioSampleRate?
                                     m_nAudioSampleRate : 48000;


    m_pAudioDecode = MM_New_Args(WFDMMSinkAudioDecode, (
                                  SINK_MODULE_AUDIO_DECODER,
                                  EBD,
                                  FBD,
                                  eventHandlerCb,
                                  (int)this));

    if(!m_pAudioDecode)
    {
        WFDMMLOGE("Failed to create Audio Decoder");
        return false;
    }

    if(m_pAudioDecode->Configure(&sConfig) != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to configure Audio Decoder");
        return false;
    }

    return true;

#endif
}

/*==============================================================================

         FUNCTION:          createVideoDecoder

         DESCRIPTION:
*//**       @brief          creates Video Decoder Module
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
bool WFDMMSink::createVideoDecoder()
{
    if(!m_pNegotiatedCap)
    {
        return false;
    }

    if(m_pNegotiatedCap->video_method == WFD_VIDEO_UNK ||
       m_pNegotiatedCap->video_method == WFD_VIDEO_INVALID)
    {
        WFDMMLOGH("Audio Only Session");
        return true;
    }

    m_pVideoDecode = MM_New_Args(WFDMMSinkVideoDecode,
                                 (SINK_MODULE_VIDEO_DECODER,
                                  EBD,
                                  FBD,
                                  eventHandlerCb,
                                  this));

    if(!m_pVideoDecode)
    {
        WFDMMLOGE("Failed to create Video Decoder");
        return false;
    }

    videoConfigType sConfig;

    sConfig.bSecure = IS_DRM_STATE_ACQUIRED ? true : false;

    sConfig.eVideoType = m_pNegotiatedCap->video_method;

    uint32 ceaMode = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_cea_mode;
    uint32 vesaMode = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_vesa_mode;
    uint32 hhMode   = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_hh_mode;
    uint32 nFrameWidth = 0;
    uint32 nFrameHeight = 0;
    uint32 nFrameRate = 0;

    (void)getFrameResolutionRefreshRate(ceaMode,
                                vesaMode,
                                hhMode,
                                &nFrameWidth,
                                &nFrameHeight,
                                &nFrameRate);

    if(nFrameWidth == 0 || nFrameHeight == 0)
    {
        WFDMMLOGE("Invalid Height or Width");
        return false;
    }

    sConfig.nFrameHeight = 1080;
    sConfig.nFrameWidth  = 1920;

    int maxSupportedFps = 0;
    int ret = getCfgItem(MAX_FPS_SUPPORTED_KEY,&maxSupportedFps);

    if(ret < 0)
        sConfig.nFps = 0;
    else
        sConfig.nFps = maxSupportedFps;

    sConfig.pWindow      = m_pWindow.get();

    if(m_pVideoDecode->Configure(&sConfig) != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to configure video decoder");
        return false;
    }
    return true;
}

/*==============================================================================

         FUNCTION:          createSinkStatistics

         DESCRIPTION:
*//**       @brief          creates Sink Statistics Module
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
bool WFDMMSink::createSinkStatistics()
{
    m_pSinkStatistics = WFDMMSinkStatistics::CreateInstance();
    if(m_pSinkStatistics)
        return true;
    else
    {
        WFDMMLOGE("WFDMMSink: Failed to create sink statistics module");
        return false;
    }
}

/*==============================================================================

         FUNCTION:          createRenderer

         DESCRIPTION:
*//**       @brief          creates Renderer Module
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
bool WFDMMSink::createRenderer()
{
    rendererConfigType sConfig;

    if(m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC)
    {
        getAACAudioParams(m_pNegotiatedCap->audio_config.aac_codec.
                          supported_modes_bitmap);
    }
    else if(m_pNegotiatedCap->audio_method == WFD_AUDIO_DOLBY_DIGITAL)
    {
        getAC3AudioParams(m_pNegotiatedCap->audio_config.dolby_digital_codec
                          .supported_modes_bitmap);
    }
#ifdef USE_OMX_AAC_CODEC
    if(m_pNegotiatedCap->audio_method != WFD_AUDIO_INVALID &&
       m_pNegotiatedCap->audio_method != WFD_AUDIO_UNK)
    {
        sConfig.sAudioInfo.eAudioFmt = WFD_AUDIO_LPCM;
    }
    else
#endif
    sConfig.sAudioInfo.eAudioFmt   = m_pNegotiatedCap->audio_method;

    sConfig.sAudioInfo.nChannels   = m_nAudioChannels? m_nAudioChannels : 2;
    sConfig.sAudioInfo.nSampleRate = m_nAudioSampleRate?
                                     m_nAudioSampleRate : 48000;
    sConfig.sAudioInfo.nFrameLength = 480;

    uint32 ceaMode = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_cea_mode;
    uint32 vesaMode = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_vesa_mode;
    uint32 hhMode   = m_pNegotiatedCap->video_config.video_config.
                                          h264_codec->supported_hh_mode;
    uint32 nFrameWidth = 0;
    uint32 nFrameHeight = 0;
    uint32 nFrameRate = 0;

    (void)getFrameResolutionRefreshRate(ceaMode,
                                vesaMode,
                                hhMode,
                                &nFrameWidth,
                                &nFrameHeight,
                                &nFrameRate);



    sConfig.sVideoInfo.nFrameHeight = nFrameHeight;
    sConfig.sVideoInfo.nFrameWidth = nFrameWidth;

    sConfig.sVideoInfo.nFrameRate = nFrameRate;

    int nFrameDropMode = 0;
    getCfgItem(VIDEO_PKTLOSS_FRAME_DROP_MODE_KEY, &nFrameDropMode);

    sConfig.sVideoInfo.nFrameDropMode = nFrameDropMode;

    sConfig.bHasAudio = m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC ||
                        m_pNegotiatedCap->audio_method == WFD_AUDIO_LPCM ?
                        true: false;

    sConfig.bHasVideo = m_pNegotiatedCap->video_method == WFD_VIDEO_H264 ?
                        true: false;

    sConfig.pWindow = m_pWindow.get();

    sConfig.nDecoderLatency = m_pNegotiatedCap->decoder_latency;

    int nAVSyncMode = 0;

    getCfgItem(DISABLE_AVSYNC_MODE_KEY, &nAVSyncMode);

    sConfig.bAVSyncMode = (nAVSyncMode == 0 ? TRUE : FALSE);
    WFDMMLOGH1("AVSync Mode = %d",sConfig.bAVSyncMode);

    m_pRenderer = MM_New_Args(WFDMMSinkRenderer, (
                                SINK_MODULE_RENDERER,
                                EBD,
                                eventHandlerCb,
                                this));

    int nAudioTrackLatencyMode = 0;

    getCfgItem(ENABLE_AUDIO_TRACK_LATENCY_MODE_KEY, &nAudioTrackLatencyMode);

    sConfig.bAudioTrackLatencyMode = (nAudioTrackLatencyMode == 1 ? TRUE : FALSE);
    WFDMMLOGH1("AudioTrackLatencyMode Mode = %d",sConfig.bAudioTrackLatencyMode);

    int32 audioAVSyncDropWindow = 0;
    getCfgItem(AUDIO_AVSYNC_DROP_WINDOW_KEY,&audioAVSyncDropWindow);
    sConfig.naudioAVSyncDropWindow = audioAVSyncDropWindow;
    if(sConfig.naudioAVSyncDropWindow == 0)
    {
       WFDMMLOGH("Default Audio AV Sync drop window");
       sConfig.naudioAVSyncDropWindow = AUDIO_AV_SYNC_DROP_WINDOW;
    }
    WFDMMLOGH1("Audio AV Sync drop window = %d",sConfig.naudioAVSyncDropWindow);

    int32 videoAVSyncDropWindow = 0;
    getCfgItem(VIDEO_AVSYNC_DROP_WINDOW_KEY,&videoAVSyncDropWindow);
    sConfig.nvideoAVSyncDropWindow = videoAVSyncDropWindow;
    if(sConfig.nvideoAVSyncDropWindow == 0)
    {
      WFDMMLOGH("Default Video AV Sync drop window");
      sConfig.nvideoAVSyncDropWindow = VIDEO_AV_SYNC_DROP_WINDOW;
    }
    WFDMMLOGH1("Video AV Sync drop window = %d",sConfig.nvideoAVSyncDropWindow);

    if(!m_pRenderer)
    {
        WFDMMLOGE("Failed to create WFD Renderer");
        return false;
    }

    if(m_pRenderer->Configure(&sConfig) != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to configure Renderer");
        return false;
    }

    return true;
}

/*==============================================================================

         FUNCTION:          destroyDataSource

         DESCRIPTION:
*//**       @brief          destroys Stream Port instance
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
bool WFDMMSink::destroyDataSource()
{
    /*--------------------------------------------------------------------------
      destroyDataSource() is called as part of DEINIT and in case of TCP, we do
      not destroy the streamport when this is called during BGFG usecase. This
      check is required to keep the TCP connection intact when DEINIT is called
      as part of BGFG usecase
    ----------------------------------------------------------------------------
    */
    if(m_pNegotiatedCap &&
       m_pNegotiatedCap->transport_capability_config.eRtpPortType ==
                                                               RTP_PORT_UDP)
    {
        WFDMMLOGE("Destroying stream port in case of UDP");
        if(m_pRTPStreamPort)
        {
            MM_Delete(m_pRTPStreamPort);
            m_pRTPStreamPort = NULL;
        }
    }
    return true;
}

/*==============================================================================

         FUNCTION:          destroyMediaSource

         DESCRIPTION:
*//**       @brief          destroys Media Source instance
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
bool WFDMMSink::destroyMediaSource()
{
    if(m_pMediaSrc)
    {
        MM_Delete(m_pMediaSrc);
        m_pMediaSrc = NULL;
    }
    return true;
}

/*==============================================================================

         FUNCTION:          destroyAudioDecoder

         DESCRIPTION:
*//**       @brief          destroys Audio Decoder module. Place holder
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
bool WFDMMSink::destroyAudioDecoder()
{
#ifdef USE_OMX_AAC_CODEC
    if(m_pAudioDecode)
    {
        MM_Delete(m_pAudioDecode);
        m_pAudioDecode = NULL;
    }
#endif
    return true;
}


/*==============================================================================

         FUNCTION:         destroyVideoDecoder

         DESCRIPTION:
*//**       @brief         destroys Video decoder wraper
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
bool WFDMMSink::destroyVideoDecoder()
{
    if(m_pVideoDecode)
    {
        MM_Delete(m_pVideoDecode);
        m_pVideoDecode = NULL;
    }
    return true;
}


/*==============================================================================

         FUNCTION:          destroyRenderer

         DESCRIPTION:
*//**       @brief          Destroys Renderer Instance
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
bool WFDMMSink::destroyRenderer()
{
    if(m_pRenderer)
    {
        MM_Delete(m_pRenderer);
        m_pRenderer = NULL;
    }
    return true;
}

/*==============================================================================

         FUNCTION:          destroySinkStatistics

         DESCRIPTION:
*//**       @brief          Destroys SinkStatistics instance
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
bool WFDMMSink::destroySinkStatistics()
{
    /*--------------------------------------------------------------------------
      Display the statistics before deleting the instance!
     ---------------------------------------------------------------------------
    */
    m_pSinkStatistics = WFDMMSinkStatistics::CreateInstance();
    if(m_pSinkStatistics)
    {
        m_pSinkStatistics->PrintStatistics();
        WFDMMSinkStatistics::DeleteInstance();
        m_pSinkStatistics = NULL;
        return true;
    }
    return false;
}

/*==============================================================================

         FUNCTION:         teardownMMSession

         DESCRIPTION:
*//**       @brief         tears down MM session.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::teardownMMSession()
{
    CRITICAL_SECT_ENTER;
    WFDMMLOGH("WFDMMSink Teardown MM Session");
/*
    if(!validateStateChange(WFD_SINK_MM_STATE_ZERO))
    {
        CRITICAL_SECT_LEAVE;
        RETURNBADSTATE;
    }
*/
    m_bMMTearingDown = true;
    if(ExecuteCommandSync(DEINIT) != true)
    {
        WFDMMLOGE("Failed to deinitialize multimedia");
        CRITICAL_SECT_LEAVE;
        RETURNBADSTATE;
    }

    /*--------------------------------------------------------------------------
      All multimedia components have been deinitialized and destroyed as part
      of DEINIT called above. It is now safe here to destroy StreamPort as
      WFD session is now tearing down.
    ----------------------------------------------------------------------------
    */
    if(m_pRTPStreamPort)
    {
        MM_Delete(m_pRTPStreamPort);
        m_pRTPStreamPort = NULL;
    }

    if(m_pNegotiatedCap)
    {
        SINK_FREEIF(m_pNegotiatedCap->video_config.video_config.h264_codec);
    }

    SINK_FREEIF(m_pNegotiatedCap);

    m_bMMTearingDown = false;
    RESET_MM_ERROR;
    WFDMMLOGH("WFDMMSink Teardown MM Session... Done");
    CRITICAL_SECT_LEAVE;
    RETURNSUCCESS;
}

/*==============================================================================

         FUNCTION:         play

         DESCRIPTION:
*//**       @brief         starts playing the content.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         av_select - which stream to play
                           pCallback - Callback to notify play status


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::play( WFD_AV_select_t av_select,
                              wfd_mm_stream_play_cb pCallback)
{
    (void) av_select;
    WFDMMLOGH("WFDMMSink Play");


    CHECK_ERROR_STATUS;

    if(IS_MM_STATE_PLAY)
    {
        if(pCallback)
        {
            pCallback(this, WFD_STATUS_SUCCESS);
        }

        RETURNSUCCESS;
    }


    if(ExecuteCommandSync(INIT) != true)
    {
        WFDMMLOGE("Play failed at INIT");
        if(pCallback)
        {
            pCallback( this, WFD_STATUS_FAIL);
        }
        SET_MM_ERROR_FAIL;

        RETURNFAIL;
    }


    if(ExecuteCommandSync(PLAY) != true)
    {
        WFDMMLOGE("Play failed to move to PLAY State");
        if(pCallback)
        {
            pCallback( this, WFD_STATUS_FAIL);
        }
        SET_MM_ERROR_FAIL;

        RETURNFAIL;
    }

    if(m_nMMStatus == WFD_STATUS_SUCCESS)
    {
        if(pCallback)
        {
            pCallback( this, WFD_STATUS_SUCCESS);
        }
    }
    else
    {
        WFDMMLOGE("Sink Failed to move to Play");
        if(pCallback)
        {
            pCallback( this, WFD_STATUS_FAIL);
        }
        SET_MM_ERROR_FAIL;

        RETURNFAIL;
    }


    WFDMMLOGE("WFDMMSink Play Done");
    RETURNSUCCESS;
}

/*==============================================================================

         FUNCTION:         pause

         DESCRIPTION:
*//**       @brief         starts pausing the content.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         av_select - which stream to pause
                           pCallback - Callback to notify play status


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFD_status_t WFDMMSink::pause( WFD_AV_select_t av_select,
                              wfd_mm_stream_pause_cb pCallback)
{
    (void) av_select;
    WFDMMLOGH("WFDMMSink Pause");


    if(IS_MM_STATE_PAUSE || IS_MM_STATE_DEINIT)
    {
        if(pCallback)
        {
            WFDMMLOGH("Pause/Standby called in already paused state");
            pCallback( this, WFD_STATUS_SUCCESS);
        }

        RETURNSUCCESS;
    }

    WFDMMLOGD("Pause validate state change");

    if(!validateStateChange(WFD_SINK_MM_STATE_PAUSE))
    {
        RETURNBADSTATE;
    }


    WFDMMLOGD("Move to DeINIT");
    m_bMMTearingDown = true;
    if(ExecuteCommandSync(DEINIT) != true)
    {
        WFDMMLOGE("Failed to move to DEINIT state");
        if(pCallback)
        {
            pCallback( this, WFD_STATUS_FAIL);
        }
        RETURNFAIL;
    }
    m_bMMTearingDown = false;

    WFDMMLOGE("WFDMMSink Pause Done sending callback to Sessionmanager");
    /*---------------------------------------------------------------------
     ToDo:Need to check,why the callback is sent to session manager
    -----------------------------------------------------------------------
    */

    if(pCallback)
    {
        pCallback( this, WFD_STATUS_SUCCESS);
    }


    RETURNSUCCESS;
}

/*==============================================================================

         FUNCTION:         WFDMMSinkUpdateSession

         DESCRIPTION:
*//**       @brief         updates the session parameters for sink
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pNegCap..the negotiated capabilities of the session
...........................pUpdateSessionCb callback to session manager


*//*     RETURN VALUE:
*//**       @return
                           WFD_status_t

@par     SIDE EFFECTS:

*//*==========================================================================*/

WFD_status_t WFDMMSink::WFDMMSinkUpdateSession(WFD_MM_capability_t *pNegCap,
                                    wfd_mm_update_session_cb pUpdateSessionCb)
{
    (void) pUpdateSessionCb;
    CRITICAL_SECT_ENTER;
    WFDMMLOGE("WFDMMSink::WFDMMSinkUpdateSession called");
    CHECK_NULL_ERROR(m_pNegotiatedCap,"m_pNegotiatedCap is NULL");
    CHECK_NULL_ERROR(pNegCap,"pNegCap is NULL");

    if(m_pNegotiatedCap->transport_capability_config.eRtpPortType == RTP_PORT_TCP
        && pNegCap->transport_capability_config.eRtpPortType == RTP_PORT_TCP
        && pNegCap->pSurface != NULL)
    {
        WFDMMLOGE("Updated surface recvd in TCP. Move to DEINIT");
        m_bUpdatedSurface = true;
        m_bMMTearingDown = true;
        ExecuteCommandSync(DEINIT);
        m_bMMTearingDown = false;
    }

    if(!IS_MM_STATE_DEINIT && pNegCap->pSurface != NULL)
    {
        WFDMMLOGE("Cannot update session in play");
        CRITICAL_SECT_LEAVE;
        return WFD_STATUS_FAIL;
    }

    if (m_pNegotiatedCap->pSurface != pNegCap->pSurface)
    {
        m_pNegotiatedCap->pSurface = pNegCap->pSurface;
        WFDMMLOGE("WFDMMSink: Updating Sink Surface");
        if(m_pNegotiatedCap->pSurface == NULL)
        {
            WFDMMLOGE("WFDMMSink: Received Sink Surface as NULL");
        }
    }

    m_pNegotiatedCap->transport_capability_config.eRtpPortType =
                 pNegCap->transport_capability_config.eRtpPortType;

    CRITICAL_SECT_LEAVE;
    RETURNSUCCESS;
}


/*==============================================================================

         FUNCTION:         validateStateChange

         DESCRIPTION:
*//**       @brief         Validate a given state change
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         WFDSinkMMStateType


*//*     RETURN VALUE:
*//**       @return
                           bool

@par     SIDE EFFECTS:

*//*==========================================================================*/
bool WFDMMSink::validateStateChange
(
    WFDSinkMMStateType eState
)
{
    switch(eState)
    {
        case WFD_SINK_MM_STATE_PLAY:
            if(m_eMMState != WFD_SINK_MM_STATE_INIT &&
               m_eMMState != WFD_SINK_MM_STATE_PAUSE)
            {
                return false;
            }
            break;
        case WFD_SINK_MM_STATE_PAUSE:
            if(m_eMMState != WFD_SINK_MM_STATE_PLAY)
            {
                return false;
            }
            break;
        case WFD_SINK_MM_STATE_ZERO:
            if(m_eMMState == WFD_SINK_MM_STATE_ZERO)
            {
                return false;
            }
            break;
        case WFD_SINK_MM_STATE_DEINIT:
            if(m_eMMState == WFD_SINK_MM_STATE_DEINIT ||
               m_eMMState == WFD_SINK_MM_STATE_ZERO)
            {
                return false;
            }
            break;
        default:
            return false;
            break;
    }
    return true;
}

/*==============================================================================

         FUNCTION:         getFrameResolutionRefreshRate

         DESCRIPTION:
*//**       @brief          Decodes frame dimensions and framerates from
                            CEA VESA and HH modes
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
void WFDMMSink::getFrameResolutionRefreshRate(uint32 cea_mode,
                                           uint32 vesa_mode,
                                           uint32 hh_mode,
                                           uint32 *nFrameWidth,
                                           uint32 *nFrameHeight,
                                           uint32 *nFramerate
                                           )
{
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::getFrameResolutionRefreshRate %d %d %d",
        cea_mode, vesa_mode, hh_mode);

    if(cea_mode)
    {
        switch (cea_mode)
        {
        case 1:
            *nFrameWidth = 640;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;
        case 2:
            *nFrameWidth = 720;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;
        case 4:
            //480i
            *nFrameWidth = 720;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;
        case 8:
            *nFrameWidth = 720;
            *nFrameHeight = 576;
            *nFramerate = 50;
            break;
        case 16:
            //576i
            *nFrameWidth = 720;
            *nFrameHeight = 576;
            *nFramerate = 50;
            break;
        case 32:
            *nFrameWidth = 1280;
            *nFrameHeight = 720;
            *nFramerate = 30;
            break;
        case 64:
            *nFrameWidth = 1280;
            *nFrameHeight = 720;
            *nFramerate = 60;
            break;
        case 128:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 30;
            break;
        case 256:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 60;
            break;
        case 512:
            //1080i 60
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 60;
            break;
        case 1024:
            *nFrameWidth = 1280;
            *nFrameHeight = 720;
            *nFramerate = 25;
            break;
        case 2048:
            *nFrameWidth = 1280;
            *nFrameHeight = 720;
            *nFramerate = 50;
            break;
        case 4096:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 25;
            break;
        case 8192:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 50;
            break;
        case 16384:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 50;
            break;
        case 32768:
            *nFrameWidth = 1280;
            *nFrameHeight = 720;
            *nFramerate = 24;
            break;
        case 65536:
            *nFrameWidth = 1920;
            *nFrameHeight = 1080;
            *nFramerate = 24;
            break;
        default:
            *nFrameWidth = 800;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        }

    }
    else if(hh_mode)
    {
        switch (hh_mode)
        {
        case 1:
            *nFrameWidth = 800;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        case 2:
            *nFrameWidth = 800;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;
        case 4:
            *nFrameWidth = 854;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        case 8:
            *nFrameWidth = 854;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;

        case 16:
            *nFrameWidth = 864;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        case 32:
            *nFrameWidth = 864;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;
        case 64:
            *nFrameWidth = 640;
            *nFrameHeight = 360;
            *nFramerate = 30;
            break;
        case 128:
            *nFrameWidth = 640;
            *nFrameHeight = 360;
            *nFramerate = 60;
            break;
        case 256:
            *nFrameWidth = 960;
            *nFrameHeight = 540;
            *nFramerate = 30;
            break;
        case 512:
            *nFrameWidth = 960;
            *nFrameHeight = 540;
            *nFramerate = 60;
            break;
        case 1024:
            *nFrameWidth = 848;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        case 2048:
            *nFrameWidth = 848;
            *nFrameHeight = 480;
            *nFramerate = 60;
            break;

        default:
            *nFrameWidth = 800;
            *nFrameHeight = 480;
            *nFramerate = 30;
            break;
        }
    }
    else if(vesa_mode)
    {
        switch (vesa_mode)
        {
        case 1:
            *nFrameWidth = 800;
            *nFrameHeight = 600;
            *nFramerate = 30;
            break;
        case 2:
            *nFrameWidth = 800;
            *nFrameHeight = 600;
            *nFramerate = 60;
            break;
        case 4:
            *nFrameWidth = 1024;
            *nFrameHeight = 768;
            *nFramerate = 30;
            break;
        case 8:
            *nFrameWidth = 1024;
            *nFrameHeight = 768;
            *nFramerate = 60;
            break;
        case 16:
            *nFrameWidth = 1152;
            *nFrameHeight = 864;
            *nFramerate = 30;
            break;
        case 32:
            *nFrameWidth = 1152;
            *nFrameHeight = 864;
            *nFramerate = 60;
            break;
        case 64:
            *nFrameWidth = 1280;
            *nFrameHeight = 768;
            *nFramerate = 30;
            break;
        case 128:
            *nFrameWidth = 1280;
            *nFrameHeight = 768;
            *nFramerate = 60;
            break;
        case 256:
            *nFrameWidth = 1280;
            *nFrameHeight = 800;
            *nFramerate = 30;
            break;

        case 512:
            *nFrameWidth = 1280;
            *nFrameHeight = 800;
            *nFramerate = 60;
            break;
        case 1024:
            *nFrameWidth = 1360;
            *nFrameHeight = 768;
            *nFramerate = 30;
            break;
        case 2048:
            *nFrameWidth = 1360;
            *nFrameHeight = 768;
            *nFramerate = 60;
            break;
        case 4096:
            *nFrameWidth = 1366;
            *nFrameHeight = 768;
            *nFramerate = 30;
            break;
        case 8192:
            *nFrameWidth = 1366;
            *nFrameHeight = 768;
            *nFramerate = 60;
            break;
        case 16384:
            *nFrameWidth = 1280;
            *nFrameHeight = 1024;
            *nFramerate = 30;
            break;
        case 32768:
            *nFrameWidth = 1280;
            *nFrameHeight = 1024;
            *nFramerate = 60;
            break;
        case 65536:
            *nFrameWidth = 1400;
            *nFrameHeight = 1050;
            *nFramerate = 30;
            break;

        default:
            *nFrameWidth = 800;
            *nFrameHeight = 600;
            *nFramerate = 30;
            break;
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::No Modes are available setting  default values");
        *nFrameWidth = 640;
        *nFrameHeight = 480;
        *nFramerate = 60;
    }
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
void WFDMMSink::FBD(void* handle, int moduleIdx, int trackID,
                             OMX_BUFFERHEADERTYPE *pBuffer)
{
    if(handle && pBuffer)
    {
        WFDMMSink *pMe = (WFDMMSink*)handle;

        pMe->processFBD(moduleIdx, trackID, pBuffer);

    }
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
void WFDMMSink::processFBD(int moduleIdx, int trackID,
                           OMX_BUFFERHEADERTYPE *pBuffer)
{

    CRITICAL_SECT_ENTER_FBD

    switch(moduleIdx)
    {
    case SINK_MODULE_MEDIASOURCE:
        if(!IS_MM_STATE_PLAY)
        {
            WFDMMLOGE("Not in PLAY state give buffer back to MediaSrc");
            m_pMediaSrc->setFreeBuffer(trackID,
                                       pBuffer);
            break;
        }
        if(trackID == SINK_VIDEO_TRACK_ID)
        {
            if(!m_pVideoDecode ||
                m_pVideoDecode->DeliverInput(pBuffer) != OMX_ErrorNone)
            {
                /*--------------------------------------------------------------
                 In the event that deliverInput fails, make sure to send the
                 Buffer back to MediaSource. At the end of the session, media
                 source is where all buffers must be
                ----------------------------------------------------------------
                */
                WFDMMLOGE("Video Decoder not avail. Push Back");
                m_pMediaSrc->setFreeBuffer(trackID,
                                       pBuffer);
            }

        }
        else if(trackID == SINK_AUDIO_TRACK_ID)
        {
#ifdef USE_OMX_AAC_CODEC
            if(m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC)
            {
                if(!m_pAudioDecode ||
                    m_pAudioDecode->DeliverInput(pBuffer) != OMX_ErrorNone)
                {
                    /*----------------------------------------------------------
                     If DeliverInput on Renderer fails return the buffer back to
                     Media Source
                    ------------------------------------------------------------
                    */
                    WFDMMLOGH("Deliver Audio To AudioDecode Failed");
                    m_pMediaSrc->setFreeBuffer(trackID,
                                               pBuffer);
                }
            }
            else
#endif
            if(!m_pRenderer ||
                m_pRenderer->DeliverInput(trackID,pBuffer) != OMX_ErrorNone)
            {
                /*--------------------------------------------------------------
                 If DeliverInput on Renderer fails return the buffer back to
                 Media Source
                ----------------------------------------------------------------
                */
                WFDMMLOGH("Deliver Audio To Renderer Failed");
                m_pMediaSrc->setFreeBuffer(trackID,
                                           pBuffer);
            }
        }
        break;
    case SINK_MODULE_VIDEO_DECODER:
        WFDMMLOGD("VideoDecoder FDB Push to Renderer");

        if(pBuffer && pBuffer->nFilledLen)
        {
            if((pBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME))
            {
                /*--------------------------------------------------------------
                 Every IFrame information that is recvd from decoder is supplied
                 to mediasource so that it can take necessary decisions.
                 ---------------------------------------------------------------
                */
                WFDMMLOGD("FrameInfo: IFrame. Notify MediaSrc");
                m_pMediaSrc->videoFrameBufferInfo(FRAME_INFO_I_FRAME);
                m_bNotifyFirstFrameInfo = false;
            }
            else if(m_bNotifyFirstFrameInfo)
            {
                /*--------------------------------------------------------------
                 Convey only first PFrame info (after session start/subsequent
                 resumptions) to mediasource
                ----------------------------------------------------------------
                */
                m_bNotifyFirstFrameInfo = false;
                WFDMMLOGD("First FrameInfo: Not an IFrame. Notify MediaSrc");
                m_pMediaSrc->videoFrameBufferInfo(FRAME_INFO_P_FRAME);
            }
        }

        if(m_bPendingIDRReq && pBuffer &&
           (pBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME) &&
           !(pBuffer->nFlags & OMX_BUFFERFLAG_DATACORRUPT))
        {
            /*------------------------------------------------------------------
              Now that we have received IDR we can reset the pending request
            --------------------------------------------------------------------
            */
            WFDMMLOGD("IDRReq Found IDR cancel Pending");
            m_bPendingIDRReq = false;
        }

        if(pBuffer && pBuffer->nFlags & OMX_BUFFERFLAG_STARTTIME)
        {
            m_nFrameCntDownStartTime = 0;
        }
        if(m_nFrameCntDownStartTime)
        {
            m_nFrameCntDownStartTime--;
            if(!m_nFrameCntDownStartTime && pBuffer)
            {
                pBuffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
            }
        }

        if(!m_pRenderer ||
            m_pRenderer->DeliverInput(trackID, pBuffer) != OMX_ErrorNone)
        {
            /*------------------------------------------------------------------
             If we are not able to push the frame to rendere we need to return
             the buffer to video decoder at all cost. Failing to do so may
             cause stop sequence to get stuck
            --------------------------------------------------------------------
            */
            WFDMMLOGE("Failed to Q Video Bufer to Renderer");
            m_pVideoDecode->SetFreeBuffer(pBuffer);
        }
        WFDMMLOGD("VideoDecoder FDB Pushed to Renderer");

        break;
    case SINK_MODULE_AUDIO_DECODER:
#ifdef USE_OMX_AAC_CODEC
        if(!IS_MM_STATE_PLAY || !m_pRenderer)
        {
            if(m_pAudioDecode)
            {
                m_pAudioDecode->SetFreeBuffer(pBuffer);
            }
        }
        if(!m_pRenderer ||
            m_pRenderer->DeliverInput(trackID, pBuffer) != OMX_ErrorNone)
        {
            /*------------------------------------------------------------------
             If we are not able to push the frame to rendere we need to return
             the buffer to video decoder at all cost. Failing to do so may
             cause stop sequence to get stuck
            --------------------------------------------------------------------
            */
            WFDMMLOGE("Failed to Q Audio Bufer to Renderer");
            if(m_pAudioDecode)
            {
                m_pAudioDecode->SetFreeBuffer(pBuffer);
            }
        }
        WFDMMLOGD("AudioDecoder FDB Pushed to Renderer");
#endif
        break;
    case SINK_MODULE_SELF:
        WFDMMLOGE("WFDMMSink calling FBD");
        break;
    case SINK_MODULE_RENDERER:
    default:
        /*----------------------------------------------------------------------
         renderer you cant call this.
        ------------------------------------------------------------------------
        */
        break;
    }
    CRITICAL_SECT_LEAVE_FBD
}

/*==============================================================================

         FUNCTION:          EBD

         DESCRIPTION:       Empty Buffer Done callback from various modules
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
void WFDMMSink::EBD(void* handle, int moduleIdx, int trackID,
                    OMX_BUFFERHEADERTYPE *pBuffer)
{
    if(handle && pBuffer)
    {
        WFDMMSink *pMe = (WFDMMSink*)handle;

        pMe->processEBD(moduleIdx, trackID, pBuffer);
    }
}

/*==============================================================================

         FUNCTION:          processEBD

         DESCRIPTION:
*//**       @brief          processes Empty Buffer Done calls and send the
                            buffers to other modules for processing
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
void WFDMMSink::processEBD(int moduleIdx, int trackID,
                           OMX_BUFFERHEADERTYPE *pBuffer)
{

    CRITICAL_SECT_ENTER_EBD

    switch(moduleIdx)
    {
    case SINK_MODULE_RENDERER:
        if(trackID == SINK_VIDEO_TRACK_ID)
        {
            if(m_pVideoDecode)
            {
                m_pVideoDecode->SetFreeBuffer(pBuffer);
            }
            else
            {
                WFDMMLOGE("Decoder is null. Cant push buffer");
            }
        }
        else if(trackID == SINK_AUDIO_TRACK_ID)
        {
#ifdef USE_OMX_AAC_CODEC
            if(m_pNegotiatedCap->audio_method == WFD_AUDIO_AAC)
            {
                if(m_pAudioDecode)
                {
                    m_pAudioDecode->SetFreeBuffer(pBuffer);
                }
            }
            else
#endif
            if(m_pMediaSrc)
            {
                m_pMediaSrc->setFreeBuffer(trackID, pBuffer);
            }
            else
            {
                WFDMMLOGE("m_pMediaSrc is null. Cant push buffer");
            }
        }
        break;
    case SINK_MODULE_VIDEO_DECODER:
        if(m_pMediaSrc)
        {
            /*------------------------------------------------------------------
               Temp Hack until video fix pass through of starttime
            --------------------------------------------------------------------
            */
            if(pBuffer->nFlags & OMX_BUFFERFLAG_STARTTIME)
            {
                m_nFrameCntDownStartTime = 2;
            }
            m_pMediaSrc->setFreeBuffer(SINK_VIDEO_TRACK_ID, pBuffer);
        }
        else
        {
            WFDMMLOGE("MediaSource is null. Push Buffer back to decoder");
        }
        break;
    case SINK_MODULE_AUDIO_DECODER:
        if(m_pMediaSrc)
        {
            m_pMediaSrc->setFreeBuffer(SINK_AUDIO_TRACK_ID, pBuffer);
        }
        else
        {
            WFDMMLOGE("MediaSource is null. Push Buffer back to decoder");
        }
        break;
    case SINK_MODULE_SELF:
        WFDMMLOGE("WFDMMSink Calling EBD at cleanup");
        break;
    case SINK_MODULE_MEDIASOURCE:
        /*----------------------------------------------------------------------
         mediasource cant call this.
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("Invalid EBD from mediasource");
    default:
        break;
    }
    CRITICAL_SECT_LEAVE_EBD
}


/*==============================================================================

         FUNCTION:         eventHandler

         DESCRIPTION:
*//**       @brief         Static function that handles events from source
                           modules and mux


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        pThis - this pointer
                          nModuleId - Id of the module reporting event
                          nEvent - Type of event
                          nStatus - status associated with event
                          nData  - More information about event


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSink::eventHandlerCb(void *pThis, OMX_U32 nModuleId,
                               WFDMMSinkEvent nEvent,
                               OMX_ERRORTYPE nStatus, int nData)
{
    if(!pThis)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "Invalid Me, can't handle device callback");
        return;
    }

    ((WFDMMSink *)pThis)->eventHandler(nModuleId,
                      nEvent, nStatus, nData);
    return;
}

/*==============================================================================

         FUNCTION:         eventHandler

         DESCRIPTION:
*//**       @brief         Handles events from source modules and mux


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        nModuleId - id of the module reporting event
                          nEvent - Type of event
                          nStatus - status associated with event
                          nData  - More information about event


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSink::eventHandler(OMX_U32 nModuleId,
                      WFDMMSinkEvent nEvent,
                      OMX_ERRORTYPE nStatus, int nData)
{
    (void)nStatus;
    switch(nEvent)
    {
    case WFDMMSINK_ERROR:
        if(nModuleId != SINK_MODULE_HDCP)
        {
            if(m_bMMTearingDown)
            {
                WFDMMLOGH("ErrorCallback. Already tearing down, Ignore");
                /*------------------------------------------------------------------
                 Set the flag MM tearing down to true to avoid multiple error
                 callbacks from flooding SM-A. The variable will be reset to false
                 when MM session is torn down
                --------------------------------------------------------------------
                */
                return;
            }
            m_bMMTearingDown = true;
        }
        WFDMMLOGH1("Error callback from module %ld", nModuleId);

        if(nModuleId == SINK_MODULE_AUDIO_DECODER)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO,
                                           WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == SINK_MODULE_VIDEO_DECODER)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_VIDEO,
                                           WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == SINK_MODULE_MEDIASOURCE)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_RTP,
                                           WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == SINK_MODULE_RENDERER)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_VIDEO,
                                           WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == SINK_MODULE_HDCP)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_HDCP,
                                           WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        break;
    case WFDMMSINK_PACKETLOSS:
        {
            CRITICAL_SECT_ENTER_EVT;
            sCallBacks.idr_cb(this);

            CRITICAL_SECT_LEAVE_EVT;
        }
        break;
    case WFDMMSINK_DECRYPT_FAILURE:
        {
            if(nModuleId == SINK_MODULE_MEDIASOURCE)
            {
              WFDMMLOGH("Decrypt Failure is Ignored");
            }
        }
        break;
    default:
        /*Handle an Unknown Event???? Give me a*/break; //TODO

    }

}


/*==============================================================================

         FUNCTION:          ExecuteCommandSync

         DESCRIPTION:
*//**       @brief          Executes commands synchronously. Blocks the caller
                            until done
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
bool WFDMMSink::ExecuteCommandSync(unsigned int command)
{
    CRITICAL_SECT_ENTER

    WFDMMLOGH("WFDMMSink ExecuteCOmmandSync");

    if(m_nMMStatus != WFD_STATUS_SUCCESS && command != DEINIT)
    {
        /*----------------------------------------------------------------------
         Already in an error
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("State change not allowed in Error");
        CRITICAL_SECT_LEAVE
        return false;
    }

    if(m_pCmdThread)
    {
        m_pCmdThread->SetSignal(command);
    }
    switch(command)
    {
    case DEINIT:
        while(m_eMMState != WFD_SINK_MM_STATE_DEINIT &&
               m_nMMStatus == WFD_STATUS_SUCCESS)
        {
            MM_Timer_Sleep(1);
        }
        break;
    case INIT:
        while(m_eMMState != WFD_SINK_MM_STATE_INIT &&
              m_nMMStatus == WFD_STATUS_SUCCESS)
        {
            MM_Timer_Sleep(1);
        }
        break;

    case PLAY:
        while(m_eMMState != WFD_SINK_MM_STATE_PLAY &&
              m_nMMStatus == WFD_STATUS_SUCCESS)
        {
            MM_Timer_Sleep(1);
        }
        break;
    case STOP:
        while(m_eMMState == WFD_SINK_MM_STATE_PLAY &&
              m_nMMStatus == WFD_STATUS_SUCCESS)
        {
            MM_Timer_Sleep(1);
        }
        break;
    default:
        WFDMMLOGE("Unknown command for state change");
    }

    if(m_nMMStatus != WFD_STATUS_SUCCESS)
    {
        WFDMMLOGE("State change command Failed");
        CRITICAL_SECT_LEAVE;
        return false;
    }
    CRITICAL_SECT_LEAVE
    return true;
}

/*==============================================================================

         FUNCTION:         decryptCb

         DESCRIPTION:
*//**       @brief          This functions decrypt HDCP encrypted data
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
bool WFDMMSink::decryptCb(void* handle, int streamType, int input, int output,
                          int size, char* pIV, int IVSize)
{
    if(!handle)
    {
        WFDMMLOGE("Decrypt call without a valid handle");
        return false;
    }

    WFDMMSink *pMe = (WFDMMSink *)handle;

    return pMe->decrypt(input, output, size, pIV, IVSize,
          streamType == SINK_AUDIO_TRACK_ID ? HDCP_AUDIO_TRACK_ID:
                                              HDCP_VIDEO_TRACK_ID);

}

/*==============================================================================

         FUNCTION:          decrypt

         DESCRIPTION:
*//**       @brief          Processes the decrypt call back and passed data
                            to HDCP library
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
bool WFDMMSink::decrypt(int input, int output,
                        int size, char* pIV, int IVSize, int nStreamId)
{
    if(!IS_DRM_STATE_ACQUIRED || !m_pHDCPManager)
    {
        WFDMMLOGE("WFDMMSink No Drm State Acquired");
        return false;
    }

    if(!input || !output || !size)
    {
        WFDMMLOGE("Invalid arguments in decrypt");
        return false;
    }


    unsigned char *pIVdata = IVSize ? (unsigned char*)pIV : NULL;

    unsigned long beforeDecrypt = 0, afterDecrypt = 0;
    MM_Time_GetTime(&beforeDecrypt);
    HDCPStatusType eRet = m_pHDCPManager->decrypt(pIVdata,
                              (unsigned char*)(int64)input,
                              (unsigned char*)(int64)output, size, nStreamId);
    if(eRet == HDCP_SUCCESS)
    {
        MM_Time_GetTime(&afterDecrypt);
        if(m_pSinkStatistics)
        {
            if(HDCP_VIDEO_TRACK_ID == nStreamId)
            {
                m_pSinkStatistics->SetVideoDecryptStatistics(
                uint64(afterDecrypt-beforeDecrypt));
            }
        }
        WFDMMLOGD("Decrypt Success");
        return true;
    }
    else if(eRet == HDCP_UNAUTHENTICATED_CONNECTION)
    {
        if(sCallBacks.update_event_cb)
        {
            sCallBacks.update_event_cb(WFD_EVENT_MM_HDCP,
                                       WFD_STATUS_RUNTIME_ERROR, NULL);
        }
    }

    WFDMMLOGD("Decrypt Fail");
    return false;
}

/*==============================================================================

         FUNCTION:          avInfoCb

         DESCRIPTION:
*//**       @brief          On idenitifying tracks in stream MediaSource sends
                            this callback. This also carries AV Sync start
                            time
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
void WFDMMSink::avInfoCb(void* handle, avInfoType *pInfo)
{
    WFDMMSink *pMe = (WFDMMSink*)handle;

    if(!pMe)
    {
        WFDMMLOGE("avInfo Cb Invalid Args");
        return;
    }

    return pMe->setAvInfo(pInfo);
}


/*==============================================================================

         FUNCTION:          setAvInfo

         DESCRIPTION:
*//**       @brief          Uses the AV info update from MediaSource to
                            configure other modules.
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
void WFDMMSink::setAvInfo(avInfoType *pInfo)
{
    if(m_pRenderer && pInfo)
    {
        WFDMMLOGH1("WFDMMSink Setting base time to renderer %llu",
                   pInfo->nBaseTime);
        m_pRenderer->SetMediaBaseTime(pInfo->nBaseTime);
        m_nActualBaseTime = pInfo->nBaseTimeStream;
    }
}

/*==============================================================================

         FUNCTION:         getAACAudioParams

         DESCRIPTION:
*//**       @brief         Get AAC Audio parameters


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        supported_audio_mode - supported Audio modes


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSink::getAACAudioParams(uint32 supported_audio_mode )
{
    WFDMMLOGH("WFDMMSource::getAACAudioParams");
    uint16 temp = (supported_audio_mode & 0X00FFF);
    switch (temp)
    {
    case 1:
        m_nAudioChannels = 2;
        m_nAudioSampleRate = 48000;
        break;
    case 2://Deprecated
        m_nAudioChannels = 4;
        m_nAudioSampleRate = 48000;
        break;
    case 4:
        m_nAudioChannels = 6;
        m_nAudioSampleRate = 48000;
        break;
    case 8:
        m_nAudioChannels = 8;
        m_nAudioSampleRate = 48000;
        break;
    default:
        m_nAudioChannels = 2;
        m_nAudioSampleRate = 48000;
        break;
    }
}

/*==============================================================================

         FUNCTION:         getAC3AudioParams

         DESCRIPTION:
*//**       @brief         Get AC3 Audio parameters


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        supported_audio_mode - supported Audio modes


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSink::getAC3AudioParams(uint32 supported_audio_mode )
{
    WFDMMLOGH("WFDMMSource::getAC3AudioParams");
    uint16 temp = (supported_audio_mode & 0X00FFF);
    switch (temp)
    {
    case 1:
        m_nAudioChannels = 6;
        m_nAudioSampleRate = 48000;
        break;
    case 2://Deprecated
        m_nAudioChannels = 4;
        m_nAudioSampleRate = 48000;
        break;
    case 4:
        m_nAudioChannels = 6;
        m_nAudioSampleRate = 48000;
        break;
    case 8:
        m_nAudioChannels = 8;
        m_nAudioSampleRate = 48000;
        break;
    default:
        m_nAudioChannels = 6;
        m_nAudioSampleRate = 48000;
        break;
    }
}

/*==============================================================================

         FUNCTION:          destroyMMComponents

         DESCRIPTION:
*//**       @brief          destroys Media Source instance,
                            Video decoder wraper,
                            Audio decoder module,
                            Renderer instance
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
bool WFDMMSink::destroyMMComponents()
{
    bool flag = destroyMediaSource();
    flag &= destroyVideoDecoder();
    flag &= destroyAudioDecoder();
    flag &= destroyRenderer();
    flag &= destroyDataSource();
    WFDMMLOGM1("WFDMMSource::destroyMMComponents flag = %d",flag);
    return flag;
}

/*==============================================================================

         FUNCTION:         WFDMMSinkAVControl

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
WFD_status_t WFDMMSink::WFDMMSinkAVControl
(
    WFD_MM_AV_Stream_Control_t control,
    int64 controlValue
)
{
    switch(control)
    {
    case AV_CONTROL_PLAY:
        m_bFlushAll = controlValue? true:false;
        if(m_bUpdatedSurface)
        {
            m_bUpdatedSurface = false;
            WFDMMLOGE("WFDMMSinkAVControl -- Move to INIT and then PLAY");
            if(ExecuteCommandSync(INIT) != true)
            {
                WFDMMLOGE("Play failed at INIT");
                return WFD_STATUS_FAIL;
            }

            ExecuteCommandSync(STREAM_SET_DECODER_LATENCY);

            if(ExecuteCommandSync(PLAY) != true)
            {
                WFDMMLOGE("Play failed to move to PLAY State");
                return WFD_STATUS_FAIL;
            }
        }
        else if(m_pNegotiatedCap->pSurface != NULL)
        {
            WFDMMLOGE("WFDMMSinkAVControl -- Normal STREAM_PLAY");
            ExecuteCommandSync(STREAM_PLAY);
        }
        else
        {
            WFDMMLOGE("WFDMMSinkAVControl -- surface is null. Abort Session");
            return WFD_STATUS_FAIL;
        }
        break;
    case AV_CONTROL_PAUSE:
        ExecuteCommandSync(STREAM_PAUSE);
        break;
    case AV_CONTROL_FLUSH:
        m_nFlushTimestamp = ((uint64)controlValue * 1000)/90 - m_nActualBaseTime;
        ExecuteCommandSync(STREAM_FLUSH);
        break;
    case AV_CONTROL_SET_VOLUME:
        ExecuteCommandSync(STREAM_SET_VOLUME);
        break;
    case AV_CONTROL_SET_DECODER_LATENCY:
        m_nDecoderLatency = (uint64)controlValue * 1000;
        ExecuteCommandSync(STREAM_SET_DECODER_LATENCY);
        break;
    default:
        break;
    }
    return WFD_STATUS_SUCCESS;
}
