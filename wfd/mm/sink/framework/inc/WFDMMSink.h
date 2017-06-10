#ifndef __WFD_MMSINK_H__
#define __WFD_MMSINK_H__
/*==============================================================================
*  WFDSink.h
*
*  DESCRIPTION:
*  Sink Framework interface
*
*
*  Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include <stdio.h>
#include "MMCriticalSection.h"
#include "wdsm_mm_interface.h"


#include "media/mediaplayer.h"
#include "binder/ProcessState.h"
#ifdef WFD_ICS
#include "surfaceflinger/Surface.h"
#else
#include <Surface.h>
#endif
#include "OMX_Core.h"
#include "WFDMMSinkCommon.h"
using namespace android;
/*------------------------------------------------------------------------------
** Global Constant Data Declarations
**------------------------------------------------------------------------------
***/

/*------------------------------------------------------------------------------
** Global Data Declarations
**------------------------------------------------------------------------------
***/

/*==============================================================================
**                          Macro Definitions
**==============================================================================
***/


/*==============================================================================
**                        Class Declarations
**==============================================================================
***/
class WFDMMThreads;
class RTPStreamPort;
class WFDMMSinkMediaSource;
class WFDMMSinkRenderer;
class WFDMMSinkVideoDecode;
class WFDMMSinkStatistics;


//! MM streaming state
typedef enum WFDMMSinkState
{
    WFD_SINK_MM_STATE_ZERO,
    WFD_SINK_MM_STATE_DEINIT,
    WFD_SINK_MM_STATE_INIT,
    WFD_SINK_MM_STATE_PLAY,
    WFD_SINK_MM_STATE_PAUSE
}WFDSinkMMStateType;

//! HDCP connection Status
typedef enum WFDDrmSinkState
{
    WFD_SINK_DRM_STATE_ZERO,
    WFD_SINK_DRM_STATE_INIT,
    WFD_SINK_DRM_STATE_ACQUIRING,
    WFD_SINK_DRM_STATE_ACQUIRED,
    WFD_SINK_DRM_STATE_RIGHTS_LOST,
}WFDSinkDrmStateType;

class HDCPManager;
class WFDMMSinkAudioDecode;

class WFDMMSink
{
public:

    static WFDMMSink* CreateInstance();
    static void       DeleteInstance();

    WFD_status_t  play
    (
        WFD_AV_select_t av_select,
        wfd_mm_stream_play_cb pCallback
    );

    WFD_status_t  pause
    (
        WFD_AV_select_t av_select,
        wfd_mm_stream_pause_cb pCallback
    );

    WFD_status_t  setupHDCPSession
    (
        char *pIpAddr,
        unsigned short port
    );

    WFD_status_t  setupMMSession
    (
        WFD_device_t eWFDDeviceType,
        WFD_MM_capability_t *pNegotiatedCap,
        WFD_MM_callbacks_t *pCallback
    );

    WFD_status_t  teardownHDCPSession
    (
    );

    WFD_status_t  teardownMMSession
    (
    );
    void processMPresponse
    (
          int32 status,
          int32 value
    );
    WFD_status_t  getHDCPStatus
    (
        char *pIpAddr,
        unsigned short port
    );
    WFD_status_t WFDMMSinkUpdateSession
    (
        WFD_MM_capability_t *WFD_negotiated_capability,
        wfd_mm_update_session_cb pUpdateSessionCb
    );

    WFD_status_t localMediaPlayerKill
    (
        bool requested
    );

    WFD_status_t WFDMMSinkAVControl
    (
        WFD_MM_AV_Stream_Control_t control,
        int64 controlValue
    );

private:
    bool createDataSource();

    bool createMediaSource();

    bool createAudioDecoder();

    bool createVideoDecoder();

    bool createRenderer();

    bool createSinkStatistics();

    bool destroyMediaSource();

    bool destroyDataSource();

    bool destroyAudioDecoder();

    bool destroyVideoDecoder();

    bool destroyRenderer();

    bool destroyMMComponents();

    bool destroySinkStatistics();

