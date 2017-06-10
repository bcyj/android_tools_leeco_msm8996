/* =======================================================================
                              WFDMMSourceScreenSource.h
DESCRIPTION

Header file for WFDMMSourceScreenSource.cpp file

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/inc/WFDMMSourceScreenSource.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */

#ifndef __WFD_MM_SCREEN_SOURCE_H__
#define __WFD_MM_SCREEN_SOURCE_H__

/*========================================================================
 *                    INCLUDE FILES FOR MODULE
 *========================================================================*/
#include "MMTimer.h"
#include "MMThread.h"
#include "MMSignal.h"
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "WFDMMSourcePmem.h"
#include "WFDMMSourceComDef.h"
#include "AEEStdDef.h"
#include "WFDMMIonMemory.h"

#define SCREEN_SOURCE_BUFFERS 5
#define ARRAY_SZ(a) (sizeof(a) / sizeof((a)[0]))
#define WFD_H264_MAX_QP 44
#define WFD_H264_MIN_QP 15
#define DEFAULT_I_FRAME_INTERVAL 4
#define DEFAULT_BITRATE 10 * 1000 * 1000
#define DEFAULT_BITRATE_PEAK 2 * DEFAULT_BITRATE
#define FILLER_ION_BUF_SIZE 4096
#define FILLER_NALU_SIZE    8
#define NUMBER_OF_BFRAME 0
/*========================================================================
 *                     CLASS F/W DECLARATIONS
 *========================================================================*/
class SignalQueue;
class CWFD_HdcpCp;

/*!
 *@brief Input Event structure.
 */
struct input_event
{
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

/*!************************************************************************
 *@brief memory information structure.
 **************************************************************************/
struct mem_info
{
    __u32 fd;
    __u32 offset;
    __u32 size;
};

/*!
 *@brief V4L2 control
 */
static struct v4l2_control controls[] = {
    {
        V4L2_CID_MPEG_VIDEO_BITRATE,
        DEFAULT_BITRATE,
    },
    {
        V4L2_CID_MPEG_VIDEO_H264_PROFILE,
        V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE,
    },
    {
        V4L2_CID_MPEG_VIDEO_H264_LEVEL,
        V4L2_MPEG_VIDEO_H264_LEVEL_3_1,
    },
    {
        V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE,
        V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_RANDOM,
    },
    {
        V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE,
        V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_IGNORE,
    },
    {
        V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME,
        V4L2_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE_DISABLED
    },
    {
        V4L2_CID_MPEG_VIDEO_H264_MAX_QP,
        WFD_H264_MAX_QP
    },
    {
        V4L2_CID_MPEG_VIDEO_H264_MIN_QP,
        WFD_H264_MIN_QP
    },
    {
      V4L2_CID_MPEG_VIDEO_HEADER_MODE,
      V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_I_FRAME
    },
    {
      V4L2_CID_MPEG_VIDC_SET_PERF_LEVEL,
      -1
    },
    {
      V4L2_CID_MPEG_VIDC_VIDEO_H264_AU_DELIMITER,
      V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_ENABLED
    },
    {
      V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE,
      V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC
    },
    {
      V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO,
      V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_ENABLED
    },
    {
      V4L2_CID_MPEG_VIDEO_BITRATE_PEAK,
      DEFAULT_BITRATE_PEAK
    },
    {
      V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL,
      V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR
    },
    {
      V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES,
      NUMBER_OF_BFRAME
    },
    {
      V4L2_CID_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY,
      V4L2_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY_ENABLED
    }
};


/**
 * @brief Delivers YUV data in two different modes.
 *
 * In live mode, buffers are pre-populated with YUV data at the time
 * of configuration. The source will loop through and deliver the
 * pre-populated buffers throughout the life of the session. Frames will be
 * delivered at the configured frame rate. This mode is useful for gathering
 * performance statistics as no file reads are performed at run-time.
 *
 * In  non-live mode, buffers are populated with YUV data at run time.
 * Buffers are delivered downstream as they become available. Timestamps
 * are based on the configured frame rate, not on the system clock.
 *
 */

class ScreenSource
{
public:

    /**
     * @brief Frame callback for YUV buffer deliver
     */
    typedef void (*FrameDeliveryFnType) (OMX_BUFFERHEADERTYPE* pBuffer, void *);

public:

    /**
     * @brief Constructor
     */
    ScreenSource();

    /**
     * @brief Destructor
     */
    ~ScreenSource();

public:

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
    OMX_ERRORTYPE Configure(
        VideoEncStaticConfigType* pConfig,
        OMX_S32 nBuffers,
        FrameDeliveryFnType pFrameDeliverFn,
        eventHandlerType pEventHandlerFn,
        OMX_BOOL bFrameSkipEnabled,
        OMX_U64 nFrameSkipLimitInterval,
        OMX_U32 nModuleId,
        void *appData);

