/* =======================================================================
                              WFDMMSource.cpp
DESCRIPTION

  This module is for WFD source implementation
  Interacts with Audio and Video Source files and Source interface

  Copyright (c) 2011 - 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                                                         PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/src/WFDMMSource.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */


/* =======================================================================
                             Edit History

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "WFDMMSource.h"
#include "WFDMMSourceMux.h"
#ifdef WFD_VDS_ARCH
#include "WFDMMSourceVideoSource.h"
#else
#include "WFDMMSourceScreenSource.h"
#endif
#include "MMMemory.h"
#include <threads.h>
#include "WFD_HdcpCP.h"
#include "wfd_netutils.h"
#include "wfd_cfg_parser.h"

#define INPUT_IMAGE_WIDTH     480 //480
#define INPUT_IMAGE_HEIGHT    800 //800
#define OUTPUT_IMAGE_WIDTH    640 //480
#define OUTPUT_IMAGE_HEIGHT   480 //800

#define WFD_MM_SOURCE_THREAD_PRIORITY -16
#define ONE_MBPS    1 * 1000 * 1000

#define MAX_ALLOWED_BITRATE  14 * ONE_MBPS

OMX_U32 WFDbitrate;
uint32_t output_width    = OUTPUT_IMAGE_WIDTH;
uint32_t output_height   = OUTPUT_IMAGE_HEIGHT;
uint32_t input_width     = INPUT_IMAGE_WIDTH;
uint32_t input_height    = INPUT_IMAGE_HEIGHT;

uint32 av_format_chnage_interval_source;


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
/* ==========================================================================
                 DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
//! Events the WFDMMSource working thread processes.
const uint32 WFDMMSource::WFD_MM_SOURCE_PLAY_EVENT = 1;
const uint32 WFDMMSource::WFD_MM_SOURCE_PAUSE_EVENT = 2;
const uint32 WFDMMSource::WFD_MM_SOURCE_FILL_THIS_BUFFER_EVENT = 3;
const uint32 WFDMMSource::WFD_MM_SOURCE_EMPTY_THIS_BUFFER_EVENT = 4;
const uint32 WFDMMSource::WFD_MM_SOURCE_THREAD_EXIT_EVENT = 5;
unsigned int nInFrameRate;
OMX_BOOL b_IsStopped;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/*==============================================================================

         FUNCTION:         WFDMMSource

         DESCRIPTION:
*//**       @brief         This is the WFDMMSource class constructor - initializes
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
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
WFDMMSource::WFDMMSource
(
  WFD_device_t             eWFDDeviceType,
  WFD_MM_capability_t      *pNegotiatedCap,
  WFD_MM_callbacks_t       *pCallback
)
{
    initDefaults();
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSource");
    m_result = OMX_ErrorNone;
    OMX_U32 nMuxNumVidBuffers = 0;
    OMX_U32 nMuxNumAudBuffers = 0;



    if( (eWFDDeviceType != WFD_DEVICE_SOURCE ) ||
        (pNegotiatedCap == NULL) ||
          (pNegotiatedCap->video_method != WFD_VIDEO_INVALID &&
           pNegotiatedCap->video_config.video_config.num_h264_profiles == 0)
        )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::WFDMMSource bad parameters");
        m_result = OMX_ErrorBadParameter;
        return;
    }

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
        MM_Malloc( sizeof(WFD_MM_capability_t) );

    if (m_pNegotiatedCap == NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
          "WFDMMSource::allocating memory for negotiated capability failed");
        m_result = OMX_ErrorInsufficientResources;
        return ;
    }

    memset(m_pNegotiatedCap,0,sizeof(WFD_MM_capability_t));
    memcpy(m_pNegotiatedCap, pNegotiatedCap, sizeof(WFD_MM_capability_t));
    // Configure HDCP Module with Callback
    if(m_pNegotiatedCap->HdcpCtx.hHDCPSession)
    {
       m_result = (OMX_ERRORTYPE)
                  m_pNegotiatedCap->HdcpCtx.hHDCPSession->WFD_HdcpCpConfigureCallback(
                                                                 eventHandlerCb,
                                                           WFDMM_HDCP_MODULE_ID,
                                                                          this);
       if (m_result && sCallBacks.update_event_cb)
       {
          sCallBacks.update_event_cb(WFD_EVENT_MM_HDCP,WFD_STATUS_FAIL, NULL);
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Configure HDCP Module Failed");
          m_result = OMX_ErrorInsufficientResources;
          return;
       }
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
      "WFDMMSource:: video method %d",pNegotiatedCap->video_method);

    if (pNegotiatedCap->video_method == WFD_VIDEO_H264)
    {
        m_pNegotiatedCap->video_config.video_config.h264_codec =
        (WFD_h264_codec_config_t*)MM_Malloc(sizeof(WFD_h264_codec_config_t) *
        pNegotiatedCap->video_config.video_config.num_h264_profiles);
    }
    else
    {
      memset(&m_pNegotiatedCap->video_config, 0, sizeof(WFD_video_config));
      //This is necessary since pNegotiatedCap has video config array allocated for MAX
      //and we would end up having a non NULL value for it.
    }

    if(m_pNegotiatedCap->video_config.video_config.h264_codec)
    {
        uint32 *pTemPtr = (uint32*)m_pNegotiatedCap->video_config.video_config.h264_codec;
        m_pNegotiatedCap->video_config.video_config.h264_codec =
            (WFD_h264_codec_config_t*)pTemPtr;

            memcpy(m_pNegotiatedCap->video_config.video_config.h264_codec,
                pNegotiatedCap->video_config.video_config.h264_codec,
                sizeof(WFD_h264_codec_config_t));
    }
    else
    {
      if (pNegotiatedCap->video_method == WFD_VIDEO_INVALID)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSource AUDIO ONLY PLAYBACK");
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::WFDMMSource memory allocation failed");
        m_result = OMX_ErrorInsufficientResources;
        return;
      }
    }

    /**---------------------------------------------------------------------------
    Validate Codecs.
    ---------------
    This is the only place where we should be checking for
    the codecs that we support as of now. This will reduce the effort to
    add support for more codecs later. Check for TODOs elsewhere too.
    Following check also takes into account whether audio only/video only playback
    session is selected.
    ------------------------------------------------------------------------------
    */
    if (!AreAVCodecsValid(pNegotiatedCap))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
          "WFDMMSource::Unsupported set of codecs");
      m_result = OMX_ErrorBadParameter;
      return;
    }

    /**---------------------------------------------------------------------------
    Create all components required
    ------------------------------------------------------------------------------
    */
    m_result = CreateComponents();

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Failed to create all components");
        return;
    }

    /**---------------------------------------------------------------------------
    Configure all components
    ------------------------------------------------------------------------------
    */

    if (m_bHDCPStatusNotified)
    {
        m_result = ConfigureComponents();

        if(m_result != OMX_ErrorNone)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Failed to configure all components");
            return;
        }
    }
    m_result = InitializeThreadsAndQueues();
    if( m_result != OMX_ErrorNone )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Failed to create Threads and queues");
        return;
    }
    //Moving the state to init as all components are created successfully
    m_eState = MMWFDSRC_STATE_INIT;
    return;
}

/*==============================================================================

         FUNCTION:         AreAVCodecsValid

         DESCRIPTION:
*//**       @brief         This is the WFDMMSource class helper function -
*//**                      It figures out if the set of audio/video codec
*//**                      combination is acceptable
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param
                           WFD_MM_capability_t  - WFD Negotiated Capability

*//*     RETURN VALUE:
*//**       @return
                           true if codec combinations are valid
                           false otherwise

@par     SIDE EFFECTS:
                           What side effects could this possibly have?

*//*==========================================================================*/

bool WFDMMSource::AreAVCodecsValid(WFD_MM_capability_t* pNegotiatedCap)
{
  bool isAudioCodecValid = false;
  bool isVideoCodecValid = false;

  switch (pNegotiatedCap->video_method)
  {
    case WFD_VIDEO_H264:
    case WFD_VIDEO_INVALID:
      isVideoCodecValid = true;
      break;
    default:
      isVideoCodecValid = false;
  }
  switch (pNegotiatedCap->audio_method)
  {
    case WFD_AUDIO_LPCM:
    case WFD_AUDIO_AAC:
    case WFD_AUDIO_DOLBY_DIGITAL:
    case WFD_AUDIO_INVALID:
      isAudioCodecValid = true;
      break;
    default:
      isAudioCodecValid = false;
  }

  if ((pNegotiatedCap->video_method == WFD_VIDEO_INVALID &&
       pNegotiatedCap->audio_method == WFD_AUDIO_INVALID) ||
       (!isVideoCodecValid && !isAudioCodecValid)
      )
  {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSource:: Neither Audio nor Video selected");
        return false;
  }
  return true;
}
/*==============================================================================

         FUNCTION:         InitializeThreadsAndQueues

         DESCRIPTION:
*//**       @brief         Creates the signals queues and threads.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None



*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::InitializeThreadsAndQueues()
{

    m_pbufferqsource = new wfdmmsourcequeue;
    if( m_pbufferqsource == NULL )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSource m_pbufferqsource creation failed");
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    m_pbufferqsink = new wfdmmsourcequeue;
    if( m_pbufferqsink == NULL )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSource m_pbufferqsink creation failed");
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    /**---------------------------------------------------------------------------
    Create the signal Q for the thread to wait on.
    ------------------------------------------------------------------------------
    */
    if ( 0 != MM_SignalQ_Create( &m_signalQ ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSource m_signalQ creation failed");
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    /**---------------------------------------------------------------------------
    Create the process sample signal. This signal is
    to process the audio/video/text/data samples.
    ------------------------------------------------------------------------------
    */
    if ( (0 != MM_Signal_Create( m_signalQ,
        (void *) &WFD_MM_SOURCE_PLAY_EVENT,
        NULL,
        &m_WFDMMSourcePlaySignal ) ) )
    {
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }
    /**---------------------------------------------------------------------------
    Create the process sample signal. This signal is to
    process the audio/video/text/data samples.
    ------------------------------------------------------------------------------
    */
    if ( (0 != MM_Signal_Create( m_signalQ,
        (void *) &WFD_MM_SOURCE_PAUSE_EVENT,
        NULL,
        &m_WFDMMSourcePauseSignal ) ) )
    {
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    /**---------------------------------------------------------------------------
    Create the process sample signal.
    This signal is to process the audio/video/text/data samples.
    ------------------------------------------------------------------------------
    */
    if ( (0 != MM_Signal_Create( m_signalQ,
        (void *) &WFD_MM_SOURCE_FILL_THIS_BUFFER_EVENT,
        NULL,
        &m_WFDMMSourceFillThisBufferSignal ) ) )
    {
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    /**---------------------------------------------------------------------------
    Create the process sample signal.
    This signal is to process the audio/video/text/data samples.
    ------------------------------------------------------------------------------
    */
    if ( (0 != MM_Signal_Create( m_signalQ,
        (void *) &WFD_MM_SOURCE_EMPTY_THIS_BUFFER_EVENT,
        NULL,
        &m_WFDMMSourceEmptyThisBufferSignal ) ) )
    {
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    /**---------------------------------------------------------------------------
    Create the thread exit signal.
    This signal is set to exit the file open
    ------------------------------------------------------------------------------
    */

    if (( 0 != MM_Signal_Create( m_signalQ,
        (void *) &WFD_MM_SOURCE_THREAD_EXIT_EVENT,
        NULL,
        &m_WFDMMSourceexitSignal ) ) )
    {
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }


    /**---------------------------------------------------------------------------
    Create the the thread.
    ------------------------------------------------------------------------------
    */
    if ( ( 0 != MM_Thread_CreateEx( 152,
        0,
        WFDMMSource::WFDMMSourceThreadEntry,
        this,
        WFDMMSource::WFD_MM_SOURCE_THREAD_STACK_SIZE,
        "Mux", &m_WFDMMSourceThreadHandle) ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSource thread creation failed");
        m_result = OMX_ErrorInsufficientResources;
        return m_result;
    }

    return m_result;

}


/*==============================================================================

         FUNCTION:         CreateComponents

         DESCRIPTION:
*//**       @brief         Create all components.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None



*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::CreateComponents()
{

    /**---------------------------------------------------------------------------
    Create component - Mux
    ------------------------------------------------------------------------------
    */
    m_pSink = new Muxer(SinkEmptyBufferDone, this,
                        eventHandlerCb, (OMX_U32)WFDMM_TSMUX_MODULE_ID);

    if(!m_pSink)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Create Components: Failed to create Mux");
        return OMX_ErrorInsufficientResources;
    }

    /**---------------------------------------------------------------------------
    Create component - VideoSource If videoenc present
    ------------------------------------------------------------------------------
    */
    if( NULL!= m_pNegotiatedCap)
    {
      if(m_pSink && m_pNegotiatedCap->video_method != WFD_VIDEO_INVALID)
      {
#ifdef WFD_VDS_ARCH
        m_pVideoSource = new VideoSource;
        if(!m_pVideoSource)
#else
        m_pScreenSource = new ScreenSource;
        if(!m_pScreenSource)
#endif
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "Create Components: Failed to create Screen Source");
            return OMX_ErrorInsufficientResources;
        }
        // Pass HDCP Library Handle to screen soruce
        if (( NULL!= m_pNegotiatedCap) &&
        ( NULL != m_pNegotiatedCap->HdcpCtx.hHDCPSession) )
        {
#ifdef WFD_VDS_ARCH
           m_pVideoSource->m_pHdcpHandle = m_pNegotiatedCap->HdcpCtx.hHDCPSession;
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"HDCPSession(%p)",
                  m_pVideoSource->m_pHdcpHandle);
#else
           m_pScreenSource->m_pHdcpHandle = m_pNegotiatedCap->HdcpCtx.hHDCPSession;
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"HDCPSession(%p)",
                  m_pScreenSource->m_pHdcpHandle);
#endif
        }
      }
   }
  if( NULL!= m_pNegotiatedCap)
  {
      if(m_pNegotiatedCap->HdcpCtx.hHDCPSession)
      {
        if(m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus !=
            HDCP_STATUS_SUCCESS
          )
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource not connected. Wait");
            int count = 200;
            //If HDCP is enabled wait until connection is made.
            while(m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus !=
                HDCP_STATUS_SUCCESS && count --)
            {
                MM_Timer_Sleep(10);
            }
            if(m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus !=
                HDCP_STATUS_SUCCESS)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource HDCP Connect timed out");
                if(sCallBacks.update_event_cb)
                {
                    sCallBacks.update_event_cb(WFD_EVENT_HDCP_CONNECT_DONE, WFD_STATUS_FAIL, NULL);
                }
                // If HDCP is enforced fail
                int nHDCPEnforced = 0;
                getCfgItem(HDCP_ENFORCED_KEY, &nHDCPEnforced);

                if(nHDCPEnforced)
                {
                  if(m_pNegotiatedCap->HdcpCtx.hHDCPSession == NULL ||
                    m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus != HDCP_STATUS_SUCCESS)
                  {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::Play HDCP Enforced but not connected FAIL");
                    if(sCallBacks.update_event_cb)
                    {
                        sCallBacks.update_event_cb(WFD_EVENT_MM_HDCP, WFD_STATUS_RUNTIME_ERROR, NULL);
                    }
                    return OMX_ErrorInsufficientResources;
                  }
                }
            }
            else
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource HDCP Connection succeeded");
                sCallBacks.update_event_cb(WFD_EVENT_HDCP_CONNECT_DONE, WFD_STATUS_SUCCESS, NULL);
            }
        }
        else
        {
            // HDCP connect succeeded in first attempt, notify upper layers
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource HDCP Connection succeeded");
            sCallBacks.update_event_cb(WFD_EVENT_HDCP_CONNECT_DONE, WFD_STATUS_SUCCESS, NULL);
        }
      }
      if(m_pNegotiatedCap->content_protection_config.content_protection_ake_port == 0)
      {
          m_bHDCPStatusNotified = OMX_TRUE;
      }
    /**---------------------------------------------------------------------------
    Create component - Audio Source
    ------------------------------------------------------------------------------
    */
    if(m_pSink &&
       m_pNegotiatedCap->audio_method != WFD_AUDIO_UNK &&
       m_pNegotiatedCap->audio_method != WFD_AUDIO_INVALID)
    {
        m_pAudioSource = new AudioSource();
        if(!m_pAudioSource)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "Create Components: Failed to create Audio Source");
            return OMX_ErrorInsufficientResources;
        }

        // Pass HDCP Library Handle to Audio source
        if (( m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus ) &&
            ( NULL!= m_pNegotiatedCap) &&
            ( NULL != m_pNegotiatedCap->HdcpCtx.hHDCPSession) )
        {
          m_pAudioSource->m_pHdcpHandle = m_pNegotiatedCap->HdcpCtx.hHDCPSession;
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                      "HDCPSession(%p)",
                      m_pAudioSource->m_pHdcpHandle);
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Create Components: No Audio");
    }
 }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         ConfigureComponents

         DESCRIPTION:
