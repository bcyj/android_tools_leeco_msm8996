/* =======================================================================
                              WFDMMSourceScreenSource.cpp
DESCRIPTION

This module is for WFD source implementation
Takes care of interaction with V4L2

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/src/WFDMMSourceScreenSource.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */

/*========================================================================
 *                             Include Files
 *==========================================================================*/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WFDMMSourceScreenSource"

#include "WFDMMSourceSignalQueue.h"
#include "WFDMMSourceScreenSource.h"
#include "MMDebugMsg.h"
#include "WFDMMSourcePmem.h"
#include "QOMX_VideoExtensions.h"
#include "WFD_HdcpCP.h"
#include "wfd_cfg_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/fb.h>
#include <linux/msm_mdp.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "AEEStdDef.h"
#include "MMMalloc.h"
#include <cutils/properties.h>

#ifndef WFD_ICS
#include "common_log.h"
#endif

#include <threads.h>
#include <poll.h>
#include "display_config.h"


#ifdef WFD_ICS
#include <surfaceflinger/SurfaceComposerClient.h>
#endif


/*========================================================================
 *                          Defines/ Macro
 *==========================================================================*/
// Events the ScreenSource working thread uses
const OMX_U32 ScreenSource::SCREEN_SOURCE_PLAY_EVENT = 1;
const OMX_U32 ScreenSource::SCREEN_SOURCE_PAUSE_EVENT = 2;
const OMX_U32 ScreenSource::SCREEN_SOURCE_EXIT_EVENT = 5;

// Screen source thread priority
#define WFD_MM_SCREEN_SOURCE_THREAD_PRIORITY -14
#define WFD_MM_SOURCE_THREAD_STACK_SIZE 16384
#define WFD_MM_POLL_THREAD_PRIORITY -2
#define WFD_MM_SOURCE_POLL_THREAD_STACK_SIZE 16384
#define WFD_MM_PCR_INTERVAL 100
#define EXT_DISPLAY_WIFI 2

#define VIDEO_PVTDATA_TYPE_FILLERNALU           0
#define VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED 1
#define PES_PVT_DATA_LEN 16
#define MAX_VIDEO_DEVICES 1
#define MAX_DEVICE_NAME_SIZE 64
#define MAX_FRAME_BUFFER_NAME_SIZE 80
#define BUFFER_EXTRA_DATA 1024
#define MAX_BUFFER_ASSUME 10
#define MAX_DISPLAY_PANELS 3

#define WFD_H264_PROFILE_CONSTRAINED_HIGH 2
#define FRAME_SKIP_FPS_VARIANCE 20
// Frame skipping delay
uint32 video_frame_skipping_start_delay = 0;

/**!
 * @brief Helper macro to set private/internal flag
 */
#define FLAG_SET(_pCtx_, _f_) (_pCtx_)->nFlags  |= (_f_)

/**!
 * @brief Helper macro to check if a flag is set or not
 */
#define FLAG_ISSET(_pCtx_, _f_) (((_pCtx_)->nFlags & (_f_)) ? OMX_TRUE : OMX_FALSE)

/**!
 * @brief Helper macro to clear a private/internal flag
 */
#define FLAG_CLEAR(_pCtx_, _f_) (_pCtx_)->


static const char  gDisplayPanels
          [MAX_DISPLAY_PANELS]
          [MAX_FRAME_BUFFER_NAME_SIZE] =
{
    "/dev/graphics/fb0",
    "/dev/graphics/fb1",
    "/dev/graphics/fb2",
};

static const char frameBufferName
           [MAX_DISPLAY_PANELS]
           [MAX_FRAME_BUFFER_NAME_SIZE] =
{
    "mipi dsi video panel",
    "dtv panel",
    "writeback panel"
};

OMX_U8 ScreenSource::sFillerNALU[FILLER_NALU_SIZE] =
{0x00, 0x00, 0x00, 0x01, 0x0c, 0xff, 0xff, 0x80};

static const int V4L2_events[] = {
          V4L2_EVENT_MSM_VIDC_SYS_ERROR,
};

inline OMX_BOOL IS_DUMP_ENABLE() {
    int ret = 0;
    char szTemp[PROPERTY_VALUE_MAX];
    ret = property_get("persist.debug.wfd.dumpv4l2",szTemp,NULL);
    if (ret <= 0 )
    {
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,"Failed to read prop %d %s value", ret, szTemp);
        return OMX_FALSE;
    }
    if(strcmp(szTemp,"1")==0)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_DEBUG,"IS_DUMP_ENABLE OMX_TRUE");
        return OMX_TRUE;
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_DEBUG,"IS_DUMP_ENABLE OMX_FALSE");
        return OMX_FALSE;
    }
}


/*!*************************************************************************
 * @brief     CTOR
 *
 * @param[in] NONE
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
ScreenSource::ScreenSource()
    : m_pBuffers(NULL),
    m_nFrameWidth(0),
    m_nFrameHeight(0),
    m_nDVSXOffset(0),
    m_nDVSYOffset(0),
    m_nV4L2QueueSize(0),
    m_nMuxBufferCount(0),
    m_ScreenSourceThreadHandle(NULL),
    m_bStarted(OMX_FALSE),
    m_bPause(OMX_FALSE),
    m_nMirrorHint(-1),
    m_nFBPanelIdx(0),
    m_pFrameDeliverFn(NULL),
    m_fd(-1),
    ionfd(-1),
    fd_fb(-1),
    m_pscreensourceOutputBuffers(NULL),
    m_numBuffers(0),
    m_signalQ(NULL),
    m_ScreenSourcePollThreadHandle(NULL),
    m_WFDScreenSourcePlaySignal(NULL),
    m_WFDScreenSourcePauseSignal(NULL),
    m_WFDScreenSourceExitSignal(NULL),
    m_secureSession(OMX_FALSE),
    m_nMuxBuffersSent(0),
    m_nMuxBuffersReceived(0),
    m_nFillerInFd(-1),
    m_nFillerOutFd(-1),
    m_hFillerInHandle(NULL),
    m_hFillerOutHandle(NULL),
    m_pFillerDataInPtr(NULL),
    m_pFillerDataOutPtr(NULL),
    m_bFillerNaluEnabled(OMX_TRUE),
    m_bPollContinue(OMX_TRUE)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource::Creating ScreenSource...");
    m_bStreamON = OMX_FALSE;
    bKeepGoing = OMX_FALSE;
    bRunning = OMX_FALSE;
    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_INIT;
    for(int bufferIndex = 0; bufferIndex< SCREEN_SOURCE_BUFFERS; bufferIndex++)
    {
      isBufWithV4l2[bufferIndex] = 0;
      bufQTimestamp[bufferIndex] = 0;
      bufDQTimestamp[bufferIndex] = 0;
    }
    m_pHdcpHandle = NULL;
    m_bHdcpSessionValid = 0;
    m_pHdcpOutBuffers = NULL;
    /**---------------------------------------------------------------------
         Decision to encrypt non secure content or not is made by application
         or user based on the WFD config file
        ------------------------------------------------------------------------
       */
    int32 nVal;
    nVal = 0;
    // CHeck if Filler NALU is disabled
    getCfgItem(DISABLE_NALU_FILLER_KEY,(int*)(&nVal));
    if(nVal == 1)
    {
        m_bFillerNaluEnabled = OMX_FALSE;
    }

    m_bCritTransact = OMX_FALSE;
    #ifdef ENABLE_WFD_STATS
    memset(&wfdStats,0,sizeof(wfd_stats_struct));
    m_pStatTimer = NULL;
    wfdStats.bEnableWfdStat = OMX_TRUE;
    m_nDuration = 5000;
    if(0 != MM_Timer_Create( m_nDuration, 1, readStatTimerHandler, (void *)(this), &m_pStatTimer))
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Creation of timer failed");
    }
    #endif
#ifdef ENABLE_V4L2_DUMP
    V4L2_dump_file = NULL;
    m_bEnableV4L2Dump = IS_DUMP_ENABLE();
#endif
}

/*!*************************************************************************
 * @brief     DTOR
 *
 * @param[in] NONE
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
ScreenSource::~ScreenSource()
{
    #ifdef ENABLE_WFD_STATS
    // WFD:STATISTICS -- start
    if(wfdStats.bEnableWfdStat && 0 != wfdStats.nStatCount)
    {
       MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
                   "WFD:STATISTICS: Average Roundtrip time of buffer \
                   in Userspace is =%llu ms Max time is =%llu ms",
                   wfdStats.nCumulativeStatTime/wfdStats.nStatCount,
                   wfdStats.nMaxStatTime);
    }
    m_nDuration = 0;
    if(m_pStatTimer != NULL)
    {
       MM_Timer_Release(m_pStatTimer);
    }
    // WFD:STATISTICS -- end
    #endif /* ENABLE_WFD_STATS */
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int timeoutCnt=1000;/*timeout counter*/
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceScreenSource::~ScreenSource()");

    if(m_eScrnSrcState !=  WFDMM_SCREENSOURCE_STATE_INIT)
    {
      MM_Signal_Set(m_WFDScreenSourceExitSignal);
      while((m_eScrnSrcState !=  WFDMM_SCREENSOURCE_STATE_INIT)&&timeoutCnt)
      {
        MM_Timer_Sleep(5);
        timeoutCnt--;
      }
    }

    if(m_nMirrorHint == MDP_WRITEBACK_MIRROR_PAUSE)
    {
      fd_fb = open(gDisplayPanels[m_nFBPanelIdx], O_RDWR);

      if(fd_fb > 0)
      {
        int eCntrl = MDP_WRITEBACK_MIRROR_OFF;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                      "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT OFF");
        if(ioctl(fd_fb, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &eCntrl) < 0)
        {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT failed");
        }
        close(fd_fb);
        fd_fb = -1;
      }
      else
      {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "Screensource Unable to open writeback panel for HINT");
      }

    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFDMMSourceScreenSource::Calling enableExternalDisplay(Disable)\n");

    if(m_ScreenSourceThreadHandle)
    {
      MM_Thread_Release( m_ScreenSourceThreadHandle );
      m_ScreenSourceThreadHandle = NULL;
    }

    //releasing signal queue
    if(m_WFDScreenSourcePlaySignal != NULL)
    {
      MM_Signal_Release(m_WFDScreenSourcePlaySignal);
    }
    if(m_WFDScreenSourcePauseSignal != NULL)
    {
      MM_Signal_Release(m_WFDScreenSourcePauseSignal);
    }
    if(m_WFDScreenSourceExitSignal != NULL)
    {
      MM_Signal_Release(m_WFDScreenSourceExitSignal);
    }
    if(m_signalQ != NULL)
    {
      MM_SignalQ_Release(m_signalQ);
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "~WFDMMSourceScreenSource:: Buffers are freed up");
#ifdef ENABLE_V4L2_DUMP
    if(V4L2_dump_file)
    {
        fclose(V4L2_dump_file);
        V4L2_dump_file = NULL;
    }
#endif /* ENABLE_V4L2_DUMP */
}

