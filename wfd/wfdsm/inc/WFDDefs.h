/*==============================================================================
*  @file WFDDefs.h
*
*  @par DESCRIPTION:
*        Data structure definitions used in Native Session manager
*
*
*  Copyright (c) 2012 - 2013 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
//#include "wdsm_mm_interface.h"

#ifndef _WFDDEFS_H_
#define _WFDDEFS_H_

#define SM_CONTROL_PORT 7236

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

typedef struct
{
   char ipaddress[32];
   char macaddress[32];
   char peermac[32];
   int  SMControlPort;
   int  deviceType;
   int  decoderLatency;

} WfdDevice;

typedef enum ServiceState {
    DISABLED,
    ENABLED
} ServiceState;

typedef enum CapabilityStatus {
  DEFAULT,
  ENABLE, // for some capabilities this may mean something else
  DISABLE
}CapabilityStatus;

//Order should match with CapabilityType enum in WfdEnums.java
typedef enum CapabilityType {
	WFD_AUDIO_CODECS,
	WFD_VIDEO_FORMATS,
	WFD_3D_VIDEO_FORMATS,
	WFD_CEA_RESOLUTIONS_BITMAP,
	WFD_VESA_RESOLUTIONS_BITMAP,
	WFD_HH_RESOLUTIONS_BITMAP,
	WFD_DISPLAY_EDID,
	WFD_COUPLED_SINK,
	WFD_I2C,
	WFD_UIBC_SUPPORTED,
	WFD_STANDBY_RESUME_CAPABILITY,
	WFD_COUPLED_SINK_SUPPORTED_BY_SOURCE,
	WFD_COUPLED_SINK_SUPPORTED_BY_SINK,
	WFD_SERVICE_DISCOVERY_SUPPORTED,
	WFD_CONTENT_PROTECTION_SUPPORTED,
	WFD_TIME_SYNC_SUPPORTED,
  UIBC_Generic,
  UIBC_INPUT,
  HDCP,
  FRAME_SKIPPING,
  STANDBY_CAP,
  INPUT_VIDEO,
  VIDEO_RECOVERY,
  PREFFERED_DISPLAY,
  MAX_CAPABILITY
} CapabilityType;

typedef enum DeviceType {
    SOURCE,
    PRIMARY_SINK,
    SECONDARY_SINK,
    SOURCE_PRIMARY_SINK,
    UNKNOWN
} DeviceType;

typedef enum MMEventType {
  NO_EVENT = 0,
  HDCP_CONNECT_SUCCESS,
  HDCP_CONNECT_FAIL,
  HDCP_UNSUPPORTED_BY_PEER,
  MM_VIDEO_EVENT_FAILURE,
  MM_AUDIO_EVENT_FAILURE,
  MM_EVENT_STREAM_STARTED,
  BUFFERING_NEGOTIATION,
  BUFFERING_STATUS_UPDATE,
  BUFFERING_CONTROL_EVENT_PLAY,
  BUFFERING_CONTROL_EVENT_PAUSE,
  BUFFERING_CONTROL_EVENT_FLUSH,
  BUFFERING_CONTROL_EVENT_STATUS,
  BUFFERING_CONTROL_EVENT_DECODER_LATENCY,
  MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK,
  MM_RTP_EVENT_FAILURE,
  MM_RTP_EVENT_RTCP_RR_MESSAGE,
  MM_SESSION_FAILURE,
  MM_AUDIO_PROXY_DEVICE_OPENED,
  MM_AUDIO_PROXY_DEVICE_CLOSED,
  MM_HDCP_EVENT_FAILURE,
  MM_SESSION_AUDIO_ONLY,
  MAX_EVENT
}MMEventType;

typedef enum MMEventStatusType {
  MM_STATUS_SUCCESS,
  MM_STATUS_FAIL,
  MM_STATUS_NOTSUPPORTED,
  MM_STATUS_BADPARAM,
  MM_STATUS_MEMORYFAIL,
  MM_STATUS_RUNTIME_ERROR,
  MM_STATUS_READY,
  MM_STATUS_CONNECTED,
  MM_STATUS_INVALID
}MMEventStatusType;

typedef enum AVPlaybackModeType {
  NO_AUDIO_VIDEO,
  AUDIO_ONLY,
  VIDEO_ONLY,
  AUDIO_VIDEO
} AVPlaybackMode;

typedef enum RtpTransportType {
    TRANSPORT_UDP,
    TRANSPORT_TCP
}RtpTransportType;

typedef enum ControlCmdType {
    TCP_FLUSH,
    TCP_PLAY,
    TCP_PAUSE,
    TCP_STATUS
}ControlCmdType;

typedef void (*stringarray_callback)(const char* eName, int numObjects, char strArray[][256]);

#endif
