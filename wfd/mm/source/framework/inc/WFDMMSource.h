#ifndef __WFD_MM_SOURCE_H__
#define __WFD_MM_SOURCE_H__

/* =======================================================================
                              WFDMMSource.h
DESCRIPTION

Header file for WFDMMSource.cpp includes relavent declarations

  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/inc/WFDMMSource.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$
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
#include "wdsm_mm_interface.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "MMSignal.h"
#include "MMThread.h"
#include "wfdmmsource_queue.h"
#include "WFDMMSourceComDef.h"
#include "WFDMMSourceMuxDefines.h"
#include "WFDMMSourceAudioSource.h"

#define MM_BITRATE_UPDATE_PERIOD    4000 //ms
#define MM_LINKSPEED_CHECK_INTERVAL 500 //ms
#define MM_BITRATE_UPDATE_AVG_WINDOW  (MM_BITRATE_UPDATE_PERIOD / MM_LINKSPEED_CHECK_INTERVAL)

#ifdef WFD_VDS_ARCH
class VideoSource;   // forward declaration
#else
class ScreenSource;  // forward declaration
#endif
class AudioSource;
class Muxer;

  /* Creates WFD MM Source instance */
/*!
   @brief   Creates WFD MM Source instance

   @detail  It takes file_format, file_brand, output_handle, notification, callBack and pClientData as input parameters.
            Params will provide the necessary information to configure the writer library.
            file_format will tell the file format that we are going to record.
            file_brand will tell the Major brand that we are going to record.
            output_handle - The output that ultimately we are going to write.
            callBack - Notifaction call back to the application.
            MUX Module will get the audio/video/text/data frames/samples, UUID, telop etc.. fomats and writes into the o/p.

   @param[in]  Params will provide the necessary information to configure the writer library.
               file_format will tell the file format that we are going to record.
               file_brand will tell the Major brand that we are going to record.
               output_handle - The output that ultimately we are going to write.
               callBack - Notifaction call back to the application.

   @return    Pointer to MUX base instance.

   @note    It is expected that except pOputStream nothing should be NULL.
   */
class WFDMMSource
{
public:
    WFDMMSource( WFD_device_t wfd_device_type,
        WFD_MM_capability_t *WFD_negotiated_capability,
        WFD_MM_callbacks_t  *pCallback);

    virtual ~WFDMMSource();
    OMX_ERRORTYPE WFDMMSourceUpdateSession(WFD_MM_capability_t *WFD_negotiated_capability,
                                           wfd_mm_update_session_cb pUpdateSessionCb);

    OMX_ERRORTYPE WFDMMSourcePlay(WFD_MM_HANDLE wfdmmhandle,
                                  WFD_AV_select_t av_select,
                                  wfd_mm_stream_play_cb mm_stream_play_cb);

    OMX_ERRORTYPE WFDMMSourcePause(WFD_MM_HANDLE wfdmmhandle,
                                   WFD_AV_select_t av_select,
                                   wfd_mm_stream_pause_cb mm_stream_pause_cb);

    OMX_ERRORTYPE GenerateIFrameNext(WFD_MM_HANDLE wfdmmhandle);

    OMX_ERRORTYPE ChangeBitrate(WFD_MM_HANDLE wfdmmhandle,
                                OMX_S32 nBitrate);

    OMX_ERRORTYPE ExecuteRunTimeCommand(WFD_MM_HANDLE wfdmmhandle,
                         WFD_runtime_cmd_t eCommand);

    OMX_ERRORTYPE ChangeFramerate(WFD_MM_HANDLE wfdmmhandle,
                                  OMX_S32 nFramerate);

    OMX_ERRORTYPE GetCurrentPTS(WFD_MM_HANDLE wfdmmhandle,
                                             OMX_U64 *nTimeMs);


    OMX_ERRORTYPE WFDMMGetProposedCapability(WFD_MM_HANDLE hHandle,
                                             WFD_MM_capability_t* pMMCfg_local,
                                             WFD_MM_capability_t* pMMCfg_remote,
                                             WFD_MM_capability_t* pMMCfg_proposed);

    WFD_MM_callbacks_t sCallBacks;

private:
    OMX_ERRORTYPE InitializeThreadsAndQueues();
    OMX_ERRORTYPE CreateComponents();
    OMX_ERRORTYPE ConfigureComponents();
    bool          AreAVCodecsValid(WFD_MM_capability_t* pNegotiatedCap);
    static void SourceDeliveryFn(OMX_BUFFERHEADERTYPE* pBuffer, void *);
    static void AudioSourceDeliveryFn(OMX_BUFFERHEADERTYPE* pBuffer, void *);
    static void SinkReleaseFn(OMX_BUFFERHEADERTYPE* pBuffer, void *);
    static void AudioSinkReleaseFn(OMX_BUFFERHEADERTYPE* pBuffer, void *);

    static OMX_ERRORTYPE SinkEmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                             OMX_IN OMX_PTR pAppData,
                                             OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
    //! The WFD MM Source thread entry function.
    static int WFDMMSourceThreadEntry( void* ptr );

