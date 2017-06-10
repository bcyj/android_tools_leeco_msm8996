/***************************************************************************
 *                             rtsp_state.cpp
 * DESCRIPTION
 *  RTSP State definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 QUALCOMM Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_state.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_state.h"
#include "rtsp_source.h"
#include "rtsp_sink.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMTimer.h"



rtspM1 rtspBaseState::m1;
rtspM2 rtspBaseState::m2;
rtspM3 rtspBaseState::m3;
rtspM4 rtspBaseState::m4;
rtspM5 rtspBaseState::m5;
rtspM6 rtspBaseState::m6;
rtspM7 rtspBaseState::m7;
rtspM8 rtspBaseState::m8;
rtspM9 rtspBaseState::m9;
rtspM10 rtspBaseState::m10;

#define WFD_SNK_RTSP_KEEPALIVE_CHECK_INTERVAL  70000
#define WFD_SNK_RTSP_MIN_KEEPALIVE_TIMER_VALUE 10000
#define WFD_SNK_RTSP_KEEPALIVE_BUFFER_TIME     20000
#define CONVERT_SEC_TO_MSEC(x) x*1000


bool m_brecvKeepAlive = false;
int m_hkeepAliveTimerValue = 0;
MM_HANDLE m_hTimer = NULL;
/*
 * Free parsed data pointers
 */
void paramsFree(rtspParams *tofree)
{
    if (tofree->next)
        paramsFree(tofree->next);

    RTSP_DELETEIF(tofree);
}

/*
 * Translate command to request state for sink
 */
void rtspBaseState::cmdToState(rtspFSM *s, int cmd) {
    switch(cmd) {
    case setupCmd:
        s->setState(&m6);
    break;
    case playCmd:
        s->setState(&m7);
    break;
    case teardownCmd:
        s->setState(&m8);
    break;
    case pauseCmd:
        s->setState(&m9);
    break;
    }
}

/*
 * Determine if the parsed message is an ack
 */
bool isAck(rtspParams *parsed)
{
    if (parsed->valid & statusValid) {
        return true;
    } else {
        return false;
    }
}

/*
 * This is invoked to check if keep alive
 * message is recevied from source to sink
 * If received it will reset variable
 * else it will initiate a tear down
 */
void rtspBaseState::isKeepAlive(void* pMe)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: isKeepAlive : Entered");
  if(!m_brecvKeepAlive)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: isKeepAlive : timeouthappened");
    rtspSink *b = (rtspSink*)pMe;
    rtspPending pending(getParameterCmd,&b->session,0,0);
    pending.setTimeout(0);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: isKeepAlive : enqueuing to pending list");
    if(b && b->instance)
    {
      b->instance->queuePending(pending);
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: isKeepAlive : b && b->instance Null");
    }
  }
  else
  {
    m_brecvKeepAlive = false;
  }
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: isKeepAlive : Exiting");
}

void rtspBaseState::releaseKeepAliveTimer()
{
  if(m_hTimer)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Releasing keepalive timer");
    MM_Timer_Release(m_hTimer);
    m_hTimer = NULL;
  }
}
/*
 * Detect any errors in an inbound message
 */