*//**       @brief         configure all components.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None



*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ConfigureComponents()
{

    OMX_U32       nTempNumBufA          = 0;
    OMX_U32       nTempNumBufB          = 0;

    /**---------------------------------------------------------------------------
    Configure Components
    ------------------------------------------------------------------------------
    */

    /**---------------------------------------------------------------------------
    Negotiate buffer requirement for each component
    Calculate num of buffers between Audio source and mux.
    ------------------------------------------------------------------------------
    */
    if (m_pSink)
    {
       m_result = m_pSink->GetInNumBuffers(PORT_INDEX_AUDIO, &nTempNumBufA);
    }

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Failed to Get Mux Audio Num Buffers");
    }
    if (m_pAudioSource)
    {
        m_result = m_pAudioSource->GetOutNumBuffers(0, &nTempNumBufB);
    }

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Failed to Get  Audio Src Num Buffers");
    }

    m_nBuffersAudSrcToMux = MAX(nTempNumBufA, nTempNumBufB);

    /**---------------------------------------------------------------------------
    Calculate num of buffers between Video and mux.
    ------------------------------------------------------------------------------
    */
    if(m_pSink)
    {
        m_result = m_pSink->GetInNumBuffers(PORT_INDEX_VIDEO, &nTempNumBufA);
    }

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Failed to Get screen src Num Buffers");
    }

    /**---------------------------------------------------------------------------
    Calculate num of buffers between VideoSource and VideoEnc
    ------------------------------------------------------------------------------
    */
#ifdef WFD_VDS_ARCH
    if (m_pVideoSource)
    {
        m_result = m_pVideoSource->GetOutNumBuffers(0, &nTempNumBufB);
    }
#else
    if (m_pScreenSource)
    {
        m_result = m_pScreenSource->GetOutNumBuffers(0, &nTempNumBufB);
    }
#endif
    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "Failed to Get Mux Video Num Buffers");
    }

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "Failed to Get Video Enc Out Num Buffers");
    }

    m_nBuffersSreenSrcToMux = MAX(nTempNumBufA, nTempNumBufB);

    /**---------------------------------------------------------------------------
    Configure mux based on audio video paramaters.
    ------------------------------------------------------------------------------
    */
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"About to config mux");
    if (m_result == OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Configuring mux");
        if(m_pSink)
        {

            if( !m_pMuxConfig )
            {
                m_result = FillSinkParameters();
            }
            if(m_pMuxConfig && m_result == OMX_ErrorNone)
            {
                m_pMuxConfig->nAudioBufferCount = m_nBuffersAudSrcToMux;
                m_pMuxConfig->nVideoBufferCount = m_nBuffersSreenSrcToMux;

                m_result = m_pSink->Configure( m_pMuxConfig, eventHandlerCb,
                                                (OMX_U32)WFDMM_TSMUX_MODULE_ID);
                if(m_result != OMX_ErrorNone)
                {
                    sCallBacks.update_event_cb(WFD_EVENT_MM_RTP, WFD_STATUS_FAIL, NULL);
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Config Mux Failed");
                }
            }
            else
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_RTP,
                                           WFD_STATUS_FAIL, NULL);
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Config Mux while FillSinkParameters Failed");
                return OMX_ErrorNone;
            }
        }
    }


    /**---------------------------------------------------------------------------
    Configure Video
    ------------------------------------------------------------------------------
    */
    if(m_result == OMX_ErrorNone)
    {
        if(!m_pVidEncConfig)
        {
            (void)ExtractVideoParameters();
        }
    }

    /**---------------------------------------------------------------------------
    Configure audio source if audio is present
    ------------------------------------------------------------------------------
    */
    if(m_result == OMX_ErrorNone && m_pAudioSource)
    {
        if(!m_pAudioConfig)
        {
            (void)ExtractAudioParameters();
            if(m_pAudioConfig && ((m_pAudioConfig->eCoding == OMX_AUDIO_CodingAAC &&
                m_pAudioConfig->nNumChannels == 4) ||
               (m_pAudioConfig->eCoding == OMX_AUDIO_CodingPCM &&
                m_pAudioConfig->nNumChannels > 2)))
            {
                m_result = OMX_ErrorBadParameter;
                sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO,
                                           WFD_STATUS_FAIL, NULL);
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "AAC 3.1 is not supported");

            }
        }
        if((OMX_ErrorNone == m_result) && m_pAudioConfig &&
            (m_pAudioConfig->eCoding == OMX_AUDIO_CodingPCM ||
            m_pAudioConfig->eCoding == OMX_AUDIO_CodingAAC ||
             m_pAudioConfig->eCoding == OMX_AUDIO_CodingAC3))
        {
            m_pAudioConfig->nAudioInBufCount = m_nBuffersAudSrcToMux;
            if( m_pAudioSource )
            {
                m_result = m_pAudioSource->Configure(
                    m_pAudioConfig,
                    AudioSourceDeliveryFn,
                    eventHandlerCb,
                    (char *)" ",
                    (OMX_U32)WFDMM_AUDIOSRC_MODULE_ID,
                    this);
            }
            if(m_result != OMX_ErrorNone)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO,
                                           WFD_STATUS_FAIL, NULL);
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Configure Audio Source Failed");
            }
        }
    }

    /**---------------------------------------------------------------------------
    Configure Screen Source if present m_pVideoSource
    ------------------------------------------------------------------------------
    */

    if(m_result == OMX_ErrorNone)
    {
#ifdef WFD_VDS_ARCH
        if( m_pVideoSource )
#else
        if( m_pScreenSource )
#endif
        {
            if(m_pNegotiatedCap->video_config.video_config.h264_codec->
                frame_rate_control_support)
            {
                uint8 frame_rate_control_support =
                    m_pNegotiatedCap->video_config.video_config.h264_codec->
                    frame_rate_control_support;

                uint8 nSkipLimitInHalfseconds =
                               (frame_rate_control_support >> 1) & 0x7;

                m_bFrameSkipEnabled = OMX_FALSE;

                m_nFrameSkipLimitInterval = 0;
                if(frame_rate_control_support & 0x1)
                {
                    m_bFrameSkipEnabled = OMX_TRUE;
                    m_nFrameSkipLimitInterval = ((OMX_U64)nSkipLimitInHalfseconds * 1000) / 2;
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,"Skip frame enabled");
                    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"Skip frame limit %lld",
                        m_nFrameSkipLimitInterval);
                }

                if(frame_rate_control_support & 0x10)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Frame Rate Change enabled");
                    m_bFrameRateChangeEnabled = OMX_TRUE;
                }
                if(m_pVidEncConfig)
                {
                    m_pVidEncConfig->bEnableFrameSkip = m_bFrameSkipEnabled;
                    m_pVidEncConfig->nFrameSkipInterval = m_nFrameSkipLimitInterval;
                }
            }
#ifdef WFD_VDS_ARCH
            m_result = m_pVideoSource->Configure(m_pVidEncConfig,
#else
            m_result = m_pScreenSource->Configure(m_pVidEncConfig,
#endif
                m_nBuffersSreenSrcToMux,
                SourceDeliveryFn,
                eventHandlerCb,
                m_bFrameSkipEnabled,
                m_nFrameSkipLimitInterval,
                (OMX_U32)WFDMM_SCREENSRC_MODULE_ID,
                this );
        }
        if (m_result != OMX_ErrorNone)
        {
            sCallBacks.update_event_cb(WFD_EVENT_MM_VIDEO,
                                       WFD_STATUS_FAIL, NULL);
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Configure VideoSource Failed");
        }
    }

    return m_result;
}


/*==============================================================================

         FUNCTION:         initDefaults

         DESCRIPTION:
*//**       @brief         This function initializes the WFDMMSource class
                           members with default values.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None



*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::initDefaults()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::initDefaults");
#ifdef WFD_VDS_ARCH
    m_pVideoSource = NULL;
#else
    m_pScreenSource = NULL;
#endif
    m_pSink = NULL;
    m_pAudioSource = NULL;
    m_nFramesCoded = 0;
    m_hLnkSpdTmr = NULL;
    m_nBits = 0;
    m_pVidEncConfig = NULL;
    m_pVidEncDynmicConfig = NULL;
    m_pNegotiatedCap = NULL;
    m_pbufferqsink = NULL;
    m_pbufferqsource = NULL;
    m_pMuxConfig = NULL;
    m_signalQ = NULL;
    m_WFDMMSourcePlaySignal = NULL;
    m_WFDMMSourcePauseSignal = NULL;
    m_WFDMMSourceFillThisBufferSignal = NULL;
    m_WFDMMSourceEmptyThisBufferSignal = NULL;
    m_WFDMMSourceexitSignal = NULL;
    m_WFDMMSourceThreadHandle = NULL;
    m_wfdmmhandle = NULL;
    m_pAudioConfig = NULL;
    m_bFrameSkipEnabled = OMX_FALSE;
    m_bFrameRateChangeEnabled = OMX_FALSE;
    m_nFrameSkipLimitInterval = 0;
    m_bMuxRunning = OMX_FALSE;
    m_eState = MMWFDSRC_STATE_INVALID;
    m_nFloatingBitrate = 0;
    m_bBitrateAdaptEnabled = OMX_FALSE;
    m_nBuffersSreenSrcToMux = 0;
    m_nBuffersAudSrcToMux   = 0;
    m_IsErrorHandling = OMX_FALSE;
    m_nMinBitrate = 0;
    m_nMaxBitrate = 0;
    m_nCurrentPTS = 0;
    m_LinkSpeedSampleCnt = 0;
    m_bHDCPStatusNotified = OMX_FALSE;
    memset(&sCallBacks,0,sizeof(WFD_MM_callbacks_t));
    m_bReinitUtils = true;
}

/*==============================================================================

         FUNCTION:         ~WFDMMSource

         DESCRIPTION:
*//**       @brief         WFDMMSource destructor


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None



*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
WFDMMSource::~WFDMMSource()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::~WFDMMSource");
    //==========================================

    /* Send the exit signal to the source thread. */
    if(m_WFDMMSourceexitSignal && m_WFDMMSourceThreadHandle)
    {
        int nExitCode = 0;
        MM_Signal_Set( m_WFDMMSourceexitSignal );
        MM_Thread_Join( m_WFDMMSourceThreadHandle, &nExitCode );
        /* Release the thread resources. */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::~WFDMMSource exited the thread");
    }

    if(m_WFDMMSourceexitSignal)
    {
        MM_Signal_Release(m_WFDMMSourceexitSignal);
    }

    if(m_WFDMMSourceFillThisBufferSignal)
    {
        MM_Signal_Release(m_WFDMMSourceFillThisBufferSignal);
    }

    if(m_WFDMMSourcePlaySignal)
    {
        MM_Signal_Release(m_WFDMMSourcePlaySignal);
    }

    if(m_WFDMMSourceEmptyThisBufferSignal)
    {
        MM_Signal_Release(m_WFDMMSourceEmptyThisBufferSignal);
    }

    if(m_WFDMMSourcePauseSignal)
    {
        MM_Signal_Release(m_WFDMMSourcePauseSignal);
    }

    if(m_signalQ)
    {
        MM_SignalQ_Release(m_signalQ);
    }
    if(m_WFDMMSourceThreadHandle)
    {
        MM_Thread_Release(m_WFDMMSourceThreadHandle);
    }

    if(m_hLnkSpdTmr)
    {
        int res = MM_Timer_Release(m_hLnkSpdTmr);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
              "WFDMMSource::Releasing Linkspeed timer %d",res);
        m_hLnkSpdTmr = NULL;
    }

    // Free our helper classes
