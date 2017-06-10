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
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "qmi_fw_debug.h"
#include "msm_ipc.h"

#define CTL_CMD_NEW_SERVER          4
#define CTL_CMD_REMOVE_SERVER       5

#define ALIGN_SIZE(x) ((4 - ((x) & 3)) & 3)
/*
 * QCCI treats instance and version fields separately and uses IDL version
 * as the instance during service lookup. IPC Router passes the instance
 * (MS 24 bits) + IDL Version(LS 8 bits) fields together as the instance info.
 *
 * Macro to translate between IPC Router specific instance information
 * to QCCI specific instance information.
 */
#define GET_XPORT_SVC_INSTANCE(x) GET_VERSION(x)

struct xport_ipc_router_server_addr {
        uint32_t service;
        uint32_t instance;
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
  qmi_cci_client_type *clnt;
  int fd;
  int ctl_fd;
  pthread_t reader_tid;
  int wakeup_pipe[2];
  uint32_t max_rx_len;
  struct msm_ipc_port_name srv_name;
};

static pthread_mutex_t lookup_fd_lock = PTHREAD_MUTEX_INITIALIZER;
static int lookup_sock_fd = -1;

int open_lookup_sock_fd(void)
{
  pthread_mutex_lock(&lookup_fd_lock);
  if (lookup_sock_fd < 0)
  {
    lookup_sock_fd = socket(AF_MSM_IPC, SOCK_DGRAM, 0);
    if (lookup_sock_fd < 0)
    {
      pthread_mutex_unlock(&lookup_fd_lock);
      QMI_FW_LOGE("%s: Lookup sock fd creation failed\n", __func__);
      return -1;
    }
    fcntl(lookup_sock_fd, F_SETFD, FD_CLOEXEC);
  }
  pthread_mutex_unlock(&lookup_fd_lock);
  return 0;
}

void close_lookup_sock_fd(void)
{
  pthread_mutex_lock(&lookup_fd_lock);
  close(lookup_sock_fd);
  lookup_sock_fd = -1;
  pthread_mutex_unlock(&lookup_fd_lock);
}

static void *reader_thread(void *arg)
{
  struct xport_handle *xp = (struct xport_handle *)arg;
  struct sockaddr_msm_ipc addr;
  struct xport_ipc_router_server_addr src_addr;
  unsigned char *buf, ch;
  union ctl_msg rx_ctl_msg;
  int rx_len, i;
  socklen_t addr_size = sizeof(struct sockaddr_msm_ipc);
  struct pollfd pbits[3];

  pbits[0].fd = xp->wakeup_pipe[0];
  pbits[0].events = POLLIN;
  pbits[1].fd = xp->ctl_fd;
  pbits[1].events = POLLIN;
  pbits[2].fd = xp->fd;
  pbits[2].events = POLLIN;
  while(1)
  {
    i = poll(pbits, 3, -1);
    if(i < 0)
    {
      QMI_FW_LOGE("%s: poll error (%d)\n", __func__, errno);
      continue;
    }

    if((pbits[2].revents & POLLIN))
    {
      buf = (unsigned char *)calloc(xp->max_rx_len, 1);
      if(!buf)
      {
        QMI_FW_LOGE("%s: Unable to allocate read buffer for %p of size %d\n",
                     __func__, xp, xp->max_rx_len);
        break;
      }
      addr_size = sizeof(struct sockaddr_msm_ipc);
      rx_len = recvfrom(xp->fd, buf, xp->max_rx_len, MSG_DONTWAIT, (struct sockaddr *)&addr, &addr_size);
      if (rx_len < 0)
      {
        QMI_FW_LOGE("%s: Error recvfrom %p - rc : %d\n", __func__, xp, rx_len);
        free(buf);
        break;
      }
      else if (rx_len == 0)
      {
        if (addr_size == sizeof(struct sockaddr_msm_ipc))
        {
          QMI_CCI_LOGD("%s: Received Resume_Tx on FD %d from port %08x:%08x\n",
                        __func__, xp->fd,addr. address.addr.port_addr.node_id,
                        addr.address.addr.port_addr.port_id);
          qmi_cci_xport_resume(xp->clnt);
        }
        else
        {
          QMI_FW_LOGE("%s: No data read from %d\n", __func__, xp->fd);
        }
        free(buf);
        continue;
      }

      QMI_CCI_LOGD("%s: Received %d bytes from %d\n", __func__, rx_len, xp->fd);
      src_addr.service = 0;
      src_addr.instance = 0;
      src_addr.node_id = addr.address.addr.port_addr.node_id;
      src_addr.port_id = addr.address.addr.port_addr.port_id;
      qmi_cci_xport_recv(xp->clnt, &src_addr, buf, rx_len);
      free(buf);
    }

    if(pbits[1].revents & POLLIN)
    {
      rx_len = recvfrom(xp->ctl_fd, &rx_ctl_msg, sizeof(rx_ctl_msg), MSG_DONTWAIT,
                        NULL, NULL);
      if (rx_len < 0)
      {
         QMI_FW_LOGE("%s: Error recvfrom ctl_fd : %d\n", __func__, rx_len);
         break;
      }
      else if (rx_len == 0)
      {
         QMI_FW_LOGE("%s: No data read from %d\n", __func__, xp->ctl_fd);
         continue;
      }

      src_addr.service = rx_ctl_msg.srv.service;
      src_addr.instance = rx_ctl_msg.srv.instance;
      src_addr.node_id = rx_ctl_msg.srv.node_id;
      src_addr.port_id = rx_ctl_msg.srv.port_id;
      if (rx_ctl_msg.cmd == CTL_CMD_NEW_SERVER)
      {
        QMI_CCI_LOGD("Received NEW_SERVER cmd for %08x:%08x\n",
              rx_ctl_msg.srv.service, rx_ctl_msg.srv.instance);
        if (xp->srv_name.service ==  rx_ctl_msg.srv.service &&
            xp->srv_name.instance == GET_XPORT_SVC_INSTANCE(rx_ctl_msg.srv.instance))
            qmi_cci_xport_event_new_server(xp->clnt, &src_addr);
      }
      else if (rx_ctl_msg.cmd == CTL_CMD_REMOVE_SERVER)
      {
        QMI_CCI_LOGD("Received REMOVE_SERVER cmd for %08x:%08x\n",
              rx_ctl_msg.srv.service, rx_ctl_msg.srv.instance);
        if (xp->srv_name.service ==  rx_ctl_msg.srv.service &&
            xp->srv_name.instance == GET_XPORT_SVC_INSTANCE(rx_ctl_msg.srv.instance))
            qmi_cci_xport_event_remove_server(xp->clnt, &src_addr);
      }
    }

    if(pbits[0].revents)
    {
      read(xp->wakeup_pipe[0], &ch, 1);
      QMI_CCI_LOGD("%s: wakeup_pipe[0]=%x ch=%c\n", __func__, pbits[0].revents, ch);
      if(ch == 'd')
        break;
    }
  }
  QMI_CCI_LOGD("%s exiting\n", __func__);
  return NULL;
}


