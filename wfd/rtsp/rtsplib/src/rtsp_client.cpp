/***************************************************************************
 *                             rtsp_client.cpp
 * DESCRIPTION
 *  RTSP Client definition for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 QUALCOMM Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_client.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_client.h"
#include "rtsp_sink.h"
#include "MMMemory.h"
#include "MMTimer.h"
#include <unistd.h>

#ifndef WFD_CFG_FILE
#define WFD_CFG_FILE "/system/etc/wfdconfigsink.xml"
#endif

extern "C" int  PargeCfgForIntValueForKey(char *filename, char *pKey, int *Val);

#define MAX_RETRY_COUNT 40
#define RETRY_CONNECT_SLEEP_TIME 200

#define ERROR_CHECK_CLIENT(sink)                                            \
  if(globalError)                                                           \
  {                                                                         \
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: exiting eventloop");\
    sink.releaseKeepAliveTimer();                                           \
    for (list<SOCKET>::iterator it = cmdSockList.begin();                   \
       it != cmdSockList.end(); it++)                                       \
    {                                                                       \
      CLOSESOCKET(*it);                                                     \
    }                                                                       \
    cmdSockList.clear();                                                    \
    commandApi.closeCmd();                                                  \
    commandApi.finishEvent();                                               \
    break;                                                                  \
  }
/*
 * Start client instance
 * Setup data/cmd sockets for client
 */
int rtspClient::startClient(string ipaddr)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Starting client");

    createSocket();
    setupClientSocket(ipaddr);
    if (globalError) {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Connect failed closing socket = %d",sock);
          CLOSESOCKET(sock);
          return globalError;
      }
    commandApi.setupCmdSocket();
    ERROR_CHECK_VAL;

    return 0;
}

/*
 * Setup data socket
 */
void rtspClient::setupClientSocket(string ipaddr)
{
    sockaddr_in saddr;

    memset((void*)&saddr, (int)sizeof(sockaddr_in), 0);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(rtspHelper::PORT);
    CLEAR_ERROR;
    INETPTON(ipaddr, saddr);
    ERROR_CHECK;
    CONNECT(sock, saddr);
    if (!globalError)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "rtsp_client Connected");
        }
    else
        {
        int uRetryCount = MAX_RETRY_COUNT; // 40 * 200 Milli Secs = 8000 Millisecs / 8 secs
        int nRetVal;
        int retryCount=0;
        nRetVal = PargeCfgForIntValueForKey((char *)WFD_CFG_FILE, (char *)"RetryCount",
                                      (int*)(&retryCount));
        if(nRetVal==0 && (retryCount>=MAX_RETRY_COUNT))
        {
            uRetryCount = retryCount;
        }
        for (;uRetryCount;uRetryCount--)
            {
                CONNECT(sock, saddr);
                if (!globalError)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "rtsp_client Connected");
                    globalError = 0;
                    break;
                }
                MM_Timer_Sleep(RETRY_CONNECT_SLEEP_TIME);
            }
    }
}

/*
 * Event loop that processes commands and network events
 */
void rtspClient::eventLoop()
{
    fd_set active_fd_set, read_fd_set;
    static char recvline[MAXLEN];
    SOCKET cmdSock = commandApi.getCmdSock();
    struct timeval selectPoll = { (CMD_PENDING_TIMEOUT/1000), 0 };
    list<rtspPending> pending;
    rtspSink sink(sock, rtpPort0, rtpPort1, hdcpCtrlPort,wfd, this, mode, coupledMac);

    open(sink.session);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering event handler");

    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);
    FD_SET(cmdSock, &active_fd_set);

    for (;;) {
        SOCKET status, i;
        selectPoll.tv_sec = CMD_PENDING_TIMEOUT/1000;
        selectPoll.tv_usec = 0;
        read_fd_set = active_fd_set;

        /*
         * Sleep on asynchronous I/O
         */
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &selectPoll) < 0) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Select failed");
            return;
        }

        timeoutPending(pending);

        for (list<rtspPending>::iterator it = pending.begin();
             it != pending.end(); it++) {
            recvCmdApi((*it).cmd, *((*it).session), timeoutError);
            stop=true;
            break;
        }

        /*
         * Event driver entry point into the RTSP state machine
         */
        if (FD_ISSET(sock, &read_fd_set)) {
            sink.response();
            if (sink.isToredown()) {
                closeConnection(sink.session);
                stop = true;
                FD_CLR(sock,&active_fd_set);
                continue;
            }
            ERROR_CHECK_CLIENT(sink);
        }

        if (FD_ISSET(cmdSock, &read_fd_set)) {

            status = commandApi.acceptCmdConnect();
            ERROR_CHECK_CLIENT(sink);
            FD_SET(status, &active_fd_set);
            cmdSockList.insert(cmdSockList.begin(), status);
        }
        /*
         * Process user-initiated commands
         */
        for (list<SOCKET>::iterator it = cmdSockList.begin();
             it != cmdSockList.end(); it++) {
            i = *it;
            size_t rLen = 0, rCount = 0;
            if (FD_ISSET(i, &read_fd_set)) {
                rLen = rCount = RECV(i, recvline);
                ERROR_CHECK_CLIENT(sink);
/*
                if (!(rCount % sizeof(rtspApiMesg)))
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Command buffer overflow, dropping command");
*/
                while (rLen >= sizeof(rtspApiMesg)) {
                    rtspApiMesg *message = (rtspApiMesg *)(recvline + (rCount - rLen));

                    switch(message->cmd) {
                    case apiPlay:
                        sink.sendCommand(playCmd);
                    break;
                    case apiPause:
                        sink.sendCommand(pauseCmd);
                    break;
                    case apiTeardown:
                        sink.sendCommand(teardownCmd);
                    break;
                    case apiStopAllSessions:
                        stop = true;
                    break;
                    case apiGet:
                        sink.sendCommandUpdate(getParameterCmd, message->wfd);
                    break;
                    case apiSet:
                        sink.sendCommandUpdate(setParameterCmd, message->wfd);
                    break;
                    default:
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Unsupported cmd");
                    break;
                    }
                    rLen -= sizeof(rtspApiMesg);
                }
            }
        }
        /*
         * Exit eventLoop(), close all sockets
         */
        if (stop == true) {
            /* Making sure that timer is released */
            sink.releaseKeepAliveTimer();
            for (list<SOCKET>::iterator it = cmdSockList.begin();
                 it != cmdSockList.end(); it++) {
                CLOSESOCKET(*it);
            }
            cmdSockList.clear();
            commandApi.closeCmd();
            commandApi.finishEvent();
            break;
        }
    }
}
