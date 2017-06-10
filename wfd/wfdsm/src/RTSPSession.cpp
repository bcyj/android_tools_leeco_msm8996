/*==============================================================================
*        @file RTSPSession.cpp
*
*  @par DESCRIPTION:
*        RTSPSession class.
*
*
*  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_NDEBUG  0
#define LOG_NDDEBUG 0
#ifndef WFD_ICS
#include <common_log.h>
#endif
#include <utils/Log.h>
#include "SessionManager.h"
#include "RTSPSession.h"
#include "WFDSession.h"
#include "wifidisplay.h"
#include <pthread.h>
#include <vector>
#include "rtsp_wfd.h"
#include "MMAdaptor.h"
#include "UIBCAdaptor.h"
#include "MMCapability.h"
#include "MMTimer.h"
#include <threads.h>
#include "wfd_netutils.h"
#include "wdsm_mm_interface.h"
#include "wfd_cfg_parser.h"
#ifdef HDCP_DISPLAY_ENABLED
#include "hdcpmgr_api.h"
#endif
#define WFD_MM_RTSP_THREAD_PRIORITY -19

extern vector<int> cfgItems;

using namespace std;

#define WFD_SRC_RTSP_KEEPALIVE_INTERVAL 50000

MMEventStatusType bHDCPStatus = MM_STATUS_INVALID;

cback::cback(RTSPSession* pRtspSess)
{
    pRtspSession = pRtspSess;
    m_hTimer = NULL;
}


void printMesg(rtspApiMesg &mesg)
{
   switch(mesg.error) {
    case noError:
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Success");
    break;
    case badStateError:
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Error: bad state");
    break;
    case timeoutError:
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Error: timeout");
    break;
    case remoteError:
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Error: remote error");
    break;
    default:
    break;
    }

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Session: %d", mesg.session);
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Port0: %d", mesg.rtpPort0);
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Port1: %d", mesg.rtpPort1);

    if (!mesg.ipAddr.empty()) {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"IP: %s", mesg.ipAddr.c_str());
    }
}

void cback::finishCallback()
{
   MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: finishCallback");
   if(m_hTimer)
   {
       int ret = MM_Timer_Release(m_hTimer);
       LOGI("Keep alive timer release returned %d",ret);
       m_hTimer = NULL;
   }
   if(pRtspSession != NULL && pRtspSession->rtspState != STOPPED)
   {
      /* destroy MM/UIBC session */
     MMAdaptor::destroySession();
     UIBCAdaptor::destroySession();
     MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"finishCallback: rtspState = %d", pRtspSession->rtspState);
     pRtspSession->rtspStateTransition(STOPPED);
   }
}
/*
 * Used to overwrite the WFD parameters before
 * RTSP begins negotiation
 */
void cback::openCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: openCallback");
    printMesg(mesg);
    mesg.wfd.dump();
    /*
     * Modify local wfd parameters in RTSP lib
     */
    #define NUMLOCALPARAMS 11
    string params[NUMLOCALPARAMS] = {
        "wfd_audio_codecs",
        "wfd_video_formats",
        "wfd_3d_video_formats",
        "wfd_content_protection",
        "wfd_display_edid",
        "wfd_coupled_sink",
        //"wfd_client_rtp_ports",
        "wfd_uibc_capability",
        "wfd_I2C",
        "wfd_connector_type",
        "wfd_presentation_URL",
        "wfd_standby_resume_capability"
    };
    SessionManager *pSM = SessionManager::Instance();
    if((pSM == NULL) || (pSM->pMyDevice == NULL) || (pSM->pMyDevice->pMMCapability == NULL))
        return;

    /* HDCP content protection is enabled in the config file, then check if the connected device supports it or not,
        if dosen't support then update the local capability structure wtih this information so that while capability
        negotiation Sink won't publish HDCP content protection.
       */
    if( mesg.wfd.contentProtection.getValid() == TRUE) {
#ifdef HDCP_DISPLAY_ENABLED
       if(HDCP1X_COMM_hdmi_status())
       {
         MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession  : HDMI is connected ");
         if((PRIMARY_SINK == pSM->pMyDevice->getDeviceType()) ||
            (SECONDARY_SINK == pSM->pMyDevice->getDeviceType())) {
            if (MM_STATUS_NOTSUPPORTED ==
                 pRtspSession->updateHdcpSupportedInConnectedDevice(
                       pSM->pMyDevice->pMMCapability->pCapability))
            {
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Setting CP to FALSE in RTSPSession OpenCB");
              /*If UnAuthorised connection then no need to enable CP */
              mesg.wfd.contentProtection.setValid(FALSE);
            }
          }
       }
#endif
    }

    char buffer[1024];
    for (int i=0; i<NUMLOCALPARAMS; i++) {
        memset(buffer, 0, sizeof(buffer));
        strlcat(buffer, params[i].c_str(), sizeof(buffer));
        strlcat(buffer, ": ", sizeof(buffer));
        strlcat(buffer, pSM->pMyDevice->pMMCapability->getKeyValue((char*)params[i].c_str()), sizeof(buffer));
        params[i] = buffer;
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Modify local wfd in rtsp lib....  %s", buffer);
    }
    if(!pSM->pMyDevice->pMMCapability->pUibcCapability->port_id &&
        SOURCE == pSM->pMyDevice->getDeviceType())
    {
      mesg.wfd.uibcCap.setValid(false);
    }
    rtspWfdParams type;
    bool isParamSet;
    for (int i=0; i<NUMLOCALPARAMS; i++) {
        isParamSet = false;
        if ((type=mesg.wfd.wfdType(params[i], isParamSet)) != wfd_invalid) {
            if (isParamSet) {
                mesg.wfd.wfdParse(type, params[i]);
            }
        }
    }

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Configured RTSP wfd mesg in Open callback:");

    if(SOURCE != pSM->pMyDevice->getDeviceType())
    {
       mesg.wfd.client.setRtpPort0(
       pSM->pMyDevice->pMMCapability->pCapability->transport_capability_config.port1_id);
    }

    if (SOURCE == pSM->pMyDevice->getDeviceType())
    {
        if (pRtspSession->pPeerDevice->decoderLatency)
        {
            mesg.wfd.buffLen.setBufferLen(pRtspSession->pPeerDevice->decoderLatency);
        }
        mesg.wfd.tcpWindowSize.setWindowSize(0);
    }
    else if(PRIMARY_SINK == pSM->pMyDevice->getDeviceType())
    {
        mesg.wfd.tcpWindowSize.setValid(true);
        mesg.wfd.tcpWindowSize.setWindowSize(0);
    }

    mesg.wfd.dump();

    /* record sessionId */
    pRtspSession->rtspSessionId = mesg.session;

    /* record peer IP address */
    if (!mesg.ipAddr.empty()) {
        pRtspSession->pPeerDevice->ipAddr = mesg.ipAddr;
    }

    /* record Local IP if not done already */
    if (pSM->pMyDevice->ipAddr.empty()) {
       char ip[20];
       int ret = getLocalIpSocket (mesg.session, ip);
       if (ret == 0)
          pSM->pMyDevice->ipAddr = ip;
    }

    pRtspSession->rtspStateTransition(CAP_NEGOTIATING);
    if( mesg.wfd.contentProtection.getValid() == TRUE) {
        if( PRIMARY_SINK == pSM->pMyDevice->getDeviceType() || SECONDARY_SINK == pSM->pMyDevice->getDeviceType()) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: createHDCPSession()");
            MMAdaptor::createHDCPSession(pSM->pMyDevice->getDeviceType(),
                                         pSM->pMyDevice->pMMCapability,
                                         pRtspSession->pPeerDevice->pMMCapability,
                                         pRtspSession->pPeerDevice->pNegotiatedMMCapability);
      }
    }
}

void cback::getCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: getCallback");
    printMesg(mesg);
    mesg.wfd.dump();

    if (mesg.error != noError && mesg.error != pendingCmdError) {
        return;
    }
    SessionManager *pSM = SessionManager::Instance();

    if((pSM == NULL) || (pSM->pMyDevice == NULL))
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something's Missing! Failed to get SessionManager instance");
        return;
    }
    /*If get parameter was not sent due to an already pending command, try resending*/
    if (mesg.error == pendingCmdError && pSM->pMyDevice->getDeviceType() == SOURCE) {
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Pending cmd error, send keep alive again");
       pRtspSession->sendWFDKeepAliveMsg();
       return;
    }

    if (SOURCE == pSM->pMyDevice->getDeviceType()) {
        /*
         * Record wfd parameters of peer device
         */
        pRtspSession->pPeerDevice->pMMCapability->configure(mesg.wfd);

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Peer MMCapability dump:");
        pRtspSession->pPeerDevice->pMMCapability->dump();

        if (mesg.wfd.tcpWindowSize.getValid())
        {
          pRtspSession->m_bTCPSupportedAtSink = true;
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Supported at Sink");
          if (pRtspSession->m_bTCPSupportStatusRequested) {
            if (pRtspSession->m_bTCPSupportedAtSink) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Client waiting for TCP support status");
                eventMMUpdate(MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK,MM_STATUS_SUCCESS,
                  0,0,0,0);
            }else{
                eventMMUpdate(MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK,MM_STATUS_FAIL,
                  0,0,0,0);
            }
            pRtspSession->m_bTCPSupportStatusRequested = false;
          }
        }
        pRtspSession->m_bTCPSupportQueried = true;
    }
    else
    {
        /*
         *   If Peer has no HDCP capability reset local capability
         */

        if (mesg.wfd.contentProtection.getValid() == false) {
            pSM->pMyDevice->pMMCapability->pCapability->content_protection_config.
                               content_protection_ake_port = 0;
            pSM->pMyDevice->pMMCapability->pCapability->content_protection_config.
                               content_protection_capability = 0;
        }
    }
}

