/* =======================================================================
                              WFDMMSourceComDef.h
DESCRIPTION

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/utils/inc/WFDMMSourceComDef.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$

========================================================================== */

#ifndef _WFD_MMSOURCE_COMDEF_H
#define _WFD_MMSOURCE_COMDEF_H
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"
#include "OMX_Video.h"

#define MAX_BUFFER_ASSUME 150

#define WFD_720P_WIDTH          1280
#define WFD_720P_HEIGHT         720


typedef enum WFDMMSourceEvent
{
    WFDMMSRC_ERROR,
    WFDMMSRC_RTP_RTCP_PORT_UPDATE,
    WFDMMSRC_RTP_SESSION_START,
    WFDMMSRC_RTCP_RR_MESSAGE,
    WFDMMSRC_AUDIO_PROXY_OPEN_DONE,
    WFDMMSRC_AUDIO_PROXY_CLOSE_DONE,
    WFDMMSRC_VIDEO_SESSION_START
}WFDMMSourceEventType;

typedef void (*eventHandlerType)(void *pHandle, OMX_U32 nStreamID,
                                 WFDMMSourceEventType nEvent, 
                                 OMX_ERRORTYPE nStatus, void* nData);



static const OMX_S32 VENC_TEST_MAX_STRING = 128;
   /**
    * @brief Resync marker types
    */
    enum ResyncMarkerType
{
      RESYNC_MARKER_NONE,      ///< No resync marker
      RESYNC_MARKER_BITS,      ///< Resync marker for MPEG4, H.264, and H.263 annex K
      RESYNC_MARKER_MB,       ///< MB resync marker for MPEG4, H.264, and H.263 annex K
      RESYNC_MARKER_GOB       ///< GOB resync marker for H.263
    };

  /**
    * @brief Encoder configuration
    */
   struct VideoEncStaticConfigType
{
     ////////////////////////////////////////
     //======== Common static config
     OMX_VIDEO_CODINGTYPE eCodec;                 ///< Config File Key: Codec
     OMX_VIDEO_CONTROLRATETYPE eControlRate;     ///< Config File Key: RC
     OMX_S32 nResyncMarkerSpacing;                 ///< Config File Key: ResyncMarkerSpacing
     ResyncMarkerType eResyncMarkerType;         ///< Config File Key: ResyncMarkerType
     OMX_BOOL bEnableIntraRefresh;                 ///< Config File Key: EnableIntraRefresh
     OMX_S32 nFrameWidth;                         ///< Config File Key: FrameWidth
     OMX_S32 nFrameHeight;                         ///< Config File Key: FrameHeight
     OMX_S32 nOutputFrameWidth;                     ///< Config File Key: OutputFrameWidth
     OMX_S32 nOutputFrameHeight;                    ///< Config File Key: OutputFrameHeight
     OMX_S32 nDVSXOffset;                         ///< Config File Key: DVSXOffset
     OMX_S32 nDVSYOffset;                         ///< Config File Key: DVSYOffset
     OMX_S32 nBitrate;                             ///< Config File Key: Bitrate
     OMX_S32 nFramerate;                         ///< Config File Key: FPS
     OMX_S32 nRotation;                          ///< Config File Key: Rotation
     OMX_S32 nInBufferCount;                     ///< Config File Key: InBufferCount
     OMX_S32 nOutBufferCount;                     ///< Config File Key: OutBufferCount
     char cInFileName[VENC_TEST_MAX_STRING];  ///< Config File Key: InFile
     char cOutFileName[VENC_TEST_MAX_STRING]; ///< Config File Key: OutFile
     OMX_S32 nFrames;                             ///< Config File Key: NumFrames
     OMX_S32 nIntraPeriod;                         ///< Config File Key: IntraPeriod
     OMX_S32 nMinQp;                             ///< Config File Key: MinQp
     OMX_S32 nMaxQp;                             ///< Config File Key: MaxQp
     OMX_S32 nQp;                                 ///< Config File Key: Qp
     OMX_BOOL bProfileMode;                      ///< Config File Key: ProfileMode
     OMX_U8 nProfile;
     OMX_U8 nLevel;

     ////////////////////////////////////////
     //======== Mpeg4 static config
     OMX_S32 nHECInterval;                         ///< Config File Key: HECInterval
     OMX_S32 nTimeIncRes;                         ///< Config File Key: TimeIncRes
     OMX_BOOL bEnableShortHeader;                 ///< Config File Key: EnableShortHeader

     ////////////////////////////////////////
     //======== H.263 static config

     OMX_BOOL bInUseBuffer;
     OMX_BOOL bOutUseBuffer;

     OMX_BOOL bEnableFrameSkip;
     OMX_U64  nFrameSkipInterval;
};

  /**
   * @brief Dynamic configuration
   */
  struct VideoEncDynamicConfigType
{
    OMX_S32 nIFrameRequestPeriod;               ///< Config File Key: IFrameRequestPeriod
    OMX_S32 nUpdatedFramerate;                  ///< Config File Key: UpdatedFPS
    OMX_S32 nUpdatedBitrate;                    ///< Config File Key: UpdatedBitrate
    OMX_S32 nUpdatedMinQp;                      ///< Config File Key: MinQp
    OMX_S32 nUpdatedMaxQp;                      ///< Config File Key: MaxQp
    OMX_S32 nUpdatedFrames;                     ///< Config File Key: UpdatedNumFrames
    OMX_S32 nUpdatedIntraPeriod;                ///< Config File Key: UpdatedIntraPeriod
  };

typedef enum WFDMMSourceState
{
  MMWFDSRC_STATE_INVALID,
  MMWFDSRC_STATE_INIT,
  MMWFDSRC_STATE_PLAY,
  MMWFDSRC_STATE_PAUSE,
  MMWFDSRC_STATE_ERROR
}WFDMMSourceStateType;

#define WFDMM_TSMUX_MODULE_ID     1001
#define WFDMM_AUDIOSRC_MODULE_ID  2002
#define WFDMM_SCREENSRC_MODULE_ID 3003
#define WFDMM_HDCP_MODULE_ID      4004
#define WFDMM_VENC_MODULE_ID      5005
#define WFDMM_VCAP_MODULE_ID      6006

#endif // #ifndef _WFD_MMSOURCE_COMDEF_H