/*!*************************************************************************
 * @brief     Configures the source
 *
 * @param[in] nFrames The number of frames to to play.
 * @param[in] nBuffers The number of buffers allocated for the session.
 * @param[in] pFrameDeliverFn Frame delivery callback.
 * @param[in] bFrameSkipEnabled frame skipping enabled
 * @param[in] nFrameSkipLimitInterval frame skipping time interval
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::Configure(
    VideoEncStaticConfigType* pConfig,
    OMX_S32 nBuffers,
    FrameDeliveryFnType pFrameDeliverFn,
    eventHandlerType pEventHandlerFn,
    OMX_BOOL bFrameSkipEnabled,
    OMX_U64 nFrameSkipLimitInterval,
    OMX_U32 nModuleId,
    void *appData)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if(pConfig)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: frame rate is %ld", pConfig->nFramerate);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::nFrameWidth is %ld",  pConfig->nFrameWidth);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: nFrameHeight is %ld", pConfig->nFrameHeight);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: nDVSXOffset is %ld", pConfig->nDVSXOffset);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: nDVSYOffset is %ld", pConfig->nDVSYOffset);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::nBuffers is %ld", nBuffers);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: pFrameDeliverFn is %p", pFrameDeliverFn);
        if (( pConfig->eCodec ==  OMX_VIDEO_CodingAVC) &&
            ( pConfig->nFramerate > 0  )&& ( pConfig->nFrameWidth > 0 ) &&
            ( pConfig->nFrameHeight > 0 )&&( pConfig->nDVSXOffset >= 0) &&
            ( pConfig->nDVSYOffset >= 0 )&&( nBuffers > 0 )&&
            ( pConfig->nBitrate > 0 ) && ( pFrameDeliverFn != NULL) &&
            pEventHandlerFn != NULL )
        {
            OMX_ERRORTYPE eErr = OMX_ErrorUndefined;
            m_nFramerate = pConfig->nFramerate;
            m_nFrameWidth = pConfig->nFrameWidth;
            m_nFrameHeight = pConfig->nFrameHeight;
            m_numBuffers = nBuffers;
            m_nDVSXOffset = pConfig->nDVSXOffset;
            m_nDVSYOffset = pConfig->nDVSYOffset;
            m_pEventHandlerFn = pEventHandlerFn;
            m_nModuleId       = nModuleId;
            if(pConfig->nProfile == WFD_H264_PROFILE_CONSTRAINED_HIGH)
            {
              /* To DO : Right now V4L2 don't support constained high profile,
                 we can change this once they support. */
              m_nProfile = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
            }
            else
            {
              m_nProfile = V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE;
            }
            m_nLevel = getV4L2Level(pConfig->nLevel);
            m_nBitrate = pConfig->nBitrate;
            m_pFrameDeliverFn = pFrameDeliverFn;
            m_bFrameSkipEnabled = bFrameSkipEnabled;
            m_nFrameSkipLimitInterval = nFrameSkipLimitInterval;
            m_appData = appData;
            m_nBitrateFactor = (m_nFrameWidth * m_nFrameHeight * m_nFramerate)/m_nBitrate;
      //      m_nFrameBytes = m_nFrameWidth * m_nFrameHeight * 3 / 2;

            if(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_INIT)
            {
              eErr = InitializeV4l2Codec();
              if(OMX_ErrorNone != eErr)
              {
                DeInitializeV4l2Codec();
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "WFDMMSourceScreenSource::InitializeV4l2Codec failed");
                return OMX_ErrorBadParameter;
              }
            }
        }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::bad params");
      return OMX_ErrorBadParameter;
    }// if pConfig

    if(m_eScrnSrcState ==  WFDMM_SCREENSOURCE_STATE_INIT)
    {
  #ifdef ENABLE_V4L2_DUMP
      if(m_bEnableV4L2Dump)
      {
        V4L2_dump_file = NULL;
        V4L2_dump_file = fopen("/data/media/v4l2_dump_user.264","wb");
        if(!V4L2_dump_file)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::file open failed");
          m_bEnableV4L2Dump = OMX_FALSE;
        }
      }
  #endif /*ENABLE_V4L2_DUMP */
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "WFDMMSourceScreenSource::Calling enableExternalDisplay(Enable)\n");

      result = CreateResources();

      if(result != OMX_ErrorNone)
      {
         DeInitializeV4l2Codec();
      }
    }
    if( result == OMX_ErrorNone )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource::Allocated all resources");
      m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_IDLE;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Timer handler for reading statistics flag from command line
 *
 * @param[in] ptr Reference to the current instance
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
#ifdef ENABLE_WFD_STATS
void ScreenSource::readStatTimerHandler(void* ptr)
{
    ScreenSource* scrSrc= (ScreenSource*)ptr;
    char szTemp[PROPERTY_VALUE_MAX];
    if(property_get("persist.debug.enable_wfd_stats",szTemp,"true")<0)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to read persist.debug.enable_wfd_stats");
        return;
    }
    if(strcmp(szTemp,"false")==0)
    {
        memset(&(scrSrc->wfdStats),0, sizeof(wfdStats));
        scrSrc->wfdStats.bEnableWfdStat = OMX_FALSE;
    }
    else
    {
        scrSrc->wfdStats.bEnableWfdStat = OMX_TRUE;
    }
}
#endif

/*!*************************************************************************
 * @brief     Create resources
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::CreateResources()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    /**------------------------------------------------------------------
     * Create the signal Q for the screen source thread to wait on.
     *-------------------------------------------------------------------*/
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::CreateResources");
    if ( 0 != MM_SignalQ_Create( &m_signalQ ) )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "WFDMMSourceScreenSource:: m_signalQ creation failed");
      result = OMX_ErrorInsufficientResources;
      return result;
    }
    // Create play event
    if ( (0 != MM_Signal_Create(  m_signalQ,
                                (void *) &SCREEN_SOURCE_PLAY_EVENT,
                                  NULL,
                                  &m_WFDScreenSourcePlaySignal ) ) )
    {
      result = OMX_ErrorInsufficientResources;
      return result;
    }
    // Create pause event
    if ( (0 != MM_Signal_Create(  m_signalQ,
                                 (void *) &SCREEN_SOURCE_PAUSE_EVENT,
                                  NULL,
                                 &m_WFDScreenSourcePauseSignal ) ) )
    {
      result = OMX_ErrorInsufficientResources;
      return result;
    }
    // Create exit event
    if ( (0 != MM_Signal_Create(  m_signalQ,
                                  (void *) &SCREEN_SOURCE_EXIT_EVENT,
                                  NULL,
                                  &m_WFDScreenSourceExitSignal ) ) )
    {
      result = OMX_ErrorInsufficientResources;
      return result;
    }
    /**---------------------------------------------------------------
     *Creation of Screen source thread
     *----------------------------------------------------------------*/
    if( (0!= MM_Thread_CreateEx( 0,
                                 0,
                                 ScreenSource::SourceThreadEntry,
                                 this,
                                 WFD_MM_SOURCE_THREAD_STACK_SIZE,
                                 "WFDScreenSourceThread",
                                 &m_ScreenSourceThreadHandle)))

    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "WFDMMSourceScreenSource::thread creation failed");
      result = OMX_ErrorInsufficientResources;
      return result;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Changes the frame rate
 *            The frame rate will take effect immediately.
 * @param[in] nFramerate New frame rate
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::ChangeFrameRate(OMX_S32 nFramerate)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "WFDMMSourceScreenSource::ChangeFrameRate %ld", nFramerate);
    return result;
}

/*!*************************************************************************
 * @brief     Initialize V4L2
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::InitializeV4l2Codec()
{
    // Initialization of  V4L2 video capture
    // Query Capabilities
    // Enumerate devices to see which one suits our need.
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    unsigned int buffer_size;

    m_secureSession = OMX_FALSE;
    m_nMuxBuffersSent = 0;
    m_nMuxBuffersReceived = 0;
    FILE *displayPanelFP = NULL;

    m_bVideoForbid = OMX_FALSE;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "WFDMMSourceScreenSource::InitializeV4l2Codec: Opening frame buffer ");
    char msmFbType [MAX_FRAME_BUFFER_NAME_SIZE];
    char fbType [MAX_FRAME_BUFFER_NAME_SIZE];
    for(int i = 0; i < MAX_DISPLAY_PANELS; i++)
    {
      snprintf (msmFbType,MAX_FRAME_BUFFER_NAME_SIZE, "/sys/class/graphics/fb%d/msm_fb_type", i);
      displayPanelFP = fopen(msmFbType, "r");
      if(displayPanelFP)
      {
        fread(fbType, sizeof(char), MAX_FRAME_BUFFER_NAME_SIZE, displayPanelFP);
        /* Right now the name of the writeback panel is unknown panel
           we can change this to writeback panel once the display team fixes this
        */
        if(strncmp(fbType, frameBufferName[2], strlen(frameBufferName[2])) == 0)
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
               "WFDMMSourceScreenSource::InitializeV4l2Codec: Opening %s", gDisplayPanels[i]);
          fd_fb = open(gDisplayPanels[i], O_RDWR);
          if(fd_fb < 0)
          {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::Error in opening frame buffer %d %s",
           errno, strerror(errno));
            fclose(displayPanelFP);
            return OMX_ErrorBadParameter;
          }
          m_nFBPanelIdx = i;
          fclose(displayPanelFP);
          break;
        }
        else
        {
          fclose(displayPanelFP);
          continue;
        }
      }
    } /* for(int i = 0; i < MAX_DISPLAY_PANELS; i++) */

    if( ( NULL != m_pHdcpHandle ) &&
            ( HDCP_STATUS_SUCCESS == m_pHdcpHandle->m_eHdcpSessionStatus ))
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFDMMSourceScreenSource::InitializeV4l2Codec: Opening secure device %s", "/dev/video39");
      m_fd = open("/dev/video39", O_RDWR);
      if (m_fd < 0)
      {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::error in open secure device m_fd %d %s",
             errno, strerror(errno));
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "WFDMMSourceScreenSource::open secure device m_fd is success");
        m_secureSession = OMX_TRUE;
      }
    }
    if (m_fd < 0)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFDMMSourceScreenSource::InitializeV4l2Codec: Opening non secure device %s", "/dev/video38");
      m_fd = open("/dev/video38", O_RDWR);
      if (m_fd < 0)
      {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::error in open non secure device m_fd %d %s",
             errno, strerror(errno));
        return  OMX_ErrorBadParameter;
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "WFDMMSourceScreenSource::open non secure device m_fd is success");
        m_secureSession = OMX_FALSE;
      }
    }
    if (m_fd > 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::InitializeV4l2Codec: Querycap");
      if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::Query capabilities VIDIOC_QUERYCAP is failed");
        close(m_fd);
        m_fd = -1;
        return OMX_ErrorBadParameter;
      }

      //Here we need to add the capability to detect the proper device
      if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
          !(cap.capabilities & V4L2_CAP_STREAMING))
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::Device does not support video capture\n");
        close(m_fd);
        m_fd = -1;
        return OMX_ErrorBadParameter;
      }

      /*Try setting the format */
      memset(&fmt, 0, sizeof(fmt));
      fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      fmt.fmt.pix.width       = 0;
      fmt.fmt.pix.height      = 0;
      fmt.fmt.pix.field       = V4L2_FIELD_NONE;
      if (ioctl(m_fd, VIDIOC_G_FMT, &fmt) < 0)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                   "WFDMMSourceScreenSource::Error in VIDIOC_S_FMT");
        close(m_fd);
        m_fd = -1;
        return OMX_ErrorBadParameter;
      }
      if (!(fmt.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) &&
          !(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_H264))
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::Device does not support V4L2_PIX_FMT_NV12\n");
        close(m_fd);
        m_fd = -1;
        return OMX_ErrorBadParameter;
      }

      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceScreenSource::size = %d\n", fmt.fmt.pix.sizeimage);
      if(m_fd < 0)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::No device to capture screen");
        return OMX_ErrorBadParameter;
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource::Found device to capture screen");
      }
    }

    if(m_fd > 0)
    {
      /* susbscribe for events */
      if (Subscribe_Events(m_fd, true))
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "WFDMMSourceScreenSource::Failed to subscribe events \n");
        return OMX_ErrorBadParameter;
      }
    }

    struct fb_var_screeninfo sVar;
    memset(&sVar, 0, sizeof(fb_var_screeninfo));

    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::Frame width %ld Frame height %ld\n", m_nFrameWidth, m_nFrameHeight);
    if(fd_fb > 0)
    {
        if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &sVar) < 0)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "WFDMMSourceScreenSource::FBIOGET_VSCREENINFO failed");
          return  OMX_ErrorBadParameter;
        }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "WFDMMSourceScreenSource::Failed in opening frame buffer as fd_fb is -1");
      return  OMX_ErrorBadParameter;
    }
      MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                   "WFDMMSourceScreenSource:: xres is :  %d, yres is : %d bpp is  %d",
                   sVar.xres, sVar.yres, sVar.bits_per_pixel);
      sVar.xres = m_nFrameWidth;
      sVar.yres = m_nFrameHeight;
      sVar.xoffset = sVar.yoffset = 0;
      sVar.xres_virtual = m_nFrameWidth;
      sVar.yres_virtual = m_nFrameHeight;

      //sVar.activate = FB_ACTIVATE_NOW;
      if(fd_fb > 0)
      {
        if(ioctl(fd_fb, FBIOPUT_VSCREENINFO, &sVar) < 0)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "WFDMMSourceScreenSource::FBIOPUT_VSCREENINFO failed");
          return  OMX_ErrorBadParameter;
        }
      }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "WFDMMSourceScreenSource::Failed in opening frame buffer as fd_fb is -1");
      return  OMX_ErrorBadParameter;
    }
    // Set format
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = m_nFrameWidth;
    fmt.fmt.pix.height      =  m_nFrameHeight;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    fmt.fmt.pix.field       = V4L2_FIELD_NONE;
    if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "WFDMMSourceScreenSource::Error in VIDIOC_S_FMT");
      return OMX_ErrorBadParameter;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::size = %d\n", fmt.fmt.pix.sizeimage);

    // Prepare for user pointer I/O
    memset(&req, 0, sizeof(req));
    req.count  = SCREEN_SOURCE_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    //populate control parameters received from upper layer
    //set bitrate
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_BITRATE)
        {
          controls[control_index].value = m_nBitrate;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting bitrate control  %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }

