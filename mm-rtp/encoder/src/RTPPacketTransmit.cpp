/*==============================================================================
*        @file RTPPacketTransmit.cpp
*
*  @par DESCRIPTION:
*       This is the implementation of the RTPPacketizer class.
*
*
*  Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

================================================================================
*/


#include "AEEstd.h"
#include "RTPPacketTransmit.h"
#include "MMDebugMsg.h"
#include "errno.h"
#include <unistd.h>
#include "MMTimer.h"
#define TCP_DATA_SEND_TIMEOUT 5000
//bool bTCP = false;
unsigned int nInPackets;
unsigned int nOutPackets;
bool bConnectionTried = true;

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

/*==============================================================================

         FUNCTION:         CRTPPacketTransmit

         DESCRIPTION:
*//**       @brief         constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTPPacketTransmit::CRTPPacketTransmit(uint32 portNum, int32 nSocket, uint32 destIP, uint8 bRtpPortTypeUdp)
{
    struct addrinfo *result = NULL, *ptr = NULL;
    char sPortNum[10] = {0};
    m_bRtpPortTypeUdp = bRtpPortTypeUdp;

    bConnectionTried = true;
    m_bSockAssigned   = false;

    _success = true;

    memset(&faraddr, 0, sizeof(sockaddr_in));
    faraddr.sin_addr.s_addr = (unsigned int)destIP;
    faraddr.sin_family = AF_INET;
    faraddr.sin_port = htons((uint16)portNum);
#ifdef RTCPONHOSTTEST
    tempSock = socket(AF_INET, SOCK_DGRAM,
                           IPPROTO_UDP);
    memset(&faraddr_rtcp, 0, sizeof(sockaddr_in));
    faraddr_rtcp.sin_addr.s_addr = inet_addr("127.0.0.1");;
    faraddr_rtcp.sin_family = AF_INET;
    faraddr_rtcp.sin_port = htons((uint16)19023);
#endif

    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "ip = %x port = %d bRtpPortTypeUdp = %d",
                 destIP, portNum,bRtpPortTypeUdp );
    XmitSocket = -1;
    if(_success)
    {
        if(!m_bRtpPortTypeUdp)
        {
           // Create a SOCKET for the server to listen for client connections
           XmitSocket = socket(AF_INET, SOCK_STREAM,
                       IPPROTO_TCP);
        }
        else
        {
           // Create a SOCKET for the server to listen for client connections
           if(nSocket <= 1)
           {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPTX Creating Socket");
               XmitSocket = socket(AF_INET, SOCK_DGRAM,
                           IPPROTO_UDP);
           }
           else
           {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPTX Accepting assigned port");
               m_bSockAssigned = true;
               XmitSocket = (int)nSocket;
           }
        }

        if (XmitSocket < 0)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "socket creation failed");
            _success = false;
        }
     //   MakeSocketBlocking(mSocket, false);
    }
}
/*==============================================================================

         FUNCTION:         SendPacket

         DESCRIPTION:
*//**       @brief         Send the data as UDP packet
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTPPacketTransmit::SendPacket(const uint8 *pData, uint32 nLen)
{
    int iSendResult;
    int ConnectResult=-1;
    int nBuffersize = 1024 * 1024;
    uint8 nOptVal = 255;
    address_t tAddr;
    if(bConnectionTried)
    {
      setsockopt(XmitSocket,IPPROTO_IP,IP_TTL, (const char*)&nOptVal, 1);
      setsockopt(XmitSocket,SOL_SOCKET,SO_SNDBUF, (const char*)&nBuffersize, 4);
      int prio = 6 << 5;//First 3 bits of TOS field
      setsockopt(XmitSocket, SOL_IP, IP_TOS, &prio, (socklen_t)sizeof(prio));
      struct timeval tv;
      tv.tv_sec = 2;
      tv.tv_usec = 0; 

      setsockopt(XmitSocket, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv,(socklen_t)sizeof(struct timeval));

      if(!m_bRtpPortTypeUdp)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Start TCP connect");
        uint32 timer = 1000;
        while(ConnectResult != 0 && timer-- > 0)
        {
          tAddr.sa_in = faraddr;
          ConnectResult = connect( XmitSocket, (struct sockaddr*)&tAddr.sa, (socklen_t)sizeof(faraddr));
          if (ConnectResult < 0)
          {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "socket connect failed == %d", errno);
            _success = false;
            usleep(1000);
          }
          else
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "TCP connected ");
            _success = true;
          }
        }
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Started UDP connect");
      }
      bConnectionTried = false;
    }
    if(_success == false)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTP port open failed");
        return 0;
    }
  //  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "RTP port writing ... %d, packetnum= %d",nLen , nOutPackets);

       nInPackets++;
       if(!m_bRtpPortTypeUdp)
       {
            unsigned long nStartTime;
            MM_Time_GetTime(&nStartTime);
            uint32 nTotalTime =  TCP_DATA_SEND_TIMEOUT;
            unsigned long nCurrTime = 0;
            uint32 nTempTime = 0;
            int nLeftLen = (int)nLen;
            int nSentBytes = 0;
            do {
                pData += nSentBytes;
                nLeftLen -= nSentBytes;
                nSentBytes = (int)send(XmitSocket, (const unsigned char*)pData, nLeftLen, 0);
                if (nSentBytes == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK ||
                        errno == ENOBUFS || errno == ETIMEDOUT)
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTP Send Retry errno %d", errno);
                        nSentBytes = 0;
                    }
                    else
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTP Send Failed errno %d", errno);
                        nLen = -1;
                        _success = false;
                        break;
                    }
                }
                MM_Time_GetTime(&nCurrTime);
                if((uint32)(nCurrTime - nStartTime) > nTotalTime)
                {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "tcp data send timeout... break");
                  nLen = -1;
                  _success = false;
                  break;
                }
           } while (nSentBytes < nLeftLen);
            iSendResult = (int)nLen;
       }
       else
       {
           tAddr.sa_in = faraddr;
           iSendResult = (int)sendto(XmitSocket,(const char*) pData, nLen,
                                       0,(struct sockaddr*)&tAddr.sa, (socklen_t)sizeof(faraddr) );
           if(iSendResult == -1|| iSendResult == 0)
           {
               MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,"RTP SendTo Failed result %d errno %d", iSendResult, errno);
               if(errno == EAGAIN || errno == EWOULDBLOCK ||
                  errno == ENOBUFS || errno == ETIMEDOUT)
               {
                   iSendResult = 0;
               }
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"Return num bytes written %d", iSendResult);

           }

       }
#ifdef RTCPONHOSTTEST
       if(nOutPackets%100 == 0)
       {

           sendto(tempSock,"RTCP Test Message", strlen("RTCP Test Message"),
                                       0,(struct sockaddr*)&faraddr_rtcp, sizeof(faraddr_rtcp) );

           MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Send RTCP test message ...");

       }
#endif
       if ((uint32)iSendResult != nLen)
       {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "mismatch in sent bytes ");
       }
       else
       {
           nOutPackets++;
       }

    return iSendResult;

}


/*==============================================================================

         FUNCTION:         ~CRTPPacketTransmit

         DESCRIPTION:
*//**       @brief         Send the data as UDP packet
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTPPacketTransmit::~CRTPPacketTransmit()
{
    if(!m_bSockAssigned)
    {
        close(XmitSocket);
    }
    XmitSocket = -1;
}
