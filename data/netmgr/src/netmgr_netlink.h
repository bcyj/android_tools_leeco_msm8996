/******************************************************************************

                        N E T M G R _ N E T L I N K . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_netlink.h
  @brief   Network Manager Netlink messaging Header File

  DESCRIPTION
  Header file for NetMgr Netlink messaging functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010, 2013, 2014 Qualcomm Technologies, Inc. All Rights Reserved

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

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/21/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_NETLINK_H__
#define __NETMGR_NETLINK_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/xfrm.h>
#include <netinet/in.h>

#include "comdef.h"
#include "ds_util.h"
#include "netmgr.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NL_MSG_MAX_LEN (2048)

/*---------------------------------------------------------------------------
   Constants for NetLink message attribute types
---------------------------------------------------------------------------*/
#define NETMGR_NL_TYPE          NETLINK_GENERIC         /* Netlink type used */
#define NETMGR_NL_GRP_EVENTS    (1<<30)   /* Multicast group for indications */
#define NETMGR_NL_EVENT         (0x01)
#define NETMGR_XFRM_TYPE        NETLINK_XFRM
#define NETMGR_XFRM_GRP_EVENTS  (1 << (XFRMNLGRP_AEVENTS-1))

#define NETMGR_RTATYPE_BASE     (0x1000)
#define NETMGR_RTATYPE_EVENT    (NETMGR_RTATYPE_BASE+0) /* Event ID           */
#define NETMGR_RTATYPE_LINK     (NETMGR_RTATYPE_BASE+1) /* Interface linkID   */
#define NETMGR_RTATYPE_FLOWINFO (NETMGR_RTATYPE_BASE+2) /* Modem QoS flow info*/
#define NETMGR_RTATYPE_ADDRINFO (NETMGR_RTATYPE_BASE+3) /* Interface address
                                                           + Subnet/PrefixLen */
#define NETMGR_RTATYPE_GTWYINFO (NETMGR_RTATYPE_BASE+4) /* Gateway address    */
#define NETMGR_RTATYPE_DNSPADDR (NETMGR_RTATYPE_BASE+5) /* Primary DNS addr   */
#define NETMGR_RTATYPE_DNSSADDR (NETMGR_RTATYPE_BASE+6) /* Secondary DNS addr */
#define NETMGR_RTATYPE_DEVNAME  (NETMGR_RTATYPE_BASE+7) /* Device Name        */
#define NETMGR_RTATYPE_USER_CMD (NETMGR_RTATYPE_BASE+8) /* User Command       */
#define NETMGR_RTATYPE_CMD_DATA (NETMGR_RTATYPE_BASE+9) /* Command data for
                                                            user command*/
#define NETMGR_RTATYPE_MTU      (NETMGR_RTATYPE_BASE+10)/* Device MTU         */

#define NETMGR_RTASIZE_EVENT      RTA_SPACE( sizeof(netmgr_nl_events_t) )
#define NETMGR_RTASIZE_LINK       RTA_SPACE( sizeof(netmgr_link_id_t) )
#define NETMGR_RTASIZE_FLOWINFO   RTA_SPACE( sizeof(netmgr_nl_flowinfo_t) )
#define NETMGR_RTASIZE_ADDRINFO   RTA_SPACE( sizeof(netmgr_nl_addrinfo_t) )
#define NETMGR_RTASIZE_GTWYINFO   RTA_SPACE( sizeof(netmgr_nl_addr_t) )
#define NETMGR_RTASIZE_DNSPADDR   RTA_SPACE( sizeof(struct sockaddr_storage) )
#define NETMGR_RTASIZE_DNSSADDR   RTA_SPACE( sizeof(struct sockaddr_storage) )
#define NETMGR_RTASIZE_DEVNAME    RTA_SPACE( NETMGR_IF_NAME_MAX_LEN )
#define NETMGR_RTASIZE_MTU        RTA_SPACE( sizeof(unsigned int))
#define NETMGR_RTASIZE_USER_CMD   RTA_SPACE( sizeof(unsigned int))
#define NETMGR_RTASIZE_CMD_DATA   RTA_SPACE( sizeof(netmgr_user_cmd_data_t))

/*---------------------------------------------------------------------------
   Types representing parsed NetLink message
---------------------------------------------------------------------------*/
#define NETMGR_NL_PARAM_NONE         (0x0000)
#define NETMGR_NL_PARAM_EVENT        (0x0001)
#define NETMGR_NL_PARAM_LINK         (0x0002)
#define NETMGR_NL_PARAM_ADDR         (0x0004)
#define NETMGR_NL_PARAM_PREFIX       (0x0008)

#define NETMGR_XFRM_PARAM_ESPTHRESH  (0x0010)