//set CBR_VFR
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL)
        {
          controls[control_index].value = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting cbr_vfr control  %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }

   //set peak bitrate
     for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_BITRATE_PEAK)
        {
          controls[control_index].value = 2 * m_nBitrate;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting peak bitrate control  %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    //set profile
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_H264_PROFILE)
        {
          controls[control_index].value = m_nProfile;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting profile control  %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    //set level
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_H264_LEVEL)
        {
          controls[control_index].value = m_nLevel;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting level control  %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    //set Intra Refresh Random mode
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE)
        {
          controls[control_index].value = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_RANDOM;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting Intra Refresh Random Mode %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }

    //set RC Timestamp Ignore mode
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE)
        {
          controls[control_index].value = V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_IGNORE;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting RC Timestamp Ignore mode %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    //SET Text quality
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY)
        {
          controls[control_index].value = V4L2_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY_ENABLED;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting Text Quality %d   %d \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    //populate control parameters received from upper layer
    //set QP values
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_H264_MAX_QP)
        {
          controls[control_index].value = WFD_H264_MAX_QP;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting MAX QP with ctrl id(%d) and value(%d) \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_H264_MIN_QP)
        {
          controls[control_index].value = WFD_H264_MIN_QP;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting MIN QP value with ctrl id(%d) and value(%d) \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_H264_AU_DELIMITER)
        {
          controls[control_index].value = V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_ENABLED;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting AU Delimiter value with ctrl id(%d) and value(%d) \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    /**---------------------------------------------------------------------
        Decision to set the core in turbo/performance mode based
        on the WFD config file entry
      ------------------------------------------------------------------------
    */
    int32 nPerfLevelTurboMode = 0;
    int32 nRetVal;

    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_SET_PERF_LEVEL)
        {
          controls[control_index].value = -1;

          nRetVal = getCfgItem(PERF_LEVEL_TURBO_MODE_KEY,(int*)(&nPerfLevelTurboMode));
          if(nRetVal == 0 && nPerfLevelTurboMode)
          {
             controls[control_index].value = V4L2_CID_MPEG_VIDC_PERF_LEVEL_TURBO;
          }
          else
          {
             nRetVal = getCfgItem(PERF_LEVEL_PERF_MODE_KEY,(int*)(&nPerfLevelTurboMode));
             if(nRetVal == 0 && nPerfLevelTurboMode)
             {
               controls[control_index].value = V4L2_CID_MPEG_VIDC_PERF_LEVEL_PERFORMANCE;
             }
          }

          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting PERF LEVEL value with ctrl id(%d) and value(%d) \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }

  /**---------------------------------------------------------------------
      As pwr WFD specification, Entropy mode should not be set
       to CABAC, it should be always CAVLC
  ------------------------------------------------------------------------
   */
   for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
   {
       if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE)
       {
         controls[control_index].value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC;
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                     "WFDMMSourceScreenSource::setting Entropy mode value with ctrl id(%d) and value(%d) \n",
                      controls[control_index].id, controls[control_index].value);
         break;
       }
   }
   //set bframes period
   for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
   {
       if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES)
       {
         controls[control_index].value = NUMBER_OF_BFRAME;
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                     "WFDMMSourceScreenSource::setting Bframes mode value with ctrl id(%d) and value(%d) \n",
                      controls[control_index].id, controls[control_index].value);
         break;
       }
   }
    // Seting Cyclic Intra Refresh MBs
    int32 nCyclicIntraRefresh = 0;
    int32 nMacroBlocks = 0;
    nRetVal = getCfgItem(CYCLIC_IR_KEY,(int*)(&nCyclicIntraRefresh));

    if(nRetVal == 0)
    {
      if(nCyclicIntraRefresh)
      {
        nRetVal = getCfgItem(CYCLIC_IR_NUM_MACRO_BLK_KEY,(int*)(&nMacroBlocks));

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                     "WFDMMSourceScreenSource::Cyclic intra refresh num macroblocks %ld",
                     nMacroBlocks);
        if(nRetVal == 0)
        {
          if( nMacroBlocks > 0)
          {
            struct v4l2_control ctrl;
            memset(&ctrl, 0, sizeof(v4l2_control));
            ctrl.id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB;
            if(((int32)(m_nFrameWidth / 16)) < nMacroBlocks)
            {
              ctrl.value = (int32)((m_nFrameWidth / 16));
            }
            else
            {
              ctrl.value = nMacroBlocks;
            }
            int rc = 0;
            rc = ioctl(m_fd, VIDIOC_S_CTRL, &ctrl);
            if (rc)
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "WFDMMSourceScreenSource::Failed to set control index is V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB \n");
            }
            else
            {
              //No need to stop the session as this is error can not lead to any issues
              MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_DEBUG,
                          "WFDMMSourceScreenSource::setting Cyclic Intra Refresh value with ctrl id(%d) and value(%d) \n",
                          ctrl.id, ctrl.value);
            }
          }
        }
      }
    }

    if (ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "WFDMMSourceScreenSource::Error in VIDIOC_REQBUFS");
      return OMX_ErrorBadParameter;
    }

    // Allocate buffers
    buffer_size = fmt.fmt.pix.sizeimage;
    m_pBuffers = (struct buffer *) calloc(req.count, sizeof(*m_pBuffers));
    m_numBuffers = req.count;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::m_numBuffers = %ld\n", m_numBuffers);
    if (!m_pBuffers)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "WFDMMSourceScreenSource::Could not allocate buffers\n");
      return OMX_ErrorInsufficientResources;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::Open ION");
    ionfd = open("/dev/ion",  O_RDONLY);
    if (ionfd < 0)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                   "WFDMMSourceScreenSource::Failed to open ion device = %d\n", ionfd);
      return OMX_ErrorInsufficientResources;
    }

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "WFDMMSourceScreenSource:: open ion device = %d\n", ionfd);
    for (unsigned int i = 0; i < (unsigned int)m_numBuffers; i++)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                   "WFDMMSourceScreenSource::pmem adsp opening");
        if(m_secureSession == OMX_TRUE)
        {
          memfd = allocate_ion_mem(buffer_size, &(m_pBuffers[i].handle), ionfd, ION_CP_MM_HEAP_ID, OMX_TRUE);
        }
        else
        {
          memfd = allocate_ion_mem(buffer_size, &(m_pBuffers[i].handle),ionfd,ION_CP_MM_HEAP_ID|ION_IOMMU_HEAP_ID, OMX_FALSE);
        }
        if(memfd < 0)
        {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "WFDMMSourceScreenSource::Failed to allocate ion memory\n");
           return OMX_ErrorInsufficientResources;
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::memfd = %d ", memfd);
        if(m_secureSession == OMX_FALSE)
        {
           m_pBuffers[i].start = (unsigned char *)
            mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
        }
        else
        {
           m_pBuffers[i].start = (void *) i;
        }
        m_pBuffers[i].length = buffer_size;
        m_pBuffers[i].fd = memfd;
        m_pBuffers[i].offset = 0;
        m_pBuffers[i].index = i;
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::allocated buffer(%p) of size = %d, fd = %d\n",
                    m_pBuffers[i].start, buffer_size, memfd);
        if ((m_secureSession == OMX_FALSE) && (m_pBuffers[i].start == MAP_FAILED))
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR ,
                        "WFDMMSourceScreenSource::Could not allocate pmem buffers\n");
            return OMX_ErrorInsufficientResources;
        }
    }//for loop allocate buffer on ion memory

    // Configure frame skipping interval
    memset(&m_parm, 0, sizeof(v4l2_streamparm));
    memset(&m_frameskip, 0, sizeof(v4l2_qcom_frameskip));
    m_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    m_parm.parm.capture.capturemode = 0;
    m_parm.parm.capture.timeperframe.numerator = 1;
    m_parm.parm.capture.timeperframe.denominator = m_nFramerate;

    if (m_bFrameSkipEnabled)
    {
        m_parm.parm.capture.capability |= V4L2_CAP_QCOM_FRAMESKIP;
        //converting msec into nano sec
        m_frameskip.maxframeinterval = m_nFrameSkipLimitInterval * 1000 * 1000;
        m_frameskip.fpsvariance = FRAME_SKIP_FPS_VARIANCE;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: frame skipping interval is %lld",m_nFrameSkipLimitInterval);
        if(m_nFrameSkipLimitInterval == 0)
        {
           m_frameskip.maxframeinterval = 0x00ffffffffffffff;
        }
        m_parm.parm.capture.extendedmode = (unsigned int)&m_frameskip;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::setting frame skipping interval %lld",m_frameskip.maxframeinterval);

    }
    else
    {
        m_parm.parm.capture.extendedmode = (unsigned int)NULL;
    }
    if (ioctl(m_fd, VIDIOC_S_PARM, &m_parm) < 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::Error in VIDIOC_S_PARM setting fps and frame skipping interval");
        return OMX_ErrorBadParameter;
    }

    //Setting VUI timing information Control after setting the Frame Rate
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO)
        {
          controls[control_index].value = V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_ENABLED;
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                      "WFDMMSourceScreenSource::setting VUI Timing value with ctrl id(%d) and value(%d) \n",
                       controls[control_index].id, controls[control_index].value);
          break;
        }
    }
    // Set control parameters
    if (set_control_parameters(m_fd))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "WFDMMSourceScreenSource::Error set_control_parameters");
    }
    // Enqueue all buffers to V4l2
    for (unsigned int i = 0; i < (unsigned int)m_numBuffers; i++)
    {
        memset(&m_sSetV4L2Buf, 0, sizeof(m_sSetV4L2Buf));
        m_sSetV4L2Buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_sSetV4L2Buf.memory    = V4L2_MEMORY_USERPTR;
        m_sSetV4L2Buf.index     = i;
        m_sSetV4L2Buf.m.userptr = (unsigned long) m_pBuffers[i].start;
        m_sSetV4L2Buf.length    = m_pBuffers[i].length;
        m_sMemInfo[i].fd        = m_pBuffers[i].fd;
        m_sMemInfo[i].offset    = m_pBuffers[i].offset;
        m_sSetV4L2Buf.reserved  = (unsigned int)&m_sMemInfo[i];

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::calling VIDIOC_QBUF");

        if (ioctl(m_fd, VIDIOC_QBUF, &m_sSetV4L2Buf) < 0)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "WFDMMSourceScreenSource::Error in VIDIOC_QBUF");
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource:: VIDIOC_QBUF is success = %d",
                        m_sSetV4L2Buf.length );
            isBufWithV4l2[m_sSetV4L2Buf.index]=1;
        }
    }//for loop to InQ allocate buffer

    // Buffer header allocation is in configure
    m_pscreensourceOutputBuffers = (OMX_BUFFERHEADERTYPE**) MM_Malloc(m_numBuffers * sizeof(OMX_BUFFERHEADERTYPE*));
    if(m_pscreensourceOutputBuffers)
    {
      for (unsigned int i = 0; i < (unsigned int)m_numBuffers; i++)
      {
        m_pscreensourceOutputBuffers[i] = (OMX_BUFFERHEADERTYPE* ) MM_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                   "WFDMMSourceScreenSource::Could not allocate buffers m_pscreensourceOutputBuffers\n");
      return OMX_ErrorInsufficientResources;
    }
    if( ( m_secureSession == OMX_TRUE) )
    {
      // Allocate buffers
      buffer_size = fmt.fmt.pix.sizeimage;
      m_pHdcpOutBuffers = (struct buffer *) calloc(req.count, sizeof(*m_pHdcpOutBuffers));
      if (!m_pHdcpOutBuffers)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                   "WFDMMSourceScreenSource::Could not allocate buffers\n");
        return OMX_ErrorInsufficientResources;
      }

      for (unsigned int i = 0; i < (unsigned int)m_numBuffers; i++)
      {
        if(m_secureSession == OMX_TRUE)
        {
          memfd = allocate_ion_mem(buffer_size, &(m_pHdcpOutBuffers[i].handle), ionfd, ION_QSECOM_HEAP_ID, OMX_FALSE);
        }

        if(memfd < 0)
        {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "WFDMMSourceScreenSource::Failed to allocate ion memory for HDCP buffers\n");
           return OMX_ErrorInsufficientResources;
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::memfd = %d ", memfd);

        m_pHdcpOutBuffers[i].start = (unsigned char *)
        mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
        m_pHdcpOutBuffers[i].length = buffer_size;
        m_pHdcpOutBuffers[i].fd = memfd;
        m_pHdcpOutBuffers[i].offset = 0;
        m_pHdcpOutBuffers[i].index = i;
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::allocated buffer(%p) of size = %d, fd = %d\n",
                    m_pHdcpOutBuffers[i].start, buffer_size, memfd);
        if (m_pHdcpOutBuffers[i].start == MAP_FAILED)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR ,
                        "WFDMMSourceScreenSource::Could not allocate pmem buffers\n");
            return OMX_ErrorInsufficientResources;
        }
      }//for loop allocate buffer on ion memory
    }//HDCP handle close
    for ( int i=0; i < m_numBuffers; i++)
    {
      m_pscreensourceOutputBuffers[i]->nAllocLen = m_pBuffers[i].length;
      if(( m_secureSession == OMX_TRUE ))
      {
        m_pscreensourceOutputBuffers[i]->pBuffer = (OMX_U8 *) m_pHdcpOutBuffers[i].start;
      }
      else
      {
        m_pscreensourceOutputBuffers[i]->pBuffer = (OMX_U8 *) m_pBuffers[i].start;
      }
      m_pscreensourceOutputBuffers[i]->pOutputPortPrivate = (OMX_U8 *) m_pBuffers[i].start;
    }


    if(m_bFillerNaluEnabled)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Screensource Allocate filler Nalu buffers");
      if(m_secureSession == OMX_TRUE)
      {
        m_nFillerInFd = allocate_ion_mem(FILLER_ION_BUF_SIZE, &m_hFillerInHandle,
                                         ionfd, ION_QSECOM_HEAP_ID, OMX_FALSE);

        if(m_nFillerInFd <= 0)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Failed to allocate In FillerNalu ION buffer");
        }
        else
        {
          m_pFillerDataInPtr = (unsigned char*)
                         mmap(NULL, FILLER_ION_BUF_SIZE, PROT_READ | PROT_WRITE,
                              MAP_SHARED, m_nFillerInFd, 0);

          if(m_pFillerDataInPtr == NULL)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Failed to allocate In FillerNalu buffer mmap");
          }
          else
          {
            //Initialize the input buffer with fixed Filler NALU
            memcpy(m_pFillerDataInPtr, sFillerNALU, sizeof(sFillerNALU));
          }

        }

        m_nFillerOutFd = allocate_ion_mem(FILLER_ION_BUF_SIZE, &m_hFillerOutHandle,
                                          ionfd, ION_QSECOM_HEAP_ID, OMX_FALSE);

        if(m_nFillerOutFd <= 0)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Failed to allocate Out FillerNalu ION buffer");
        }
        else
        {
          m_pFillerDataOutPtr = (unsigned char*)
                         mmap(NULL, FILLER_ION_BUF_SIZE, PROT_READ | PROT_WRITE,
                                                  MAP_SHARED, m_nFillerOutFd, 0);
          if(m_pFillerDataOutPtr == NULL)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Failed to allocate In FillerNalu buffer mmap");
          }
        }
      }
      else
      {
        m_pFillerDataOutPtr = (unsigned char*)sFillerNALU;
      }
    }
