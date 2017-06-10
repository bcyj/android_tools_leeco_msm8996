/***************************************************************************
 *                             rtsp_api.cpp
 * DESCRIPTION
 *  RTSP API definition for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012,2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_api.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_api.h"
#include "MMDebugMsg.h"

/*
 * Accept command connection from user
 */
SOCKET rtspCommandApi::acceptCmdConnect()
{
    return ACCEPT(cmdListenSock, cmdListenSaddr);
}

/*
 * Setup cmd socket to process inbound commands from user
 */
void rtspCommandApi::setupCmdSocket()
{
    struct sockaddr_in saddr;
    unsigned port = cmdStartPort, i;

    memset(&saddr, 0, sizeof(saddr));

    cmdListenSaddr.sin_family = AF_INET;
    cmdListenSaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cmdListenSaddr.sin_port = htons(port);


	cmdListenSock = SOCK();
	SETSOCKOPT(cmdListenSock, 0);
	ERROR_CHECK;

	for (i = 0; i < MAX_BIND_TRIES; i++) {
        address_t tAddr;
        tAddr.sa_in = cmdListenSaddr;
		if ((bind(cmdListenSock, (sockaddr*)&tAddr.sa, (socklen_t)sizeof(cmdListenSaddr))) >= 0)
			break;
        port++;
        cmdListenSaddr.sin_port = htons(port);
	}

    /*
     * Try to bind to multiple sequential ports till we find one that's free
     */
    if (i == MAX_BIND_TRIES) {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Failed to bind command socket");
        return;
	}

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);


	LISTEN(cmdListenSock, BACKLOG);
	cmdConnectSock = SOCK();
	INETPTON("127.0.0.1", saddr);
	CONNECT(cmdConnectSock, saddr);
    ERROR_CHECK;
}

/*
 * Play command
 */
int rtspCommandApi::play(const SESSION &mySession)
{
    rtspApiMesg mesg;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::Sending play");
    mesg.cmd = apiPlay;
    mesg.session = mySession;
    return sendMessage(mesg);
}

/*
 * Pause command
 */
int rtspCommandApi::pause(const SESSION &mySession)
{
    rtspApiMesg mesg;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending pause");
    mesg.cmd = apiPause;
    mesg.session = mySession;
    return sendMessage(mesg);
}

/*
 * Teardown command
 */
int rtspCommandApi::teardown(const SESSION &mySession)
{
    rtspApiMesg mesg;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending teardown");
    mesg.cmd = apiTeardown;
    mesg.session = mySession;
    return sendMessage(mesg);
}

/*
 * Stop command: exit eventLoop(), close connections
 */
int rtspCommandApi::stop()
{
    rtspApiMesg mesg;

     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending stop");
    mesg.cmd = apiStopAllSessions;
    return sendMessage(mesg);
}

/*
 * Get command
 */
int rtspCommandApi::get(const SESSION &mySession, rtspWfd &wfd)
{
    rtspApiMesg mesg;

     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending get");
    mesg.cmd = apiGet;
    mesg.session = mySession;
    mesg.wfd = wfd;
    return sendMessage(mesg);
}

/*
 * Set command
 */
int rtspCommandApi::set(const SESSION &mySession, rtspWfd &wfd)
{
    rtspApiMesg mesg;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending set");
    mesg.cmd = apiSet;
    mesg.session = mySession;
    mesg.wfd = wfd;
    return sendMessage(mesg);
}

/*
 * Queue command for processing in the eventLoop()
 */
int rtspCommandApi::sendMessage(rtspApiMesg &mesg)
{

    SEND(cmdConnectSock, mesg);
    ERROR_CHECK_VAL;
    return 0;
}

/*
 * Close command sockets
 */
void rtspCommandApi::closeCmd()
{
    CLOSESOCKET(cmdListenSock);
    CLOSESOCKET(cmdConnectSock);
}
