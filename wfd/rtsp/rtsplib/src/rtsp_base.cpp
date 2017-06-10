/***************************************************************************
 *                             rtsp_base.cpp
 * DESCRIPTION
 *  RTSP Base definition for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_base.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_base.h"
#include "rtsp_helper.h"
#include "rtsp_state.h"

#define NOT_APPLICABLE 406
#define APPLICABLE 0
/*
 * Parse inbound messages and update state
 */
void rtspBase::processParsedMesg(rtspParams *parsed)
{
    rtspHelper helper;
    /*
     * Receive socket message
     */
    recvCmd(helper.recvMesg(session), parsed);
    ERROR_CHECK;

    do {
        if (parsed->valid & mesgValid) {

            /*
             * Update rx sequence number
             */
            if (parsed->mesg.type == cmdRequest) {
                if (parsed->valid & cRxSeqValid) {
                    session.setRxCseq(parsed->state.rxCseq);
                }
            }
             /*
             * Update state from parsed message data
             */
            applySettings(parsed);

            /*
             * Send ack's for get/set messages
             */
            if (parsed->mesg.cmd == getParameterCmd) {
                getParamCommand cmd(cmdResponse, parsed->mesg.wfdParams, session);
                helper.sendMesg(session, cmd.send());
                ERROR_CHECK;
            }
            if (parsed->mesg.cmd == setParameterCmd) {
                setParamCommand cmd(cmdResponse, parsed->mesg.wfdParams, session);
                cmd.status = APPLICABLE;
                if(parsed->mesg.subCmd != playCmd &&
                   parsed->mesg.subCmd != teardownCmd &&
                   parsed->mesg.subCmd != invalidCmd)
                {
                  if(parsed->rtsp_state == standby)
                  {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Sending to rtsp_session 406");
                    cmd.status = NOT_APPLICABLE;
                  }
                }
                helper.sendMesg(session, cmd.send());
                ERROR_CHECK;
            }
        }
        parsed = parsed->next;

    } while (parsed != NULL);
}