#ifdef WFD_ICS
    android::SurfaceComposerClient::enableExternalDisplay(EXT_DISPLAY_WIFI,2); // 2- enable WIFI and 0  disable WIFI
#endif
    return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     DeInitialize V4L2
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note
 **************************************************************************/
void ScreenSource::DeInitializeV4l2Codec()
{
  int ret = 0;
  struct ion_handle_data handle_data;
  int nExitCode = 0;
#if 0
    for ( int nIndex= 0; nIndex < m_numBuffers; nIndex++ )
    {
        memset(&m_sSetV4L2Buf, 0, sizeof(m_sSetV4L2Buf));
        m_sSetV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_sSetV4L2Buf.memory = V4L2_MEMORY_USERPTR;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceScreenSource::Before dqbuf in pause\n");
        // Dequeue buffer
       if(isBufWithV4l2[m_sSetV4L2Buf.index])
       {
        if ( ioctl(m_fd, VIDIOC_DQBUF, &m_sSetV4L2Buf) == -1 )
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "WFDMMSourceScreenSource::Error in VIDIOC_DQBUF");
            //bKeepGoing = OMX_FALSE;
        }
        else
        {
         isBufWithV4l2[m_sSetV4L2Buf.index]=0;
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource:: success in Dequeue buffer pause");
        }
      }
    }//for loop to DQBufer
#endif // #if 0

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "~WFDMMSourceScreenSource:: DeInitializeV4l2Codec is Called");

    while( m_nMuxBuffersReceived != m_nMuxBuffersSent )
    {
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"WFDMMSourceScreenSource::num Mux Buffers sent(%ld) is not same as received(%ld)"
                                             "sleeping for 100ms", m_nMuxBuffersSent, m_nMuxBuffersReceived);
      MM_Timer_Sleep(2);
    }
    m_nMuxBuffersSent = 0;
    m_nMuxBuffersReceived = 0;

    if(m_fd > 0)
    {
      /* Un subscibe of the events */
      if (Subscribe_Events(m_fd, false))
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::Failed to un subscribe events \n");
      }
    }
    if(m_fd > 0)
    {
      close(m_fd);
      m_fd = -1;
    }

    m_bPollContinue = OMX_FALSE;
    if(m_ScreenSourcePollThreadHandle)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "DeInitializeV4l2Codec:: Before Join");
      MM_Thread_Join( m_ScreenSourcePollThreadHandle, &nExitCode );
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "DeInitializeV4l2Codec:: Poll Thread returned");
    }
    if(m_ScreenSourcePollThreadHandle)
    {
      MM_Thread_Release( m_ScreenSourcePollThreadHandle );
      m_ScreenSourcePollThreadHandle = NULL;
    }
    for (int i = 0; i < m_numBuffers; i++)
    {
      if(m_pBuffers)
      {
          if(m_secureSession == OMX_FALSE && m_pBuffers[i].start != NULL)
          {
            if (munmap(m_pBuffers[i].start, m_pBuffers[i].length) == -1)
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                            "WFDMMSourceScreenSource::error in munmap = %d", i);
            }
            m_pBuffers[i].start = NULL;
            m_pBuffers[i].length = 0;
          }
          if(m_pBuffers[i].handle != NULL)
          {
            handle_data.handle = m_pBuffers[i].handle;
            ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
          }
          if(m_pBuffers[i].fd > 0)
          {
            close(m_pBuffers[i].fd);
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                          "closing ion fd = %d ret type %d\n", m_pBuffers[i].fd, ret);
            m_pBuffers[i].fd = -1;
          }
      }
     }
     // Free all omx buffer headers
     if(m_pscreensourceOutputBuffers)
     {
       for (unsigned int i = 0; i < (unsigned int)m_numBuffers; i++)
       {
           if(m_pscreensourceOutputBuffers[i])
           {
               if((m_pHdcpHandle != NULL) &&(m_pHdcpOutBuffers != NULL))
               {
                if(m_pHdcpOutBuffers[i].start != NULL)
                {
                  if (munmap(m_pHdcpOutBuffers[i].start, m_pHdcpOutBuffers[i].length) == -1)
                  {
                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                          "WFDMMSourceScreenSource::error in munmap = %d", i);
                  }
                  m_pHdcpOutBuffers[i].start = NULL;
                  m_pHdcpOutBuffers[i].length = 0;
                }
                if(m_pHdcpOutBuffers[i].handle != NULL)
                {
                  handle_data.handle = m_pHdcpOutBuffers[i].handle;
                  ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
                  m_pHdcpOutBuffers[i].handle = NULL;
                }
                if(m_pHdcpOutBuffers[i].fd > 0)
                {
                  close(m_pHdcpOutBuffers[i].fd);
                  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                            "closing hdcp ion fd = %d ret type = %d \n", m_pHdcpOutBuffers[i].fd, ret);
                  m_pHdcpOutBuffers[i].fd = -1;
                }
              }

              if(m_pscreensourceOutputBuffers[i] != NULL)
              {
                MM_Free(m_pscreensourceOutputBuffers[i]);
                m_pscreensourceOutputBuffers[i] = NULL;
              }
          }
      }
      if(m_nFillerInFd > 0)
      {
         if(m_pFillerDataInPtr)
         {
           munmap(m_pFillerDataInPtr, FILLER_ION_BUF_SIZE);
           m_pFillerDataInPtr = NULL;
         }

         if(m_hFillerInHandle != NULL)
         {
           handle_data.handle = m_hFillerInHandle;
           ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
           m_hFillerInHandle = NULL;
         }
         close(m_nFillerInFd);
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                    "closing hdcp filler ion fd = %d ret type = %d \n", m_nFillerInFd, ret);
         m_nFillerInFd = -1;
      }

      if(m_nFillerOutFd > 0)
      {
         if(m_pFillerDataOutPtr)
         {
           munmap(m_pFillerDataOutPtr, FILLER_ION_BUF_SIZE);
           m_pFillerDataOutPtr = NULL;
         }

         if(m_hFillerOutHandle != NULL)
         {
           handle_data.handle = m_hFillerOutHandle;
           ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
           m_hFillerOutHandle = NULL;
         }
         close(m_nFillerOutFd);
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                  "closing hdcp filler ion fd = %d ret type = %d \n", m_nFillerOutFd, ret);
         m_nFillerOutFd = -1;
      }

      if(m_pHdcpOutBuffers)
      {
        MM_Free(m_pHdcpOutBuffers);
        m_pHdcpOutBuffers = NULL;
      }
      MM_Free(m_pscreensourceOutputBuffers);
      m_pscreensourceOutputBuffers = NULL;
    }
    if(m_pBuffers != NULL)
    {
      free(m_pBuffers);
      m_pBuffers = NULL;
    }
    if(ionfd > 0)
    {
      close(ionfd);
      ionfd = -1;
    }
    if(fd_fb > 0)
    {
      close(fd_fb);
      fd_fb = -1;
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "~WFDMMSourceScreenSource:: DeInitializeV4l2Codec Completed");
#ifdef WFD_ICS
    android::SurfaceComposerClient::enableExternalDisplay(EXT_DISPLAY_WIFI,0); // 2- enable WIFI and 0 disable WIFI
