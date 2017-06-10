/***************************************************************************
 *                             rtsp_linux.cpp
 * DESCRIPTION
 *  Linux BSD socket definitions for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_linux.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "rtsp_api.h"
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>


#ifndef WIN_BUILD

int globalError = 0;


void WINSTARTUP() {}
void WINSHUTDOWN() {}

void CLOSESOCKET(SOCKET sock)
{
    close(sock);
}

void SETSOCKOPT(SOCKET cmdSock, int reuseaddr)
{
    if (setsockopt(cmdSock, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&reuseaddr, (socklen_t)sizeof(reuseaddr)) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: setsockopt %s" , string(strerror(errno)).c_str());
        SET_ERROR;
        return;
    }
}

void INETPTON(string ipaddr, sockaddr_in &saddr)
{
    if (inet_pton(AF_INET, ipaddr.c_str(), &saddr.sin_addr) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: inetpton  %s" , string(strerror(errno)).c_str());
        SET_ERROR;
        return;

    }
}

void SEND(SOCKET sock, string message)
{
    if (send(sock, (const void *)message.c_str(), message.length(), 0) < 0) {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: send %s ", string(strerror(errno)).c_str());
        SET_ERROR;
        return;
    }
}

void SEND(SOCKET sock, rtspApiMesg &mesg)
{
    if (send(sock, (const void *)&mesg, sizeof(mesg), 0) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: send %s " , string(strerror(errno)).c_str());
        SET_ERROR;
        return;

    }
}

int RECV(SOCKET sock, char *recvline)
{
    ssize_t retval = 0;

    if ((retval = recv(sock, (void *)recvline, MAXLEN, 0)) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: recv %s" , string(strerror(errno)).c_str());
        SET_ERROR;
    }
    if (retval == 0) {

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Connection closed remotely");
        SET_ERROR;
    }
    return (int)retval;
}

void LISTEN(SOCKET sock, int backlog)
{
    if (listen(sock, backlog) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: listen %s " , string(strerror(errno)).c_str());
        SET_ERROR;
        return;
    }
}

SOCKET SOCK()
{
    SOCKET retval;

	if ((retval = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: socket %s", string(strerror(errno)).c_str());
        SET_ERROR;
    }
    else
    {
        int prio = 6 << 5;//First 3 bits of TOS field
        setsockopt(retval, SOL_IP, IP_TOS, &prio, (socklen_t)sizeof(prio));
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(retval, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv,(socklen_t)sizeof(struct timeval));
        setsockopt(retval, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,(socklen_t)sizeof(struct timeval));
    }
    return retval;
}

void CONNECT(SOCKET sock, sockaddr_in &saddr)
{
    socklen_t addrlen = (socklen_t)sizeof(saddr);

    if (connect(sock, (sockaddr *)&saddr, addrlen) < 0) {

        if(EINPROGRESS == errno || EWOULDBLOCK == errno)
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: connect %s", string(strerror(errno)).c_str());
          fd_set fdReadSet;
          FD_ZERO(&fdReadSet);
          FD_SET(sock, &fdReadSet);

          struct timeval timeout;
          timeout.tv_sec = 0;//inseconds
          timeout.tv_usec = 1000;//in micro seconds

          int selectStatus = select(sock+1, &fdReadSet, NULL, NULL, &timeout);
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: connect :: select return status %d",selectStatus);

          if (selectStatus > 0) {
            if(FD_ISSET(sock, &fdReadSet))
            {
              int errorNum = 0;
              socklen_t size = (socklen_t)sizeof(int);
              getsockopt(sock,SOL_SOCKET,SO_ERROR,(char*)(&errorNum), &size);

              if(errorNum == 0)
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: connect :: connection established");
                globalError = 0;
                return;
              }
              else
              {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: connect %s",string(strerror(errno)).c_str());
                SET_ERROR;
                return;
              }
            }
          }
          else
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: connect :: select is not successful");
            SET_ERROR;
            return;
          }
          return;
        }
        if(errno == EISCONN)
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Client already Connected : %s", string(strerror(errno)).c_str());
          CLEAR_ERROR;
          return;
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: connect %s", string(strerror(errno)).c_str());
        SET_ERROR;
        return;
    }
}

void BIND(SOCKET sock, sockaddr_in &saddr)
{
    socklen_t addrlen = (socklen_t)sizeof(saddr);

    if (bind(sock, (sockaddr *)&saddr, addrlen) < 0) {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: bind %s " , string(strerror(errno)).c_str());
        SET_ERROR;
        return;
    }
}

SOCKET ACCEPT(SOCKET sock, sockaddr_in &saddr)
{
    size_t addrlen = sizeof(saddr);
    SOCKET retval;

    if ((retval = accept(sock, (sockaddr *)&saddr, (socklen_t *)&addrlen)) < 0) {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Error: accept %s" , string(strerror(errno)).c_str());
        SET_ERROR;

    }
    return retval;
}

unsigned GET_TICK_COUNT()
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        return 0;

    return (unsigned)(tv.tv_sec * 1000) + (unsigned)(tv.tv_usec / 1000);
}

#endif