template <class Type>
bool rtspBaseState::isError(rtspFSM *s, Type *b, rtspParams *parsed, rtspCmds cmd)
{
    rtspPending pending;

    /*
     * No ack found
     */
    if (!isAck(parsed)) {
        return false;
    }

    /*
     * Pending count is greater than 1. This shouldn't happen
     */
    if (b->instance->numPending(b->session.getSessionID()) > 1) {
        assert(0);
    }

    /*
     * Remove pending command from pend queue
     */
    if (b->instance->removePending(b->session.getSessionID(), pending)) {
        if (pending.seq != parsed->state.rxCseq) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Warning: Mismatching sequence number");
            //assert(0);
        }
        if ((cmd != invalidCmd) &&
            (pending.cmd != cmd)) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Warning: Mismatching commands");
            assert(0);
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Warning: Received ack while no commands pending");
        //assert(0);
    }

    /*
     * Found an error
     */
    if (error == parsed->status) {
        if (s->isInit()) {
            b->instance->recvCmdApi(setupCmd, b->session, remoteError);
        } else {
            if (parsed->respCode == rtspStatus[Not_Acceptable].num ||
                parsed->respCode == rtspStatus[See_Other].num )
            {
                b->instance->recvCmdApi(pending.cmd, b->session, alreadyError);
            } else {
                  b->instance->recvCmdApi(pending.cmd, b->session, remoteError);
            }
        }
        return true;
    }

    /*
     * Perform get/set callbacks
     */
    if (pending.cmd == getParameterCmd) {
        b->instance->get(b->session, b->theirWfd);
    } else if ((pending.cmd == setParameterCmd) && (pending.subCmd == invalidCmd)) {
        if (pending.wfd.halt.getValid()) {
            s->setMacroState(paused);
        } else if (pending.wfd.start.getValid()) {
            s->setMacroState(ready);
        }
        if(pending.wfd.client.getValid()) {
            pending.wfd.client = b->theirWfd.client;
        }

        b->instance->set(b->session, pending.wfd);
    }

    return false;
}

void rtspM1::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    optionsCommand cmd(cmdRequest, b->session, b->methodSupp);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s : Entering Source Request", name.c_str());

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(optionsCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Source Request", name.c_str());
}

void rtspM1::response(rtspFSM *s, rtspSource *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB :: %s: Entering Source Response", name.c_str());

    tmp->reset();

    /* Setting the bitmap for use of incoming response parsing */
    /* to validate whether all required methods are present */
    tmp->mesg.wfdOptionsParams.set(getParameterCmd);
    tmp->mesg.wfdOptionsParams.set(setParameterCmd);
    tmp->mesg.wfdOptionsParams.set(wfdCmd);
    b->processParsedMesg(tmp);
    if (globalError) {
      if (parsed.next)
          paramsFree(parsed.next);
      return;
    }

    s->setState(&m2);

    while (tmp) {
        if (isAck(tmp)) {
            if (tmp->valid & wfdValid) {
                if (tmp->state.wfdSupp == false) {

                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: WFD not supported");
                  if (parsed.next)
                     paramsFree(parsed.next);
                  return;
                }
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: WFD supported");
            } else {

                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: WFD not valid");
                if (parsed.next)
                   paramsFree(parsed.next);
                return;
            }
        }

        if (isError(s, b, tmp, optionsCmd)) {

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: Options response");
            if (parsed.next)
               paramsFree(parsed.next);
            return;
        }

        if (tmp->mesg.cmd == optionsCmd) {
            optionsCommand cmd(cmdResponse, b->session, b->methodSupp);

            b->instance->sendMesg(b->session, cmd.send());
            if (globalError) {
              if (parsed.next)
                  paramsFree(parsed.next);
              return;
            }
            s->setState(&m3);
            b->request();
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB :: %s: Exiting Source Response", name.c_str());
}

void rtspM2::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: %s: Invalid Source Request", name.c_str());
}

void rtspM2::response(rtspFSM *s, rtspSource *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Source Response", name.c_str());
    parsed.reset();
    b->processParsedMesg(&parsed);
    optionsCommand cmd(cmdResponse, b->session, b->methodSupp);
    b->instance->sendMesg(b->session, cmd.send());
    if (globalError) {
      if (parsed.next)
          paramsFree(parsed.next);
      return;
    }

    s->setState(&m3);

    b->request();
    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Response" , name.c_str() );
}

void rtspM3::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    getParamCommand cmd(cmdRequest, b->wfdGet, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Request", name.c_str() );

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(getParameterCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;
    b->session.incTxCseq();
    // Adding idr request to supported Src capabilities
    b->wfdGet.set(wfd_idr_request);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  " RTSP_LIB :: %s: Exiting Source Request", name.c_str() );
}

void rtspM3::response(rtspFSM *s, rtspSource *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Response", name.c_str() );
    parsed.reset();

    b->processParsedMesg(&parsed);
    if (globalError) {
      if (parsed.next)
          paramsFree(parsed.next);
      return;
    }

    if (isError(s, b, &parsed, getParameterCmd)) {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: Get Parameter response");
        if (parsed.next)
              paramsFree(parsed.next);
        return;
    }

    if (parsed.mesg.wfdParams[wfd_client_rtp_ports]) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Got client rtp ports");
        b->wfdGet.reset(wfd_client_rtp_ports);
    }

    if (parsed.mesg.wfdParams[wfd_audio_codecs]) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Got audio codecs");
        b->wfdGet.reset(wfd_audio_codecs);
    }

    if (parsed.mesg.wfdParams[wfd_video_formats]) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Got video formats");
        b->wfdGet.reset(wfd_video_formats);
    }

    b->getIntersect();

    b->instance->intersect(b->session);

    s->setState(&m4);

    b->request();

    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Response", name.c_str() );
}