#endif

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Calling setSecondaryDisplayStatus");

    qdutils::setSecondaryDisplayStatus(qdutils::DISPLAY_VIRTUAL, qdutils::EXTERNAL_OFFLINE);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Done calling setSecondaryDisplayStatus");
}

/*!*************************************************************************
 * @brief     Changes the bit rate
 *            The bit rate will take effect immediately.
 *
 * @param[in] nBitrate The new bit rate.
 *
 * @return    OMX_ERRORTYPE
 * @note      None
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::ChangeBitrate(OMX_S32 nBitrate)
{
    int rc = 0;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::ChangeBitrate %ld", nBitrate);
    if(nBitrate > 0)
    {
      if(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_IDLE)
      {
        m_nBitrate = nBitrate;
      }
      else
      {
        for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
        {
            if (controls[control_index].id == V4L2_CID_MPEG_VIDEO_BITRATE)
            {
                controls[control_index].value = nBitrate;
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSourceScreenSource::changing bit rate = %d   %d \n",
                            controls[control_index].id,
                            controls[control_index].value);
                rc = ioctl(m_fd, VIDIOC_S_CTRL, &controls[control_index]);
                if (rc)
                {
                    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                "WFDMMSourceScreenSource::ChangeBitrate failed to set bitrate = %d \n", rc);
                    break;
                }
              m_nBitrate = nBitrate;
                break;
            }//if control.id = bitrate
        }//for loop
      }
    }// if nBitRate
    return result;
}

/*!*************************************************************************
 * @brief     Request Intra VOP
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::RequestIntraVOP()
{
    int rc = 0;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::RequestIntraVOP");
    for (unsigned int control_index = 0; control_index < ARRAY_SZ(controls); ++control_index)
    {
        if (controls[control_index].id == V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME)
        {
            controls[control_index].value = V4L2_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE_I_FRAME;
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                        "WFDMMSourceScreenSource::setting to generate I-Frame = %d   %d \n",
                         controls[control_index].id,
                         controls[control_index].value);
            rc = ioctl(m_fd, VIDIOC_S_CTRL, &controls[control_index]);
            if (rc)
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSourceScreenSource::RequestIntraVOP failed to RequestIntraVOP = %d \n",
                            rc);
                break;
            }
            break;
        }
    }
    return result;
}

/*!*************************************************************************
 * @brief     Get buffer header
 *
 * @param[in] NONE
 *
 * @return    OMX_BUFFERHEADERTYPE**
 *
 * @note      NONE
 **************************************************************************/
OMX_BUFFERHEADERTYPE** ScreenSource::GetBuffers()
{
    OMX_BUFFERHEADERTYPE** ppBuffers;
    if(m_pscreensourceOutputBuffers)
    {
        ppBuffers = m_pscreensourceOutputBuffers;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::GetBuffers success");
    }
    else
    {
        ppBuffers = NULL;
    }
    return ppBuffers;
}

/*!*************************************************************************
 * @brief     Get number of buffer
 *
 * @param[in] NONE
 * @param[out]pnBuffers
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::GetOutNumBuffers(
    OMX_U32 nPortIndex, OMX_U32* pnBuffers)
{
    if(nPortIndex != 0)
    {
        return OMX_ErrorBadPortIndex;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFDMMSourceScreenSource::GetBuffers m_numBuffers %ld",
                 m_numBuffers);
    *pnBuffers = m_numBuffers;
    return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     Start screen source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::Start()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    // make sure we've been configured
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "WFDMMSourceScreenSource::SourceThread Start");
    if ( m_numBuffers > 0 )
    {
        m_bStarted = OMX_TRUE;
        bKeepGoing = OMX_TRUE;
        m_nMuxBufferCount = 0;
        m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_PLAYING;
        MM_Signal_Set(m_WFDScreenSourcePlaySignal);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::need to register all buffers with the source");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Stop screen source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::Finish()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int timeoutCnt=5000;/*wait counter*/
    bKeepGoing = OMX_FALSE;
    // Calculate  V4L2 queue size
    for( int index = 0; index < SCREEN_SOURCE_BUFFERS; index ++)
    {
      if(isBufWithV4l2[index])
      m_nV4L2QueueSize++;
    }
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::Finish :Before WFDMM_SCREENSOURCE_STATE_PLAYING loop");

    while(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_PLAYING)
    {
      MM_Timer_Sleep(2);
    }
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::Finish :After WFDMM_SCREENSOURCE_STATE_PLAYING loop");
    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_STOPPING;
    if (m_bStarted == OMX_TRUE)
    {
        m_bStarted = OMX_FALSE;
        bKeepGoing = OMX_FALSE;

        if( m_fd > 0 )
        {
           enum v4l2_buf_type type;
           type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           if ( m_bStreamON == OMX_TRUE )
           {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::Finish :Before m_bCritTransact loop");
              while(m_bCritTransact == OMX_TRUE)
              {
                MM_Timer_Sleep(1);
              }
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::Finish :After m_bCritTransact loop");
              if(fd_fb > 0)
              {
               if(m_nMirrorHint != -1)
                {
                  int eCntrl = MDP_WRITEBACK_MIRROR_OFF;
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                              "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT %d",
                              eCntrl);
                  if(ioctl(fd_fb, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &eCntrl) < 0)
                  {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT failed");
                  }
                  else
                  {
                    m_nMirrorHint = eCntrl;
                  }
                }
              }
              if (ioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0 )
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                              "WFDMMSourceScreenSource:: error in VIDIOC_STREAMOFF");
              }
              else
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource:: success in VIDIOC_STREAMOFF");
              }
              m_bStreamON = OMX_FALSE;
           }
        }
        MM_Signal_Set(m_WFDScreenSourceExitSignal);
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::m_WFDScreenSourceExitSignal");
        while((m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_INIT)&&timeoutCnt)
        {
            MM_Timer_Sleep(4);/*20sec*/
            timeoutCnt--;
            if(timeoutCnt == 0)
            {
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"ScreenSource::Finish timed out");
            }
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::already stopped");
        result = OMX_ErrorIncorrectStateTransition;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Pause screen source
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::Pause()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    bKeepGoing = OMX_FALSE;
    int timeoutCnt=2500;/*wait count*/
    // Calculate  V4L2 queue size
    for( int index = 0; index < SCREEN_SOURCE_BUFFERS; index ++)
    {
      if(isBufWithV4l2[index])
      m_nV4L2QueueSize++;
    }
    while(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_PLAYING)
    {
      MM_Timer_Sleep(2);
    }
    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_STOPPING;
    if (m_bStarted == OMX_TRUE)
    {
        m_bStarted = OMX_FALSE;
        bKeepGoing = OMX_FALSE;

        if( m_fd > 0 )
        {
           enum v4l2_buf_type type;
           type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           if ( m_bStreamON == OMX_TRUE )
           {
              while(m_bCritTransact == OMX_TRUE)
              {
                MM_Timer_Sleep(1);
              }
	      if(fd_fb > 0)
              {
                if(m_nMirrorHint != -1)
                {
                  int eCntrl = MDP_WRITEBACK_MIRROR_PAUSE;
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                              "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT %d",
                              eCntrl);
                  if(ioctl(fd_fb, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &eCntrl) < 0)
                  {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT failed");
                  }
                  else
                  {
                    m_nMirrorHint = eCntrl;
                  }
                }
              }
	      if (ioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0 )
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                              "WFDMMSourceScreenSource:: error in VIDIOC_STREAMOFF");
              }
              else
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource:: success in VIDIOC_STREAMOFF");
              }
              m_bStreamON = OMX_FALSE;
           }
        }
        MM_Signal_Set(m_WFDScreenSourcePauseSignal);
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "WFDMMSourceScreenSource::m_WFDScreenSourcePauseSignal");
        while((m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_IDLE)&&timeoutCnt)
        {
            MM_Timer_Sleep(2);
            timeoutCnt--;
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFDMMSourceScreenSource::already stopped");
        result = OMX_ErrorIncorrectStateTransition;
    }
    return result;
}

/*!*************************************************************************
 * @brief     Set Free buffer
 *
 * @param[in] pBufferHdr OMX_BUFFERHEADERTYPE passed in
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::SetFreeBuffer(OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    struct mem_info *meminfo = NULL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceScreenSource::SetFreeBuffer");
    if (pBufferHdr != NULL && pBufferHdr->pBuffer != NULL)
    {
        // if we have not started then the client is registering buffers
        if (m_bStarted == OMX_FALSE)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::register buffer");
        }
        if ( WFDMM_SCREENSOURCE_STATE_PLAY != m_eScrnSrcState )
        {
           // If state is stop state then dont push buffers to V4L2
           // Increment mux buffer count
           m_nMuxBuffersReceived++;
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::SetFreeBuffer: Mux recvd buffers %ld",
                        m_nMuxBuffersReceived);
        }
        else
        {
        meminfo = (mem_info *)MM_Malloc(sizeof(struct mem_info));
        if(meminfo == NULL)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "WFDMMSourceScreenSource::Malloc fail!!!");
            m_nMuxBuffersReceived++;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::SetFreeBuffer: Mux recvd buffers %ld",
                        m_nMuxBuffersReceived);
            return OMX_ErrorInsufficientResources;
        }
        struct v4l2_buffer *buf ;
        buf = (v4l2_buffer *)MM_Malloc(sizeof(v4l2_buffer));
        if(buf == NULL)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "WFDMMSourceScreenSource::Malloc fail!!!");
            MM_Free(meminfo);
            meminfo = NULL;
            m_nMuxBuffersReceived++;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::SetFreeBuffer: Mux recvd buffers %ld",
                        m_nMuxBuffersReceived);
            return OMX_ErrorInsufficientResources;
        }
        memset(buf, 0, sizeof(v4l2_buffer));
        for (int bufindex = 0; bufindex < m_numBuffers; bufindex++)
        {
            if((pBufferHdr->pOutputPortPrivate == m_pBuffers[bufindex].start))
            {
                buf->type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf->memory    = V4L2_MEMORY_USERPTR;
                buf->index     = bufindex;
                buf->m.userptr = (unsigned long) m_pBuffers[bufindex].start;
                buf->length    = m_pBuffers[bufindex].length;
                meminfo->fd      = m_pBuffers[bufindex].fd;
                meminfo->offset  = m_pBuffers[bufindex].offset;
                buf->reserved  = (unsigned int)meminfo;
                //Pushing source buffer to V4L2
                if (ioctl(m_fd, VIDIOC_QBUF, buf) < 0)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "WFDMMSourceScreenSource::SetFreeBuffer  Error in VIDIOC_QBUF");
                }
                else
                {
                    #ifdef ENABLE_WFD_STATS
                    if(wfdStats.bEnableWfdStat)
                    {
                        // WFD:STATISTICS -- start
                        MM_Time_GetTime(&bufQTimestamp[bufindex]);
                        if(wfdStats.nMaxStatTime < (bufQTimestamp[bufindex]\
                                                  -bufDQTimestamp[bufindex]))
                        {
                           wfdStats.nMaxStatTime = bufQTimestamp[bufindex]\
                                                  -bufDQTimestamp[bufindex];
                        }
                        if( (0 != wfdStats.nStatCount) &&
                                 (wfdStats.nStatCount % 100 ==0))
                        {
                          MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
                          "WFD:STATISTICS: Roundtrip time of buffer \
                          between DQ and Q is =%lu ms Max time is =%llu ms",
                          bufQTimestamp[bufindex] -bufDQTimestamp[bufindex] ,
                          wfdStats.nMaxStatTime);

                          MM_MSG_PRIO1(MM_STATISTICS, MM_PRIO_MEDIUM,
                          "WFD:STATISTICS: Avrage Roundtrip time of buffer\
                          in Userspace is =%llu ms",wfdStats.nCumulativeStatTime/
                          wfdStats.nStatCount);
                        }
                        wfdStats.nCumulativeStatTime += (bufQTimestamp[bufindex]\
                                                        -bufDQTimestamp[bufindex]);
                        wfdStats.nStatCount++;
                        // WFD:STATISTICS -- end
                    }
                    #endif /* ENABLE_WFD_STATS */
                    isBufWithV4l2[bufindex]=1;
                    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                "WFDMMSourceScreenSource::buffer index after push is  %d",
                                buf->index);
                }
                m_nMuxBuffersReceived++;
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::SetFreeBuffer: Mux recvd buffers %ld",
                        m_nMuxBuffersReceived);
            }
        }
        if(buf != NULL)
        {
          MM_Free(buf);
          buf = NULL;
        }
        if(meminfo != NULL)
        {
          MM_Free(meminfo);
          meminfo = NULL;
        }
    }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::bad params");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/*!*************************************************************************
 * @brief     SourceThreadEntry
 *
 * @param[in] pThreadData
 *
 * @return    Error code
 *
 * @note      NONE
 **************************************************************************/
