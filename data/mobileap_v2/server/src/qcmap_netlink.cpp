/******************************************************************************

                        QCMAP_NETLINK.C

******************************************************************************/

/******************************************************************************

  @file    qcmap_netlink.c
  @brief   QCMAP Netlink Messaging Implementation File

  DESCRIPTION
  Implementation file for QCMAP Netlink messaging functions.

  ---------------------------------------------------------------------------
   Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
10/01/13   pd         Created. Similar to qti_netlink.c, ipacm_netlink.c and
                      ipacm_neigh.c
01/11/14   sr         Added support for connected Devices in SoftAP
25/02/14   pm         Added handling of RTM_NEWADDR event
03/27/14   cp         Added support to DUN+SoftAP.

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "qcmap_netlink.h"
#include "qcmap_cm_api.h"
#include "QCMAP_ConnectionManager.h"
#include <linux/kernel.h>
#include <linux/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <linux/filter.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
/*===========================================================================
                              VARIABLE DEFINITIONS
===========================================================================*/
#define QCMAP_NLA_PARAM_NONE        (0x0000)
#define QCMAP_NDA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#define QCMAP_IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define QCMAP_RTN_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))
extern unsigned int nl_qcmap_sockfd;
#define LOG_MSG_LEN           200

#define QCMAP_IFA_MIN_LINK_LOCAL_IP 0xa9fe0100 /* 169.254.1.0 */
#define QCMAP_IFA_MAX_LINK_LOCAL_IP 0xa9fefeff /* 169.254.254.255 */
#define QCMAP_IFA_IS_LINK_LOCAL_IP(x) \
        (htonl(x) >= QCMAP_IFA_MIN_LINK_LOCAL_IP && htonl(x) <= QCMAP_IFA_MAX_LINK_LOCAL_IP)

/* Mask needed to ensure To ensure pid is 31 bits */
#define QCMAP_NL_PID_MASK                   (0x7FFFFFFF)
#define QCMAP_NL_PID                        (getpid() & QCMAP_NL_PID_MASK)
#define QCMAP_NL_GETNEIGH_SEQ               555666
#define QCMAP_NL_SOCK_PORTID                QCMAP_NL_PID + QCMAP_NL_GETNEIGH_SEQ

/* Global QCMAP NL Listen Sock FD */
int qcmap_nl_listen_sock_fd=0;

/*------------------------------------------------------------------------
  Structure to store USB TE Information
-------------------------------------------------------------------------*/
struct ether_addr qcmap_nl_usb_mac_addr;
/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================

FUNCTION QCMAP_NL_OPEN_SOCKET()