void rtspM4::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    setParamCommand cmd(cmdRequest, b->wfdSet, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Request", name.c_str() );

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(setParameterCmd, &b->session, 0, b->session.getTxCseq(), b->session.getWfd(), invalidCmd);
    b->instance->queuePending(pending);
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Request", name.c_str() );
}

void rtspM4::response(rtspFSM *s, rtspSource *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Response", name.c_str() );
    parsed.reset();

    b->processParsedMesg(&parsed);
    if (globalError) {
      if (parsed.next)
          paramsFree(parsed.next);
      return;
    }

    if (isError(s, b, &parsed, setParameterCmd)) {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: Set Parameter response");
        if (parsed.next)
              paramsFree(parsed.next);
        return;
    }

    if (b->instance->getOpMode() == rtsp_wfd::coupledPrimarySink)
        return;

    s->setState(&m5);
    m5.previousCmd = invalidCmd;
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Resetting Previous Command %d", name.c_str(),m5.previousCmd );
    b->request(setupCmd);
    if (parsed.next)
        paramsFree(parsed.next);


    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Response", name.c_str() );
}

void rtspM5::request(rtspFSM *s, rtspSource *b, rtspCmds trigger)
{
    UNUSED(s);
    setParamCommand cmd(cmdRequest, trigger, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Request", name.c_str());

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(setParameterCmd, &b->session, 0 , b->session.getTxCseq(), b->session.getWfd(), trigger);
    ERROR_CHECK;
    if(teardownCmd == trigger || pauseCmd == trigger) {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB: Storing previous trigger command ");
      previousCmd = trigger;
    }
    b->instance->queuePending(pending);
    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Request", name.c_str());
}

void rtspM5::request(rtspFSM *s, rtspSource *b, rtspCmds c, rtspWfd &wfd)
{
    UNUSED(s);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Request", name.c_str());

    if (c == setParameterCmd) {
        b->session.setWfd(wfd);
        setParamCommand setCmd(cmdRequest, wfd.getValidWfd(), b->session);
        b->instance->sendMesg(b->session, setCmd.send());
        rtspPending pending(setParameterCmd, &b->session, 0 , b->session.getTxCseq(), wfd, invalidCmd);
        b->instance->queuePending(pending);
    } else {
        getParamCommand getCmd(cmdRequest, wfd.getValidWfd(), b->session);
        b->instance->sendMesg(b->session, getCmd.send());
        rtspPending pending(getParameterCmd, &b->session, 0 , b->session.getTxCseq());
        pending.setTimeout(getCmd.getCmdTimeout());
        b->instance->queuePending(pending);
    }
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Request", name.c_str());
}

void rtspM5::response(rtspFSM *s, rtspSource *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;
    rtspCodes  errCode = OK;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Entering Source Response", name.c_str());
    tmp->reset();

    b->processParsedMesg(tmp);
    if (globalError) {
      if (parsed.next)
          paramsFree(parsed.next);
      return;
    };

    while (tmp) {
        switch(tmp->mesg.cmd) {
        case setupCmd:
        {
            s->setMacroState(ready);
            b->instance->recvCmdApi(setupCmd, b->session, noError);

            setupCommand cmd(cmdResponse, b->session);
            b->instance->sendMesg(b->session, cmd.send());
            if (globalError) {
              if (parsed.next)
                  paramsFree(parsed.next);

              s->setMacroState(init);
              return;
            }
        }
        break;
        case playCmd:
        {
            if (!s->isPlaying())
            {
              s->setMacroState(playing);
              b->instance->recvCmdApi(playCmd, b->session, noError);
              errCode = OK;
            }
            else
            {
              errCode =  Not_Acceptable;
            }
            playCommand cmd(cmdResponse, b->session, errCode);

            b->instance->sendMesg(b->session, cmd.send());
            if (globalError) {
              if (parsed.next)
                  paramsFree(parsed.next);
              return;
            }

        }
        break;
        case pauseCmd:
        {
            if (!s->isPaused() && !s->isStandby())
            {
              s->setMacroState(paused);
              b->instance->recvCmdApi(pauseCmd, b->session, noError);
              errCode = OK;
            }
            else
            {
              errCode = Not_Acceptable;
            }
            pauseCommand cmd(cmdResponse, b->session, errCode);

            b->instance->sendMesg(b->session, cmd.send());
            if (globalError) {
              if (parsed.next)
                  paramsFree(parsed.next);
              return;
            }

        }
        break;
        case teardownCmd:
        {
            if (!s->isToredown())
            {
              s->setMacroState(toredown);
              b->instance->recvCmdApi(teardownCmd, b->session, noError);
              errCode = OK;
            }
            else
            {
              errCode = Not_Acceptable;
            }
            teardownCommand cmd(cmdResponse, b->session, errCode);

            b->instance->sendMesg(b->session, cmd.send());
            if (globalError) {
              if (parsed.next)
                  paramsFree(parsed.next);
              return;
            }


            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Session closed");
            // If return is removed from here, remove the following as well
            if (parsed.next)
                paramsFree(parsed.next);
            return;
        }
        break;
        case getParameterCmd:
        break;
        case setParameterCmd:
            //Check if standby was set, and only update the macrostate
            //if the intersect capability also supports
            if (tmp->mesg.wfdParams[wfd_standby]&&
                b->isect.standbyCap.getValid() &&
                b->isect.standbyCap.getSetting()) {
                s->setMacroState(standby);
            } else if (b->wfd.start.getValid()) {
                s->setMacroState(ready);
            }
        break;
        default:
        {
            if (isError(s, b, tmp, invalidCmd)) {

                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: Get/Set Parameter response");
                if (parsed.next)
                    paramsFree(parsed.next);
                return;
            }
            if (previousCmd == teardownCmd || previousCmd == pauseCmd) {
                rtspPending pending(setParameterCmd, &b->session, 0 , b->session.getTxCseq(),b->session.getWfd(), previousCmd);
                pending.setTimeout(REQUEST_TIMEOUT);
                b->instance->queuePending(pending);
                previousCmd = invalidCmd; //reinitializing
            }
        }
        break;
        }

        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "RTSP_LIB ::  %s: Exiting Source Response", name.c_str());
}

void rtspM6::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Request" ,name.c_str());
}

