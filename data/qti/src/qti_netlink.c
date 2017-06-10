/******************************************************************************

                        QTI_NETLINK.C

******************************************************************************/

/******************************************************************************

  @file    qti_netlink.c
  @brief   Qualcomm Tethering Interface Netlink Messaging Implementation File

  DESCRIPTION
  Implementation file for NetMgr Netlink messaging functions.
 
  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 
 
******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/28/14   rk         KW fixes.
04/23/13   sb/mp      Fix to prevent race condition during call bring up.
03/29/13   sb         QTI boot up optimizations for RNDIS.
02/04/12   sb/mp      Added support for dynamic USB composition switching.
11/23/12   mp         Added check to bring up call only if the tethering
                      interface is physically connected.
11/06/12   mp         Bringing up tethering interface after a while to ensure
                      that PC gets IP properly.
10/15/12   mp         Initialize interfaces index to a non-zero default value.
10/08/12   mp         Fix to recognize the tethering interface upon power on.
06/29/12   sc         Revised version
05/24/12   sb         Initial version

******************************************************************************/

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

#include "qti_cmdq.h"
#include "ds_Utils_DebugMsg.h"
#include "AEEstd.h"


/*---------------------------------------------------------------------------
   QTI configuration file
---------------------------------------------------------------------------*/
static qti_conf_t qti_conf;

typedef struct
{
  struct nlmsghdr hdr;
  struct rtgenmsg gen;
}nl_req_type;

