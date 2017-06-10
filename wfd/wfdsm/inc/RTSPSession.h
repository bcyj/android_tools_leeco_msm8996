/*==============================================================================
*        @file RTSPSession.h
*
*  @par DESCRIPTION:
*        RTSPSession class.
*
*
*  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _RTSPSESSION_H
#define _RTSPSESSION_H

#include "rtsp_server.h"
#include "rtsp_client.h"
#include "MMTimer.h"
#include <pthread.h>


/* Forward Declarations */
class WFDSession;
class RTSPSession;
class Device;



typedef enum RTSPState {
    STOPPED,
    CAP_NEGOTIATING,
    CAP_NEGOTIATED,
    SESS_ESTABLISHING,
    SESS_ESTABLISHED,
    STANDBY,
    STARTING,
    TEARING_DOWN
} RTSPState;

typedef enum StreamCtrlType {
    PLAY,
    PLAY_DONE,
    PAUSE,
    PAUSE_DONE,
    TEARDOWN
} StreamCtrlType;



class cback : public rtspCallbacks {
private:
    RTSPSession* pRtspSession;
    MM_HANDLE    m_hTimer;
public:
    cback(RTSPSession*);
    void setupCallback(rtspApiMesg &mesg);
    void playCallback(rtspApiMesg &mesg);
    void pauseCallback(rtspApiMesg &mesg);
    void teardownCallback(rtspApiMesg &mesg);
    void closeCallback(rtspApiMesg &mesg);
    void openCallback(rtspApiMesg &mesg);
    void intersectCallback(rtspApiMesg &mesg);
    void getCallback(rtspApiMesg &mesg);
    void setCallback(rtspApiMesg &mesg);
    void finishCallback();
    static void keepAliveTimerCallback(void *);
};

typedef struct
{
    int rtpPort;
    int rtcpPort;
}localTransportInfo;

/**----------------------------------------------------------------------------
   RTSPSession class
-------------------------------------------------------------------------------
*/

class RTSPSession
{
private:
    rtspCallbacks *events;
    rtspServer *server;
    rtspClient *client;
    pthread_t session_thread;
    WFDSession* pWFDSession;

    void play();
    void pause();
    void teardown();

    bool enableUIBC(bool);

    static void *rtspServerLoopFunc(void *s);
    static void *rtspClientLoopFunc(void *s);

public:
    RTSPState                rtspState;
    int                      rtspSessionId;
    Device*                  pPeerDevice;
    localTransportInfo       m_sLocalTransportInfo;
    static AVPlaybackMode           m_eplayMode;
    bool                     m_bUIBCSupported;
    bool                     m_bTCPSupportedAtSink;
    bool                     m_bTCPSupportQueried;
    bool                     m_bTCPSupportStatusRequested;

    RTSPSession(WFDSession*, Device*);
    ~RTSPSession();

    bool startServer(string, int, int);
    bool startClient(string, int, int, int,int);
    void stop();
    void rtspStateTransition(RTSPState, bool notify = true);
    //This function is to query the currently negotiated resolution for the WFD session
    static bool getNegotiatedResolution(int32_t* , int32_t*);
    MMEventStatusType updateHdcpSupportedInConnectedDevice(void *pCfgCababilities);

    static void streamControl(int, StreamCtrlType);
    static bool uibcControl(int, bool);
    static void Cleanup();
    static bool setUIBC(int);
    static bool standby(int);
    static bool setResolution(int ,int, int* resParams = NULL);
    static void setTransport(int);
    static void queryTCPTransportSupport();
    static void sendBufferingControlRequest(int cmdType, int cmdVal);
    static void sendTransportChangeRequest(int TransportType, int BufferLenMs, int portNum);
    static void setDecoderLatency(int latency);
    static void sendIDRRequest();
    static void UpdateLocalTransportInfo(localTransportInfo *);
    static void sendAVFormatChangeRequest();
    static void sendWFDKeepAliveMsg();
    static bool setAVPlaybackMode (AVPlaybackMode);
    static void setSinkSurface (void* surface);
    static void getNegotiatedResolution();
    static uint32_t* getCommonResloution(int*);
    static uint32_t* getNegotiatedResloution();
    static int  DISPhdcpCallBack(int type, void *param);
    static int getCfgItems(int** configItems, size_t* len);

};




#endif /*_RTSPSESSION_H*/