void rtspM6::response(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Response", name.c_str() );
}

void rtspM7::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Request", name.c_str() );
}

void rtspM7::response(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Response", name.c_str() );
}

void rtspM8::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Request", name.c_str() );
}

void rtspM8::response(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Response", name.c_str() );
}

void rtspM9::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Response", name.c_str() );
}

void rtspM9::response(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  " RTSP_LIB :: %s: Invalid Source Response", name.c_str() );
}

void rtspM10::request(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  " RTSP_LIB :: %s: Invalid Source Request", name.c_str() );
}

void rtspM10::response(rtspFSM *s, rtspSource *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Source Response", name.c_str() );
}


/*********************************************************/

void rtspM1::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Invalid Sink Request", name.c_str() );
}

void rtspM1::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,  "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    parsed.reset();


    b->processParsedMesg(&parsed);

    if (isError(s, b, &parsed, setupCmd)) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: Options response");
        return;
    }

    optionsCommand cmd(cmdResponse, b->session, b->methodSupp);
    b->instance->sendMesg(b->session, cmd.send());

    s->setState(&m2);

    b->request();

    /* Initialising to avoid any issue in back to back sessions.*/
    m_brecvKeepAlive = false;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s: Exiting Sink Response", name.c_str());
}

void rtspM2::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    optionsCommand cmd(cmdRequest, b->session, b->methodSupp);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str());

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(optionsCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;


    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str());
}