/*
 * Used to overwrite the WFD parameters before
 * RTSP begins negotiation
 */
void cback::intersectCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: intersectCallback");
    printMesg(mesg);
    mesg.wfd.dump();
    SessionManager *pSM = SessionManager::Instance();
    if((pSM == NULL) || (pSM->pMyDevice == NULL)  || (pSM->pMyDevice->pMMCapability == NULL))
        return;

    /*
     * Modify negotiated wfd parameters by querying the negotiated result from MM lib
     */

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Get Negotiated MMCapability from MM lib");
    pRtspSession->pPeerDevice->pMMCapability->pCapability->peer_ip_addrs.ipv4_addr1 =
                                 inet_addr(pRtspSession->pPeerDevice->ipAddr.c_str());
    MMAdaptor::getNegotiatedCapability(pSM->pMyDevice->pMMCapability,
                                       pRtspSession->pPeerDevice->pMMCapability,
                                       pRtspSession->pPeerDevice->pNegotiatedMMCapability,
                                       pRtspSession->pPeerDevice->pCommonCapability);
    UIBCAdaptor::getNegotiatedCapability(pSM->pMyDevice->pMMCapability->pUibcCapability,
                                              pRtspSession->pPeerDevice->pMMCapability->pUibcCapability,
                                              pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability);
    //UIBC Capability does not have port param called.
    pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->ipv4_addr =
        inet_addr(pRtspSession->pPeerDevice->ipAddr.c_str());
    if(pSM->pMyDevice->pMMCapability->pUibcCapability != NULL)
    {
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->port_id =
            pSM->pMyDevice->pMMCapability->pUibcCapability->port_id;
    }
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"UIBC capability port = %d",
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->port_id);

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Local MMCapability:");
    pSM->pMyDevice->pMMCapability->dump();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Peer MMCapability:");
    pRtspSession->pPeerDevice->pMMCapability->dump();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Negotiated MMCapability:");
    pRtspSession->pPeerDevice->pNegotiatedMMCapability->dump();

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"createHDCPSession()");
    if( SOURCE == pSM->pMyDevice->getDeviceType() &&
        ( pRtspSession->pPeerDevice->pNegotiatedMMCapability->isHDCPVersionSupported(
            pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_capability
            )))
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"createHDCPSession(): SOURCE");
        MMAdaptor::createHDCPSession(pSM->pMyDevice->getDeviceType(),
                                     pSM->pMyDevice->pMMCapability,
                                     pRtspSession->pPeerDevice->pMMCapability,
                                     pRtspSession->pPeerDevice->pNegotiatedMMCapability);
    }

    #define NUMNEGPARAMS 10

    string params[NUMNEGPARAMS] = {
        "wfd_audio_codecs",
        "wfd_video_formats",
        "wfd_3d_video_formats",
        "wfd_content_protection",
        "wfd_display_edid",
        "wfd_coupled_sink",
        //"wfd_client_rtp_ports",
        "wfd_I2C",
        "wfd_uibc_capability",
        "wfd_connector_type"//,
        "wfd_standby_resume_capability"
    };
    mesg.wfd.standbyCap.setValid(false);

    //Check if A/V mode is set, otherwise wipe it out
    if (RTSPSession::m_eplayMode == AUDIO_ONLY)  {
      mesg.wfd.h264Cbp.setValid(false);
      mesg.wfd.h264Chp.setValid(false);
      mesg.wfd.h264Chi444p.setValid(false);
      mesg.wfd.videoHeader.setValid(false);

    }
    else if (RTSPSession::m_eplayMode == VIDEO_ONLY) {
       mesg.wfd.audioLpcm.setValid(false);
       mesg.wfd.audioAac.setValid(false);
       mesg.wfd.audioEac.setValid(false);
       mesg.wfd.audioDts.setValid(false);
       mesg.wfd.audioAc3.setValid(false);
    }
    else if (RTSPSession::m_eplayMode == AUDIO_VIDEO)  {
      mesg.wfd.h264Cbp.setValid(false);
      mesg.wfd.h264Chp.setValid(false);
      mesg.wfd.h264Chi444p.setValid(false);
    }
    // Hack to wipe out other audio codecs other than the negotiated one
    switch (pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_method) {
    case WFD_AUDIO_LPCM:
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec));
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec));
        mesg.wfd.audioAac.setValid(false);
        mesg.wfd.audioEac.setValid(false);
        mesg.wfd.audioDts.setValid(false);
        mesg.wfd.audioAc3.setValid(false);
        break;
    case WFD_AUDIO_AAC:
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec));
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec));
        mesg.wfd.audioLpcm.setValid(false);
        mesg.wfd.audioEac.setValid(false);
        mesg.wfd.audioDts.setValid(false);
        mesg.wfd.audioAc3.setValid(false);
        break;
    case WFD_AUDIO_DOLBY_DIGITAL:
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec));
        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec));
        mesg.wfd.audioAac.setValid(false);
        mesg.wfd.audioEac.setValid(false);
        mesg.wfd.audioDts.setValid(false);
        mesg.wfd.audioLpcm.setValid(false);
        break;
    default:
        break;
    }

    char buffer[500];
    rtspWfdParams type;
    bool isParamSet;
    for (int i=0; i<NUMNEGPARAMS; i++) {
        memset(buffer, 0, sizeof(buffer));
        strlcat(buffer, params[i].c_str(), sizeof(buffer));
        strlcat(buffer, ": ", sizeof(buffer));
        strlcat(buffer, pRtspSession->pPeerDevice->pNegotiatedMMCapability->getKeyValue((char*)params[i].c_str()), sizeof(buffer));
        params[i] = buffer;

        isParamSet = false;
        if ((type=mesg.wfd.wfdType(params[i], isParamSet)) != wfd_invalid) {
            if ((RTSPSession::m_eplayMode == VIDEO_ONLY && type == wfd_audio_codecs) ||
                (RTSPSession::m_eplayMode == AUDIO_ONLY && type == wfd_video_formats) ||
                (RTSPSession::m_eplayMode == AUDIO_ONLY && type == wfd_3d_video_formats))
              isParamSet = false;
            if(mesg.wfd.uibcCap.getValid()) {
               //parse for M14 support only if UIBC is supported in the first place
               pRtspSession->m_bUIBCSupported = true;
               if (type == wfd_uibc_capability) {
                   int m14Support = 0;
                   getCfgItem(UIBC_M14_KEY,&m14Support);
                   if(m14Support) {
                     //Since M14 suppport is enabled, don't set UIBC capabilties in M4
                     mesg.wfd.uibcCap.setValid(false);//Invalidate UIBC cap from RTSP message
                     isParamSet = false; //Do not add wfd_uibc in M4 message
                   }
               }
            }
            if (isParamSet) {
                mesg.wfd.wfdParse(type, params[i]);
            }
        }
    }
    if(pRtspSession->m_bUIBCSupported == true && (SOURCE == pSM->pMyDevice->getDeviceType()))
    { //Start server on source irrespective of M4/M14 to be able to accept connection at any point
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Start UIBC Server on source");
       MMCapability tempCap;
       int32 negHeight =0, negWidth = 0;
       //TODO for source we can perhaps move the configure altogether here itself
       tempCap.configure(mesg.wfd);
       if(tempCap.pCapability)
       {
         negHeight = tempCap.pCapability->video_config.\
                    video_config.h264_codec[0].max_vres;
         negWidth = tempCap.pCapability->video_config.\
                    video_config.h264_codec[0].max_hres;
       }
       MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession_CPP ::UIBC negotiated_height %d negotiated_width %d",\
                                                negHeight, negWidth );
       pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->\
                                  negotiated_height = negHeight;
       pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->\
                                  negotiated_width = negWidth;
       UIBCAdaptor::createSession(pRtspSession->pPeerDevice->\
                                  pNegotiatedMMCapability->pUibcCapability);
    }
    if (pRtspSession->m_bTCPSupportedAtSink && pRtspSession->pPeerDevice->decoderLatency)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Client requested decoder latency for Sink. Send in SET_PARAMETER");
        mesg.wfd.buffLen.setBufferLen(pRtspSession->pPeerDevice->decoderLatency);
    }
    else
    {
        mesg.wfd.buffLen.setValid(false);
    }
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Configured rtsp mesg:");
    mesg.wfd.dump();

}

