/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/time.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_common.h"
#include "qmi_csi_target.h"
#include "qmi_fw_debug.h"
#include "msm_ipc.h"

#define MAX(a,b) (a > b ? a : b)
#define CTL_CMD_REMOVE_CLIENT       6
#define CTL_CMD_RESUME_TX 7
#define ALIGN_SIZE(x) ((4 - ((x) & 3)) & 3)
/* Tx queues for handling flow control:
 * +--------+  +--------+  +--------+
 * | q head |->| dest 1 |->| dest 2 |->...
 * +--------+  +--------+  +--------+
 *                 |            |
 *                buf 1        buf 1
 *                 |            |
 *                buf 2        buf 2
 *                 |            |
 *                ...          ...
 */
struct buf_s
{
  LINK(struct buf_s, link);
  void *msg;
  uint32_t len;
};

struct dest_s
{
  struct xport_ipc_router_client_addr *dest_addr;
  LINK(struct dest_s, link);
  LIST(struct buf_s, bufs);
  uint8_t dest_busy;
};

struct xport_ipc_router_client_addr {
        uint32_t node_id;
        uint32_t port_id;
};

union ctl_msg {
        uint32_t cmd;
        struct {
                uint32_t cmd;
                uint32_t service;
                uint32_t instance;
                uint32_t node_id;
                uint32_t port_id;
        } srv;
        struct {
                uint32_t cmd;
                uint32_t node_id;
                uint32_t port_id;
        } cli;
};

struct xport_handle
{
  qmi_csi_xport_type *xport;
  int fd;
  int ctl_fd;
  uint32_t max_rx_len;
  qmi_csi_lock_type tx_q_lock;
  LIST(struct dest_s, tx_q);
  qmi_csi_xport_options_type *xport_options;
};

union address_t {
  struct sockaddr sa;
  struct sockaddr_msm_ipc sa_msm_ipc;
};

/* List functions */
static struct dest_s *find_tx_q
(
struct xport_handle *xp,
struct xport_ipc_router_client_addr *addr
)
{
  struct dest_s *dest = LIST_HEAD(xp->tx_q);
  while(dest)
  {
    if(!memcmp(addr, dest->dest_addr, sizeof(struct xport_ipc_router_client_addr)))
      break;
    dest = dest->link.next;
  }
  return dest;
}

static struct dest_s *get_tx_q
(
struct xport_handle *xp,
struct xport_ipc_router_client_addr *addr
)
{
  struct dest_s *dest = find_tx_q(xp, addr);
  if(!dest)
  {
    dest = calloc(1, sizeof(struct dest_s));
    if(dest)
    {
      LINK_INIT(dest->link);
      LIST_INIT(dest->bufs);
      dest->dest_addr = addr;
      LIST_ADD(xp->tx_q, dest, link);
    }
  }
  return dest;
}

static void purge_tx_q
(
struct xport_handle *xp,
struct dest_s *dest
)
{
  struct buf_s *buf = LIST_HEAD(dest->bufs);
  LIST_REMOVE(xp->tx_q, dest, link);
  while(buf)
  {
    struct buf_s *to_free = buf;
    FREE(buf->msg);
    buf = buf->link.next;
    FREE(to_free);
  }
  FREE(dest);
}