void rtspM2::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response" , name.c_str());

    tmp->reset();

    /* Setting the bitmap for use of incoming response parsing */
    /* to validate whether all required methods are present */
    tmp->mesg.wfdOptionsParams.set(getParameterCmd);
    tmp->mesg.wfdOptionsParams.set(setParameterCmd);
    tmp->mesg.wfdOptionsParams.set(wfdCmd);
    tmp->mesg.wfdOptionsParams.set(setupCmd);
    tmp->mesg.wfdOptionsParams.set(playCmd);
    tmp->mesg.wfdOptionsParams.set(pauseCmd);
    tmp->mesg.wfdOptionsParams.set(teardownCmd);
    b->processParsedMesg(tmp);
    ERROR_CHECK;

    s->setState(&m3);
    while (tmp) {
        if (isAck(tmp)) {
            if (tmp->valid & wfdValid) {
                if (tmp->state.wfdSupp == false) {

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: WFD not supported");
                    return;
                }
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: WFD supported");
            } else {

                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: WFD not valid");
                return;
            }
        }

        if (isError(s, b, tmp, optionsCmd)) {

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Options response");
            return;
        }

        if (tmp->mesg.cmd == getParameterCmd) {
            b->getIntersect(tmp->mesg.wfdParams);
            s->setState(&m4);
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::  %s: Exiting Sink Response", name.c_str());
}

void rtspM3::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB ::  %s: Invalid Sink Request", name.c_str());
}

void rtspM3::response(rtspFSM *s, rtspSink *b)
{
    getParamCommand cmd(cmdResponse, b->session);
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s: Entering Sink Response", name.c_str());
    parsed.reset();

    b->processParsedMesg(&parsed);
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s: globalError = %d",name.c_str(),globalError);
    ERROR_CHECK;
    b->getIntersect(parsed.mesg.wfdParams);

    b->instance->get(b->session, b->session.getWfd());

    s->setState(&m4);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::  %s: Exiting Sink Response", name.c_str());
}

void rtspM4::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB ::  %s: Invalid Sink Request", name.c_str() );
}

void rtspM4::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    parsed.reset();

    b->processParsedMesg(&parsed);
    ERROR_CHECK;
    s->setState(&m5);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}

void rtspM5::request(rtspFSM *s, rtspSink *b, rtspCmds c, rtspWfd &wfd)
{
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str() );

    if (c == setParameterCmd) {
        /**
           If session is in standby or pause state,
           then ignore request for I frame.
        **/
        if((s->isPaused() || s->isStandby()) && wfd.idrReq.getValid())
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Ignoring IDR request in Pause/Standby state",
                                                    name.c_str() );
            return;
        }
        else
        {
            b->session.setWfd(wfd);
            setParamCommand setCmd(cmdRequest, wfd.getValidWfd(), b->session);
            b->instance->sendMesg(b->session, setCmd.send());
            /**For I frame request don't queue it to pending list
               as if ack is delayed it will unnecessarily timeout current session
            **/
            if(!wfd.idrReq.getValid())
            {
                rtspPending pending(setParameterCmd, &b->session, 0 , b->session.getTxCseq(), wfd, invalidCmd);
                b->instance->queuePending(pending);
            }
        }
    } else {
        getParamCommand getCmd(cmdRequest, wfd.getValidWfd(), b->session);
        b->instance->sendMesg(b->session, getCmd.send());
        rtspPending pending(getParameterCmd, &b->session, 0 , b->session.getTxCseq());
        b->instance->queuePending(pending);
    }
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Request", name.c_str() );
}