int ScreenSource::SourceThreadEntry(OMX_PTR pThreadData)
{
    int result = OMX_ErrorBadParameter;
    if (pThreadData)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                   "WFDMMSourceScreenSource::SourceThread Entry");
        result = ((ScreenSource*) pThreadData)->SourceThread();
    }
    return result;
}

/*!*************************************************************************
 * @brief     SourceThread
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::SourceThread()
{
    bKeepGoing = OMX_TRUE;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_S32     index =0;
    OMX_U32 *pEvent = NULL;
    int bufindex = 0, count = 0;
    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD: Screensource  priority b4 %d ",
      androidGetThreadPriority(tid));
    androidSetThreadPriority(0,WFD_MM_SCREEN_SOURCE_THREAD_PRIORITY);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD:Screensource  priority after%d ",
       androidGetThreadPriority(tid));

    bRunning = OMX_TRUE;
	//LOGE("PRIORITY_STAT: ScreenSourceThread Priority = -4");

    for ( int i =0; bRunning == OMX_TRUE; i++ )
    {
        if ( 0 == MM_SignalQ_Wait( m_signalQ, (void **) &pEvent ) )
        {
            switch ( *pEvent )
            {
            case SCREEN_SOURCE_PLAY_EVENT:
                {
                int nCurrBuffIndx = 0;
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource:: SCREEN_SOURCE_PLAY_EVENT");
                    // Initialize V4L2 Codec for Resume
                    if(OMX_TRUE == m_bPause)
                    {
                      OMX_ERRORTYPE eErr = InitializeV4l2Codec();
                      if(OMX_ErrorNone != eErr)
                      {
                        m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_IDLE;
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::InitializeV4l2Codec failed in play");
                        /**-----------------------------------------------------
                         Video forbidden as HDCP is not supported by sink
                         and a video clip is being played. So we shall
                         continue audio playback and move screensource to a
                         waiting state.
                        --------------------------------------------------------
                        */
                        if(m_pEventHandlerFn && m_bVideoForbid != OMX_TRUE)
                        {
                          m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR, eErr, 0);
                        }
                        continue;
                      }
                    }
                    if(m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_PLAYING)
                    {
                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                   "WFDMMSourceScreenSource::ScreenSource not in PLAYING");
                      continue;
                    }
                    // Start capturing
                        if ( m_bStreamON != OMX_TRUE)
                        {
                            if(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_PLAYING)
                            {
                                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                                m_bStreamON = OMX_TRUE;

                                if ( ioctl(m_fd, VIDIOC_STREAMON, &type) < 0 )
                                {
                                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                    "WFDMMSourceScreenSource::Error in VIDIOC_STREAMON");
                                    bKeepGoing = OMX_FALSE;
                                    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_STOPPING;
                                    if(m_pEventHandlerFn)
                                    {
                                        m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR, OMX_ErrorBadParameter, 0);
                                    }
                                    break;
                                }
                                else
                                {
                                    m_bPollContinue = OMX_TRUE;
                                    m_ScreenSourcePollThreadHandle = NULL;
                                    /**---------------------------------------------------------------
                                    *Creating poll thread
                                    *----------------------------------------------------------------*/
                                    if( (0!= MM_Thread_CreateEx( 0,
                                                                0,
                                                                ScreenSource::PollThreadEntry,
                                                                this,
                                                                WFD_MM_SOURCE_POLL_THREAD_STACK_SIZE,
                                                                "WFDScreenSourcePollThread",
                                                                &m_ScreenSourcePollThreadHandle)))

                                    {
                                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                                  "WFDMMSourceScreenSource::Poll thread creation failed");
                                      m_bPollContinue = OMX_FALSE;
                                      m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_STOPPING;
                                      if(m_pEventHandlerFn)
                                      {
                                          m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR, OMX_ErrorBadParameter, NULL);
                                      }
                                      break;
                                    }
                                    if(fd_fb > 0)
                                    {
                                        int eCntrl = m_nMirrorHint == -1 ?
                                                                 MDP_WRITEBACK_MIRROR_ON :
                                                     m_bPause == OMX_TRUE ?
                                                                 MDP_WRITEBACK_MIRROR_RESUME:
                                                                 MDP_WRITEBACK_MIRROR_ON;
                                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                                    "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT %d",
                                                    eCntrl);
                                        if(ioctl(fd_fb, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &eCntrl) < 0)
                                        {
                                           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                                      "Screensource MSMFB_WRITEBACK_SET_MIRRORING_HINT failed");
                                           m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_STOPPING;
                                           if(m_pEventHandlerFn)
                                           {
                                              m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR,
                                                                 OMX_ErrorBadParameter, 0);
                                              break;
                                           }
                                        }
                                        m_nMirrorHint = eCntrl;
                                    }
                                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                    "WFDMMSourceScreenSource::success in VIDIOC_STREAMON play1");
                                    if(m_pEventHandlerFn)
                                    {
                                        m_pEventHandlerFn( m_appData, m_nModuleId,
                                                WFDMMSRC_VIDEO_SESSION_START, OMX_ErrorNone, 0);
                                    }
                                }
                            }
                            else
                            {
                                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                    "WFDMMSourceScreenSource::StreamOFF but not it play");
                                continue;
                            }
                        }// if m_bStreamON
		    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_PLAY;
                    bKeepGoing = OMX_TRUE;
                    for ( OMX_S32 i = 0; m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_PLAY; i++ )
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceScreenSource::ScreenSource keep going");
                        if ( m_pBuffers == NULL )
                        {
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceScreenSource::m_pBuffer is NULL");
                            if(m_pEventHandlerFn)
                            {
                              m_pEventHandlerFn(m_appData, m_nModuleId, WFDMMSRC_ERROR, OMX_ErrorBadParameter, NULL);
                            }
                            bKeepGoing = OMX_FALSE;
                            continue;
                        }
                        // Reset m_ssSetV4L2Buf previous setting
                        memset(&m_sSetV4L2Buf, 0, sizeof(m_sSetV4L2Buf));
                        m_sSetV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        m_sSetV4L2Buf.memory = V4L2_MEMORY_USERPTR;
                        // Dequeue buffer
                        if(m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_PLAY)
                        {
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "WFDMMSourceScreenSource::Before DQ state not in play");
                            break;
                        }
                        if ( ioctl(m_fd, VIDIOC_DQBUF, &m_sSetV4L2Buf) < 0 )
                        {
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "WFDMMSourceScreenSource::Error in VIDIOC_DQBUF");
                            bKeepGoing = OMX_FALSE;

                            if(!(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_STOPPING))
                            {
                                if(m_pEventHandlerFn)
                                {
                                        m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR, OMX_ErrorBadParameter, NULL);
                                }
                            break;
                            }
                            else
                            {
                               continue;
                            }
                        }
                        #ifdef ENABLE_WFD_STATS
                        if(wfdStats.bEnableWfdStat)
                        {
                            // WFD:STATISTICS -- start
                            MM_Time_GetTime(&bufDQTimestamp[m_sSetV4L2Buf.index]);
                            MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_LOW,
                                        "WFD:STATISTICS: Screensource\
                                         dequeue buffer @ %lu for index %d",
                            bufDQTimestamp[m_sSetV4L2Buf.index],m_sSetV4L2Buf.index);
                            // WFD:STATISTICS -- end
                        }
                        #endif /* ENABLE_WFD_STATS */
                        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                            "WFDMMSourceScreenSource::DQed buffer (%p) of index (%d)",
                            (void *)m_sSetV4L2Buf.m.userptr,
                            m_sSetV4L2Buf.index);
                    nCurrBuffIndx = -1;
                    if(m_pscreensourceOutputBuffers != NULL)
                    {
                        // check if DQed buffer index match with processing buffer
                        for ( bufindex = 0; bufindex < m_numBuffers; bufindex++ )
                        {
                            if ( m_sSetV4L2Buf.m.userptr ==
                                (unsigned long)m_pscreensourceOutputBuffers[bufindex]->pOutputPortPrivate )
                            {
                                nCurrBuffIndx = bufindex;
                                break;
                            }
                        }
                    }

                    if(m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_PLAY)
                    {
                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                            "WFDMMSourceScreenSource::After DQBuf state not in play");
                      break;
                    }
                    if(m_pscreensourceOutputBuffers != NULL)
                    {
                        m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFlags = 0;
                        m_pscreensourceOutputBuffers[nCurrBuffIndx]->nOffset = 0;
                    }
                    if((nCurrBuffIndx == -1)||(m_sSetV4L2Buf.bytesused == 0))
                    {
                        QInUsedV4L2Buffer(&m_sSetV4L2Buf);
                        continue;
                    }
                    else if((m_sSetV4L2Buf.flags  & V4L2_QCOM_BUF_FLAG_CODECCONFIG)&&
                             (!(m_sSetV4L2Buf.flags & V4L2_BUF_FLAG_KEYFRAME)))
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource::SourceThread received sps/pps");
                        /* Dont send frame to mux, Q Back buffer to V4L2 */
                        QInUsedV4L2Buffer(&m_sSetV4L2Buf);
                        continue;
                    }
                    else
                    {
                          m_pscreensourceOutputBuffers[nCurrBuffIndx]->nTimeStamp =
                              (((long long) m_sSetV4L2Buf.timestamp.tv_sec * 1000 * 1000) + ((long long) m_sSetV4L2Buf.timestamp.tv_usec));
                          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                              "WFDMMSourceScreenSource::SourceThread TimeStamp Video =  %lld",
                              m_pscreensourceOutputBuffers[nCurrBuffIndx]->nTimeStamp);

                          if (m_sSetV4L2Buf.flags  & V4L2_BUF_FLAG_KEYFRAME)
                          {
                              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceScreenSource::received I-Frame bytes used = %d\n", m_sSetV4L2Buf.bytesused);
                              m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFlags =
                                  ( m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFlags | OMX_BUFFERFLAG_SYNCFRAME);

                          }
                     }
                     m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFilledLen =  m_sSetV4L2Buf.bytesused;
                     if( ( m_secureSession == OMX_TRUE ) )
                     {
                         m_pscreensourceOutputBuffers[nCurrBuffIndx]->pOutputPortPrivate = (void*)m_sSetV4L2Buf.m.userptr;
                     }
                     else
                     {
                         m_pscreensourceOutputBuffers[nCurrBuffIndx]->pBuffer = (OMX_U8 *)m_sSetV4L2Buf.m.userptr;
                     }

                     // Start encryption in in-place buffer if HDCP Session valid
                      if( ( m_secureSession == OMX_TRUE ) )
                      {
                        uint8 ucPESPvtData[PES_PVT_DATA_LEN] = {0};
                        // Clear previous PES Pvt Data
                     //   memset(ucPESPvtData, 0, PES_PVT_DATA_LEN );
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                    "WFDMMSourceScreenSource::HDCP Session Status(%d)",
                                    m_pHdcpHandle->m_eHdcpSessionStatus);
                        m_bHdcpSessionValid = true;

                        if(m_pHdcpOutBuffers != NULL)
                        {
#if 1 /*H/w lib*/
                              unsigned long ulStatus = m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_VIDEO ,
                                                                  (unsigned char*)ucPESPvtData,
                                                                  (unsigned char *) (m_pBuffers[nCurrBuffIndx].fd),
                                                                  (unsigned char *) (m_pHdcpOutBuffers[nCurrBuffIndx].fd),
                                                                   m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFilledLen);