   /*!*************************************************************************
    * @brief     Changes the frame rate
    *            The frame rate will take effect immediately.
    * @param[in] nFramerate New frame rate
    * @return    OMX_ERRORTYPE
    * @note
    **************************************************************************/
    OMX_ERRORTYPE ChangeFrameRate(OMX_S32 nFramerate);

   /*!*************************************************************************
    * @brief     Changes the bit rate
    *            The bit rate will take effect immediately.
    *
    * @param[in] nBitrate The new bit rate.
    *
    * @return    OMX_ERRORTYPE
    * @note      None
    **************************************************************************/
    OMX_ERRORTYPE ChangeBitrate(OMX_S32 nBitrate);


   /**************************************************************************
    * @brief        Request an I-Frame
    *
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    **************************************************************************/
    OMX_ERRORTYPE RequestIntraVOP();

   /**************************************************************************
    * @brief Starts the delivery of buffers.
    *
    *               All buffers should be registered through the SetFreeBuffer
    *               function prior to calling this function. Source will continue
    *               to deliver buffers at the specified rate until the specified
    *               number of frames have been delivered.
    *
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    *
    * @note         Note that Source will not deliver buffers unless it has ownership
    *               of atleast 1 buffer. If Source does not have ownership of buffer when
    *               timer expires, it will block until a buffer is given to the component.
    *               Free buffers should be given to the Source through SetFreeBuffer
    *               function.
    **************************************************************************/
    OMX_ERRORTYPE Start();

   /**************************************************************************
    * @brief        Wait for the thread to finish.
    *
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    *
    * @note         Function will block until the Source has delivered all frames.
    **************************************************************************/
    OMX_ERRORTYPE Finish();

   /**************************************************************************
    * @brief        Wait for the thread to Pause.
    *
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    *
    * @note         Function will block until the Source has delivered all frames.
    **************************************************************************/
    OMX_ERRORTYPE Pause();

   /**************************************************************************
    * @brief        Gives ownership of the buffer to the source.
    *               All buffers must be registered with the Source prior
    *               to invoking Start. This will give initial ownership to
    *               the source.
    *
    * @param[in]    pBuffer Pointer to the buffer
    *
    * @return       OMX_ERRORTYPE
    *
    * @note         After Start is called, buffers must be given ownership
    *               when YUV buffers are free.
    ***************************************************************************/
    OMX_ERRORTYPE SetFreeBuffer(OMX_BUFFERHEADERTYPE* pBuffer);

   /***************************************************************************
    * @brief        Get number of Output buffers required by the port.
    *
    * @param[in]    nPortIndex[in] - Port number
    * @param[in]    nBuffers[rout] - Number of buffers
    *
    * @return       OMX_ERRORTYPE
    ***************************************************************************/
    OMX_ERRORTYPE GetOutNumBuffers(OMX_U32 nPortIndex, OMX_U32 *nBuffers);

   /***************************************************************************
    * @brief        Get omx buffer header
    *
    * @param[in]    None
    *
    * @return       OMX_BUFFERHEADERTYPE
    ***************************************************************************/
    OMX_BUFFERHEADERTYPE** GetBuffers();

   /***************************************************************************
    * @brief        Set Control parameters
    *
    * @param[in]    Descriptor info
    *
    * @return       Error Code
    ***************************************************************************/
    int set_control_parameters(int fd);
    void change_control_value(unsigned int id, int value);
    int Subscribe_Events(int fd, bool subscribe);
    OMX_ERRORTYPE  FillHdcpCpExtraData(OMX_BUFFERHEADERTYPE *pBufHdr,
                                             OMX_U8* pucPESPvtHeader,
                                             OMX_U32 nPortIndex);
    bool  FillFillerBytesExtraData(
                                      OMX_BUFFERHEADERTYPE *pBufHdr,
                                      OMX_U8* pucPESPvtHeader,
                                      OMX_U32 nPortIndex);
    CWFD_HdcpCp* m_pHdcpHandle;
    bool m_bHdcpSessionValid;
private:
    static int SourceThreadEntry(OMX_PTR pThreadData);
    OMX_ERRORTYPE SourceThread();
    static int PollThreadEntry(OMX_PTR pThreadData);
    OMX_ERRORTYPE PollThread();
    static void Screen_TimerHandler (void *);
    OMX_ERRORTYPE CreateResources();
    void QInUsedV4L2Buffer( v4l2_buffer *pV4L2buf);
    OMX_S32 getV4L2Level(OMX_U8 level);