void cback::setCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: setCallback");
    printMesg(mesg);
    mesg.wfd.dump();

    if (mesg.error != noError) {

        if(mesg.wfd.tcpStreamControl.getValid())
        {
            rtsp_wfd::tcp_control_command eCmd = mesg.wfd.tcpStreamControl.getCommand();

            MMEventType eEvent = NO_EVENT;
            switch(eCmd) {
            case rtsp_wfd::flush:
                eEvent = BUFFERING_CONTROL_EVENT_FLUSH;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event FLush Fail");
                break;
            case rtsp_wfd::pause:
                eEvent = BUFFERING_CONTROL_EVENT_PAUSE;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event Pause Fail");
                break;
            case rtsp_wfd::play:
                eEvent = BUFFERING_CONTROL_EVENT_PLAY;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event Play Fail");
                break;
            case rtsp_wfd::status:
                eEvent = BUFFERING_CONTROL_EVENT_STATUS;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event Status Fail");
                break;
            default:
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event invalid event");
                break;
            }

            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Stream control failed");
            eventMMUpdate(eEvent,MM_STATUS_FAIL, 0,0,0,0);
            return;
        }
#ifdef TCP_LEGACY
        if(mesg.wfd.buffLen.getValid())
        {
            if(!mesg.wfd.tcpWindowSize.getValid())
            {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Buffering negotiation fail");
                eventMMUpdate(BUFFERING_NEGOTIATION,MM_STATUS_FAIL,
                      0,0,0,0);
            }
        }
#else
        if (pRtspSession->rtspState == SESS_ESTABLISHED)
        {
            if(mesg.wfd.client.getValid())
            {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Buffering negotiation fail");
                eventMMUpdate(BUFFERING_NEGOTIATION,MM_STATUS_FAIL,
                      0,0,0,0);
            }
            if (mesg.wfd.buffLen.getValid())
            {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Set decoder latency fail");
                eventMMUpdate(BUFFERING_CONTROL_EVENT_DECODER_LATENCY,MM_STATUS_FAIL,
                      0,0,0,0);
            }
        }
#endif

        return;
    }

    SessionManager *pSM = SessionManager::Instance();
    if((pSM == NULL) || (pSM->pMyDevice == NULL)) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something amiss!! SessionManager instance is null!!");
        return;
    }


    /*When SINK receives a SET PARAMETER with UIBC valid (either in M4/M14) go ahead and
          set SINK's parameters, spawn UIBC threads and attempt to connect to source [the server]
          Nothing to do for source since it should be already up at this point to accept connections*/
    if (mesg.wfd.uibcCap.getValid() && (SOURCE != pSM->pMyDevice->getDeviceType()))
    {
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->configure(mesg.wfd);
        pRtspSession->m_bUIBCSupported = true;
        //Populate UIBC Capabilities
        memcpy(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability,
                       pSM->pMyDevice->pMMCapability->pUibcCapability,
                       sizeof(*pSM->pMyDevice->pMMCapability->pUibcCapability));
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->ipv4_addr =
            inet_addr(pRtspSession->pPeerDevice->ipAddr.c_str());
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"UIBC IP IN network order %d",
            pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->ipv4_addr);
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->port_id =
            mesg.wfd.uibcCap.getPort();
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"UIBC port from SM %d",mesg.wfd.uibcCap.getPort());
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->negotiated_height =
            pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->\
                                       video_config.video_config.h264_codec[0].max_vres;
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->negotiated_width=
            pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->\
                                       video_config.video_config.h264_codec[0].max_hres;
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession_CPP :: UIBC negotiated_height %u negotiated_width %u",
           pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->negotiated_height,
           pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability->negotiated_width);
        UIBCAdaptor::createSession(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pUibcCapability);
    }

    if (pRtspSession->rtspState == CAP_NEGOTIATING) {
        /*
         * Configure negotiated MM capability
         */

        pRtspSession->pPeerDevice->pNegotiatedMMCapability->configure(mesg.wfd);
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Dump final negotiated MMCapability:");
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->dump();
        pRtspSession->rtspStateTransition(CAP_NEGOTIATED);
        pRtspSession->rtspStateTransition(SESS_ESTABLISHING);

        if(pRtspSession->pPeerDevice->getDeviceType() == SOURCE ) {
            if (mesg.wfd.standbyCap.getValid()){
                pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->standby_resume_support= true;
            }
            if(mesg.wfd.buffLen.getValid()) {
                pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->decoder_latency
                          = mesg.wfd.buffLen.getBufferLen();
            }
        }
    }
    else if (pRtspSession->rtspState == SESS_ESTABLISHED)
    {
        WFD_MM_capability_t *pMMCfg = pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability;
        if (SOURCE != pSM->pMyDevice->getDeviceType() && mesg.wfd.client.getValid())
        {
            if (pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->transport_capability_config.eRtpPortType == RTP_PORT_UDP)
            {
                if (mesg.wfd.client.getTCP())
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Switch to TCP rquested from source");
                    MMAdaptor::streamPause();
                    getIPSockPair(true, &pMMCfg->transport_capability_config.rtpSock,
                        &pMMCfg->transport_capability_config.rtcpSock,
                        (int*)(&pMMCfg->transport_capability_config.port1_id),
                        (int*)(&pMMCfg->transport_capability_config.port1_rtcp_id), true );
                    close(pMMCfg->transport_capability_config.rtpSock);
                    close(pMMCfg->transport_capability_config.rtcpSock);
                    pMMCfg->transport_capability_config.rtpSock = -1;
                    pMMCfg->transport_capability_config.rtcpSock = -1;
                    pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->transport_capability_config.eRtpPortType = RTP_PORT_TCP;
                    MMAdaptor::updateSession(pRtspSession->pPeerDevice->pNegotiatedMMCapability);
                    MMAdaptor::streamPlay();
                    if (mesg.wfd.tcpWindowSize.getValid())
                    {
                      MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK, AV_CONTROL_SET_DECODER_LATENCY,
                                                (uint64)mesg.wfd.tcpWindowSize.getWindowSize());
                    }

                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"disable UIBC events in TCP mode ");
                    UIBCAdaptor::handleUIBCEvent(false);
                }
                else
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Switch to TCP requested when in TCP. Ignore");
                }
            }
            else
            {
                if (!mesg.wfd.client.getTCP())
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Switch to UDP rquested from source");
                    MMAdaptor::streamPause();
                    getIPSockPair(true, &pMMCfg->transport_capability_config.rtpSock,
                        &pMMCfg->transport_capability_config.rtcpSock,
                        (int*)(&pMMCfg->transport_capability_config.port1_id),
                        (int*)(&pMMCfg->transport_capability_config.port1_rtcp_id), false );
                    close(pMMCfg->transport_capability_config.rtpSock);
                    close(pMMCfg->transport_capability_config.rtcpSock);
                    pMMCfg->transport_capability_config.rtpSock = -1;
                    pMMCfg->transport_capability_config.rtcpSock = -1;
                    pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->transport_capability_config.eRtpPortType = RTP_PORT_UDP;
                    MMAdaptor::updateSession(pRtspSession->pPeerDevice->pNegotiatedMMCapability);
                    MMAdaptor::streamPlay();
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"enable UIBC events in UDP mode ");
                    UIBCAdaptor::handleUIBCEvent(true);
                }
                else
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Switch to UDP requested when in UDP. Ignore");
                }
            }
            mesg.wfd.client.setRtpPort0(pMMCfg->transport_capability_config.port1_id);
            if (mesg.wfd.client.getRtcpPort0())
            {
                mesg.wfd.client.setRtcpPort0(pMMCfg->transport_capability_config.port1_rtcp_id);
            }
        }
        else if (mesg.wfd.client.getValid())
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Received new port from Sink. Update it");
            pMMCfg->transport_capability_config.port1_id = mesg.wfd.client.getRtpPort0();
            pMMCfg->transport_capability_config.port1_rtcp_id = mesg.wfd.client.getRtcpPort0();
            pMMCfg->transport_capability_config.eRtpPortType = mesg.wfd.client.getTCP()?RTP_PORT_TCP:RTP_PORT_UDP;
            eventMMUpdate(BUFFERING_NEGOTIATION,MM_STATUS_SUCCESS,
                          mesg.wfd.client.getTCP()? TRANSPORT_TCP: TRANSPORT_UDP,
                          mesg.wfd.client.getTCP()? TRANSPORT_UDP: TRANSPORT_TCP,
                          0,0);
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Buffering negotiation success");
        }

        if (mesg.wfd.buffLen.getValid())
        {
            if (SOURCE != pSM->pMyDevice->getDeviceType())
            {
                MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK, AV_CONTROL_SET_DECODER_LATENCY,
                                                (uint64)mesg.wfd.buffLen.getBufferLen());
            }
            else
            {
                eventMMUpdate(BUFFERING_CONTROL_EVENT_DECODER_LATENCY,MM_STATUS_SUCCESS,0,0,0,0);
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Set decoder latency success");
            }
        }
    }

    /* When source receives wfd_idr request, call MM lib to send idr frame */
    if ( mesg.wfd.idrReq.getValid() && (pSM->pMyDevice->getDeviceType() == SOURCE) &&
         RTSPSession::m_eplayMode != AUDIO_ONLY ) {
        MMAdaptor::sendIDRFrame();
    }



    /* Notify UIBCManager (in SessionManagerA) of UIBC enable/disable event */
    if (mesg.wfd.uibcSet.getValid()) {
        if (mesg.wfd.uibcSet.getSetting()) {
            // send UIBC enable event
            eventUIBCEnabled(pRtspSession->rtspSessionId);
        } else {
            // send UIBC disable event
            eventUIBCDisabled(pRtspSession->rtspSessionId);
        }
    }

    /* Capability renegotiation request*/
    if (mesg.wfd.timing.getValid()) {
        // Currently we support only video frame rate change.
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->configure(mesg.wfd);
        MMAdaptor::updateSession(pRtspSession->pPeerDevice->pNegotiatedMMCapability);
    }

    /* STANDBY request */
    if (mesg.wfd.halt.getValid()) {
        pRtspSession->rtspStateTransition(STANDBY);
        pauseCallback(mesg);
    }

    //B3
    MMEventStatusType status = MM_STATUS_SUCCESS;

    if(pSM->pMyDevice->getDeviceType() == SOURCE)
    {
        if(mesg.wfd.tcpStreamControl.getValid()) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event");
            rtsp_wfd::tcp_control_command eCmd;

            eCmd = mesg.wfd.tcpStreamControl.getCommand();

            MMEventType eEvent = NO_EVENT;
            switch(eCmd) {
            case rtsp_wfd::flush:
                eEvent = BUFFERING_CONTROL_EVENT_FLUSH;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event FLush Success");
                break;
            case rtsp_wfd::pause:
                eEvent = BUFFERING_CONTROL_EVENT_PAUSE;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event Pause Success");
                break;
            case rtsp_wfd::play:
                eEvent = BUFFERING_CONTROL_EVENT_PLAY;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event Play success");
                break;
            case rtsp_wfd::status:
                eEvent = BUFFERING_CONTROL_EVENT_STATUS;
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event Status success");
                break;
            default:
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"TCP Buffer Control Event invalid event");
                break;
            }

            int BuffLen = 0;
            int WindowSize = 0;

            if(mesg.wfd.tcpWindowSize.getValid()) {
                WindowSize = mesg.wfd.tcpWindowSize.getWindowSize();
            }

            if(mesg.wfd.buffLen.getValid()) {
                BuffLen = mesg.wfd.buffLen.getBufferLen();
            }
            MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event windowSize = %d, BuffLen = %d",
                 WindowSize, BuffLen);
            eventMMUpdate(eEvent,status, WindowSize, BuffLen,0,0);
        }