#ifdef WFD_VDS_ARCH
    if (m_pVideoSource)
    {
        MM_Delete(m_pVideoSource);
        m_pVideoSource = NULL;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::~WFDMMSource m_pVideoSource is freed");
#else
    if (m_pScreenSource)
    {
        MM_Delete(m_pScreenSource);
        m_pScreenSource = NULL;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::~WFDMMSource m_pScreenSource is freed");
#endif
    if (m_pAudioSource)
    {
        MM_Delete(m_pAudioSource);
        m_pAudioSource = NULL;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
            "WFDMMSource::~WFDMMSource m_pAudioSource is freed");
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::~WFDMMSource m_pVidEnc is freed");
    if (m_pSink)
    {
        MM_Delete(m_pSink);
        m_pSink = NULL;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::~WFDMMSource m_pSink is freed");

    if( m_pNegotiatedCap != NULL && m_pNegotiatedCap->HdcpCtx.hHDCPSession != NULL )
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::~WFDMMSource calling HDCP destructor ");
       delete m_pNegotiatedCap->HdcpCtx.hHDCPSession;
       m_pNegotiatedCap->HdcpCtx.hHDCPSession = NULL;
    }

    if (m_pNegotiatedCap && m_pNegotiatedCap->video_config.video_config.h264_codec)
    {
      MM_Free(m_pNegotiatedCap->video_config.video_config.h264_codec);
      m_pNegotiatedCap->video_config.video_config.h264_codec = NULL;
    }
    if(m_pNegotiatedCap)
    {
        MM_Free(m_pNegotiatedCap);
        m_pNegotiatedCap = NULL;
    }
    if(m_pVidEncConfig)
    {
        MM_Free(m_pVidEncConfig);
        m_pVidEncConfig = NULL;
    }
    if(m_pVidEncDynmicConfig)
    {
        MM_Free(m_pVidEncDynmicConfig);
        m_pVidEncDynmicConfig = NULL;
    }
    if(m_pMuxConfig)
    {
        MM_Free(m_pMuxConfig);
        m_pMuxConfig = NULL;
    }

    if(m_pAudioConfig)
    {
        MM_Free(m_pAudioConfig);
        m_pAudioConfig = NULL;
    }

    if(m_pbufferqsink)
    {
        MM_Delete(m_pbufferqsink);
        m_pbufferqsink = NULL;
    }

    if(m_pbufferqsource)
    {
        MM_Delete(m_pbufferqsource);
        m_pbufferqsource = NULL;
    }
}

/*==============================================================================

         FUNCTION:         WFDMMSourcePlay

         DESCRIPTION:
*//**       @brief         This function is play interface


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        WFD_MM_HANDLE - Handle to WFDMMSource
                          WFD_AV_select_t - To select auido only/video only/ audio + video
                          wfd_mm_stream_play_cb - Callback


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::WFDMMSourcePlay(WFD_MM_HANDLE wfdmmhandle,
                                           WFD_AV_select_t av_select,
                                           wfd_mm_stream_play_cb mm_stream_play_cb)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSourcePlay");

    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource In Unrecoverable Error");
        return OMX_ErrorInvalidComponent;
    }
    if( !mm_stream_play_cb )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSourcePlay mm_stream_play_cb NULL");
        return OMX_ErrorBadParameter;
    }
    /**---------------------------------------------------------------------------
    Play is handled asynchronously. Set the signal for play
    ------------------------------------------------------------------------------
    */
    m_wfd_mm_stream_play_cb = mm_stream_play_cb;
    m_av_select = av_select;
    m_wfdmmhandle = wfdmmhandle;
    MM_Signal_Set( m_WFDMMSourcePlaySignal );
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         WFDMMSourceUpdateSession

         DESCRIPTION:
*//**       @brief         This function Updated the WFD Session


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        WFD_MM_HANDLE - WFD MM handle
                          WFD_MM_capability_t - MM capability
                          wfd_mm_update_session_cb - Session callback

*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/

OMX_ERRORTYPE WFDMMSource::WFDMMSourceUpdateSession( WFD_MM_capability_t *WFD_negotiated_capability,
                                                    wfd_mm_update_session_cb pUpdateSessionCb)
{
    (void) pUpdateSessionCb;
    if(!m_pVidEncConfig || !m_pMuxConfig || !m_pNegotiatedCap)
    {
      return OMX_ErrorInsufficientResources;
    }
    OMX_S32 nCurrWidth     = m_pVidEncConfig->nFrameWidth;
    OMX_S32 nCurrHeight    = m_pVidEncConfig->nFrameHeight;
    OMX_S32 nCurrFramerate = m_pVidEncConfig->nFramerate;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFDMMSrc Update Capability called");

    if(!WFD_negotiated_capability)
    {
        m_result = OMX_ErrorBadParameter;
    }
    else
    {
        getResolutionRefreshRate(
        WFD_negotiated_capability->video_config.video_config.h264_codec->supported_cea_mode,
        WFD_negotiated_capability->video_config.video_config.h264_codec->supported_vesa_mode,
        WFD_negotiated_capability->video_config.video_config.h264_codec->supported_hh_mode);

        if(nCurrWidth != m_pVidEncConfig->nFrameWidth ||
               nCurrHeight != m_pVidEncConfig->nFrameHeight ||
               nCurrFramerate != m_pVidEncConfig->nFramerate)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSrc Resolution changed");
#ifdef WFD_VDS_ARCH
            if( m_pVideoSource)
            {
               m_result = m_pVideoSource->Configure(m_pVidEncConfig,
#else
            if( m_pScreenSource)
            {
               m_result = m_pScreenSource->Configure(m_pVidEncConfig,
#endif
                    m_nBuffersSreenSrcToMux,
                    SourceDeliveryFn,
                    eventHandlerCb,
                    m_bFrameSkipEnabled,
                    m_nFrameSkipLimitInterval,
                    (OMX_U32)WFDMM_SCREENSRC_MODULE_ID,
                    this );
            }
            else
            {
              m_result = OMX_ErrorUnsupportedSetting;
            }
        }
        else if(WFD_negotiated_capability->transport_capability_config.eRtpPortType !=
                m_pNegotiatedCap->transport_capability_config.eRtpPortType)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSource:: Update transport type");
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP =
               (WFD_negotiated_capability->transport_capability_config.eRtpPortType ==
                RTP_PORT_UDP)? true : false;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.portno1 =
                WFD_negotiated_capability->transport_capability_config.port1_id;

            OMX_ERRORTYPE err = OMX_ErrorUndefined;

            err = m_pSink->Update(m_pMuxConfig);

            if(err != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                            "WFDMMSource:: Failed to Update transport type");
            }
            else
            {
                m_pNegotiatedCap->transport_capability_config.eRtpPortType
                     = WFD_negotiated_capability->transport_capability_config.eRtpPortType;
            }
        }

    }

    return m_result;
}
/*==============================================================================

         FUNCTION:         WFDMMGetProposedCapability

         DESCRIPTION:
*//**       @brief         This function Get the proposed capability


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        WFD_MM_HANDLE - WFD MM handle
                          WFD_MM_capability_t - MM capability
                          WFD_MM_capability_t - MM capability
                          WFD_MM_capability_t - MM capability


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::WFDMMGetProposedCapability(WFD_MM_HANDLE hHandle,
                                                      WFD_MM_capability_t* pMMCfg_local,
                                                      WFD_MM_capability_t* pMMCfg_remote,
                                                      WFD_MM_capability_t* pMMCfg_proposed)
{
    (void) hHandle;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, " WFDMMSource:: Get Proposed Capability");
    bool bFound = false;
    OMX_U32 nModeFound = 0;


    if(!m_bFrameRateChangeEnabled)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, " WFDMMSource:: Sink doesnt support frame rate change");
    }
    if(m_bFrameRateChangeEnabled)
    {
        OMX_S32 nCurrWidth     = m_pVidEncConfig->nFrameWidth;
        OMX_S32 nCurrHeight    = m_pVidEncConfig->nFrameHeight;
        OMX_S32 nCurrFramerate = m_pVidEncConfig->nFramerate;
        OMX_U32 nMode = 0;
        OMX_U32 nIntersectMode = 0;

        //Loop only once for now. No profile level change supported.
        for(int i = 0; i < 1 /*pMMCfg_remote->video_config.video_config.num_h264_profiles*/; i++)
        {

            //Resolution change is not supported. SO comparisons across modes is not required
            if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_cea_mode)
            {
                if( pMMCfg_remote->video_config.video_config.h264_codec[i].supported_cea_mode)
                {
                    nMode = m_pNegotiatedCap->video_config.video_config.h264_codec[i].
                        supported_cea_mode;

                    nIntersectMode = pMMCfg_remote->video_config.video_config.h264_codec[i].
                        supported_cea_mode &
                        pMMCfg_local->video_config.video_config.h264_codec[i].
                        supported_cea_mode;

                    if(!nIntersectMode)
                    {
                        nIntersectMode = 1;
                    }

                    // As we do not support resolution change, different frame rate can be possible only for previous and next
                    // bit in the bit map.
                    getResolutionRefreshRate((uint32)(nIntersectMode & (nMode << 1)),
                        0,0);

                    if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                        nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                        nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New CEA Mode for AVFormat Change Found %ld",
                            nMode << 1);
                        nModeFound = nMode << 1;
                        bFound = true;
                    }
                    else
                    {
                        getResolutionRefreshRate((uint32)(nIntersectMode & (nMode >> 1)),
                            0,0);

                        if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                            nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                            nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                        {
                            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New CEA Mode for AVFormat Change Found %ld",
                                nMode >> 1);
                            nModeFound = nMode >> 1;
                            bFound = true;
                        }
                    }


                }
            }
            else if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_vesa_mode)
            {
                if( pMMCfg_remote->video_config.video_config.h264_codec[i].supported_vesa_mode)
                {
                    nMode = m_pNegotiatedCap->video_config.video_config.h264_codec[i].
                        supported_vesa_mode;
                    nIntersectMode = pMMCfg_remote->video_config.video_config.h264_codec[i].
                        supported_vesa_mode &
                        pMMCfg_local->video_config.video_config.h264_codec[i].
                        supported_vesa_mode;
                    // As we do not support resolution change, different frame rate can be possible only for previous and next
                    // bit in the bit map.

                    getResolutionRefreshRate(0,(uint32)(nIntersectMode & (nMode << 1)),
                        0);

                    if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                        nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                        nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New VESA Mode for AVFormat Change Found %ld",
                            nMode << 1);
                        nModeFound = nMode << 1;
                        bFound = true;
                    }
                    else
                    {
                        getResolutionRefreshRate(0,(uint32) (nIntersectMode & (nMode >> 1)),
                            0);

                        if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                            nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                            nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                        {
                            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New VESA Mode for AVFormat Change Found %ld",
                                nMode >> 1);
                            nModeFound = nMode >> 1;
                            bFound = true;
                        }
                    }

                }

            }
            else if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_hh_mode)
            {
                if( pMMCfg_remote->video_config.video_config.h264_codec[i].supported_hh_mode)
                {
                    nMode = m_pNegotiatedCap->video_config.video_config.h264_codec[i].
                        supported_hh_mode;
                    nIntersectMode = pMMCfg_remote->video_config.video_config.h264_codec[i].
                        supported_hh_mode &
                        pMMCfg_local->video_config.video_config.h264_codec[i].
                        supported_hh_mode;
                    // As we do not support resolution change, different frame rate can be possible only for previous and next
                    // bit in the bit map.

                    getResolutionRefreshRate(0,0,(uint32)(nIntersectMode & (nMode << 1)));

                    if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                        nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                        nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New HH Mode for AVFormat Change Found %ld",
                            nMode << 1);
                        nModeFound = nMode << 1;
                        bFound = true;
                    }
                    else
                    {
                        getResolutionRefreshRate(0,0, (uint32)(nIntersectMode & (nMode >> 1)));

                        if(nCurrWidth == m_pVidEncConfig->nFrameWidth &&
                            nCurrHeight == m_pVidEncConfig->nFrameHeight &&
                            nCurrFramerate !=  m_pVidEncConfig->nFramerate  )
                        {
                            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"New HH Mode for AVFormat Change Found %ld",
                                nMode >> 1);
                            nModeFound = nMode >> 1;
                            bFound = true;
                        }
                    }


                }

            }

            if(bFound)
            {
                uint32 *pTemPtr = (uint32*)pMMCfg_proposed->video_config.video_config.h264_codec;
                memcpy(pMMCfg_proposed, m_pNegotiatedCap,
                    sizeof(WFD_MM_capability_t));
                pMMCfg_proposed->video_config.video_config.h264_codec =
                    (WFD_h264_codec_config_t*)pTemPtr;

                memcpy(pMMCfg_proposed->video_config.video_config.h264_codec,
                    m_pNegotiatedCap->video_config.video_config.h264_codec,
                    sizeof(WFD_h264_codec_config_t));

                if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_cea_mode)
                {
                    pMMCfg_proposed->video_config.video_config.h264_codec->supported_cea_mode =
                       (uint32) nModeFound;
                }
                else if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_vesa_mode)
                {
                    pMMCfg_proposed->video_config.video_config.h264_codec->supported_vesa_mode =
                        (uint32)nModeFound;
                }
                else if(m_pNegotiatedCap->video_config.video_config.h264_codec->supported_hh_mode)
                {
                    pMMCfg_proposed->video_config.video_config.h264_codec->supported_hh_mode =
                        (uint32)nModeFound;

                }
                m_pVidEncConfig->nFrameWidth   = nCurrWidth;
                m_pVidEncConfig->nFrameHeight  = nCurrHeight;
                return OMX_ErrorNone;
            }

        }

        m_pVidEncConfig->nFrameWidth   = nCurrWidth;
        m_pVidEncConfig->nFrameHeight  = nCurrHeight;

    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"MFDMMSrc : No New Matching Mode for AVFormat Change Found");
    return OMX_ErrorUnsupportedSetting;

}

