#ifndef _RTSP_BASE_H
#define _RTSP_BASE_H

/***************************************************************************
 *                             rtsp_base.h
 * DESCRIPTION
 *  RTSP Base class for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_base.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "rtsp_session.h"

/*
 * Base class that the source and sink classes are derived from
 */
class rtspBase {
public:
    rtspBase(SOCKET networkSocket, string ip, rtspWfd display, unsigned uibcPort, sockaddr_in addr)
	{
        session.setSessionID(networkSocket);
        session.setSocket(networkSocket);

        session.copyWfd(display);
        session.setIp0(ip);
        session.setIp1(ip);
        session.setUibcPort(uibcPort);
        session.setIpAddr(inet_ntoa(addr.sin_addr));
        wfd = display;
	}
    rtspBase(SOCKET networkSocket, unsigned port0, unsigned port1, rtspWfd display, rtsp_wfd::rtspMode mode, string mac, unsigned hdcpPort)
	{
        session.setSessionID(networkSocket);
        session.setSocket(networkSocket);

        session.copyWfd(display);
        session.setRtpPort0(port0);
        session.setRtpPort1(port1);
        session.setHdcpPort(hdcpPort);

        if (mode == rtsp_wfd::coupledPrimarySink ||
            mode == rtsp_wfd::coupledSecondarySink) {
            session.setCoupled();
            if (mac.length())
                session.setCoupledMac(mac);
        }
        wfd = display;
	}
    virtual ~rtspBase() {}
    /* Message processing functions */
    void processParsedMesg(rtspParams *);
	virtual void applySettings(rtspParams *) {}

    friend class rtspM1;
	friend class rtspM2;
	friend class rtspM3;
	friend class rtspM4;
	friend class rtspM5;
	friend class rtspM6;
	friend class rtspM7;
	friend class rtspM8;
	friend class rtspM9;
	friend class rtspM10;
    friend class rtspServer;
    friend class rtspClient;
    friend class rtspBaseState;

protected:
    /* Per connection session */
    rtspSession session;
    rtspWfd wfd;
    rtspWfd theirWfd;
    bitset<WFD_MAX_SIZE> wfdGet;
    bitset<WFD_MAX_SIZE> wfdSet;
    bitset<METHOD_MAX_SIZE> methodSupp;
    sockaddr_in saddr;
};

#endif /*_RTSP_BASE_H*/