#ifdef LEGACY_TCP
        if(mesg.wfd.tcpWindowSize.getValid()) {
            eventMMUpdate(BUFFERING_STATUS_UPDATE,status,
                          mesg.wfd.buffLen.getBufferLen(),
                           mesg.wfd.tcpWindowSize.getWindowSize(),0,0);
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Buffering update");
        }

        if(mesg.wfd.buffLen.getValid()) {

            unsigned int portNum = mesg.wfd.client.getRtpPort0();
            if(portNum > 65536) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid port num in response");
                eventMMUpdate(BUFFERING_NEGOTIATION,MM_STATUS_FAIL,
                          0,0,0,0);
                return;
            }
            pRtspSession->pPeerDevice->pNegotiatedMMCapability->
                pCapability->transport_capability_config.port1_id = mesg.wfd.client.getRtpPort0();
            eventMMUpdate(BUFFERING_NEGOTIATION,MM_STATUS_SUCCESS,
                          0,0,0,0);
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Buffering negotiation success");
        }
#endif
    } else {
        if(mesg.wfd.tcpStreamControl.getValid()) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"TCP Buffer Control Event");
            rtsp_wfd::tcp_control_command eCmd;

            eCmd = mesg.wfd.tcpStreamControl.getCommand();

            if (eCmd == rtsp_wfd::flush)
            {
                MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK, AV_CONTROL_FLUSH,
                                  (uint64)mesg.wfd.tcpStreamControl.getDuration());
            }
        }
    }

}

void cback::setupCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: setupCallback");
    printMesg(mesg);
    mesg.wfd.dump();

    SessionManager *pSM = SessionManager::Instance();
    if((pSM == NULL) || (pSM->pMyDevice == NULL) || (pSM->pMyDevice->pMMCapability == NULL))
        return;

    if (mesg.error != noError) {
        return;
    }

    /* If Audio Only or video only mode is enabled, update the audio/video
     * method to INVALID (since it is NON zero and needs to be set explicitly
     */
    if (RTSPSession::m_eplayMode == AUDIO_ONLY) {
      pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_method = WFD_VIDEO_INVALID;
    mesg.wfd.videoHeader.setValid(false);
    mesg.wfd.h264Cbp.setValid(false);
    mesg.wfd.h264Chi444p.setValid(false);
    mesg.wfd.h264Chp.setValid(false);
    }
    else if (RTSPSession::m_eplayMode == VIDEO_ONLY) {
      pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_method = WFD_AUDIO_INVALID;
    mesg.wfd.audioAac.setValid(false);
    mesg.wfd.audioDts.setValid(false);
    mesg.wfd.audioEac.setValid(false);
    mesg.wfd.audioLpcm.setValid(false);
    mesg.wfd.audioAc3.setValid(false);
    }

    MMCapability* pNegotiatedMMCapability = pRtspSession->pPeerDevice->pNegotiatedMMCapability;

    /* Configure transport parameters */
    pNegotiatedMMCapability->pCapability->transport_capability_config.port1_id = mesg.rtpPort0;
    pNegotiatedMMCapability->pCapability->transport_capability_config.port2_id = mesg.rtpPort1;

    if(mesg.wfd.client.getRtcpPort0()) {
       pNegotiatedMMCapability->pCapability->transport_capability_config.port1_rtcp_id = mesg.wfd.client.getRtcpPort0();
    }
    pNegotiatedMMCapability->pCapability->peer_ip_addrs.ipv4_addr1=
             inet_addr(pRtspSession->pPeerDevice->ipAddr.c_str());
    strlcpy((char*)pNegotiatedMMCapability->pCapability->peer_ip_addrs.device_addr1,
                    pRtspSession->pPeerDevice->macAddr.c_str(),
                     sizeof(pNegotiatedMMCapability->pCapability->peer_ip_addrs.device_addr1));


    if(SOURCE != pSM->pMyDevice->getDeviceType())
    {
      if(pSM->pMyDevice->pMMCapability->pCapability != NULL)
      {
          pNegotiatedMMCapability->pCapability->pSurface = NULL;
          pNegotiatedMMCapability->pCapability->
                          content_protection_config.content_protection_ake_port
                = pSM->pMyDevice->pMMCapability->pCapability->content_protection_config.
                           content_protection_ake_port;
          pNegotiatedMMCapability->pCapability->content_protection_config.
                           content_protection_capability
                = pSM->pMyDevice->pMMCapability->pCapability->
                                content_protection_config.content_protection_capability;
      }
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Updating local sockets");
      pNegotiatedMMCapability->pCapability->transport_capability_config.rtpSock =
           pSM->pMyDevice->pMMCapability->pCapability->transport_capability_config.rtpSock;
      pNegotiatedMMCapability->pCapability->transport_capability_config.rtcpSock =
           pSM->pMyDevice->pMMCapability->pCapability->transport_capability_config.rtcpSock;
    }

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Dump MM capability used for mm_create_session():");
    pRtspSession->pPeerDevice->pNegotiatedMMCapability->dump();

    MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_HIGH,"Create MM session with peerDevice:  MacAddr=%s  IP=%s  port=%d", pRtspSession->pPeerDevice->macAddr.c_str(), pRtspSession->pPeerDevice->ipAddr.c_str(), pNegotiatedMMCapability->pCapability->transport_capability_config.port1_id);

    /* Configure content protection parameters */
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"RTSP setupCallback:negotiated capability:HDCP port %d,version %d",
                                        pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_ake_port,
                                        ((pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_capability == 0) ?
                                         pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_capability :
                                        (pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_capability - 1)));

    /* If link protection is enforeced, when SOURCE supports HDCP and SINK does not supports HDCP,
       the session should not be established. */
    int nHDCPEnforced = 0;
    getCfgItem(HDCP_ENFORCED_KEY, &nHDCPEnforced);
    if (nHDCPEnforced && SOURCE == pSM->pMyDevice->getDeviceType() &&
          ( pSM->pMyDevice->pMMCapability->isHDCPVersionSupported(
              pSM->pMyDevice->pMMCapability->pCapability->content_protection_config.content_protection_capability
           )))
    {
       if (!(pNegotiatedMMCapability->isHDCPVersionSupported(
                pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_capability)) ||
                (pNegotiatedMMCapability->pCapability->content_protection_config.content_protection_ake_port == 0))
       {
          eventMMUpdate(HDCP_UNSUPPORTED_BY_PEER, MM_STATUS_FAIL, 0,0,0,0);
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"WFD Sink Doesn't Support HDCP. Teardown Session");
          mesg.error = badStateError;
          return;
       }
    }

    /*Notify upper layers that RTSP session has been established*/
    if (pRtspSession->rtspState == SESS_ESTABLISHING) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSP Session has been established");
        pRtspSession->rtspStateTransition(SESS_ESTABLISHED);
    }

    /* start MM session */
    MMAdaptor::createSession(pNegotiatedMMCapability, pSM->pMyDevice->getDeviceType());
    if(pSM->pMyDevice->getDeviceType() != SOURCE && getStreamingSurface() !=NULL)
    {
      pNegotiatedMMCapability->pCapability->pSurface = getStreamingSurface();
      MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Setting Video Surface %p",pNegotiatedMMCapability->pCapability->pSurface);
      MMAdaptor::updateSession(pNegotiatedMMCapability);
      MMAdaptor::streamPlay();
    }
    if(pRtspSession->m_sLocalTransportInfo.rtpPort) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Set RTP Source port num");
        mesg.wfd.server.setRtpPort0(pRtspSession->m_sLocalTransportInfo.rtpPort);
    }

    if(pRtspSession->m_sLocalTransportInfo.rtcpPort &&
       pNegotiatedMMCapability->pCapability->transport_capability_config.port1_rtcp_id) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Set RTCP Source Port Num");
        mesg.wfd.server.setRtcpPort0(pRtspSession->m_sLocalTransportInfo.rtcpPort);
    }

}

