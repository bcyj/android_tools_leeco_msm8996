/******************************************************************************

                        QTI_NETLINK.C

******************************************************************************/

/******************************************************************************

  @file    qti_netlink.c
  @brief   Tethering Interface module Netlink Messaging Implementation File

  DESCRIPTION
  Implementation file for Tethering Interface module Netlink messaging functions.

  ---------------------------------------------------------------------------
   Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
04/04/13   sb         Add support for dynamic switching between USB PIDs
11/15/12   sb         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>

#include "qti.h"
#include "qti_cmdq.h"

/*===========================================================================
                              VARIABLE DEFINITIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   QTI configuration file
---------------------------------------------------------------------------*/
static qti_conf_t * qti_netlink_conf;

static qti_nl_sk_info_t     sk_info;


/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================

FUNCTION QTI_GET_IF_INDEX()

DESCRIPTION

  This function
  - calls a system ioctl to get the interface ID for the corresponding
  passed interface name

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_get_if_index
(
  char *   if_name,
  int *    if_index
)
{
  int      fd;
  struct   ifreq ifr;

/*------------------------------------------------------------------------*/
  LOG_MSG_INFO1("Entering get interface index",0,0,0);

/*-------------------------------------------------------------------------
  Open a socket which is required to call an ioctl
--------------------------------------------------------------------------*/
  if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("get interface index socket create failed",0,0,0);
    return QTI_FAILURE;
  }

  memset(&ifr, 0, sizeof(struct ifreq));