void rtspM5::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    tmp->reset();
    if(s->isStandby() || s->isPaused())
    {
      tmp->rtsp_state = standby;
    }

    b->processParsedMesg(tmp);
    ERROR_CHECK;
    while (tmp) {
        if (tmp->mesg.subCmd == invalidCmd) {
            switch(tmp->mesg.cmd) {
            case getParameterCmd:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "RTSP_LIB :: rtspM5::response : received keepalive");
                    /*Since we are in M5 state,
                     * any get parameter is assumed to be keep alive
                    **/
                    m_brecvKeepAlive = true;
                }
            break;
            case setParameterCmd:
                if (b->theirWfd.halt.getValid()) {
                    s->setMacroState(paused);
                } else if (b->theirWfd.start.getValid()) {
                    s->setMacroState(ready);
                }
            break;
            default:
                if (isError(s, b, tmp, invalidCmd)) {

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Get/Set Parameter response");
                    return;
                }
            break;
            }
        }else if(parsed.mesg.subCmd == pauseCmd){
          if(s->isStandby() || s->isPaused())
          {
            //Pause in Standby is not allowed.
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Already in Standby/Pause response");
          }
          else
          {
            cmdToState(s, tmp->mesg.subCmd);
            b->request();
          }
        }else {
            cmdToState(s, tmp->mesg.subCmd);
            b->request();
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}

void rtspM6::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    setupCommand cmd(cmdRequest, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str() );

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(setupCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Request", name.c_str() );
}

void rtspM6::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    parsed.reset();

    b->processParsedMesg(&parsed);
    ERROR_CHECK;

    if (isError(s, b, &parsed, setupCmd)) {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Setup response");
        return;
    }

    if(b != NULL)
    {
      if(CONVERT_SEC_TO_MSEC(b->keepAliveTimeout) < WFD_SNK_RTSP_MIN_KEEPALIVE_TIMER_VALUE)
      {
        m_hkeepAliveTimerValue = WFD_SNK_RTSP_KEEPALIVE_CHECK_INTERVAL;
      }
      else
      {
        m_hkeepAliveTimerValue = CONVERT_SEC_TO_MSEC(b->keepAliveTimeout) + WFD_SNK_RTSP_KEEPALIVE_BUFFER_TIME;
      }

      s->setMacroState(ready);
      b->instance->recvCmdApi(setupCmd, b->session, noError);
      //b->theirWfd.dump();
      s->setState(&m7);

      b->request();
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}

void rtspM7::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    playCommand cmd(cmdRequest, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str() );

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Sending Prepare to client before PLAY", name.c_str() );
    b->instance->recvCmdApi(playCmd, b->session, noErrorPreSendCmdNotify);
    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(playCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Request", name.c_str() );
}

