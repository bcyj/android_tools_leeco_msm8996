/*
*  @file wfd_netutils.cpp
*  @par DESCRIPTION:
*       Definition of the wfd network utilities
*
*
*  Copyright (c) 2012-2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <stdlib.h>
#include <errno.h>

#include "MMDebugMsg.h"

#include "wfd_netutils.h"
#define QCSAP_IOCTL_PRIV_GET_SOFTAP_LINK_SPEED (SIOCIWFIRSTPRIV+31)
#define MAC_ADDR_LEN 6


#define RTP_PORT_START_NUM 19022
#define RTP_PORT_END_NUM   19189

#define MAX_IFS 64

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

#define UNUSED(x) ((void)x)

#ifndef WFD_ICS
int getP2pInterfaceName(char* pInterfaceName, int size)
{
    struct ifreq *ifr, *ifend;
    struct ifconf ifc;
    struct ifreq ifs[MAX_IFS];
    struct ifreq ifreq;
    int s;

    // check input parameters

    s = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = static_cast<int>(sizeof(struct ifreq)) * MAX_IFS;
    ifc.ifc_req = ifs;
    if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "getP2pInterfaceName: error at ioctl(SIOCGIFCONF)");
      close(s);
      return -1;
    }

    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
    for (ifr = ifc.ifc_req; ifr < ifend; ifr++)
    {
      if (ifr->ifr_addr.sa_family == AF_INET)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                     "getP2pInterfaceName: ifname = %s", ifr->ifr_name);

        if (strncmp(ifr->ifr_name, "p2p", 3) == 0)
        {
          strlcpy(pInterfaceName, ifr->ifr_name, size);
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                       "getP2pInterfaceName: p2p-ifname = %s", pInterfaceName);
           close(s);
           return 0;
        }
      }
    }
    close(s);
    return -1;
}
#endif

int getLocalIpAddress(char* pLocalIpAddr, int size)
{
    //getting local IP address
    int requiredSize = 20;
    struct ifreq ifr;
    int s;
    unsigned int addr;
    char p2pifname[IFNAMSIZ];

    if (pLocalIpAddr == NULL) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "pLocalIpAddr is NULL");
        return -1;
    }
    if (size < requiredSize) {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "Not enough size for IPv4 ip address. Required min: %d, given: %d", requiredSize, size);
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
#ifdef WFD_ICS
    strlcpy(ifr.ifr_name, "wlan0", IFNAMSIZ);
#else
    if (getP2pInterfaceName(p2pifname, IFNAMSIZ) == -1) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "getP2pInterfaceName Failed,"\
                                              "Falling back to p2p0");
        strlcpy(ifr.ifr_name, "p2p0", IFNAMSIZ);
    }
    else
    {
      p2pifname[IFNAMSIZ-1]='\0';
      strlcpy(ifr.ifr_name, p2pifname, IFNAMSIZ);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "getP2pInterfaceName ,"\
                                            "interface name = %s",ifr.ifr_name);
    }
#endif
    ifr.ifr_name[IFNAMSIZ-1] = 0;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "getLocalIpAddress :Error in creating socket");
        return -1;
    } else {
        if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "getLocalIpAddress :: %s",strerror(errno));
            close(s);
            return -1;
        }
        else
        {
            addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "getLocalIpAddress :addr[%x]",addr);
            snprintf(pLocalIpAddr, requiredSize, "%d.%d.%d.%d",
                addr & 0xff,
                ((addr >> 8) & 0xff),
                ((addr >> 16) & 0xff),
                ((addr >> 24) & 0xff));
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "getLocalIpAddress :local IP Address %s", pLocalIpAddr);
            close(s);
            return 0;
        }
    }
}

int getLocalMacAddress(char* pMacAddr, int size) {
    UNUSED(pMacAddr);
    UNUSED(size);
    return -1;
}

unsigned int getLinkSpeed(char *macaddr, unsigned int len, bool& queryIfName) {


    struct iwreq iwrrequest;

    char inputstring[64] ={0};
    int result = 0;
    unsigned int linkspeed = 0;
    static char p2pifname[IFNAMSIZ] = {0};

    if(!macaddr || len < MAC_ADDR_LEN)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "NetUtils Invalid Mac Address");
        return 0;
    }

    int nSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(nSocket < 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "NetUtils Failed to create socket");
    return 0;
    }
#ifdef WFD_ICS
    strlcpy(iwrrequest.ifr_name, "wlan0", IFNAMSIZ);
#else
    if(queryIfName)
    {
        if (getP2pInterfaceName(p2pifname, IFNAMSIZ) == -1)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "getP2pInterfaceName failed "\
                                                  " Falling back to p2p0");
            strlcpy(p2pifname, "p2p0", IFNAMSIZ);//Do we need to re-query??
        }
        else
        {
          p2pifname[IFNAMSIZ-1] = '\0';
          queryIfName = false;//Interface name query was a success , don't redo
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "getP2pInterfaceName ,"\
                                            "interface name = %s",iwrrequest.ifr_name);
        }
    }
    if (p2pifname != NULL)
    {
        strlcpy(iwrrequest.ifr_name, p2pifname, IFNAMSIZ);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "p2pifname is NULL");
        return 0;
    }
#endif
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "getLinkSpeed interface name = %s",iwrrequest.ifr_name);
    int offset = static_cast<int>(strlen(inputstring));

    if(macaddr != NULL)
    {
        strlcpy((char*)inputstring, (const char*)macaddr, 64);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "macaddr is NULL");
    }

    iwrrequest.u.data.pointer = (char*)inputstring;
    iwrrequest.u.data.length = static_cast<unsigned short>(strlen((char*)inputstring));

    result =  ioctl(nSocket,
                    QCSAP_IOCTL_PRIV_GET_SOFTAP_LINK_SPEED,
                    &iwrrequest);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "NetUtils Linkspeed  = %s", inputstring);

    if(result == 0)
    {
        //sscanf(inputstring, "%d", &linkspeed);
        linkspeed = static_cast<unsigned short>(atoi(inputstring));

    }
    else
    {
        linkspeed = 0;
    }

    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                 "NetUtils:Linkspeed result = %d, linkspeed = %u",
                 result,linkspeed * 500000);

    close(nSocket);

    return linkspeed * 500000;
}

bool getIPSockPair(bool bPair, int *sock1, int * sock2,
                    int *nPort1, int *nPort2, bool tcp)
{

   if(sock1 == NULL)
   {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                   "Invalid Arguments to find socket");
       return false;
   }
   int RTPSocket = -1;
   int RTCPSocket = -1;

   //Usually this is used to find RTP/RTCP port pair
   //Hence the naming
   if(bPair)
   {
       sockaddr_in saddr;
       address_t addr;
       int ret = -1;
       unsigned short portNum;

       saddr.sin_family = AF_INET;
       saddr.sin_addr.s_addr = INADDR_ANY;

       for(portNum = RTP_PORT_START_NUM;
            portNum < RTP_PORT_END_NUM; portNum+=2)
       {
           saddr.sin_port = htons(portNum);

           RTPSocket = socket(AF_INET, tcp? SOCK_STREAM:SOCK_DGRAM,
                    tcp? IPPROTO_TCP:IPPROTO_UDP);
           if(RTPSocket < 0)
           {
               return false;
           }


           addr.sa_in = saddr;
           ret = bind(RTPSocket, (sockaddr *)&addr.sa, (socklen_t)sizeof(saddr));

           if(ret < 0)
           {
               //Check if bind failed due to port already being used.
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                            "Bind Failed for RTP port num %d Retry", portNum);
               close(RTPSocket);
               continue;
           }
           else
           {
               RTCPSocket = socket(AF_INET, tcp? SOCK_STREAM:SOCK_DGRAM,
                    tcp? IPPROTO_TCP:IPPROTO_UDP);
               saddr.sin_port = htons((unsigned short)(portNum + 1));
               addr.sa_in = saddr;
               ret = bind(RTCPSocket, (sockaddr *)&addr.sa, (socklen_t)sizeof(saddr));
               if(ret < 0)
               {
                   close(RTPSocket);
                   close(RTCPSocket);
                   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                       "RTCP port Bind failed %d, retry ", portNum + 1);
               }
               else
               {
                   MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                               "Found RTP_RTCP port pair %d %d",
                               portNum, portNum+1);
                   if(sock2)
                   {
                       *sock2 = RTCPSocket;
                       if(nPort2)
                       {
                           *nPort2 = portNum + 1;
                       }
                   }
                   else
                   {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                               "Invalid Arg for Sock2");
                       close(RTCPSocket);
                       false;
                   }
                   *sock1 = RTPSocket;
                   if(nPort1)
                   {
                       *nPort1 = portNum;
                   }
                   break;
               }
           }
       }
   }
   else
   {
       int ret;
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                     "Find single Port");
       RTPSocket = socket(AF_INET, tcp? SOCK_STREAM:SOCK_DGRAM,
                    tcp? IPPROTO_TCP:IPPROTO_UDP);
       sockaddr_in saddr;
       address_t addr;
       saddr.sin_family = AF_INET;
       saddr.sin_addr.s_addr = INADDR_ANY;
       saddr.sin_port = 0;
       addr.sa_in = saddr;
       ret = bind(RTPSocket, (sockaddr *)&addr.sa, (socklen_t)sizeof(saddr));

       if(ret < 0)
       {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                 "RTP port Bind failed");
       }

       int size = (int)sizeof(saddr);

       ret = getsockname(RTPSocket, (struct sockaddr *)&addr.sa, (socklen_t*)(&size));
       if(ret == 0)
       {
           if(nPort1)
           {
               *nPort1 = ntohs(saddr.sin_port);
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                           "Server RTP port Num = %d", *nPort1);
           }
       }
       else
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                 "getsockname failed");

       *sock1 = RTPSocket;
   }

   return true;
}

int getLocalIpSocket (int sock, char* IP)
{
    sockaddr_in             saddr;
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port        = 0;
    char* localIP         = NULL;
    int size              = (int)sizeof(saddr);
    address_t addr;

    addr.sa_in = saddr;
    int ret = getsockname(sock, (struct sockaddr *)&addr.sa, (socklen_t*)(&size));
    if(ret == 0)
    {
        localIP = inet_ntoa(saddr.sin_addr);
        if (localIP == NULL || IP == NULL)
        {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                       "WFD NetUtils:Local IP is null or input param failure return failed");
           return -1;
        }
        else
        {
           strlcpy(IP,localIP,strlen(localIP)+1);

           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                        "WFD NetUtils: LocalIP from socket  = %s", IP);
           return 0;
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "WFD NetUtils:getsockname failed");
    }
    return -1;
}

int socketClose(int nSock)
{
    if(nSock > 0)
    {
        return close(nSock);
    }
    else
        return -1;
}