static void *xport_open
(
 void *xport_data,
 qmi_cci_client_type *clnt,
 uint32_t service_id,
 uint32_t version,
 void *addr,
 uint32_t max_rx_len
 )
{
  struct xport_handle *xp = calloc(sizeof(struct xport_handle), 1);
  int align_size = 0;
  int flags;

  if (!xp)
  {
    QMI_FW_LOGE("%s: xp calloc failed\n", __func__);
    return NULL;
  }

  xp->clnt = clnt;
  xp->srv_name.service = service_id;
  xp->srv_name.instance = version;
  xp->max_rx_len = (max_rx_len + QMI_HEADER_SIZE);
  align_size = ALIGN_SIZE(xp->max_rx_len);
  xp->max_rx_len += align_size;

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

  if (pipe(xp->wakeup_pipe) == -1)
  {
    QMI_FW_LOGE("%s: failed to create pipe\n", __func__);
    goto xport_open_close_ctl_fd;
  }

  /* create reader thread */
  if(pthread_create(&xp->reader_tid, NULL, reader_thread, (void *)xp))
  {
    QMI_FW_LOGE("%s: Reader thread creation failed\n", __func__);
    goto xport_open_close_pipe;
  }
  flags = fcntl(xp->fd, F_GETFL, 0);
  fcntl(xp->fd, F_SETFL, flags | O_NONBLOCK);
  if(write(xp->wakeup_pipe[1], "a", 1) < 0)
    QMI_FW_LOGE("%s: Error writing to pipe\n", __func__);
  QMI_CCI_LOGD("xport_open[%d]: max_rx_len=%d\n", xp->fd, max_rx_len);
  return xp;

xport_open_close_pipe:
  close(xp->wakeup_pipe[0]);
  close(xp->wakeup_pipe[1]);
xport_open_close_ctl_fd:
  close(xp->ctl_fd);
xport_open_close_fd:
  close(xp->fd);
xport_open_free_xp:
  free(xp);
  return NULL;
}


