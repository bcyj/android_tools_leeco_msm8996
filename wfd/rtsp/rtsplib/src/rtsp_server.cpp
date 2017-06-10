/***************************************************************************
 *                             rtsp_server.cpp
 * DESCRIPTION
 * RTSP Server definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_server.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_server.h"
#include "rtsp_source.h"
#define ERROR_CHECK_SERVER(x) ({\
    if (globalError) {\
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, x);\
        stop = true;\
        break;\
    }\
})

/*
 * Create a server instance
 * Setup data/cmd sockets for server
 */
int rtspServer::createServer()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Creating server");
    CLEAR_ERROR;
    createSocket();
    setupServerSocket();
    commandApi.setupCmdSocket();
    ERROR_CHECK_VAL;

    return 0;
}

/*
 * Wait for a connection
 */
SOCKET rtspServer::startServer()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Starting server");

    return acceptConnect(saddr);
}

/*
 * Setup data socket
 */
void rtspServer::setupServerSocket()
{
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(rtspHelper::PORT);

    SETSOCKOPT(sock, 1);
    BIND(sock, saddr);
    LISTEN(sock, BACKLOG);
    ERROR_CHECK;
}

/*
 * Accept a connection
 */
SOCKET rtspServer::acceptConnect(sockaddr_in &addr)
{
	return ACCEPT(sock, addr);
}

/*
 * Event loop that processes commands and network events
 */