    ///// Temp AV format change

    static void AVFormatChangeTimerCb(void * handle);
    ////
    static void LinkSpeedCheckTimerCb(void * handle);
    void NetworkAdaptiveRateControl(OMX_U32 nLinkSpeed);
    OMX_U32 CalculateLinkspeedWeightedAvg();
    OMX_U32 GetLinkSpeedFromWLANDriver();
    //! The WFD MM Source working thread entry function.
    int WFDMMSourceThread( void );
    OMX_ERRORTYPE Play();
    OMX_ERRORTYPE Stop();
    OMX_ERRORTYPE Pause();
    void initDefaults();
    OMX_ERRORTYPE ExtractVideoParameters();
    OMX_ERRORTYPE ExtractAudioParameters();
    OMX_ERRORTYPE FillSinkParameters();
    void getResolutionRefreshRate(uint32, uint32, uint32 );
    void getLPCMAudioParams(uint32 );
    void getAACAudioParams(uint32 );
    void getAC3AudioParams(uint32 supported_audio_mode );
    int getMaxBitSet(uint32 word);

    static void eventHandlerCb(void *pMe, OMX_U32 nModuleId,
                      WFDMMSourceEventType nEvent,
                      OMX_ERRORTYPE nStatus, void* nData);

    void eventHandler(OMX_U32 nModuleId,
                      WFDMMSourceEventType nEvent,
                      OMX_ERRORTYPE nStatus, int64 nData);

    void updateCurrPTS(OMX_U64 nPTS){m_nCurrentPTS = nPTS;};

#ifdef WFD_VDS_ARCH
    VideoSource*  m_pVideoSource;
#else
    ScreenSource* m_pScreenSource;
#endif
    Muxer* m_pSink;
    AudioSource * m_pAudioSource;
    OMX_S32 m_nFramesCoded;
    OMX_S64 m_nBits;
    OMX_BOOL m_bMuxRunning;
    VideoEncStaticConfigType  *m_pVidEncConfig;
    VideoEncDynamicConfigType *m_pVidEncDynmicConfig;
    AudioEncConfigType        *m_pAudioConfig;

    WFD_MM_capability_t *m_pNegotiatedCap;
    wfd_mm_stream_play_cb m_wfd_mm_stream_play_cb;
    wfd_mm_stream_pause_cb m_wfd_mm_stream_pause_cb;
    WFD_AV_select_t m_av_select;
    //! Queue to hold the buffer
    wfdmmsourcequeue* m_pbufferqsource;
    wfdmmsourcequeue* m_pbufferqsink;
    //! The signal Q for the WFD MM Source working thread to wait on.
    MM_HANDLE m_signalQ;
    MM_HANDLE m_WFDMMSourcePlaySignal;
    MM_HANDLE m_WFDMMSourcePauseSignal;
    MM_HANDLE m_WFDMMSourceFillThisBufferSignal;
    MM_HANDLE m_WFDMMSourceEmptyThisBufferSignal;
    MM_HANDLE m_WFDMMSourceexitSignal;

    //! The MUX working handle.
    MM_HANDLE m_WFDMMSourceThreadHandle;
    WFD_MM_HANDLE m_wfdmmhandle;
    MuxerConfigType* m_pMuxConfig;
    OMX_ERRORTYPE m_result;
    WFDMMSourceStateType m_eState;
    static const uint32 WFD_MM_SOURCE_PLAY_EVENT;
    static const uint32 WFD_MM_SOURCE_PAUSE_EVENT;
    static const uint32 WFD_MM_SOURCE_FILL_THIS_BUFFER_EVENT;
    static const uint32 WFD_MM_SOURCE_EMPTY_THIS_BUFFER_EVENT;
    static const uint32 WFD_MM_SOURCE_THREAD_EXIT_EVENT;
    //! The MUX working thread stack size
    static const unsigned int WFD_MM_SOURCE_THREAD_STACK_SIZE = 16384;
    MM_HANDLE m_hLnkSpdTmr;
    OMX_U32  m_nFloatingBitrate;
    OMX_U32  m_nMinBitrate;
    OMX_U32  m_nMaxBitrate;
    OMX_U32  m_LinkSpeedSamples[MM_BITRATE_UPDATE_AVG_WINDOW];
    OMX_U32  m_LinkSpeedSampleCnt;
    OMX_BOOL m_bBitrateAdaptEnabled;
    OMX_BOOL m_bFrameSkipEnabled;
    OMX_U64  m_nFrameSkipLimitInterval;
    OMX_BOOL m_bFrameRateChangeEnabled;
    OMX_U64  m_nCurrentPTS;
	OMX_BOOL m_IsErrorHandling;
	OMX_U32  m_nBuffersSreenSrcToMux;
    OMX_U32  m_nBuffersAudSrcToMux;
    bool m_bReinitUtils;
    OMX_BOOL m_bHDCPStatusNotified;
};
#endif // #ifndef __WFD_MM_SOURCE_H__