/*==============================================================================

         FUNCTION:         Play

         DESCRIPTION:
*//**       @brief         This function is to play MM components


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::Play()
{
    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource In Unrecoverable Error");
        return OMX_ErrorInvalidComponent;
    }
    if(m_bHDCPStatusNotified != OMX_TRUE)
    {
      if(m_pNegotiatedCap->HdcpCtx.hHDCPSession &&
          m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus ==
          HDCP_STATUS_SUCCESS
          )
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource HDCP Connection succeeded");
          sCallBacks.update_event_cb(WFD_EVENT_HDCP_CONNECT_DONE, WFD_STATUS_SUCCESS, NULL);
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFD MMsource HDCP Connect timed out");
          sCallBacks.update_event_cb(WFD_EVENT_HDCP_CONNECT_DONE, WFD_STATUS_FAIL, NULL);
      }
      m_result = ConfigureComponents();
      if(m_result != OMX_ErrorNone)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::Failed to configure components");
          return OMX_ErrorInsufficientResources;
      }
      m_bHDCPStatusNotified = OMX_TRUE;
    }
    m_IsErrorHandling = OMX_FALSE;

    OMX_BUFFERHEADERTYPE** ppVidOutputBuffers = NULL;
    OMX_BUFFERHEADERTYPE** ppAudBuffers = NULL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::Play");
    if( m_eState == MMWFDSRC_STATE_PLAY)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSource already in play state");
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play calling play callback WFD_STATUS_SUCCESS");
        m_wfd_mm_stream_play_cb(m_wfdmmhandle, WFD_STATUS_SUCCESS);
        return m_result;
    }

    //Bitrate Adaptation
    m_LinkSpeedSampleCnt = 0;


    //==========================================
    // Get the allocated input buffers
    if (m_result == OMX_ErrorNone && m_bMuxRunning == OMX_FALSE && m_pAudioSource
      && m_pAudioConfig)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play giving buffers to audio source %ld",
            m_pAudioConfig->nAudioInBufCount);

        ppAudBuffers = m_pAudioSource->GetBuffers();
        if(NULL == ppAudBuffers)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "WFDMMSource::Play getbuffer from audiosource errored out");
            m_result = OMX_ErrorNotReady;
        }
    }

    //==========================================
    // Get the allocated output buffers
#ifdef WFD_VDS_ARCH
    if (m_result == OMX_ErrorNone && m_bMuxRunning == OMX_FALSE && m_pVideoSource)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play giving buffers to video encoder");

        ppVidOutputBuffers = m_pVideoSource->GetBuffers();
#else
    if (m_result == OMX_ErrorNone && m_bMuxRunning == OMX_FALSE && m_pScreenSource)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play giving buffers to video encoder");

        ppVidOutputBuffers = m_pScreenSource->GetBuffers();
#endif
        if(NULL == ppVidOutputBuffers)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "WFDMMSource::Play getbuffer from encoder errored out");
            m_result = OMX_ErrorNotReady;
        }
    }

    //==========================================
    // Move sink to executing state first (also allocate buffers)
    if (m_result == OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play moving sync to executing state");

        if(m_bMuxRunning == OMX_FALSE)
        {
            m_bMuxRunning = OMX_TRUE;
            if(ppVidOutputBuffers)
            {
                m_pSink->EnableUseBufferModel(
                    OMX_TRUE, PORT_INDEX_VIDEO,
                    ppVidOutputBuffers,
                    m_pMuxConfig->nVideoBufferCount );
            }
            if(ppAudBuffers)
            {
                m_pSink->EnableUseBufferModel(
                    OMX_TRUE, PORT_INDEX_AUDIO,
                    ppAudBuffers,
                    m_pAudioConfig->nAudioInBufCount);
            }

            m_result = m_pSink->GoToExecutingState();
        }
    }

    //==========================================
    // Start reading and delivering frames
#ifdef WFD_VDS_ARCH
    if (m_result == OMX_ErrorNone && m_pVideoSource)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play starting video source");
        m_result = m_pVideoSource->Start();
#else
    if (m_result == OMX_ErrorNone && m_pScreenSource)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play starting video source");
        m_result = m_pScreenSource->Start();
#endif
    }
    // delay start of audio thread by 200 ms in order to give chance
    // to screen source thread to run. As audio thread is very high
    // priority and will take CPU once it get a chance to run.

    //MM_Timer_Sleep(200);

    if (m_result == OMX_ErrorNone && m_pAudioSource)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play starting audio source");
        m_result = m_pAudioSource->Start();
    }
    if (m_result == OMX_ErrorNone)
    {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Play calling play callback WFD_STATUS_SUCCESS");
        m_wfd_mm_stream_play_cb(m_wfdmmhandle, WFD_STATUS_SUCCESS);

    }

    m_eState = MMWFDSRC_STATE_PLAY;

    if(m_bBitrateAdaptEnabled == OMX_TRUE)
    {
        if(!m_hLnkSpdTmr)
        {
            if(0 != MM_Timer_Create(MM_LINKSPEED_CHECK_INTERVAL, 1,
                                    WFDMMSource::LinkSpeedCheckTimerCb,
                                    (void *)this, &m_hLnkSpdTmr))
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                      "WFDMMSource:: Linkspeed Timer failed");
            }
            else
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                      "WFDMMSource::Linkspeed Timer success");
            }
        }
        else
        {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                         "WFDMMSource:: Linkspeed Timer Already Created Error");
        }
    }
    return m_result;
}
/*==============================================================================

         FUNCTION:         LinkSpeedCheckTimerCb

         DESCRIPTION:
*//**       @brief         This function timer callback to periodically
                           checking Wifi MCS rate


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        void *  WFD MM Handle


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::LinkSpeedCheckTimerCb(void * handle)
{
    OMX_U32 nLinkSpeed = 0;
    OMX_U32 nUsefulBW = 0;
    WFDMMSource *pWFDMMSource = (WFDMMSource *)handle;

    if(!handle)
    {
        return;
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,"LinkSpeed Timer");

    nLinkSpeed = pWFDMMSource->GetLinkSpeedFromWLANDriver();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"LinkSpeed %ld", nLinkSpeed);

    if(nLinkSpeed == 0)
    {
        return;
    }

    pWFDMMSource->m_LinkSpeedSamples[pWFDMMSource->m_LinkSpeedSampleCnt] = nLinkSpeed;
    pWFDMMSource->m_LinkSpeedSampleCnt++;

    if(pWFDMMSource->m_LinkSpeedSampleCnt >= MM_BITRATE_UPDATE_AVG_WINDOW)
    {
        pWFDMMSource->m_LinkSpeedSampleCnt = 0;
        nLinkSpeed = pWFDMMSource->CalculateLinkspeedWeightedAvg();
        pWFDMMSource->NetworkAdaptiveRateControl(nLinkSpeed);
    }

    return;
}

/*==============================================================================

         FUNCTION:         CalculateLinkspeedWeightedAvg

         DESCRIPTION:
*//**       @brief         This function calculates average of linkspeed over
                           past N samples of MCS rate


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_U32 WFDMMSource::CalculateLinkspeedWeightedAvg()
{

#define WEIGHT_0  2 //0.02
#define WEIGHT_1  3 //0.03
#define WEIGHT_2  5 //0.05
#define WEIGHT_3  8 //0.08
#define WEIGHT_4  12//0.12
#define WEIGHT_5  15//0.15
#define WEIGHT_6  20//0.20
#define WEIGHT_7  35//0.35
    //Take a weighted average to find the current linkspeed
    //Pre-Divide by 100 to avoid overflow sacrificing some precision.

    return (m_LinkSpeedSamples[0] / 100) * WEIGHT_0  +
           (m_LinkSpeedSamples[1] / 100) * WEIGHT_1  +
           (m_LinkSpeedSamples[2] / 100) * WEIGHT_2  +
           (m_LinkSpeedSamples[3] / 100) * WEIGHT_3  +
           (m_LinkSpeedSamples[4] / 100) * WEIGHT_4  +
           (m_LinkSpeedSamples[5] / 100) * WEIGHT_5  +
           (m_LinkSpeedSamples[6] / 100) * WEIGHT_6  +
           (m_LinkSpeedSamples[7] / 100) * WEIGHT_7;

}

/*==============================================================================

         FUNCTION:         NetworkAdaptiveRateControl

         DESCRIPTION:
*//**       @brief         This function timer changes bitrate based on network
                            conditions.


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        void *  WFD MM Handle


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::NetworkAdaptiveRateControl(OMX_U32 nLinkSpeed)
{
    OMX_U32 nUsefulBW = 0;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                         "NetworkAdaptiveRateControl : avg linkspeed %ld",
                          nLinkSpeed);
    if(nLinkSpeed == 0)
    {
        return;
    }

    /**-------------------------------------------------------------------------
     Once we have a valid linkspeed(MCS rate in this case) we can control
     bitrate if required. We can reduce bitrate if linkspeed is lesser than
     the preferred bitrate for current settings. Alternately if linkspeed has
     increased over a period of time and we have been running at a lower than
     preferred bitrate we can increase bitrate upto preferred bitrate for
     current settings.
    ----------------------------------------------------------------------------
    */
    nUsefulBW = ((nLinkSpeed/10) * 8);

    if(m_pAudioConfig)
    {
        /**---------------------------------------------------------------------
         If session includes audio consider audio fixed bitrate while doing
         bitrate adaptation.
        ------------------------------------------------------------------------
        */
        if(nUsefulBW <= (OMX_U32)m_pAudioConfig->nBitrate)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "WFDMM Network too bad, no room for video");
            /**-----------------------------------------------------------------
             No bandwidth for video.
            --------------------------------------------------------------------
            */
            nUsefulBW = 0;
        }
        else
        {
            nUsefulBW -= m_pAudioConfig->nBitrate;
        }

    }

    /**-------------------------------------------------------------------------
        Never let bitrate go below minBitrate
    ----------------------------------------------------------------------------
    */

    nUsefulBW >>= 1; //Instantaneous video bitrate can be double of NOMINAL

    if(nUsefulBW < m_nMinBitrate)
    {
        nUsefulBW = m_nMinBitrate;
    }

    if(nUsefulBW < ((m_nFloatingBitrate / 100) * 70))
    {
        /**---------------------------------------------------------------------
          Dont change bitrate  to less than 30% of current value
        ------------------------------------------------------------------------
        */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                     "Linkspeed limit to 70 percent");
        nUsefulBW = ((m_nFloatingBitrate / 100) * 70);
    }

    if(m_nFloatingBitrate > nUsefulBW)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "Linkspeed less than bitrate..Update %ld -> %ld",
                     m_nFloatingBitrate, nUsefulBW);
        m_nFloatingBitrate = nUsefulBW;
        ChangeBitrate(this, nUsefulBW);
    }
    else if(m_nFloatingBitrate < nUsefulBW &&
            m_nFloatingBitrate < WFDbitrate)
    {
        if(nUsefulBW <= WFDbitrate)
        {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                         "Increase bitrate to linkspeed %ld -> %ld",
                         m_nFloatingBitrate, nUsefulBW);
            m_nFloatingBitrate =  nUsefulBW;
            ChangeBitrate(this, nUsefulBW);
        }
        else
        {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "Increase bitrate to normal. %ld -> %d",
            m_nFloatingBitrate, WFDbitrate);
            m_nFloatingBitrate = WFDbitrate;
            ChangeBitrate(this, WFDbitrate);
        }

    }

    return;
}