void rtspM7::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;
    bool bResetState = false;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    tmp->reset();

    b->processParsedMesg(tmp);
    ERROR_CHECK;

    while (tmp) {
        if (tmp->mesg.subCmd == invalidCmd) {
            if(tmp->mesg.cmd == getParameterCmd)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: rtspM7::response : received keepalive");
                m_brecvKeepAlive = true;
                return;
            }
            else if (isError(s, b, tmp, playCmd)) {
                bResetState = true;
                if (tmp->respCode != rtspStatus[Not_Acceptable].num) {
                    b->instance->recvCmdApi(playCmd, b->session, remoteError);

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Play response");
                    return;
                } else
                    b->instance->recvCmdApi(playCmd, b->session, alreadyError);
            } else {
                bResetState = true;
                s->setMacroState(playing);
                b->instance->recvCmdApi(playCmd, b->session, noError);
                b->theirWfd.halt.setValid(false);
                if (m_hTimer == NULL) {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: keepAliveTimerValue timer created");
                  MM_Timer_Create(m_hkeepAliveTimerValue,1,rtspBaseState::isKeepAlive, (void*)b, &m_hTimer);
                }
            }
        } else {
            bResetState = true;
            cmdToState(s, tmp->mesg.subCmd);
            b->request();
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    if (bResetState) {
         s->setState(&m5);
    }
    else
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Persist in M7 ");

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}

void rtspM8::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );

    tmp->reset();

    b->processParsedMesg(tmp);
    ERROR_CHECK;


    while (tmp) {
        if (tmp->mesg.subCmd == invalidCmd) {
            if (isError(s, b, tmp, teardownCmd)) {
                if (tmp->respCode != rtspStatus[Not_Acceptable].num)
                    b->instance->recvCmdApi(playCmd, b->session, remoteError);
                //throw string("Error: Teardown response");
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB ::  %s: Teardown failed", name.c_str() );
            } else {
                s->setMacroState(toredown);
                b->instance->recvCmdApi(teardownCmd, b->session, noError);
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Session closed");
                return;
            }
        } else {
            cmdToState(s, tmp->mesg.subCmd);
            b->request();
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    s->setState(&m5);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}

void rtspM8::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    teardownCommand cmd(cmdRequest, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str() );

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(teardownCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;
    b->session.incTxCseq();
    releaseKeepAliveTimer();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Request", name.c_str() );
}

void rtspM9::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    pauseCommand cmd(cmdRequest, b->session);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Request", name.c_str() );

    b->instance->sendMesg(b->session, cmd.send());
    rtspPending pending(pauseCmd, &b->session, 0, b->session.getTxCseq());
    b->instance->queuePending(pending);
    ERROR_CHECK;

    b->session.incTxCseq();

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Request", name.c_str() );
}

void rtspM9::response(rtspFSM *s, rtspSink *b)
{
    rtspParams parsed;
    rtspParams *tmp = &parsed;
    bool bResetState = false;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Entering Sink Response", name.c_str() );
    tmp->reset();

    b->processParsedMesg(tmp);
    ERROR_CHECK;

    while (tmp) {
        if (tmp->mesg.subCmd == invalidCmd) {
            if(tmp->mesg.cmd == getParameterCmd)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "RTSP_LIB :: rtspM9::response : received keepalive");
                m_brecvKeepAlive = true;
            }
            else if (isError(s, b, tmp, pauseCmd)) {
                bResetState = true;
                if (tmp->respCode != rtspStatus[Not_Acceptable].num) {
                    b->instance->recvCmdApi(playCmd, b->session, remoteError);

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Error: Pause response");
                    return;
                } else
                    b->instance->recvCmdApi(pauseCmd, b->session, alreadyError);
            } else {
                bResetState = true;
                s->setMacroState(paused);
                b->instance->recvCmdApi(pauseCmd, b->session, noError);
            }
        } else {
            bResetState = true;
            cmdToState(s, tmp->mesg.subCmd);
            b->request();
        }
        tmp = tmp->next;
    }

    if (parsed.next)
        paramsFree(parsed.next);

    if (bResetState)
        s->setState(&m5);
    else
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Persist in M9");

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB ::  %s: Exiting Sink Response", name.c_str() );
}
void rtspM10::request(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB ::  %s: Invalid Sink Request", name.c_str() );
}

void rtspM10::response(rtspFSM *s, rtspSink *b)
{
    UNUSED(s);
    UNUSED(b);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB ::  %s: Invalid Sink Response", name.c_str() );
}