static qmi_csi_error  put_tx_q
(
struct xport_handle *xp,
struct xport_ipc_router_client_addr *addr,
uint8_t *msg,
uint32_t msg_len,
uint32_t  max_q_len
)
{
  struct dest_s *dest;
  struct buf_s *buf;
  qmi_csi_error rc = QMI_CSI_NO_ERR;

  dest = get_tx_q(xp, addr);
  if(!dest)
  {
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  if(max_q_len > 0 && LIST_CNT(dest->bufs) >=  max_q_len)
  {
    dest->dest_busy =  1;
    rc = QMI_CSI_CONN_BUSY;
    goto bail_fail;
  }
  buf = calloc(1, sizeof(struct buf_s));

  if(!buf)
  {
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  LINK_INIT(buf->link);
  buf->len = msg_len;
  buf->msg = MALLOC(msg_len);
  if(!buf->msg)
  {
    FREE(buf);
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  memcpy(buf->msg, msg, msg_len);
  LIST_ADD(dest->bufs, buf, link);
  return QMI_CSI_NO_ERR;

bail_fail:
  return rc;
}

static void handle_resume_tx
(
void *handle,
struct xport_ipc_router_client_addr *addr
)
{
  union address_t dest_addr;
  struct dest_s *dest;
  struct buf_s *q_buf, *to_free;
  ssize_t sendto_rc;
  struct xport_handle *xp = (struct xport_handle *)handle;
  uint32_t  max_q_len = 0;
  int notify_resume_client = 0;

  if (!xp || !addr)
      return;

  if (QMI_CSI_SEND_FLAG_RATE_LIMITED && NULL != xp->xport_options)
  {
    max_q_len = xp->xport_options->rate_limited_queue_size;
  }

  LOCK(&xp->tx_q_lock);
  dest = find_tx_q(xp, addr);
  if (dest) {
    q_buf = LIST_HEAD(dest->bufs);
    dest_addr.sa_msm_ipc.family = AF_MSM_IPC;
    dest_addr.sa_msm_ipc.address.addrtype = MSM_IPC_ADDR_ID;
    dest_addr.sa_msm_ipc.address.addr.port_addr.node_id = dest->dest_addr->node_id;
    dest_addr.sa_msm_ipc.address.addr.port_addr.port_id = dest->dest_addr->port_id;
    while (q_buf)
    {
      sendto_rc = sendto(xp->fd, q_buf->msg, q_buf->len, MSG_DONTWAIT,
                         &dest_addr.sa, sizeof(dest_addr));
      if((sendto_rc < 0) && (errno == EAGAIN))
      {
        QMI_CSI_LOGD("%s Send Failed! for port %08x:%08x, Retry Later\n", __func__,
                    dest->dest_addr->node_id, dest->dest_addr->port_id);
        break;
      }
      else if(sendto_rc >= 0)
      {
        QMI_CSI_LOGD("%s Sent [%d]: %d queued bytes for port %08x:%08x\n", __func__,
                                       xp->fd, q_buf->len, dest->dest_addr->node_id,
                                       dest->dest_addr->port_id);
      }
      else
      {
        QMI_CSI_LOGD("%s Send Failed! for port %08x:%08x\n", __func__,
                    dest->dest_addr->node_id, dest->dest_addr->port_id);
      }
      to_free = q_buf;
      LIST_REMOVE(dest->bufs, q_buf, link);
      q_buf = q_buf->link.next;
      FREE(to_free->msg);
      FREE(to_free);
    }

    if (LIST_CNT(dest->bufs) < max_q_len && dest->dest_busy)
    {
      notify_resume_client = 1;
      dest->dest_busy = 0;
    }

    if(!(LIST_CNT(dest->bufs)))
    {
      LIST_REMOVE(xp->tx_q, dest, link);
      FREE(dest);
    }
  }
  UNLOCK(&xp->tx_q_lock);

  if (notify_resume_client)
    qmi_csi_xport_resume_client(xp->xport, addr);
}

static void *xport_open
(
 void *xport_data,
 qmi_csi_xport_type *xport,
 uint32_t max_rx_len,
 qmi_csi_os_params *os_params,
 qmi_csi_xport_options_type *options
)
{
  struct xport_handle *xp = calloc(1, sizeof(struct xport_handle));
  int align_size = 0, val;

  if (!xp)
  {
    QMI_FW_LOGE("%s: xp calloc failed\n", __func__);
    return NULL;
  }

  xp->xport = xport;
  xp->max_rx_len = (max_rx_len + QMI_HEADER_SIZE);
  align_size = ALIGN_SIZE(xp->max_rx_len);
  xp->max_rx_len += align_size;
  xp->xport_options = options;
  LOCK_INIT(&xp->tx_q_lock);
  LIST_INIT(xp->tx_q);

  xp->fd = socket(AF_MSM_IPC, SOCK_DGRAM, 0);
  if(xp->fd <= 0)
  {
    QMI_FW_LOGE("%s: socket creation failed - %d\n", __func__, errno);
    goto xport_open_free_xp;
  }

  if(fcntl(xp->fd, F_SETFD, FD_CLOEXEC) < 0)
  {
    QMI_FW_LOGE("%s: Close on exec can't be set on fd - %d\n",
                __func__, errno);
    goto xport_open_close_fd;
  }

  xp->ctl_fd = socket(AF_MSM_IPC, SOCK_DGRAM, 0);
  if(xp->ctl_fd <= 0)
  {
    QMI_FW_LOGE("%s: control socket creation failed - %d\n", __func__, errno);
    goto xport_open_close_fd;
  }

  if(fcntl(xp->ctl_fd, F_SETFD, FD_CLOEXEC) < 0)
  {
    QMI_FW_LOGE("%s: Close on exec can't be set on ctl_fd - %d\n",
                __func__, errno);
    goto xport_open_close_ctl_fd;
  }

  if(ioctl(xp->ctl_fd, IPC_ROUTER_IOCTL_BIND_CONTROL_PORT, NULL) < 0)
  {
    QMI_FW_LOGE("%s: failed to bind as control port\n", __func__);
    goto xport_open_close_ctl_fd;
  }

  /* set fd nonblocking */
  val = fcntl(xp->fd, F_GETFL, 0);
  fcntl(xp->fd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(xp->fd, &os_params->fds);
  os_params->max_fd = MAX(os_params->max_fd, xp->fd);

  val = fcntl(xp->ctl_fd, F_GETFL, 0);
  fcntl(xp->ctl_fd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(xp->ctl_fd, &os_params->fds);
  os_params->max_fd = MAX(os_params->max_fd, xp->ctl_fd);
  QMI_CSI_LOGD("xport_open[%d]: max_rx_len=%d\n", xp->fd, max_rx_len);
  return xp;

xport_open_close_ctl_fd:
  close(xp->ctl_fd);
xport_open_close_fd:
  close(xp->fd);
xport_open_free_xp:
  free(xp);
  return NULL;
}

static qmi_csi_error xport_reg
(
 void *handle,
 uint32_t service_id,
 uint32_t version
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  union address_t addr;

  bzero(&addr, sizeof(struct sockaddr_msm_ipc));
  addr.sa_msm_ipc.family = AF_MSM_IPC;
  addr.sa_msm_ipc.address.addrtype = MSM_IPC_ADDR_NAME;
  addr.sa_msm_ipc.address.addr.port_name.service = service_id;
  addr.sa_msm_ipc.address.addr.port_name.instance = version;
  if(bind(xp->fd, &addr.sa, sizeof(addr)) < 0)
  {
    QMI_FW_LOGE("%s Failed for service_id=0x%x version=0x%x on %d\n", __func__,
                                                   service_id, version, xp->fd);
    return QMI_CSI_INTERNAL_ERR;
  }
  QMI_CSI_LOGD("xport_reg[%d]: service_id=0x%x version=0x%x\n", xp->fd, service_id, version);
  return QMI_CSI_NO_ERR;
}

static qmi_csi_error xport_unreg
(
 void *handle,
 uint32_t service_id,
 uint32_t version
 )
{
  struct xport_handle *xp;
  xp = (struct xport_handle *)handle;
  QMI_CSI_LOGD("xport_unreg[%d]: type=0x%x version=0x%x\n", xp->fd, service_id, version);
  return QMI_CSI_NO_ERR;
}

static qmi_csi_error xport_send
(
 void *handle,
 void *addr,
 uint8_t *msg,
 uint32_t msg_len,
 uint32_t flags,
 void **client_data
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  union address_t dest_addr;
  struct xport_ipc_router_client_addr *s_addr = (struct xport_ipc_router_client_addr *)addr;
  struct dest_s *dest;
  qmi_csi_error rc;
  ssize_t sendto_rc;
  uint32_t max_q_len = 0;

  if (!s_addr)
  {
    QMI_FW_LOGE("%s: Invalid address parameter\n", __func__);
    return QMI_CSI_INTERNAL_ERR;
  }

  if (0 != (flags & QMI_CSI_SEND_FLAG_RATE_LIMITED) && NULL != xp->xport_options)
  {
    max_q_len = xp->xport_options->rate_limited_queue_size;
  }

  dest_addr.sa_msm_ipc.family = AF_MSM_IPC;
  dest_addr.sa_msm_ipc.address.addrtype = MSM_IPC_ADDR_ID;
  dest_addr.sa_msm_ipc.address.addr.port_addr.node_id = s_addr->node_id;
  dest_addr.sa_msm_ipc.address.addr.port_addr.port_id = s_addr->port_id;

  LOCK(&xp->tx_q_lock);
  dest = find_tx_q(xp, s_addr);

  if( dest && LIST_CNT(dest->bufs))
  {
    /* Queue the message so that it doesn't go out of order */
    rc = put_tx_q(xp, s_addr, msg, msg_len, max_q_len);
    if(rc == QMI_CSI_CONN_BUSY)
    {
      QMI_FW_LOGE("%s Queue exceeded, Retry sending for port %08x:%08x\n",
                               __func__, s_addr->node_id, s_addr->port_id);
    }
    else if(rc == QMI_CSI_NO_ERR)
    {
      QMI_CSI_LOGD("%s Packet queued for port %08x:%08x\n", __func__,
                                    s_addr->node_id, s_addr->port_id);
    }
    else
    {
      QMI_FW_LOGE("%s Error queuing packet for port %08x:%08x\n", __func__,
                                         s_addr->node_id, s_addr->port_id);
    }
    UNLOCK(&xp->tx_q_lock);
    return rc;
  }

  sendto_rc = sendto(xp->fd, msg, msg_len, MSG_DONTWAIT, &dest_addr.sa,
                     sizeof(dest_addr));
  if ((sendto_rc < 0) && (errno == EAGAIN))
  {
    /* queue to tx queue */
     rc = put_tx_q(xp, addr, msg, msg_len, max_q_len);
     if(rc == QMI_CSI_CONN_BUSY)
     {
       QMI_FW_LOGE("%s Queue exceeded, Retry sending for port %08x:%08x\n",
                                __func__, s_addr->node_id, s_addr->port_id);
     }
     else if(rc == QMI_CSI_NO_ERR)
     {
       QMI_CSI_LOGD("%s Packet queued for port %08x:%08x\n", __func__,
                                     s_addr->node_id, s_addr->port_id);
     }
     else
     {
       QMI_FW_LOGE("%s Error queuing packet for port %08x:%08x\n", __func__,
                                           s_addr->node_id, s_addr->port_id);
     }
  }
  else if (sendto_rc >= 0)
  {
    QMI_CSI_LOGD("Sent[%d]: %d bytes to port %08x:%08x\n", xp->fd, msg_len,
                 s_addr->node_id, s_addr->port_id);
    UNLOCK(&xp->tx_q_lock);
    return QMI_CSI_NO_ERR;
  }
  else /* Err on all other cases */
  {
    rc = QMI_CSI_INTERNAL_ERR;
    QMI_FW_LOGE("%s Sendto failed for port %08x:%08x\n", __func__, s_addr->node_id, s_addr->port_id);
  }
  UNLOCK(&xp->tx_q_lock);

  return rc;
}

static void xport_handle_event
(
 void *handle,
 qmi_csi_os_params *os_params
 )
{
  int rx_len;
  socklen_t src_addr_size = sizeof(union address_t);
  unsigned char *buf;
  union address_t src_addr;
  struct xport_ipc_router_client_addr addr;
  union ctl_msg rx_ctl_msg;
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct dest_s *dest;

  if(FD_ISSET(xp->fd, &os_params->fds))
  {
    buf = malloc(xp->max_rx_len);
    if(!buf)
    {
      QMI_FW_LOGE("%s: Unable to allocate memory for buf\n", __func__);
      return;
    }
    do {
      src_addr_size = sizeof(union address_t);
      rx_len = recvfrom(xp->fd, buf, xp->max_rx_len, MSG_DONTWAIT, &src_addr.sa, &src_addr_size);
      addr.node_id = src_addr.sa_msm_ipc.address.addr.port_addr.node_id;
      addr.port_id = src_addr.sa_msm_ipc.address.addr.port_addr.port_id;
      if ((src_addr_size == sizeof(struct sockaddr_msm_ipc)) && (rx_len == 0x0))
      {
        QMI_CSI_LOGD("%s: Received Resume_Tx from %08x:%08x on FD- %d\n",
                      __func__, addr.node_id, addr.port_id, xp->fd);
        handle_resume_tx(xp, &addr);
      }
      else if(rx_len > 0)
      {
        qmi_csi_xport_recv(xp->xport, &addr, buf, rx_len);
      }
      else
      {
        break;
      }
    } while(rx_len >= 0);
    QMI_CSI_LOGD("xport_handle_event[%d]\n", xp->fd);
    free(buf);
  }

  if(FD_ISSET(xp->ctl_fd, &os_params->fds))
  {
    while((rx_len = recvfrom(xp->ctl_fd, &rx_ctl_msg, sizeof(rx_ctl_msg), MSG_DONTWAIT, NULL, NULL)) > 0)
    {
      addr.node_id = rx_ctl_msg.cli.node_id;
      addr.port_id = rx_ctl_msg.cli.port_id;
      if (rx_ctl_msg.cmd == CTL_CMD_REMOVE_CLIENT) {
        QMI_CSI_LOGD("Received REMOVE_CLIENT cmd for %08x:%08x\n",
                      rx_ctl_msg.cli.node_id, rx_ctl_msg.cli.port_id);
        /* Purge the Tx queue */
        LOCK(&xp->tx_q_lock);
        dest = find_tx_q(xp, &addr);
        if (dest)
           purge_tx_q(xp, dest);
        UNLOCK(&xp->tx_q_lock);
        qmi_csi_xport_disconnect(xp->xport, &addr);
      }
    }
    QMI_CSI_LOGD("xport_handle_event[%d]\n", xp->ctl_fd);
  }
}

static void xport_close
(
 void *handle
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct dest_s *dest, *dest_to_free;

  /* Purge the Tx queue */
  LOCK(&xp->tx_q_lock);
  dest = LIST_HEAD(xp->tx_q);
  while (dest)
  {
    dest_to_free =  dest;
    dest = dest->link.next;
    purge_tx_q(xp, dest_to_free);
  }
  UNLOCK(&xp->tx_q_lock);

  close(xp->ctl_fd);
  QMI_CSI_LOGD("xport_close[%d]\n", xp->fd);
  close(xp->fd);
  qmi_csi_xport_closed(xp->xport);
  free(xp);
}

static uint32_t xport_addr_len
(
 void
 )
{
  return sizeof(struct xport_ipc_router_client_addr);
}

qmi_csi_xport_ops_type qcsi_ipc_router_ops = {
  NULL, /* Legacy Open */
  xport_reg,
  xport_unreg,
  NULL, /* Legacy Send */
  xport_handle_event,
  xport_close,
  xport_addr_len,
  xport_open,
  xport_send
};

