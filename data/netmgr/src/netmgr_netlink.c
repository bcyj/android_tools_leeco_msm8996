/******************************************************************************

                        N E T M G R _ N E T L I N K . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_netlink.c
  @brief   Network Manager Netlink messaging Implementation File

  DESCRIPTION
  Implementation file for NetMgr Netlink messaging functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010, 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#ifndef NETMGR_OFFTARGET
#include <netinet/in.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>

#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_util.h"
#include "netmgr_netlink.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* NETLINK message originator */
#define NETMGR_NL_PID_MASK  (0x7FFFFFFF)
#define NETMGR_NL_PID       (getpid() & NETMGR_NL_PID_MASK)
//#define NETMGR_NL_PID     ((getpid() | (pthread_self() << 16)) & NETMGR_NL_PID_MASK)

/* NETLINK message sequence number */
uint32                   nl_seqno = 0;

#define NETMGR_NL_RTA_NEXT( type, len )            \
        rtah->rta_type = type;                     \
        rtah->rta_len = len;                       \
        rtad = RTA_DATA(rtah);

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))

#define NETMGR_NL_COPY_ADDR( name, mask, element )                                        \
        addr_info->attr_info.element.ss_family = addr_info->metainfo.ifa_family;          \
        memcpy( &addr_info->attr_info.element.__data,                                     \
                RTA_DATA(rtah),                                                           \
                sizeof(addr_info->attr_info.element.__data) );                            \
        addr_info->attr_info.param_mask |= mask;                                          \
        NETMGR_NL_REPORT_ADDR( med, "Attribute: " name, addr_info->attr_info.element );

#else/*(!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))*/

#define NETMGR_NL_COPY_ADDR( name, mask, element )                                        \
        addr_info->attr_info.element.ss_family = addr_info->metainfo.ifa_family;          \
        memcpy( &addr_info->attr_info.element.__ss_padding,                               \
                RTA_DATA(rtah),                                                           \
                sizeof(addr_info->attr_info.element.__ss_padding) );                      \
        addr_info->attr_info.param_mask |= mask;                                          \
        NETMGR_NL_REPORT_ADDR( med, "Attribute: " name, addr_info->attr_info.element );

#endif /*(!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))*/

#define XFRM_RTA(nlh, x) ((struct rtattr*)((char*)NLMSG_DATA(nlh) + NLMSG_ALIGN(sizeof(x))))
#define XFRM_PAYLOAD(nlh, x) NLMSG_PAYLOAD(nlh, sizeof(x))

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_nl_socklthrd_init
===========================================================================*/
/*!
@brief
  Initializes a Socket Listener thread handle.

@return
  int - 0 on success, -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_socklthrd_init
(
  netmgr_socklthrd_info_t * info,
  netmgr_socklthrd_fdmap_t * fdmap,
  unsigned int maxnfd
)
{
  /* Verify that handle and fdmap ptrs are valid before proceeding */
  if ((info == NULL) || (fdmap == NULL))
  {
    return -1;
  }

  /* Initialize the handle */
  info->fdmap = fdmap;
  info->maxnfd = maxnfd;
  info->nfd = 0;
  info->maxfd = 0;
  FD_ZERO(&info->fdset);

  return 0;
}

/*===========================================================================
  FUNCTION  netmgr_nl_socklthrd_addfd
===========================================================================*/
/*!
@brief
  Adds a socket descriptor to the list of descriptors to read from for a
  socket listener thread represented by the given handle.

@return
  int - 0 on success, -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_nl_socklthrd_addfd
(
  netmgr_socklthrd_info_t * info,
  int fd,
  netmgr_socklthrd_fd_read_f read_f
)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Make sure passed parameters are valid and there is space in the fdmap
  ** array for adding this new fd, before proceeding.
  */
  if ((info == NULL) ||
      (info->fdmap == NULL) ||
      (read_f == NULL) ||
      (info->maxnfd == info->nfd))
  {
    return -1;
  }

  /* Set new fd in fdset */
  FD_SET(fd, &info->fdset);

  /* Add fd to fdmap array and store read handler function ptr */
  (info->fdmap + info->nfd)->fd = fd;
  (info->fdmap + info->nfd)->read_f = read_f;

  /* Increment number of fds stored in fdmap */
  info->nfd++;

  /* Change maxfd if this is the largest valued fd in fdmap */
  if (fd > info->maxfd)
  {
    info->maxfd = fd;
  }

  NETMGR_LOG_FUNC_EXIT;
  return 0;
}

/*===========================================================================
  FUNCTION  netmgr_nl_socklthrd_main
===========================================================================*/
/*!
@brief
  The main function of a Socket Listener thread.

@return
  void * - Does not return

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void *
netmgr_nl_socklthrd_main (void * arg)
{
  netmgr_socklthrd_info_t * info;
  netmgr_socklthrd_fdmap_t * fdmap;
  int ret;
  int i;

  NETMGR_LOG_FUNC_ENTRY;

  /* Make sure handle and handle contents are valid before proceeding. Note
  ** that this checks should all pass since the handle params are verified
  ** in netmgr_socklthrd_start routine.
  */
  info = arg;
  NETMGR_ASSERT(info);
  NETMGR_ASSERT(info->fdmap);
  NETMGR_ASSERT(info->nfd > 0);

  info->running = TRUE;

  fdmap = info->fdmap;

  while( info->running )
  {
    /* Call select to block on incoming message on all registered fds */
    if ((ret = select(info->maxfd+1, &info->fdset, NULL, NULL, NULL)) < 0)
    {
      netmgr_log_err("select returned with errno=[%d,%s]\n",
                     errno,
                     strerror(errno));

      /* Ignore select errors due to EINTR */
      if (-1 == ret && EINTR == errno)
      {
        continue;
      }

      NETMGR_ABORT("netmgr_nl_socklthrd_main: select failed!");
      return NULL;
    }

    if (ret > 0)
    {
      /* One or more fds became readable. Call their respective
      ** notification handlers.
      */
      for (i = 0; (unsigned int)i < info->nfd; ++i)
      {
        if( FD_ISSET((fdmap + i)->fd, &info->fdset) )
        {
          if( (fdmap + i)->read_f )
          {
            if( NETMGR_SUCCESS != (* (fdmap + i)->read_f)((fdmap + i)->fd) ) {
              netmgr_log_err( "Error on read callback[%d] fd=%d\n",
                              i, (fdmap + i)->fd );
            }
          }
        }
      }
    }
    else
    {
      /* For some reason select returned 0 indicating nothing became
      ** readable. Print debug message and continue.
      */
      netmgr_log_high("select returned with 0");
    }
  } /* end of while */

  NETMGR_LOG_FUNC_EXIT;
  return NULL;
}

