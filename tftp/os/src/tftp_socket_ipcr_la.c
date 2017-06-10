/***********************************************************************
 * tftp_socket_ipcr_la.c
 *
 * Short description.
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2015-01-05   vm    Compiling server for TN Apps.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-10-14   dks   Move OS specific arguments out of tftp_server.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_socket.h"
#include "tftp_socket_ipcr_la.h"
#include "msm_ipc.h"
#include "tftp_assert.h"
#include "tftp_os.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


#ifdef TFTP_SIMULATOR_BUILD
  #include "sim_ipcr_socket_la.h"
  #define ipcr_la_bind      sim_ipcr_socket_la_bind
  #define ipcr_la_sendto    sim_ipcr_socket_la_sendto
  #define ipcr_la_recvfrom  sim_ipcr_socket_la_recvfrom
  #define ipcr_la_connect   sim_ipcr_socket_la_connect
#else
  #define ipcr_la_bind      bind
  #define ipcr_la_sendto    sendto
  #define ipcr_la_recvfrom  recvfrom
  #define ipcr_la_connect   connect
#endif

enum tftp_socket_result_type
tftp_socket_open (tftp_socket *socket_handle, int do_bind)
{
  enum tftp_socket_result_type sock_res;

  (void) do_bind;

#ifdef TFTP_SIMULATOR_BUILD
  *socket_handle = sim_ipcr_socket_la_socket (AF_MSM_IPC, SOCK_DGRAM, 0,
                                              do_bind);
#else
  *socket_handle = socket (AF_MSM_IPC, SOCK_DGRAM, 0);
#endif
  sock_res = (*socket_handle == -1) ? TFTP_SOCKET_RESULT_FAILED :
                                      TFTP_SOCKET_RESULT_SUCCESS;
  return sock_res;
}


enum tftp_socket_result_type
tftp_socket_close (tftp_socket *socket_handle)
{
  enum tftp_socket_result_type sock_res = TFTP_SOCKET_RESULT_FAILED;
  int result;

  result = close(*socket_handle);
  sock_res = (result == 0) ? TFTP_SOCKET_RESULT_SUCCESS:
                             TFTP_SOCKET_RESULT_FAILED;
  return sock_res;
}

int tftp_socket_get_last_errno (tftp_socket *socket_handle)
{
  (void) socket_handle;
  return errno;
}


enum tftp_socket_result_type
tftp_socket_set_addr_by_name (tftp_sockaddr *sockaddr,
                              uint32 service_id, uint32 instance_id)
{
  memset(sockaddr, 0, sizeof(tftp_sockaddr));
  sockaddr->family = AF_MSM_IPC;
  sockaddr->address.addrtype = MSM_IPC_ADDR_NAME;
  sockaddr->address.addr.port_name.service = service_id;
  sockaddr->address.addr.port_name.instance = instance_id;
  return TFTP_SOCKET_RESULT_SUCCESS;
}

enum tftp_socket_result_type
tftp_socket_set_addr_by_port (tftp_sockaddr *sockaddr,
                              uint32 proc_id, uint32 port_id)
{
  memset (sockaddr, 0, sizeof(tftp_sockaddr));
  sockaddr->family = AF_MSM_IPC;
  sockaddr->address.addrtype = MSM_IPC_ADDR_ID;
  sockaddr->address.addr.port_addr.node_id = proc_id;
  sockaddr->address.addr.port_addr.port_id = port_id;
  return TFTP_SOCKET_RESULT_SUCCESS;
}

enum tftp_socket_result_type
tftp_socket_get_addr_by_port (tftp_socket *socket_handle,
                              tftp_sockaddr *sockaddr,
                              uint32 *proc_id, uint32 *port_id)
{
  (void) socket_handle;

  TFTP_ASSERT (sockaddr != NULL);
  TFTP_ASSERT (proc_id != NULL);
  TFTP_ASSERT (port_id != NULL);

  if ((socket_handle == NULL) ||
      (sockaddr == NULL) ||
      (proc_id == NULL) ||
      (port_id == NULL))
  {
    return TFTP_SOCKET_RESULT_FAILED;
  }

  TFTP_ASSERT (sockaddr->family == AF_MSM_IPC);
  TFTP_ASSERT (sockaddr->address.addrtype == MSM_IPC_ADDR_ID);
  *proc_id = sockaddr->address.addr.port_addr.node_id;
  *port_id = sockaddr->address.addr.port_addr.port_id;

  return TFTP_SOCKET_RESULT_SUCCESS;
}

enum tftp_socket_result_type
tftp_socket_connect(tftp_socket *socket_handle, tftp_sockaddr *remote_addr)
{
  enum tftp_socket_result_type sock_res;
  int32 result;

  result = ipcr_la_connect (*socket_handle, (struct sockaddr *)remote_addr,
                            sizeof (tftp_sockaddr));
  if (result == -1)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else
  {
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

  return sock_res;
}

enum tftp_socket_result_type
tftp_socket_bind (tftp_socket *socket_handle, tftp_sockaddr *local_addr)
{
  enum tftp_socket_result_type sock_res = TFTP_SOCKET_RESULT_FAILED;
  int result;

  result = ipcr_la_bind (*socket_handle, (struct sockaddr *)local_addr,
                          sizeof(tftp_sockaddr));
  if (result == 0)
  {
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

  return sock_res;
}

enum tftp_socket_result_type
tftp_socket_sendto (tftp_socket *socket_handle, void *buffer,
                       int32 buffer_size, tftp_sockaddr *remote_addr,
                       int32 *sent_size)
{
  enum tftp_socket_result_type sock_res;
  int32 result;

  result = ipcr_la_sendto (*socket_handle, buffer, buffer_size, 0,
                          (struct sockaddr *)remote_addr,
                          sizeof (tftp_sockaddr));
  if (result == -1)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else
  {
    *sent_size = result;
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

  return sock_res;
}

enum tftp_socket_result_type
tftp_socket_send (tftp_socket *socket_handle, void *buffer, int32 buffer_size,
                  int32 *sent_size)
{
  enum tftp_socket_result_type sock_res;
  int32 result;

  result = send (*socket_handle, buffer, buffer_size, 0);
  if (result == -1)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else
  {
    *sent_size = result;
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

  return sock_res;
}

int
tftp_socket_poll(tftp_socket_pollfd *pfd, uint32 pfd_count,
                      int32 timeout_in_ms)
{
  int poll_res;
  uint32 i;

  TFTP_ASSERT (pfd != NULL);
  TFTP_ASSERT (pfd_count > 0);
  if ((pfd == NULL) ||
      (pfd_count == 0))
  {
    //todo : return an proper errno?
    return -1;
  }

  for (i = 0; i < pfd_count; i++)
  {
    pfd[i].events = POLLIN;
    pfd[i].revents = 0;
  }

  poll_res = poll (pfd, pfd_count, timeout_in_ms);

  if (poll_res < 0)
  {
    poll_res = -errno;
  }
  return poll_res;
}

enum tftp_socket_result_type
tftp_socket_interpret_poll_result(tftp_socket_pollfd *pfd, int poll_res)
{
  enum tftp_socket_result_type sock_res;

  if(poll_res < 0)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else if(poll_res == 0)
  {
    sock_res = TFTP_SOCKET_RESULT_TIMEOUT;
  }
  else if (pfd->revents & POLLIN)
  {
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }
  else
  {
    sock_res = TFTP_SOCKET_RESULT_HANGUP;
  }

  return sock_res;
}

static enum tftp_socket_result_type
tftp_socket_wait(tftp_socket *socket_handle, int32 timeout_in_ms)
{
  tftp_socket_pollfd pfd;
  enum tftp_socket_result_type sock_res;
  int poll_res;

  pfd.fd = *socket_handle;
  pfd.events = POLLIN;
  pfd.revents = 0;

  poll_res = tftp_socket_poll (&pfd, 1, timeout_in_ms);
  sock_res = tftp_socket_interpret_poll_result (&pfd, poll_res);
  return sock_res;
}


enum tftp_socket_result_type
tftp_socket_recvfrom (tftp_socket *socket_handle, void *buffer,
                       int32 buffer_size, tftp_sockaddr *remote_addr,
                       uint32 timeout_in_ms, int32 *recd_size)
{
  enum tftp_socket_result_type sock_res;
  int32 result;
  tftp_sockaddr_len_type size_of_addr = sizeof (tftp_sockaddr);

  if (timeout_in_ms > 0)
  {
    sock_res = tftp_socket_wait(socket_handle, timeout_in_ms);
    if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
    {
      goto End;
    }
  }

  result = ipcr_la_recvfrom (*socket_handle, buffer, buffer_size, 0,
                            (struct sockaddr *)remote_addr, &size_of_addr);
  if (result < 0)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else
  {
    *recd_size = result;
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

End:
  return sock_res;
}

enum tftp_socket_result_type
tftp_socket_recv (tftp_socket *socket_handle, void *buffer, int32 buffer_size,
                  uint32 timeout_in_ms, int32 *recd_size)
{
  enum tftp_socket_result_type sock_res;
  int32 result;

  if (timeout_in_ms > 0)
  {
    sock_res = tftp_socket_wait(socket_handle, timeout_in_ms);
    if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
    {
      goto End;
    }
  }

  result = recv(*socket_handle, buffer, buffer_size, 0);
  if (result < 0)
  {
    sock_res = TFTP_SOCKET_RESULT_FAILED;
  }
  else
  {
    *recd_size = result;
    sock_res = TFTP_SOCKET_RESULT_SUCCESS;
  }

End:
  return sock_res;
}

int
tftp_socket_get_socket_desc (tftp_socket *socket_handle)
{
  TFTP_ASSERT (socket_handle != NULL);
  if (socket_handle == NULL)
  {
    return -1;
  }
  return *socket_handle;
}

enum tftp_socket_result_type
tftp_socket_invalidate_socket_handle (tftp_socket *socket_handle)
{
  TFTP_ASSERT (socket_handle != NULL);
  if (socket_handle == NULL)
  {
    return TFTP_SOCKET_RESULT_FAILED;
  }

  *socket_handle = -1;
  return TFTP_SOCKET_RESULT_SUCCESS;
}

