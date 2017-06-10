#ifndef _RTSP_CLIENT_H
#define _RTSP_CLIENT_H

/***************************************************************************
 *                             rtsp_client.h
 * DESCRIPTION
 *  RTSP Client class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_client.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_helper.h"
/*
 * Client class that serves as an interface to the sink instance
 */
class rtspClient : public rtspHelper {
public:
    ~rtspClient()
    {
        closeSocket();
    }
	rtspClient(unsigned port0, unsigned port1, unsigned hdcpPort,rtspCallbacks *callbacks,
               string cfgFile, unsigned rtspPort, rtsp_wfd::rtspMode mode, string mac) : rtspHelper(callbacks, cfgFile, rtspPort, mode), coupledMac(mac)
    {
        rtpPort0 = port0;
        rtpPort1 = port1;
        hdcpCtrlPort = hdcpPort;
    }

    void eventLoop();
    int startClient(string);

private:
    void setupClientSocket(string);
    unsigned rtpPort0;
    unsigned rtpPort1;
    unsigned hdcpCtrlPort;
    string coupledMac;
};

#endif /*_RTSP_CLIENT_H*/