void cback::keepAliveTimerCallback(void *pMe)
{
   cback *pCback = (cback*)pMe;
   MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: keepAliveTimerCallback");
   pCback->pRtspSession->sendWFDKeepAliveMsg();
   return;
}

void cback::playCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: playCallback");
    printMesg(mesg);

    SessionManager *pSM = SessionManager::Instance();

    if (!pRtspSession || !pSM)
    {
        return;
    }

    if (mesg.error == noErrorPreSendCmdNotify) {
       SessionManager *pSM = SessionManager::Instance();
       if (pSM && pSM->pMyDevice &&
           (pSM->pMyDevice->getDeviceType() == PRIMARY_SINK ||
            pSM->pMyDevice->getDeviceType() == SECONDARY_SINK) &&
           pRtspSession->pPeerDevice->pNegotiatedMMCapability->
           pCapability->transport_capability_config.eRtpPortType != RTP_PORT_TCP) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Call prepare play to setup multimedia in sink");
          if(false == MMAdaptor::streamPlayPrepare())
          {
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"cback::playCallback : stream play prepare returns false, tearing down session");
              eventMMUpdate(MM_VIDEO_EVENT_FAILURE,MM_STATUS_RUNTIME_ERROR,0,0,0,0);
              return;
          }
          if ((pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_method == WFD_VIDEO_INVALID ||
                pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_method == WFD_VIDEO_UNK) &&
                (pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_method != WFD_AUDIO_INVALID &&
                pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_method != WFD_AUDIO_UNK))
            {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Notify Client about Audio Only Session");
                eventMMUpdate(MM_SESSION_AUDIO_ONLY,MM_STATUS_SUCCESS,0,0,0,0);
            }
       }
       return;
    }

    if (mesg.error != noError) {
        return;
    }

    if (pRtspSession->rtspState == STANDBY) {
        /*Upper layers needn't be notified of this internal state change*/
        pRtspSession->rtspStateTransition(SESS_ESTABLISHED,false);
    }

    if (pRtspSession->rtspState == SESS_ESTABLISHED) {
        eventStreamControlCompleted(PLAY, pRtspSession->rtspSessionId);
    }

    bool bRet = false;

    if (pSM && pSM->pMyDevice && pSM->pMyDevice->getDeviceType() != SOURCE &&
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->
         pCapability->transport_capability_config.eRtpPortType == RTP_PORT_TCP)
    {
        bRet = MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK,
                                        AV_CONTROL_PLAY,
                                        1);//1 is for flushing before play
        eventStreamControlCompleted(PLAY_DONE, pSM->getRTSPSessionId());
    }
    else
    {
        bRet = MMAdaptor::streamPlay();
    }
    if(false == bRet)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"cback::playCallback : stream play returns false, tearing down session");
        eventMMUpdate(MM_VIDEO_EVENT_FAILURE,MM_STATUS_RUNTIME_ERROR,0,0,0,0);
        return;
    }


    /* RTSP keepalive */
    if((pSM == NULL) || (pSM->pMyDevice == NULL))
      return;
    if (pSM && pSM->pMyDevice && pSM->pMyDevice->getDeviceType() == SOURCE) {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Creating timer for RTSP Keep Alive");
    if (m_hTimer == NULL) {
      MM_Timer_Create(WFD_SRC_RTSP_KEEPALIVE_INTERVAL,1,cback::keepAliveTimerCallback, (void*)this, &m_hTimer);
     }
    }
}

void cback::pauseCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: pauseCallback");

    SessionManager *pSM = SessionManager::Instance();

    if (!pRtspSession || !pSM)
    {
        return;
    }

    printMesg(mesg);

    if (mesg.error != noError) {
        return;
    }

    if (pRtspSession->rtspState == SESS_ESTABLISHED) {
        eventStreamControlCompleted(PAUSE, pRtspSession->rtspSessionId);
    }

    bool bRet = false;

    if (pSM && pSM->pMyDevice && pSM->pMyDevice->getDeviceType() != SOURCE &&
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->
         pCapability->transport_capability_config.eRtpPortType == RTP_PORT_TCP)
    {
        bRet = MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK,
                                        AV_CONTROL_PAUSE,
                                        0);
    }
    else
    {
        bRet = MMAdaptor::streamPause();
    }
    if(false == bRet)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"cback::pauseCallback : stream pause returns false, tearing down session");
        eventMMUpdate(MM_VIDEO_EVENT_FAILURE,MM_STATUS_RUNTIME_ERROR,0,0,0,0);
        return;
    }

    if (pSM && pSM->pMyDevice && pSM->pMyDevice->getDeviceType() != SOURCE &&
        pRtspSession->pPeerDevice->pNegotiatedMMCapability->
         pCapability->transport_capability_config.eRtpPortType == RTP_PORT_TCP)
    {
        eventStreamControlCompleted(PAUSE_DONE, pSM->getRTSPSessionId());
    }

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"rtspState = %d", pRtspSession->rtspState);
}

void cback::teardownCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: teardownCallback");
    printMesg(mesg);

    if (pRtspSession->rtspState == TEARING_DOWN)
      return;

    pRtspSession->rtspState = TEARING_DOWN;

    if(m_hTimer)
    {
        int ret = MM_Timer_Release(m_hTimer);
        LOGI("Keep alive timer release returned %d",ret);
        m_hTimer = NULL;
    }


        eventStreamControlCompleted(TEARDOWN, pRtspSession->rtspSessionId);
        /* destroy MM/UIBC session */
        MMAdaptor::destroySession();
        UIBCAdaptor::destroySession();
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"rtspState = %d", pRtspSession->rtspState);
        pRtspSession->stop();

}

void cback::closeCallback(rtspApiMesg &mesg)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Callback: closeCallback");
    printMesg(mesg);

    if (pRtspSession->rtspState == TEARING_DOWN)
      return;

    pRtspSession->rtspState = TEARING_DOWN;
    eventError("RTSPCloseCallback");
    if(m_hTimer) {
        int ret = MM_Timer_Release(m_hTimer);
        LOGI("Keep alive timer release returned %d",ret);
        m_hTimer = NULL;
    }
    /* destroy MM/UIBC session */
    MMAdaptor::destroySession();
    UIBCAdaptor::destroySession();

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"rtspState = %d", pRtspSession->rtspState);
    pRtspSession->stop();
}





AVPlaybackMode RTSPSession::m_eplayMode = AUDIO_VIDEO;


RTSPSession::RTSPSession(WFDSession* pWfdSession, Device* pDev) {

    server = NULL;
    client = NULL;
    pWFDSession = pWfdSession;
    pPeerDevice = pDev;
    rtspState = STOPPED;
    rtspSessionId = -1;
    memset(&m_sLocalTransportInfo, 0, sizeof(m_sLocalTransportInfo));
    m_bUIBCSupported = false;
    m_bTCPSupportedAtSink = false;
    m_bTCPSupportQueried = false;
    m_bTCPSupportStatusRequested = false;
    events = new cback(this);
}


RTSPSession::~RTSPSession() {
    delete events;
}



void* RTSPSession::rtspServerLoopFunc(void *s)
{
    rtspServer* server = (rtspServer*)s;
    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"WFDD: RTSP thread Priority before = %d",androidGetThreadPriority(tid));
    androidSetThreadPriority(0,WFD_MM_RTSP_THREAD_PRIORITY);
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"WFDD: RTSP thread Priority after = %d",androidGetThreadPriority(tid));

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Start rtspServer loop.");
    server->eventLoop();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Exit rtspServer loop.");
    delete server;
    return NULL;
}

void* RTSPSession::rtspClientLoopFunc(void *c)
{
    rtspClient* client = (rtspClient*)c;
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Start rtspClient loop.");
    client->eventLoop();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Exit rtspClient loop.");
    delete client;
    return NULL;
}



bool RTSPSession::startServer(string ipAddr, int rtspPort, int uibcPort) {

    if (server != NULL) {
        return false;
    }

    server = new rtspServer(ipAddr, events, "", rtspPort, uibcPort, rtsp_wfd::source);
    if (server->createServer() < 0) {
        return false;
    }

    pthread_create(&session_thread, NULL, &RTSPSession::rtspServerLoopFunc, (void *)server);
    rtspStateTransition(STARTING); //RTSP server has started at this point

    return true;
}

bool RTSPSession::startClient(string ipAddr, int rtpPort0, int rtpPort1, int rtspPort, int hdcpPort) {

    if (client != NULL) {
        return false;
    }

    client = new rtspClient(rtpPort0, rtpPort1, hdcpPort,events, "", rtspPort, rtsp_wfd::sink, "");
    if (client->startClient(ipAddr) < 0) {
        return false;
    }

    pthread_create(&session_thread, NULL, &RTSPSession::rtspClientLoopFunc, (void *)client);

    return true;
}

void RTSPSession::rtspStateTransition(RTSPState newState, bool notify) {
    if (rtspState != newState) {
        MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession state transition: %d --> %d  (sessionId=%d)", rtspState, newState, rtspSessionId);
        rtspState = newState;

        pWFDSession->updateSessionState(notify);
    }
}