DESCRIPTION

  This function
  - opens netlink sockets
  - binds the socket to listen to the required netlink events

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_nl_open_socket
(
  qcmap_nl_sk_info_t     *sk_info,
  int                    protocol,
  unsigned int           grps
)
{
  int                  *p_sk_fd;
  struct sockaddr_nl   *p_sk_addr_loc ;

  ds_assert(sk_info != NULL);

  LOG_MSG_INFO1("Entering qcmap_nl_open_socket protocol = %d, grps = %d", protocol,grps, 0);
//  printk(KERN_INFO,"Entering qcmap_nl_open_socket protocol = %d, grps = %d \n", protocol,grps);

  p_sk_fd = &(sk_info->sk_fd);
  p_sk_addr_loc = &(sk_info->sk_addr_loc);

/*--------------------------------------------------------------------------
  Open netlink socket for specified protocol
---------------------------------------------------------------------------*/
  if ((*p_sk_fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0)
  {
    LOG_MSG_ERROR("Cannot open netlink socket",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  LOG_MSG_INFO1("Socket open succeeds",0,0,0);

  qcmap_nl_listen_sock_fd = *p_sk_fd;

/*--------------------------------------------------------------------------
  Initialize socket parameters to 0
--------------------------------------------------------------------------*/
  memset(p_sk_addr_loc, 0, sizeof(struct sockaddr_nl));

/*-------------------------------------------------------------------------
  Populate socket parameters
--------------------------------------------------------------------------*/
  p_sk_addr_loc->nl_family = AF_NETLINK;
  p_sk_addr_loc->nl_pid = QCMAP_NL_SOCK_PORTID;
  p_sk_addr_loc->nl_groups = grps;

/*-------------------------------------------------------------------------
  Bind socket to receive the netlink events for the required groups
--------------------------------------------------------------------------*/

  if( bind( *p_sk_fd,
            (struct sockaddr *)p_sk_addr_loc,
            sizeof(struct sockaddr_nl) ) < 0)
  {
    ds_log_med("Socket bind failed %s- Make sure no-one has opened a NL socket with %d pid \n",
                                                         strerror(errno),QCMAP_NL_SOCK_PORTID);
    return QCMAP_NL_FAILURE;
  }
  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_NL_ALLOC_MSG()

DESCRIPTION

  This function
  - allocates memory to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_SUCCESS on success
  QCMAP_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static struct msghdr * qcmap_nl_alloc_msg
(
  uint32_t         msglen
)
{
  unsigned char          *buf = NULL;
  struct sockaddr_nl     * nladdr = NULL;
  struct iovec           * iov = NULL;
  struct msghdr          * msgh = NULL;

/*-------------------------------------------------------------------------*/

  if(QCMAP_NL_MSG_MAX_LEN < msglen)
  {
    LOG_MSG_ERROR("Netlink message exceeds maximum length", 0, 0, 0);
    return NULL;
  }

  if((msgh = malloc(sizeof(struct msghdr))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for msghdr", 0, 0, 0);
    return NULL;
  }

  if((nladdr = malloc(sizeof(struct sockaddr_nl))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for sockaddr", 0, 0, 0);
    free(msgh);
    return NULL;
  }

  if((iov = malloc(sizeof(struct iovec))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for iovec", 0, 0, 0);
    free(nladdr);
    free(msgh);
    return NULL;
  }

  if((buf = malloc(msglen))== NULL)
  {
    LOG_MSG_ERROR("Failed malloc for buffer to store netlink message",
                  0, 0, 0);
    free(iov);
    free(nladdr);
    free(msgh);
    return NULL;
  }

  memset(nladdr, 0, sizeof(struct sockaddr_nl));
  nladdr->nl_family = AF_NETLINK;

  memset(msgh, 0x0, sizeof(struct msghdr));
  msgh->msg_name = nladdr;
  msgh->msg_namelen = sizeof(struct sockaddr_nl);
  msgh->msg_iov = iov;
  msgh->msg_iovlen = 1;

  memset(iov, 0x0, sizeof(struct iovec));
  iov->iov_base = buf;
  iov->iov_len = msglen;

  return msgh;
}

/*===========================================================================

FUNCTION QCMAP_NL_ADDFD_MAP()

DESCRIPTION

  This function
  - maps the socket descriptor with the corresponding callback function
  - add the socket descriptor to the set of socket desc the listener thread
    listens on.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_nl_addfd_map
(
  qcmap_nl_sk_fd_set_info_t      *fd_set,
  int                            fd,
  qcmap_sock_thrd_fd_read_f      read_f
)
{
  ds_assert(fd_set != NULL);
  LOG_MSG_INFO1("Entering qcmap_nl_addfd_map() fd= %d",fd,0,0);

  if( fd_set->num_fd < QCMAP_NL_MAX_NUM_OF_FD )
  {
/*-----------------------------------------------------------------------
  Add the fd to the fd set which the listener thread should listen on
------------------------------------------------------------------------*/
    FD_SET(fd, &(fd_set->fdset));

/*-----------------------------------------------------------------------
  Associate fd with the corresponding read function
------------------------------------------------------------------------*/
    fd_set->sk_fds[fd_set->num_fd].sk_fd = fd;
    fd_set->sk_fds[fd_set->num_fd].read_func = read_f;
    fd_set->num_fd++;

/*-----------------------------------------------------------------------
  Increment the max socket desc number which the listener should listen
  if required
------------------------------------------------------------------------*/
    if(fd_set->max_fd < fd)
    {
      fd_set->max_fd = fd;
    }
  }
  else
  {
    LOG_MSG_ERROR("Exceeds maximum num of FD",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_NL_QUERY_IF()

DESCRIPTION

  This function
  - sends RTM_GETNEIGH to kernel

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_nl_query_if
(
   qcmap_nl_sk_info_t      *sk_info
)
{
  struct msghdr        *nl_msg_hdr = NULL;
  qcmap_nl_req_type    *nl_req = NULL;
  ssize_t              sndbytes=0;
  char logmsg[LOG_MSG_LEN];
/*------------------------------------------------------------------------*/

  ds_assert(sk_info != NULL);

/*-------------------------------------------------------------------------
  Send RTM_GETNEIGH to find if RNDIS/ECM interfaces are running
-------------------------------------------------------------------------*/
  LOG_MSG_INFO1("qcmap_nl_query_if(): Sending RTM_GETNEIGH to kernel",0,0,0);
  nl_msg_hdr = qcmap_nl_alloc_msg( QCMAP_NL_MSG_MAX_LEN );
  if(nl_msg_hdr == NULL)
  {
    LOG_MSG_ERROR("qcmap_nl_query_if(): Failed in qcmap_nl_alloc_msg",0, 0, 0);
    return QCMAP_NL_FAILURE;
  }
  nl_req = (qcmap_nl_req_type *)(nl_msg_hdr->msg_iov->iov_base);

  /*-------------------------------------------------------------------------
    Populate the required parameters in the netlink message
  ---------------------------------------------------------------------------*/
  nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  nl_req->hdr.nlmsg_type = RTM_GETNEIGH|RTM_GETROUTE;
  /* NLM_F_REQUEST - has to be set for request messages
     NLM_F_DUMP -  equivalent to NLM_F_ROOT|NLM_F_MATCH */
  nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nl_req->hdr.nlmsg_seq = 1;
  nl_req->hdr.nlmsg_pid = sk_info->sk_addr_loc.nl_pid;
  nl_req->gen.rtgen_family = AF_PACKET;

  if (sendmsg(sk_info->sk_fd, (struct msghdr *)nl_msg_hdr, 0) <= 0)
  {
     LOG_MSG_ERROR("IOCTL Send Failed errno:%d",errno,0,0);
     snprintf((char *)logmsg,LOG_MSG_LEN,"echo USB Qcmap NL IOCTL Snd GETNEIGH Failed %d > /dev/kmsg",errno);
     ds_system_call((char *)logmsg,strlen(logmsg));
     return QCMAP_NL_FAILURE;
  }

  snprintf((char *)logmsg,LOG_MSG_LEN,"echo USB QCMAP NL IOCTL Snd GETNEIGH Succ> /dev/kmsg");
  ds_system_call((char *)logmsg,strlen(logmsg));

  free(nl_msg_hdr->msg_iov->iov_base);
  free(nl_msg_hdr->msg_iov);
  free(nl_msg_hdr->msg_name);
  free(nl_msg_hdr);

  return QCMAP_NL_SUCCESS;
}


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
  unsigned int              nl_type,
  unsigned int              nl_groups,
  qcmap_nl_sk_fd_set_info_t *sk_fdset,
  qcmap_sock_thrd_fd_read_f read_f
)
{
  qcmap_nl_sk_info_t     sk_info;
  int                    ret_val;

  LOG_MSG_INFO1("Initialize netlink listener thread for QCMAP",0,0,0);
  memset(&sk_info, 0, sizeof(qcmap_nl_sk_info_t));

  /*---------------------------------------------------------------------------
    Open netlink sockets
  ----------------------------------------------------------------------------*/

  if( qcmap_nl_open_socket( &sk_info, nl_type, nl_groups ) == QCMAP_NL_SUCCESS)
  {
    LOG_MSG_INFO1("Open: QCMAP Netlink socket succeeds",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Open: QCMAP Netlink socket failed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  /*--------------------------------------------------------------------------
    Add the NETLINK socket to the list of sockets that the listener
    thread should listen on.
  ---------------------------------------------------------------------------*/
  if( qcmap_nl_addfd_map(sk_fdset,sk_info.sk_fd,read_f ) != QCMAP_NL_SUCCESS)
  {
    LOG_MSG_ERROR("cannot add Netlink routing sock for reading",0,0,0);
    close(sk_info.sk_fd);
    return QCMAP_NL_FAILURE;
  }

  LOG_MSG_INFO1("add fd map succeeds FD: %d ",sk_info.sk_fd, 0, 0);

  /* -------------------------------------------------------------------------
    Query the kernel about the current Neighbors by sending RTM_GETNEIGH.
    This is useful for the interfaces if the USB is plugged in and then powered up
    Its Just a safe guard, since DHCP addresses wouldnt be assigned until DHCP server is bought up
  --------------------------------------------------------------------------*/
  ret_val = qcmap_nl_query_if(&sk_info);
  if(ret_val != QCMAP_NL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed sending RTM_GETNEIGH to kernel",0,0,0);
  }

  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_NL_RELEASE_MSG()

DESCRIPTION

  This function
  - releases memory which was alloacted to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static void qcmap_nl_release_msg
(
  struct msghdr * msgh
)
{
  unsigned char * buf = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;

  if( NULL == msgh )
  {
    return;
  }

  nladdr = msgh->msg_name;
  iov = msgh->msg_iov;
  if( msgh->msg_iov )
  {
    buf = msgh->msg_iov->iov_base;
  }

  free(buf);
  free(iov);
  free(nladdr);
  free(msgh);
  return;
}

/*===========================================================================

FUNCTION QCMAP_NL_RECV()

DESCRIPTION

  This function
  - retrieves the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_nl_recv
(
  int              fd,
  struct msghdr    **msg_pptr,
  unsigned int     *msglen_ptr
)
{
  struct msghdr    *msgh = NULL;
  int              rmsgl;

  /*--------------------------------------------------------------------------
    Allocate the memory to receive the netlink message
  ---------------------------------------------------------------------------*/
  if( NULL == (msgh = qcmap_nl_alloc_msg( QCMAP_NL_MSG_MAX_LEN )) )
  {
    LOG_MSG_ERROR("Failed to allocate NL message",0,0,0);
    goto error;
  }

  /*--------------------------------------------------------------------------
    Receive message over the socket
  ---------------------------------------------------------------------------*/
  rmsgl = recvmsg(fd, msgh, 0);

  /*--------------------------------------------------------------------------
    Verify that something was read
  ---------------------------------------------------------------------------*/
  if (rmsgl <= 0)
  {
    LOG_MSG_ERROR("Received nl_msg, recvmsg failed:",0,0,0);
    perror("NL recv error");
    goto error;
  }

  /*--------------------------------------------------------------------------
    Verify that NL address length in the received message is expected value
  ---------------------------------------------------------------------------*/
  if (msgh->msg_namelen != sizeof(struct sockaddr_nl))
  {
    LOG_MSG_ERROR("rcvd msg with namelen != sizeof sockaddr_nl",0,0,0);
    goto error;
  }

  /*--------------------------------------------------------------------------
    Verify that message was not truncated. This should not occur
  ---------------------------------------------------------------------------*/
  if (msgh->msg_flags & MSG_TRUNC)
  {
    LOG_MSG_ERROR("Rcvd msg truncated!",0,0,0);
    goto error;
  }

  *msg_pptr    = msgh;
  *msglen_ptr = rmsgl;

  return QCMAP_NL_SUCCESS;

error:
  /*--------------------------------------------------------------------------
    An error occurred while receiving the message. Free all memory before
    returning.
  ---------------------------------------------------------------------------*/

  qcmap_nl_release_msg( msgh );
  *msg_pptr    = NULL;
  *msglen_ptr  = 0;
  return QCMAP_NL_FAILURE;
}

/*===========================================================================

FUNCTION QCMAP_NL_DECODE_RTM_NEIGH()

DESCRIPTION

  This function
  - decodes the Netlink Neighbour message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qcmap_nl_decode_rtm_neigh
(
  const char              *buffer,
  unsigned int             buflen,
  qcmap_nl_neigh_info_t   *neigh_info
)
{
  struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;  /* NL message header */
  struct rtattr *rtah = NULL;

  /* Extract the header data */
  neigh_info->metainfo = *((struct ndmsg *)NLMSG_DATA(nlh));
  /* Subtracting NL Header to decode RT MSG*/
  buflen -= sizeof(struct nlmsghdr);

  /* Extract the available attributes */
  neigh_info->attr_info.param_mask = QCMAP_NLA_PARAM_NONE;

  rtah = QCMAP_NDA_RTA(NLMSG_DATA(nlh));

  while(RTA_OK(rtah, buflen))
  {
    switch(rtah->rta_type)
    {
      case NDA_DST:
        neigh_info->attr_info.local_addr.ss_family = neigh_info->metainfo.ndm_family;
        memcpy(&neigh_info->attr_info.local_addr.__ss_padding,
               RTA_DATA(rtah),sizeof(neigh_info->attr_info.local_addr.__ss_padding));
        break;

      case NDA_LLADDR:
        memcpy(neigh_info->attr_info.lladdr_hwaddr.sa_data,
               RTA_DATA(rtah),sizeof(neigh_info->attr_info.lladdr_hwaddr.sa_data));
        break;

      default:
        break;

    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

  return QCMAP_NL_SUCCESS;
}


/*===========================================================================

FUNCTION QCMAP_NL_DECODE_RTM_ADDR()

DESCRIPTION

  This function
  - decodes the Netlink Address message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/

static int qcmap_nl_decode_rtm_addr
(
  const char              *buffer,
  unsigned int             buflen,
  qcmap_nl_if_addr_t      *if_addr_info
)
{
  struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;  /* NL message header */
  struct rtattr *rtah = NULL;

  if (buffer == NULL)
  {
    LOG_MSG_ERROR("qcmap_nl_decode_rtm_addr : NULL buffer passed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  if (if_addr_info == NULL)
  {
    LOG_MSG_ERROR("qcmap_nl_decode_rtm_addr : NULL if_addr_info passed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  /* Extract the header data */
  if_addr_info->metainfo = *((struct ifaddrmsg *)NLMSG_DATA(nlh));

  if (if_addr_info->metainfo.ifa_family != AF_INET)
  {
    LOG_MSG_INFO1("RTM_NEWADDR not received for IPv4",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  /* Subtracting NL Header to decode RT MSG*/
  buflen -= sizeof(struct nlmsghdr);
  rtah = QCMAP_IFA_RTA(NLMSG_DATA(nlh));
  /*Setting ifa_local to zero.*/
  if_addr_info->attr_info.ifa_local = 0;

  while(RTA_OK(rtah, buflen))
  {
    switch(rtah->rta_type)
    {
      case IFA_LOCAL:
        if (if_addr_info->metainfo.ifa_family == AF_INET)
          if_addr_info->attr_info.ifa_local = *((uint32_t *)RTA_DATA(rtah));
        break;

      default:
        break;
    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

return QCMAP_NL_SUCCESS;
}
/*===========================================================================

FUNCTION QCMAP_NL_DECODE_RTM_ROUTE()

DESCRIPTION

  This function
  - decodes the Netlink Route message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qcmap_nl_decode_rtm_route
(
  const char              *buffer,
  unsigned int             buflen,
  qcmap_nl_route_info_s   *route_info
)
{
  struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;  /* NL message header */
  struct rtattr *rtah = NULL;

  /* Extract the header data */
  route_info->metainfo = *((struct rtmsg *)NLMSG_DATA(nlh));
  /* Subtracting NL Header to decode RT MSG*/
  buflen -= sizeof(struct nlmsghdr);

  route_info->attr_info.param_mask = 0;

  if ( route_info->metainfo.rtm_protocol != RTPROT_STATIC )
  {
    /* Ignore the route silently. */
    return QCMAP_NL_SUCCESS;
  }

  rtah = QCMAP_RTN_RTA(NLMSG_DATA(nlh));

  while(RTA_OK(rtah, buflen))
  {
    switch(rtah->rta_type)
    {
      case RTA_DST:
        memcpy(&route_info->attr_info.dst_addr,
               RTA_DATA(rtah),sizeof(route_info->attr_info.dst_addr));
        route_info->attr_info.param_mask |= QCMAP_NL_ROUTE_INFO_DST_ADDR;
        break;
      case RTA_OIF:
        memcpy(&route_info->attr_info.ifindex,
               RTA_DATA(rtah),sizeof(route_info->attr_info.ifindex));
        route_info->attr_info.param_mask |= QCMAP_NL_ROUTE_INFO_IFINDEX;
        break;
      default:
        break;

    }

    /* Advance to next attribute */
    rtah = RTA_NEXT(rtah, buflen);
  }

  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_GET_IF_INDEX_FOR_BRIDGE()

DESCRIPTION

  This function
  - calls a system ioctl to get the interface index for the corresponding
  passed interface name

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_get_if_for_bridge
(
  int   *if_index
)
{
  int      fd;
  struct   ifreq ifr;

  /*-------------------------------------------------------------------------
    Open a socket which is required to call an ioctl
  --------------------------------------------------------------------------*/
  if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("get interface index socket create failed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  memset(&ifr, 0, sizeof(struct ifreq));

  /*-------------------------------------------------------------------------
   Populate the interface name
  --------------------------------------------------------------------------*/
  snprintf((void *)&ifr.ifr_name, sizeof(ifr.ifr_name),BRIDGE_IFACE);

  /*-------------------------------------------------------------------------
   Call the ioctl to get the interface index
  --------------------------------------------------------------------------*/
  if (ioctl(fd, SIOGIFINDEX , &ifr) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: ioctl failed:",0,0,0);
    close(fd);
    return QCMAP_NL_FAILURE;
  }

  *if_index = ifr.ifr_ifindex;
  close(fd);
  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_GET_IF_NAME()

DESCRIPTION

  This function
  - calls a system ioctl to get the interface Name for the corresponding
  passed interface ID

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_get_if_name
(
  char  *if_name,
  int   if_index
)
{
  int      fd;
  struct   ifreq ifr;

  /*-------------------------------------------------------------------------
    Open a socket which is required to call an ioctl
  --------------------------------------------------------------------------*/
  if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("get interface index socket create failed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  memset(&ifr, 0, sizeof(struct ifreq));

  /*-------------------------------------------------------------------------
    Populate the passed interface name
  --------------------------------------------------------------------------*/
  ifr.ifr_ifindex = if_index;

  /*-------------------------------------------------------------------------
    Call the ioctl to get the interface index
  --------------------------------------------------------------------------*/
  if (ioctl(fd,SIOCGIFNAME , &ifr) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: ioctl failed:",0,0,0);
    close(fd);
    return QCMAP_NL_FAILURE;
  }

  (void)strncpy(if_name, ifr.ifr_name, sizeof(ifr.ifr_name));
  close(fd);
  return QCMAP_NL_SUCCESS;
}

/*------------------------------------------------------------------------
  Utility Function to compare dev name
------------------------------------------------------------------------*/
static int qcmap_nl_is_recv_on_usb(char * dev_name)
{
  if ( (strncmp(dev_name,QCMAP_NL_RNDIS_INTERFACE,strlen(QCMAP_NL_RNDIS_INTERFACE)) == 0 ) ||
        (strncmp(dev_name,QCMAP_NL_ECM_INTERFACE,strlen(QCMAP_NL_ECM_INTERFACE))    == 0 ))
      return QCMAP_NL_SUCCESS;
  return QCMAP_NL_FAILURE;
}

static int qcmap_nl_is_recv_on_ecm(char * dev_name)
{
  if (strncmp(dev_name,QCMAP_NL_ECM_INTERFACE,strlen(QCMAP_NL_ECM_INTERFACE)) == 0 )
      return QCMAP_NL_SUCCESS;
  return QCMAP_NL_FAILURE;
}

static int qcmap_nl_is_recv_on_bridge(char * dev_name)
{
  if (strncmp(dev_name,BRIDGE_IFACE,strlen(BRIDGE_IFACE)) == 0 )
     return QCMAP_NL_SUCCESS;
  return QCMAP_NL_FAILURE;
}

static int qcmap_nl_is_recv_on_wlan(char * dev_name)
{
  if (strncmp(dev_name,
              QCMAP_NL_WLAN_INTERFACE,
              strlen(QCMAP_NL_WLAN_INTERFACE)) == 0 )
  {
    return QCMAP_NL_SUCCESS;
  }
  return QCMAP_NL_FAILURE;
}


static int qcmap_nl_is_recv_on_ppp(char * dev_name)
{
  if (strncmp(dev_name,QCMAP_NL_PPP_INTERFACE,strlen(QCMAP_NL_PPP_INTERFACE)) == 0 )
     return QCMAP_NL_SUCCESS;
  return QCMAP_NL_FAILURE;
}

/*===========================================================================

FUNCTION QCMAP_NL_CM_SEND()

DESCRIPTION

  This function
  - Send a message to QCMAP CM Process

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qcmap_nl_cm_send(qcmap_nl_sock_msg_t * buf)
{
  struct sockaddr_un nl_qcmap;
  int numBytes=0, len;

  nl_qcmap.sun_family = AF_UNIX;
  strcpy(nl_qcmap.sun_path, QCMAP_NL_UDS_FILE);
  len = strlen(nl_qcmap.sun_path) + sizeof(nl_qcmap.sun_family);

  if((numBytes = sendto(nl_qcmap_sockfd, (void *)buf, sizeof(qcmap_nl_sock_msg_t), 0,(struct sockaddr *)&nl_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from  QCMAP Netlink thread context, errno:%d",errno, 0, 0);
    return QCMAP_NL_FAILURE;
  }
    LOG_MSG_INFO1("Send succeeded from QCMAP Netlink thread context Bytes Snd: %d", numBytes, 0, 0);
    return QCMAP_NL_SUCCESS;
}


/*===========================================================================

FUNCTION QCMAP_NL_DECODE_NLMSG()

DESCRIPTION

  This function
  - decodes the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qcmap_nl_decode_nlmsg
(
  const char       *buffer,
  unsigned int     buflen,
  qcmap_nl_msg_t   *msg_ptr
)
{

  struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;
  char dev_name[IF_NAME_LEN];
  char ipaddr[INET6_ADDRSTRLEN], ipv6addr[INET6_ADDRSTRLEN];
  int ret_val = QCMAP_NL_FAILURE;
  qcmap_nl_sock_msg_t pktbuf;
  char logmsg[LOG_MSG_LEN];
  uint16       addr_family;
  uint16       addr_size;
  uint8 nullMac[QCMAP_MSGR_MAC_ADDR_LEN_V01];
  static boolean kpi_first_wlan_client = true;

  memset(&pktbuf, 0, sizeof(qcmap_nl_sock_msg_t));
  memset(nullMac, 0, sizeof(nullMac));

  while(NLMSG_OK(nlh, buflen))
  {
    /*-------------------------------------------------------------------------
      Decode the message type
    -------------------------------------------------------------------------*/
    switch(nlh->nlmsg_type)
    {
      /*----------------------------------------------------------------------
        RTM NEWNEIGH
      ----------------------------------------------------------------------*/
      case RTM_NEWNEIGH:
        LOG_MSG_INFO1("RTM_NEWNEIGH recvd",0,0,0);
        if(QCMAP_NL_SUCCESS != qcmap_nl_decode_rtm_neigh(buffer, buflen, &(msg_ptr->nl_neigh_info)))
        {
          LOG_MSG_ERROR("Failed to decode rtm neighbor message\n",0,0,0);
          return QCMAP_NL_FAILURE;
        }

        if ( nlh->nlmsg_seq == QCMAP_NL_GETNEIGH_SEQ )
        {
          LOG_MSG_INFO1("Rcvd NEWNEIGH Event as a result of GETNEIGH event",0,0,0);
          if ((msg_ptr->nl_neigh_info.attr_info.local_addr.ss_family == AF_INET6) &&
              (msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data[0] == 0x33) &&
              (msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data[1] == 0x33))
            {
              LOG_MSG_INFO1("Recieved Neigh Event for IPv6 Multicast MAC Address --- Ignore",0,0,0);
              break;
            }
        }

        ret_val = qcmap_get_if_name(dev_name,
                                    msg_ptr->nl_neigh_info.metainfo.ndm_ifindex);
        if(ret_val != QCMAP_NL_SUCCESS)
        {
           LOG_MSG_ERROR("Error while getting interface index",0,0,0);
           return QCMAP_NL_FAILURE;
        }

        //This check is done prevent handling of netlink messages which have NULL MAC addr
        if(!(memcmp(msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data,
                    nullMac,sizeof(nullMac))))
        {
          LOG_MSG_ERROR("RTM_NEWNEIGH received with NULL MAC",0,0,0);
          break;
        }

        // Only listen to USB & bridge interface
        if ( (qcmap_nl_is_recv_on_usb(dev_name) == QCMAP_NL_FAILURE) &&
             (qcmap_nl_is_recv_on_bridge(dev_name) == QCMAP_NL_FAILURE) )
        {
          LOG_MSG_INFO1("New Neigbour Message was not recieved"
                        " on USB/BRIDGE Interface",0,0,0);
          break;
        }

        // If new Neigh is received on USB interface and there is no IP Addresss.
        // Store the MAC address, since its client MAC
        if ( (qcmap_nl_is_recv_on_usb(dev_name) == QCMAP_NL_SUCCESS) &&
             (msg_ptr->nl_neigh_info.attr_info.local_addr.ss_family == 0) )
        {
          memcpy((void *)&qcmap_nl_usb_mac_addr.ether_addr_octet,
                 msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data,
                 sizeof(ether_addr));
          snprintf((char *)logmsg,
                   LOG_MSG_LEN,
                   "echo USB Client Mac Address is %s > /dev/kmsg",
                   ether_ntoa(&qcmap_nl_usb_mac_addr));
          ds_system_call((char *)logmsg,strlen(logmsg));
           //TODO: Send the USB MAC to Qcmap context, to make behaviour similar to WLAN
          break;
        }

        /* If NEWNEIGH is recieved on Bridge Interface or USB Interface and family=0 OR ipaddr =0;ignore*/
        if ( ( (qcmap_nl_is_recv_on_bridge(dev_name) == QCMAP_NL_SUCCESS) ||
               (qcmap_nl_is_recv_on_usb(dev_name) == QCMAP_NL_SUCCESS) ) &&
             ( (msg_ptr->nl_neigh_info.attr_info.local_addr.ss_family == 0) ||
               (msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding == 0)) )
        {
          ds_log_med(" NEWNEIGH Received on %s IFACE with IP family as %d and ip_addr as %d ",dev_name,
                                                         msg_ptr->nl_neigh_info.attr_info.local_addr.ss_family,
                                                         msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding);
          break;
        }

        //Populate netlink event type
        pktbuf.nl_event = QCMAP_NL_NEWNEIGH;

        //Copy the MAC address from Netlink event
        memcpy(pktbuf.nl_addr.mac_addr,
               msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data,
               QCMAP_MSGR_MAC_ADDR_LEN_V01);

        addr_family = msg_ptr->nl_neigh_info.attr_info.local_addr.ss_family;
        addr_size = (addr_family == AF_INET) ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;

        if (addr_family == AF_INET)
        {
          //Copy the IPv4 address
          memcpy(&pktbuf.nl_addr.ip_addr,
                 msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding,
                 sizeof(uint32_t));
          pktbuf.nl_addr.isValidIPv4address = true;
        }
        else if (addr_family == AF_INET6)
        {
          //Copy the IPv6 address
          memcpy(pktbuf.nl_addr.ip_v6_addr,
                 msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding,
                 QCMAP_MSGR_IPV6_ADDR_LEN_V01);
          pktbuf.nl_addr.isValidIPv6address = true;
        }
        else
        {
          LOG_MSG_ERROR("Incorrect Address Family type %d",addr_family,0,0);
          break;
        }

        /* If Received event *DOES* have USB Client MAC and
                 is generated on bridge interface then copy client IP */
        if ((qcmap_nl_is_recv_on_bridge(dev_name) == QCMAP_NL_SUCCESS) &&
            (!memcmp(&qcmap_nl_usb_mac_addr.ether_addr_octet,
                     msg_ptr->nl_neigh_info.attr_info.lladdr_hwaddr.sa_data,
                     sizeof(qcmap_nl_usb_mac_addr))))
        {
          pktbuf.nl_iface = QCMAP_NL_USB;
          inet_ntop(addr_family,
                      (void *)msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding,
                      ipaddr, addr_size);
          snprintf((char *)logmsg,
                     LOG_MSG_LEN,
                     "echo QCMAP:USB Client IP Addr %s > /dev/kmsg",
                     ipaddr);
          ds_system_call((char *)logmsg,strlen(logmsg));
        }
        else
        {
          pktbuf.nl_iface = QCMAP_NL_WLAN;
          inet_ntop(addr_family,
                      (void *)msg_ptr->nl_neigh_info.attr_info.local_addr.__ss_padding,
                      ipaddr, addr_size);
          ds_log_med(" New Neigh Address on WLAN %s \n",ipaddr);
          if ( pktbuf.nl_addr.isValidIPv4address == true )
          {
            /* Log only the first wlan client ip address */
            if( kpi_first_wlan_client )
            {
              snprintf( (char *)logmsg,
                        LOG_MSG_LEN,
                        "echo QCMAP:WLAN Client IP Addr %s > /dev/kmsg",
                        ipaddr );
              ds_system_call((char *)logmsg,strlen(logmsg));
              kpi_first_wlan_client = false;
            }
          }
        }

        if (QCMAP_NL_FAILURE == qcmap_nl_cm_send(&pktbuf))
        {
          LOG_MSG_ERROR("Send Failed from QCMAP Netlink thread context",
                        0, 0, 0);
        }
        else
        {
          LOG_MSG_INFO1("Send succeeded from QCMAP Netlink thread context",
                        0, 0, 0);
        }
        break;

      case RTM_NEWADDR:
        LOG_MSG_INFO1("RTM_NEWADDR recvd",0,0,0);
        if(QCMAP_NL_SUCCESS != qcmap_nl_decode_rtm_addr(buffer,
                                                        buflen,
                                                        &(msg_ptr->nl_if_addr)))
        {
          LOG_MSG_ERROR("Failed to decode rtm address message\n",0,0,0);
          return QCMAP_NL_FAILURE;
        }
        ret_val = qcmap_get_if_name(dev_name,
                                    msg_ptr->nl_if_addr.metainfo.ifa_index);
        if(ret_val != QCMAP_NL_SUCCESS)
        {
          LOG_MSG_ERROR("Error while getting interface name",0,0,0);
          return QCMAP_NL_FAILURE;
        }

        // Only listen to wlan and bridge interface
        if ((qcmap_nl_is_recv_on_wlan(dev_name) == QCMAP_NL_FAILURE) &&
            (qcmap_nl_is_recv_on_bridge(dev_name) == QCMAP_NL_FAILURE) &&
            (qcmap_nl_is_recv_on_ecm(dev_name) == QCMAP_NL_FAILURE))
        {
          LOG_MSG_INFO1("RTM_NEWADDR recvd on interface other than WLAN/BRIDGE/ECM",0,0,0);
          break;
        }

        // NEWADDR received for IPv4 events
        if (msg_ptr->nl_if_addr.metainfo.ifa_family == AF_INET)
        {
          /* Ignore link local ip. */
          if (msg_ptr->nl_if_addr.attr_info.ifa_local &&
              !QCMAP_IFA_IS_LINK_LOCAL_IP(msg_ptr->nl_if_addr.attr_info.ifa_local))
          {
            pktbuf.nl_addr.ip_addr = msg_ptr->nl_if_addr.attr_info.ifa_local;
            LOG_MSG_INFO1("IP Addr of wlan0 is ",0,0,0);
            IPV4_ADDR_MSG(pktbuf.nl_addr.ip_addr);
            // Send Update to QCMAP
            pktbuf.nl_event = QCMAP_NL_NEWADDR;
            if ((qcmap_nl_is_recv_on_wlan(dev_name) == QCMAP_NL_SUCCESS))
              pktbuf.nl_iface = QCMAP_NL_WLAN;
            else if (qcmap_nl_is_recv_on_bridge(dev_name) == QCMAP_NL_SUCCESS)
              pktbuf.nl_iface = QCMAP_NL_BRIDGE;
            else
              pktbuf.nl_iface = QCMAP_NL_USB;

            if (QCMAP_NL_FAILURE == qcmap_nl_cm_send(&pktbuf))
            {
              LOG_MSG_ERROR("Send Failed from QCMAP Netlink thread context",
                            0, 0, 0);
            }
            else
            {
              LOG_MSG_INFO1("Send succeeded in QCMAP Netlink thread context",
                            0, 0, 0);
            }
          }
        }

        break;

      /*----------------------------------------------------------------------
        RTM_NEWROUTE
      ----------------------------------------------------------------------*/
      case RTM_NEWROUTE:
        if(QCMAP_NL_SUCCESS != qcmap_nl_decode_rtm_route(buffer, buflen, &(msg_ptr->nl_route_info)))
        {
          LOG_MSG_ERROR("Failed to decode rtm neighbor message\n",0,0,0);
          return QCMAP_NL_FAILURE;
        }

        if ( !((msg_ptr->nl_route_info.attr_info.param_mask & QCMAP_NL_ROUTE_INFO_DST_ADDR) &&
             (msg_ptr->nl_route_info.attr_info.param_mask & QCMAP_NL_ROUTE_INFO_IFINDEX)) )
        {
          /* Not the required route message ignore and continue. */
          LOG_MSG_ERROR("Not the required route ignore.",0,0,0);
          break;
        }

        ret_val = qcmap_get_if_name(dev_name, msg_ptr->nl_route_info.attr_info.ifindex);
        if(ret_val != QCMAP_NL_SUCCESS)
        {
           LOG_MSG_ERROR("Error while getting interface index",0,0,0);
           return QCMAP_NL_FAILURE;
        }

        // Only listen to PPP interface
        if ( qcmap_nl_is_recv_on_ppp(dev_name) == QCMAP_NL_FAILURE )
        {
           LOG_MSG_INFO1("Route message was not recieved on PPP Interface",0,0,0);
           break;
        }

        if ( msg_ptr->nl_route_info.metainfo.rtm_family != AF_INET6)
        {
           LOG_MSG_INFO1("Route message was not recieved on PPP Interface",0,0,0);
           break;
        }

        // Send Update to QCMAP
        pktbuf.nl_event = QCMAP_NL_PPP_IPV6_ROUTE;
        pktbuf.nl_iface = QCMAP_NL_PPP;
        memcpy(&pktbuf.nl_addr.ip_v6_addr, &msg_ptr->nl_route_info.attr_info.dst_addr, sizeof(struct in6_addr));
        if (QCMAP_NL_FAILURE == qcmap_nl_cm_send(&pktbuf))
        {
          LOG_MSG_ERROR("Send Failed from PPP QCMAP Netlink thread context", 0, 0, 0);
        }
        else
        {
          LOG_MSG_INFO1("Send succeeded in PPP QCMAP Netlink thread context", 0, 0, 0);
        }
        break;
      default:
        break;
    }
    nlh = NLMSG_NEXT(nlh, buflen);
  }
  return QCMAP_NL_SUCCESS;
}


/*===========================================================================

FUNCTION QCMAP_NL_RECV_MSG()

DESCRIPTION

  This function
  - receives the netlink message associated with the netlink event which
  was received on select call
  - decodes the netlink message to find if the netlink message was found
  on the desired interface
  - Posts it to QCMAP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qcmap_nl_recv_msg(int fd)
{
  struct msghdr                    *msghdr = NULL;
  struct sockaddr_nl               *nladdr = NULL;
  struct iovec                     *iov = NULL;
  unsigned int                     msglen = 0;
  qcmap_nl_msg_t                   *nlmsg = NULL;
  int                              interface_num,interface;

  /*--------------------------------------------------------------------------
    Allocate memory to decode the netlink message
  ---------------------------------------------------------------------------*/
  nlmsg = malloc(sizeof(qcmap_nl_msg_t));
  if(NULL == nlmsg)
  {
    LOG_MSG_ERROR("Failed alloc of nlmsg",0,0,0);
    goto error;
  }
  else
  {
    /*--------------------------------------------------------------------------
      Receive the netlink message
    ---------------------------------------------------------------------------*/
    if(QCMAP_NL_SUCCESS != qcmap_nl_recv(fd, &msghdr, &msglen))
    {
      LOG_MSG_ERROR("Failed to receive nl message",0,0,0);
      goto error;
    }

    if( msghdr == NULL )
    {
      LOG_MSG_ERROR("Netlink: NULL message received",0,0,0);
      goto error;
    }

    nladdr = msghdr->msg_name;
    iov = msghdr->msg_iov;

    /*--------------------------------------------------------------------------
      Decode the received netlink message
    ---------------------------------------------------------------------------*/
    memset(nlmsg, 0, sizeof(qcmap_nl_msg_t));
    if(QCMAP_NL_SUCCESS != qcmap_nl_decode_nlmsg((char*)iov->iov_base,
                                          msglen,
                                          nlmsg))
    {
      LOG_MSG_ERROR("Failed to decode nl message",0,0,0);
      goto error;
    }
  }

  if(msghdr)
  {
    qcmap_nl_release_msg(msghdr);
  }
  if(nlmsg)
  {
    free(nlmsg);
  }

  return QCMAP_NL_SUCCESS;
error:
  if(msghdr)
  {
    qcmap_nl_release_msg(msghdr);
  }
  if(nlmsg)
  {
    free(nlmsg);
  }

  return QCMAP_NL_FAILURE;
}

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
int qcmap_nl_sock_listener_start(qcmap_nl_sk_fd_set_info_t    *sk_fd_set)
{
  int i,ret;
  LOG_MSG_INFO1("Starting QCMAP Netlink listener",0,0,0);

  while(TRUE)
  {
    for(i = 0; i < sk_fd_set->num_fd; i++ )
    {
      FD_SET(sk_fd_set->sk_fds[i].sk_fd, &(sk_fd_set->fdset));
    }

    /*--------------------------------------------------------------------------
      Call select system function which will listen to netlink events
      coming on netlink socket which we would have opened during
      initialization
    --------------------------------------------------------------------------*/
    if((ret = select(sk_fd_set->max_fd+1,
                     &(sk_fd_set->fdset),
                     NULL,
                     NULL,
                     NULL)) < 0)
    {
      LOG_MSG_ERROR("Error in select, errno:%d", errno, 0, 0);
      if( errno == EINTR )
      {
        continue;
      }
      else {
        return -1;
        LOG_MSG_ERROR("qcmap nl select failed", 0, 0, 0);
      }
    }
    else
    {
      for(i = 0; i < sk_fd_set->num_fd; i++ )
      {
        if( FD_ISSET(sk_fd_set->sk_fds[i].sk_fd,
                     &(sk_fd_set->fdset) ) )
        {
          if(sk_fd_set->sk_fds[i].read_func)
          {
            if( QCMAP_NL_SUCCESS != ((sk_fd_set->sk_fds[i].read_func)(sk_fd_set->sk_fds[i].sk_fd)) )
            {
              LOG_MSG_ERROR("Error on read callback[%d] fd=%d",
                            i,
                            sk_fd_set->sk_fds[i].sk_fd,
                            0);
            }
            else
            {
              FD_CLR(sk_fd_set->sk_fds[i].sk_fd, &(sk_fd_set->fdset));
            }
          }
          else
          {
            LOG_MSG_ERROR("No read function",0,0,0);
          }
        }
      }
    }
  }
  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_OPEN_PACKET_SOCKET()

DESCRIPTION

  This function
  - opens paket sockets
  - binds the socket to listen to the required NA packets.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qcmap_open_packet_socket
(
  qcmap_pf_sk_info_t     *sk_info
)
{
  int *packet_sock;
  int ret= -1;
  struct sockaddr_ll sa;
  struct sock_fprog fprog;

  struct sock_filter filter[] =
  {
    /*
     * {opcode, jump to offset if true, jump to offset if false, value}
     * 0x30 --> Opcode stands for offset value to move.
     * 0x15 --> Opcode to compare with value.
    */
    /* Move to offset 0 i.e check the first byte of IP packet.*/
    { 0x30, 0, 0, 0x00000000 },
    /* Compare if it is IPV6. If true move to next command, else move to end.*/
    { 0x15, 0, 5, 0x00000060},
    /* Move to offset 6 i.e to check if it is ICMPV6.*/
    { 0x30, 0, 0, 0x00000006 },
    /* Compare if it is ICMPV6. If true move to next command, else move to end.*/
    { 0x15, 0, 3, 0x0000003a },
    /* Move to offset 40 i.e to check if it is RA packet.*/
    { 0x30, 0, 0, 0x00000028 },
    /* Compare if it is RA packet. If true move to next command, else move to end.*/
    { 0x15, 0, 1, 0x00000086 },
    /* Return maximum of 0xffff bytes..*/
    { 0x6, 0, 0, 0x0000ffff },
    /* Return 0 bytes..*/
    { 0x6, 0, 0, 0x00000000 }
  };

  ds_assert(sk_info != NULL);

  fprog.len = (sizeof(filter)/sizeof(struct sock_filter));
  fprog.filter = filter;

  LOG_MSG_INFO1("Entering qcmap_open_packet_socket %d 0x%p", fprog.len, fprog.filter, 0);

  packet_sock = &(sk_info->sk_fd);

/*--------------------------------------------------------------------------
  Open netlink socket for specified protocol
---------------------------------------------------------------------------*/
  if ((*packet_sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IPV6))) < 0)
  {
    LOG_MSG_ERROR("Cannot open packet socket",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  LOG_MSG_INFO1("Socket open succeeds",0,0,0);

  /* bind this socket to specific iface */
  memset(&sa, 0, sizeof(sa));
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(ETH_P_IPV6);
  sa.sll_ifindex = 0;

  if (bind(*packet_sock, (struct sockaddr *)&sa, sizeof(sa)) != 0)
  {
    LOG_MSG_ERROR("couldn't bind socket %d to iface 0, errno=%d",
           *packet_sock, errno, 0);
    return QCMAP_NL_FAILURE;
  }

  /* install filter to only receive unsolicited ICMPv6 RA traffic */
  if (setsockopt(*packet_sock,
                 SOL_SOCKET,
                 SO_ATTACH_FILTER,
                 &fprog,
                 sizeof(fprog)) == -1)
  {
    LOG_MSG_ERROR("couldn't attach BPF filter on sock %d, errno=%d",
                  *packet_sock, errno, 0);
    return QCMAP_NL_FAILURE;
  }

  return QCMAP_NL_SUCCESS;

}

/*===========================================================================
FUNCTION QCMAP_PACKET_SOCKET_INIT()

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
)
{
  qcmap_pf_sk_info_t     sk_info;
  int                    ret_val;

  LOG_MSG_INFO1("Initialize packet socket for QCMAP",0,0,0);
  memset(&sk_info, 0, sizeof(qcmap_pf_sk_info_t));

  /*---------------------------------------------------------------------------
    Open netlink sockets
  ----------------------------------------------------------------------------*/

  if( qcmap_open_packet_socket( &sk_info ) == QCMAP_NL_SUCCESS)
  {
    LOG_MSG_INFO1("Open: QCMAP packet socket succeeds",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Open: QCMAP packet socket failed",0,0,0);
    return QCMAP_NL_FAILURE;
  }

  /*--------------------------------------------------------------------------
    Add the NETLINK socket to the list of sockets that the listener
    thread should listen on.
  ---------------------------------------------------------------------------*/
  if( qcmap_nl_addfd_map(sk_fdset,sk_info.sk_fd,read_f ) != QCMAP_NL_SUCCESS)
  {
    LOG_MSG_ERROR("cannot add packet sock for reading",0,0,0);
    close(sk_info.sk_fd);
    return QCMAP_NL_FAILURE;
  }

  LOG_MSG_INFO1("add fd map succeeds FD: %d ",sk_info.sk_fd, 0, 0);

  return QCMAP_NL_SUCCESS;
}
/*===========================================================================

FUNCTION QCMAP_PACKET_SOCK_RECV_MSG()

DESCRIPTION

  This function
  - receives the packet socket message.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qcmap_packet_sock_recv_msg(int fd)
{
  struct ip6_hdr *ipv6_packet;
  struct nd_router_advert *icmpv6_ra_packet;
  struct nd_opt_prefix_info *prefix_info;/* prefix information */
  struct nd_opt_hdr *curr_option;
#define MAX_ND_PKT_SIZE 1280
  char nd_packet_buf[MAX_ND_PKT_SIZE], *option_buf;
  ssize_t pkt_size;
  int i=0;
  char ipaddr[INET6_ADDRSTRLEN] = {0};
  qcmap_nl_sock_msg_t pktbuf;
  boolean link_address_set = false;
  uint8 nullMac[QCMAP_MSGR_MAC_ADDR_LEN_V01] = {0};

  /*--------------------------------------------------------------------------
    Read the packet over the socket.
  ---------------------------------------------------------------------------*/
  LOG_MSG_INFO1("ND packet received", 0, 0, 0);
  memset(nd_packet_buf, 0, sizeof(nd_packet_buf));
  pkt_size = recv(fd, nd_packet_buf, MAX_ND_PKT_SIZE, 0);

  if ( pkt_size <= 0 )
  {
    LOG_MSG_ERROR("Not the expected packet %d: ignore", errno, 0, 0);
    return QCMAP_NL_FAILURE;
  }

  /* Check if the packet is ICMPV6. */
  ipv6_packet = (struct ip6_hdr *)nd_packet_buf;
  if ( ipv6_packet->ip6_nxt != IPPROTO_ICMPV6 )
  {
    LOG_MSG_ERROR("Not ICMPV6 packet 0x%x: ignore", ipv6_packet->ip6_nxt, 0, 0);
    return QCMAP_NL_FAILURE;
  }

  icmpv6_ra_packet = (struct nd_router_advert *)(nd_packet_buf + sizeof(struct ip6_hdr));

  memset(&pktbuf, 0, sizeof(qcmap_nl_sock_msg_t));
  memcpy(pktbuf.nl_addr.ra_ipv6_dst_addr,ipv6_packet->ip6_dst.s6_addr, sizeof(struct in6_addr));
  pktbuf.nl_event =   QCMAP_NL_RA;
  pktbuf.nl_iface = QCMAP_NL_ANY;

  inet_ntop(AF_INET6, pktbuf.nl_addr.ra_ipv6_dst_addr, ipaddr, INET6_ADDRSTRLEN);

  option_buf =
          (nd_packet_buf + sizeof(struct ip6_hdr) + sizeof(struct nd_router_advert));

  /* Decrement Length and see if we have room for options. */
  pkt_size -= (sizeof(struct ip6_hdr) + sizeof(struct nd_router_advert));


  /* Look for Link Address option. */
  while (pkt_size > 0)
  {
    if ( pkt_size > sizeof(struct nd_opt_hdr) )
    {
      curr_option = (struct nd_opt_hdr *)option_buf;
      #define OCTET_SIZE 8
      if ( (curr_option->nd_opt_type != ND_OPT_PREFIX_INFORMATION) ||
           (((curr_option->nd_opt_len)*OCTET_SIZE)  != (sizeof(struct nd_opt_prefix_info))) )
      {
        pkt_size -= (curr_option->nd_opt_len*OCTET_SIZE);
        option_buf += (curr_option->nd_opt_len*OCTET_SIZE);
        continue;
      }

      prefix_info = (struct nd_opt_prefix_info *)curr_option;

      /* Ignore deprecated prefix. */
      #define DEPRECATED_PREFERRED_LIFE_TIME 1
      if ( prefix_info->nd_opt_pi_preferred_time == htonl(DEPRECATED_PREFERRED_LIFE_TIME))
      {
        LOG_MSG_ERROR("RA to deprecate prefix. Ignore!!", 0, 0, 0);
        return QCMAP_NL_FAILURE;
      }

      pktbuf.nl_addr.isValidIPv6address = true;
      memcpy(&pktbuf.nl_addr.ip_v6_addr,prefix_info->nd_opt_pi_prefix.s6_addr, sizeof(struct in6_addr));

      inet_ntop(AF_INET6, pktbuf.nl_addr.ip_v6_addr, ipaddr, INET6_ADDRSTRLEN);

      if (QCMAP_NL_FAILURE == qcmap_nl_cm_send(&pktbuf))
      {
        LOG_MSG_ERROR("Send Failed from QCMAP Netlink thread context",
                      0, 0, 0);
      }
      else
      {
        LOG_MSG_INFO1("Send succeeded from QCMAP Netlink thread context",
                      0, 0, 0);
      }
      break;
    }
    else
    {
      break;
    }
  }

  return QCMAP_NL_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_NL_SEND_GETNEIGH_EVENT()

DESCRIPTION

  This function
  - Sends a RTM_NEWNEIGH message for bridge iface.

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP_NL_SUCCESS on success
  QCMAP_NL_FAILURE on failure


SIDE EFFECTS
  None
==========================================================================*/
int qcmap_nl_send_getneigh_event(void)
{
  ssize_t            sndbytes=0;
  int ifindex=-1;
  struct {
    struct nlmsghdr n;
    struct ndmsg r;
  } req;

  ds_log_med("%s %d \n",__func__,__LINE__);

  if (qcmap_get_if_for_bridge(&ifindex) == QCMAP_NL_SUCCESS)
  {
    LOG_MSG_INFO1("Bridge Iface Index = %d",ifindex,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Bridge Iface retrival error Index = %d",ifindex,0,0);
    return QCMAP_NL_FAILURE;
  }

  /* Send the GETNEIGH message to the kernel*/
  memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT | NLM_F_REQUEST;
  req.n.nlmsg_seq = QCMAP_NL_GETNEIGH_SEQ;
  req.n.nlmsg_pid = QCMAP_NL_SOCK_PORTID;
  req.n.nlmsg_type = RTM_GETNEIGH;
  req.r.ndm_state = NUD_REACHABLE | NUD_STALE | NUD_DELAY;
  req.r.ndm_family = AF_INET6;
  req.r.ndm_ifindex = ifindex;

  ds_log_med(" Dump getneigh len=%d , flags =%d, seq=%d, pid=%d, type=%d, family=%d ifindex %d\n",
               req.n.nlmsg_len, req.n.nlmsg_flags, req.n.nlmsg_seq, req.n.nlmsg_pid, req.n.nlmsg_type, req.r.ndm_family,  req.r.ndm_ifindex);
  sndbytes = send(qcmap_nl_listen_sock_fd, &req, req.n.nlmsg_len, 0);

  if (sndbytes < 0)
  {
    ds_log_med("Send GETNEIGH failed: %s \n",strerror(errno));
    return QCMAP_NL_FAILURE;
  }
  else
  {
    ds_log_med("Send GETNEIGH succeded: %d bytes send \n",sndbytes);
  }

  return QCMAP_NL_SUCCESS;
}