/*==============================================================================

         FUNCTION:         GetLinkSpeedFromWLANDriver

         DESCRIPTION:
*//**       @brief         This function gets MCS rate from WLAN driver using
                           netutils


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        void *  WFD MM Handle


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_U32 WFDMMSource::GetLinkSpeedFromWLANDriver()
{
    uint32 linkspeed = 0;

    linkspeed =
        getLinkSpeed((char*)(&m_pNegotiatedCap->peer_ip_addrs.device_addr1[0]),
                     (unsigned int) sizeof(m_pNegotiatedCap->peer_ip_addrs.device_addr1),m_bReinitUtils);
    return linkspeed;

}


/*==============================================================================

         FUNCTION:         WFDMMSourcePause

         DESCRIPTION:
*//**       @brief         This function is pause interface


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        wfdmmhandle  WFD MM Handle
                          av_select -  To select auido only/video only/ audio + video


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::WFDMMSourcePause(WFD_MM_HANDLE wfdmmhandle,
                                            WFD_AV_select_t av_select,
                                            wfd_mm_stream_pause_cb mm_stream_pause_cb)
{
    OMX_TICKS nRunTimeMillis = 0;
    OMX_TICKS nRunTimeSec = 0;
    if(m_result != OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource In Unrecoverable Error");
        return OMX_ErrorInvalidComponent;
    }
    m_result = OMX_ErrorNone;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSourcePause");
    if( !mm_stream_pause_cb )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::WFDMMSourcePause mm_stream_pause_cb is NULL");
        return OMX_ErrorBadParameter;
    }
    m_wfd_mm_stream_pause_cb = mm_stream_pause_cb;
    m_av_select = av_select;
    m_wfdmmhandle = wfdmmhandle;
    MM_Signal_Set( m_WFDMMSourcePauseSignal );
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         Pause

         DESCRIPTION:
*//**       @brief         This function is to play MM components


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::Pause()
{
    OMX_TICKS nRunTimeMillis = 0;
    OMX_TICKS nRunTimeSec = 0;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::Pause");
    if ( m_eState == MMWFDSRC_STATE_PAUSE)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource:: source is in Pause state already");
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause calling pause callback WFD_STATUS_SUCCESS");
      m_wfd_mm_stream_pause_cb(m_wfdmmhandle, WFD_STATUS_SUCCESS);
      return m_result;
    }
    if ( m_eState == MMWFDSRC_STATE_INIT)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource:: pause is not allowed before play");
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause calling pause callback WFD_STATUS_FAIL");
      m_wfd_mm_stream_pause_cb(m_wfdmmhandle, WFD_STATUS_FAIL);
      m_result = OMX_ErrorBadParameter;
      return m_result;
    }
    if(m_hLnkSpdTmr)
    {
        int res = MM_Timer_Release(m_hLnkSpdTmr);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
              "WFDMMSource::Releasing Linkspeed timer %d",res);
        m_hLnkSpdTmr = NULL;
    }

    if(m_pSink)
    {
        m_pSink->Flush();
    }

    // Wait for the video source to finish delivering all frames
#ifdef WFD_VDS_ARCH
    if (m_pVideoSource != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause waiting for video source to finish");
        m_result = m_pVideoSource->Pause();
#else
    if (m_pScreenSource != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause waiting for video source to finish");
        m_result = m_pScreenSource->Pause();
#endif
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause video source to finished");
    }

    // Wait for the audio source to finish delivering all frames
    if (m_pAudioSource!= NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause waiting for audio source to finish");
        m_result = m_pAudioSource->Finish();
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause audio source to finished");
    }


    // Tear down the MUX (also deallocate buffers)
    if (m_pSink!= NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause no state change for sink");

    }


    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::Pause  Time = %d millis, Encoded = %d, Dropped = %d",
        (int) nRunTimeMillis,
        (int) m_nFramesCoded,
        (int) (m_pVidEncConfig->nFrames - m_nFramesCoded));

    if (nRunTimeSec > 0) // ensure no divide by zero
    {
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause  Bitrate = %d, InputFPS = %d, OutputFPS = %d",
            (int) (m_nBits / nRunTimeSec),
            (int) (m_pVidEncConfig->nFrames / nRunTimeSec),
            (int) (m_nFramesCoded / nRunTimeSec));
    }
    else
    {
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause  Bitrate = %d, InputFPS = %d, OutputFPS = %d",0,0,0);
    }

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Avg encode time = %d millis per frame",
        (int) (nRunTimeMillis / m_pVidEncConfig->nFrames));


    if (m_result == OMX_ErrorNone)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Pause calling pause callback WFD_STATUS_SUCCESS");
        m_wfd_mm_stream_pause_cb(m_wfdmmhandle, WFD_STATUS_SUCCESS);
    }
    m_eState = MMWFDSRC_STATE_PAUSE;
    return m_result;
}

/*==============================================================================

         FUNCTION:         Stop

         DESCRIPTION:
*//**       @brief         This function is to stop the playback


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::Stop()
{
    OMX_TICKS nRunTimeMillis = 0;
    OMX_TICKS nRunTimeSec = 0;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::Stop");
    b_IsStopped = OMX_TRUE;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::Stop  b_IsStopped is true");

    if(m_pSink)
    {
        m_pSink->Flush();
    }

    // Wait for the video source to finish delivering all frames
#ifdef WFD_VDS_ARCH
    if (m_pVideoSource != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop waiting for video source to finish");
        m_result = m_pVideoSource->Finish();
#else
    if (m_pScreenSource != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop waiting for video source to finish");
        m_result = m_pScreenSource->Finish();
#endif
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop video source to finished %d", m_result);
    }

    // Wait for the audio source to finish delivering all frames
    if (m_pAudioSource != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop waiting for audio source to finish");
        m_result = m_pAudioSource->Finish();
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop audio source to finished %d", m_result);
    }

    // Tear down the MUX (also deallocate buffers)
    if (m_pSink != NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop move sink to loaded state");
        m_result = m_pSink->GoToLoadedState();
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::Stop sink moved to loaded state %d", m_result);
    }

    m_eState = MMWFDSRC_STATE_INIT;
    return m_result;
}


/*==============================================================================

         FUNCTION:         WFDMMSourceThreadEntry

         DESCRIPTION:
*//**       @brief         The WFDMMSource working thread entry function. Once
                           the thread is created the control comes to this function.


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        void* WFDMMSource pointer


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
int WFDMMSource::WFDMMSourceThreadEntry( void* ptr )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSourceThreadEntry");
    WFDMMSource* pThis = (WFDMMSource *) ptr;

    if ( NULL != pThis )
    {
        pThis->WFDMMSourceThread();
    }
    return 0;
}


/*==============================================================================

         FUNCTION:         WFDMMSourceThread

         DESCRIPTION:
*//**       @brief         The WFDMMSource working thread which handles commands
                            and events posted to it.


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
int WFDMMSource::WFDMMSourceThread( void )
{
    bool bRunning = true;
    wfdmmsource_item *item = NULL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::WFDMMSourceThread");
    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD: WFDMMSourceThread  priority b4 %d ",
       androidGetThreadPriority(tid));
    androidSetThreadPriority(0,WFD_MM_SOURCE_THREAD_PRIORITY);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD:WFDMMSourceThread  priority after%d ",
       androidGetThreadPriority(tid));
    while ( bRunning )
    {
        /* Wait for a signal to be set on the signal Q. */
        uint32 *pEvent = NULL;
        if ( 0 == MM_SignalQ_Wait( m_signalQ, (void **) &pEvent ) )
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource::WFDMMSourceThread");
            switch ( *pEvent )
            {
            case WFD_MM_SOURCE_PLAY_EVENT:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::WFDMMSourceThread WFD_MM_SOURCE_PLAY_EVENT");
                    m_result = Play();
                    if(m_result != OMX_ErrorNone)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSource::Error from Play(), Exiting!");
                        /* Exit the thread */
                        bRunning = false;
                    }
                    break;
                }
            case WFD_MM_SOURCE_PAUSE_EVENT:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::WFDMMSourceThread WFD_MM_SOURCE_PAUSE_EVENT");
                    m_result = Pause();
                    break;
                }

            case WFD_MM_SOURCE_EMPTY_THIS_BUFFER_EVENT:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                        "WFDMMSource::WFDMMSourceThread WFD_MM_SOURCE_EMPTY_THIS_BUFFER_EVENT");
                    if( m_pbufferqsource )
                    {
                        //pop the sample from the queue
                        item = m_pbufferqsource->Pop_Front();
                        if(item)
                        {
                            // Deliver encoded m4v output to sink
                            m_result = m_pSink->DeliverInput(
                                (OMX_BUFFERHEADERTYPE *)item->m_pBuffer);
                        }
                    }
                    break;
                }
            case WFD_MM_SOURCE_FILL_THIS_BUFFER_EVENT:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                        "WFDMMSource::WFDMMSourceThread WFD_MM_SOURCE_FILL_THIS_BUFFER_EVENT");
                    if( m_pbufferqsink )
                    {
                        //pop the sample from the queue
                        item = m_pbufferqsink->Pop_Front();

                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                            "WFDMMSource::WFDMMSourceThread Popped item");
                        if(item)
                        {
                            if(((OMX_BUFFERHEADERTYPE *)(item->m_pBuffer))->nInputPortIndex == 0)
                            {
                                // Deliver free audio buffer to audio source
                                m_result = m_pAudioSource->SetFreeBuffer(
                                    (OMX_BUFFERHEADERTYPE *)item->m_pBuffer);
                                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                    "WFDMMSource::WFDMMSourceThread Audio buffer for fill");

                            }
                            break;
                        }
                    }
                    break;
                }
            case WFD_MM_SOURCE_THREAD_EXIT_EVENT:
                {
                    if(!b_IsStopped)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSource::~WFDMMSource in Play");
                        /**-------------------------------------------------------------------------
                        Calling Stop will stop all components and bring them to loaded state.
                        ----------------------------------------------------------------------------
                        */
                        if( m_pNegotiatedCap != NULL && m_pNegotiatedCap->HdcpCtx.hHDCPSession != NULL )
                        {
                           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDDEBUG WFD_MM_SOURCE_THREAD_EXIT_EVENT set connect to false");
                           m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_bHdcpSessionConnect = false;
                        }

                        Stop();
                    }
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::WFDMMSourceThread WFD_MM_SOURCE_THREAD_EXIT_EVENT");
                    /* Exit the thread. */
                    bRunning = false;
                    MM_Thread_Exit( WFDMMSource::m_WFDMMSourceThreadHandle, 0 );
                    break;
                }
            default:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::WFDMMSourceThread received UNKNOWN EVENT");
                    /* Not a recognized event, ignore it. */
                }
            }
        }
    }
    return 0;
}

