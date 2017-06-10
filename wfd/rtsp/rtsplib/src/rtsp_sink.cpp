/***************************************************************************
 *                             rtsp_sink.cpp
 * DESCRIPTION
 *  RTSP Sink definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_sink.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_sink.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "AEEstd.h"
#include "RTSPStringStream.h"

/*
 * Entry point for sink commands
 */
int rtspSink::sendCommand(rtspCmds cmd)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending Cmd Request");

    /*
     * Check state machine state before proceeding
     */
    if (!fsm.isReady()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Attempting to send command in invalid state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    /*
     * Verify we're not in standby mode
     */
    if (fsm.isStandby() && (cmd == pauseCmd)) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Attempting to send command in standby state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    /*
     * Make sure there are no pending commands. In case of teardown scenario do
     * not fail in case of pending commands. Request it straight away!
     */
    if (instance->numPending(session.getSessionID()) && cmd != teardownCmd) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Attempting to send command while cmd pending.");
        instance->recvCmdApi(cmd, session, pendingCmdError);
        return -1;
    }

    switch(cmd) {
    case playCmd:
        fsm.play(this);
    break;
    case pauseCmd:
        fsm.pause(this);
    break;
    case teardownCmd:
        fsm.teardown(this);
    break;
    default:
        RTSPStringStream ss;

        ss << "Unhandled cmd: " << (long int)cmd;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
    break;
    ERROR_CHECK_VAL;
    }

    return 0;
}

/*
 * Entry point for sink get/set commands
 */
int rtspSink::sendCommandUpdate(rtspCmds cmd, rtspWfd &w)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Sending Cmd Request");

    if (!fsm.isReady()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Attempting to send command in invalid state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    if ((cmd == setParameterCmd) &&
        (wfd != w)) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Attempting to set parameters outside of intersection.");
        instance->recvCmdApi(cmd, session, badParamsError);
        return -1;
    }


    request(cmd, w);
    ERROR_CHECK_VAL;

    return 0;
}

/*
 * Get standby resume capability intersection
 */
void rtspSink::getIntersect(bitset<WFD_MAX_SIZE> parsedWfdBitmap)
{
    rtspWfd workingSet,tempSet;
    bitset<WFD_MAX_SIZE> standbyBitChk;
    tempSet = session.getWfd();
    standbyBitChk = tempSet.getValidWfd();

    if(parsedWfdBitmap[wfd_standby_resume_capability] && standbyBitChk[wfd_standby_resume_capability])
        isect.standbyCap.setValid(true);
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::getIntersect standbyCap "\
                                           "parsed = %lu StandbyBitChk = %lu" \
                                           "isect.standbyCap = %d",
                                           parsedWfdBitmap.to_ulong(),
                                           standbyBitChk.to_ulong(),
                                           isect.standbyCap.getValid());
    if (!parsedWfdBitmap[wfd_content_protection]) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB: CP not available");
        tempSet.contentProtection.setValid(false);
        session.copyWfd(tempSet);
    }
}

/*
 * Update state based on inbound data
 */
void rtspSink::applySettings(rtspParams *parsed)
{
    theirWfd += parsed->state.wfd;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::parsed wfd = %lu",(parsed->state.wfd.getValidWfd()).to_ulong());
    if (parsed->valid & sessionValid) {
        char *str = (char*)(parsed->state.sessStr.c_str());
        char *timeoutStr = NULL;
        if(str != NULL)
        {
          timeoutStr = strstr(str, timeout);
        }
        if(timeoutStr != NULL)
        {
          int size = (int)((timeoutStr - str)/sizeof(char));
          /* Timeout points after ';', so not adding +1
             to size for null termination*/
          char *sessionStr = (char*)MM_Malloc(size);
          if(sessionStr)
          {
            std_strlcpy(sessionStr,str,size);
            session.setSessionStr(sessionStr);
            MM_Free(sessionStr);
          }
          if(this->keepAliveTimeout == 0)
          {
            timeoutStr += std_strlen(timeout);
            this->keepAliveTimeout = atoi(timeoutStr);
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: applySettings this->keepAliveTimeout = %d",this->keepAliveTimeout);
          }
        }
        else
        {
          session.setSessionStr(parsed->state.sessStr);
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: applySettings session = %s",session.getSessionStr().c_str());
    }

    if (parsed->mesg.cmd == setParameterCmd &&
        !parsed->mesg.wfdParams[wfd_trigger_method]) {

        if (parsed->state.wfd.client.getValid() && getFSMState()!= M4 && getFSMState()!= M3) {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::applySettings client port update");
          parsed->state.wfd.client.setRenegotiated(true);
        }

        if(isect.standbyCap.getValid() && getFSMState()== M4) {
          //Set standby cap only if its in M4 state
          parsed->state.wfd.standbyCap.setValid(true);
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::applySettings parsed wfd = %d",parsed->state.wfd.standbyCap.getValid());
          }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::parsed isect wfd = %lu",(parsed->state.wfd.getValidWfd()).to_ulong());
        instance->set(session, parsed->state.wfd);
        if(parsed->state.wfd.client.getRenegotiated()) {
          theirWfd += parsed->state.wfd;
        }
        session.setWfd(theirWfd);
    }
}
