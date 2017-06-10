#ifndef _RTSP_SOURCE_H
#define _RTSP_SOURCE_H

/***************************************************************************
 *                             rtsp_source.h
 * DESCRIPTION
 *  RTSP Source class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_source.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_base.h"
#include "rtsp_state.h"
#include "rtsp_server.h"

/*
 * RTSP source class
 */
class rtspSource : public rtspBase {
public:
    rtspSource(SOCKET networkSocket, string ipAddr, rtspWfd wfd, rtspServer *server, unsigned uibcPort, rtsp_wfd::rtspMode mode, sockaddr_in saddr) : rtspBase(networkSocket, ipAddr, wfd, uibcPort, saddr), instance(server)
    {
        wfdGet.reset();
        wfdSet.reset();

        if (mode == rtsp_wfd::coupledPrimarySink) {
            wfdGet.set(wfd_audio_codecs);
            wfdSet.set(wfd_video_formats);
            wfdSet.set(wfd_presentation_URL);
        } else {
            wfdGet.set(wfd_client_rtp_ports);
            wfdGet.set(wfd_audio_codecs);
            wfdGet.set(wfd_video_formats);
            wfdGet.set(wfd_uibc_capability);
            wfdGet.set(wfd_coupled_sink);
            wfdGet.set(wfd_display_edid);
            wfdGet.set(wfd_standby_resume_capability);
            wfdGet.set(wfd_content_protection);
            wfdGet.set(wd_initial_buffer);
            wfdSet.set(wfd_audio_codecs);
            wfdSet.set(wfd_video_formats);
            wfdSet.set(wfd_presentation_URL);
            wfdSet.set(wfd_client_rtp_ports);
            wfdSet.set(wfd_uibc_capability);
            wfdSet.set(wd_decoder_latency);
            //wfdSet.set(wfd_standby_resume_capability);
        }

        methodSupp.set(wfdCmd);
        methodSupp.set(getParameterCmd);
        methodSupp.set(setParameterCmd);
        methodSupp.set(setupCmd);
        methodSupp.set(playCmd);
        methodSupp.set(pauseCmd);
        methodSupp.set(teardownCmd);
    }
    void request() { fsm.request(this); }
    void request(rtspCmds trigger) { fsm.request(this, trigger); }
    void request(rtspCmds cmd, rtspWfd &wfd) { fsm.request(this, cmd, wfd); }
    void response() { fsm.response(this); }

    bool isToredown() { return fsm.isToredown(); }
    int sendCommand(rtspCmds);
    int sendCommandUpdate(rtspCmds, rtspWfd&);
    void getIntersect();
    void resetCapabilities(rtspWfdParams);

    rtspServer *instance;
    rtspWfd isect;

private:
    void applySettings(rtspParams *);

    rtspFSM fsm;
};

#endif /*_RTSP_SOURCE_H*/