#define NETMGR_NLA_PARAM_NONE        (0x0000)
#define NETMGR_NLA_PARAM_PREFIXADDR  (0x0001)
#define NETMGR_NLA_PARAM_LOCALADDR   (0x0002)
#define NETMGR_NLA_PARAM_LABELNAME   (0x0004)
#define NETMGR_NLA_PARAM_BCASTADDR   (0x0008)
#define NETMGR_NLA_PARAM_ACASTADDR   (0x0010)
#define NETMGR_NLA_PARAM_MCASTADDR   (0x0020)
#define NETMGR_NLA_PARAM_CACHEINFO   (0x0080)
#define NETMGR_NLA_PARAM_PROTOINFO   (0x0100)
#define NETMGR_NLA_PARAM_FLAGS       (0x0200)

typedef struct netmgr_nl_proto_info_s {
  unsigned int                    param_mask;
  unsigned int                    flags;
  struct ifla_cacheinfo           cache_info;
} netmgr_nl_proto_info_t;

typedef struct netmgr_nl_addr_info_s {
  struct ifaddrmsg                metainfo;     /* from header */
  struct {                                      /* attributes  */
    unsigned int                  param_mask;
    unsigned char                 label_name[NETMGR_IF_NAME_MAX_LEN];
    struct sockaddr_storage       prefix_addr;
    struct sockaddr_storage       local_addr;
    struct sockaddr_storage       bcast_addr;
    struct sockaddr_storage       acast_addr;
    struct sockaddr_storage       mcast_addr;
    struct ifa_cacheinfo          cache_info;
    netmgr_nl_proto_info_t        proto_info;
  } attr_info;
} netmgr_nl_addr_info_t;

typedef struct netmgr_nl_link_info_s {
  struct ifinfomsg                metainfo;     /* from header */
} netmgr_nl_link_info_t;

/* This struct must match struct prefix_info in kernel/include/net/autoconf.h */
typedef struct netmgr_nl_prefix_s {
  unsigned char              type;
  unsigned char              length;
  unsigned char              prefix_len;
  unsigned char              reserved : 6,
                             autoconf : 1,
                             onlink : 1;
  unsigned char              valid;
  unsigned char              prefered;
  unsigned char              reserved2;
  struct in6_addr            prefix;
} netmgr_nl_prefix_t;

typedef struct netmgr_nl_prefix_info_s {
  struct prefixmsg           metainfo;       /* from header */
  netmgr_nl_prefix_t         prefixinfo;     /* attributes  */
  struct prefix_cacheinfo    cacheinfo;
} netmgr_nl_prefix_info_t;

/* Struct for XFRM events */
typedef struct netmgr_nl_xfrm_info_s {
  struct xfrm_aevent_id     xfrm_aevent;
  struct xfrm_replay_state  xfrm_replay;
} netmgr_nl_xfrm_info_t;

typedef struct netmgr_nl_msg_s {
  unsigned int               type;
  unsigned int               param_mask;
  /* Optional parameters */
  netmgr_nl_event_info_t     event_info;
  netmgr_nl_link_info_t      link_info;
  netmgr_nl_addr_info_t      addr_info;
  netmgr_nl_prefix_info_t    prefix_info;
  netmgr_nl_xfrm_info_t      xfrm_info;

#if 0 /* TBD */
  void *                     route_info;
  void *                     neigh_info;
  void *                     tc_rule_info;
  void *                     tc_qdisc_info;
  void *                     tc_class_info;
  void *                     tc_filter_info;
#endif
} netmgr_nl_msg_t;

/*---------------------------------------------------------------------------
   Type representing collection of control info related to a netlink socket
---------------------------------------------------------------------------*/
typedef struct {
  int                 sk_fd;       /* socket descriptor */
  struct sockaddr_nl  sk_addr_loc; /* local address of socket */
  struct sockaddr_nl  sk_addr_rem; /* remote endpoint's address */
} netmgr_nl_sk_info_t;

/*---------------------------------------------------------------------------
   Type representing function callback registered with a socket listener
   thread for reading from a socket on receipt of an incoming message
---------------------------------------------------------------------------*/
typedef int (* netmgr_socklthrd_fd_read_f) (int fd);


/*---------------------------------------------------------------------------
   Type representing collection of info registered with a socket listener
   thread by clients
---------------------------------------------------------------------------*/
typedef struct {
  int fd;                            /* Socket descriptor */
  netmgr_socklthrd_fd_read_f read_f; /* Incoming data notify handler */
} netmgr_socklthrd_fdmap_t;

