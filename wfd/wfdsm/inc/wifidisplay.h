/*==============================================================================
*  @file wifidisplay.h
*
*  @par DESCRIPTION:
*        Wifi Display Native APIs
*        Interface between SM-B and SM-A JNI
*
*  @notes This file needs to be C friendly
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#ifndef _WIFIDISPLAY_H
#define _WIFIDISPLAY_H


#define ERROR_EVENT             "Error"
#define SERVICE_CHANGED_EVENT   "ServiceStateChanged"
#define SESSION_CHANGED_EVENT   "SessionStateChanged"
#define STREAM_CONTROL_EVENT    "StreamControlCompleted"
#define UIBC_CONTROL_EVENT      "UIBCControlCompleted"
#define UIBC_ROTATE_EVENT       "UIBCRotateEvent"
#define MM_UPDATE_EVENT         "MMEvent"
#define MM_VIDEO_EVENT          "VideoEvent"
#define MM_AUDIO_EVENT          "AudioEvent"
#define MM_HDCP_EVENT           "HdcpEvent"
#define MM_NETWORK_EVENT        "NetworkEvent"


#define WFD_ENABLED     "Enabled"
#define WFD_DISABLED    "Disabled"

#ifdef __cplusplus
extern "C" {
#endif

#include "UIBCDefs.h"
#include "WFDDefs.h"

int getRTSPSessionId();

int enableWfd (WfdDevice *thisDevice, stringarray_callback cb);

void setSurface(void *surface);

int disableWfd ();

void startWfdSession (WfdDevice *peerDevice);

void stopWfdSession (int sessId);

void play_rtsp (int sessId, unsigned char secureFlag );

void pause_rtsp (int sessId, unsigned char secureFlag );

void teardown_rtsp(int sessId, int isRTSP);

int standby_rtsp (int sessId);

int enableUIBC (int sessId);

int disableUIBC (int sessId);

void startUIBC ();

void stopUIBC ();

void registerUibcCallbacks(wfd_uibc_attach_cb Attach,
                           wfd_uibc_send_event_cb SendEvent,
                           wfd_uibc_hid_event_cb sendHIDEvent);

void* getStreamingSurface();

int sendUIBCEvent (WFD_uibc_event_t *event);

void setCapability (int capType, void *value);

int setResolution (int type, int value, int *resParams);

void setRtpTransport (int transportType);

void queryTCPTransportSupport(void);

void setDecoderLatency (int latency);

void tcpPlaybackControl(int cmdType, int cmdVal);

void negotiateRtpTransport(int TransportType,int BufferLenMs, int portNum);

void setBitrate(int value);
int executeRuntimeCommand (int cmd);

int  setAVPlaybackMode (AVPlaybackMode);
int  setUIBC(int);

int getCfgItems(int** cfgItems, size_t* len);

void eventError(const char*);
void eventServiceEnabled();
void eventServiceDisabled();
void eventSessionStateChanged(int, const char*, const char*);
void eventStreamControlCompleted(int, int);
void eventUIBCEnabled(int);
void eventUIBCDisabled(int);
void eventUIBCRotate(int);
void eventMMUpdate (MMEventType, MMEventStatusType, long, long, long evtData3, long evtData4);
int getResolution(int32_t*, int32_t*);
uint32_t* getCommonResolution(int*);
uint32_t* getNegotiatedResolution();
void sendIDRRequest();
#ifdef __cplusplus
}
#endif

#endif /* _WIFIDISPLAY_H */

