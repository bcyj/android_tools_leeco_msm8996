#ifndef _QCMAP_NETLINK_H_
#define _QCMAP_NETLINK_H_

/******************************************************************************

                           QCMAP_NETLINK.H

******************************************************************************/

/******************************************************************************

  @file    qcmap_netlink.h
  @brief   Mobile AP Netlink Socket API

  DESCRIPTION
  Header file for QCMAP NETLINK.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
10/01/13   pd         Created. Followed IPACM and QTI_V2 coding conventions.
03/27/14   cp         Added support to DUN+SoftAP.
******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <netinet/ether.h>

/*===========================================================================
                              MACRO DEFINITIONS
===========================================================================*/
#define QCMAP_NL_RNDIS_INTERFACE            "rndis0"
#define QCMAP_NL_ECM_INTERFACE              "ecm0"
#define QCMAP_NL_WLAN_INTERFACE             "wlan0"
#define QCMAP_NL_PPP_INTERFACE              "ppp0"
#define QCMAP_NL_MAX_NUM_OF_FD              10
#define QCMAP_NL_MSG_MAX_LEN                1024
#define IF_NAME_LEN                         16
#define QCMAP_NL_SUCCESS                    0
#define QCMAP_NL_FAILURE                    (-1)

/*===========================================================================
                              VARIABLE DECLARARTIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Function pointer registered with the socket listener
   This function is used for reading from a socket on receipt of an incoming
   netlink event
---------------------------------------------------------------------------*/
typedef int (* qcmap_sock_thrd_fd_read_f) (int fd);

/*--------------------------------------------------------------------------
   Stores the mapping of a socket descriptor and its associated read
   function
---------------------------------------------------------------------------*/
typedef struct
{
 int sk_fd;
 qcmap_sock_thrd_fd_read_f read_func;
} qcmap_nl_sk_fd_map_info_t;

/*--------------------------------------------------------------------------
   Stores the socket information associated with netlink sockets required
   to listen to netlink events
---------------------------------------------------------------------------*/
typedef struct
{
 qcmap_nl_sk_fd_map_info_t sk_fds[QCMAP_NL_MAX_NUM_OF_FD];
 fd_set fdset;
 int num_fd;
 int max_fd;
} qcmap_nl_sk_fd_set_info_t;

/*--------------------------------------------------------------------------
   Socket descriptor paramters
---------------------------------------------------------------------------*/
typedef struct
{
 int                 sk_fd;       /* socket descriptor */
 struct sockaddr_nl  sk_addr_loc; /*  stores socket parameters */
} qcmap_nl_sk_info_t;

/*--------------------------------------------------------------------------
   Packet socket descriptor paramters
---------------------------------------------------------------------------*/
typedef struct
{
 int                 sk_fd;       /* socket descriptor */
} qcmap_pf_sk_info_t;

/*--------------------------------------------------------------------------
   Stoes the metainfo present in the incoming netlink message
---------------------------------------------------------------------------*/
typedef struct
{
 struct ifinfomsg  metainfo;
} qcmap_nl_link_info_t;

/*-------------------------------------------------------------------------
   Stores the address info present in incoming netlink message
--------------------------------------------------------------------------*/
typedef struct qcmap_nl_if_addr_s {
        struct ifaddrmsg          metainfo;      /*from header*/
        struct                                   /*attributes*/
        {
          uint32_t               ifa_local;
        } attr_info;
} qcmap_nl_if_addr_t;

/*--------------------------------------------------------------------------
   Stoes the neighbor info present in the incoming netlink message
---------------------------------------------------------------------------*/
typedef struct qcmap_nl_neigh_info_s {
        struct ndmsg                  metainfo;     /* from header */
        struct                                      /* attributes  */
        {
          unsigned int                param_mask;
          struct sockaddr_storage     local_addr;
          struct  sockaddr            lladdr_hwaddr;
        } attr_info;
} qcmap_nl_neigh_info_t;

/*--------------------------------------------------------------------------
   Stoes the Route info present in the incoming netlink message
---------------------------------------------------------------------------*/
#define QCMAP_NL_ROUTE_INFO_DST_ADDR 0x0001
#define QCMAP_NL_ROUTE_INFO_IFINDEX 0x0002
typedef struct qcmap_nl_route_info_s {
        struct rtmsg                   metainfo;     /* from header */
        struct                                      /* attributes  */
        {
          unsigned int                 param_mask;
          struct in6_addr              dst_addr;
          int                          ifindex;
        } attr_info;
} qcmap_nl_route_info_t;

/*--------------------------------------------------------------------------
   Netlink message: used to decode the incoming netlink message
---------------------------------------------------------------------------*/
typedef struct
{
 unsigned int type;
 bool link_event;
 qcmap_nl_neigh_info_t  nl_neigh_info;
 qcmap_nl_if_addr_t nl_if_addr;
 qcmap_nl_route_info_t  nl_route_info;
}qcmap_nl_msg_t;

/*--------------------------------------------------------------------------
  Netlink message structure used to send GET_LINK
---------------------------------------------------------------------------*/
typedef struct
{
  struct nlmsghdr hdr;
  struct rtgenmsg gen;
}qcmap_nl_req_type;

/*===========================================================================
                       FUNCTION DECLARATIONS
===========================================================================*/
/*===========================================================================
FUNCTION QCMAP_NL_LISTENER_INIT()

DESCRIPTION

  This function initializes netlink sockets and also performs a query to find
  any netlink events that could happened before netlink socket
  initialization.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None
=========================================================================*/
int qcmap_nl_listener_init
(
  unsigned int nl_type,
  unsigned int nl_groups,
  qcmap_nl_sk_fd_set_info_t * sk_fdset,
  qcmap_sock_thrd_fd_read_f read_f
);

/*===========================================================================
FUNCTION QCMAP_NA_PACKET_SOCKET_INIT()

DESCRIPTION

  This function initializes packet socket to listen for RA packets.
DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None
=========================================================================*/
int qcmap_packet_socket_init
(
  qcmap_nl_sk_fd_set_info_t * sk_fdset,
  qcmap_sock_thrd_fd_read_f read_f
);

/*===========================================================================
FUNCTION QCMAP_NL_RECV_MSG()

DESCRIPTION

  Function to receive incoming messages over the NETLINK routing socket.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None
==========================================================================*/
int qcmap_nl_recv_msg(int fd);

/*===========================================================================

FUNCTION QCMAP_PACKET_SOCK_RECV_MSG()

DESCRIPTION

  This function
  - receives the packet socket message.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qcmap_packet_sock_recv_msg(int fd);

/*===========================================================================
FUNCTION QCMAP_NL_SOCK_LISTENER_START()

DESCRIPTION

  This function
  - calls the select system call and listens to netlink events coming on
    netlink socket and call the appropriate callback for received packets

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qcmap_nl_sock_listener_start(qcmap_nl_sk_fd_set_info_t    *sk_fd_set);

/*===========================================================================
FUNCTION QCMAP_NL_SEND_GETNEIGH_EVENT()

DESCRIPTION

  This function
  - Send RTM_GETNEIGH message for bridge interface

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None
==========================================================================*/
int qcmap_nl_send_getneigh_event(void);

#endif /*_QCMAP_NETLINK_H_*/