void RTSPSession::play() {
    if (server != NULL) {
        server->Play(rtspSessionId);
    } else if (client != NULL) {
        client->Play(rtspSessionId);
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid RTSP session.");
    }
}

void RTSPSession::pause() {
    if (server != NULL) {
        server->Pause(rtspSessionId);
    } else if (client != NULL) {
        client->Pause(rtspSessionId);
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid RTSP session.");
    }
}

void RTSPSession::teardown() {
    int err = 0;
    if (server != NULL) {
        err = server->Teardown(rtspSessionId);
    } else if (client != NULL) {
        err = client->Teardown(rtspSessionId);
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid RTSP session.");
    }
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Teardown return Code = %d", err);

    if(err < 0) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Teardown Failed. Trying to Stop RTSP");
        stop();
    }
}

void RTSPSession::stop() {
    if (server != NULL) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Stopping rtspServer");
        server->Stop();
    } else if (client != NULL) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Stopping rtspClient");
        client->Stop();
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid RTSP session.");
    }
}

bool RTSPSession::enableUIBC(bool enabled) {
    if(m_bUIBCSupported) {
       rtspWfd myWfd;
       myWfd.uibcSet.setSetting(enabled);
       if (server != NULL) {
          server->Set(rtspSessionId, myWfd);
       } else if (client != NULL) {
          client->Set(rtspSessionId, myWfd);
       } else {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid RTSP session.");
          return false;
       }
     } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"UIBC is not supported for session");
        return false;
     }
     MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"enableUIBC %d successful ",enabled);
     return true;
}


bool RTSPSession::getNegotiatedResolution(int32_t* pWidth, int32_t* pHeight) {
    vector<WFDSession*>::iterator it;
    SessionManager *pSM = SessionManager::Instance();
    if(pSM && pWidth && pHeight) {
      for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
          WFDSession* wfdSession = *it;
          RTSPSession* rtspSession = wfdSession->getRtspSession();
          if (rtspSession != NULL) {
              (*pWidth) = rtspSession->pPeerDevice->pNegotiatedMMCapability->\
                pCapability->video_config.video_config.h264_codec[0].max_hres;
              (*pHeight) = rtspSession->pPeerDevice->pNegotiatedMMCapability->\
                pCapability->video_config.video_config.h264_codec[0].max_vres;
              return true;
          }
      }//end for loop
    }
    return false;
}

void RTSPSession::streamControl(int rtspSessId, StreamCtrlType cmdType) {
    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL && rtspSession->rtspSessionId == rtspSessId) {
            switch (cmdType) {
            case PLAY:
                rtspSession->play();
                break;
            case PAUSE:
                rtspSession->pause();
                break;
            case TEARDOWN:
                rtspSession->teardown();
                break;
            default:
                break;
            }
            break;
        }
    }
}


bool RTSPSession::uibcControl(int rtspSessId, bool enabled) {
    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL && rtspSession->rtspSessionId == rtspSessId) {
                 MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Valid RTSP session.");
            return rtspSession->enableUIBC(enabled);
        }
    }
    return false;
}

void RTSPSession::Cleanup()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSP Cleanup");

    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();
    RTSPSession *pRtspSession = NULL;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        if (it != NULL) {
           WFDSession* wfdSession = *it;
           if (wfdSession) {
              pRtspSession = wfdSession->getRtspSession();
           }

           if (pRtspSession != NULL) {
               MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Valid RTSP session.");
               break;
           }
        }
    }
    if (pRtspSession)
    {

        if (pRtspSession->rtspState == TEARING_DOWN)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Already tearing down");
            return;
        }

        pRtspSession->rtspState = TEARING_DOWN;

        /*Stop RTSP processing. MM/UIBC will be cleaned up later*/
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling stop on RTSP");
        pRtspSession->stop();

    }
}

bool RTSPSession::setUIBC(int rtspSessId) {
    UNUSED(rtspSessId);
    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();
    if(!pSM) {
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"SessionManager instance is null!!");
       return false;
    }
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* pRtspSession = wfdSession->getRtspSession();
        if (pRtspSession && pRtspSession->m_bUIBCSupported) {
            rtspWfd myWfd;
            myWfd.uibcCap.setValid(true);
            string uibcParam ("wfd_uibc_capability");
            if (pRtspSession->server != NULL) {
                rtspWfdParams type;
                bool isParamSet = false;
                uibcParam += string(": ") + string(pRtspSession->pPeerDevice->\
                    pNegotiatedMMCapability->getKeyValue((char*)uibcParam.c_str()));
                if ((type=myWfd.wfdType(uibcParam, isParamSet)) != wfd_invalid) {
                    if (isParamSet) {
                        myWfd.wfdParse(type, uibcParam);
                    } else {
                        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something fishy!! UIBC Param not found!!");
                        return false;
                    }
                } else {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something amiss!! UIBC parameter is invalid!!");
                    return false;
                }
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Configured RTSP M14 Message from Source");
                myWfd.dump();
                pRtspSession->server->Set(pRtspSession->rtspSessionId, myWfd);
            } else if (pRtspSession->client != NULL) {
                //pRtspSession->client->Set(pRtspSession->rtspSessionId, myWfd);
                //TODO Implement M14 for sink side
                return false;//For now
            }
        } else if (!pRtspSession){
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            return false;
        } else if (!(pRtspSession->m_bUIBCSupported)) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"UIBC not supported in current session");
            return false;
        }
        break;
    }
    return true;
}
bool RTSPSession::standby(int rtspSessId) {
    UNUSED(rtspSessId);
    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* pRtspSession = wfdSession->getRtspSession();
        if (pRtspSession != NULL) {
            rtspWfd myWfd;
            if (pRtspSession->server != NULL
                &&
                pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->standby_resume_support) {
                myWfd.halt.setValid(true);
                pRtspSession->server->Set(pRtspSession->rtspSessionId, myWfd);
            } else if (pRtspSession->client != NULL
                       &&
                       pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->standby_resume_support) {
                myWfd.halt.setValid(true);
                pRtspSession->client->Set(pRtspSession->rtspSessionId, myWfd);
            } else if (!(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->standby_resume_support)){
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No standby_resume_support ");
                return false;
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session in progress");
                return false;
            }
            break;
        }
    }
    return true;
}

void RTSPSession::UpdateLocalTransportInfo(
    localTransportInfo *pTransport){
    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession:: Update local rtp port numbers %d, %d",
             pTransport->rtpPort, pTransport->rtcpPort);
        if (rtspSession != NULL) {
            rtspSession->m_sLocalTransportInfo.rtpPort =
                               pTransport->rtpPort;
            rtspSession->m_sLocalTransportInfo.rtcpPort =
                               pTransport->rtcpPort;

        }
    }

}


void RTSPSession::sendIDRRequest() {

    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            rtspWfd myWfd;
            myWfd.idrReq.setValid(true);
            if (rtspSession->server != NULL) {
                rtspSession->server->Set(rtspSession->rtspSessionId, myWfd);
            } else if (rtspSession->client != NULL) {
                rtspSession->client->Set(rtspSession->rtspSessionId, myWfd);
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            }
            break;
        }
    }
}

bool RTSPSession::setResolution (int type, int value, int *resParams)
{
  LOGD ("RTSPSession::setResolution");

  SessionManager *pSM                = SessionManager::Instance();
  MMCapability *commonCapability     = NULL;
  MMCapability *negotiatedCapability = NULL;
  bool bUpdate                       = false;
  CapabilityType resolutionType      = (CapabilityType)type;

  vector<WFDSession*>::iterator it;
  for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++)
  {
    WFDSession* wfdSession = *it;
    if (wfdSession == NULL)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No Active WFD Session, can't set resolution");
      return false;
    }
    RTSPSession* rtspSession = wfdSession->getRtspSession();
    if (rtspSession != NULL)
    {
      commonCapability = rtspSession->pPeerDevice->pCommonCapability;
      negotiatedCapability = rtspSession->pPeerDevice->pNegotiatedMMCapability;
      switch (resolutionType)
      {
        case WFD_CEA_RESOLUTIONS_BITMAP:
          if (commonCapability->pCapability->video_config.video_config.h264_codec->supported_cea_mode & value)
          {
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_cea_mode = value;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_vesa_mode = 0;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_hh_mode = 0;
            bUpdate = true;
          }
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Update CEA resolution");
          break;
        case WFD_VESA_RESOLUTIONS_BITMAP:
          if (commonCapability->pCapability->video_config.video_config.h264_codec->supported_vesa_mode & value)
          {
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_cea_mode = 0;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_vesa_mode = value;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_hh_mode = 0;
            bUpdate = true;
          }
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Update VESA resolution");
          break;
        case WFD_HH_RESOLUTIONS_BITMAP:
          if (commonCapability->pCapability->video_config.video_config.h264_codec->supported_hh_mode & value)
          {
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_cea_mode = 0;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_vesa_mode = 0;
            negotiatedCapability->pCapability->video_config.video_config.h264_codec->supported_hh_mode = value;
            bUpdate = true;
          }
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Update HH resolution");
          break;
        default:
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Unknown resolution type");
      }
      if (bUpdate && MMAdaptor::updateSession(negotiatedCapability))
      {
        if(resParams)
        {
          //resParams has width at [0], height at [1] and FPS at [2]
          negotiatedCapability->pCapability->video_config.video_config.h264_codec->max_hres = resParams[0];
          negotiatedCapability->pCapability->video_config.video_config.h264_codec->max_vres = resParams[1];
        }
        return true;
      }
    } //if (rtspSession != NULL)

  } // end for
  return false;

}