/*===========================================================================
  FUNCTION  netmgr_nl_socklthrd_start
===========================================================================*/
/*!
@brief
  Starts the socket listener thread and associates it with the specified
  handle.

@return
  int - NSTMGR_SUCCESS on success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - Spawns a pthread for reading data received on associated sockets.
*/
/*=========================================================================*/
int netmgr_nl_socklthrd_start
(
  netmgr_socklthrd_info_t * info
)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Verify that handle is valid and at least one fd was registered with
  ** handle before proceeding.
  */
  if ((info == NULL) || (info->fdmap == NULL) || (info->nfd == 0))
  {
    return NETMGR_FAILURE;
  }

  /* Create and start listener thread */
  if( 0 != pthread_create(&info->thrd, NULL, netmgr_nl_socklthrd_main, info) )
  {
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

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
int
netmgr_nl_open_sock
(
  netmgr_nl_sk_info_t * sk_info,
  int proto,
  unsigned int grps
)
{
  int rval = NETMGR_FAILURE;
  int * p_sk_fd;
  struct sockaddr_nl * p_sk_addr_loc, * p_sk_addr_rem;

  NETMGR_LOG_FUNC_ENTRY;

  p_sk_fd = &sk_info->sk_fd;
  p_sk_addr_loc = &sk_info->sk_addr_loc;
  p_sk_addr_rem = &sk_info->sk_addr_rem;

  /* Open netlink socket for specified protocol */
  if ((*p_sk_fd = socket(AF_NETLINK, SOCK_RAW, proto)) < 0) {
    netmgr_log_sys_err("nl_open_sock: socket failed:");
    goto error;
  }

  /* Initialize socket addresses to null */
  memset(p_sk_addr_loc, 0, sizeof(struct sockaddr_nl));
  memset(p_sk_addr_rem, 0, sizeof(struct sockaddr_nl));

  /* Populate local socket address using specified groups */
  p_sk_addr_loc->nl_family = AF_NETLINK;
  p_sk_addr_loc->nl_pid = NETMGR_NL_PID;
  /* p_sk_addr_loc->nl_pid = 0; */
  p_sk_addr_loc->nl_groups = grps;

  /* Bind socket to the local address, i.e. specified groups. This ensures
  ** that multicast messages for these groups are delivered over this
  ** socket.
  */
  if( bind( *p_sk_fd,
            (struct sockaddr *)p_sk_addr_loc,
            sizeof(struct sockaddr_nl) ) < 0) {
    netmgr_log_sys_err("nl_open_sock: bind failed:");
    goto error;
  }

  rval = NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return rval;
}

/*===========================================================================
  FUNCTION  netmgr_nl_alloc_msg
===========================================================================*/
/*!
@brief
  Allcoate a NETLINK message structure from dynamic memory.
  The msglen parameter  denotes the size of the message payload in bytes.

@return
  struct msghdr * - Pointer on successful allocaiton, NULL otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - Client is responsible for releasing dynamic memory
*/
/*=========================================================================*/
struct msghdr * netmgr_nl_alloc_msg
(
  uint32  msglen
)
{
  unsigned char * buf = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;
  struct msghdr * msgh = NULL;

  if( NL_MSG_MAX_LEN < msglen )
    return NULL;

  /* Allocate memory for the message header */
  if ((msgh = netmgr_malloc(sizeof(struct msghdr))) == NULL) {
    netmgr_log_err("failed on msgh netmgr_malloc: %d\n", (int)sizeof(struct msghdr));
    goto error;
  }

  /* Allocate memory for the message address structure */
  if ((nladdr = netmgr_malloc(sizeof(struct sockaddr_nl))) == NULL) {
    netmgr_log_err("failed on nladdr netmgr_malloc: %d\n", (int)sizeof(struct sockaddr_nl));
    goto error;
  }

  /* Allocate memory for the io vector */
  if ((iov = netmgr_malloc(sizeof(struct iovec))) == NULL) {
    netmgr_log_err("failed on iov netmgr_malloc: %d\n", (int)sizeof(struct iovec));
    goto error;
  }

  /* Allocate memory for the actual message contents */
  if ((buf = netmgr_malloc(msglen)) == NULL) {
    netmgr_log_err("failed on buf netmgr_malloc: %d\n", (int)msglen );
    goto error;
  }

  /* Populate message address */
  memset(nladdr, 0, sizeof(struct sockaddr_nl));
  nladdr->nl_family = AF_NETLINK;

  /* Populate message header */
  memset( msgh, 0x0, sizeof(struct msghdr));
  msgh->msg_name = nladdr;
  msgh->msg_namelen = sizeof(struct sockaddr_nl);
  msgh->msg_iov = iov;
  msgh->msg_iovlen = 1;

  /* Set io vector fields */
  memset( iov, 0x0, sizeof(struct iovec));
  iov->iov_base = buf;
  iov->iov_len = msglen;

  return msgh;

 error:
  /* An error occurred while allocating the message. Free all memory before
  ** returning.
  */
  netmgr_free(buf);
  netmgr_free(iov);
  netmgr_free(nladdr);
  netmgr_free(msgh);
  return NULL;
}

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
    - Memory is returned to heap
*/
/*=========================================================================*/
void netmgr_nl_release_msg
(
  struct msghdr * msgh
)
{
  unsigned char * buf = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;

  if( NULL == msgh )
    return;

  nladdr = msgh->msg_name;
  iov = msgh->msg_iov;
  if( msgh->msg_iov )
    buf = msgh->msg_iov->iov_base;

  netmgr_free(buf);
  netmgr_free(iov);
  netmgr_free(nladdr);
  netmgr_free(msgh);

  return;
}

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
)
{
  struct rtattr *rtah = NULL;    /* NLA header       */
  void * rtad = NULL;            /* NLA data payload */
  unsigned int size = 0;

  NETMGR_ASSERT( event_info );
  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( buflen );

  NETMGR_LOG_FUNC_ENTRY;

  /* Prepare message buffer based on event.  Note only attributes are
   * processed here; the NetLink msg header is added in later send
   * routine. */
  size = NETMGR_RTASIZE_EVENT;
  size+= (NETMGR_EVT_PARAM_LINK     & event_info->param_mask)? (int)NETMGR_RTASIZE_LINK : 0;
  size+= (NETMGR_EVT_PARAM_FLOWINFO & event_info->param_mask)? (int)NETMGR_RTASIZE_FLOWINFO : 0;
  size+= (NETMGR_EVT_PARAM_ADDRINFO & event_info->param_mask)? (int)NETMGR_RTASIZE_ADDRINFO : 0;
  size+= (NETMGR_EVT_PARAM_GTWYINFO & event_info->param_mask)? (int)NETMGR_RTASIZE_GTWYINFO : 0;
  size+= (NETMGR_EVT_PARAM_DNSPADDR & event_info->param_mask)? (int)NETMGR_RTASIZE_DNSPADDR : 0;
  size+= (NETMGR_EVT_PARAM_DNSSADDR & event_info->param_mask)? (int)NETMGR_RTASIZE_DNSSADDR : 0;
  size+= (NETMGR_EVT_PARAM_DEVNAME  & event_info->param_mask)? (int)NETMGR_RTASIZE_DEVNAME : 0;
  size+= (NETMGR_EVT_PARAM_MTU      & event_info->param_mask)? (int)NETMGR_RTASIZE_MTU : 0;
  size+= (NETMGR_EVT_PARAM_USER_CMD & event_info->param_mask)? (int)NETMGR_RTASIZE_USER_CMD : 0;
  size+= (NETMGR_EVT_PARAM_CMD_DATA & event_info->param_mask)? (int)NETMGR_RTASIZE_CMD_DATA : 0;

  switch( event_info->event ) {
    case NET_PLATFORM_UP_EV:
    case NET_PLATFORM_DOWN_EV:
    case NET_PLATFORM_RECONFIGURED_EV:
    case NET_PLATFORM_FLOW_ACTIVATED_EV:
    case NET_PLATFORM_FLOW_DELETED_EV:
    case NET_PLATFORM_FLOW_MODIFIED_EV:
    case NET_PLATFORM_FLOW_SUSPENDED_EV:
    case NET_PLATFORM_FLOW_ENABLED_EV:
    case NET_PLATFORM_FLOW_DISABLED_EV:
    case NET_PLATFORM_RESET_EV:
    case NET_PLATFORM_NEWADDR_EV:
    case NET_PLATFORM_DELADDR_EV:
    case NET_PLATFORM_MTU_UPDATE_EV:
    case NETMGR_READY_REQ:
    case NETMGR_READY_RESP:
    case NETMGR_USER_CMD:
      if ((*buffer = netmgr_malloc( (size_t)size )) == NULL) {
        netmgr_log_err("netmgr_nl_encode_netmgr_event: malloc failed for event" );
        *buflen = 0;
        NETMGR_LOG_FUNC_EXIT;
        return NETMGR_FAILURE;
      }
      *buflen = size;

      /* Mandatory parameters */
      rtah = (struct rtattr *)*buffer;
      NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_EVENT, NETMGR_RTASIZE_EVENT );
      memcpy( rtad, &event_info->event, sizeof(event_info->event) );

      /* Optional parameters */
      if(NETMGR_EVT_PARAM_LINK & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_LINK, NETMGR_RTASIZE_LINK );
        memcpy( rtad, &event_info->link, sizeof(event_info->link) );
      }

      if(NETMGR_EVT_PARAM_FLOWINFO & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_FLOWINFO, NETMGR_RTASIZE_FLOWINFO );
        memcpy( rtad, &event_info->flow_info, sizeof(event_info->flow_info) );
      }

      if(NETMGR_EVT_PARAM_ADDRINFO & event_info->param_mask) {
              char * ptr;
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_ADDRINFO, NETMGR_RTASIZE_ADDRINFO );
        memcpy( rtad, &event_info->addr_info, sizeof(event_info->addr_info) );
      }

      if(NETMGR_EVT_PARAM_GTWYINFO & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_GTWYINFO, NETMGR_RTASIZE_GTWYINFO );
        memcpy( rtad, &event_info->gtwy_info, sizeof(event_info->gtwy_info) );
      }

      if(NETMGR_EVT_PARAM_DNSPADDR & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_DNSPADDR, NETMGR_RTASIZE_DNSPADDR );
        memcpy( rtad, &event_info->dnsp_addr, sizeof(event_info->dnsp_addr) );
      }
      if(NETMGR_EVT_PARAM_DNSSADDR & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_DNSSADDR, NETMGR_RTASIZE_DNSSADDR );
        memcpy( rtad, &event_info->dnss_addr, sizeof(event_info->dnss_addr) );
      }

      if(NETMGR_EVT_PARAM_DEVNAME & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_DEVNAME, NETMGR_RTASIZE_DEVNAME );
        memcpy( rtad, &event_info->dev_name, sizeof(event_info->dev_name) );
      }

      if(NETMGR_EVT_PARAM_MTU & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT( NETMGR_RTATYPE_MTU, NETMGR_RTASIZE_MTU );
        memcpy( rtad, &event_info->mtu, sizeof(event_info->mtu) );
      }

      if(NETMGR_EVT_PARAM_USER_CMD & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT(NETMGR_RTATYPE_USER_CMD, NETMGR_RTASIZE_USER_CMD);
        memcpy( rtad, &event_info->user_cmd, sizeof(event_info->user_cmd) );
      }

      if(NETMGR_EVT_PARAM_CMD_DATA & event_info->param_mask) {
        rtah = RTA_NEXT(rtah, size);
        NETMGR_NL_RTA_NEXT(NETMGR_RTATYPE_CMD_DATA, NETMGR_RTASIZE_CMD_DATA);
        memcpy( rtad, &event_info->cmd_data, sizeof(event_info->cmd_data) );
      }

      netmgr_log_med( "Event=[%d] link=[%d] devname=[%s]\n",
                      event_info->event, event_info->link, event_info->dev_name );
      break;

    default:
      netmgr_log_err("unsupported event %d", event_info->event);
      NETMGR_LOG_FUNC_EXIT;
      return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}


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
  const char             *buffer,
  unsigned int            buflen,
  netmgr_nl_event_info_t *event_info
)
{
  struct rtattr *rtah = NULL;    /* NLA header       */

  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( event_info );

  NETMGR_LOG_FUNC_ENTRY;

  rtah = (struct rtattr*)buffer;
  event_info->param_mask = NETMGR_EVT_PARAM_NONE;

  /* Loop over octet buffer */
  while( RTA_OK(rtah, buflen ) ) {
    /* Process attribute type */
    switch( rtah->rta_type ) {
      case NETMGR_RTATYPE_EVENT:
        event_info->event = *(netmgr_nl_events_t*)RTA_DATA(rtah);
        netmgr_log_med( "Attribute: Event 0x%x\n", event_info->event );
        break;

      case NETMGR_RTATYPE_LINK:
        event_info->link = *(netmgr_link_id_t*)RTA_DATA(rtah);
        netmgr_log_med( "Attribute: Link 0x%x\n", event_info->link );
        break;

      case NETMGR_RTATYPE_FLOWINFO:
        memcpy( &event_info->flow_info, RTA_DATA(rtah), sizeof(netmgr_nl_flowinfo_t) );
        event_info->param_mask |=  NETMGR_EVT_PARAM_FLOWINFO;
        netmgr_log_med( "Attribute: FlowID 0x%08lx\n", event_info->flow_info.flow_id );
        netmgr_log_med( "Attribute: FlowType %d\n", event_info->flow_info.flow_type );
        break;

      case NETMGR_RTATYPE_ADDRINFO:
        memcpy( &event_info->addr_info, RTA_DATA(rtah), sizeof(event_info->addr_info) );
        event_info->param_mask |= NETMGR_EVT_PARAM_IPADDR;
        NETMGR_NL_REPORT_ADDR( med, "Attribute: IfaceAddress", event_info->addr_info.addr.ip_addr );
        if( AF_INET == event_info->addr_info.addr.ip_addr.ss_family ) {
          NETMGR_LOG_IPV4_ADDR( med, "Attribute: IfaceAddressMask", (event_info->addr_info.addr.mask) );
        } else {
          netmgr_log_med( "Attribute: IfaceAddressMask=%d", event_info->addr_info.addr.mask );
        }
        /* Check for defined cache info */
        if( 0 != event_info->addr_info.cache_info.cstamp ) {
          event_info->param_mask |= NETMGR_EVT_PARAM_CACHE;
          netmgr_log_med( "Attribute: Address Cache Info - prefered=%d valid=%d cstamp=0x%x tstamp=0x%x\n",
                          event_info->addr_info.cache_info.prefered,
                          event_info->addr_info.cache_info.valid,
                          event_info->addr_info.cache_info.cstamp,
                          event_info->addr_info.cache_info.tstamp );
        }
        break;

      case NETMGR_RTATYPE_GTWYINFO:
        memcpy( &event_info->gtwy_info, RTA_DATA(rtah), sizeof(event_info->gtwy_info) );
        event_info->param_mask |= NETMGR_EVT_PARAM_GTWYINFO;
        NETMGR_NL_REPORT_ADDR( med, "Attribute: GatewayAddress", event_info->gtwy_info.ip_addr );
        if( AF_INET6 == event_info->addr_info.addr.ip_addr.ss_family ) {
          netmgr_log_med( "Attribute: GatewayAddressMask=%d", event_info->gtwy_info.mask );
        }
        break;

      case NETMGR_RTATYPE_DNSPADDR:
        memcpy( &event_info->dnsp_addr, RTA_DATA(rtah), sizeof(event_info->dnsp_addr) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DNSPADDR;
        NETMGR_NL_REPORT_ADDR( med, "Attribute: DNSPrimaryAddress", event_info->dnsp_addr );
        break;

      case NETMGR_RTATYPE_DNSSADDR:
        memcpy( &event_info->dnss_addr, RTA_DATA(rtah), sizeof(event_info->dnss_addr) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DNSSADDR;
        NETMGR_NL_REPORT_ADDR( med, "Attribute: DNSSecondaryAddress", event_info->dnss_addr );
        break;

      case NETMGR_RTATYPE_DEVNAME:
        memcpy( &event_info->dev_name, RTA_DATA(rtah), sizeof(event_info->dev_name ) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;
        netmgr_log_med( "Attribute: DeviceName=%s", event_info->dev_name );
        break;

      case NETMGR_RTATYPE_MTU:
        memcpy( &event_info->mtu, RTA_DATA(rtah), sizeof(event_info->mtu ) );
        event_info->param_mask |= NETMGR_EVT_PARAM_MTU;
        netmgr_log_med( "Attribute: MTU=%d", event_info->mtu );
        break;

      case NETMGR_RTATYPE_USER_CMD:
        memcpy( &event_info->user_cmd, RTA_DATA(rtah), sizeof(event_info->user_cmd) );
        event_info->param_mask |= NETMGR_EVT_PARAM_USER_CMD;
        netmgr_log_med( "Attribute: User command=%d", event_info->user_cmd);
        break;

      case NETMGR_RTATYPE_CMD_DATA:
        memcpy( &event_info->cmd_data, RTA_DATA(rtah), sizeof(event_info->cmd_data) );
        event_info->param_mask |= NETMGR_EVT_PARAM_CMD_DATA;
        netmgr_log_med( "Attribute: cmd_data cmd_id[%d] pid[%d] txn_id[%d] txn_status[%d]",
                        event_info->cmd_data.cmd_id,
                        event_info->cmd_data.txn.pid,
                        event_info->cmd_data.txn.txn_id,
                        event_info->cmd_data.txn.txn_status);
        break;

      default:
        netmgr_log_err("unsupported attribute type %d, ignoring", rtah->rta_type );
    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_nl_decode_rtm_link
===========================================================================*/
/*!
@brief
  Decode kernel link message parameters from Netlink attribute TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_nl_decode_rtm_link
(
  const char              *buffer,
  unsigned int             buflen,
  netmgr_nl_link_info_t   *link_info
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;  /* NL message header */

  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( link_info );
  (void)buflen;

  NETMGR_LOG_FUNC_ENTRY;

  /* Extract the header data */
  link_info->metainfo = *(struct ifinfomsg*)NLMSG_DATA(nlh);
  netmgr_log_med( "Metainfo:  Index=%d Family=%d Type=0x%x "
                  "Change=[0x%x]%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s "
                  "Flags=[0x%x]%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
                  link_info->metainfo.ifi_index, link_info->metainfo.ifi_family,
                  link_info->metainfo.ifi_type,
                  link_info->metainfo.ifi_change,
                  (IFF_UP            & link_info->metainfo.ifi_change)?"UP ":"",
                  (IFF_BROADCAST     & link_info->metainfo.ifi_change)?"BROADCAST ":"",
                  (IFF_DEBUG         & link_info->metainfo.ifi_change)?"DEBUG ":"",
                  (IFF_LOOPBACK      & link_info->metainfo.ifi_change)?"LOOPBACK ":"",
                  (IFF_POINTOPOINT   & link_info->metainfo.ifi_change)?"POINTOPOINT ":"",
                  (IFF_NOTRAILERS    & link_info->metainfo.ifi_change)?"NOTRAILERS ":"",
                  (IFF_RUNNING       & link_info->metainfo.ifi_change)?"RUNNING ":"",
                  (IFF_NOARP         & link_info->metainfo.ifi_change)?"NOARP ":"",
                  (IFF_PROMISC       & link_info->metainfo.ifi_change)?"PROMISC ":"",
                  (IFF_ALLMULTI      & link_info->metainfo.ifi_change)?"ALLMULTI ":"",
                  (IFF_MASTER        & link_info->metainfo.ifi_change)?"MASTER ":"",
                  (IFF_SLAVE         & link_info->metainfo.ifi_change)?"SLAVE ":"",
                  (IFF_MULTICAST     & link_info->metainfo.ifi_change)?"MULTICAST ":"",
                  (IFF_PORTSEL       & link_info->metainfo.ifi_change)?"PORTSEL ":"",
                  (IFF_AUTOMEDIA     & link_info->metainfo.ifi_change)?"AUTOMEDIA ":"",
                  (IFF_DYNAMIC       & link_info->metainfo.ifi_change)?"DYNAMIC ":"",
                  (IFF_LOWER_UP      & link_info->metainfo.ifi_change)?"LOWER_UP ":"",
                  (IFF_DORMANT       & link_info->metainfo.ifi_change)?"DORMANT ":"",
                  link_info->metainfo.ifi_flags,
                  (IFF_UP            & link_info->metainfo.ifi_flags)?"UP ":"",
                  (IFF_BROADCAST     & link_info->metainfo.ifi_flags)?"BROADCAST ":"",
                  (IFF_DEBUG         & link_info->metainfo.ifi_flags)?"DEBUG ":"",
                  (IFF_LOOPBACK      & link_info->metainfo.ifi_flags)?"LOOPBACK ":"",
                  (IFF_POINTOPOINT   & link_info->metainfo.ifi_flags)?"POINTOPOINT ":"",
                  (IFF_NOTRAILERS    & link_info->metainfo.ifi_flags)?"NOTRAILERS ":"",
                  (IFF_RUNNING       & link_info->metainfo.ifi_flags)?"RUNNING ":"",
                  (IFF_NOARP         & link_info->metainfo.ifi_flags)?"NOARP ":"",
                  (IFF_PROMISC       & link_info->metainfo.ifi_flags)?"PROMISC ":"",
                  (IFF_ALLMULTI      & link_info->metainfo.ifi_flags)?"ALLMULTI ":"",
                  (IFF_MASTER        & link_info->metainfo.ifi_flags)?"MASTER ":"",
                  (IFF_SLAVE         & link_info->metainfo.ifi_flags)?"SLAVE ":"",
                  (IFF_MULTICAST     & link_info->metainfo.ifi_flags)?"MULTICAST ":"",
                  (IFF_PORTSEL       & link_info->metainfo.ifi_flags)?"PORTSEL ":"",
                  (IFF_AUTOMEDIA     & link_info->metainfo.ifi_flags)?"AUTOMEDIA ":"",
                  (IFF_DYNAMIC       & link_info->metainfo.ifi_flags)?"DYNAMIC ":"",
                  (IFF_LOWER_UP      & link_info->metainfo.ifi_flags)?"LOWER_UP ":"",
                  (IFF_DORMANT       & link_info->metainfo.ifi_flags)?"DORMANT ":"" );

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_nl_decode_rtm_addr
===========================================================================*/
/*!
@brief
  Decode kernel address message parameters from Netlink attribute TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_nl_decode_rtm_addr
(
  const char              *buffer,
  unsigned int             buflen,
  netmgr_nl_addr_info_t   *addr_info
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;  /* NL message header */
  struct rtattr *rtah = NULL;
  unsigned int  nestlen = 0;

  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( addr_info );

  NETMGR_LOG_FUNC_ENTRY;

  /* Extract the header data */
  addr_info->metainfo = *((struct ifaddrmsg*)NLMSG_DATA(nlh));
  buflen = buflen - (unsigned int)sizeof(struct nlmsghdr);

  /* Temporary feature wrap, until the codesorcery toolchain on LE
   * is updated to include IFA_F_DADFAILED.
   */
#ifdef FEATURE_DS_LINUX_ANDROID
  netmgr_log_med( "Metainfo:  Family=%d PrefixLen=%d Scope=0x%x Index=%d "
                  "Flags=[0x%x]%s%s%s%s%s%s%s%s\n",
                  addr_info->metainfo.ifa_family, addr_info->metainfo.ifa_prefixlen,
                  addr_info->metainfo.ifa_scope, addr_info->metainfo.ifa_index,
                  addr_info->metainfo.ifa_flags,
                  (IFA_F_TEMPORARY   & addr_info->metainfo.ifa_flags)?"TEMPORARY ":"",
                  (IFA_F_HOMEADDRESS & addr_info->metainfo.ifa_flags)?"HOMEADDRESS ":"",
                  (IFA_F_OPTIMISTIC  & addr_info->metainfo.ifa_flags)?"OPTIMISTIC ":"",
                  (IFA_F_NODAD       & addr_info->metainfo.ifa_flags)?"NODAD ":"",
                  (IFA_F_DADFAILED   & addr_info->metainfo.ifa_flags)?"DADFAILED ":"",
                  (IFA_F_DEPRECATED  & addr_info->metainfo.ifa_flags)?"DEPRECATED ":"",
                  (IFA_F_TENTATIVE   & addr_info->metainfo.ifa_flags)?"TENTATIVE ":"",
                  (IFA_F_PERMANENT   & addr_info->metainfo.ifa_flags)?"PERMANENT ":"" );
#endif

  /* Extract the available attributes */
  addr_info->attr_info.param_mask = NETMGR_NLA_PARAM_NONE;

  rtah = IFA_RTA( NLMSG_DATA(nlh) );

  while( RTA_OK(rtah, buflen ) ) {
    switch( rtah->rta_type ) {
      case IFA_LABEL:   /* same as IFLA_IFNAME */
        memcpy( &addr_info->attr_info.label_name,
                RTA_DATA(rtah),
                sizeof(addr_info->attr_info.label_name) );
        /* Explicitly set string terminator just in case */
        addr_info->attr_info.label_name[sizeof(addr_info->attr_info.label_name)-1] = 0;
        addr_info->attr_info.param_mask |= NETMGR_NLA_PARAM_LABELNAME;
        netmgr_log_med( "Attribute: Label name %s\n", &addr_info->attr_info.label_name );
        break;

      case IFA_ADDRESS: /* same as IFLA_ADDRESS */
        NETMGR_NL_COPY_ADDR( "Prefix", NETMGR_NLA_PARAM_PREFIXADDR, prefix_addr );
        break;

      case IFA_LOCAL:  /* same as IFLA_BROADCAST */
        NETMGR_NL_COPY_ADDR( "Local", NETMGR_NLA_PARAM_LOCALADDR, local_addr );
        break;

      case IFA_BROADCAST:
        NETMGR_NL_COPY_ADDR( "Broadcast", NETMGR_NLA_PARAM_BCASTADDR, bcast_addr );
        break;

      case IFA_ANYCAST:
        NETMGR_NL_COPY_ADDR( "Anycast", NETMGR_NLA_PARAM_ACASTADDR, acast_addr );
        break;

      case IFA_MULTICAST:
        NETMGR_NL_COPY_ADDR( "Multicast", NETMGR_NLA_PARAM_MCASTADDR, mcast_addr );
        break;

      case IFA_CACHEINFO:
        memcpy( &addr_info->attr_info.cache_info,
                RTA_DATA(rtah),
                sizeof(addr_info->attr_info.cache_info) );
        addr_info->attr_info.param_mask |= NETMGR_NLA_PARAM_CACHEINFO;
        netmgr_log_med( "Attribute: Address Cache Info - prefered=%d valid=%d cstamp=0x%x tstamp=0x%x\n",
                        addr_info->attr_info.cache_info.ifa_prefered,
                        addr_info->attr_info.cache_info.ifa_valid,
                        addr_info->attr_info.cache_info.cstamp,
                        addr_info->attr_info.cache_info.tstamp );
        break;

      case IFLA_OPERSTATE:
        /* Just output value for debug purposes; not saved */
        netmgr_log_med( "Attribute: OperState: %d\n", *(uint8*)RTA_DATA(rtah) );
        break;

      case IFLA_LINKMODE:
        /* Just output value for debug purposes; not saved */
        netmgr_log_med( "Attribute: LinkMode: %d\n", *(uint8*)RTA_DATA(rtah) );
        break;

      case IFLA_PROTINFO:
        /* Begin nested attribute */
        nestlen = rtah->rta_len;
        rtah = RTA_DATA(rtah);
        addr_info->attr_info.proto_info.param_mask = NETMGR_NLA_PARAM_NONE;

        while( RTA_OK(rtah, nestlen ) ) {
          switch( rtah->rta_type ) {
            case IFLA_INET6_FLAGS:
              memcpy( &addr_info->attr_info.proto_info.flags,
                      RTA_DATA(rtah),
                      sizeof(addr_info->attr_info.proto_info.flags) );
              addr_info->attr_info.param_mask |= NETMGR_NLA_PARAM_FLAGS;
              break;

            case IFLA_INET6_CACHEINFO:
              memcpy( &addr_info->attr_info.proto_info.cache_info,
                      RTA_DATA(rtah),
                      sizeof(addr_info->attr_info.proto_info.cache_info) );
              addr_info->attr_info.proto_info.param_mask |= NETMGR_NLA_PARAM_CACHEINFO;
              netmgr_log_med( "Attribute: Address Cache Info - max_reasm_len=%d tstamp=0x%x reachable_time=0x%x retrans_time=0x%x\n",
                              addr_info->attr_info.proto_info.cache_info.max_reasm_len,
                              addr_info->attr_info.proto_info.cache_info.tstamp,
                              addr_info->attr_info.proto_info.cache_info.reachable_time,
                              addr_info->attr_info.proto_info.cache_info.retrans_time );
              break;

            case IFLA_INET6_CONF:
            case IFLA_INET6_MCAST:
            case IFLA_INET6_STATS:
              /* Not supported currently */
              break;

            default:
              netmgr_log_err("Unsupported attribute type, ignoring 0x%x\n", rtah->rta_type );
              break;
          }

          /* Advance to next nested attribute */
          rtah = RTA_NEXT(rtah, nestlen);
        }
        break;

      default:
        netmgr_log_err("Unsupported attribute type, ignoring 0x%x\n", rtah->rta_type );
        break;
    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_nl_decode_rtm_prefix
===========================================================================*/
/*!
@brief
  Decode kernel prefix message parameters from Netlink attribute TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_nl_decode_rtm_prefix
(
  const char                *buffer,
  unsigned int               buflen,
  netmgr_nl_prefix_info_t   *prefix_info
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;  /* NL message header */
  struct rtattr *rtah = NULL;

  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( prefix_info );

  NETMGR_LOG_FUNC_ENTRY;

  /* Extract the header data */
  prefix_info->metainfo = *((struct prefixmsg*)NLMSG_DATA(nlh));
  buflen = buflen - (unsigned int)sizeof(struct nlmsghdr);

  /* Extract the available attributes */
  rtah = IFA_RTA( NLMSG_DATA(nlh) );

  while( RTA_OK(rtah, buflen ) ) {
    switch( rtah->rta_type ) {
      case PREFIX_ADDRESS:
        memcpy( &prefix_info->prefixinfo,
                RTA_DATA(rtah),
                sizeof(prefix_info->prefixinfo) );
        netmgr_log_med( "Attribute: Prefix Info - type=%d length=%d prefix_len=%d valid=%d "
                        "autoconf=%d onlink=%d prefered=%d\n",
                        prefix_info->prefixinfo.type,
                        prefix_info->prefixinfo.length,
                        prefix_info->prefixinfo.prefix_len,
                        prefix_info->prefixinfo.valid,
                        prefix_info->prefixinfo.autoconf,
                        prefix_info->prefixinfo.onlink,
                        prefix_info->prefixinfo.prefered );
        NETMGR_LOG_IPV6_ADDR( med, "Attribute: Prefix", (prefix_info->prefixinfo.prefix.s6_addr));
        break;

      case PREFIX_CACHEINFO:
        memcpy( &prefix_info->cacheinfo,
                RTA_DATA(rtah),
                sizeof(prefix_info->cacheinfo) );
        netmgr_log_med( "Attribute: Prefix Cache Info - preferred=0x%x valid=0x%x\n",
                        prefix_info->cacheinfo.preferred_time,
                        prefix_info->cacheinfo.valid_time );
        break;

      default:
        netmgr_log_err("Unsupported attribute type, ignoring 0x%x\n", rtah->rta_type );
        break;
    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_nl_decode_xfrm_msg
===========================================================================*/
/*!
@brief
  Decode XFRM event parameters from Netlink attribute TLVs.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_nl_decode_xfrm_msg
(
  const char             *buffer,
  netmgr_nl_xfrm_info_t  *xfrm_info
)
{
  struct nlmsghdr          * nlh = (struct nlmsghdr*)buffer;
  struct rtattr            *rta = NULL;
  struct xfrm_aevent_id    *temp_aevent = NULL;
  struct xfrm_replay_state *temp_replay = NULL;
  unsigned int             rtasize;

  netmgr_log_med("netmgr_nl_decode_xfrm_msg ENTRY");

  if (buffer == NULL
      || xfrm_info == NULL)
  {
    netmgr_log_err("No data from netlink!");
    return NETMGR_FAILURE;
  }

  temp_aevent = NLMSG_DATA(nlh);

  if (NULL != temp_aevent)
  {
    rta = XFRM_RTA(nlh, struct xfrm_aevent_id);
    rtasize = XFRM_PAYLOAD(nlh, struct xfrm_aevent_id);
    while (RTA_OK(rta, rtasize))
    {
      if (rta->rta_type == XFRMA_REPLAY_VAL
          && RTA_PAYLOAD(rta) == sizeof(struct xfrm_replay_state))
      {
        temp_replay = malloc(RTA_PAYLOAD(rta));
        if (NULL == temp_replay)
        {
          netmgr_log_err("Failed to allocate memory for xfrm replay struct");
          break;
        }
        memcpy(temp_replay,
               RTA_DATA(rta),
               RTA_PAYLOAD(rta));
        break;
      }
    }

    if (NULL != temp_replay)
    {
      memcpy(&xfrm_info->xfrm_replay,
           temp_replay,
           sizeof(struct xfrm_replay_state));
      /* Free local pointer memory */
      free(temp_replay);
    }

    memcpy(&xfrm_info->xfrm_aevent,
           temp_aevent,
           sizeof(struct xfrm_aevent_id));
  }
  else
  {
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

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
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;  /* NL message header */

  NETMGR_ASSERT( buffer );
  NETMGR_ASSERT( msg_ptr );

  NETMGR_LOG_FUNC_ENTRY;

  memset( msg_ptr, 0x0, sizeof(netmgr_nl_msg_t) );

  while( NLMSG_OK(nlh, buflen) ) {
    switch( nlh->nlmsg_type ) {
      case NETMGR_NL_EVENT:
        msg_ptr->type = nlh->nlmsg_type;
        netmgr_log_med( "Processing NETMGR_NL_EVENT\n" );

        /* Process the Netlink attributes */
        if( NETMGR_SUCCESS !=
            netmgr_nl_decode_netmgr_event( NLMSG_DATA(nlh), NLMSG_PAYLOAD(nlh,0), &msg_ptr->event_info ) ) {
          netmgr_log_err( "Error on  netmgr_nl_decode_netmgr_event\n" );
          return NETMGR_FAILURE;
        }
        msg_ptr->param_mask |= NETMGR_NL_PARAM_EVENT;
        break;

      case RTM_NEWADDR:
      case RTM_DELADDR:
        msg_ptr->type = nlh->nlmsg_type;
        netmgr_log_med( "Processing %s\n", (RTM_NEWADDR == msg_ptr->type)?"RTM_NEWADDR":"RTM_DELADDR" );

        /* Process the Netlink attributes */
        if( NETMGR_SUCCESS !=
            netmgr_nl_decode_rtm_addr( buffer, buflen, &msg_ptr->addr_info ) ) {
          netmgr_log_err( "Error on  netmgr_nl_decode_rtm_addr\n" );
          return NETMGR_FAILURE;
        }
        msg_ptr->param_mask |= NETMGR_NL_PARAM_ADDR;
        break;

      case RTM_NEWLINK:
      case RTM_DELLINK:
        msg_ptr->type = nlh->nlmsg_type;
        netmgr_log_med( "Processing %s\n", (RTM_NEWLINK == nlh->nlmsg_type)?"RTM_NEWLINK":"RTM_DELLINK" );

        /* Process the Netlink attributes */
        if( NETMGR_SUCCESS !=
            netmgr_nl_decode_rtm_link( buffer, buflen, &msg_ptr->link_info ) ) {
          netmgr_log_err( "Error on netmgr_nl_decode_rtm_link\n" );
          return NETMGR_FAILURE;
        }
        msg_ptr->param_mask |= NETMGR_NL_PARAM_LINK;
        break;

      case RTM_NEWPREFIX:
        msg_ptr->type = nlh->nlmsg_type;
        memset( &msg_ptr->prefix_info, 0x0, sizeof(msg_ptr->prefix_info) );
        netmgr_log_med( "Processing RTM_NEWPREFIX\n" );

        /* Process the Netlink attributes */
        if( NETMGR_SUCCESS !=
            netmgr_nl_decode_rtm_prefix( buffer, buflen, &msg_ptr->prefix_info ) ) {
          netmgr_log_err( "Error on  netmgr_nl_decode_rtm_prefix\n" );
          return NETMGR_FAILURE;
        }
        msg_ptr->param_mask |= NETMGR_NL_PARAM_PREFIX;
        break;

      case RTM_NEWNEIGH:
      case RTM_DELNEIGH:
        netmgr_log_med( "Received %s, ignoring\n",
                        (RTM_NEWNEIGH == nlh->nlmsg_type)?"RTM_NEWNEIGH":"RTM_DELNEIGH" );
        break;

      case XFRM_MSG_NEWAE:
        msg_ptr->type = nlh->nlmsg_type;
        netmgr_log_med( "Processing XFRM_MSG_NEWAE\n");

        /* Add method to decode XFRM event */
        if ( NETMGR_SUCCESS !=
            netmgr_nl_decode_xfrm_msg(buffer, &msg_ptr->xfrm_info) ) {
            return NETMGR_FAILURE;
        }
        msg_ptr->param_mask = NETMGR_XFRM_PARAM_ESPTHRESH;
        break;

      default:
        netmgr_log_med( "Unsupported NetLink message, ignoring type 0x%x\n",
                        nlh->nlmsg_type );
        break;
    }

    /* Advance to next header */
    nlh = NLMSG_NEXT(nlh, buflen);
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}


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
int
netmgr_nl_send_msg( int fd, void* buffer, uint16 buflen )
{
  struct sockaddr_nl * nladdr = NULL;
  struct msghdr * msgh = NULL;
  struct nlmsghdr * nlh = NULL;
  int smsgl;
  int rc = NETMGR_SUCCESS;

  NETMGR_LOG_FUNC_ENTRY;

  if( NULL == (msgh = netmgr_nl_alloc_msg( NLMSG_SPACE(buflen) )) ) {
    netmgr_log_err("failed on netmgr_nl_alloc_msg\n");
    return NETMGR_FAILURE;
  }

  /* Populate message address for multicast group NETMGR_NLGRP_EVENTS */
  nladdr = msgh->msg_name;
  nladdr->nl_family = AF_NETLINK;
  nladdr->nl_pid = 0;
  nladdr->nl_groups = NETMGR_NL_GRP_EVENTS;

  /* Fill the netlink message header */
  nlh = msgh->msg_iov->iov_base;
  nlh->nlmsg_len = NLMSG_SPACE(buflen);
  nlh->nlmsg_type = NETMGR_NL_EVENT;
  nlh->nlmsg_flags = 0;
  nlh->nlmsg_seq = (__u32)++nl_seqno;
  nlh->nlmsg_pid = NETMGR_NL_PID;  /* thread pid */

  /* Fill in the netlink message payload */
  if( buffer && buflen )
    memcpy( NLMSG_DATA(nlh), buffer, buflen );

  /* Send message over the socket */
  smsgl = sendmsg( fd, msgh, 0 );

  /* Verify that something was written */
  if (smsgl <= 0) {
    netmgr_log_sys_err("Writing nl_msg, sendmsg failed:");
    rc = NETMGR_FAILURE;
  } else {
    netmgr_log_med("Generated nl msg, sendmsg returned %d\n", smsgl);
  }

  /* Release dynamic memory */
  netmgr_nl_release_msg( msgh );

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}


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
)
{
  struct msghdr * msgh = NULL;
  int rmsgl;

  NETMGR_ASSERT(msg_pptr);
  NETMGR_ASSERT(msglen_ptr);

  NETMGR_LOG_FUNC_ENTRY;

  if( NULL == (msgh = netmgr_nl_alloc_msg( NL_MSG_MAX_LEN )) ) {
    netmgr_log_err("failed on netmgr_nl_alloc_msg\n");
    goto error;
  }

  /* Receive message over the socket */
  rmsgl = recvmsg(fd, msgh, 0);

  /* Verify that something was read */
  if (rmsgl <= 0) {
    netmgr_log_sys_err("Received nl_msg, recvmsg failed:");
    goto error;
  }

  /* Verify that NL address length in the received message is expected value */
  if (msgh->msg_namelen != sizeof(struct sockaddr_nl)) {
    netmgr_log_err("rcvd msg with namelen != sizeof sockaddr_nl\n");
    goto error;
  }

  /* Verify that message was not truncated. This should not occur */
  if (msgh->msg_flags & MSG_TRUNC) {
    netmgr_log_err("Rcvd msg truncated!\n");
    goto error;
  }

  netmgr_log_med("Received nl msg, recvmsg returned %d\n", rmsgl);
  *msg_pptr    = msgh;
  *msglen_ptr = (unsigned int)rmsgl;

  /* Return message ptr. Caller is responsible for freeing the memory */
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  /* An error occurred while receiving the message. Free all memory before
  ** returning.
  */
  netmgr_nl_release_msg( msgh );
  *msg_pptr    = NULL;
  *msglen_ptr  = 0;
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}



/*===========================================================================
  FUNCTION  netmgr_nl_listener_init
===========================================================================*/
/*!
@brief
  Initialization routine for NetLink socket interface message listener.

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
netmgr_nl_listener_init
(
  netmgr_socklthrd_info_t  * sk_thrd_info,
  netmgr_socklthrd_fdmap_t * sk_thrd_fdmap,
  unsigned int               sk_thrd_fdmap_size,
  netmgr_nl_sk_info_t      * sk_info,
  unsigned int               nl_type,
  unsigned int               nl_groups,
  netmgr_socklthrd_fd_read_f read_f
)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Initialize socket listener thread. This thread is used to listen for
  ** incoming messages on all netink sockets.
  */
  if( netmgr_nl_socklthrd_init( sk_thrd_info,
                                sk_thrd_fdmap,
                                sk_thrd_fdmap_size ) < 0)
  {
    netmgr_log_err("cannot init sock listener thread\n");
    return NETMGR_FAILURE;
  }

  /* Open a netlink socket for NETLINK protocol. This socket is used
  ** for receiving netlink messaging
  */
  if( netmgr_nl_open_sock( sk_info, (int)nl_type, nl_groups ) < 0)
  {
    netmgr_log_err("cannot open nl routing sock\n");
    return NETMGR_FAILURE;
  }

  /* Add the NETLINK socket to the list of sockets that the listener
  ** thread should listen on.
  */
  if( netmgr_nl_socklthrd_addfd( sk_thrd_info,
                                 sk_info->sk_fd,
                                 read_f ) < 0)
  {
    netmgr_log_err("cannot add nl routing sock for reading\n");
    return NETMGR_FAILURE;
  }

  /* Start the socket listener thread */
  if( netmgr_nl_socklthrd_start( sk_thrd_info ) < 0 ) {
    netmgr_log_err("cannot start sock listener thread\n");
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

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
  netmgr_socklthrd_info_t  * sk_thrd_info,
  netmgr_nl_sk_info_t      * rt_sk
)
{
  unsigned int i;

  NETMGR_LOG_FUNC_ENTRY;

  /* Stop the socket listener thread */
  sk_thrd_info->running = FALSE;
  write( rt_sk->sk_fd, "1", 1 );

  /* Remove NETLINK socket from map */
  for( i=0; i<sk_thrd_info->nfd; i++) {
    if( sk_thrd_info->fdmap[i].fd == rt_sk->sk_fd ) {
      sk_thrd_info->fdmap[i].fd = 0;
      sk_thrd_info->fdmap[i].read_f = NULL;
    }
  }

  /* Close the netlink socket */
  if( close( rt_sk->sk_fd ) < 0)
  {
    netmgr_log_err("cannot close nl routing sock\n");
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}