/*-------------------------------------------------------------------------
  Populate the passed interface name
--------------------------------------------------------------------------*/
  strlcpy((char *)ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
  LOG_MSG_INFO1("interface name %s", ifr.ifr_name,0,0);

/*-------------------------------------------------------------------------
  Call the ioctl to get the interface index
--------------------------------------------------------------------------*/
  if (ioctl(fd,SIOCGIFINDEX , &ifr) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: ioctl failed:",0,0,0);
    close(fd);
    return QTI_FAILURE;
  }

  *if_index = ifr.ifr_ifindex;
  LOG_MSG_INFO1("Interface index %d", *if_index,0,0);
  close(fd);
  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_NETLINK_INIT()

DESCRIPTION

  This function initializes QTI:
  - obtains the interface index and name mapping
  - initializes the QTI configuration varaible

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/

int qti_netlink_init(qti_conf_t * qti_conf)
{
  int    i;
  int    ret_val;

/*------------------------------------------------------------------------*/
  //ds_assert(qti_conf != NULL);

  LOG_MSG_INFO1("qti_netlink_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Static pointer to QTI configuration variable
------------------------------------------------------------------------------*/
  qti_netlink_conf = qti_conf;

/*-------------------------------------------------------------------------
   Initialize QTI netlink state
--------------------------------------------------------------------------*/
  qti_netlink_conf->state = QTI_INIT;

/*-------------------------------------------------------------------------
   Initialize interface names
--------------------------------------------------------------------------*/
  strlcpy(qti_netlink_conf->if_dev[0].dev_name,
              RNDIS_INTERFACE,
              IF_NAME_LEN);
  strlcpy(qti_netlink_conf->if_dev[1].dev_name,
              ECM_INTERFACE,
              IF_NAME_LEN);
  strlcpy(qti_netlink_conf->if_dev[2].dev_name,
              ODU_INTERFACE,
              IF_NAME_LEN);

/*-------------------------------------------------------------------------
   Initialize interface indices to default and then obtain the right
   interface indices.
--------------------------------------------------------------------------*/
  for(i=0; i < QTI_INTERFACES; i++)
  {
    qti_netlink_conf->if_dev[i].if_index = QTI_DEFAULT_INTERFACE_ID;
    ret_val = qti_get_if_index(qti_netlink_conf->if_dev[i].dev_name,
                               &(qti_netlink_conf->if_dev[i].if_index));
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Error while getting interface index for %d device", i, 0, 0);
    }

    LOG_MSG_INFO1("Device names and index: %s = %d",
                  qti_netlink_conf->if_dev[i].dev_name,
                  qti_netlink_conf->if_dev[i].if_index, 0);
  }

  return QTI_SUCCESS;
}



/*===========================================================================

FUNCTION QTI_NL_OPEN_SOCKET()

DESCRIPTION

  This function
  - opens netlink sockets
  - binds the socket to listen to the required netlink events

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_nl_open_socket
(
  int                  protocol,
  unsigned int         grps
)
{
  int                  * p_sk_fd;
  struct sockaddr_nl   * p_sk_addr_loc ;
/*-------------------------------------------------------------------------*/


  LOG_MSG_INFO1("Entering qti_nl_open_socket", 0, 0, 0);

  p_sk_fd = &(sk_info.sk_fd);
  p_sk_addr_loc = &(sk_info.sk_addr_loc);

/*--------------------------------------------------------------------------
  Open netlink socket for specified protocol
---------------------------------------------------------------------------*/
  if ((*p_sk_fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0)
  {
    LOG_MSG_ERROR("Cannot open netlink socket",0,0,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Socket open succeeds",0,0,0);

/*--------------------------------------------------------------------------
  Initialize socket parameters to 0
--------------------------------------------------------------------------*/
  memset(p_sk_addr_loc, 0, sizeof(struct sockaddr_nl));

/*-------------------------------------------------------------------------
  Populate socket parameters
--------------------------------------------------------------------------*/
  p_sk_addr_loc->nl_family = AF_NETLINK;
  p_sk_addr_loc->nl_pid = getpid();
  p_sk_addr_loc->nl_groups = grps;

/*-------------------------------------------------------------------------
  Bind socket to receive the netlink events for the required groups
--------------------------------------------------------------------------*/

  if( bind( *p_sk_fd,
            (struct sockaddr *)p_sk_addr_loc,
            sizeof(struct sockaddr_nl) ) < 0)
  {
    LOG_MSG_ERROR("Socket bind failed",0,0,0);
    return QTI_FAILURE;
  }
  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_NL_ADDFD_MAP()

DESCRIPTION

  This function
  - maps the socket descriptor with the corresponding callback function
  - add the socket descriptor to the set of socket desc the listener thread
    listens on.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_nl_addfd_map
(
  qti_sk_fd_set_info_t      * fd_set,
  int                          fd,
  qti_sock_thrd_fd_read_f      read_f
)
{
  ds_assert(fd_set != NULL);
  LOG_MSG_INFO1("Entering qti_nl_addfd_map() fd= %d",fd,0,0);

  if( fd_set->num_fd < MAX_NUM_OF_FD )
  {
/*-----------------------------------------------------------------------
  Add the fd to the fd set which the listener thread should listen on
------------------------------------------------------------------------*/
    FD_SET(fd, &fd_set->fdset);

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
    return QTI_FAILURE;
  }

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_NL_ALLOC_MSG()

DESCRIPTION

  This function
  - allocates memory to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static struct msghdr * qti_nl_alloc_msg
(
  uint32_t         msglen
)
{
  unsigned char          *buf = NULL;
  struct sockaddr_nl     * nladdr = NULL;
  struct iovec           * iov = NULL;
  struct msghdr          * msgh = NULL;

/*-------------------------------------------------------------------------*/

  if(QTI_NL_MSG_MAX_LEN < msglen)
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

FUNCTION QTI_NL_RELEASE_MSG()

DESCRIPTION

  This function
  - releases memory which was alloacted to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static void qti_nl_release_msg
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

FUNCTION QTI_NL_QUERY_IF()

DESCRIPTION

  This function
  - brings up RNDIS and ECM interfaces to listen to netlink events
    coming up on those interfaces
  - sends RTM_GETLINK to kernel to check if RNDIS/ECM interfaces
    are running

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qti_nl_query_if()
{
  struct msghdr        *nl_msg_hdr = NULL;
  nl_req_type          *nl_req = NULL;

/*------------------------------------------------------------------------*/
  if(sk_info.sk_fd < 0)
  {
    LOG_MSG_ERROR("qti_nl_query_if(): Invalid netlink fd",
                  0, 0, 0);
    return QTI_FAILURE;
  }

/*-------------------------------------------------------------------------
  Bring up rndis0, ecm0, and odu0 interfaces to listen to netlink events on them.
  This was moved from QTI initializaton to here to allow USB to complete
  rndis0, ecm0, odu0 enumeration.
--------------------------------------------------------------------------*/
  LOG_MSG_INFO1("ifconfig rndis0 up", 0, 0, 0);
  ds_system_call("ifconfig rndis0 up", strlen("ifconfig rndis0 up"));
  LOG_MSG_INFO1("ifconfig ecm0 up", 0, 0, 0);
  ds_system_call("ifconfig ecm0 up", strlen("ifconfig ecm0 up"));
  LOG_MSG_INFO1("ifconfig odu0 up", 0, 0, 0);
  ds_system_call("ifconfig odu0 up", strlen("ifconfig odu0 up"));

/*-------------------------------------------------------------------------
  Send RTM_GETLINK to find if RNDIS/ECM interfaces are running
-------------------------------------------------------------------------*/
  LOG_MSG_INFO1("qti_nl_query_if(): Sending RTM_GETLINK to kernel",0,0,0);
  nl_msg_hdr = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN );
  if(nl_msg_hdr == NULL)
  {
    LOG_MSG_ERROR("qti_nl_query_if(): Failed in qti_nl_alloc_msg",
                  0, 0, 0);
    return QTI_FAILURE;
  }
  nl_req = (nl_req_type *)(nl_msg_hdr->msg_iov->iov_base);

/*-------------------------------------------------------------------------
  Populate the required parameters in the netlink message
---------------------------------------------------------------------------*/
  nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  nl_req->hdr.nlmsg_type = RTM_GETLINK ;
  /* NLM_F_REQUEST - has to be set for request messages
     NLM_F_DUMP -  equivalent to NLM_F_ROOT|NLM_F_MATCH */
  nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nl_req->hdr.nlmsg_seq = 1;
  nl_req->hdr.nlmsg_pid = sk_info.sk_addr_loc.nl_pid;
  nl_req->gen.rtgen_family = AF_PACKET;

  if(sendmsg(sk_info.sk_fd, (struct msghdr *)nl_msg_hdr, 0)<=0)
  {
    LOG_MSG_ERROR("GETLINK send nl msg failed", 0, 0, 0);
  }

  LOG_MSG_INFO1("GETLINK send nl msg succeeded", 0, 0, 0);

  free(nl_msg_hdr->msg_iov->iov_base);
  free(nl_msg_hdr->msg_iov);
  free(nl_msg_hdr->msg_name);
  free(nl_msg_hdr);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_NL_LISTENER_INIT()

DESCRIPTION

  This function
  - initialises netlink sockets
  - sends an RTM_GETLINK which queries all kernel interfaces
    for netlink events
  - starts listening on netlink events

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qti_nl_listener_init
(
  unsigned int               nl_type,
  unsigned int               nl_groups,
  qti_sk_fd_set_info_t    * sk_fdset,
  qti_sock_thrd_fd_read_f    read_f
)
{
  int                  ret_val;
/*--------------------------------------------------------------------------*/

  //ds_assert(sk_fdset != NULL);

  LOG_MSG_INFO1("Initialize netlink socket",0,0,0);

  memset(&sk_info, 0, sizeof(qti_nl_sk_info_t));

/*---------------------------------------------------------------------------
  Open netlink sockets
----------------------------------------------------------------------------*/

  if( qti_nl_open_socket(nl_type, nl_groups) == QTI_SUCCESS)
  {
    LOG_MSG_INFO1("Open netlink socket succeeds",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Netlink socket open failed",0,0,0);
    return QTI_FAILURE;
  }

/*--------------------------------------------------------------------------
  Add the NETLINK socket to the list of sockets that the listener
  thread should listen on.
---------------------------------------------------------------------------*/
  if( qti_nl_addfd_map(sk_fdset,sk_info.sk_fd,read_f ) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("cannot add nl routing sock for reading",0,0,0);
    close(sk_info.sk_fd);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("add fd map succeeds", 0, 0, 0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_NL_RECV()

DESCRIPTION

  This function
  - retrieves the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_nl_recv
(
  int              fd,
  struct msghdr    ** msg_pptr,
  unsigned int     *  msglen_ptr
)
{
  struct msghdr    * msgh = NULL;
  int              rmsgl;
/*------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  Allocate the memory to receive the netlink message
---------------------------------------------------------------------------*/
  if( NULL == (msgh = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN )) )
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

  LOG_MSG_INFO1("Received message size %d", rmsgl, 0, 0);

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

  return QTI_SUCCESS;

error:
/*--------------------------------------------------------------------------
  An error occurred while receiving the message. Free all memory before
  returning.
---------------------------------------------------------------------------*/

  qti_nl_release_msg( msgh );
  *msg_pptr    = NULL;
  *msglen_ptr  = 0;
  return QTI_FAILURE;
}

/*===========================================================================

FUNCTION QTI_NL_DECODE_RTM_LINK()

DESCRIPTION

  This function
  - Iterate over the array of interfaces, and return the link id for
    the interface matching the specified if index.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qti_get_link_for_ifindex (int ifindex)
{
  int link = -1;
  int i;

  for( i = 0; i < QTI_INTERFACES; i++ )
  {
    if( qti_netlink_conf->if_dev[i].if_index == ifindex )
    {
      link = i;
      break;
    }
  }

  return link;
}

/*===========================================================================

FUNCTION QTI_NL_DECODE_RTM_LINK()

DESCRIPTION

  This function
  - decodes the RTM link type netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qti_nl_decode_rtm_link
(
  const char              *buffer,
  unsigned int             buflen,
  qti_nl_link_info_t      *link_info
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;    /* NL message header */
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Extract the header data
-----------------------------------------------------------------------------*/
  link_info->metainfo = *(struct ifinfomsg*)NLMSG_DATA(nlh);
  LOG_MSG_INFO3("metainfo:  index = %d, family = %d, type = %d",
                link_info->metainfo.ifi_index,
                link_info->metainfo.ifi_family,
                link_info->metainfo.ifi_type);
  LOG_MSG_INFO3("metainfo: link up/down = %d",
                link_info->metainfo.ifi_change,0,0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_NL_DECODE_NLMSG()

DESCRIPTION

  This function
  - decodes the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qti_nl_decode_nlmsg
(
  const char     *buffer,
  unsigned int   buflen,
  qti_nl_msg_t   *msg_ptr
)
{

  struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;

/*-------------------------------------------------------------------------*/

  while(NLMSG_OK(nlh, buflen))
  {
/*-------------------------------------------------------------------------
  Decode the message type
-------------------------------------------------------------------------*/
    switch(nlh->nlmsg_type)
    {
      case RTM_NEWLINK:
      case RTM_DELLINK:
        msg_ptr->type = nlh->nlmsg_type;
        msg_ptr->link_event = TRUE;
/*-------------------------------------------------------------------------
  Decode the link message type
-------------------------------------------------------------------------*/
        if(QTI_SUCCESS != qti_nl_decode_rtm_link(buffer,
                                                 buflen,
                                                 &(msg_ptr->nl_link_info)))
        {
          LOG_MSG_ERROR("Failed to decode rtm link message",0,0,0);
          return QTI_FAILURE;
        }

        break;

      default:
        break;
    }
    nlh = NLMSG_NEXT(nlh, buflen);
  }
  return QTI_SUCCESS;
}

/*===========================================================================
FUNCTION QTI_GET_ALL_IF_INDEX()

DESCRIPTION

  This function
  - Gets the indices of the tethering interfaces.

DEPENDENCIES
  None.

RETURN VALUE
  None


SIDE EFFECTS
  None

=========================================================================*/
static void qti_get_all_if_index()
{
  int        i;
  int        ret_val;
  char       command[100];

/*----------------------------------------------------------------------------*/
  strlcpy(qti_netlink_conf->if_dev[0].dev_name, RNDIS_INTERFACE, IF_NAME_LEN);
  strlcpy(qti_netlink_conf->if_dev[1].dev_name, ECM_INTERFACE, IF_NAME_LEN);
  strlcpy(qti_netlink_conf->if_dev[2].dev_name, ODU_INTERFACE, IF_NAME_LEN);
  LOG_MSG_INFO1("Assigned device names %s,%s",
                qti_netlink_conf->if_dev[0].dev_name,
                qti_netlink_conf->if_dev[1].dev_name,
                qti_netlink_conf->if_dev[2].dev_name);

  for(i=0; i < QTI_INTERFACES; i++)
  {
/*----------------------------------------------------------------------------
  Initialize interface index to default
----------------------------------------------------------------------------*/
    qti_netlink_conf->if_dev[i].if_index = QTI_DEFAULT_INTERFACE_ID;

    ret_val = qti_get_if_index(qti_netlink_conf->if_dev[i].dev_name,
                               &(qti_netlink_conf->if_dev[i].if_index));
    if(ret_val!=QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Error while getting interface index for %d device",
                     i, 0, 0);
    }
  }
}


/*===========================================================================

FUNCTION RETRY_LINK_BRINGUP()

DESCRIPTION

  This function
  - enables QTI to support dynamic composition switch without rebooting
  - if we get netlink events on RNDIS/ECM interfaces and they are not
  enabled this function enables them and sends getlink message to
  re-trigger netlink events on those interfaces.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
void retry_link_bringup(int fd, int interface)
{
  struct msghdr   *nl_msg_hdr = NULL;
  nl_req_type     *nl_req = NULL;
/*------------------------------------------------------------------------*/

  LOG_MSG_INFO1("Re-enable RNDIS/ECM/ODUU interfaces", 0, 0, 0);
  if(interface == 0)
  {
    ds_system_call("ifconfig rndis0 up", strlen("ifconfig rndis0 up"));
  }
  else if(interface == 1)
  {
    ds_system_call("ifconfig ecm0 up", strlen("ifconfig ecm0 up"));
  }
  else if(interface == 2)
  {
    ds_system_call("ifconfig odu0 up", strlen("ifconfig odu0 up"));
  }

  nl_msg_hdr = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN );
  if(nl_msg_hdr == NULL)
  {
    LOG_MSG_ERROR("retry_link_bringup: Failed in qti_nl_alloc_msg",
                  0, 0, 0);
    return;
  }

  nl_req = (nl_req_type *)(nl_msg_hdr->msg_iov->iov_base);

  /*-----------------------------------------------------------------------
   Populate the required parameters in the netlink message
  -----------------------------------------------------------------------*/
  nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  nl_req->hdr.nlmsg_type = RTM_GETLINK ;
  /*----------------------------------------------------------------------
   NLM_F_REQUEST - has to be set for request messages
   NLM_F_DUMP -  equivalent to NLM_F_ROOT|NLM_F_MATCH
  ----------------------------------------------------------------------*/
  nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nl_req->hdr.nlmsg_seq = 1;
  nl_req->hdr.nlmsg_pid = getpid();
  nl_req->gen.rtgen_family = AF_PACKET;

  if(sendmsg(fd, (struct msghdr *)nl_msg_hdr, 0)<=0)
  {
    LOG_MSG_ERROR("GETLINK netlink message send failed", 0, 0, 0);
  }

  LOG_MSG_INFO1("GETLINK netlink message send succeeded", 0, 0, 0);

  free(nl_msg_hdr->msg_iov->iov_base);
  free(nl_msg_hdr->msg_iov);
  free(nl_msg_hdr->msg_name);
  free(nl_msg_hdr);
}
/*===========================================================================

FUNCTION QTI_NL_RECV_MSG()

DESCRIPTION

  This function
  - receives the netlink message associated with the netlink event which
  was received on select call
  - decodes the netlink message to find if the netlink message was found
  on the desired interface and if it was a link up or link down event
  - sends a command to command queue to process the link up or link down
    event

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qti_nl_recv_msg(int fd)
{
  struct msghdr                    * msghdr = NULL;
  struct sockaddr_nl               * nladdr = NULL;
  struct iovec                     * iov = NULL;
  unsigned int                     msglen = 0;
  qti_nl_msg_t                     * nlmsg = NULL;
  qti_cmdq_cmd_t                   * cmd_buf = NULL;
  int                              interface_num,interface;
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  Allocate memory to decode the netlink message
---------------------------------------------------------------------------*/
  nlmsg = malloc(sizeof(qti_nl_msg_t));
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
    if(QTI_SUCCESS != qti_nl_recv(fd, &msghdr, &msglen))
    {
      LOG_MSG_ERROR("Failed to receive nl message",0,0,0);
      goto error;
    }

    nladdr = msghdr->msg_name;
    iov = msghdr->msg_iov;

/*--------------------------------------------------------------------------
  Decode the received netlink message
---------------------------------------------------------------------------*/
    memset(nlmsg, 0, sizeof(qti_nl_msg_t));
    if(QTI_SUCCESS != qti_nl_decode_nlmsg((char*)iov->iov_base,
                                          msglen,
                                          nlmsg))
    {
      LOG_MSG_ERROR("Failed to decode nl message",0,0,0);
      goto error;
    }

/*--------------------------------------------------------------------------
  Update indices. They can change if the enumeration has happened again.
--------------------------------------------------------------------------*/
    qti_get_all_if_index();

/*--------------------------------------------------------------------------
  Get the interface on which we got the netlink event
---------------------------------------------------------------------------*/
    interface_num = qti_get_link_for_ifindex(nlmsg->nl_link_info.metainfo.ifi_index);

/*--------------------------------------------------------------------------
  We got the netlink event on RNDIS interface
---------------------------------------------------------------------------*/
    if(interface_num == 0)
    {
      interface = RNDIS_IF;
      LOG_MSG_INFO1("Netlink event got on RNDIS interface",0,0,0);

/*-------------------------------------------------------------------------
  Ignore netlink event on RNDIS since ECM/ODU is enabled
-------------------------------------------------------------------------*/
      if(qti_netlink_conf->if_dev[1].enabled || qti_netlink_conf->if_dev[2].enabled)
      {
        LOG_MSG_INFO1("Ignoring Netlink event on RNDIS since ECM/ODU is enabled",
                       0, 0, 0);
        qti_nl_release_msg(msghdr);
        free(nlmsg);
        return QTI_SUCCESS;
      }
    }
/*--------------------------------------------------------------------------
 We got the netlink event on ECM interface
---------------------------------------------------------------------------*/
    else if(interface_num ==1)
    {
      interface= ECM_IF;
      LOG_MSG_INFO1("Netlink event got on ECM interface",0,0,0);

/*-------------------------------------------------------------------------
  Ignore netlink event on ECM since RNDIS/ODU is enabled
-------------------------------------------------------------------------*/
      if(qti_netlink_conf->if_dev[0].enabled || qti_netlink_conf->if_dev[2].enabled)
      {
        LOG_MSG_INFO1("Ignoring Netlink event on ECM since RNDIS/ODU is enabled",
                       0, 0, 0);
        qti_nl_release_msg(msghdr);
        free(nlmsg);
        return QTI_SUCCESS;
      }
    }
/*--------------------------------------------------------------------------
 We got the netlink event on ODU interface
---------------------------------------------------------------------------*/
    else if(interface_num ==2)
    {
      interface= ODU_IF;
      LOG_MSG_INFO1("Netlink event got on ODU interface",0,0,0);

/*-------------------------------------------------------------------------
  Ignore netlink event on ODU since RNDIS/ECM is enabled
-------------------------------------------------------------------------*/
      if(qti_netlink_conf->if_dev[0].enabled || qti_netlink_conf->if_dev[1].enabled)
      {
        LOG_MSG_INFO1("Ignoring Netlink event on ODU since RNDIS/ECM is enabled",
                       0, 0, 0);
        qti_nl_release_msg(msghdr);
        free(nlmsg);
        return QTI_SUCCESS;
      }
    }
    else
    {
      qti_nl_release_msg(msghdr);
      free(nlmsg);
      return QTI_SUCCESS;
    }

    if(nlmsg->type == RTM_NEWLINK && !(nlmsg->nl_link_info.metainfo.ifi_flags & IFF_RUNNING))
    {
      retry_link_bringup(fd,interface_num);
    }
/*--------------------------------------------------------------------------
  Check if its a RTM link event
---------------------------------------------------------------------------*/
    if(nlmsg->link_event)
    {
      LOG_MSG_INFO1("flag: nl_link_info.metainfo.ifi_change %d IFF_UP %d",
                    nlmsg->nl_link_info.metainfo.ifi_change,
                    IFF_UP,
                    0);
/*--------------------------------------------------------------------------
  Check if the interface is running.If its a RTM_NEWLINK and the interface
  is running then it means that its a link up event
---------------------------------------------------------------------------*/
      if((nlmsg->nl_link_info.metainfo.ifi_flags & IFF_RUNNING) &&
         (nlmsg->nl_link_info.metainfo.ifi_flags & IFF_LOWER_UP) &&
         (nlmsg->type == RTM_NEWLINK))
      {
        LOG_MSG_INFO1("Got a new link event",0,0,0);

        if(qti_netlink_conf->if_dev[interface_num].enabled)
        {
          LOG_MSG_ERROR("Interface already enabled. Ignorning LINK_UP", 0, 0, 0);
        }
        else
        {
/*--------------------------------------------------------------------------
  Post a link up command
---------------------------------------------------------------------------*/
           cmd_buf = qti_cmdq_get_cmd();
           if(cmd_buf == NULL)
           {
             LOG_MSG_ERROR("qti_cmdq: failed to allocate memeory for cmd", 0, 0, 0);
             goto error;
           }
           cmd_buf->data.event = QTI_LINK_UP_EVENT;
           cmd_buf->data.interface = interface;
           if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
           {
             qti_cmdq_release_cmd(cmd_buf);
             LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
             goto error;
           }

           qti_netlink_conf->if_dev[interface_num].enabled = TRUE;
           LOG_MSG_INFO1("Sucessfully put cmd into command buffer",0,0,0);

/*--------------------------------------------------------------------------
 Required for KPI logs
---------------------------------------------------------------------------*/
          if(interface == ECM_IF)
          {
            ds_system_call("echo QTI:ECM mode > /dev/kmsg",
                           strlen("echo QTI:ECM mode > /dev/kmsg"));
          }
          else if (interface == RNDIS_IF)
          {
            ds_system_call("echo QTI:RNDIS mode > /dev/kmsg",
                           strlen("echo QTI:RNDIS mode > /dev/kmsg"));
          }
/*--------------------------------------------------------------------------
  Write to dmesg log. It will help in debugging customer issues quickly.
  But we need to make sure we dont write too many messages to dmesg.
---------------------------------------------------------------------------*/
           ds_system_call("echo QTI:LINK_UP message posted > /dev/kmsg",
                          strlen("echo QTI:LINK_UP message posted > /dev/kmsg"));
        }
      }
/*--------------------------------------------------------------------------
  If the interface is not connected and its a RTM_NEWLINK message then it
  means the link has gone down.
---------------------------------------------------------------------------*/
      else if((nlmsg->type == RTM_NEWLINK) &&
             !(nlmsg->nl_link_info.metainfo.ifi_flags & IFF_LOWER_UP))
      {
        LOG_MSG_INFO1("Got a del link event",0,0,0);
        if(!qti_netlink_conf->if_dev[interface_num].enabled)
        {
          LOG_MSG_ERROR("Interface not enabled. Ignorning LINK_DOWN", 0, 0, 0);
        }
        else
        {
/*--------------------------------------------------------------------------
  Post a link down command
---------------------------------------------------------------------------*/
          cmd_buf = qti_cmdq_get_cmd();
          if(cmd_buf == NULL)
          {
            LOG_MSG_ERROR("qti_cmdq: failed to allocate memeory for cmd", 0, 0, 0);
            goto error;
          }
          cmd_buf->data.event = QTI_LINK_DOWN_EVENT;
          cmd_buf->data.interface = interface;
          if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
          {
            qti_cmdq_release_cmd(cmd_buf);
            LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
            goto error;
          }

          qti_netlink_conf->if_dev[interface_num].enabled = FALSE;
          LOG_MSG_INFO1("Sucessfully put cmd into command buffer",0,0,0);
/*--------------------------------------------------------------------------
  Write to dmesg log. It will help in debugging customer issues quickly.
  But we need to make sure we dont write too many messages to dmesg.
---------------------------------------------------------------------------*/
          ds_system_call("echo QTI:LINK_DOWN message posted > /dev/kmsg",
                         strlen("echo QTI:LINK_DOWN message posted > /dev/kmsg"));
        }
      }
    }
    else
    {
      LOG_MSG_INFO1("Ignoring event %d", nlmsg->type,0,0);
    }
    qti_nl_release_msg(msghdr);
    free(nlmsg);
  }

  return QTI_SUCCESS;
error:
  if(msghdr)
  {
    qti_nl_release_msg(msghdr);
  }
  if(nlmsg)
  {
    free(nlmsg);
  }
  return QTI_FAILURE;
}