void RTSPSession::sendTransportChangeRequest(int TransportType, int BufferLenMs, int portNum)
{
    UNUSED(portNum);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession: sendTransportChangeRequest");

    SessionManager *pSM = SessionManager::Instance();

    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            rtspWfd myWfd;
            if (rtspSession->server != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling Set: sendTransportUpdate");
                if(TransportType == 1) {
#ifdef LEGACY_TCP
                    myWfd.buffLen.setValid(true);
                    myWfd.buffLen.setBufferLen(BufferLenMs);
#else
                    myWfd.tcpWindowSize.setValid(true);
                    myWfd.tcpWindowSize.setWindowSize(BufferLenMs);
#endif
                    myWfd.client.setValid(true);
                    if(rtspSession->m_sLocalTransportInfo.rtpPort) {
                        myWfd.client.setRtpPort0(rtspSession->m_sLocalTransportInfo.rtpPort);
                    }

                    if(rtspSession->m_sLocalTransportInfo.rtcpPort) {
                        myWfd.client.setRtcpPort0(rtspSession->m_sLocalTransportInfo.rtcpPort);
                    }
                    myWfd.client.setTCP(true);
                }
                else
                {
                    myWfd.client.setValid(true);
                    if(rtspSession->m_sLocalTransportInfo.rtpPort) {
                        myWfd.client.setRtpPort0(rtspSession->m_sLocalTransportInfo.rtpPort);
                    }

                    if(rtspSession->m_sLocalTransportInfo.rtcpPort) {
                        myWfd.client.setRtcpPort0(rtspSession->m_sLocalTransportInfo.rtcpPort);
                    }
                    myWfd.client.setTCP(false);
                }
                rtspSession->server->Set(rtspSession->rtspSessionId, myWfd);
            } else if (rtspSession->client != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid request for client");
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            }
            break;
        }
    }
    return;
}

void RTSPSession::setDecoderLatency(int latency)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession: setDecoderLatency");

    SessionManager *pSM = SessionManager::Instance();

    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            rtspWfd myWfd;
            if (rtspSession->server != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling Set: setDecoderLatency");
                myWfd.buffLen.setValid(true);
                myWfd.buffLen.setBufferLen(latency);
                rtspSession->server->Set(rtspSession->rtspSessionId, myWfd);
            } else if (rtspSession->client != NULL &&
                      (SESS_ESTABLISHED == rtspSession->rtspState)) {
                      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"setRenderLatency");
                      MMAdaptor::streamControl(WFD_DEVICE_PRIMARY_SINK,
                                        AV_CONTROL_SET_DECODER_LATENCY,
                                        latency);
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            }
            break;
        }
    }
    return;
}

void RTSPSession::sendBufferingControlRequest(int ControlType, int cmdVal)
{
    UNUSED(cmdVal);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession: sendBufferingControlRequest");

    SessionManager *pSM = SessionManager::Instance();

    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            rtspWfd myWfd;
            if (rtspSession->server != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling Set: sendBufferingControl");
                rtsp_wfd::tcp_control_command eCmd = rtsp_wfd::cmdNone;
                uint32 flushTimeStamp = 0;
                switch((ControlCmdType)ControlType) {
                case TCP_FLUSH:
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Fetching buffering timestamp");
                    flushTimeStamp = MMAdaptor::getCurrentPTS();
                    myWfd.tcpStreamControl.setDuration(flushTimeStamp);
                    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Fetching flush timestamp from rtsp = %llu",myWfd.tcpStreamControl.getDuration());
                    eCmd = rtsp_wfd::flush;
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Send Buffering Control FLUSH");
                    break;
                case TCP_PLAY:
                    eCmd = rtsp_wfd::play;
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Send Buffering Control PLAY");
                    break;
                case TCP_PAUSE:
                    eCmd = rtsp_wfd::pause;
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Send Buffering Control PAUSE");
                    break;
                case TCP_STATUS:
                    eCmd = rtsp_wfd::status;
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Send Buffering control STATUS");
                    break;
                default:
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid Buffering Control Cmd");
                    eCmd = rtsp_wfd::cmdNone;
                    break;

                }

                if(eCmd == rtsp_wfd::cmdNone) {
                    return;
                }

                myWfd.tcpStreamControl.setCommand(eCmd);
                rtspSession->server->Set(rtspSession->rtspSessionId, myWfd);
            } else if (rtspSession->client != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid request for client");
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            }
            break;
        }
    }
    return;
}


void RTSPSession::queryTCPTransportSupport()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession: queryTCPTransportSupport");

    SessionManager *pSM = SessionManager::Instance();

    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            if (rtspSession->m_bTCPSupportQueried) {
                if (rtspSession->m_bTCPSupportedAtSink) {
                    eventMMUpdate(MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK,MM_STATUS_SUCCESS,
                      0,0,0,0);
                }else{
                    eventMMUpdate(MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK,MM_STATUS_FAIL,
                      0,0,0,0);
                }
            } else{
                rtspSession->m_bTCPSupportStatusRequested = true;
            }
            break;
        }
    }
}


void RTSPSession::setTransport (int value)
{
  LOGD ("RTSPSession::setResolution");

  SessionManager *pSM                = SessionManager::Instance();
  MMCapability *negotiatedCapability = NULL;
  bool bUpdate                       = false;

  vector<WFDSession*>::iterator it;
  for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++)
  {
    WFDSession* wfdSession = *it;
    if (wfdSession == NULL)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No Active WFD Session, can't set transport");
      return;
    }
    RTSPSession* rtspSession = wfdSession->getRtspSession();
    if (rtspSession != NULL)
    {
      negotiatedCapability = rtspSession->pPeerDevice->pNegotiatedMMCapability;
    }
    if((negotiatedCapability!=NULL) && (negotiatedCapability->pCapability!=NULL))
    {
        negotiatedCapability->pCapability->transport_capability_config.eRtpPortType =
                (value == 0)? RTP_PORT_UDP:RTP_PORT_TCP;
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Update Transport");
        if (MMAdaptor::updateSession(negotiatedCapability))
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"succesfully set Transport");
            return;
        }
    }
  } // end for
  return;

}

void RTSPSession::sendWFDKeepAliveMsg()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession: sendWFDKeepAliveMsg");

    SessionManager *pSM = SessionManager::Instance();

    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession != NULL) {
            rtspWfd myWfd;
            if (rtspSession->server != NULL) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling Get: sendWFDKeepAliveMsg");
                rtspSession->server->Get(rtspSession->rtspSessionId, myWfd);
            } else if (rtspSession->client != NULL) {
                rtspSession->client->Get(rtspSession->rtspSessionId, myWfd);
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
            }
            break;
        }
    }
}

void RTSPSession::sendAVFormatChangeRequest() {

    vector<WFDSession*>::iterator it;

    SessionManager *pSM = SessionManager::Instance();
    if((pSM == NULL) || (pSM->pMyDevice == NULL) || (pSM->pMyDevice->pMMCapability == NULL))
        return;

    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        RTSPSession* pRtspSession = wfdSession->getRtspSession();
        if (pRtspSession != NULL) {
            rtspWfd myWfd;
            // query the new proposed capability from MM
            if(MMAdaptor::getProposedCapability(pSM->pMyDevice->pMMCapability, pRtspSession->pPeerDevice->pMMCapability, pRtspSession->pPeerDevice->pNegotiatedMMCapability))
            {
                #if 1
                // Hack to wipe out other audio codecs other than the negotiated one
                switch (pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_method) {
                    case WFD_AUDIO_LPCM:
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec));
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec));
                        break;
                    case WFD_AUDIO_AAC:
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec));
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.dolby_digital_codec));
                        break;
                    case WFD_AUDIO_DOLBY_DIGITAL:
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.lpcm_codec));
                        memset(&(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec), 0, sizeof(pRtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->audio_config.aac_codec));
                        break;
                    default:
                        break;
                }
                #endif

                string params[2] = {
                    "wfd_audio_codecs",
                    "wfd_video_formats"
                };
                char buffer[200];
                rtspWfdParams type;
                bool isParamSet;
                for (int i=0; i<2; i++) {
                    memset(buffer, 0, sizeof(buffer));
                    strlcat(buffer, params[i].c_str(), sizeof(buffer));
                    strlcat(buffer, ": ", sizeof(buffer));
                    strlcat(buffer, pRtspSession->pPeerDevice->pNegotiatedMMCapability->getKeyValue((char*)params[i].c_str()), sizeof(buffer));
                    params[i] = buffer;

                    isParamSet = false;
                    if ((type=myWfd.wfdType(params[i], isParamSet)) != wfd_invalid) {
                        if (isParamSet) {
                            myWfd.wfdParse(type, params[i]);
                        }
                    }
                }

                // query PTS/DTS from MM lib
                uint32 pts, dts;
                MMAdaptor::getAVFormatChangeTiming(&pts, &dts);
                myWfd.timing.setPts(pts);
                myWfd.timing.setDts(dts);
                myWfd.timing.setValid(true);

                if (pRtspSession->server != NULL) {
                    pRtspSession->server->Set(pRtspSession->rtspSessionId, myWfd);
                } else if (pRtspSession->client != NULL) {
                    pRtspSession->client->Set(pRtspSession->rtspSessionId, myWfd);
                } else {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No valid RTSP session.");
                }
                break;
        }
        }
    }
}