    typedef enum WFDScreenSourceStates
    {
        WFDMM_SCREENSOURCE_STATE_INIT = 0,
        WFDMM_SCREENSOURCE_STATE_IDLE,
        WFDMM_SCREENSOURCE_STATE_ERROR,
        WFDMM_SCREENSOURCE_STATE_PLAYING,
        WFDMM_SCREENSOURCE_STATE_PLAY,
        WFDMM_SCREENSOURCE_STATE_STOPPING,
        WFDMM_SCREENSOURCE_STATE_UNDEFINED
    }WFDScreenSourceStatesType;

    WFDScreenSourceStatesType m_eScrnSrcState;
    eventHandlerType m_pEventHandlerFn;
    OMX_U32 m_nModuleId;
    OMX_BOOL bKeepGoing;
    OMX_BOOL bRunning;
    OMX_BOOL m_bStreamON;
    struct buffer *m_pBuffers;
    struct buffer *m_pHdcpOutBuffers;
    OMX_S32 m_nFramerate;
    OMX_S32 m_nFrameWidth;
    OMX_S32 m_nFrameHeight;
    OMX_S32 m_nDVSXOffset;
    OMX_S32 m_nDVSYOffset;
    OMX_S32 m_nProfile;
    OMX_S32 m_nLevel;
    OMX_S32 m_nBitrateFactor;
    OMX_S32 m_nBitrate;
    OMX_TICKS m_nTimeStamp;
    OMX_BOOL m_bFrameSkipEnabled;
    OMX_U64  m_nFrameSkipLimitInterval;
    OMX_U32 m_nV4L2QueueSize;
    OMX_U32 m_nMuxBufferCount;
    MM_HANDLE m_ScreenSourceThreadHandle;
    MM_HANDLE m_ScreenSourcePollThreadHandle;
    OMX_BOOL m_bStarted;
    OMX_BOOL m_bPause;
    OMX_S32  m_nMirrorHint;
    OMX_S32  m_nFBPanelIdx;
    OMX_S32 isBufWithV4l2[SCREEN_SOURCE_BUFFERS];
    unsigned long bufQTimestamp[SCREEN_SOURCE_BUFFERS];
    unsigned long bufDQTimestamp[SCREEN_SOURCE_BUFFERS];
    SignalQueue* m_pBufferQueue;
    FrameDeliveryFnType m_pFrameDeliverFn;
    void *m_appData;
    int memfd;
    int m_fd;
    int ionfd;
    int fd_fb;
    enum v4l2_buf_type type;
    struct v4l2_buffer m_sSetV4L2Buf;
    struct mem_info m_sMemInfo[SCREEN_SOURCE_BUFFERS];
    struct v4l2_requestbuffers req;
    struct v4l2_streamparm m_parm;
    struct v4l2_qcom_frameskip m_frameskip;
#ifdef ENABLE_V4L2_DUMP
      FILE *V4L2_dump_file;
      OMX_BOOL m_bEnableV4L2Dump;
#endif /* ENABLE_V4L2_DUMP */
    OMX_BUFFERHEADERTYPE** m_pscreensourceOutputBuffers;
    OMX_S32 m_numBuffers;
    MM_HANDLE m_signalQ;
    static const OMX_U32 SCREEN_SOURCE_PLAY_EVENT;
    static const OMX_U32 SCREEN_SOURCE_PAUSE_EVENT;
    static const OMX_U32 SCREEN_SOURCE_EXIT_EVENT;
    MM_HANDLE m_WFDScreenSourcePlaySignal;
    MM_HANDLE m_WFDScreenSourcePauseSignal;
    MM_HANDLE m_WFDScreenSourceExitSignal;
    OMX_ERRORTYPE InitializeV4l2Codec();
    void DeInitializeV4l2Codec();
    OMX_BOOL m_secureSession;
    OMX_S32 m_nMuxBuffersSent;
    OMX_S32 m_nMuxBuffersReceived;
    #ifdef ENABLE_WFD_STATS
    typedef struct wfd_stats_struct
    {
        OMX_U32  nStatCount;
        OMX_U64  nMaxStatTime;
        OMX_U64  nCumulativeStatTime;
        OMX_BOOL bEnableWfdStat;
    }wfd_stats;
    wfd_stats wfdStats;
    MM_HANDLE m_pStatTimer;
    OMX_S32 m_nDuration;
    static void readStatTimerHandler(void * ptr);
    #endif
    static OMX_U8 sFillerNALU[FILLER_NALU_SIZE];
    int m_nFillerInFd;
    int m_nFillerOutFd;
    ion_user_handle_t m_hFillerInHandle;
    ion_user_handle_t m_hFillerOutHandle;
    unsigned char *m_pFillerDataInPtr;
    unsigned char *m_pFillerDataOutPtr;
    OMX_BOOL     m_bFillerNaluEnabled;
    OMX_BOOL m_bVideoForbid;
    OMX_BOOL m_bCritTransact;
    OMX_BOOL m_bPollContinue;
  };
#endif // #ifndef __WFD_MM_SCREEN_SOURCE_H__
