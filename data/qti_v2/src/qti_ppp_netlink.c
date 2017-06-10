/******************************************************************************

                        QTI_PPP_NETLINK.C

******************************************************************************/

/******************************************************************************

  @file    qti_ppp_netlink.c
  @brief   Qualcomm Tethering Interface PPP Netlink Messaging Implementation File

  DESCRIPTION
  Implementation file for QTI Netlink messaging functions related to PPP interface.

  ---------------------------------------------------------------------------
   Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
02/19/14   cp         Initial version

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
#include "qti_ppp.h"
/*===========================================================================

FUNCTION QTI_PPP_NL_OPEN_SOCKET()

DESCRIPTION

  This function
  - opens netlink sockets
  - binds the socket to listen to the required netlink events

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_ppp_nl_open_socket
(
  qti_ppp_nl_sk_info_t     *sk_info,
  int                  protocol,
  unsigned int         grps
)
{
  int                  * p_sk_fd;
  struct sockaddr_nl   * p_sk_addr_loc ;
/*-------------------------------------------------------------------------*/

  ds_assert(sk_info != NULL);

  LOG_MSG_INFO1("Entering qti_ppp_nl_open_socket", 0, 0, 0);

  p_sk_fd = &(sk_info->sk_fd);
  p_sk_addr_loc = &(sk_info->sk_addr_loc);

/*--------------------------------------------------------------------------
  Open netlink socket for specified protocol
---------------------------------------------------------------------------*/
  if ((*p_sk_fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0)
  {
    LOG_MSG_ERROR("Cannot open netlink socket",0,0,0);
    return QTI_PPP_FAILURE;
  }

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
    close(*p_sk_fd);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Socket open succeeds",0,0,0);

  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_PPP_NL_ADDFD_MAP()

DESCRIPTION

  This function
  - maps the socket descriptor with the corresponding callback function
  - add the socket descriptor to the set of socket desc the listener thread
    listens on.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_ppp_nl_addfd_map
(
  qti_ppp_nl_sk_fd_set_info_t      * fd_set,
  int                          fd,
  qti_ppp_sock_thrd_fd_read_f      read_f
)
{
  ds_assert(fd_set != NULL);
  LOG_MSG_INFO1("Entering qti_ppp_nl_addfd_map() fd= %d",fd,0,0);

  if( fd_set->num_fd < MAX_NUM_OF_FD )
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
    return QTI_PPP_FAILURE;
  }

  return QTI_PPP_SUCCESS;
}
/*===========================================================================

FUNCTION QTI_PPP_NL_LISTENER_INIT()

DESCRIPTION

  This function
  - initialises netlink sockets
  - sends an RTM_GETLINK which queries all kernel interfaces
    for netlink events
  - starts listening on netlink events

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qti_ppp_nl_listener_init
(
  unsigned int               nl_type,
  unsigned int               nl_groups,
  qti_ppp_nl_sk_fd_set_info_t    * sk_fdset,
  qti_ppp_sock_thrd_fd_read_f    read_f
)
{
  qti_ppp_nl_sk_info_t     sk_info;
  int                  ret_val;
/*--------------------------------------------------------------------------*/

  ds_assert(sk_fdset != NULL);

  LOG_MSG_INFO1("Initialize netlink socket",0,0,0);

  memset(&sk_info, 0, sizeof(qti_ppp_nl_sk_info_t));

/*---------------------------------------------------------------------------
  Open netlink sockets
----------------------------------------------------------------------------*/

  if( qti_ppp_nl_open_socket( &sk_info, nl_type, nl_groups ) == QTI_PPP_SUCCESS)
  {
    LOG_MSG_INFO1("Open netlink socket succeeds",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Netlink socket open failed",0,0,0);
    return QTI_PPP_FAILURE;
  }

/*--------------------------------------------------------------------------
  Add the NETLINK socket to the list of sockets that the listener
  thread should listen on.
---------------------------------------------------------------------------*/
  if( qti_ppp_nl_addfd_map(sk_fdset,sk_info.sk_fd,read_f ) != QTI_PPP_SUCCESS)
  {
    LOG_MSG_ERROR("cannot add nl routing sock for reading",0,0,0);
    close(sk_info.sk_fd);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("add fd map succeeds", 0, 0, 0);

  return QTI_PPP_SUCCESS;
}
/*===========================================================================

FUNCTION QTI_PPP_NL_ALLOC_MSG()

DESCRIPTION

  This function
  - allocates memory to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static struct msghdr * qti_ppp_nl_alloc_msg
(
  uint32_t         msglen
)
{
  unsigned char          *buf = NULL;
  struct sockaddr_nl     * nladdr = NULL;
  struct iovec           * iov = NULL;
  struct msghdr          * msgh = NULL;

/*-------------------------------------------------------------------------*/

  if(QTI_PPP_NL_MSG_MAX_LEN < msglen)
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

FUNCTION QTI_PPP_NL_RELEASE_MSG()

DESCRIPTION

  This function
  - releases memory which was alloacted to receive the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

============================================================================*/
static void qti_ppp_nl_release_msg
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

FUNCTION QTI_PPP_NL_DECODE_RTM_LINK()

DESCRIPTION

  This function
  - decodes the RTM link type netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qti_ppp_nl_decode_rtm_link
(
  const char              *buffer,
  unsigned int             buflen,
  qti_ppp_nl_link_info_t      *link_info
)
{
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;    /* NL message header */
/*---------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if ( buffer == NULL )
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

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

  return QTI_PPP_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_PPP_NL_DECODE_NLMSG()

DESCRIPTION

  This function
  - decodes the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

=============================================================================*/
static int qti_ppp_nl_decode_nlmsg
(
  const char     *buffer,
  unsigned int   buflen,
  qti_ppp_nl_msg_t   *msg_ptr
)
{

  struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;

  /* Check for NULL Args. */
  if ( buffer == NULL || msg_ptr == NULL )
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

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
        if(QTI_PPP_SUCCESS != qti_ppp_nl_decode_rtm_link(buffer,
                                                 buflen,
                                                 &(msg_ptr->nl_link_info)))
        {
          LOG_MSG_ERROR("Failed to decode rtm link message",0,0,0);
          return QTI_PPP_FAILURE;
        }

        break;

      default:
        break;
    }
    nlh = NLMSG_NEXT(nlh, buflen);
  }
  return QTI_PPP_SUCCESS;
}
/*===========================================================================

FUNCTION QTI_PPP_NL_RECV()

DESCRIPTION

  This function
  - retrieves the netlink message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_ppp_nl_recv
(
  int              fd,
  struct msghdr    ** msg_pptr,
  unsigned int     *  msglen_ptr
)
{
  struct msghdr    * msgh = NULL;
  int              rmsgl;
/*------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (msg_pptr == NULL || msglen_ptr == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

/*--------------------------------------------------------------------------
  Allocate the memory to receive the netlink message
---------------------------------------------------------------------------*/
  if( NULL == (msgh = qti_ppp_nl_alloc_msg( QTI_PPP_NL_MSG_MAX_LEN )) )
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

  return QTI_PPP_SUCCESS;