bool RTSPSession::setAVPlaybackMode (AVPlaybackMode mode) {

  bool ret = false;
  //TODO , you can't set the playback mode at any time
  //Add checks for state when this is being called
  switch (mode) {

  case  AUDIO_ONLY:
  case  VIDEO_ONLY:
  case  AUDIO_VIDEO:
        m_eplayMode = mode;
        ret = true;
        break;
  case NO_AUDIO_VIDEO:
        // What are we supposed to do with this? May be only UIBC session?
        ret = false;

  }
  return ret;
}

void RTSPSession::setSinkSurface (void* surface) {
  SessionManager *pSM = SessionManager::Instance();
  MMCapability *negotiatedCapability = NULL;

  vector<WFDSession*>::iterator it;
  for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++)
  {
    WFDSession* wfdSession = *it;
    if (wfdSession == NULL)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No Active WFD Session, can't set Sink Surface");
      return;
    }
    RTSPSession* rtspSession = wfdSession->getRtspSession();
    if (rtspSession != NULL && rtspSession->pPeerDevice != NULL)
    {
      negotiatedCapability = rtspSession->pPeerDevice->pNegotiatedMMCapability;
    }
    if((negotiatedCapability!=NULL) && (negotiatedCapability->pCapability!=NULL))
    {
        negotiatedCapability->pCapability->pSurface =  surface;
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Update Surface");
        if (MMAdaptor::updateSession(negotiatedCapability))
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"succesfully set surface");
            return;
        }
    }
  } //for loop end
  return;
}

uint32_t* RTSPSession::getCommonResloution(int* numProf) {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"getCommonResloution");
    SessionManager *pSM = SessionManager::Instance();
    if(!pSM) {
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"SessionManager instance is null!!");
       return NULL;
    }
    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        if (wfdSession == NULL) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No Active WFD Session, can't get Common Resloution");
          return NULL;
        }
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession == NULL || rtspSession->pPeerDevice == NULL ||
            rtspSession->pPeerDevice->pCommonCapability == NULL ||
            rtspSession->pPeerDevice->pCommonCapability->pCapability == NULL ||
            rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.h264_codec == NULL) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something's wrong. Can't get Common Resloution");
          return NULL;
        }
        /*For each of the H264 profiles there will be 3 values to consider CEA,
         *VESA and HH. So create an array with a size of multiple of
         * 4 = [1Profile + its 3 corresponding bitmaps]
         */
        *numProf = rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.num_h264_profiles;
        uint32_t* comRes = new uint32_t[4*(*numProf)];
        for(int i =0;i<*numProf;i++) {
            comRes[4*i]= rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.h264_codec[i].h264_profile;
            comRes[4*i +1] = rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.h264_codec[i].supported_cea_mode;
            comRes[4*i +2] = rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.h264_codec[i].supported_vesa_mode;
            comRes[4*i +3] = rtspSession->pPeerDevice->pCommonCapability->pCapability->video_config.video_config.h264_codec[i].supported_hh_mode;
            MM_MSG_PRIO4(MM_GENERAL,MM_PRIO_HIGH,"For profile %d CEA mode is %u, VESA mode is %u, HH mode is %u",comRes[4*i],comRes[4*i +1],comRes[4*i +2],comRes[4*i +3]);
        }
        return comRes;
    }
    return NULL;
}

uint32_t* RTSPSession::getNegotiatedResloution() {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"getNegotiatedResloution");
    SessionManager *pSM = SessionManager::Instance();
    if(!pSM) {
       MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"SessionManager instance is null!!");
       return NULL;
    }
    vector<WFDSession*>::iterator it;
    for (it=pSM->vecWFDSessions.begin(); it!=pSM->vecWFDSessions.end(); it++) {
        WFDSession* wfdSession = *it;
        if (wfdSession == NULL) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"No Active WFD Session, can't get Common Resloution");
          return NULL;
        }
        RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession == NULL || rtspSession->pPeerDevice == NULL ||
            rtspSession->pPeerDevice->pNegotiatedMMCapability == NULL ||
            rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability == NULL ||
            rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_config.video_config.h264_codec == NULL) {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Something's wrong. Can't get Negotiated Resloution");
          return NULL;
        }
        /*For the H264 profile there will be 3 values to consider CEA,VESA & HH
         *Create array of size 4 = [1Profile + its 3 corresponding bitmaps]
         */
        uint32_t* negRes = new uint32_t[4];
        negRes[0] = rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_config.video_config.h264_codec[0].h264_profile;
        negRes[1] = rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_config.video_config.h264_codec[0].supported_cea_mode;
        negRes[2] = rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_config.video_config.h264_codec[0].supported_vesa_mode;
        negRes[3] = rtspSession->pPeerDevice->pNegotiatedMMCapability->pCapability->video_config.video_config.h264_codec[0].supported_hh_mode;
        MM_MSG_PRIO4(MM_GENERAL,MM_PRIO_HIGH,"For profile %d CEA mode is %u, VESA mode is %u, HH mode is %u",negRes[0],negRes[1],negRes[2],negRes[3]);
        return negRes;
    }
    return NULL;
}
MMEventStatusType RTSPSession::updateHdcpSupportedInConnectedDevice(void *pCfgCababilities)
{
  UNUSED(pCfgCababilities);

  MMEventStatusType nStatus = MM_STATUS_SUCCESS;
#ifdef HDCP_DISPLAY_ENABLED
  WFD_MM_capability_t *pReadCababilities = (WFD_MM_capability_t *)pCfgCababilities;

  LOGV("Disp HDCP Manager Init");

  int ret = 0;
  ret = HDCP1X_COMM_Init(&RTSPSession::DISPhdcpCallBack);
  if (ret)
  {
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Fail Initializing disp hdcp manager!");
     return MM_STATUS_FAIL;
  }
  else
  {
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Initializing disp hdcp manager successful!");
  }


  bHDCPStatus = MM_STATUS_INVALID;

  ret = HDCP1X_COMM_Send_hdcp2x_event( EV_REQUEST_TOPOLOGY, NULL, NULL);

  if (ret)
  {
     bHDCPStatus = MM_STATUS_FAIL;
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Fail sending request to disp hdcp manager!");

  }
  else
  {
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Request_topology event to disp hdcp manager success!");
  }

  // wait here for the CB from DISPLAY.
  while (true)
  {
     if (bHDCPStatus == MM_STATUS_CONNECTED)
     {
        nStatus = MM_STATUS_SUCCESS;
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"HDCP Content protection is supprted in the connected device");
        break;
     }
     else if ((bHDCPStatus == MM_STATUS_NOTSUPPORTED) ||
              (bHDCPStatus == MM_STATUS_FAIL)) // FAIL means send req faild to display,
     {
       nStatus = bHDCPStatus;
       if (pReadCababilities)
       {
         // over write the config file read values wtih content protection not supported
         pReadCababilities->content_protection_config.content_protection_capability = 0;
         pReadCababilities->content_protection_config.content_protection_ake_port = 0;

         MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"HDCP Content protection is not supprted in the connected device");
       }
       break;
     }
     else
     {
       // wait for Display to notify us on the connected/authenticated status.
       MM_Timer_Sleep(5);
     }
  }

  //!Warning: Need to Enable
  ret = HDCP1X_COMM_Term();
  if (ret)
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"fail DeInit disp hdcp manager!");
  else
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"DeInit disp hdcp manager successful!");


  bHDCPStatus = MM_STATUS_INVALID;
#endif
  return nStatus;
}


int RTSPSession::DISPhdcpCallBack(int type, void *param)
{
    UNUSED(type);
    UNUSED(param);
    int ret = 0, *temp = NULL;
    //struct HDCP_V2V1_DS_TOPOLOGY *msg = NULL;
#ifdef HDCP_DISPLAY_ENABLED
    switch (type) {
    case EV_REQUEST_TOPOLOGY:
    case EV_SEND_TOPOLOGY:

      // supports HDCP
      bHDCPStatus = MM_STATUS_CONNECTED;
      break;

    case EV_ERROR:
        temp = (int *)param;
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Received error from hdcp manager, error = %d!",*temp);

        bHDCPStatus = MM_STATUS_NOTSUPPORTED;
        break;
    }
#endif
    return ret;
}

int RTSPSession::getCfgItems(int** configItems, size_t* len)
{
    if(!len || !configItems)
    {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Invalid args to %s!",__FUNCTION__);
        return -1;
    }
    *len = cfgItems.size();
    *configItems = new int[*len];//Caller to take care of deletion
    for(unsigned int i=0;i< *len;i++) {
        (*configItems)[i] = cfgItems.at(i);
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"configItems[%d] = %d",i,(*configItems)[i]);
    }
    return 0;
}

/*bool RTSPSession::isHDMIConnected()
{
  char *path = "/sys/class/graphics/fb1/connected";
  int connected = 0;
  int fd = open(path, O_RDONLY, 0);
  if(fd > 0)
  {
    int datasize = read(fd, connected, sizeof(int));
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTSPSession::isHDMIConnected() connected = %d!",connected);
  }
  return connected;
}*/

