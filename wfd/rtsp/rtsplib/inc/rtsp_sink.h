#ifndef _RTSP_SINK_H
#define _RTSP_SINK_H

/***************************************************************************
 *                             rtsp_sink.h
 * DESCRIPTION
 *  RTSP Sink class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2013 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_sink.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_base.h"
#include "rtsp_state.h"
#include "rtsp_client.h"

/*
 * RTSP sink class
 */
class rtspSink : public rtspBase {
public:
    rtspSink(SOCKET networkSocket, unsigned rtpPort0, unsigned rtpPort1, unsigned hdcpPort, rtspWfd wfd,
             rtspClient *client, rtsp_wfd::rtspMode mode, string mac) :
             rtspBase(networkSocket, rtpPort0, rtpPort1, wfd, mode, mac, hdcpPort), instance(client)
    {
        wfdGet.reset();
        wfdSet.reset();
        methodSupp.set(wfdCmd);
        methodSupp.set(getParameterCmd);
        methodSupp.set(setParameterCmd);
        keepAliveTimeout = 0;
    }
    rtspState getFSMState(){ return fsm.getState(); }
    void getIntersect(bitset<WFD_MAX_SIZE> );
    void request() { fsm.request(this); }
    void response() { fsm.response(this); }
    void request(rtspCmds cmd, rtspWfd &wfd) { fsm.request(this, cmd, wfd); }

    bool isToredown() { return fsm.isToredown(); }
    int sendCommand(rtspCmds);
    int sendCommandUpdate(rtspCmds, rtspWfd&);

    rtspClient *instance;

    int keepAliveTimeout;
    void releaseKeepAliveTimer(){fsm.releaseKeepAliveTimer();};

private:
    void applySettings(rtspParams *);
    rtspWfd isect;
    rtspFSM fsm;
};

#endif /*_RTSP_SINK_H*/
