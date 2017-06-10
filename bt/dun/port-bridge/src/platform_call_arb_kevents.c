/******************************************************************************

  @file    platform_call_arb_kevents.c
  @brief   Platform Arbitration kernel events

  DESCRIPTION
  This function deals with the rmnet kernel events.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

  ===========================================================================*/
/*Including necessary header files*/
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <CommandApi.h>

/*Header File Declarations*/
#include "portbridge_common.h"
#include "platform_call_arb.h"
#include "platform_call_arb_kevents.h"

/*The number of bytes to be read from rmnet netlink socket*/
#define RMNET_BUF_LEN 1024
#define MAX_IFS 8

#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV

/*Rmnet netlink socket*/
static int route_sock;
/*Rmnet monitoring thread*/
static pthread_t rmnet_kevent_thread;
/*Buffer to store whats read from rmnet socket*/
static char rmnet_buf[RMNET_BUF_LEN];
/*state to check if embedded data call disabled
 * or not
 */
static boolean bt_dun_disconnected_embedded_call = FALSE;

/*===========================================================================

FUNCTION     : pb_rmnet_port_thread_exit_handler

DESCRIPTION  : This function does rmnet monitor thread exit handling

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void pb_rmnet_port_thread_exit_handler(int sig)
{
    if(sig == SIGUSR1) {
        pthread_exit(0);
    }
    else {
        port_log_err("Error in rmnet monitor exit handler! Sig value not SIGUSR1");
    }
}

/*===========================================================================

FUNCTION     : pb_rmnet_interface_check_status

DESCRIPTION  : This function checks current rmnet status for up/down

DEPENDENCIES : None

RETURN VALUE : RMNETSTATE_EVENT_E reflecting the rmnet Status

============================================================================*/
RMNETSTATE_EVENT_E pb_rmnet_interface_check_status(void)
{
    struct ifreq *ifr;
    struct ifreq *ifend;
    struct ifreq ifs[MAX_IFS];
    struct ifconf ifc;
    int sock_fd;

    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if ( sock_fd < 0) {
        port_log_err("Error while opening socket!\n");
        return RMNETSTATE_ERROR;
    }

    /*Returns all configured IP interfaces*/
    if (ioctl(sock_fd, SIOCGIFCONF, &ifc) < 0) {
        port_log_err("ioctl(SIOCGIFCONF) failed \n");
        close(sock_fd);
        return RMNETSTATE_ERROR;
    }

    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));

    /* Run through all available interfaces */
    for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
        if (ifr->ifr_addr.sa_family == AF_INET) {
            /* Filter loopback*/
            port_log_low("Available interface: %s", ifr->ifr_name);
            if (!strncmp(pb_dun_phys_net_dev_name, ifr->ifr_name,
                         strlen(pb_dun_phys_net_dev_name))) {
                /*Get the interface flags*/
                if (ioctl(sock_fd, SIOCGIFFLAGS, ifr) < 0) {
                    port_log_err("Ioctl(SIOCGIFFLAGS) failed\n");
                    close(sock_fd);
                    return RMNETSTATE_ERROR;
                }
                if (IFF_UP & ifr->ifr_flags ){
                    close(sock_fd);
                    return RMNETSTATE_UP;
                }
                else{
                    close(sock_fd);
                    return RMNETSTATE_DOWN;
                }
            }
        }
    }
    close(sock_fd);
    return RMNETSTATE_DOWN;
}



/*===========================================================================

FUNCTION     : pb_rmnet_open_route_sock

DESCRIPTION  : This function opens & binds route sock for the rmnet interface

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
static int pb_rmnet_open_route_sock(void)
{
    struct sockaddr_nl nlsock;

    memset(&nlsock, 0, sizeof(nlsock));
    nlsock.nl_family = AF_NETLINK;
    nlsock.nl_pid = getpid();
    nlsock.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    /*Open a netlink socket for the rmnet*/
    if((route_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
        port_log_err("Unable to create route socket: %s\n", strerror(errno));
        return -1;
    }

    if(bind(route_sock, (struct sockaddr*)&nlsock, sizeof(nlsock)) < 0) {
        port_log_err("Unable to bind route socket: %s\n", strerror(errno));
        close(route_sock);
        return -1;
    }
    return 0;
}