/*==============================================================================

         FUNCTION:         GenerateIFrameNext

         DESCRIPTION:
*//**       @brief         This Function is used for generating the I Frame


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::GenerateIFrameNext(WFD_MM_HANDLE wfdmmhandle)

{
    (void)wfdmmhandle;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::WFDMMSourceThread  sending i-frame");
#ifdef WFD_VDS_ARCH
    return(m_pVideoSource->RequestIntraVOP());
#else
    return(m_pScreenSource->RequestIntraVOP());
#endif
}

/*==============================================================================

         FUNCTION:         ChangeBitrate

         DESCRIPTION:
*//**       @brief         This Function is used for changing the bitrate


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ChangeBitrate(WFD_MM_HANDLE wfdmmhandle,
                                         OMX_S32 nBitrate)

{
    (void) wfdmmhandle;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::WFDMMSourceThread changing bitrate = %ld",
        nBitrate);
    if(nBitrate > 0  &&  nBitrate <= MAX_ALLOWED_BITRATE)
    {
#ifdef WFD_VDS_ARCH
       return(m_pVideoSource->ChangeBitrate(nBitrate));
#else
       return(m_pScreenSource->ChangeBitrate(nBitrate));
#endif
    }
    else
    {
      return OMX_ErrorBadParameter;
    }
}

/*==============================================================================

         FUNCTION:         ExecuteRunTimeCommand

         DESCRIPTION:
*//**       @brief         This Function is used to notify runtime command to
*                        mm lower layer

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None

*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ExecuteRunTimeCommand(
                    WFD_MM_HANDLE wfdmmhandle,WFD_runtime_cmd_t eCommand)
{
    (void) wfdmmhandle;
    OMX_ERRORTYPE retval = OMX_ErrorNone;

    switch (eCommand)
    {
      case WFD_MM_CMD_OPEN_AUDIO_PROXY:
      {
        // Keeping this in first phase for any OEM implementation of SM-A who may
        // call this function.
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
          "WFDMMSource:: calling OpenProxyDevice : Deprecated");

        break;
      }
      case WFD_MM_CMD_CLOSE_AUDIO_PROXY:
      {
        // Keeping this in first phase for any OEM implementation of SM-A who may
        // call this function.
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
          "WFDMMSource:: calling CloseProxyDevice : Deprecated");
        break;
      }
      case WFD_MM_CMD_ENABLE_BITRATE_ADAPT:
      {
        if (m_eState == MMWFDSRC_STATE_PLAY)
        {
          m_bBitrateAdaptEnabled = OMX_TRUE;
          if(!m_hLnkSpdTmr)
          {
            if(0 != MM_Timer_Create(MM_LINKSPEED_CHECK_INTERVAL, 1,
                                WFDMMSource::LinkSpeedCheckTimerCb,
                                (void *)this, &m_hLnkSpdTmr))
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                    "WFDMMSource:: Linkspeed Timer failed");
              return OMX_ErrorUndefined;
            }
            else
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                    "WFDMMSource::Linkspeed Timer success");
            }
          }
          else
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "WFDMMSource:: Linkspeed Timer Already Created Error");
            return OMX_ErrorSameState;
          }
        }
        else
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                       "WFDMMSource:: Invalid state for Linkspeed timer");
            return OMX_ErrorInvalidState;
        }
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
          "WFDMMSource:: Dynamic Bitrate Adaptation enabled");
        break;
      }
      case WFD_MM_CMD_DISABLE_BITRATE_ADAPT:
      {
        m_bBitrateAdaptEnabled = OMX_FALSE;
        if(m_hLnkSpdTmr)
        {
          int res = MM_Timer_Release(m_hLnkSpdTmr);
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSource::Releasing Linkspeed timer %d",res);
          m_hLnkSpdTmr = NULL;
        }
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
          "WFDMMSource:: Dynamic Bitrate Adaptation disabled");
        break;
      }
      default:
        break;
   }
   return retval;
}


/*==============================================================================

         FUNCTION:         ChangeFramerate

         DESCRIPTION:
*//**       @brief         This Function is used for changing the framerate


*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ChangeFramerate(WFD_MM_HANDLE wfdmmhandle,
                                           OMX_S32 nFramerate)

{
    (void)wfdmmhandle;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::changing framerate = %ld", nFramerate);
#ifdef WFD_VDS_ARCH
    return(m_pVideoSource->ChangeFrameRate(nFramerate));
#else
    return(m_pScreenSource->ChangeFrameRate(nFramerate));
#endif
}

/*==============================================================================

         FUNCTION:         GetCurrentVideoTimeStampMs

         DESCRIPTION:
*//**       @brief         This Function is used for gettting the TS


*//**

@par     DEPENDENCIES:    None

*//*
         PARAMETERS:
*//**       @param         WFD_MM_HANDLE  MM Handle
                           OMX_U32 *      TimeStamp


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::GetCurrentPTS(
                   WFD_MM_HANDLE wfdmmhandle,OMX_U64 *pTimeMs)

{
    (void) wfdmmhandle;
    if(!pTimeMs)
    {
        return OMX_ErrorBadParameter;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource:: Querying PTS %ld", m_nCurrentPTS);
    *pTimeMs = m_nCurrentPTS;
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         AudioSourceDeliveryFn

         DESCRIPTION:
*//**       @brief         Call back from the AudioSource


*//**

@par     DEPENDENCIES:   None

*//*
         PARAMETERS:
*//**       @param        pBuffer - Buffer to be delivered to sink
                           ptr -  WFDMMSource handle


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::AudioSourceDeliveryFn(OMX_BUFFERHEADERTYPE* pBuffer,
                                        void *ptr )
{

    if(ptr && !b_IsStopped)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource::AudioSourceDeliveryFn");
        WFDMMSource* pWFDMMSource = (WFDMMSource*) ptr;

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
            "WFDMMSource::AudioEncoderFillBufferDone %ld",
            pBuffer->nFilledLen );

        pBuffer->nInputPortIndex = PORT_INDEX_AUDIO;

        eError = pWFDMMSource->m_pSink->DeliverInput(
            (OMX_BUFFERHEADERTYPE *)pBuffer);

        pWFDMMSource->m_result = eError;

        if (eError != OMX_ErrorNone)
        {
          if (pWFDMMSource->m_pAudioSource)
          {
             pWFDMMSource->m_result = OMX_ErrorNone;
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                         "WFDMMSource::SourceDeliveryFn:Failed. Giving back buffer to AudioSource");
             pWFDMMSource->m_pAudioSource->SetFreeBuffer(
                    (OMX_BUFFERHEADERTYPE *)pBuffer);
          }
        }

    }
    else if(ptr)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                  "WFDMMSource::AudioSourceDeliveryFn After Stop, Return");
        WFDMMSource* pWFDMMSource = (WFDMMSource*) ptr;
        if (pWFDMMSource->m_pAudioSource)
        {
             pWFDMMSource->m_pAudioSource->SetFreeBuffer(
                    (OMX_BUFFERHEADERTYPE *)pBuffer);
        }
    }
    return;

}


  /*==============================================================================

         FUNCTION:         SourceDeliveryFn

         DESCRIPTION:
*//**       @brief         Call back from the VideoSource


*//**

@par     DEPENDENCIES:    None

*//*
         PARAMETERS:
*//**       @param        pBuffer - Buffer to be delivered to sink
                           ptr -  WFDMMSource handle


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::SourceDeliveryFn(OMX_BUFFERHEADERTYPE* pBuffer, void *ptr)
{
    WFDMMSource * pWFDMMSource = NULL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource::SourceDeliveryFn");
    if(ptr && !b_IsStopped)
    {
        pWFDMMSource = (WFDMMSource *)ptr;
        // get performance data
        if (pBuffer->nFilledLen != 0)
        {
            // if it's only the syntax header don't count it as a frame
            if ((pBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) == 0 &&
                (pBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME))
            {
                ++pWFDMMSource->m_nFramesCoded;
            }

            // always count the bits regarding whether or not its only syntax header
            pWFDMMSource->m_nBits = pWFDMMSource->m_nBits +
                (OMX_S32)(pBuffer->nFilledLen * 8);

            pBuffer->nInputPortIndex = PORT_INDEX_VIDEO;
            if(!b_IsStopped)
            {
                OMX_ERRORTYPE eError = OMX_ErrorNone;
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSource::SourceDeliveryFn  %lld",
                    ((OMX_BUFFERHEADERTYPE *)pBuffer)->nTimeStamp);
                eError = pWFDMMSource->m_pSink->DeliverInput(
                    (OMX_BUFFERHEADERTYPE *)pBuffer);
                pWFDMMSource->m_result = eError;
                if (eError != OMX_ErrorNone)
                {
#ifdef WFD_VDS_ARCH
                    if( pWFDMMSource->m_pVideoSource != NULL )
                    {
                        pWFDMMSource->m_result = OMX_ErrorNone;
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                "WFDMMSource::SourceDeliveryFn:Failed. Giving back buffer to VideoSource");
                        pWFDMMSource->m_pVideoSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
                    }
#else
                    if( pWFDMMSource->m_pScreenSource != NULL )
                    {
                        pWFDMMSource->m_result = OMX_ErrorNone;
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                "WFDMMSource::SourceDeliveryFn:Failed. Giving back buffer to ScreenSource");
                        pWFDMMSource->m_pScreenSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
                    }
#endif
                }
            }
        }
        else
        {
#ifdef WFD_VDS_ARCH
            if( pWFDMMSource->m_pVideoSource != NULL )
            {
                pWFDMMSource->m_result = OMX_ErrorNone;
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSource::SourceDeliveryFn:Failed. "
                            "Giving back buffer to VideoSource len=0");
                pWFDMMSource->m_pVideoSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
            }
#else
            if( pWFDMMSource->m_pScreenSource != NULL )
            {
                pWFDMMSource->m_result = OMX_ErrorNone;
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSource::SourceDeliveryFn:Failed."
                            " Giving back buffer to ScreenSource len=0");
                pWFDMMSource->m_pScreenSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
            }
#endif
        }

    }
    else if( ptr && b_IsStopped)
    {
      pWFDMMSource = (WFDMMSource *)ptr;
#ifdef WFD_VDS_ARCH
      if( pWFDMMSource->m_pVideoSource != NULL )
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::SourceDeliveryFn:Stop Received. Giving back buffer to VideoSource");
        pWFDMMSource->m_pVideoSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
      }
#else
      if( pWFDMMSource->m_pScreenSource != NULL )
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::SourceDeliveryFn:Stop Received. Giving back buffer to ScreenSource");
        pWFDMMSource->m_pScreenSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
      }
#endif
    }

}

/*==============================================================================

         FUNCTION:         SinkEmptyBufferDone

         DESCRIPTION:
*//**       @brief         Empty buffer done Call back from the sink


*//**

@par     DEPENDENCIES:    None

*//*
         PARAMETERS:
*//**       @param        hComponent - component handle
               pAppData - WFDMMSource handle
               pBuffer -  Buffer to be delivered to encoder


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::SinkEmptyBufferDone(
                                             OMX_IN OMX_HANDLETYPE hComponent,
                                             OMX_IN OMX_PTR pAppData,
                                             OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
        (void) hComponent;
        WFDMMSource* pWFDMMSource = (WFDMMSource*) pAppData;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource:SinkEmptyBufferDone");
        if (!pBuffer || !pAppData)
        {
            return OMX_ErrorBadParameter;
        }

        pWFDMMSource->updateCurrPTS(pBuffer->nTimeStamp);

        if(pBuffer->nInputPortIndex == 0)//AUDIO)
        {
            // Deliver free audio buffer to audio source
            pWFDMMSource->m_result =
                pWFDMMSource->m_pAudioSource->SetFreeBuffer(
                (OMX_BUFFERHEADERTYPE *)pBuffer);
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSource::WFDMMSourceThread Audio buffer for fill");
        }
        if(pBuffer->nInputPortIndex == 1)//VIDEO)
        {

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSource::WFDMMSourceThread Video buffer for fill");

            if(pBuffer->nFilledLen != pBuffer->nOffset)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSource::WFDMMSourceThread Video buffer not fully consumed request IDR");
                (void)pWFDMMSource->GenerateIFrameNext(pWFDMMSource);
            }
            // Deliver free yuv buffer to source
#ifdef WFD_VDS_ARCH
            pWFDMMSource->m_result = pWFDMMSource->m_pVideoSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
#else
            pWFDMMSource->m_result = pWFDMMSource->m_pScreenSource->SetFreeBuffer((OMX_BUFFERHEADERTYPE *)pBuffer);
#endif

        }
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
            "WFDMMSource:sinkEmptyBufferDone set m_WFDMMSourceFillThisBufferSignal");
        return pWFDMMSource->m_result;
    }

/*==============================================================================

         FUNCTION:         ExtractVideoParameters

         DESCRIPTION:
*//**       @brief         Extract video parameters from the pNegotiatedCap


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ExtractVideoParameters()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::ExtractVideoParameters");

    if(!m_pVidEncConfig)
    {
        m_pVidEncConfig = (VideoEncStaticConfigType *)
            MM_Malloc(sizeof(VideoEncStaticConfigType));
    }

    if(!m_pVidEncConfig)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::WFDMMSource m_pVidEncConfig failed");
    }

    if(!m_pVidEncDynmicConfig)
    {
        m_pVidEncDynmicConfig = (VideoEncDynamicConfigType*)
            MM_Malloc(sizeof(VideoEncDynamicConfigType));
    }

    if(!m_pVidEncDynmicConfig)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::WFDMMSource m_pVidEncDynmicConfig failed");
    }

    if(m_pVidEncConfig != NULL)
    {
        memset(m_pVidEncConfig, 0, sizeof(VideoEncStaticConfigType));
        if((m_pNegotiatedCap->video_method == WFD_VIDEO_H264)
           /*&& (m_pNegotiatedCap->video_config.video_config.num_h264_profiles > 0)*/)
        {
            WFD_h264_codec_config_t *pH264Config =
                m_pNegotiatedCap->video_config.video_config.h264_codec;

            if(!pH264Config)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "No valid h264 setting");
                return OMX_ErrorBadParameter;
            }

            m_pVidEncConfig->eCodec = OMX_VIDEO_CodingAVC;

            /**-------------------------------------------------------------------------
               Extract Resolution and bitrates from different modes
               ----------------------------------------------------------------------------
            */
            (void)getResolutionRefreshRate(
                pH264Config->supported_cea_mode,
                pH264Config->supported_vesa_mode,
                pH264Config->supported_hh_mode);

            m_pVidEncConfig->nOutputFrameWidth  =  m_pVidEncConfig->nFrameWidth;
            m_pVidEncConfig->nOutputFrameHeight =  m_pVidEncConfig->nFrameHeight;

            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                " WFDMMSource::After get Resolution rate %ld , %ld",
                m_pVidEncConfig->nFrameWidth,m_pVidEncConfig->nFrameHeight);

            if(pH264Config->h264_profile !=0)
                m_pVidEncConfig->nProfile = pH264Config->h264_profile;
            else
                m_pVidEncConfig->nProfile =1;
            if(pH264Config->h264_level !=0)
                m_pVidEncConfig->nLevel = pH264Config->h264_level;
            else
                m_pVidEncConfig->nLevel = 2;
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSource::ExtractVideoParameters Profile = %d, Level = %d",
                m_pVidEncConfig->nProfile,m_pVidEncConfig->nLevel);
            if(pH264Config->min_slice_size)
            {
                m_pVidEncConfig->nResyncMarkerSpacing =
                    (OMX_S32)pH264Config->min_slice_size;
                m_pVidEncConfig->eResyncMarkerType = RESYNC_MARKER_MB;
            }
            else
            {
                m_pVidEncConfig->nResyncMarkerSpacing = 0;
                m_pVidEncConfig->eResyncMarkerType = RESYNC_MARKER_NONE;
            }
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSource::ExtractVideoParameters nResyncMarkerSpacing = %ld,"
                 "eResyncMarkerType = %d",  m_pVidEncConfig->nResyncMarkerSpacing,
                 m_pVidEncConfig->eResyncMarkerType);

            MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSource::ExtractVideoParameters %ld %ld %ld",
                m_pVidEncConfig->nFramerate, m_pVidEncConfig->nFrameWidth,
                m_pVidEncConfig->nFrameHeight);
        }
        m_pVidEncConfig->eControlRate = OMX_Video_ControlRateVariable;
        m_pVidEncConfig->nDVSXOffset  = 0;
        m_pVidEncConfig->nDVSYOffset  = 0;
        m_pVidEncConfig->nBitrate     = WFDbitrate;
        m_nFloatingBitrate            = WFDbitrate;
        m_pVidEncConfig->nTimeIncRes  = m_pVidEncConfig->nFramerate;
        m_pVidEncConfig->nRotation    = 0;

        m_pVidEncConfig->nHECInterval = 0;

        m_pVidEncConfig->bEnableIntraRefresh = OMX_TRUE;
        m_pVidEncConfig->nFrames = m_pVidEncConfig->nFramerate;
        m_pVidEncConfig->bEnableShortHeader = OMX_FALSE;
        m_pVidEncConfig->nMinQp = 5;
        m_pVidEncConfig->nMaxQp = 51;
        m_pVidEncConfig->bProfileMode = OMX_FALSE;
        m_pVidEncConfig->bInUseBuffer= OMX_TRUE;
        m_pVidEncConfig->bOutUseBuffer= OMX_FALSE;
    }
    /**---------------------------------------------------------------------------
    Following settings can be used if we want to dynamically change any settings
    ------------------------------------------------------------------------------
    */
    if(m_pVidEncDynmicConfig != NULL)
    {
        memset(m_pVidEncDynmicConfig, 0, sizeof(VideoEncDynamicConfigType));
        m_pVidEncDynmicConfig->nIFrameRequestPeriod = 5;
        m_pVidEncDynmicConfig->nUpdatedBitrate = WFDbitrate; //14 mbps
        m_pVidEncDynmicConfig->nUpdatedMinQp = 5;
        m_pVidEncDynmicConfig->nUpdatedMaxQp = 51;//31;
        if(m_pVidEncConfig != NULL){
            m_pVidEncDynmicConfig->nUpdatedFramerate = m_pVidEncConfig->nFramerate;
            m_pVidEncDynmicConfig->nUpdatedFrames = m_pVidEncConfig->nFramerate;
            m_pVidEncDynmicConfig->nUpdatedIntraPeriod = m_pVidEncConfig->nFramerate * 2;
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSource::After extract video parameters %ld , %ld",
            m_pVidEncConfig->nFrameWidth,m_pVidEncConfig->nFrameHeight);
        }
    }
    return OMX_ErrorNone;
}
/*==============================================================================

         FUNCTION:         ExtractAudioParameters

         DESCRIPTION:
*//**       @brief         Extract Audio parameters from the pNegotiatedCap


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::ExtractAudioParameters()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::ExtractVideoParameters");


    if(!m_pAudioConfig)
    {
        m_pAudioConfig = (AudioEncConfigType*)
            MM_Malloc(sizeof(AudioEncConfigType));
    }
    if(!m_pAudioConfig)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "WFDMMSource::WFDMMSource m_pAudioConfig failed");
        return OMX_ErrorInsufficientResources;
    }

    memset(m_pAudioConfig, 0, sizeof(AudioEncConfigType));

    switch(m_pNegotiatedCap->audio_method)
    {
    case WFD_AUDIO_LPCM:
        {

            m_pAudioConfig->eCoding = OMX_AUDIO_CodingPCM;

            /**-----------------------------------------------------------------------
            Extract Sampling Rate, channels and .....
            ----------------------------------------------------------------------------
            */
            (void)getLPCMAudioParams(
                m_pNegotiatedCap->audio_config.lpcm_codec.supported_modes_bitmap);
        }
        break;

    case WFD_AUDIO_AAC:
        {
            m_pAudioConfig->eCoding = OMX_AUDIO_CodingAAC;
            /**-----------------------------------------------------------------------
            Extract Sampling Rate, channels and .....
            ----------------------------------------------------------------------------
            */
            (void)getAACAudioParams(
                m_pNegotiatedCap->audio_config.aac_codec.supported_modes_bitmap);
        }
        break;

        /**-------------------------------------------------------------------------
        Add support for following codecs.
        ----------------------------------------------------------------------------
        */
    case WFD_AUDIO_DOLBY_DIGITAL:
        m_pAudioConfig->eCoding = OMX_AUDIO_CodingAC3;
        (void)getAC3AudioParams(m_pNegotiatedCap->audio_config.dolby_digital_codec.supported_modes_bitmap);
        break;
    case WFD_AUDIO_INVALID:
    case WFD_AUDIO_UNK:
    default:
        m_pAudioConfig->eCoding = OMX_AUDIO_CodingUnused;
        break;
    }

    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         FillSinkParameters

         DESCRIPTION:
*//**       @brief         Fill sink parameters from the pNegotiatedCap


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        None


*//*     RETURN VALUE:
*//**       @return       OMX_ERRORTYPE


