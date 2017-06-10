/******************************************************************************

                        QMI_IP_MULTICAST_LISTENER.C

******************************************************************************/

/******************************************************************************

  @file    qmi_ip.c
  @brief   Qualcomm mapping interface over IP Multicast Listener

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include "qmi_ip.h"

/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  MULTICAST LISTENER
===========================================================================*/
/*!
@brief

  Listens for mulitcast packets.

@return

  - Waits for multicast packet from GW
  - Checks SSL connection heartbeat
  - Starts SSL connection

@note

  - Dependencies
    - eth0 up

  - Side Effects
    - None
*/
/*=========================================================================*/

void* multicast_listener(void){
    struct addrinfo* multi;
    struct sockaddr_in6 serv;
    int sd ,rc;
    char databuf[1024] = "foobar";
    int datalen = sizeof(databuf);
    struct ipv6_mreq group;
    struct addrinfo hint = {0};
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE;
    struct sockaddr_in6 addrFrom;
    char ipstr[1024];
    socklen_t len = sizeof(addrFrom);
    int resp_s;
    struct sockaddr_in6 dest;
    int odu_iface = 0;
    FILE *fp;

    multicast_server_finished = 0;

    while(fp == NULL){
        sleep(1);
        //check to see if server should shut down
        if (multicast_server_finished)
            return;

        fp = fopen(ODU_NET_CARRIER, "r");
    }
    fclose(fp);

    if (getaddrinfo("FF02::1", "5005", &hint, &multi) != 0){
        LOG_MSG_ERROR("getaddrinfo...ERROR",0,0,0);
        return;
    }

    sd = socket(AF_INET6, SOCK_DGRAM, 0);

    if(sd < 0)
    {
        LOG_MSG_ERROR("Opening the datagram socket...ERROR",0,0,0);
        return;
    }
    else
        LOG_MSG_INFO2("Opening the datagram socket...OK.",0,0,0);

    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(5005);
    serv.sin6_flowinfo = 0;
    if (device_mode == BRIDGE_MODE)
        serv.sin6_scope_id = if_nametoindex("odu0");
    else
        serv.sin6_scope_id = if_nametoindex("bridge0");

    inet_pton(AF_INET6, "FF02::1", (void *)&serv.sin6_addr);

    if(bind(sd, (struct sockaddr*) &serv, sizeof(serv)) == -1)
    {
        close(sd);
        LOG_MSG_ERROR("bind...ERROR",0,0,0);
        return;
    }
    else
        LOG_MSG_INFO2("bind to address... OK",0,0,0);

    memset(&group, 0, sizeof(group));
    memcpy(&group.ipv6mr_multiaddr, &((struct sockaddr_in6*) multi->ai_addr)->sin6_addr, sizeof(group.ipv6mr_multiaddr));
    group.ipv6mr_interface = 3;
    if(setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&group, sizeof(group)) < 0)
    {
        LOG_MSG_ERROR("setsockopt...ERROR",0,0,0);
        close(sd);
        return;
    }
    else
        LOG_MSG_INFO2("Adding multicast group...OK.",0,0,0);

    while(!multicast_server_finished)
    {
        memset(databuf, ' ', 1024);
        if(recvfrom(sd, databuf, datalen, 0, (struct sockaddr *)&addrFrom, &len) < 0)
        {
            LOG_MSG_ERROR("Reading datagram message error",0,0,0);
        }
        else
        {
            LOG_MSG_INFO1("received multicast pkts",0,0,0);
            inet_ntop(AF_INET6, &addrFrom.sin6_addr, ipstr, 46);
            if(!sslConnecting)
            {
                //do we currently have a connection?
                if (acceptTCPConnection) {
                    //check for heartbeat
                    LOG_MSG_INFO2("Perform heartbeat check", 0,0,0);
                    heartbeat_check = 1;
                    heartbeat_response = 0;

                    //wait on response
                    rc = pthread_mutex_lock(&mutex);
                    while (heartbeat_response == 0) {
                        rc = pthread_cond_wait(&cond, &mutex);
                    }
                    rc = pthread_mutex_unlock(&mutex);

                    /* we have a response, stop checking for heartbeat in main thread */
                    heartbeat_check = 0;

                    if (heartbeat_response == -1) {
                        LOG_MSG_ERROR("GW is still running, ignoring packet",0,0,0);
                        continue;
                    }

                    LOG_MSG_INFO1("GW is down accept new connection",0,0,0);
                }
                LOG_MSG_INFO1("Accept new TCP connection",0,0,0);

                resp_s = socket(AF_INET6, SOCK_DGRAM, 0);
                memset (&dest, '\0', sizeof(dest));
                dest.sin6_family      = AF_INET6;
                inet_pton(AF_INET6, ipstr, &dest.sin6_addr);
                dest.sin6_port        = htons     (5005);          /* Server Port number */
                dest.sin6_flowinfo = 0;
                if (device_mode == BRIDGE_MODE)
                    dest.sin6_scope_id = if_nametoindex("odu0");
                else
                    dest.sin6_scope_id = if_nametoindex("bridge0");

                memcpy(databuf, "foobar", sizeof("foobar"));
                if(sendto(resp_s, databuf, datalen, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
                   LOG_MSG_ERROR("Sending datagram message error",0,0,0);

                rc = pthread_mutex_lock(&mutex);
                acceptTCPConnection = 1;
                rc = pthread_cond_broadcast(&cond);
                rc = pthread_mutex_unlock(&mutex);

                close(resp_s);
            }
            else
                LOG_MSG_ERROR("SSL is connecting, skipping this packet", 0,0,0);
        }
    }

    close(sd);
    LOG_MSG_ERROR("Multicast server down",0,0,0);
}