/*===========================================================================

FUNCTION     : pb_rmnet_monitor_kevents

DESCRIPTION  : This function monitors rmnet network status update events
on netlink socket.

DEPENDENCIES : None

RETURN VALUE : void function pointer to the rmnet monitor thread

============================================================================*/
static void *pb_rmnet_monitor_kevents(void *arg)
{
    RMNETSTATE_EVENT_E rstate;
    int cnt;
    struct msghdr msg;
    struct iovec iov;
    struct sigaction actions;
    fd_set fds;

    /*Adding its own thread handler*/
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = pb_rmnet_port_thread_exit_handler;

    memset((void *)rmnet_buf, 0, RMNET_BUF_LEN);
    memset((void *)&msg, 0, sizeof(msg));
    memset((void *)&iov, 0, sizeof(iov));
    iov.iov_base = (void *)rmnet_buf;
    iov.iov_len = sizeof(rmnet_buf);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (sigaction(SIGUSR1,&actions,NULL) < 0) {
        port_log_err("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
    }

    /* open netlink route socket */
    if(pb_rmnet_open_route_sock() < 0) {
        port_log_err("Error while opening the netlink route socket\n");
        return NULL;
    }

    /* post initial netowrk state to control thread */
    rstate =  pb_rmnet_interface_check_status();

    /* When phone is rebooted the rmnet status can return ERROR.
     * We should proceed further and  register for netlink route socket */
    /*   if(rstate == RMNETSTATE_ERROR) {
         port_log_err("rstate == PLATFORM_RMNETSTATE_ERROR in %s\n", __func__);
         }
         else if(rstate == RMNETSTATE_DOWN){
         port_log_high("Rmnet event down posted from the monitor thread");
         post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_DOWN);
         }
         else{
         port_log_high("Rmnet event up posted from the monitor thread");
         post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_UP);
         } */

    /*Select on the rmnet interface*/
    while(1) {
        FD_ZERO(&fds);
        FD_SET(route_sock, &fds);

        if (select(route_sock + 1, &fds, NULL, NULL, NULL) < 0) {
            port_log_err("select() failed (%s)", strerror(errno));
            return NULL;
        }

        if(FD_ISSET(route_sock, &fds))
        {
            /*Read from the socket*/
            cnt = recvmsg(route_sock, &msg, 0);

            if (cnt < 0 ) {
                port_log_err("recvmsg failed in netlink: %s\n", strerror(errno));
                return NULL;
            }

            /*Check for the rmnet status*/
            rstate =  pb_rmnet_interface_check_status();

            if(rstate == RMNETSTATE_UP){
                post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_UP);
                bt_dun_disconnected_embedded_call = FALSE;
            }
            else if(rstate == RMNETSTATE_DOWN){
                post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_DOWN);
            }
            else if(rstate == RMNETSTATE_ERROR) {
                post_rmnet_event_to_platform(PLATFORM_EVENT_ERROR);
            }
        }
        else
            port_log_high("Error: fd set was unsuccesul");
    }

    return NULL;
}

/*===========================================================================

FUNCTION     : pb_start_rmnet_mon_thread

DESCRIPTION  : Starts rmnet monitoring thread

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
int pb_start_rmnet_mon_thread(void)
{
    if( pthread_create(&rmnet_kevent_thread, NULL,
                pb_rmnet_monitor_kevents,(void *)NULL) != 0) {
        port_log_err("Unable to create rmnet mon thread : %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
/*===========================================================================

FUNCTION     : pb_stop_rmnet_mon_thread

DESCRIPTION  : Kills rmnet monitoring thread

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

void pb_stop_rmnet_mon_thread(void)
{
    int status;

    close(route_sock);

    if((status = pthread_kill(rmnet_kevent_thread, SIGUSR1)) != 0) {
        port_log_err("Error cancelling thread %d, error = %d (%s)",
                (int)rmnet_kevent_thread, status, strerror(status));
    }

    if((status = pthread_join(rmnet_kevent_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)rmnet_kevent_thread, status, strerror(status));
    }
}

/*===========================================================================

FUNCTION     : pb_disable_embedded_data_call

DESCRIPTION  : This function disables embedded data connection. Required for
single PDP context. For multiple PDP contexts data connection
and DUN connection can coexist.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void pb_disable_embedded_data_call(void)
{

    RMNETSTATE_EVENT_E inf_status;

    /*For multiple PDP contexts, the DUN and data connection can co-exist*/
    if (!SINGLE_PDP) {
        port_log_low("For Multiple PDP, posting RMNET_DOWN event w/o writing DUN_INITIATED ");
        post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_DOWN);
        return;
    }

    inf_status = pb_rmnet_interface_check_status();
    if ((RMNETSTATE_DOWN == inf_status) || (RMNETSTATE_ERROR == inf_status)) {
        port_log_low("%s is down, posting RMNET_DOWN event w/o writing DUN_INITIATED 0x%02x",
            pb_dun_phys_net_dev_name, inf_status);
        post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_DOWN);
        return;
    }

    enableDataConnectivity(DUN_INITIATED);
    port_log_low("set bt_dun_disconnected_embedded_call to TRUE");
    bt_dun_disconnected_embedded_call = TRUE;

}

/*===========================================================================

FUNCTION     : pb_enable_embedded_data_call

DESCRIPTION  : This function enables embedded data connection. Required for
single PDP context. For multiple PDP contexts data connection
and DUN connection can coexist.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void pb_enable_embedded_data_call(void)
{

    RMNETSTATE_EVENT_E inf_status;

    if (!SINGLE_PDP) {
        port_log_low("For Multiple PDP, posting RMNET_UP event w/o writing DUN_END ");
        post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_UP);
        return;
    }

    if (bt_dun_disconnected_embedded_call == FALSE) {
        /*don't need to connect the embeeded call back
          if it was not disconnected by BT-DUN*/
        port_log_high("bt_dun_disconnected_embedded_call is FALSE, hence no reconnection");
        return;
    }

    inf_status = pb_rmnet_interface_check_status();
    if (RMNETSTATE_UP == inf_status) {
        port_log_low("%s is UP, posting RMNET_UP event w/o writing DUN_END 0x%02x",
            pb_dun_phys_net_dev_name, inf_status);
        post_rmnet_event_to_platform(PLATFORM_EVENT_RMNET_UP);
        return;
    }

    enableDataConnectivity(DUN_END);

}