#else /*S.w Lib*/
                              unsigned long ulStatus = m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_VIDEO ,
                                                                  (unsigned char*)ucPESPvtData,
                                                                  (unsigned char*)m_pscreensourceOutputBuffers[nCurrBuffIndx]->pOutputPortPrivate,
                                                                   m_pscreensourceOutputBuffers[nCurrBuffIndx]->pBuffer,
                                                                   m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFilledLen);
#endif
                              if( ulStatus == QC_HDCP_CP_ErrorCipherDisabled)
                              {
                                QInUsedV4L2Buffer(&m_sSetV4L2Buf);
                                if(false == m_pHdcpHandle->proceedWithHDCP())
                                {
                                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "Cipher enablement wait timed out");
                                  bKeepGoing = OMX_FALSE;
                                  if(!(m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_STOPPING))
                                  {
                                    if(m_pEventHandlerFn)
                                    {
                                      m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR,
                                        OMX_ErrorUndefined, NULL);
                                    }
                                    break;
                                  }
                                  else
                                  {
                                    continue;
                                  }
                                }
                                else
                                {
                                   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Cipher is not enabled still");
                                   continue;
                                }
                              }
                              else if(ulStatus == 0 )
                              {
                               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Success in HDCP Encryption");
                              }
                              else
                              {
                               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Error in HDCP Encryption,teardownsession");
                               if(m_pEventHandlerFn)
                               {
                                m_pEventHandlerFn( m_appData, m_nModuleId,WFDMMSRC_ERROR, OMX_ErrorBadParameter, NULL);
                                }
                                break;
                              }
                            }
                        for ( int idx = 0; idx < PES_PVT_DATA_LEN; idx++)
                        {
                          MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                                      "Encrypt PayloadLen[%lu] PES_PVTData[%d]:%x",
                                      m_pscreensourceOutputBuffers[nCurrBuffIndx]->nFilledLen,
                                      idx,
                                      ucPESPvtData[idx]);
                        }
                        // Fill PESPvtData at end of the encrypted buffer, as an extra data
                        FillHdcpCpExtraData( m_pscreensourceOutputBuffers[nCurrBuffIndx], ucPESPvtData, 1);

                        if(m_bFillerNaluEnabled)
                        {
                          memset((void*)ucPESPvtData, 0, sizeof(ucPESPvtData));
                          if(m_nFillerInFd > 0 && m_nFillerOutFd > 0)
                          {
#if 1 /*H/w lib*/
                             unsigned long ulStatus = m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_VIDEO ,
                                                                  (unsigned char*)ucPESPvtData,
                                                                  (unsigned char *) (m_nFillerInFd),
                                                                  (unsigned char *) (m_nFillerOutFd),
                                                                   FILLER_NALU_SIZE);
#else /*S.w Lib*/
                             unsigned long ulStatus = m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_VIDEO ,
                                                                  (unsigned char*)ucPESPvtData,
                                                                   m_pFillerDataInPtr,
                                                                   m_pFillerDataOutPtr,
                                                                   FILLER_NALU_SIZE);
#endif
                             if( ulStatus != 0)
                             {
                               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                           "ScreenSource Error in Filler NALU HDCP Encryption");
                             }
                             else
                             {
                               FillFillerBytesExtraData(m_pscreensourceOutputBuffers[nCurrBuffIndx],
                                                        ucPESPvtData, 1);
                               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                                                        "ScreenSource Filler NALU HDCP Encryption");
                             }
                          }
                        }

                      }
                      else
                      {
                        if(m_bFillerNaluEnabled)
                        {
                           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Screensource Set filler NALU extras");
                           FillFillerBytesExtraData(m_pscreensourceOutputBuffers[nCurrBuffIndx],
                                                        NULL, 1);
                        }
                      }

                      m_nMuxBuffersSent++;
#ifdef ENABLE_V4L2_DUMP
                      if(m_bEnableV4L2Dump)
                      {
                        if (( V4L2_dump_file ) && (m_secureSession == OMX_FALSE))
                        {
                           fwrite((void *)m_sSetV4L2Buf.m.userptr, m_sSetV4L2Buf.bytesused, 1, V4L2_dump_file);
                        }
                      }
#endif /* ENABLE_V4L2_DUMP */
                     m_pFrameDeliverFn(m_pscreensourceOutputBuffers[nCurrBuffIndx], m_appData);
                }
            }

            break;

            case SCREEN_SOURCE_PAUSE_EVENT:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource:: WFD_MM_SOURCE_PAUSE_EVENT");
                    m_bPause = OMX_TRUE;
                    DeInitializeV4l2Codec();
                    m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_IDLE;
                }
                break; // case SCREEN_SOURCE_PAUSE_EVENT
            case SCREEN_SOURCE_EXIT_EVENT:
              {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "WFDMMSourceScreenSource:: WFD_MM_SOURCE_EXIT_EVENT");
                 if(m_eScrnSrcState != WFDMM_SCREENSOURCE_STATE_INIT)
                 {
                    m_bPause = OMX_FALSE;
                    DeInitializeV4l2Codec();
                 }
                 m_eScrnSrcState = WFDMM_SCREENSOURCE_STATE_INIT;
                 return OMX_ErrorNone;
              }

            }//switch-case(pEvent)
        }//if MM_SignalQ_Wait
    }//for loop bRunning
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSourceScreenSource:: SCREEN_SOURCE_EXITED");
    return result;
}

/*!*************************************************************************
 * @brief     QInUsedV4L2Buffer
 *
 * @param[in] pV4L2buf
 *
 * @return    NONE
 *
 * @note      NONE
 **************************************************************************/
void ScreenSource::QInUsedV4L2Buffer( v4l2_buffer *pV4L2buf)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"ScreenSource::QInUsedV4L2Buffer");
    int nIndex = 0;
    struct mem_info sMemInfo;
    sMemInfo.fd=0;

    m_bCritTransact = OMX_TRUE;

    //WARNING DO NOT ADD return statment in IF block
    if( pV4L2buf && (m_eScrnSrcState == WFDMM_SCREENSOURCE_STATE_PLAY))
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "WFD ScreenSource::req.count %d buffers = %ld",req.count, m_numBuffers);
        for( nIndex= 0; nIndex < m_numBuffers; nIndex++ )
        {
            //Find used in buffer index
            if(m_sSetV4L2Buf.m.userptr == (unsigned long) m_pBuffers[nIndex].start)
            {
                pV4L2buf->index  = nIndex;
                sMemInfo.fd = m_pBuffers[nIndex].fd;
                sMemInfo.offset = m_pBuffers[nIndex].offset;
                MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                            "ScreenSource::useprt (%p) bufindex is %d",
                            (void *)m_sSetV4L2Buf.m.userptr,nIndex);
                break;
            }
        }
        //Queue back buffer to V4L2
        pV4L2buf->type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        pV4L2buf->memory    = V4L2_MEMORY_USERPTR;
        pV4L2buf->m.userptr = (unsigned long) m_pBuffers[nIndex].start;
        pV4L2buf->length    = m_pBuffers[nIndex].length;
        pV4L2buf->reserved  = (unsigned int)&sMemInfo;

        if (ioctl(m_fd, VIDIOC_QBUF, pV4L2buf) < 0)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "ScreenSource::Source thread Error in VIDIOC_QBUF");
        }
        else
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "ScreenSource::Source thread buffer index after push is%d",
                        pV4L2buf->index);
            isBufWithV4l2[m_sSetV4L2Buf.index]=1;
         }//if-else ioctl
    }//if pV4L2Buf valid
    m_bCritTransact = OMX_FALSE;
}

/*!*************************************************************************
  * @brief       Set Control parameters
 *
 * @param[in]    Fd
 *
 * @return       Error Code
 ***************************************************************************/
int ScreenSource::set_control_parameters(int fd)
{
    struct v4l2_control *ctrl;
    unsigned int i;
    int rc = 0;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::set_control_parameters\n");
    for (i = 0; i < ARRAY_SZ(controls); ++i)
    {
     //Now low bitrate wfd session starts in vbr_cfr mode
     if( m_nBitrateFactor < 13 )
     {
     // CBR_VFR MODE
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::Set Controls for high bit rate\n");
     }
     else
     {
      // VBR_CFR MODE
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::Set Controls for low bit rate\n");
      if(controls[i].id == V4L2_CID_MPEG_VIDEO_BITRATE_PEAK)
      {
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::Don't set Peak Bitrate mode = %d ", controls[i].id);
        continue;
       }
       if(controls[i].id == V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL)
       {
        controls[i].value = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::set VBR_CFR mode = %d ", controls[i].id);
       }
       if(controls[i].id == V4L2_CID_MPEG_VIDEO_H264_MAX_QP)
       {
         //Set max QP;
         controls[i].value = 32;
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                    "WFDMMSourceScreenSource::MAX_QP = %d ", controls[i].value);
        }
        if(controls[i].id == V4L2_CID_MPEG_VIDEO_H264_MIN_QP)
        {
         //Set Min QP;
         controls[i].value = 10;
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                   "WFDMMSourceScreenSource::MIN_QP = %d ", controls[i].value);
        }
      }
       if(controls[i].id == V4L2_CID_MPEG_VIDC_SET_PERF_LEVEL)
       {
        if(controls[i].value == -1)
        {
          continue;
        }
       }
        rc = ioctl(fd, VIDIOC_S_CTRL, &controls[i]);
        if (rc)
        {
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                "WFDMMSourceScreenSource::Failed to set control index is %d\n", i);
         }
    }
    return rc;
}

/*!*************************************************************************
  * @brief        Change control value
 *
 * @param[in]   id
 * @param[in]   value
 *
 * @return       Error Code
 ***************************************************************************/
void ScreenSource::change_control_value(unsigned int id, int value)
{
    unsigned int i;
    for (i = 0; i < ARRAY_SZ(controls); ++i)
    {
        if (controls[i].id == id)
        {
            controls[i].value = value;
        }
    }
}
/*!*************************************************************************
  * @brief        getV4L2Level
 *
 * @param[in]   level - need to convert from WFD levels bitmap value to V4L2 enum
 *
 * @return       level value to be set to V4L2
 ***************************************************************************/
OMX_S32 ScreenSource::getV4L2Level(OMX_U8 level)
{
  OMX_S32 return_value = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
  switch (level)
  {
    case 1:
      return_value = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
      break;
    case 2:
      return_value = V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
      break;
    case 4:
      return_value = V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
      break;
    case 8:
      return_value = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
      break;
    case 16:
      return_value = V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
      break;
    default:
       return_value = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
       break;
  }
  return return_value;
}

/***********************************************************************************!
 * @brief      Fill Extra data in buffer
 * @details    Fill HDCP PES pvt extra data at end of buffer.
 * @param[in]  pBufHdr         Payload buffer header
 * @param[in]  pucPESPvtHeader PES Pvt data
 * @param[in]  nPortIndex      Port index
 * @return     RETURN 'OMX_ErrorNone' if SUCCESS
 *             OMX_ErrorBadParameter code in FAILURE
 ***********************************************************************************/
