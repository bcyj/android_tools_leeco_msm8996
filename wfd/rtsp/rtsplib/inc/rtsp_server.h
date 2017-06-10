#ifndef _RTSP_SERVER_H
#define _RTSP_SERVER_H
/***************************************************************************
 *                             rtsp_server.h
 * DESCRIPTION
 *  RTSP Server class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012,2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_server.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_helper.h"


class rtspSource;

/*
 * Server class that serves as an interface to the source instances
 */
class rtspServer : public rtspHelper {
public:
    rtspServer() : uibcPort(0), newConn(NULL)
    {
        memset((void*)&saddr, (int)sizeof(sockaddr_in), 0);
    };
    ~rtspServer()
    {
        closeSocket();
    }
    rtspServer(string ip, rtspCallbacks *callbacks, string cfgFile,
               unsigned rtspPort, unsigned uibcPort, rtsp_wfd::rtspMode mode) :
               rtspHelper(callbacks, cfgFile, rtspPort, mode), ipAddr(ip),
               uibcPort(uibcPort), newConn(NULL) {}

    void eventLoop();
    int createServer();

private:
    SOCKET startServer();
    void setupServerSocket();
    SOCKET acceptConnect(sockaddr_in&);
    map<SOCKET, rtspSource *> instances;
    string ipAddr;
    sockaddr_in saddr;
    unsigned uibcPort;
    rtspSource *newConn;
};

#endif  /*_RTSP_SERVER_H*/