    static bool decryptCb(void*, int, int, int, int, char*, int);

    bool decrypt(int, int, int, char*, int , int nStreamId = SINK_VIDEO_TRACK_ID);

    static void avInfoCb(void* handle, avInfoType *pInfo);

    void setAvInfo(avInfoType *pInfo);

    WFDMMSink();
    ~WFDMMSink();
    HDCPManager             *m_pHDCPManager;
    MM_HANDLE                m_hCritSect;
    MM_HANDLE                m_hCritSectEBD;
    MM_HANDLE                m_hCritSectFBD;
    sp<Surface>              m_pVideoSurface;
    sp<ANativeWindow>        m_pWindow;
    WFDMMThreads            *m_pCmdThread;
    RTPStreamPort           *m_pRTPStreamPort;
    WFDMMSinkMediaSource    *m_pMediaSrc;
    WFDMMSinkRenderer       *m_pRenderer;
    WFDMMSinkVideoDecode    *m_pVideoDecode;
    WFDMMSinkAudioDecode    *m_pAudioDecode;
    WFDMMSinkStatistics     *m_pSinkStatistics;
    int                      m_nUniqueId;
    WFDSinkMMStateType       m_eMMState;
    WFDSinkDrmStateType      m_eDrmState;
    WFD_status_t             m_nMMStatus;
    WFD_status_t             m_nDrmStatus;
    WFD_MM_capability_t     *m_pNegotiatedCap;
    WFD_MM_callbacks_t       sCallBacks;
    static WFDMMSink        *m_pMe;
    static unsigned          m_nRefCnt;
    unsigned long            m_LastSystemTime;
    uint32                   m_nAudioSampleRate;
    uint32                   m_nAudioChannels;
    uint32                   m_nTimeLastIdrReq;
    uint32                   m_nFrameCntDownStartTime;
    MM_HANDLE                m_hIDRDelayTimer;
    MM_HANDLE                m_hCritSectEvtHdlr;
    bool                     m_bPendingIDRReq;
    bool                     m_bIDRTimerPending;
    uint64                   m_nFlushTimestamp;
    uint64                   m_nDecoderLatency;
    uint64                   m_nActualBaseTime;
    bool                     m_bMMTearingDown;
    bool                     m_bStreamPaused;
    bool                     m_bFlushAll;
    bool                     m_bUpdatedSurface;
    bool                     m_bNotifyFirstFrameInfo;

    void getFrameResolutionRefreshRate(uint32 cea_mode,
                                           uint32 vesa_mode,
                                           uint32 hh_mode,
                                           uint32 *nFrameWidth,
                                           uint32 *nFrameHeight,
                                           uint32 *nFramerate
                                           );
    bool validateStateChange
    (
        WFDSinkMMStateType eState
    );

    static void EBD(void* handle,
                    int moduleIdx,
                    int trackID,
                    OMX_BUFFERHEADERTYPE *pBuffer);

    void processEBD(int moduleIdx,
                    int trackID,
                    OMX_BUFFERHEADERTYPE *pBuffer);

    static void FBD(void* handle,
                    int moduleIdx,
                    int trackID,
                    OMX_BUFFERHEADERTYPE *pBuffer);

    void processFBD(int moduleIdx,
                    int trackID,
                    OMX_BUFFERHEADERTYPE *pBuffer);


    static void eventHandlerCb(void *pThis,
                        OMX_U32 nModuleId,
                        WFDMMSinkEvent nEvent,
                        OMX_ERRORTYPE nStatus, int nData);

    void eventHandler(  OMX_U32 nModuleId,
                        WFDMMSinkEvent nEvent,
                        OMX_ERRORTYPE nStatus, int nData);

    static void CmdThreadEntry(void* pHandle,
                               unsigned int nCmd);

    void CmdThread(int nCmd);

    bool ExecuteCommandSync(unsigned int command);

    void getAC3AudioParams(uint32 supported_audio_mode );

    void getAACAudioParams(uint32 supported_audio_mode );

};
#endif/*__WFD_MMSINK_H__ */
