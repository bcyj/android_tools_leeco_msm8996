/***************************************************************************
 *                             rtsp_source.cpp
 * DESCRIPTION
 *  RTSP Source definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_source.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_source.h"
#include "MMDebugMsg.h"

/*
 * Entry point for source commands
 */
int rtspSource::sendCommand(rtspCmds cmd)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Sending Cmd Request");

    if (!fsm.isReady()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to send command in invalid state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    if (instance->numPending(session.getSessionID())) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to send command while cmd pending.");
        instance->recvCmdApi(cmd, session, pendingCmdError);
        return -1;
    }

    if (fsm.isStandby() && (cmd == pauseCmd)) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to send command in standby state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    request(cmd);
    ERROR_CHECK_VAL;

    return 0;
}

/*
 * Entry point for source get/set commands
 */
int rtspSource::sendCommandUpdate(rtspCmds cmd, rtspWfd &w)
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Sending Cmd Request");

    if (!fsm.isReady()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to send command in invalid state.");
        instance->recvCmdApi(cmd, session, badStateError);
        return -1;
    }

    if ((cmd == setParameterCmd) &&
        (isect != w)) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to set parameters outside of intersection.");
        instance->recvCmdApi(cmd, session, badParamsError);
        return -1;
    }

    if (instance->numPending(session.getSessionID())) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Attempting to send command while cmd pending.");
        instance->recvCmdApi(cmd, session, pendingCmdError);
        return -1;
    }


    request(cmd, w);
    ERROR_CHECK_VAL;

    return 0;
}

/*
 * Get intersection of source and sink capabilities
 */
void rtspSource::getIntersect()
{
    if (instance->getOpMode() == rtsp_wfd::coupledPrimarySink) {
        rtspWfd tmp = session.getWfd();
        bool videoValid = tmp.videoHeader.getValid();
        bool cbpValid = tmp.h264Cbp.getValid();
        bool chpValid = tmp.h264Chp.getValid();

        tmp.reset();
        if (videoValid) {
            tmp.videoHeader.setValid(true);
            if (cbpValid)
                tmp.h264Cbp.setValid(true);
            if (chpValid)
                tmp.h264Chp.setValid(true);
        }
        session.copyWfd(tmp);
    } else {
        rtspWfd workingSet;

        workingSet.assign(session.getWfd() & theirWfd);
        isect = workingSet;
        workingSet.calcWorkingSet();
        workingSet.dump();
        session.setWfd(workingSet);
    }
}

/*
 * Update state based on inbound data
 */
void rtspSource::applySettings(rtspParams *parsed)
{
    theirWfd += parsed->state.wfd;

    if (instance->getOpMode() != rtsp_wfd::coupledPrimarySink) {
        if (session.isSecondarySink()) {
            theirWfd.client.setRtpPort0(0);
            session.setIp0("");
        } else {
            theirWfd.client.setRtpPort1(0);
            session.setIp1("");
        }
    }

    if (parsed->mesg.cmd == setParameterCmd) {
        rtspWfd workingSet;
        workingSet.assign(isect & theirWfd);
        session.setWfd(workingSet);

        //Checking if the parameter set was supported at all

        if (((parsed->mesg.wfdParams & wfdGet) != 0 )) {
        instance->set(session, parsed->state.wfd);
        }

        else if(parsed->mesg.wfdParams.test(wfd_standby)){
           if(wfdGet.test(wfd_standby_resume_capability)){
                  instance->set(session, parsed->state.wfd);
           }
        }
        else if(parsed->mesg.wfdParams.test(wfd_uibc_setting)){
           if(isect.uibcCap.getValid()){
                  instance->set(session, parsed->state.wfd);
           }
        }
        else {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Don't call set callback because its unsupported cap");
        }
    }
    if(parsed->mesg.cmd == setupCmd) {
        rtspWfd workingSet = session.getWfd();
        if(parsed->state.wfd.client.getRtcpPort0()) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Client has rtcp port");
            workingSet.client.setRtcpPort0(parsed->state.wfd.client.getRtcpPort0());
            session.setWfd(workingSet);
        }
    }
}

void rtspSource::resetCapabilities(rtspWfdParams param)
{
    wfdGet.reset(param);
}
