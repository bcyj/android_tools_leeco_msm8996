#ifndef _RTSP_API_H
#define _RTSP_API_H

/***************************************************************************
 *                             rtsp_api.h
 * DESCRIPTION
 *  RTSP command API for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_api.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "rtsp_wfd.h"

#define MAX_BIND_TRIES    10

/*
 * Error messages
 */
enum rtspError {
    noError,
    noErrorPreSendCmdNotify,
    badStateError,
    timeoutError,
    remoteError,
    noWfdSuppError,
    badParamsError,
    pendingCmdError,
    alreadyError
};

/*
 * Commands
 */
enum cmdApi {
    apiInvalid,
    apiSetup,
    apiPlay,
    apiPause,
    apiTeardown,
    apiStopSession,
    apiStopAllSessions,
    apiOpen,
    apiIntersect,
    apiGet,
    apiSet
};

/*
 * Message class for user/RTSP communication
 */
class rtspApiMesg {
public:
    rtspApiMesg() : cmd(apiInvalid), session(0), rtpPort0(0), rtpPort1(0), error(noError) { wfd.reset(); }
    rtspApiMesg(cmdApi c, SESSION s, unsigned p0, unsigned p1, rtspWfd w, string ip) :
                cmd(c), session(s), rtpPort0(p0), rtpPort1(p1), wfd(w), error(noError), ipAddr(ip) {}
    rtspApiMesg(cmdApi c, SESSION s, unsigned p0, unsigned p1) :
                cmd(c), session(s), rtpPort0(p0), rtpPort1(p1), error(noError) { wfd.reset(); }

    cmdApi cmd;
    SESSION session;
    unsigned rtpPort0;
    unsigned rtpPort1;
    rtspWfd wfd;
    rtspError error;
    string ipAddr;
};

/*
 * Callbacks from RTSP to user. User must derive a class from this
 * abstract class and pass it into the rtspServer()/rtspClient()
 * constructor
 */
class rtspCallbacks
{
public:
    rtspCallbacks() {}
    virtual ~rtspCallbacks() {}

    virtual void setupCallback(rtspApiMesg &mesg) = 0;
    virtual void playCallback(rtspApiMesg &mesg) = 0;
    virtual void pauseCallback(rtspApiMesg &mesg) = 0;
    virtual void teardownCallback(rtspApiMesg &mesg) = 0;
    virtual void openCallback(rtspApiMesg &mesg) = 0;
    virtual void closeCallback(rtspApiMesg &mesg) = 0;
    virtual void intersectCallback(rtspApiMesg &mesg) = 0;
    virtual void getCallback(rtspApiMesg &mesg) = 0;
    virtual void setCallback(rtspApiMesg &mesg) = 0;
    virtual void finishCallback() = 0;
};

/*
 * Main API class
 */
class rtspCommandApi {
public:
    int play(const SESSION&);
    int pause(const SESSION&);
    int teardown(const SESSION&);
    int get(const SESSION&, rtspWfd&);
    int set(const SESSION&, rtspWfd&);
    int stop();
    void playEvent(rtspApiMesg &mesg) { callbacks->playCallback(mesg); }
    void pauseEvent(rtspApiMesg &mesg) { callbacks->pauseCallback(mesg); }
    void setupEvent(rtspApiMesg &mesg) { callbacks->setupCallback(mesg); }
    void teardownEvent(rtspApiMesg &mesg) { callbacks->teardownCallback(mesg); }
    void closeEvent(rtspApiMesg &mesg) { callbacks->closeCallback(mesg); }
    void openEvent(rtspApiMesg &mesg) { callbacks->openCallback(mesg); }
    void intersectEvent(rtspApiMesg &mesg) { callbacks->intersectCallback(mesg); }
    void getEvent(rtspApiMesg &mesg) { callbacks->getCallback(mesg); }
    void setEvent(rtspApiMesg &mesg) { callbacks->setCallback(mesg); }
    void finishEvent() { callbacks->finishCallback(); }
    SOCKET getCmdSock() { return cmdListenSock; }
    void setCallback(rtspCallbacks *c) { callbacks = c; }
    int sendMessage(rtspApiMesg&);
    void setupCmdSocket();
    void closeCmd();
    SOCKET acceptCmdConnect();
private:
    static const unsigned BACKLOG = 10;
    SOCKET cmdListenSock;
    SOCKET cmdConnectSock;
    struct sockaddr_in cmdListenSaddr;
    rtspCallbacks *callbacks;
    static const unsigned cmdStartPort = 9000;
};

#endif