/*===========================================================================
  FUNCTION  qti_nl_open_socket
===========================================================================*/
/*!
@brief
  Opens a netlink socket for the specified protocol and multicast group
  memberships.

@return
  QTI_SUCCESS or QTI_FAILURE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_open_socket
(
  qti_nl_sk_info_t *sk_info,
  int protocol,
  unsigned int grps
)
{
  int * p_sk_fd;
  struct sockaddr_nl * p_sk_addr_loc ;

  LOG_MSG_INFO1("Entering QTI NL open socket",0,0,0);
  p_sk_fd = &(sk_info->sk_fd);
  p_sk_addr_loc = &(sk_info->sk_addr_loc);

  /* Open netlink socket for specified protocol */
  if ((*p_sk_fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0) 
  {
    LOG_MSG_ERROR("cannot open netlink socket",0,0,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("socket open succeeds",0,0,0);

  /* Initialize socket addresses to null */
  memset(p_sk_addr_loc, 0, sizeof(struct sockaddr_nl));
  LOG_MSG_INFO1("initialize socket address succeds no seg fault",0,0,0);

  /* Populate local socket address using specified groups */
  p_sk_addr_loc->nl_family = AF_NETLINK;
  p_sk_addr_loc->nl_pid = getpid();
  p_sk_addr_loc->nl_groups = grps;

  /* Bind socket to the local address, i.e. specified groups. This ensures
   that multicast messages for these groups are delivered over this 
   socket. */

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
  FUNCTION  qti_nl_addfd_map
===========================================================================*/
/*!
@brief
  Add fd to fdmap array and store read handler function ptr
  (up to MAX_NUM_OF_FD).

@return
  QTI_SUCCESS or QTI_FAILURE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_addfd_map
(
  qti_nl_sk_fd_set_info_t * fd_set,
  int fd,
  qti_sock_thrd_fd_read_f read_f
)
{
  LOG_MSG_INFO1("Entering FD map function= %d",fd,0,0);

  if( fd_set->num_fd < MAX_NUM_OF_FD )
  {
    FD_SET(fd, &(fd_set->fdset));
    /* Add fd to fdmap array and store read handler function ptr */
    fd_set->sk_fds[fd_set->num_fd].sk_fd = fd;
    fd_set->sk_fds[fd_set->num_fd].read_func = read_f;
	
	/* Increment number of fds stored in fdmap */
    fd_set->num_fd++;
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
  FUNCTION  qti_nl_sock_listener_start
===========================================================================*/
/*!
@brief
  start socket listener.

@return
  QTI_SUCCESS or QTI_FAILURE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_sock_listener_start
(
  qti_nl_sk_fd_set_info_t * sk_fd_set
)
{
  int i,ret;  
  LOG_MSG_INFO1("Starting NL listener",0,0,0);

  qti_conf.state = QTI_LINK_UP_WAIT;

  while(TRUE)
  {
    LOG_MSG_INFO1("waiting on select",0,0,0);
    LOG_MSG_INFO1("QTI state %d", qti_conf.state,0,0);
    if((ret = select(sk_fd_set->max_fd+1, &(sk_fd_set->fdset), NULL, NULL, NULL)) < 0) 
    {
      LOG_MSG_ERROR("qti_nl select failed",0,0,0);
    }
    else
    {
      for(i = 0; i < sk_fd_set->num_fd; i++ )
      {
        if( FD_ISSET(sk_fd_set->sk_fds[i].sk_fd, &(sk_fd_set->fdset) ) )
        {
          if(sk_fd_set->sk_fds[i].read_func)
          {
            if( QTI_SUCCESS != ((sk_fd_set->sk_fds[i].read_func)(sk_fd_set->sk_fds[i].sk_fd)) )
            {
              LOG_MSG_ERROR("Error on read callback[%d] fd=%d", i, sk_fd_set->sk_fds[i].sk_fd,0);
            }
          }
          else
          {
            LOG_MSG_ERROR("No read function",0,0,0);
          }
        }
        else
        {
          LOG_MSG_INFO1("did not find the matching interface in FD_ISSET",0,0,0);
        }
      }
    }
  }
  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_nl_alloc_msg
===========================================================================*/
/*!
@brief
  allocate memory for qti_nl__msg.

@return
   struct msghdr

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static struct msghdr * qti_nl_alloc_msg
(
  uint32 msglen
)
{
  unsigned char *buf = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;
  struct msghdr * msgh = NULL;

  LOG_MSG_INFO1("Entering alloc recv msg",0,0,0);

  if(QTI_NL_MSG_MAX_LEN < msglen)
  {
    LOG_MSG_ERROR("Netlink message exceeds maximum length",0,0,0);
    return NULL;
  }

  if((msgh = malloc(sizeof(struct msghdr))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for msghdr",0,0,0);
    free(msgh);
    return NULL;
  }

  if((nladdr = malloc(sizeof(struct sockaddr_nl))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for sockaddr",0,0,0);
    free(nladdr);
    free(msgh);
    return NULL;
  }
    
  if((iov = malloc(sizeof(struct iovec))) == NULL)
  {
    LOG_MSG_ERROR("Failed malloc for iovec",0,0,0);
    free(iov);
    free(nladdr);
    free(msgh);
    return NULL;
  }

  if((buf = malloc(msglen))== NULL)
  {
    LOG_MSG_ERROR("Failed malloc for mglen",0,0,0);
    free(buf);
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

  LOG_MSG_INFO1("alloc mess succeeds",0,0,0);
  return msgh;
}

/*===========================================================================
  FUNCTION  qti_nl_release_msg
===========================================================================*/
/*!
@brief
  release QTI message.

@return
  None

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
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
  FUNCTION  qti_nl_recv
===========================================================================*/
/*!
@brief
  receive and process nl message

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_recv
(
  int              fd,
  struct msghdr ** msg_pptr,
  unsigned int  *  msglen_ptr
)
{
  struct msghdr * msgh = NULL;
  int rmsgl;

  LOG_MSG_INFO1("Entering NL recv",0,0,0);
  if( NULL == (msgh = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN )) ) 
  {
    LOG_MSG_ERROR("Failed to allocate NL message",0,0,0);
    goto error;
  }

  LOG_MSG_INFO1("succeeds nl mess allocation",0,0,0);
  /* Receive message over the socket */
  rmsgl = recvmsg(fd, msgh, 0);
  LOG_MSG_INFO1("result of recv msg is %d",rmsgl,0,0);

  /* Verify that something was read */
  if (rmsgl <= 0) 
  {
    LOG_MSG_ERROR("Received nl_msg, recvmsg failed:",0,0,0);
    perror("NL recv error");
    goto error;
  }

  /* Verify that NL address length in the received message is expected value */
  if (msgh->msg_namelen != sizeof(struct sockaddr_nl)) 
  {
    LOG_MSG_ERROR("rcvd msg with namelen != sizeof sockaddr_nl",0,0,0);
    goto error;
  }

  /* Verify that message was not truncated. This should not occur */
  if (msgh->msg_flags & MSG_TRUNC) 
  {
    LOG_MSG_ERROR("Rcvd msg truncated!",0,0,0);
    goto error;
  }

  LOG_MSG_INFO1("Received nl msg, recvmsg returned %d", rmsgl,0,0);
  *msg_pptr    = msgh;
  *msglen_ptr = rmsgl; 
  
  /* Return message ptr. Caller is responsible for freeing the memory */
  return QTI_SUCCESS;

error:
  /* An error occurred while receiving the message. Free all memory before 
  ** returning. 
  */
  qti_nl_release_msg( msgh );
  *msg_pptr    = NULL;
  *msglen_ptr  = 0; 
  return QTI_FAILURE;
}

/*===========================================================================
  FUNCTION  qti_get_link_for_ifindex
===========================================================================*/
/*!
@brief
  Iterate over the array of interfaces, and return the link id for
  the interface matching the specified if index.

@return
  index of matched interface

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_get_link_for_ifindex (int ifindex)
{
  int link = -1;
  int i;

  for( i = 0; i < QTI_INTERFACES; i++ ) 
  {
    if( qti_conf.if_dev[i].if_index == ifindex ) 
    {
      link = i;
      break;
    }
  }

  return link;
}

/*===========================================================================
  FUNCTION  qti_nl_decode_rtm_link
===========================================================================*/
/*!
@brief
  decode the rtm netlink message

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_decode_rtm_link
(
  const char              *buffer,
  unsigned int             buflen,
  qti_nl_link_info_t      *link_info
)
{
  LOG_MSG_INFO1("entered nl decode rtm link", 0,0,0);

  /* NL message header */
  struct nlmsghdr * nlh = (struct nlmsghdr*)buffer;  

  /* Extract the header data */
  link_info->metainfo = *(struct ifinfomsg*)NLMSG_DATA(nlh);
  LOG_MSG_INFO1("metainfo:  index = %d, family = %d, type = %d",
				link_info->metainfo.ifi_index, 
				link_info->metainfo.ifi_family,
                link_info->metainfo.ifi_type);
  LOG_MSG_INFO1("metainfo: link up/down = %d", link_info->metainfo.ifi_change,0,0);
  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_nl_decode_nlmsg
===========================================================================*/
/*!
@brief
  decode the qti nl-message

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_nl_decode_nlmsg
(
  const char   *buffer,
  unsigned int  buflen,
  qti_nl_msg_t  *msg_ptr
)
{
  LOG_MSG_INFO1("Entering NL decode message",0,0,0);
  struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;

  while(NLMSG_OK(nlh, buflen))
  {
    LOG_MSG_INFO1("got nl msg of type %d",nlh->nlmsg_type,0,0);
    switch(nlh->nlmsg_type)
    {
      case RTM_NEWLINK:
        LOG_MSG_INFO1("got new link",0,0,0);
      case RTM_DELLINK:
        LOG_MSG_INFO1("got new/del link",0,0,0);
        msg_ptr->type = nlh->nlmsg_type;
        msg_ptr->link_event = TRUE;
        LOG_MSG_INFO1("entering rtm decode",0,0,0);
        if(QTI_SUCCESS != qti_nl_decode_rtm_link(buffer, buflen, &(msg_ptr->nl_link_info)))
        {
          LOG_MSG_ERROR("Failed to decode rtm link message",0,0,0);
          return QTI_FAILURE;
        }
        break;
      default:
        LOG("Ignoring event %d", nlh->nlmsg_type,0,0);
    }
    nlh = NLMSG_NEXT(nlh, buflen);
  }
  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_get_if_index
===========================================================================*/
/*!
@brief
  Get each QTI interface index

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int qti_get_if_index
(
  char * if_name,
  int * if_index
)
{
  int fd;
  struct ifreq ifr;

  LOG_MSG_INFO1("Entering get interface index",0,0,0);

  if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("get interface index socket create failed",0,0,0);
    return QTI_FAILURE;
  }

  memset(&ifr, 0, sizeof(struct ifreq));

  (void)std_strlcpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
  LOG_MSG_INFO1("interface name %s", ifr.ifr_name,0,0);

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
  FUNCTION  qti_get_all_if_index
===========================================================================*/
/*!
@brief
 Gets the indices of the tethering interfaces

@return
  TO DO

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void qti_get_all_if_index()
{
  int i;
  int ret_val;
  char command[100];
  std_strlcpy(qti_conf.if_dev[0].dev_name, RNDIS_INTERFACE, IF_NAME_LEN);
  std_strlcpy(qti_conf.if_dev[1].dev_name, ECM_INTERFACE, IF_NAME_LEN);
  LOG_MSG_INFO1("Assigned device names %s,%s",qti_conf.if_dev[0].dev_name,qti_conf.if_dev[1].dev_name,0);

  /*Initialize interface index to default*/
  for(i=0; i < QTI_INTERFACES; i++)
  {
    qti_conf.if_dev[i].if_index = QTI_DEFAULT_INTERFACE_ID;

    ret_val = qti_get_if_index(qti_conf.if_dev[i].dev_name, &(qti_conf.if_dev[i].if_index));
    if(ret_val!=QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Error while getting interface index for %d device", i,0,0);
    }
  }
}

/*===========================================================================
  FUNCTION  retry_link_bringup
===========================================================================*/
/*!
@brief
 Retries bringing up the link by giving ifconfig up command
 followed by sending an RTM_GETLINK message.

@return
  TO DO

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void retry_link_bringup(int fd, int interface)
{
  struct msghdr *nl_msg_hdr = NULL;
  nl_req_type *nl_req = NULL;

  if(interface == 0)
  {
    ds_system_call("ifconfig rndis0 up", strlen("ifconfig rndis0 up"));
  }
  else if(interface == 1)
  {
    ds_system_call("ifconfig ecm0 up", strlen("ifconfig ecm0 up"));
  }

  nl_msg_hdr = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN );
  if(nl_msg_hdr == NULL)
  {
    LOG_MSG_ERROR("retry_link_bringup: Failed in qti_nl_alloc_msg",0,0,0);
    return;
  }

  nl_req = (nl_req_type *)(nl_msg_hdr->msg_iov->iov_base);

  /* Populate the required parameters in the netlink message */
  nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  nl_req->hdr.nlmsg_type = RTM_GETLINK ;
  /* NLM_F_REQUEST - has to be set for request messages
     NLM_F_DUMP -  equivalent to NLM_F_ROOT|NLM_F_MATCH */
  nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nl_req->hdr.nlmsg_seq = 1;
  nl_req->hdr.nlmsg_pid = getpid();
  nl_req->gen.rtgen_family = AF_PACKET;

  sendmsg(fd, (struct msghdr *) &nl_msg_hdr, 0);

  free(nl_msg_hdr->msg_iov->iov_base);
  free(nl_msg_hdr->msg_iov);
  free(nl_msg_hdr->msg_name);
  free(nl_msg_hdr);
}
/*===========================================================================
  FUNCTION  qti_nl_recv_msg
===========================================================================*/
/*!
@brief
  Virtual function registered with the socket listener thread to receive
  incoming messages over the NETLINK routing socket.

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_nl_recv_msg(int fd)
{
  struct msghdr * msghdr = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;
  unsigned int msglen = 0;
  qti_nl_msg_t * nlmsg = NULL;

  qti_cmdq_cmd_t * cmd_buf = NULL;

  int interface_num,interface;
  LOG_MSG_INFO1("Entered nl rev msg function to process link event on fd = %d",fd,0,0);

  nlmsg = malloc(sizeof(qti_nl_msg_t));
  if(NULL == nlmsg) 
  {
    LOG_MSG_ERROR("Failed alloc of nlmsg",0,0,0);
    goto error;
  }
  else
  {
    if(QTI_SUCCESS != qti_nl_recv(fd, &msghdr, &msglen))
    {
      LOG_MSG_ERROR("Failed to receive nl message",0,0,0);
      goto error;
    }

    nladdr = msghdr->msg_name;
    iov = msghdr->msg_iov;

    memset(nlmsg, 0, sizeof(qti_nl_msg_t));
    if(QTI_SUCCESS != qti_nl_decode_nlmsg((char*)iov->iov_base, msglen, nlmsg))
    {
      LOG_MSG_ERROR("Failed to decode nl message",0,0,0);
      goto error;
    }

    qti_get_all_if_index();
    interface_num =
             qti_get_link_for_ifindex(nlmsg->nl_link_info.metainfo.ifi_index);

    if(interface_num == 0)
    {
      interface = RNDIS_IF;
      LOG_MSG_INFO1("interface is rndis",0,0,0);
      if(qti_conf.if_dev[1].enabled)
      {
        qti_nl_release_msg(msghdr);
        free(nlmsg);
        return QTI_SUCCESS;
      }
    }
    else if(interface_num == 1)
    {
      LOG_MSG_INFO1("interface is ecm",0,0,0);
      interface= ECM_IF;
      if(qti_conf.if_dev[0].enabled)
      {
        qti_nl_release_msg(msghdr);
        free(nlmsg);
        return QTI_SUCCESS;
      }
    }
    else
    {
      LOG_MSG_INFO1("Ignored RTM_NEWLINK on a different interface",0,0,0);
      qti_nl_release_msg(msghdr);
      free(nlmsg);
      return QTI_SUCCESS;
    }
	
    if(nlmsg->type == RTM_NEWLINK && !(nlmsg->nl_link_info.metainfo.ifi_flags & IFF_RUNNING))
    {
      retry_link_bringup(fd,interface_num);
    }

    if(nlmsg->link_event)
    {
      LOG_MSG_INFO1("flag: nl_link_info.metainfo.ifi_change %d IFF_UP %d",
                    nlmsg->nl_link_info.metainfo.ifi_change,IFF_UP,0);

      if((nlmsg->nl_link_info.metainfo.ifi_flags & IFF_RUNNING) &&
         (nlmsg->nl_link_info.metainfo.ifi_flags & IFF_LOWER_UP) &&
         (nlmsg->type == RTM_NEWLINK))
      {
        if(qti_conf.if_dev[interface_num].enabled)
        {
          LOG_MSG_ERROR("Got link up event in LINK_DOWN_WAIT or"
                        " LINK_UP state. Ignorning", 0, 0, 0);
          goto error;
        }
        else
        {
          /*--------------------------------------------------------------------------
           Post a link up command
          ---------------------------------------------------------------------------*/
          cmd_buf = qti_cmdq_get_cmd();
          if(cmd_buf == NULL)
          {
            LOG_MSG_ERROR("cmd_buf is set to NULL",0,0,0);
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

          qti_conf.state = QTI_LINK_UP;
          qti_conf.if_dev[interface_num].enabled = TRUE;
          LOG_MSG_INFO1("Sucessfully put cmd into command buffer",0,0,0);
          ds_system_call("echo QTI:LINK_UP message posted > /dev/kmsg",
                        strlen("echo QTI:LINK_UP message posted > /dev/kmsg"));
        }
      }
      else if (!(nlmsg->nl_link_info.metainfo.ifi_flags & IFF_LOWER_UP) &&
              (nlmsg->type == RTM_NEWLINK))
      {
        if(!qti_conf.if_dev[interface_num].enabled)
        {
          LOG_MSG_ERROR("Got link down event in LINK_UP_WAIT or"
                        " LINK_DOWN state. Ignorning", 0, 0, 0);
          goto error;
        }
        else
        {
          /*----------------------------------------------------------
          Post a link down command
          ---------------------------------------------------------- */ 
          cmd_buf = qti_cmdq_get_cmd();
          if(cmd_buf == NULL)
          {
            LOG_MSG_ERROR("cmd_buf is set to NULL",0,0,0);
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

          qti_conf.state = QTI_LINK_DOWN;
          qti_conf.if_dev[interface_num].enabled = FALSE;
          LOG_MSG_INFO1("Sucessfully put cmd into command buffer",0,0,0);
          ds_system_call("echo QTI:LINK_DOWN message posted > /dev/kmsg",
                      strlen("echo QTI:LINK_DOWN message posted > /dev/kmsg"));
        }
      }//rtm_newlink
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

/*===========================================================================
  FUNCTION  qti_init 
===========================================================================*/
/*!
@brief
 Initialize qti setup

@return
  TO DO

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_init(void)
{
  int i;
  int ret_val;

  LOG_MSG_INFO1("Entering QTI init",0,0,0);

  qti_get_all_if_index();

  //qti_wda_init(qti_conf);
  ret_val = qti_qmi_init(&qti_conf);
  if(ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("QTI QMI init failed",0,0,0);
  }
  
  return ret_val;
}

/*===========================================================================
  FUNCTION  qti_nl_listener_init 
===========================================================================*/
/*!
@brief
  Initialization routine for NetLink socket interface message listener. 

@return
  QTI_SUCCESS/QTI_FAILURE

@note

  - Dependencies
    - None 

  - Side Effects
    - Listening thread is created
*/
/*=========================================================================*/
int qti_nl_listener_init
(
  unsigned int nl_type,
  unsigned int nl_groups,
  qti_nl_sk_fd_set_info_t * sk_fdset,
  qti_sock_thrd_fd_read_f read_f
)
{
  qti_nl_sk_info_t sk_info;
  int ret_val;

  LOG_MSG_INFO1("Entering QTI NL listener init",0,0,0);
  memset(&sk_info, 0, sizeof(qti_nl_sk_info_t));

  if( qti_nl_open_socket( &sk_info, nl_type, nl_groups ) == QTI_SUCCESS)
  {
    LOG_MSG_INFO1("Open netlink socket succeeds",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Netlink socket open failed",0,0,0);
    return QTI_FAILURE;
  }

  /* Add the NETLINK socket to the list of sockets that the listener 
  ** thread should listen on. 
  */
  if( qti_nl_addfd_map(sk_fdset,sk_info.sk_fd,read_f ) != QTI_SUCCESS) 
  {
    LOG_MSG_ERROR("cannot add nl routing sock for reading",0,0,0);
    return QTI_FAILURE;
  }
  
  LOG_MSG_INFO1("add fd map succeeds",0,0,0);
  /* Start the socket listener thread */

  /* Query the kernel about the current links by sending RTM_GETLINK.
     This is useful to get RTM_NEWLINK for the interface upon power on
     even without plugging out the USB cable */
  ret_val = qti_nl_query_if(&sk_info);
  if(ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed sending RTM_GETLINK to kernel",0,0,0);
  }
  
  ret_val = qti_nl_sock_listener_start(sk_fdset);

  if(ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to start NL listener",0,0,0);
  }
    return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_conf_state_init 
===========================================================================*/
/*!
@brief
  Initialize qti state

@return
  Void

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_conf_state_init(void)
{
  qti_cmdq_cmd_t * cmd_buf = NULL;
  int              interface;

  qti_conf.qti_qcmap_proceed = 1;

  /*------------------------------------------------------------------------
  Fake a LINK UP event once QCMAP signals QTI about LAN connection
  ------------------------------------------------------------------------*/
  if(qti_conf.state == QTI_LINK_UP)
  {
    if(qti_conf.if_dev[RNDIS_IF].enabled)
    {
      interface = RNDIS_IF;
    }
    else if(qti_conf.if_dev[ECM_IF].enabled)
    {
      interface = ECM_IF;
    }

    cmd_buf = qti_cmdq_get_cmd();
    if(cmd_buf == NULL)
    {
      LOG_MSG_ERROR("cmd_buf is set to NULL",0,0,0);
      return QTI_FAILURE;
    }
    cmd_buf->data.event = QTI_LINK_UP_EVENT;
    cmd_buf->data.interface = interface;
    if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
    {
      qti_cmdq_release_cmd(cmd_buf);
      LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
    }
  }

  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_nl_query_if 
===========================================================================*/
/*!
@brief
  Send an RTM_GETLINK to kernel to get the current link configuration.

@return
  QTI_FAILURE or QTI_SUCCESS

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_nl_query_if
(
qti_nl_sk_info_t *sk_info
)
{
  struct msghdr *nl_msg_hdr = NULL;
  nl_req_type *nl_req = NULL;

  LOG_MSG_INFO1("qti_nl_query_if(): Sending RTM_GETLINK to kernel",0,0,0);

  LOG_MSG_INFO1("ifconfig rndis0 up",0,0,0);
  ds_system_call("ifconfig rndis0 up", strlen("ifconfig rndis0 up"));
  LOG_MSG_INFO1("ifconfig ecm0 up",0,0,0);
  ds_system_call("ifconfig ecm0 up", strlen("ifconfig ecm0 up"));

  nl_msg_hdr = qti_nl_alloc_msg( QTI_NL_MSG_MAX_LEN );
  if(nl_msg_hdr == NULL)
  {
    LOG_MSG_ERROR("qti_nl_query_if: Failed in qti_nl_alloc_msg",0,0,0);
    return QTI_FAILURE;
  }

  nl_req = (nl_req_type *)(nl_msg_hdr->msg_iov->iov_base);

  /* Populate the required parameters in the netlink message */
  nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  nl_req->hdr.nlmsg_type = RTM_GETLINK ;
  /* NLM_F_REQUEST - has to be set for request messages
     NLM_F_DUMP -  equivalent to NLM_F_ROOT|NLM_F_MATCH */
  nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nl_req->hdr.nlmsg_seq = 1;
  nl_req->hdr.nlmsg_pid = sk_info->sk_addr_loc.nl_pid;
  nl_req->gen.rtgen_family = AF_PACKET;

  sendmsg(sk_info->sk_fd, (struct msghdr *) &nl_msg_hdr, 0); 

  free(nl_msg_hdr->msg_iov->iov_base);
  free(nl_msg_hdr->msg_iov);
  free(nl_msg_hdr->msg_name);
  free(nl_msg_hdr);

  return QTI_SUCCESS;
}
