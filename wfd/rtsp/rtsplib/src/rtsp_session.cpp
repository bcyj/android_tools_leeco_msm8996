/***************************************************************************
 *                             rtsp_session.cpp
 * DESCRIPTION
 *  RTSP Session definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014  QUALCOMM Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_session.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_session.h"
#include "MMDebugMsg.h"
#include <string>
#include "RTSPStringStream.h"

#define NOT_APPLICABLE 406


/*
 * Overloaded stream operators used for creating request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, rtspCommands *Cmd)
{
    rtspSession *state = &Cmd->state;

    if (Cmd->mesg.type == cmdRequest) {
        stream << supportedCmds[Cmd->mesg.cmd].cmdName << " " << wfdUrl << " " << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
    } else {
        stream << rtspVersion << " " << successCode << " " << success << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate() << crlf;
    }
    stream << crlf;

    return stream;
}

/*
 * Overloaded stream operators used for creating Option request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, optionsCommand *Opt)
{
    rtspSession *state = &Opt->state;
    size_t count = Opt->suppModes.count();

    if (Opt->mesg.type == cmdRequest) {
        stream << supportedCmds[Opt->mesg.cmd].cmdName << " * " << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        stream << "Require: " << supportedCmds[1].cmdName << crlf;
    } else {
        stream << rtspVersion << " " << successCode << " " << success << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << "Public: ";
        for (int i = 1; i < numSupportedCmds; i++, count--) {
            if (!Opt->suppModes[i])
                continue;
            stream << supportedCmds[i].cmdName.c_str();
            if (count >= 1) {
                stream << ", ";
            }
        }

        stream << crlf;
    }
    stream << crlf;

    return stream;
}

/*
 * Overloaded stream operators used for creating Get Parameter request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, getParamCommand *GetP)
{
    rtspSession *state = &GetP->state;
    bool prev = false;
    size_t length = 0;
    RTSPStringStream ss;
    string myStr;

    if (GetP->mesg.type == cmdRequest) {
        stream << supportedCmds[GetP->mesg.cmd].cmdName << " " << wfdUrl << " " << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;

        for (int i = 1; i < numSupportedWfdParams; i++) {
            if (GetP->mesg.wfdParams[i]) {
                ss << wfdTable[i].wfdName << crlf;
            }
        }
        myStr = ss.str();
        if(myStr.length() == 0) {
           stream << session << " " << setfill('0') << setw(8) << state->getSessionStr()  << crlf << crlf;
           GetP->setCmdTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT);
       }
        else
        {
            length = myStr.length();
            stream << contentLength << " " << length << crlf;
            stream << contentType << crlf << crlf;
            stream << myStr;
        }
    }
    else {
        stream << rtspVersion << " " << successCode << " " << success << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;

        for (int i = 1; i < numSupportedWfdParams; i++) {
            if (GetP->mesg.wfdParams[i]) {
                switch(wfdTable[i].wfdParam) {
                case wfd_client_rtp_ports:
                    if (state->state.wfd.client.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.client << crlf;
                    }
                break;
                case wfd_audio_codecs:
                    prev = false;
                    if ((state->state.wfd.getValidWfd())[wfd_audio_codecs]) {
                        ss << wfdTable[i].wfdName << ": ";

                        if (state->state.wfd.audioLpcm.getValid()) {
                            ss << state->state.wfd.audioLpcm;
                            prev = true;
                        }
                        if (state->state.wfd.audioAac.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioAac;
                            prev = true;
                        }
                        if (state->state.wfd.audioEac.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioEac;
                            prev = true;
                        }
                        if (state->state.wfd.audioAc3.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioAc3;
                            prev = true;
                        }
                        if (state->state.wfd.audioDts.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioDts;
                            prev = true;
                        }
                        ss << crlf;
                    }
                    else
                    {
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                    }
                break;
                case wfd_video_formats:
                    prev = false;
                    if ((state->state.wfd.getValidWfd())[wfd_video_formats]) {
                        ss << wfdTable[i].wfdName << ": ";
                        if (state->state.wfd.videoHeader.getValid())
                            ss << state->state.wfd.videoHeader << " ";
                        if (state->state.wfd.h264Cbp.getValid()) {
                            ss << state->state.wfd.h264Cbp;
                            prev = true;
                        }
                        if (state->state.wfd.h264Chp.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.h264Chp;
                            prev = true;
                        }
                        if (state->state.wfd.h264Chi444p.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.h264Chi444p;
                            prev = true;
                        }
                        ss << crlf;
                    }
                    else
                    {
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                    }
                break;

                case wfd_uibc_capability:
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.uibcCap << crlf;
                break;
                case wfd_standby_resume_capability:
                  if (state->state.wfd.standbyCap.getValid())
                  {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.standbyCap << crlf;
                  }
                  else
                  {
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                  }
                  break;
                case wfd_coupled_sink:
                    if (state->state.wfd.coupledSink.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.coupledSink << crlf;
                    }
                    else
                    {
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                    }
                break;
                case wfd_content_protection:
                    if (state->state.wfd.contentProtection.getValid()) {
                  ss << wfdTable[i].wfdName << ": ";
                  ss << state->state.wfd.contentProtection << crlf;
                    }
                    else
                    {
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                    }
                  break;

                case wfd_display_edid:
                    if (state->state.wfd.edid.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.edid << crlf;
                    }
                    else
                    {
                        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSP_LIB :: edid NONE");
                        ss << wfdTable[i].wfdName << ": " << none << crlf;
                    }
                break;
                case wfd_route:
                case wfd_idr_request:
                    // Nothing to respond
                break;
                case wfd_connector_type:
                   MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"RTSP_LIB :: ConnectorType = %d ",state->state.wfd.connectorType);

                   ss << wfdTable[i].wfdName << ": " << "0" << state->state.wfd.connectorType << crlf;
                break;
#ifndef LEGACY_TCP
                case wd_initial_buffer:
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSP_LIB :: wd_initial_buffer ");
                    if (state->state.wfd.tcpWindowSize.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.tcpWindowSize.getWindowSize() << crlf;
                    }
                break;
#endif
                default:
                    ss << wfdTable[i].wfdName << ": " << none << crlf;
                break;
                }
            }
        }
        myStr = ss.str();
        length = myStr.length();
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", myStr.c_str());
        if(length) {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s",  myStr.c_str());
            stream << contentLength << " " << length << crlf;
            stream << contentType << crlf << crlf;
            stream << myStr;
        }
        else
        {
            stream << crlf;
        }
    }
    return stream;
}

/*
 * Overloaded stream operators used for creating Set Parameter request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, setParamCommand *SetP)
{
    rtspSession *state = &SetP->state;
    bool prev = false;
    bool bError = false;
    size_t length = 0;
    RTSPStringStream ss;
    string myStr;

    if (SetP->mesg.type == cmdRequest) {
        stream << supportedCmds[SetP->mesg.cmd].cmdName << " " << wfdUrl << " " << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        stream << session << " " << setfill('0') << setw(8) << state->getSessionStr() << crlf;

        for (int i = 1; i < numSupportedWfdParams; i++) {
            if (SetP->mesg.wfdParams[i]) {
                switch(wfdTable[i].wfdParam) {
#ifdef LEGACY_TCP
                case wfd_vnd_sec_max_buffer_length:
                    if (state->state.wfd.buffLen.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.buffLen.getBufferLen() << crlf;
                    }
                    break;
                case wfd_vnd_sec_tcp_window_size:
                    if (state->state.wfd.tcpWindowSize.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.tcpWindowSize.getWindowSize() << crlf;
                    }
                    break;
                case wfd_vnd_sec_control_playback:
                    if(state->state.wfd.tcpStreamControl.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << wfdTable[i].wfdName << "=";

                        rtsp_wfd::tcp_control_command eCmd =
                            state->state.wfd.tcpStreamControl.getCommand();
                        switch(eCmd) {
                        case rtsp_wfd::flush:
                            ss << "flush"<< crlf;
                            break;
                        case rtsp_wfd::pause:
                            ss << "pause" << crlf;
                            break;
                        case rtsp_wfd::play:
                            ss << "play" << crlf;
                            break;
                        case rtsp_wfd::status:
                            ss << "status" << crlf;
                            break;
                        case rtsp_wfd::cmdNone:
                        default:
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Invalid tcp control command");
                            ss << "none" << crlf;
                            break;

                        }
                    }
                    state->state.wfd.tcpStreamControl.setValid(false);
                    break;
#else
                    case wd_initial_buffer:
                    if (state->state.wfd.tcpWindowSize.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.tcpWindowSize.getWindowSize() << crlf;
                    }
                    break;

                    case wd_decoder_latency:
                    if (state->state.wfd.buffLen.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.buffLen.getBufferLen() << crlf;
                    }
                    break;
                    case wd_playback_control:
                    if(state->state.wfd.tcpStreamControl.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << wfdTable[i].wfdName << "=";

                        rtsp_wfd::tcp_control_command eCmd =
                            state->state.wfd.tcpStreamControl.getCommand();
                        unsigned long timeStamp =
                            state->state.wfd.tcpStreamControl.getDuration();
                        switch(eCmd) {
                        case rtsp_wfd::flush:
                            ss << "flush"<< ";" <<"flush_timing=" << timeStamp <<crlf;
                            break;
                        case rtsp_wfd::pause:
                            ss << "pause" << crlf;
                            break;
                        case rtsp_wfd::play:
                            ss << "play" << crlf;
                            break;
                        case rtsp_wfd::status:
                            ss << "status" << crlf;
                            break;
                        case rtsp_wfd::cmdNone:
                        default:
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Invalid tcp control command");
                            ss << "none" << crlf;
                            break;

                        }
                    }
                    state->state.wfd.tcpStreamControl.setValid(false);
                    break;
#endif
                    case wfd_client_rtp_ports:
                    if (state->state.wfd.client.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.client << crlf;
                    }
                break;
                case wfd_trigger_method:
                    switch(SetP->mesg.subCmd) {
                    case setupCmd:
                    case playCmd:
                    case pauseCmd:
                    case teardownCmd:
                        ss << wfdTable[i].wfdName;
                        ss << ": ";
                        ss << supportedCmds[SetP->mesg.subCmd].cmdName << crlf;
                    break;
                    default:
                    break;
                    }
                break;
                case wfd_presentation_URL:
                    if (state->state.wfd.uri.getValid()) {
                        ss << wfdTable[i].wfdName;
                        ss << ": " << state->state.wfd.uri << crlf;
                    }
                break;
                case wfd_audio_codecs:
                    prev = false;
                    if ((state->state.wfd.getValidWfd())[wfd_audio_codecs]) {
                        ss << wfdTable[i].wfdName << ": ";
                        if (state->state.wfd.audioLpcm.getValid()) {
                            ss << state->state.wfd.audioLpcm;
                            prev = true;
                        }
                        if (state->state.wfd.audioAac.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioAac;
                            prev = true;
                        }
                        if (state->state.wfd.audioEac.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioEac;
                            prev = true;
                        }
                         if (state->state.wfd.audioAc3.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioAc3;
                            prev = true;
                        }
                        if (state->state.wfd.audioDts.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.audioDts;
                            prev = true;
                        }
                        ss << crlf;
                    }
                break;
                case wfd_video_formats:
                    prev = false;
                    if ((state->state.wfd.getValidWfd())[wfd_video_formats]) {
                        ss << wfdTable[i].wfdName << ": ";
                        if (state->state.wfd.videoHeader.getValid())
                            ss << state->state.wfd.videoHeader << " ";
                        if (state->state.wfd.h264Cbp.getValid()) {
                            ss << state->state.wfd.h264Cbp;
                            prev = true;
                        }
                        if (state->state.wfd.h264Chp.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.h264Chp;
                            prev = true;
                        }
                        if (state->state.wfd.h264Chi444p.getValid()) {
                            if (prev) ss << ", ";
                            ss << state->state.wfd.h264Chi444p;
                           prev = true;
                        }
                        ss << crlf;
                    }
                break;
                case wfd_uibc_capability:
                    if (state->state.wfd.uibcCap.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.uibcCap << crlf;
                    }
                break;
                case wfd_content_protection:
                if (state->state.wfd.contentProtection.getValid()) {
                    ss << wfdTable[i].wfdName << ": ";
                    ss << state->state.wfd.contentProtection<< crlf;
                }
                break;
                case wfd_uibc_setting:
                    if (state->state.wfd.uibcSet.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.uibcSet << crlf;
                    }
                break;
                case wfd_standby_resume_capability:
                    if (state->state.wfd.standbyCap.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.standbyCap << crlf;
                    }
                break;
                case wfd_standby:
                    if (state->state.wfd.halt.getValid()) {
                        ss << wfdTable[i].wfdName << crlf;
                    }
                break;
                case wfd_resume:
                    if (state->state.wfd.start.getValid()) {
                        ss << wfdTable[i].wfdName << crlf;
                    }
                break;
                case wfd_coupled_sink:
                    if (state->state.wfd.coupledSink.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.coupledSink << crlf;
                    }
                break;

                case wfd_route:
                    if (state->state.wfd.audioRoute.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.audioRoute << crlf;
                    }
                break;
                case wfd_idr_request:
                    if (state->state.wfd.idrReq.getValid()) {
                        ss << wfdTable[i].wfdName << crlf;
                    }
                break;
                case wfd_av_format_change_timing:
                    if (state->state.wfd.timing.getValid()) {
                        ss << wfdTable[i].wfdName << ": ";
                        ss << state->state.wfd.timing;
                        ss << crlf;
                    }
                break;
                default:
                break;
                }
            }
        }
        myStr = ss.str();
        length = myStr.length();

        stream << contentLength << " " << length << crlf;
        stream << contentType << crlf << crlf;
        stream << myStr;
    } else {
        for (int i = 1; i < numSupportedWfdParams; i++) {
          if (SetP->mesg.wfdParams[i]) {
              switch((unsigned int)wfdTable[i].wfdParam) {
              case wfd_standby:
              case wfd_resume:
                  if (!SetP->state.state.wfd.standbyCap.getSetting()) {
                      ss << wfdTable[i].wfdName << ": ";
                      ss << "404" << crlf;
                      bError = true;
                  }
              break;
              }
           }
        }
        if (bError)
        {
          stream << rtspVersion << " " << errorCode << crlf;
        }
        else
        {
          if(SetP->status == NOT_APPLICABLE)  //406
          {
            stream << rtspVersion << " " << standbyNotApplicable << crlf;
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Creating notApplicable");
          }else
          {
            stream << rtspVersion << " " << successCode << " " << success << crlf;
          }
        }
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate()<<crlf ;

        // Add response string if bufferring update is present
#ifdef LEGACY_TCP
        if(state->state.wfd.buffLen.getValid() ||
           state->state.wfd.tcpWindowSize.getValid()) {
            stream << wfdTable[wfd_vnd_sec_max_buffer_length].wfdName << ": ";
            stream << state->state.wfd.buffLen.getBufferLen() << crlf;
            stream << wfdTable[wfd_vnd_sec_tcp_window_size].wfdName << ": ";
            stream << state->state.wfd.tcpWindowSize.getWindowSize() << crlf;

        }
#else
        if (state->state.wfd.client.getValid() && state->state.wfd.client.getRenegotiated()) {
            state->state.wfd.client.setRenegotiated(false);
            stream << crlf << wfdTable[wfd_client_rtp_ports].wfdName << ": ";
            stream << state->state.wfd.client;
        }
#endif
        if (bError) {
          myStr = ss.str();
          length = myStr.length();
          stream << contentLength << " " << length << crlf;
          stream << contentType << crlf << crlf;
          stream << myStr;
       }
        else
        {
          stream << crlf ;
        }
    }

    return stream;
}

/*
 * Overloaded stream operators used for creating Setup request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, setupCommand *Setup)
{
    rtspSession *state = &Setup->state;
    rtspWfd wfd = state->getWfd();

    if (Setup->mesg.type == cmdRequest) {
        stream << supportedCmds[Setup->mesg.cmd].cmdName << " " << state->getUri();
        stream << " " << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        //stream << session << " " << state->session << crlf;
        stream << "Transport: " << profile;
        if(wfd.client.getValid()) {
            stream << ";"<< "client_port=" << dec << wfd.client.getRtpPort0();
            if(wfd.client.getRtcpPort0()) {
                stream << "-" << dec << wfd.client.getRtcpPort0();
            }
        }


        stream << crlf;
    } else {
        stream << rtspVersion << " " << successCode << " " << success << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate() << crlf;
        stream << session << " " << setfill('0') << setw(8) << state->getSessionStr() << crlf;
        stream << "Transport: " << profile;

        if(wfd.client.getValid()) {
            stream << ";"<< "client_port=" << dec << wfd.client.getRtpPort0();
            if(wfd.client.getRtcpPort0() &&
               (wfd.server.getValid() && wfd.server.getRtcpPort0())) {
                stream << "-" << dec << wfd.client.getRtcpPort0();
            }
        }

        if(wfd.server.getValid()) {
            stream << ";"<< "server_port=" << dec << wfd.server.getRtpPort0();
            if(wfd.server.getRtcpPort0()) {
                stream << "-" << dec << wfd.server.getRtcpPort0();
            }
        }
        stream << crlf;
    }
    stream << crlf;

    return stream;
}

/*
 * Overloaded stream operators used for creating Play request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, playCommand *Play)
{
    rtspSession *state = &Play->state;

    if (Play->mesg.type == cmdRequest) {
        stream << supportedCmds[Play->mesg.cmd].cmdName << " " << state->getUri() << " ";
        stream << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        stream << session << " " << setfill('0') << setw(8) << state->getSessionStr() << crlf;
    } else {
        stream << rtspVersion << " " << rtspStatus[Play->status].num << " " << rtspStatus[Play->status].text << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate() << crlf;
        //stream << session << " " << setfill('0') << setw(8) << state->getSessionID() << crlf;
    }
    stream << crlf;

    return stream;
}

/*
 * Overloaded stream operators used for creating Pause request/response messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, pauseCommand *Pause)
{
    rtspSession *state = &Pause->state;

    if (Pause->mesg.type == cmdRequest) {
        stream << supportedCmds[Pause->mesg.cmd].cmdName << " " << state->getUri() << " ";
        stream << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        stream << session << " " << setfill('0') << setw(8) << state->getSessionStr() << crlf;
    } else {
        stream << rtspVersion << " " << rtspStatus[Pause->status].num << " " << rtspStatus[Pause->status].text << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate() << crlf;
        //stream << session << " " << setfill('0') << setw(8) << state->getSessionID() << crlf;
    }
    stream << crlf;

    return stream;
}

/*
 * Overloaded stream operators used for creating Teardown response/request messages
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, teardownCommand *Teardown)
{
    rtspSession *state = &Teardown->state;

    if (Teardown->mesg.type == cmdRequest) {
        stream << supportedCmds[Teardown->mesg.cmd].cmdName << " " << state->getUri() << " ";
        stream << rtspVersion << crlf;
        stream << cSeq << " " << state->getTxCseq() << crlf;
        stream << session << " " << setfill('0') << setw(8) << state->getSessionStr() << crlf;
    } else {
        stream << rtspVersion << " " << rtspStatus[Teardown->status].num << " " << rtspStatus[Teardown->status].text << crlf;
        stream << cSeq << " " << state->getRxCseq() << crlf;
        stream << state->getDate() << crlf;
        //stream << session << " " << setfill('0') << setw(8) << state->getSessionID() << crlf;
    }
    stream << crlf;

    return stream;
}


/*
 * Send routines for each command. We have one for each command
 * in order to create command specific overloaded stream operators.
 * See above.
 */
string rtspCommands::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Command Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Command Send");
    return output.str();
}

string optionsCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Options Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Options Send");
    return output.str();
}

string getParamCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Get Parameter Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Get Parameter Send");
    return output.str();
}

string setParamCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Set Parameter Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Set Parameter Send");
    return output.str();
}

string setupCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Setup Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Setup Send");
    return output.str();
}

string playCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Play Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Play Send");
    return output.str();
}

string pauseCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Pause Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Pause Send");
    return output.str();
}

string teardownCommand::send()
{
    RTSPStringStream output;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Entering Teardown Send");
    output << this;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", output.str().c_str());
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Exiting Teardown Send");
    return output.str();
}
