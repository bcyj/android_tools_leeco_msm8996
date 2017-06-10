#ifndef _RTSP_COMMON_H
#define _RTSP_COMMON_H
/***************************************************************************
 *                             rtsp_common.h
 * DESCRIPTION
 *  RTSP Common interface for RTSP_LIB module
 *
 * Copyright (c)  2011 - 2014 QUALCOMM Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_common.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#if defined (__WIN32__) || defined (_WIN32)
#define WIN_BUILD	1
#endif
#define RTSP_FREEIF(x) if(x){ MM_Free(x);x=NULL;}
#define RTSP_DELETEIF(x) if(x){ MM_Delete(x);x=NULL;}

#ifdef WIN_BUILD
#include "winsock2.h"
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <strings.h>
#endif
#include <string.h>
#include <string>
#include <list>
#include <map>
#include <bitset>
#include "time.h"
#include <stdlib.h>
#include <algorithm>
#include <assert.h>
#include <cctype>
#include "MMMemory.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"

using namespace std;
using std::string;

#define MAXLEN	                            16384
#define RTSP_PORT                           "554"
#define DEFAULT_TIMEOUT                     5000
#define RESPONSE_TIMEOUT                    DEFAULT_TIMEOUT
#define REQUEST_TIMEOUT                     6000
#define DEFAULT_KEEP_ALIVE_TIMEOUT          60000
/* The default keep alive timeout is different from other keep alive timeout
 * defined in the session manager. That value refers to the interval in which
 * the keep alive request is sent. The above value refers to the time after
 * which RTSP server can close connection on not receiving keep alive response
 */

#define UNUSED(x) ((void)x)

#ifndef WIN_BUILD
#define SOCKET int
#endif

class rtspApiMesg;

void WINSTARTUP();
void WINSHUTDOWN();
void CLOSESOCKET(SOCKET);
void SETSOCKOPT(SOCKET, int);
void INETPTON(string, sockaddr_in &);
void SEND(SOCKET, string);
void SEND(SOCKET, rtspApiMesg &);
int RECV(SOCKET, char *);
SOCKET ACCEPT(SOCKET, sockaddr_in &);
SOCKET SOCK();
void CONNECT(SOCKET, sockaddr_in &);
void LISTEN(SOCKET, int);
void BIND(SOCKET, sockaddr_in &);
unsigned GET_TICK_COUNT();

#define SESSION SOCKET
#define MAC_ADDR_LEN	12

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

enum rtspSubState {
    request,
    response
};

enum rtspState {
    M1,
    M2,
    M3,
    M4,
    M5,
    M6,
    M7,
    M8,
    M9,
    M10,
    INVALID_RTSP_STATE
};

// Global error flag when not using exceptions
extern int globalError;
#define ERROR_CHECK	do {	\
	if (globalError) return;		\
} while(0);
#define ERROR_CHECK_VAL	do {		\
	if (globalError) return globalError;		\
} while(0);
#define ERROR_CHECK_STR	do {		\
	if (globalError) return string("");		\
} while(0);
#define SET_ERROR	globalError=-1
#define CLEAR_ERROR	globalError=0
#define IS_ERROR globalError


#endif /*_RTSP_COMMON_H*/