@par     SIDE EFFECTS:

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSource::FillSinkParameters()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::FillSinkParameters");

    if(!m_pNegotiatedCap)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "FillSinkParameters Nego Cap is NULL");
        return OMX_ErrorInsufficientResources;
    }

    if(!m_pMuxConfig)
    {
        m_pMuxConfig =  (MuxerConfigType*)MM_Malloc(sizeof(MuxerConfigType));
    }

    if(!m_pMuxConfig)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "FillSinkParameters Malloc failed");
        return OMX_ErrorInsufficientResources;
    }

    memset(m_pMuxConfig, 0, sizeof(MuxerConfigType));

    if(m_pMuxConfig)
    {
        //common parameters
        m_pMuxConfig->eFormat = QOMX_FORMATMPEG_TS;
        m_pMuxConfig->sOutputobj.outputType = MUXER_OUTPUT_RTP;

        if(m_pNegotiatedCap)
        {
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.ipaddr1 =
                m_pNegotiatedCap->peer_ip_addrs.ipv4_addr1;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.portno1 =
                m_pNegotiatedCap->transport_capability_config.port1_id;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.rtcpportno1 =
                m_pNegotiatedCap->transport_capability_config.port1_rtcp_id;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.qos_dscp1 =
                m_pNegotiatedCap->rtp_qos_settings.qos_dscp1;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.ipaddr2 =
                m_pNegotiatedCap->peer_ip_addrs.ipv4_addr2;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.portno2 =
                m_pNegotiatedCap->transport_capability_config.port2_id;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.qos_dscp2 =
                m_pNegotiatedCap->rtp_qos_settings.qos_dscp2;
            m_pMuxConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP =
               (m_pNegotiatedCap->transport_capability_config.eRtpPortType == RTP_PORT_UDP)? true : false;
        }

        if(m_pNegotiatedCap->HdcpCtx.hHDCPSession &&
           m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus == HDCP_STATUS_SUCCESS)
        {
            m_pMuxConfig->sEncryptParam.nStreamEncrypted = OMX_TRUE;
            m_pMuxConfig->sEncryptParam.nType = QOMX_ENCRYPT_TYPE_HDCP;
            /* setting to HDCP 2.1*/
            m_pMuxConfig->sEncryptParam.nEncryptVersion = 0x20;
        }
        else
        {
            m_pMuxConfig->sEncryptParam.nStreamEncrypted = OMX_FALSE;
            m_pMuxConfig->sEncryptParam.nType = QOMX_ENCRYPT_TYPE_INVALID;
            m_pMuxConfig->sEncryptParam.nEncryptVersion = 0x00;
        }
        if(!m_pVidEncConfig)
        {
            (void)ExtractVideoParameters();
        }

        if( m_pVidEncConfig )
        {
            /**-----------------------------------------------------------------------
            TODO- Need to add interface for 3D video at mux interface
            --------------------------------------------------------------------------
            */
            if(m_pVidEncConfig->eCodec == OMX_VIDEO_CodingAVC)
            {
                m_pMuxConfig->svideoParams.format     = MUX_VIDEO_H264;
                m_pMuxConfig->svideoParams.frame_rate = (OMX_S16)m_pVidEncConfig->nFramerate;
                m_pMuxConfig->svideoParams.height     = (OMX_S16)m_pVidEncConfig->nFrameHeight;
                m_pMuxConfig->svideoParams.width      = (OMX_S16)m_pVidEncConfig->nFrameWidth;
                m_pMuxConfig->svideoParams.bitrate    = m_pVidEncConfig->nBitrate;
                m_pMuxConfig->svideoParams.eH264Level =
                    (OMX_VIDEO_AVCLEVELTYPE)((m_pNegotiatedCap->video_config.
                    video_config.h264_codec)+ 0)->h264_level;
                m_pMuxConfig->svideoParams.eH264Profile =
                    (OMX_VIDEO_AVCPROFILETYPE)((m_pNegotiatedCap->video_config.
                    video_config.h264_codec)+ 0)->h264_profile;
            }
            else
            {
                m_pMuxConfig->svideoParams.format = MUX_VIDEO_NONE;
            }
        }
        else
        {
            m_pMuxConfig->svideoParams.format = MUX_VIDEO_NONE;
        }

        if(!m_pAudioConfig)
        {
            (void)ExtractAudioParameters();
        }

        if( m_pAudioConfig )
        {
            /**-----------------------------------------------------------------------
            TODO add DTS and DOLBY codecs support at Mux interface
            --------------------------------------------------------------------------
            */
            if(m_pAudioConfig->eCoding == OMX_AUDIO_CodingPCM)
            {
                m_pMuxConfig->sAudioParams.format = MUX_AUDIO_PCM;
            }
            else if (m_pAudioConfig->eCoding == OMX_AUDIO_CodingAAC)
            {
                m_pMuxConfig->sAudioParams.format = MUX_AUDIO_MPEG4_AAC;
                m_pMuxConfig->sAudioParams.aac_params.
                    aac_scalefactor_data_resilience_flag = OMX_FALSE;
                m_pMuxConfig->sAudioParams.aac_params.
                    aac_section_data_resilience_flag     = OMX_FALSE;
                m_pMuxConfig->sAudioParams.aac_params.
                    aac_spectral_data_resilience_flag = OMX_FALSE;
                m_pMuxConfig->sAudioParams.aac_params.eAACProfile
                    = OMX_AUDIO_AACObjectLC;
                m_pMuxConfig->sAudioParams.aac_params.eAACStreamFormat
                    = OMX_AUDIO_AACStreamFormatMP4ADTS;

                m_pMuxConfig->sAudioParams.aac_params.eChannelMode
                    = OMX_AUDIO_ChannelModeStereo;
                m_pMuxConfig->sAudioParams.aac_params.ep_config = 0;
                m_pMuxConfig->sAudioParams.aac_params.sbr_present_flag = OMX_FALSE;
            }
            else if (m_pAudioConfig->eCoding == OMX_AUDIO_CodingAC3)
            {
                m_pMuxConfig->sAudioParams.format = MUX_AUDIO_AC3;
            }
            else
            {
                m_pMuxConfig->sAudioParams.format = MUX_AUDIO_NONE;
            }
            /**-----------------------------------------------------------------------
            Audio common settings.
            --------------------------------------------------------------------------
            */
            m_pMuxConfig->sAudioParams.sampling_frequency =
                m_pAudioConfig->nSamplingFrequency;
            m_pMuxConfig->sAudioParams.num_channels =
                m_pAudioConfig->nNumChannels;
            m_pMuxConfig->sAudioParams.bits_per_sample =
                m_pAudioConfig->nBitsPerSample;
            m_pMuxConfig->sAudioParams.block_align =
                m_pAudioConfig->nBlockAlign;
            m_pMuxConfig->sAudioParams.bitrate =
                m_pAudioConfig->nBitrate;
        }
        else
        {
            m_pMuxConfig->sAudioParams.format = MUX_AUDIO_NONE;
        }
    }
    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:         getResolutionRefreshRate

         DESCRIPTION:
*//**       @brief         Get Resolution and refresh rate from the cea_mode


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        cea_mode - cea_mode value received via pNegotiatedCap


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::getResolutionRefreshRate(uint32 cea_mode,
                                           uint32 vesa_mode,
                                           uint32 hh_mode )
{
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSource::getResolutionRefreshRate %d %d %d",
        cea_mode, vesa_mode, hh_mode);
    m_nMinBitrate = 0;
    m_nMaxBitrate = 0;
    int tempMinBitrate = 0;
    int tempMaxBitrate = 0;
    int nEnabled = 0;
    getCfgItem(DYN_BIT_ADAP_KEY, &nEnabled);

    if(nEnabled)
    {
        m_bBitrateAdaptEnabled = OMX_TRUE;
    }

    if(cea_mode)
    {
        switch (cea_mode)
        {
        case 1:
            m_pVidEncConfig->nFrameWidth = 640;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 640;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 2:
            m_pVidEncConfig->nFrameWidth = 720;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 720;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 4:
            //480i
            m_pVidEncConfig->nFrameWidth = 720;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 720;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 8:
            m_pVidEncConfig->nFrameWidth = 720;
            m_pVidEncConfig->nFrameHeight = 576;
            m_pVidEncConfig->nFramerate = 50;
            output_width = 720;
            output_height = 576;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 16:
            //576i
            m_pVidEncConfig->nFrameWidth = 720;
            m_pVidEncConfig->nFrameHeight = 576;
            m_pVidEncConfig->nFramerate = 50;
            output_width = 720;
            output_height = 576;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 32:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 720;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1280;
            output_height = 720;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 64:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 720;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1280;
            output_height = 720;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 128:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 14 * ONE_MBPS;
            m_nMinBitrate = 6 * ONE_MBPS;
            break;
        case 256:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 512:
            //1080i 60
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 1024:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 720;
            m_pVidEncConfig->nFramerate = 25;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 2048:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 720;
            m_pVidEncConfig->nFramerate = 50;
            output_width = 1280;
            output_height = 720;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 4096:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 25;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 8192:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 50;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 16384:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 50;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 32768:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 720;
            m_pVidEncConfig->nFramerate = 24;
            output_width = 1280;
            output_height = 720;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 65536:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1080;
            m_pVidEncConfig->nFramerate = 24;
            output_width = 1920;
            output_height = 1080;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;

            /** Test cases proprietary for debug purpose */
        case -1:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 4096000;
            m_nMinBitrate = 4096000 >> 2;
            break;
        case -2:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 8192000;
            m_nMinBitrate = 8192000 >> 2;
            break;
        case -3:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 14000000;
            m_nMinBitrate = 14000000 >> 2;
            break;
        case -4:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 2048000;
            m_nMinBitrate = 2048000 >> 2;
            break;
        case -5:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 4096000;
            m_nMinBitrate = 4096000 >> 2;
            break;

        default:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 2048000;
            m_nMinBitrate = 2048000 >> 2;
            break;
        }

        WFDbitrate = m_nMaxBitrate;

        {
            ParseCfgForBitrate(WFD_MM_CEA_MODE, getMaxBitSet(cea_mode),(char *)WFD_CFG_FILE,
                               &tempMinBitrate, &tempMaxBitrate);
            m_nMinBitrate = tempMinBitrate;
            if (tempMaxBitrate <= m_nMaxBitrate)
            {
                m_nMaxBitrate = tempMaxBitrate;
            }
            if(m_nMaxBitrate == 0)
            {
                m_nMaxBitrate = WFDbitrate;
            }
            if(m_nMaxBitrate <=  WFDbitrate)
            {
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMsource Supplied NomBitrate and WFDBitrate %ld, %d",
                m_nMaxBitrate,WFDbitrate);
                WFDbitrate = m_nMaxBitrate;
            }
            if(m_nMinBitrate == 0)
            {
                m_nMinBitrate = WFDbitrate >> 2;
            }
        }
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, " MM Bitrates Nominal = %ld, Minimum = %ld",
               m_nMaxBitrate, m_nMinBitrate);
    }
    else if(hh_mode)
    {
        switch (hh_mode)
        {
        case 1:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 2:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 4:
            m_pVidEncConfig->nFrameWidth = 854;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 854;
            output_height = 480;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 8:
            m_pVidEncConfig->nFrameWidth = 854;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 854;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;

        case 16:
            m_pVidEncConfig->nFrameWidth = 864;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 864;
            output_height = 480;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 32:
            m_pVidEncConfig->nFrameWidth = 864;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 864;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 64:
            m_pVidEncConfig->nFrameWidth = 640;
            m_pVidEncConfig->nFrameHeight = 360;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 640;
            output_height = 360;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 128:
            m_pVidEncConfig->nFrameWidth = 640;
            m_pVidEncConfig->nFrameHeight = 360;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 640;
            output_height = 360;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 256:
            m_pVidEncConfig->nFrameWidth = 960;
            m_pVidEncConfig->nFrameHeight = 540;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 960;
            output_height = 540;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 512:
            m_pVidEncConfig->nFrameWidth = 960;
            m_pVidEncConfig->nFrameHeight = 540;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 960;
            output_height = 540;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 1024:
            m_pVidEncConfig->nFrameWidth = 848;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 848;
            output_height = 480;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 2048:
            m_pVidEncConfig->nFrameWidth = 848;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 848;
            output_height = 480;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;

        default:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 480;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 480;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        }

        WFDbitrate = m_nMaxBitrate;

        {
            ParseCfgForBitrate(WFD_MM_HH_MODE, getMaxBitSet(hh_mode), (char *)WFD_CFG_FILE,
                               (int*)&tempMinBitrate, (int*)&tempMaxBitrate);
            m_nMinBitrate = tempMinBitrate;
            if (tempMaxBitrate <= m_nMaxBitrate)
            {
                m_nMaxBitrate = tempMaxBitrate;
            }
            if(m_nMaxBitrate == 0)
            {
                m_nMaxBitrate = WFDbitrate;
            }
            if(m_nMaxBitrate <=  WFDbitrate)
            {
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMsource Supplied NomBitrate and WFDBitrate %ld, %d",
                m_nMaxBitrate,WFDbitrate);
                WFDbitrate = m_nMaxBitrate;
            }
            if(m_nMinBitrate == 0)
            {
                m_nMinBitrate = WFDbitrate >> 2;
            }
        }
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, " MM Bitrates Nominal = %ld, Minimum = %ld",
                     m_nMaxBitrate, m_nMinBitrate);
    }
    else if(vesa_mode)
    {
        switch (vesa_mode)
        {
        case 1:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 600;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 600;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        case 2:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 600;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 800;
            output_height = 600;
            m_nMaxBitrate = 4 * ONE_MBPS;
            m_nMinBitrate = 1 * ONE_MBPS;
            break;
        case 4:
            m_pVidEncConfig->nFrameWidth = 1024;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1024;
            output_height = 768;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 8:
            m_pVidEncConfig->nFrameWidth = 1024;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1024;
            output_height = 768;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 16:
            m_pVidEncConfig->nFrameWidth = 1152;
            m_pVidEncConfig->nFrameHeight = 864;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1152;
            output_height = 864;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 32:
            m_pVidEncConfig->nFrameWidth = 1152;
            m_pVidEncConfig->nFrameHeight = 864;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1152;
            output_height = 864;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 64:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1280;
            output_height = 768;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 128:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1280;
            output_height = 768;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 256:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 800;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1280;
            output_height = 800;
            m_nMaxBitrate = 14 * ONE_MBPS;
            m_nMinBitrate = (7 * ONE_MBPS)>>1;
            break;

        case 512:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 800;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1280;
            output_height = 800;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 1024:
            m_pVidEncConfig->nFrameWidth = 1360;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1360;
            output_height = 768;
            m_nMaxBitrate = 14 * ONE_MBPS;
            m_nMinBitrate = (7 * ONE_MBPS)>>1;
            break;
        case 2048:
            m_pVidEncConfig->nFrameWidth = 1360;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1360;
            output_height = 768;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 4096:
            m_pVidEncConfig->nFrameWidth = 1366;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1366;
            output_height = 768;
            m_nMaxBitrate = 10 * ONE_MBPS;
            m_nMinBitrate = (5 * ONE_MBPS) / 2;
            break;
        case 8192:
            m_pVidEncConfig->nFrameWidth = 1366;
            m_pVidEncConfig->nFrameHeight = 768;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1366;
            output_height = 768;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 16384:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 1024;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1280;
            output_height = 1024;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 32768:
            m_pVidEncConfig->nFrameWidth = 1280;
            m_pVidEncConfig->nFrameHeight = 1024;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1280;
            output_height = 1024;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 65536:
            m_pVidEncConfig->nFrameWidth = 1400;
            m_pVidEncConfig->nFrameHeight = 1050;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1400;
            output_height = 1050;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;

        case 131072:
            m_pVidEncConfig->nFrameWidth = 1400;
            m_pVidEncConfig->nFrameHeight = 1050;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1400;
            output_height = 1050;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 262144:
            m_pVidEncConfig->nFrameWidth = 1440;
            m_pVidEncConfig->nFrameHeight = 900;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1440;
            output_height = 900;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;

        case 524288:
            m_pVidEncConfig->nFrameWidth = 1440;
            m_pVidEncConfig->nFrameHeight = 900;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1440;
            output_height = 900;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 1048576:
            m_pVidEncConfig->nFrameWidth = 1600;
            m_pVidEncConfig->nFrameHeight = 900;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1600;
            output_height = 900;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 2097152:
            m_pVidEncConfig->nFrameWidth = 1600;
            m_pVidEncConfig->nFrameHeight = 900;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1600;
            output_height = 900;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 4194304:
            m_pVidEncConfig->nFrameWidth = 1600;
            m_pVidEncConfig->nFrameHeight = 1200;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1600;
            output_height = 1200;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 8388608:
            m_pVidEncConfig->nFrameWidth = 1600;
            m_pVidEncConfig->nFrameHeight = 1200;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1600;
            output_height = 1200;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 16777216:
            m_pVidEncConfig->nFrameWidth = 1680;
            m_pVidEncConfig->nFrameHeight = 1024;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1680;
            output_height = 1024;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 33554432:
            m_pVidEncConfig->nFrameWidth = 1680;
            m_pVidEncConfig->nFrameHeight = 1024;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1680;
            output_height = 1024;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 67108864:
            m_pVidEncConfig->nFrameWidth = 1680;
            m_pVidEncConfig->nFrameHeight = 1050;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1680;
            output_height = 1050;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 134217728:
            m_pVidEncConfig->nFrameWidth = 1680;
            m_pVidEncConfig->nFrameHeight = 1050;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1680;
            output_height = 1050;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        case 268435456:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1200;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 1920;
            output_height = 1200;
            m_nMaxBitrate = 20 * ONE_MBPS;
            m_nMinBitrate = 5 * ONE_MBPS;
            break;
        case 536870912:
            m_pVidEncConfig->nFrameWidth = 1920;
            m_pVidEncConfig->nFrameHeight = 1200;
            m_pVidEncConfig->nFramerate = 60;
            output_width = 1920;
            output_height = 1200;
            m_nMaxBitrate = 40 * ONE_MBPS;
            m_nMinBitrate = 10 * ONE_MBPS;
            break;
        default:
            m_pVidEncConfig->nFrameWidth = 800;
            m_pVidEncConfig->nFrameHeight = 600;
            m_pVidEncConfig->nFramerate = 30;
            output_width = 800;
            output_height = 600;
            m_nMaxBitrate = 2 * ONE_MBPS;
            m_nMinBitrate = (ONE_MBPS) >> 1;
            break;
        }
        WFDbitrate = m_nMaxBitrate;

        {
            ParseCfgForBitrate(WFD_MM_VESA_MODE, getMaxBitSet(vesa_mode),
                              (char *)WFD_CFG_FILE,&tempMinBitrate,
                              &tempMaxBitrate);
            m_nMinBitrate = tempMinBitrate;
            if (tempMaxBitrate <= m_nMaxBitrate)
            {
                m_nMaxBitrate = tempMaxBitrate;
            }
            if(m_nMaxBitrate == 0)
            {
                m_nMaxBitrate = WFDbitrate;
            }
            if(m_nMaxBitrate <=  WFDbitrate)
            {
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMsource Supplied NomBitrate and WFDBitrate %ld, %d",
                m_nMaxBitrate,WFDbitrate);
                WFDbitrate = m_nMaxBitrate;
            }
            if(m_nMinBitrate == 0)
            {
                m_nMinBitrate = WFDbitrate >> 2;
            }
        }
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, " MM Bitrates Nominal = %ld, Minimum = %ld",
                     m_nMaxBitrate, m_nMinBitrate);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "WFDMMSource::No Modes are available setting  default values");
        m_pVidEncConfig->nFrameWidth = 640;
        m_pVidEncConfig->nFrameHeight = 480;
        m_pVidEncConfig->nFramerate = 60;
        output_width = 640;
        output_height = 480;
        WFDbitrate = 4 * ONE_MBPS;
    }
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
        "getResolutionRefreshRate width= %d, height= %d frame rate = %d",
        output_width, output_height, nInFrameRate);
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
        "getResolutionRefreshRate encoder frame width= %ld,\
        encoder frame height= %ld encoder frame rate = %ld",
        m_pVidEncConfig->nFrameWidth, m_pVidEncConfig->nFrameHeight,
        m_pVidEncConfig->nFramerate);
}