static qmi_client_error_type xport_send
(
 void *handle,
 void *addr,
 uint8_t *buf,
 uint32_t len
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct sockaddr_msm_ipc dest_addr;
  struct xport_ipc_router_server_addr *s_addr = (struct xport_ipc_router_server_addr *)addr;
  int send_ret_val;

  if (!s_addr)
  {
    QMI_FW_LOGE("%s: Invalid address parameter\n", __func__);
    return QMI_INTERNAL_ERR;
  }

  dest_addr.family = AF_MSM_IPC;
  dest_addr.address.addrtype = MSM_IPC_ADDR_ID;
  dest_addr.address.addr.port_addr.node_id = s_addr->node_id;
  dest_addr.address.addr.port_addr.port_id = s_addr->port_id;
  send_ret_val = sendto(xp->fd, buf, len, MSG_DONTWAIT, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_msm_ipc));
  if ((send_ret_val < 0) && (errno == EAGAIN))
  {
    QMI_CCI_LOGD("%s: Remote port %08x:%08x is busy for FD - %d\n",
                 __func__, s_addr->node_id, s_addr->port_id, xp->fd);
    return QMI_XPORT_BUSY_ERR;
  }
  else if ((send_ret_val < 0) && (errno == ENODEV || errno == EHOSTUNREACH))
  {
    QMI_FW_LOGE("%s: sendto failed errno = [%d]\n", __func__, errno);
    return QMI_SERVICE_ERR;
  }
  else if(send_ret_val < 0)
  {
    QMI_FW_LOGE("%s: Sendto failed for port %d\n", __func__, ntohs(s_addr->port_id));
    return QMI_INTERNAL_ERR;
  }
  QMI_CCI_LOGD("Sent[%d]: %d bytes to port %d\n", xp->fd, len, ntohs(s_addr->port_id));
  return QMI_NO_ERR;
}


static void xport_close
(
 void *handle
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;

  if(!xp)
  {
    QMI_FW_LOGE("%s: Invalid Handle %p\n", __func__, xp);
    return;
  }

  if (write(xp->wakeup_pipe[1], "d", 1) < 0)
    QMI_FW_LOGE("%s: Error writing to pipe\n", __func__);

  pthread_join(xp->reader_tid, NULL);
  close(xp->wakeup_pipe[0]);
  close(xp->wakeup_pipe[1]);
  close(xp->ctl_fd);
  QMI_CCI_LOGD("Close[%d]\n", xp->fd);
  close(xp->fd);
  qmi_cci_xport_closed(xp->clnt);
  free(xp);
}


static uint32_t xport_lookup
(
 void *xport_data,
 uint8_t xport_num,
 uint32_t service_id,
 uint32_t version,
 uint32_t *num_entries,
 qmi_cci_service_info *service_info
)
{
  uint32_t num_servers_found = 0;
  uint32_t num_entries_to_fill = 0;
  struct server_lookup_args *lookup_arg;
  struct xport_ipc_router_server_addr addr;
  int i;

  QMI_CCI_LOGD("Lookup: type=%d instance=%d\n", service_id, version);
  if (num_entries)
  {
    num_entries_to_fill = *num_entries;
    *num_entries = 0;
  }

  if (open_lookup_sock_fd() < 0)
    return 0;

  lookup_arg = (struct server_lookup_args *)malloc(sizeof(*lookup_arg)
			+ (num_entries_to_fill * sizeof(struct msm_ipc_server_info)));
  if (!lookup_arg) {
    QMI_FW_LOGE("%s: Malloc failed\n", __func__);
    return 0;
  }

  lookup_arg->port_name.service = service_id;
  lookup_arg->port_name.instance = GET_XPORT_SVC_INSTANCE(version);
  lookup_arg->num_entries_in_array = num_entries_to_fill;
  lookup_arg->lookup_mask = VERSION_MASK;
  lookup_arg->num_entries_found = 0;
  if (ioctl(lookup_sock_fd, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup_arg) < 0) {
    QMI_CCI_LOGD("%s: Lookup failed for %08x: %08x\n", __func__, service_id, version);
    free(lookup_arg);
    return 0;
  }

  for (i = 0; ((i < (int)num_entries_to_fill) && (i < lookup_arg->num_entries_found)); i++) {
    addr.service = lookup_arg->srv_info[i].service;
    addr.instance = lookup_arg->srv_info[i].instance;
    addr.node_id = lookup_arg->srv_info[i].node_id;
    addr.port_id = lookup_arg->srv_info[i].port_id;

    service_info[i].xport = xport_num;
    service_info[i].version = GET_VERSION(lookup_arg->srv_info[i].instance);
    service_info[i].instance = GET_INSTANCE(lookup_arg->srv_info[i].instance);
    service_info[i].reserved = 0;
    memcpy(&service_info[i].addr, &addr, sizeof(struct xport_ipc_router_server_addr));
  }
  if (num_entries)
    *num_entries = i;
  num_servers_found = lookup_arg->num_entries_found;
  free(lookup_arg);
  return num_servers_found;
}


static uint32_t xport_addr_len
(
 void
 )
{
  return sizeof(struct xport_ipc_router_server_addr);
}

qmi_cci_xport_ops_type qcci_ipc_router_ops =
{
  xport_open,
  xport_send,
  xport_close,
  xport_lookup,
  xport_addr_len
};