OMX_ERRORTYPE  ScreenSource::FillHdcpCpExtraData(
                          OMX_BUFFERHEADERTYPE *pBufHdr,
                          OMX_U8* pucPESPvtHeader,
                          OMX_U32 nPortIndex)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  uint32 ulAddr;
  OMX_OTHER_EXTRADATATYPE *pHdcpCpExtraData;
  if( (NULL != pBufHdr ) && (NULL != pucPESPvtHeader))
  {
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                "WFDMMSourceScreenSource::pBufHdr->pBuffer[%p] FilledLen[%ld]",
                pBufHdr->pBuffer,
                pBufHdr->nFilledLen);
    /* Skip encoded frame payload length filled by V4L2 driver */
    ulAddr = (uint32) ( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceScreenSource::ulAddr[%ld]", ulAddr);
    /* Aligned address to DWORD boundary */
    ulAddr = (ulAddr + 0x3) & (~0x3);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceScreenSource::Aligned ulAddr[%ld]", ulAddr);
    pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
    /* Update pBufHdr flag, to indicate that it carry extra data */
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceScreenSource::pHdcpCpExtraData[%p]", pHdcpCpExtraData);

    FLAG_SET(pBufHdr,OMX_BUFFERFLAG_EXTRADATA);
    /* Extra Data size = ExtraDataType*/
    pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) + sizeof(OMX_U8)* PES_PVT_DATA_LEN -4;
    pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
    pHdcpCpExtraData->nPortIndex = nPortIndex;
    pHdcpCpExtraData->nDataSize = PES_PVT_DATA_LEN;
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDMMSourceScreenSource::size[%ld] PortIndex[%ld] nDataSize[%ld]",
      pHdcpCpExtraData->nSize,
      pHdcpCpExtraData->nPortIndex,pHdcpCpExtraData->nDataSize );
    pHdcpCpExtraData->eType = (OMX_EXTRADATATYPE)QOMX_ExtraDataHDCPEncryptionInfo;
    /* Fill PES_PVT_DATA into data*/
    memcpy(pHdcpCpExtraData->data,pucPESPvtHeader, PES_PVT_DATA_LEN );
    /* Append OMX_ExtraDataNone */
    ulAddr += pHdcpCpExtraData->nSize;
    pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
    pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE);
    pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
    pHdcpCpExtraData->nPortIndex = nPortIndex;
    pHdcpCpExtraData->eType = OMX_ExtraDataNone;
    pHdcpCpExtraData->nDataSize = 0;
  }
  else
  {
    eError = OMX_ErrorBadParameter;
  }
  return eError;
}

/***********************************************************************************!
 * @brief      Fill FillerBytes Extra data in buffer
 * @details    Fill Fillerbytes NALU extra data at end of buffer.
 * @param[in]  pBufHdr         Payload buffer header
 * @param[in]  pucPESPvtHeader PES Pvt data
 * @param[in]  nPortIndex      Port index
 * @return     RETURN 'OMX_ErrorNone' if SUCCESS
 *             OMX_ErrorBadParameter code in FAILURE
 ***********************************************************************************/
bool  ScreenSource::FillFillerBytesExtraData(
                          OMX_BUFFERHEADERTYPE *pBuffHdr,
                          OMX_U8* pucPESPvtHeader,
                          OMX_U32 nPortIndex)
{
  OMX_OTHER_EXTRADATATYPE *pExtra;

  if(NULL != pBuffHdr)
  {
    OMX_U8 *pTmp = pBuffHdr->pBuffer +
                         pBuffHdr->nOffset + pBuffHdr->nFilledLen + 3;

    pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U32) pTmp) & ~3);

    if(pBuffHdr->nFlags & OMX_BUFFERFLAG_EXTRADATA)
    {
      //Extra Data already set. Find the end
      while(pExtra->eType != OMX_ExtraDataNone)
      {
        pExtra = (OMX_OTHER_EXTRADATATYPE *)
                      (((OMX_U8 *) pExtra) + pExtra->nSize);

        if((OMX_U32)pExtra + sizeof(OMX_OTHER_EXTRADATATYPE) >=
              (OMX_U32)pBuffHdr->pBuffer + pBuffHdr->nAllocLen)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "Fiiler Bytes Reached out of bounds");
          return false;
        }
      }
    }

    OMX_U32 nBytesToFill = FILLER_NALU_SIZE + 1 /*Signalling Byte*/
                                            + 1 /*Length */;

    if(pucPESPvtHeader != NULL)
    {
      /*Filler Nalu us encrypted*/
      nBytesToFill += PES_PVT_DATA_LEN + 1 /*Length */;
    }

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "Screensource Fiiler Bytes nBytesToFill = %lu",
                  nBytesToFill);

    /** Check out of bound access in the pBuffer */
    if((OMX_U32)pExtra + sizeof(OMX_OTHER_EXTRADATATYPE) +
                                           ((nBytesToFill +3) & (~3))  >
               (OMX_U32)pBuffHdr->pBuffer + pBuffHdr->nAllocLen)
    {
      /*Can't fit in filler bytes*/
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "Screensource Can't fit in fillerNALU");
      return false;
    }

    FLAG_SET(pBuffHdr,OMX_BUFFERFLAG_EXTRADATA);

    /* Extra Data size = ExtraDataType*/
    pExtra->nVersion = pBuffHdr->nVersion;
    pExtra->nPortIndex = nPortIndex;
    pExtra->nDataSize = nBytesToFill;
    nBytesToFill += 3;
    nBytesToFill &= (~3);
    pExtra->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) + nBytesToFill -4;

    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDMMSourceScreenSource:: Filler size[%ld] PortIndex[%ld] nDataSize[%ld]",
      pExtra->nSize,
      pExtra->nPortIndex,pExtra->nDataSize );

    /* Using MAX to pass generic ExtraData Info using signalling byte to
       identify the type of data*/
    pExtra->eType = OMX_ExtraDataMax;

    /**------------------------------------------------------------------------
    Filler NALU format:= |SigByte|Size|PES Pvt Data|Size|Filler NALU |
    ---------------------------------------------------------------------------
    */

    /**Fill the extra data bytes */
    uint32 nOffset = 0;

    /*Signal Encrypted Payload or non-encrypted Payload*/
    pExtra->data[nOffset] = VIDEO_PVTDATA_TYPE_FILLERNALU;

    if(pucPESPvtHeader != NULL)
    {
      pExtra->data[nOffset] = VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED;
    }
    nOffset++;

    /**If encypted first add the PES private data */
    if(pucPESPvtHeader != NULL)
    {
      pExtra->data[nOffset] = PES_PVT_DATA_LEN;
      nOffset++;
      /* Fill PES_PVT_DATA into data*/
      memcpy(pExtra->data + nOffset,pucPESPvtHeader, PES_PVT_DATA_LEN );
      nOffset += PES_PVT_DATA_LEN;
    }

    /** FIll the filler NALU bytes */
    pExtra->data[nOffset] = FILLER_NALU_SIZE;
    nOffset++;
    memcpy(pExtra->data + nOffset, m_pFillerDataOutPtr, FILLER_NALU_SIZE);

    /** Fill the extradataNone if there is space left. Mux will
     *  check against AllocLen to prevent access beyond limit */
    pExtra = (OMX_OTHER_EXTRADATATYPE *)
                           ((OMX_U32)pExtra + pExtra->nSize);
    if((OMX_U32)pExtra + sizeof(OMX_OTHER_EXTRADATATYPE) >=
                    (OMX_U32)pBuffHdr->pBuffer + pBuffHdr->nAllocLen)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "Fiiler Bytes: XtraNone Reached out of bounds");
      return true;
    }
    pExtra->nSize = sizeof(OMX_OTHER_EXTRADATATYPE);
    pExtra->nVersion = pBuffHdr->nVersion;
    pExtra->nPortIndex = nPortIndex;
    pExtra->eType = OMX_ExtraDataNone;
    pExtra->nDataSize = 0;
  }

  return false;
}


/*!*************************************************************************
 * @brief     PollThreadEntry
 *
 * @param[in] pThreadData
 *
 * @return    Error code
 *
 * @note      NONE
 **************************************************************************/
int ScreenSource::PollThreadEntry(OMX_PTR pThreadData)
{
    int result = OMX_ErrorBadParameter;
    if (pThreadData)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                   "WFDMMSourceScreenSource::PollThread Entry");
        result = ((ScreenSource*) pThreadData)->PollThread();
    }
    return result;
}

/*!*************************************************************************
 * @brief     PollThread
 *
 * @param[in] NONE
 *
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
OMX_ERRORTYPE ScreenSource::PollThread()
{
  int rc = 0;
  int tid = androidGetTid();
  struct pollfd fds;

  memset(&fds, 0, sizeof(pollfd));
  fds.fd = m_fd;

  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD: PollThread priority b4 %d ",
      androidGetThreadPriority(tid));
  androidSetThreadPriority(0, WFD_MM_POLL_THREAD_PRIORITY);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "WFDD:Poll thread priority after%d ",
    androidGetThreadPriority(tid));

  do
  {
    fds.events = ~0; /*Everything*/
    rc = poll(&fds, 1 /* number of items in the fds array */, -1 /*no timeout*/);

    if (rc <= 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                    "WFDMMSourceScreenSource::PollThread Error from poll");
      break;
    }

    if((rc == EINTR) && (m_bStreamON == OMX_FALSE))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "WFDMMSourceScreenSource::PollThread EINTR and STREAM OFF already called exiting from this thread");
      break;
    }

    if (fds.revents & POLLPRI)
    {
      struct v4l2_event events;
      if (ioctl(m_fd, VIDIOC_DQEVENT, &events) < 0)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                 "WFDMMSourceScreenSource::PollThread error in VIDIOC_DQEVENT ");
        break;
      }

      switch (events.type)
      {
        case V4L2_EVENT_MSM_VIDC_SYS_ERROR:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                   "WFDMMSourceScreenSource::PollThread received V4L2_EVENT_MSM_VIDC_SYS_ERROR");
          if(m_pEventHandlerFn)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                    "WFDMMSourceScreenSource::PollThread received V4L2_EVENT_MSM_VIDC_SYS_ERROR");
            m_pEventHandlerFn( m_appData, m_nModuleId, WFDMMSRC_ERROR, OMX_ErrorBadParameter, NULL);
          }
          m_bPollContinue = OMX_FALSE;
          break;
        }
        default:
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                    "WFDMMSourceScreenSource::PollThread Got event %d...ignoring\n", events.type);
        }
      }
    }
  } while ((!(fds.revents & POLLERR)) && (rc != EFAULT) && (m_bPollContinue == OMX_TRUE));

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
        "WFDMMSourceScreenSource:: PollThread is exiting");
  MM_Thread_Exit( ScreenSource::m_ScreenSourcePollThreadHandle, 0 );
  return OMX_ErrorNone;
}

/*!*************************************************************************
 * @brief     Subscribe_Events
 *
 * @param[in] fd  file descriptor
 *            subscribe  subscribe
 * @return    OMX_ERRORTYPE
 *
 * @note      NONE
 **************************************************************************/
int ScreenSource::Subscribe_Events(int fd, bool subscribe)
{
  struct v4l2_event_subscription sub;
  int rc = 0;

  memset(&sub, 0, sizeof(sub));
  for (int index = 0; index < ARRAY_SZ(V4L2_events); ++index)
  {
    int action = subscribe ? VIDIOC_SUBSCRIBE_EVENT :
    VIDIOC_UNSUBSCRIBE_EVENT;

    sub.type = V4L2_events[index];
    rc = ioctl(fd, action, &sub);
    if (rc)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"ScreenSource::subscribe_events");
      return rc;
    }
  }
  return 0;
}
