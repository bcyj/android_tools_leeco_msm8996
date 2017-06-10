/***************************************************************************
 *                             rtsp_helper.h
 * DESCRIPTION
 *  RTSP Helper class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_helper.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/


#ifndef _RTSP_HELPER_H
#define _RTSP_HELPER_H

#include "rtsp_session.h"
#include "rtsp_api.h"
#include "rtsp_common.h"
#include "rtsp_wfd.h"
#include "rtsp_parser.h"


/*
 * Base class that the client and server classes are derived from
 */
class rtspHelper {
public:
    rtspHelper() : sock(-1)
    {
        stop = false;
        PORT = 554;
        WINSTARTUP();
    }
    rtspHelper(rtspCallbacks *cback, string cfgFile, unsigned rtspPort, rtsp_wfd::rtspMode mode) : PORT(rtspPort), mode(mode)
    {
		commandApi.setCallback(cback);
        wfd.init(cfgFile);
        stop = false;
        WINSTARTUP();
    }
    virtual ~rtspHelper()
    {
        WINSHUTDOWN();
    }

    /* User interface for issuing commands */
	int Play(SESSION session) { return commandApi.play(session); }
    int Pause(SESSION session) { return commandApi.pause(session); }
    int Teardown(SESSION session) { return commandApi.teardown(session); }
    int Stop() { return commandApi.stop(); }
    int Get(SESSION session, rtspWfd &wfd) { return commandApi.get(session, wfd); }
    int Set(SESSION session, rtspWfd &wfd) { return commandApi.set(session, wfd); }

    /* Network generated event related functions */
    void recvCmdApi(rtspCmds, rtspSession&, rtspError);
    void recvConnection(rtspSession&);
    void closeConnection(rtspSession&);
    void open(rtspSession&);
    void intersect(rtspSession&);
    void get(rtspSession&, rtspWfd&);
    void set(rtspSession&, rtspWfd&);

    /* State machine generated events */
    void sendMesg(rtspSession&, string);
    string recvMesg(rtspSession&);

    /* Pending commands */
    void queuePending(rtspPending&);
    bool removePending(SESSION, rtspPending&);
    size_t numPending(SESSION);
    void timeoutPending(list<rtspPending> &);

    rtsp_wfd::rtspMode getOpMode() const { return mode; }

protected:
    /* Socket create/destroy */
    void createSocket();
    void closeSocket();

    /* Accept Command Connection */
    SOCKET acceptCmdConnect();

    list<SOCKET> cmdSockList;
    list<rtspPending> pendingList;
    unsigned PORT;
    static const unsigned BACKLOG = 10;
    // The following constant is the timeout value for polling
    // on the socket for RTSP commands
    static const unsigned CMD_PENDING_TIMEOUT = 1000;
    bool stop;
    map<string, string> inputFile;
    SOCKET sock;
    rtspCommandApi commandApi;
    rtspWfd wfd;
    rtsp_wfd::rtspMode mode;
};

#endif
