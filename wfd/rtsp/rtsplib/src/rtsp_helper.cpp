/***************************************************************************
 *                             rtsp_helper.cpp
 * DESCRIPTION
 *  RTSP Helper definition for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014  Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_helper.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_helper.h"

/*
 * Create a data socket
 */
void rtspHelper::createSocket()
{
    sock = SOCK();
    ERROR_CHECK;
}

/*
 * Send a network message
 */
void rtspHelper::sendMesg(rtspSession &session, string message)
{
    SEND(session.getSocket(), message);
    ERROR_CHECK;

}

/*
 * Receive a network message
 */
string rtspHelper::recvMesg(rtspSession &session)
{
    int n;
    char recvline[MAXLEN];

    n = RECV(session.getSocket(), recvline);
    ERROR_CHECK_STR;

    if(n > 0 && n < MAXLEN) {
        recvline[n] = 0;
    }
    else{
        recvline[0] = 0;
    }
    string input(recvline);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",recvline);
    return input;
}

/*
 * Log an inbound connection
 */
void rtspHelper::recvConnection(rtspSession &session)
{
    RTSPStringStream ss;

    ss << "Received a new connection. Session: " << session.getSessionID();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
}

/*
 * Open callback
 */
void rtspHelper::open(rtspSession &session)
{
    rtspApiMesg mesg(apiOpen, session.getSessionID(), 0, 0, session.getWfd(), session.getIpAddr());
    commandApi.openEvent(mesg);
    mesg.wfd.dump();
    session.copyWfd(mesg.wfd);
}

/*
 * Intersect callback
 */
void rtspHelper::intersect(rtspSession &session)
{
    rtspApiMesg mesg(apiIntersect, session.getSessionID(), 0, 0, session.getWfd(), session.getIpAddr());
    commandApi.intersectEvent(mesg);
    mesg.wfd.dump();
    session.copyWfd(mesg.wfd);
}

/*
 * Get callback
 */
void rtspHelper::get(rtspSession &session, rtspWfd &wfd)
{
    rtspApiMesg mesg(apiGet, session.getSessionID(), 0, 0, wfd, session.getIpAddr());
    commandApi.getEvent(mesg);
}

/*
 * Set callback
 */
void rtspHelper::set(rtspSession &session, rtspWfd &wfd)
{
    rtspApiMesg mesg(apiSet, session.getSessionID(), 0, 0, wfd, session.getIpAddr());
    commandApi.setEvent(mesg);
    if (mesg.wfd.client.getRenegotiated())
    {
        wfd.client = mesg.wfd.client;
    }
}

/*
 * Close callback
 */
void rtspHelper::closeConnection(rtspSession &session)
{
    rtspPending pending;
    RTSPStringStream ss;

    ss << "Received a close connection. Session: " << session.getSessionID();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());

    rtspApiMesg mesg(apiStopSession, session.getSessionID(), 0, 0);
    removePending(session.getSessionID(), pending);
    CLOSESOCKET(session.getSocket());
    commandApi.closeEvent(mesg);
}

/*
 * Callbacks for RTSP messages:
 * Play, Pause, Setup, Teardown
 */
void rtspHelper::recvCmdApi(rtspCmds cmd, rtspSession &session, rtspError error)
{
    rtspApiMesg message;
    rtspPending pending;

    message.session = session.getSessionID();
    message.rtpPort0 = session.getRtpPort0();
    message.rtpPort1 = session.getRtpPort1();
    message.wfd = session.getWfd();
    message.error = error;
    message.ipAddr = session.getIpAddr();

    switch(cmd) {
    case setupCmd:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Received Setup");
        message.cmd = apiSetup;
        commandApi.setupEvent(message);
        if(message.wfd.server.getValid()) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Server valid");
            session.setWfd(message.wfd);
        }
    break;
    case playCmd:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Received Play");
        message.cmd = apiPlay;
        commandApi.playEvent(message);
    break;
    case pauseCmd:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Received Pause");
        removePending(session.getSessionID(), pending);
        message.cmd = apiPause;
        commandApi.pauseEvent(message);
    break;
    case teardownCmd:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Received Teardown");
        removePending(session.getSessionID(), pending);
        message.cmd = apiTeardown;
        commandApi.teardownEvent(message);
    break;
    case getParameterCmd:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Get Parameter");
        message.cmd = apiGet;
        commandApi.getEvent(message);
    break;
    default:
        RTSPStringStream ss;

        ss << "Unsupported cmd: " << (long int)cmd;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());
    break;
    }
}

/*
 * Close socket
 */
void rtspHelper::closeSocket()
{
    CLOSESOCKET(sock);
}

/*
 * Queue pending commands
 */
void rtspHelper::queuePending(rtspPending &pending)
{
    RTSPStringStream ss;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "RTSP_LIB :: Entering queuePending");

    ss << "Adding pending cmd: " << supportedCmds[pending.cmd].cmdName << " Session: " <<  pending.session->getSessionID();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());
    pending.time = GET_TICK_COUNT();
    pendingList.insert(pendingList.begin(), pending);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "RTSP_LIB ::Exiting queuePending");
}

/*
 * Remove pending commands
 */
bool rtspHelper::removePending(SESSION session, rtspPending &pending)
{
    bool retval = false;
    RTSPStringStream ss;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering removePending, list size %d",pendingList.size());

    for (list<rtspPending>::iterator it = pendingList.begin();
         it != pendingList.end(); it++) {
        if ((*it).session->getSessionID() == session) {
            ss << "Removing pending cmd: " << supportedCmds[(*it).cmd].cmdName << " Session: " << (*it).session->getSessionID();
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());
            pending = *it;
            pendingList.erase(it);
            retval = true;
            break;
        }
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting removePending");

    return retval;
}

/*
 * Return number of pending commands
 */
size_t rtspHelper::numPending(SESSION session)
{
    size_t count = 0;
    RTSPStringStream ss;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,"RTSP_LIB :: Entering numPending");

    for (list<rtspPending>::iterator it = pendingList.begin();
         it != pendingList.end(); it++) {
        if ((*it).session->getSessionID() == session) {
            ss << "Found pending cmd: " << supportedCmds[(*it).cmd].cmdName << " Session: " << (*it).session->getSessionID();
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());
            count++;
        }
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "RTSP_LIB ::Exiting numPending");

    return count;
}

/*
 * Timeout pending commands
 */
void rtspHelper::timeoutPending(list<rtspPending> &pending)
{
    list<rtspPending>::iterator toDelete = pendingList.begin();
    RTSPStringStream ss;

    //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering timeoutPending");

    for (list<rtspPending>::iterator it = pendingList.begin();
         it != pendingList.end(); it++) {
        if ((GET_TICK_COUNT() - (*it).time) > (*it).timeOut /*CMD_PENDING_TIMEOUT*/) {

            ss << "Timed out pending cmd: " << supportedCmds[(*it).cmd].cmdName;
            ss << " Session: " << (*it).session->getSessionID();
            ss << "timeout value"<<(*it).timeOut;

            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s",ss.str().c_str());
            pending.insert(pending.begin(), *it);
            toDelete++;
        }
    }
    if (toDelete != pendingList.begin()) {
        pendingList.erase(pendingList.begin(), toDelete);
    }

    //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::Exiting timeoutPending");
}