error:
/*--------------------------------------------------------------------------
  An error occurred while receiving the message. Free all memory before
  returning.
---------------------------------------------------------------------------*/

  qti_ppp_nl_release_msg( msgh );
  *msg_pptr    = NULL;
  *msglen_ptr  = 0;
  return QTI_PPP_FAILURE;
}
/*===========================================================================

FUNCTION QTI_PPP_NL_RECV_MSG()

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
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
int qti_ppp_nl_recv_msg(int fd)
{
  struct msghdr                    * msghdr = NULL;
  struct sockaddr_nl               * nladdr = NULL;
  struct iovec                     * iov = NULL;
  unsigned int                     msglen = 0;
  qti_ppp_nl_msg_t                     * nlmsg = NULL;
  int                              interface_num,interface;
  int                              ret_val = QTI_PPP_SUCCESS;
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  Allocate memory to decode the netlink message
---------------------------------------------------------------------------*/
  nlmsg = malloc(sizeof(qti_ppp_nl_msg_t));
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
    if(QTI_PPP_SUCCESS != qti_ppp_nl_recv(fd, &msghdr, &msglen))
    {
      LOG_MSG_ERROR("Failed to receive nl message",0,0,0);
      goto error;
    }

    nladdr = msghdr->msg_name;
    iov = msghdr->msg_iov;

/*--------------------------------------------------------------------------
  Decode the received netlink message
---------------------------------------------------------------------------*/
    memset(nlmsg, 0, sizeof(qti_ppp_nl_msg_t));
    if(QTI_PPP_SUCCESS != qti_ppp_nl_decode_nlmsg((char*)iov->iov_base,
                                          msglen,
                                          nlmsg))
    {
      LOG_MSG_ERROR("Failed to decode nl message",0,0,0);
      goto error;
    }

    LOG_MSG_INFO1("Netlink event got on unknow interface, check if PPP",0,0,0);
    if ((nlmsg->type == RTM_DELLINK) &&
        (nlmsg->nl_link_info.metainfo.ifi_flags & IFF_POINTOPOINT) &&
        !(nlmsg->nl_link_info.metainfo.ifi_flags & IFF_UP))
    {
      LOG_MSG_INFO1("Netlink event got on PPP interface, IF Down",0,0,0);

      if ( usb_tty_config_info.is_ppp_active )
      {
        /*--------------------------------------------------------------------------
        Bring down PPP tethering
        ---------------------------------------------------------------------------*/
        qti_ppp_usb_link_down();

        /*--------------------------------------------------------------------------
        Disable QC Mobile AP
        ---------------------------------------------------------------------------*/
        qti_ppp_disable_mobile_ap();
      }

      /* Update the PPP active flag. */
      usb_tty_config_info.is_ppp_active = 0;

      /*---------------------------------------------------------------------
        Call into the USB TTY listener init function which sets up QTI to
        listen to AT Commands coming in from the USB device file for DUN
      ---------------------------------------------------------------------*/
      ret_val = qti_usb_tty_listener_init(&usb_tty_config_info,
                                          &sk_fdset,
                                          qti_usb_tty_recv_msg);
      if (ret_val != QTI_PPP_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QTI USB TTY listener",0,0,0);
      }

    }

    qti_ppp_nl_release_msg(msghdr);
    free(nlmsg);

    return QTI_PPP_SUCCESS;
  }
  return QTI_PPP_SUCCESS;
error:
  if(msghdr)
  {
    qti_ppp_nl_release_msg(msghdr);
  }
  if(nlmsg)
  {
    free(nlmsg);
  }

  return QTI_PPP_FAILURE;
}