/*---------------------------------------------------------------------------
   Type representing a handle to a socket listener thread. Clients must
   use this as an opaque pointer.
---------------------------------------------------------------------------*/
typedef struct {
  pthread_t               thrd;      /* Pthread object for the thread */
  netmgr_socklthrd_fdmap_t * fdmap;  /* Ptr to array of fdmap structs */
  unsigned int            nfd;       /* Number of valid fds in fdmap */
  unsigned int            maxnfd;    /* Size of fdmap array */
  int                     maxfd;     /* Maximum valued fd */
  fd_set                  fdset;     /* fdset used in select() operation */
  boolean                 running;   /* Flag for processing thread state */
} netmgr_socklthrd_info_t;


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/



/*===========================================================================
  FUNCTION  netmgr_nl_alloc_msg
===========================================================================*/
/*!
@brief
  Allocate a NETLINK message structure from dynamic memory.
  The msglen parameter denotes the size of the message payload in bytes.
  Client is responsible for releasing message dynamic memory.

@return
  struct msghdr * - Pointer on successful allocaiton, NULL otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - Memory is allocated from system heap.
*/
/*=========================================================================*/
struct msghdr * netmgr_nl_alloc_msg
(
  uint32  msglen
);

/*===========================================================================
  FUNCTION  netmgr_nl_release_msg
===========================================================================*/
/*!
@brief
  Release a NETLINK message structure from dynamic memory.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Memory is returned to system heap
*/
/*=========================================================================*/
void netmgr_nl_release_msg
(
  struct msghdr * msgh
);

/*===========================================================================
  FUNCTION  netmgr_nl_recv_msg
===========================================================================*/
/*!
@brief
  Reads a complete NETLINK message incoming over the specified socket
  descriptor and returns it. Note that the memory for the message is
  dynamically allocated.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Message buffer allocated from system heap
*/
/*=========================================================================*/
int netmgr_nl_recv_msg
(
  int              fd,
  struct msghdr ** msg_pptr,
  unsigned int  *  msglen_ptr
);


/*===========================================================================
  FUNCTION  netmgr_nl_open_sock
===========================================================================*/
/*!
@brief
  Opens a netlink socket for the specified protocol and multicast group
  memberships.

@return
  int - NETMGR_SUCCESS if socket is successfully opened,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_open_sock
(
  netmgr_nl_sk_info_t * sk_info,
  int proto,
  unsigned int grps
);


/*===========================================================================
  FUNCTION  netmgr_nl_send_msg
===========================================================================*/
/*!
@brief
  Writes a complete NETLINK message over the specified socket
  descriptor. The specified buffer is assumed to conform with NETLINK
  message format specified in include/net/netlink.h

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_send_msg( int fd, void* buffer, uint16 buflen );


/*===========================================================================
  FUNCTION  netmgr_nl_encode_netmgr_event
===========================================================================*/
/*!
@brief
  Encode event parameters into Netlink attribute TLVs.  Octet buffer
  is allocated internally and returned to caller; caller must release
  buffer when done with it.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Buffer allocated from dynamic memory
*/
/*=========================================================================*/
int netmgr_nl_encode_netmgr_event
(
  const netmgr_nl_event_info_t *event_info,
  char                        **buffer,
  unsigned int                 *buflen
);


/*===========================================================================
  FUNCTION  netmgr_nl_decode_netmgr_event
===========================================================================*/
/*!
@brief
  Decode NetMgr event parameters from Netlink attribute TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_decode_netmgr_event
(
  const char              *buffer,
  unsigned int             buflen,
  netmgr_nl_event_info_t  *event_info
);


/*===========================================================================
  FUNCTION  netmgr_nl_decode_nlmsg
===========================================================================*/
/*!
@brief
  Decode Netlink message TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_decode_nlmsg
(
  const char         *buffer,
  unsigned int        buflen,
  netmgr_nl_msg_t    *msg_ptr
);


/*===========================================================================
  FUNCTION  netmgr_nl_listener_init
===========================================================================*/
/*!
@brief
  Initialization routine for listener on NetLink sockets interface.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Listening thread is created
*/
/*=========================================================================*/
int netmgr_nl_listener_init
(
  netmgr_socklthrd_info_t  * sk_thrd_info,
  netmgr_socklthrd_fdmap_t * sk_thrd_fdmap,
  unsigned int               sk_thrd_fdmap_size,
  netmgr_nl_sk_info_t      * sk_info,
  unsigned int               nl_type,
  unsigned int               nl_groups,
  netmgr_socklthrd_fd_read_f read_f
);

/*===========================================================================
  FUNCTION  netmgr_nl_listener_teardown
===========================================================================*/
/*!
@brief
  Teardown routine for NetLink socket interface message listener.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Listening thread is created
*/
/*=========================================================================*/
int
netmgr_nl_listener_teardown
(
  netmgr_socklthrd_info_t   * sk_thrd_info,
  netmgr_nl_sk_info_t       * rt_sk
);

#endif /* __NETMGR_NETLINK_H__ */