/*==============================================================================

         FUNCTION:         getLPCMAudioParams

         DESCRIPTION:
*//**       @brief         Get LPCM Audio parameters


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        supported_audio_mode - supported Audio modes


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
void WFDMMSource::getLPCMAudioParams(uint32 supported_audio_mode )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::getAudioParams");
    uint16 temp = (supported_audio_mode & 0X00FFF);
    switch (temp)
    {
    case 1:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 44100;
        m_pAudioConfig->nBitsPerSample = 16;
        break;
    case 2:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        break;
    default:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        break;
    }
    m_pAudioConfig->nBitrate = m_pAudioConfig->nSamplingFrequency *
        m_pAudioConfig->nBitsPerSample     *
        m_pAudioConfig->nNumChannels;
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
void WFDMMSource::getAACAudioParams(uint32 supported_audio_mode )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::getAACAudioParams");
    uint16 temp = (supported_audio_mode & 0X00FFF);
    switch (temp)
    {
    case 1:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 256000;
        break;
    case 2:
        m_pAudioConfig->nNumChannels = 4;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 384000;
        break;
    case 4:
        m_pAudioConfig->nNumChannels = 6;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 512000;
        break;
    case 8:
        m_pAudioConfig->nNumChannels = 8;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 512000;
        break;
    default:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 128000;
        break;
    }

    int nMinBitrate = 0;
    int nMaxBitrate = 0;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "AAC Mode = %d", temp);
    ParseCfgForBitrate(WFD_MM_AAC_MODE, getMaxBitSet(temp),
                              (char *)WFD_CFG_FILE, (int*)&nMinBitrate, (int*)&nMaxBitrate);

    if(nMaxBitrate > 0 && nMaxBitrate < 1536000)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "Override AAC bitrate from cfg file %ld -> %d",
                     m_pAudioConfig->nBitrate, nMaxBitrate);
        m_pAudioConfig->nBitrate = nMaxBitrate;
    }

}


void WFDMMSource::getAC3AudioParams(uint32 supported_audio_mode )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::getAC3AudioParams");
    uint16 temp = (supported_audio_mode & 0X00FFF);
    switch (temp)
    {
    case 1:
        m_pAudioConfig->nNumChannels = 2;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 256000;
        break;
    case 2:
        m_pAudioConfig->nNumChannels = 4;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 384000;
        break;
    case 4:
        m_pAudioConfig->nNumChannels = 6;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 768000;
        break;
    case 8:
        m_pAudioConfig->nNumChannels = 8;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 1024000;
        break;
    default:
        m_pAudioConfig->nNumChannels = 6;
        m_pAudioConfig->nSamplingFrequency = 48000;
        m_pAudioConfig->nBitsPerSample = 16;
        m_pAudioConfig->nBitrate = 768000;
        break;
    }

    int nMinBitrate = 0;
    int nMaxBitrate = 0;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "AC3 Mode = %d", temp);
    ParseCfgForBitrate(WFD_MM_AAC_MODE, getMaxBitSet(temp),
                              (char *)WFD_CFG_FILE, (int*)&nMinBitrate, (int*)&nMaxBitrate);

    if(nMaxBitrate > 0 && nMaxBitrate < 1536000)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "Override AAC bitrate from cfg file %ld -> %d",
                     m_pAudioConfig->nBitrate, nMaxBitrate);
        m_pAudioConfig->nBitrate = nMaxBitrate;
    }

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
void WFDMMSource::eventHandlerCb(void *pThis, OMX_U32 nModuleId,
                      WFDMMSourceEventType nEvent,
                      OMX_ERRORTYPE nStatus, void* nData)
{
    if(!pThis)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Invalid Me, can't handle device callback");
        return;
    }

    ((WFDMMSource *)pThis)->eventHandler(nModuleId,
                      nEvent, nStatus, (int64)nData);
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
void WFDMMSource::eventHandler(OMX_U32 nModuleId,
                      WFDMMSourceEventType nEvent,
                      OMX_ERRORTYPE nStatus, int64 nData)
{
    (void)nModuleId;
    (void)nStatus;
    switch(nEvent)
    {
    case WFDMMSRC_RTP_RTCP_PORT_UPDATE:
    case WFDMMSRC_RTP_SESSION_START:
        {//TODO fix the warning below
            MUX_RTPinfo *rtpInfo =  (MUX_RTPinfo *)nData;
            WFD_transport_capability_config_t transportInfo = {0,0,0,0,0,0,RTP_PORT_UDP};
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                          "RTP SESSION START callback from module %ld",
                         nModuleId);

            transportInfo.port1_id = (uint16)rtpInfo->portno1;
            transportInfo.port1_rtcp_id = (uint16)rtpInfo->rtcpportno1;

            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "RTP Update Server Ports %d, %d",
                        transportInfo.port1_id, transportInfo.port1_rtcp_id);

            sCallBacks.update_event_cb(WFD_EVENT_MM_RTP,
                                       WFD_STATUS_READY, (void*)(&transportInfo));
        }
        break;
    case WFDMMSRC_VIDEO_SESSION_START:
        {
           /* Update the current Session Info to the session manager */
           struct mmSessionInfo
           {
               int height;
               int width;
               int hdcp;
               long producer;
               //Add audio params as needed
            }sMMInfo;

            sMMInfo.hdcp = 0;
            sMMInfo.height =(int) m_pVidEncConfig->nFrameHeight;
            sMMInfo.width =(int) m_pVidEncConfig->nFrameWidth;
#ifdef WFD_VDS_ARCH
            if(m_pVideoSource)
            {
               sMMInfo.producer = reinterpret_cast<long>((m_pVideoSource->getSurface()));
               MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
               "WFDMMSRC_VIDEO_SESSION_START from module %ld %ld",nModuleId,sMMInfo.producer);
            }
#else
            sMMInfo.producer = 0;
#endif
            if (m_pNegotiatedCap->HdcpCtx.hHDCPSession &&
              m_pNegotiatedCap->HdcpCtx.hHDCPSession->m_eHdcpSessionStatus ==
               HDCP_STATUS_SUCCESS)
            {
                sMMInfo.hdcp = 1;
            }
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_SESSION_EVENT,
                                   WFD_STATUS_SUCCESS, (void*)(&sMMInfo));
            }
        }
        break;
    case WFDMMSRC_ERROR:

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Error callback from module %ld",
                     nModuleId);
        if(m_IsErrorHandling == OMX_TRUE || b_IsStopped)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Already recovering from Error or being stopped");
            return;
        }
        //WFD MM has encountered a non recoverable run time error.
        // Instance must be destroyed from client.
        m_IsErrorHandling = OMX_TRUE;

        if(nModuleId == WFDMM_AUDIOSRC_MODULE_ID)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO, WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == WFDMM_SCREENSRC_MODULE_ID)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_VIDEO, WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == WFDMM_TSMUX_MODULE_ID)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_RTP, WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        else if(nModuleId == WFDMM_HDCP_MODULE_ID)
        {
            if(sCallBacks.update_event_cb)
            {
                sCallBacks.update_event_cb(WFD_EVENT_MM_HDCP, WFD_STATUS_RUNTIME_ERROR, NULL);
            }
        }
        break;
    case WFDMMSRC_RTCP_RR_MESSAGE:
        if(sCallBacks.update_event_cb)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCP RR message");
            sCallBacks.update_event_cb(WFD_EVENT_MM_RTP,
                                       WFD_STATUS_RTCP_RR_MESSAGE,reinterpret_cast<void*>(nData));
        }
        break;
    case WFDMMSRC_AUDIO_PROXY_OPEN_DONE:
       if(sCallBacks.update_event_cb)
       {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                     "eventHandler: WFD_STATUS_PROXY_OPENED");
           sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO,
                      WFD_STATUS_PROXY_OPENED, NULL);
       }
       break;
    case WFDMMSRC_AUDIO_PROXY_CLOSE_DONE:
      if(sCallBacks.update_event_cb)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "eventHandler: WFD_STATUS_PROXY_CLOSED");
          sCallBacks.update_event_cb(WFD_EVENT_MM_AUDIO,
                      WFD_STATUS_PROXY_CLOSED, NULL);
      }
      break;

    default:
        /*Handle an Unknown Event???? Give me a*/break; //TODO

    }

}

/*==============================================================================

         FUNCTION:         getMaxBitSet

         DESCRIPTION:
*//**       @brief         Get Position of the largest bit set


*//**

@par     DEPENDENCIES:      None

*//*
         PARAMETERS:
*//**       @param        word - uint32 value


*//*     RETURN VALUE:
*//**       @return       None


@par     SIDE EFFECTS:

*//*==========================================================================*/
int WFDMMSource::getMaxBitSet(uint32 word)
{
    int pos = -1;

    while(word)
    {
        word >>= 1;
	    pos++;
    }
    return pos;
}