void rtspServer::eventLoop()
{
    fd_set active_fd_set, read_fd_set, cmd_fd_set, sink_fd_set;
    static char recvline[MAXLEN];
    list<SOCKET> deleteList;
    int maxfd = -1;
    SOCKET cmdSock = commandApi.getCmdSock();
    list<rtspPending> pending;
    struct timeval selectPoll;

    FD_ZERO(&active_fd_set);
    FD_ZERO(&cmd_fd_set);
    FD_ZERO(&sink_fd_set);
    FD_ZERO(&read_fd_set);

    FD_SET(sock, &active_fd_set);
    FD_SET(cmdSock, &active_fd_set);

    if (sock <= cmdSock)
      maxfd = cmdSock;
    else
      maxfd = sock;

    for (;;) {
        SOCKET i, status;
        read_fd_set = active_fd_set;
        selectPoll.tv_sec = CMD_PENDING_TIMEOUT/1000;
        selectPoll.tv_usec = 0;
        /*
         * Sleep on asynchronous I/O
         */
        int ret = select (maxfd + 1, &read_fd_set, NULL, NULL,&selectPoll);
        if (ret  < 0) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Select failed");
            break;
        }

        timeoutPending(pending);

        for (list<rtspPending>::iterator it = pending.begin();
             it != pending.end(); it++) {
            deleteList.insert(deleteList.begin(), (*it).session->getSocket());
            recvCmdApi((*it).cmd, *((*it).session), timeoutError);
        }
        pending.clear();

        /*
         * Event driver entry point into the RTSP state machine
         */
        if (FD_ISSET(sock, &read_fd_set) && !newConn) {
            sockaddr_in caddr;
            status = acceptConnect(caddr);
            ERROR_CHECK_SERVER("RTSP_LIB :: acceptConnect failed");

            newConn = MM_New_Args(rtspSource, (status, ipAddr, wfd, this, uibcPort, mode, caddr));
            if(newConn == NULL)
            {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: newconn memory allocation failed");
               break;
            }
            recvConnection(newConn->session);
            open(newConn->session);
            //Resetting all unsupported capabilities
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Resetting all unsupported capabilities");
            rtspWfd tempWfd = newConn->session.getWfd();
            if (!tempWfd.contentProtection.getValid())
            {
              newConn->resetCapabilities(wfd_content_protection);
            }
            if (!tempWfd.standbyCap.getValid())
            {
              newConn->resetCapabilities(wfd_standby_resume_capability);
            }
            if (!tempWfd.uibcCap.getValid())
            {
              newConn->resetCapabilities(wfd_uibc_capability);
            }
            if (!tempWfd.edid.getValid())
            {
              newConn->resetCapabilities(wfd_display_edid);
            }
            if (!tempWfd.coupledSink.getValid())
            {
              newConn->resetCapabilities(wfd_coupled_sink);
            }

            newConn->request();
             if (IS_ERROR) {
                closeConnection(newConn->session);
                FD_CLR(newConn->session.getSocket(),&active_fd_set);
                FD_CLR(newConn->session.getSocket(),&sink_fd_set);
                RTSP_DELETEIF(newConn);
                CLEAR_ERROR;
            }
             else {
                  instances[status] = newConn;
                  FD_SET(status, &active_fd_set);
                  if (status > maxfd)
                  maxfd = status;
                  FD_SET(status, &sink_fd_set);
             }
        }

        for (map<SOCKET, rtspSource *>::iterator it = instances.begin();
             it != instances.end(); it++) {
            i = (*it).first;
            if (FD_ISSET(i, &read_fd_set)) {
                if (FD_ISSET(i, &sink_fd_set)) {
                    rtspSource *serv = instances[i];

                    serv->response();
                    if (IS_ERROR) {
                        deleteList.insert(deleteList.begin(), i);
                        CLEAR_ERROR;
                    }
                }
            }
        }

        for (list<SOCKET>::iterator it = deleteList.begin();
             it != deleteList.end(); it++) {

            if (instances.find(*it) != instances.end()) {
                FD_CLR(instances[*it]->session.getSocket(), &active_fd_set);
                FD_CLR(instances[*it]->session.getSocket(), &sink_fd_set);

                closeConnection(instances[*it]->session);
                delete instances[*it];
                instances.erase(*it);
            }
        }
        deleteList.clear();

        if (FD_ISSET(cmdSock, &read_fd_set)) {

            status = commandApi.acceptCmdConnect();
            ERROR_CHECK_SERVER ("RTSP_LIB :: acceptCmdConnect failed");
            FD_SET(status, &active_fd_set);
            if (status > maxfd)
               maxfd = status;
            FD_SET(status, &cmd_fd_set);

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
                if (FD_ISSET(i, &cmd_fd_set)) {

                    rLen = rCount = RECV(i, recvline);
                    ERROR_CHECK_SERVER("RTSP_LIB :: RECV failure while processing user-initiated commands");
/*
                    if (!(rCount % sizeof(rtspApiMesg)))
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Command buffer overflow, dropping command");
*/
                    while (rLen >= sizeof(rtspApiMesg)) {
                        rtspApiMesg *message = (rtspApiMesg *)(recvline + (rCount - rLen));
                        rtspSource *serv;

                        switch(message->cmd) {
                        case apiPlay:
                            if (instances.find(message->session) != instances.end()) {
                                serv = instances[message->session];
                                serv->sendCommand(playCmd);
                            }
                        break;
                        case apiPause:
                            if (instances.find(message->session) != instances.end()) {
                                serv = instances[message->session];
                                serv->sendCommand(pauseCmd);
                            }
                        break;
                        case apiTeardown:
                            if (instances.find(message->session) != instances.end()) {
                                serv = instances[message->session];
                                serv->sendCommand(teardownCmd);
                            }
                        break;
                        case apiStopAllSessions:
                            stop = true;
                        break;
                        case apiGet:
                            if (instances.find(message->session) != instances.end()) {
                            serv = instances[message->session];
                            serv->sendCommandUpdate(getParameterCmd, message->wfd);
                            }
                        break;
                        case apiSet:
                            if (instances.find(message->session) != instances.end()) {
                            serv = instances[message->session];
                            serv->sendCommandUpdate(setParameterCmd, message->wfd);
                            }
                        break;
                        default:
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Unsupported cmd");
                        break;
                        }
                        rLen -= sizeof(rtspApiMesg);
                    }
                }
            }
        }
        if (stop == true) {
           break;
        }
    }
    for (map<SOCKET, rtspSource *>::iterator it = instances.begin();
         it != instances.end(); it++) {
            delete (*it).second;
    }
    instances.clear();
    for (list<SOCKET>::iterator it = cmdSockList.begin();
         it != cmdSockList.end(); it++) {
        CLOSESOCKET(*it);
    }
    cmdSockList.clear();
    commandApi.closeCmd();
    commandApi.finishEvent();
    return;
}
