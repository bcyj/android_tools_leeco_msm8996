/***************************************************************************
 *                             rtsp_windows.cpp
 * DESCRIPTION
 *  WIN32 socket definitions for RTSP_LIB module
 *
 * Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_windows.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "rtsp_api.h"

#ifdef WIN_BUILD

void CLOSESOCKET(SOCKET sock)
{
    closesocket(sock);
}

void SETSOCKOPT(SOCKET sock, int reuseaddr)
{
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
        (const char *)&reuseaddr, sizeof(reuseaddr)) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: setsockopt " << WSAGetLastError();
        throw(ss.str());
    }
}

void WINSTARTUP()
{
    WSADATA wsaData;
    int error;

    if ((error = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
        stringstream ss;

        ss << "Error: WSAStartup " << error;
        throw(ss.str());
    }
}

void WINSHUTDOWN()
{
    WSACleanup();
}

void INETPTON(string ipaddr, sockaddr_in &saddr)
{
    if (InetPton(AF_INET, ipaddr.c_str(), &saddr.sin_addr) < 0) {
        stringstream ss;

        ss << "Error: inetpton " << WSAGetLastError();
        throw(ss.str());
    }
}

void SEND(SOCKET sock, string message)
{
    if (send(sock, (const char *)message.c_str(), (int)message.length(), 0)
        == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: send " << WSAGetLastError();
        throw(ss.str());
    }
}

void SEND(SOCKET sock, rtspApiMesg &mesg)
{
    if (send(sock, (const char *)&mesg, sizeof(mesg), 0) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: send " << WSAGetLastError();
        throw(ss.str());
    }
}

int RECV(SOCKET sock, char *recvline)
{
    int retval = 0;

    if ((retval = recv(sock, (char*)recvline, MAXLEN, 0)) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: recv " << WSAGetLastError();
        throw(ss.str());
    }
    if (retval == 0) {
        throw string("Connection closed remotely");
    }
    return retval;
}

void LISTEN(SOCKET sock, int backlog)
{
    if (listen(sock, backlog) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: listen " << WSAGetLastError();

        throw(ss.str());
    }
}

SOCKET SOCK()
{
    SOCKET retval;

	if ((retval = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        stringstream ss;

        ss << "Error: socket " << WSAGetLastError();
        throw(ss.str());
    }
    return retval;
}

void CONNECT(SOCKET sock, sockaddr_in &saddr)
{
    int addrlen = sizeof(saddr);

    if (connect(sock, (sockaddr *)&saddr, addrlen) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: connect " << WSAGetLastError();
        throw(ss.str());
    }
}

void BIND(SOCKET sock, sockaddr_in &saddr)
{
    int addrlen = sizeof(saddr);

    if (bind(sock, (sockaddr *)&saddr, addrlen) == SOCKET_ERROR) {
        stringstream ss;

        ss << "Error: bind " << WSAGetLastError();
        throw(ss.str());
    }
}

SOCKET ACCEPT(SOCKET sock, sockaddr_in &saddr)
{
    int addrlen = sizeof(saddr);
    SOCKET retval;

    if ((retval = accept(sock, (sockaddr *)&saddr, &addrlen)) == INVALID_SOCKET) {
        stringstream ss;

        ss << "Error: accept " << WSAGetLastError();
        throw(ss.str());
    }
    return retval;
}

unsigned GET_TICK_COUNT()
{
    return GetTickCount();
}

#endif
