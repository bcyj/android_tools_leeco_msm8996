/***********************************************************************
 * tftp_connection.c
 *
 * TFTP Connection layer.
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP Connection layer
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2015-01-05   dks   Compile server for TN Apps.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-08-26   rp    Bring in changes from target-unit-testing.
2014-07-28   rp    Add debug_info to measure timings.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2013-11-24   nr    Support to set timeout and remote addr for an open conn.
2012-12-05   nr    Add support to abstract LWIP sockets.
2013-12-05   rp    Add new debug-log interface.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-24   rp    Support TFTP extension options.
2013-11-15   dks   Fix printf warnings.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_connection.h"
#include "tftp_connection_addr.h"
#include "tftp_assert.h"
#include "tftp_log.h"
#include "tftp_string.h"

#include <stdlib.h>

volatile int tftp_connection_enable_pkt_logging = 0;
volatile int tftp_connection_enable_debug_logs = 0;

void
tftp_connection_init (void)
{
#ifdef TFTP_CONNECTION_ENABLE_PKT_LOGGING
  tftp_connection_enable_pkt_logging = 1;
#endif
}

enum tftp_connection_result
tftp_connection_open (struct tftp_connection_handle *handle,
          struct tftp_connection_ipcr_addr *con_remote_addr,
          uint32 timeout_in_ms)
{
  enum tftp_connection_result connection_result=TFTP_CONNECTION_RESULT_FAILED;
  enum tftp_socket_result_type sock_res;

  TFTP_ASSERT (handle != NULL);
  TFTP_ASSERT (con_remote_addr != NULL);
  if ((handle == NULL) ||
      (con_remote_addr == NULL))
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  memset (handle, 0, sizeof (struct tftp_connection_handle));

  handle->timeout_in_ms = timeout_in_ms;
  handle->debug_info.min_send_time_delta = 0xFFFFFFFF;
  handle->debug_info.min_recv_time_delta = 0xFFFFFFFF;

  sock_res = tftp_socket_open (&handle->socket_handle, 1);
  if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
  {
    TFTP_LOG_ERR ("Socket open failed errno = %d",
                  tftp_socket_get_last_errno (&handle->socket_handle));
    return TFTP_CONNECTION_RESULT_FAILED;
  }


  tftp_memscpy (&handle->con_remote_addr, sizeof (handle->con_remote_addr),
                con_remote_addr, sizeof(handle->con_remote_addr));

  switch(con_remote_addr->addr_type)
  {
    case TFTP_CONNECTION_IPCR_ADDR_TYPE_NAME:
    {
      sock_res = tftp_socket_set_addr_by_name (&handle->remote_addr,
                  handle->con_remote_addr.u.name_addr.service_id,
                  handle->con_remote_addr.u.name_addr.instance_id);
      TFTP_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);
      if (tftp_connection_enable_debug_logs)
      {
        TFTP_LOG_DBG ("conn opened sd = %d, name = [sid = %d, iid = %d]",
                      tftp_socket_get_socket_desc (&handle->socket_handle),
                      handle->con_remote_addr.u.name_addr.service_id,
                      handle->con_remote_addr.u.name_addr.instance_id);
      }
      break;
    }

    case TFTP_CONNECTION_IPCR_ADDR_TYPE_PORT:
    {
      sock_res = tftp_socket_set_addr_by_port (&handle->remote_addr,
                  handle->con_remote_addr.u.port_addr.proc_id,
                  handle->con_remote_addr.u.port_addr.port_id);
      TFTP_ASSERT (sock_res == TFTP_SOCKET_RESULT_SUCCESS);
      if (tftp_connection_enable_debug_logs)
      {
        TFTP_LOG_DBG ("conn opened sd = %d, port = [proc = %d, port = %d]",
                      tftp_socket_get_socket_desc (&handle->socket_handle),
                      handle->con_remote_addr.u.port_addr.proc_id,
                      handle->con_remote_addr.u.port_addr.port_id);
      }
    }
    break;

    default:
      TFTP_ASSERT (0);
      break;
  }

  handle->is_remote_addr_valid = 1;

  handle->is_valid = 1;
  handle->is_connected = 0;
  connection_result = TFTP_CONNECTION_RESULT_SUCCESS;
  return connection_result;
}

enum tftp_connection_result
tftp_connection_get_new_remote_addr (struct tftp_connection_handle *handle,
     struct tftp_connection_ipcr_addr *remote_addr)
{
  enum tftp_connection_result con_res = TFTP_CONNECTION_RESULT_FAILED;
  enum tftp_socket_result_type sock_res;
  uint32 proc_id, port_id;

  TFTP_ASSERT (handle != NULL);
  TFTP_ASSERT (remote_addr != NULL);
  if ((handle == NULL) ||
      (remote_addr == NULL))
  {
    con_res = TFTP_CONNECTION_RESULT_INVALID_ARGS;
    goto End;
  }

  TFTP_ASSERT (handle->is_valid == 1);
  TFTP_ASSERT (handle->is_new_remote_addr_valid == 1);

  sock_res = tftp_socket_get_addr_by_port (&handle->socket_handle,
                    &handle->new_remote_addr, &proc_id, &port_id);
  TFTP_ASSERT(sock_res == TFTP_SOCKET_RESULT_SUCCESS);
  if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
  {
    goto End;
  }

  memset(remote_addr, 0, sizeof (struct tftp_connection_ipcr_addr));
  remote_addr->addr_type = TFTP_CONNECTION_IPCR_ADDR_TYPE_PORT;
  remote_addr->u.port_addr.proc_id = proc_id;
  remote_addr->u.port_addr.port_id = port_id;

  if (tftp_connection_enable_debug_logs)
  {
    TFTP_LOG_DBG("get-new-remote-addr. sd = %d, port = [proc = %d, port = %d]",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  handle->con_remote_addr.u.port_addr.proc_id,
                  handle->con_remote_addr.u.port_addr.port_id);
  }

  con_res = TFTP_CONNECTION_RESULT_SUCCESS;

End:
  return con_res;
}

enum tftp_connection_result
tftp_connection_send_data (struct tftp_connection_handle *handle,
                            void *buffer, uint32 buffer_size,
                            uint32 *out_buffer_size)
{
  enum tftp_connection_result send_result;
  enum tftp_socket_result_type sock_res;
  int32 send_size = -1;
  uint64 start_time_tick, end_time_tick, time_delta;

  TFTP_ASSERT (handle != NULL);
  TFTP_ASSERT (buffer != NULL);
  TFTP_ASSERT (out_buffer_size != NULL);
  if ((handle == NULL) ||
      (buffer == NULL) ||
      (out_buffer_size == NULL))
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (handle->is_valid == 1);
  TFTP_ASSERT (handle->is_remote_addr_valid == 1);

  *out_buffer_size = 0;

  if (tftp_connection_enable_pkt_logging)
  {
    TFTP_LOG_RAW_TFTP_BUF (buffer, buffer_size,
                  "SEND : sd = %d size = %d : [%d, %d] -> [%d, %d]",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  buffer_size,
                  handle->con_local_addr.u.port_addr.proc_id,
                  handle->con_local_addr.u.port_addr.port_id,
                  handle->con_remote_addr.u.port_addr.proc_id,
                  handle->con_remote_addr.u.port_addr.port_id);
  }

  if (tftp_connection_enable_debug_logs)
  {
    TFTP_LOG_DBG ("SEND : sd = %d size = %d : [%d, %d] -> [%d, %d]",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  buffer_size,
                  handle->con_local_addr.u.port_addr.proc_id,
                  handle->con_local_addr.u.port_addr.port_id,
                  handle->con_remote_addr.u.port_addr.proc_id,
                  handle->con_remote_addr.u.port_addr.port_id);
  }

  start_time_tick = tftp_timetick_get ();

  if (handle->is_connected == 0)
  {
    sock_res = tftp_socket_sendto (&handle->socket_handle, buffer,
                                   buffer_size, &handle->remote_addr,
                                   &send_size);
  }
  else
  {
    sock_res = tftp_socket_send (&handle->socket_handle, buffer,
                                 buffer_size, &send_size);
  }

  end_time_tick = tftp_timetick_get ();

  if (sock_res == TFTP_SOCKET_RESULT_SUCCESS)
  {
    TFTP_ASSERT (send_size >= 0);
    *out_buffer_size = (uint32)send_size;
    send_result = TFTP_CONNECTION_RESULT_SUCCESS;
  }
  else
  {
    int err_num = tftp_socket_get_last_errno (&handle->socket_handle);

    send_result = TFTP_CONNECTION_RESULT_FAILED;
    handle->last_errno = err_num;
    TFTP_LOG_ERR ("SEND failed : sd = %d : size = %d : errno = %d : msg = %s",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  buffer_size, err_num, strerror (err_num));
  }

  time_delta = tftp_timetick_diff_in_usec (start_time_tick, end_time_tick);

  handle->debug_info.last_send_time_tick = start_time_tick;
  handle->debug_info.last_send_time_delta = time_delta;
  handle->debug_info.total_send_time_delta += time_delta;
  if (time_delta > handle->debug_info.max_send_time_delta)
  {
    handle->debug_info.max_send_time_delta = time_delta;
  }
  if (time_delta < handle->debug_info.min_send_time_delta)
  {
    handle->debug_info.min_send_time_delta = time_delta;
  }

  return send_result;
}

enum tftp_connection_result
tftp_connection_get_data (struct tftp_connection_handle *handle,
                          void *buffer, uint32 buffer_size,
                          uint32 *out_buffer_size)
{
  enum tftp_connection_result read_result = TFTP_CONNECTION_RESULT_FAILED;
  enum tftp_socket_result_type sock_res;
  int32 received_size = -1;
  uint64 start_time_tick, end_time_tick, time_delta;

  TFTP_ASSERT (handle != NULL);
  TFTP_ASSERT (buffer != NULL);
  TFTP_ASSERT (out_buffer_size != NULL);
  if ((handle == NULL) ||
      (buffer == NULL) ||
      (out_buffer_size == NULL))
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (handle->is_valid == 1);

  *out_buffer_size = 0;

  start_time_tick = tftp_timetick_get ();

  if (handle->is_connected == 0)
  {
    sock_res = tftp_socket_recvfrom (&handle->socket_handle, buffer,
                                     buffer_size, &handle->new_remote_addr,
                                     handle->timeout_in_ms,
                                     &received_size);
  }
  else
  {
    sock_res = tftp_socket_recv (&handle->socket_handle, buffer,
                                 buffer_size, handle->timeout_in_ms,
                                 &received_size);;
  }

  end_time_tick = tftp_timetick_get ();

  if (sock_res == TFTP_SOCKET_RESULT_SUCCESS)
  {
    if (tftp_connection_enable_pkt_logging)
    {
      TFTP_LOG_RAW_TFTP_BUF (buffer, received_size,
                    "RECV. sd = %d : size = %d : [%d, %d] <- [%d, %d]",
                    tftp_socket_get_socket_desc (&handle->socket_handle),
                    received_size,
                    handle->con_local_addr.u.port_addr.proc_id,
                    handle->con_local_addr.u.port_addr.port_id,
                    handle->con_remote_addr.u.port_addr.proc_id,
                    handle->con_remote_addr.u.port_addr.port_id);
    }

    if (tftp_connection_enable_debug_logs)
    {
      TFTP_LOG_DBG ("RECV. sd = %d : size = %d : [%d, %d] <- [%d, %d]",
                    tftp_socket_get_socket_desc (&handle->socket_handle),
                    received_size,
                    handle->con_local_addr.u.port_addr.proc_id,
                    handle->con_local_addr.u.port_addr.port_id,
                    handle->con_remote_addr.u.port_addr.proc_id,
                    handle->con_remote_addr.u.port_addr.port_id);
    }
  }

  switch (sock_res)
  {
    case TFTP_SOCKET_RESULT_SUCCESS:
    {
      TFTP_ASSERT (received_size >= 0);
      *out_buffer_size = (size_t)received_size;
      handle->is_new_remote_addr_valid = 1;
      read_result = TFTP_CONNECTION_RESULT_SUCCESS;
      break;
    }

    case TFTP_SOCKET_RESULT_TIMEOUT:
    {
      read_result = TFTP_CONNECTION_RESULT_TIMEOUT;
      break;
    }

    case TFTP_SOCKET_RESULT_FAILED:
    {
      int err_num = tftp_socket_get_last_errno (&handle->socket_handle);
      read_result = TFTP_CONNECTION_RESULT_FAILED;
      handle->last_errno = err_num;
      TFTP_LOG_ERR ("recv failed. sd = %d : errno = %d, err-msg = [%s]",
                    tftp_socket_get_socket_desc (&handle->socket_handle),
                    err_num, strerror(err_num));
      break;
    }

    case TFTP_SOCKET_RESULT_HANGUP:
    {
      int err_num = tftp_socket_get_last_errno (&handle->socket_handle);
      read_result = TFTP_CONNECTION_RESULT_HANGUP;
      TFTP_LOG_ERR ("recv failed remote hangup. sd = %d :"
                    "errno = %d, err-msg = [%s]",
                    tftp_socket_get_socket_desc (&handle->socket_handle),
                    err_num, strerror(err_num));
      break;
    }
  }

  if (sock_res != TFTP_SOCKET_RESULT_SUCCESS)
  {
    goto End;
  }

  ++handle->recieve_packet_count;

  time_delta = tftp_timetick_diff_in_usec (start_time_tick, end_time_tick);

  handle->debug_info.last_recv_time_tick = start_time_tick;
  handle->debug_info.last_recv_time_delta = time_delta;
  handle->debug_info.total_recv_time_delta += time_delta;
  if (time_delta > handle->debug_info.max_recv_time_delta)
  {
    handle->debug_info.max_recv_time_delta = time_delta;
  }
  if (time_delta < handle->debug_info.min_recv_time_delta)
  {
    handle->debug_info.min_recv_time_delta = time_delta;
  }


End:
  return read_result;
}

enum tftp_connection_result
tftp_connection_close (struct tftp_connection_handle *handle)
{
  TFTP_ASSERT (handle != NULL);
  if (handle == NULL)
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (handle->is_valid == 1);

  if (tftp_connection_enable_debug_logs)
  {
    TFTP_LOG_DBG ("close. sd = %d",
                  tftp_socket_get_socket_desc (&handle->socket_handle));
  }

  (void) tftp_socket_close (&handle->socket_handle);

  memset (handle, 0, sizeof (struct tftp_connection_handle));
  return TFTP_CONNECTION_RESULT_SUCCESS;
}

enum tftp_connection_result
tftp_connection_switch_to_new_remote_addr (struct tftp_connection_handle *hdl)
{
  TFTP_ASSERT (hdl != NULL);
  if (hdl == NULL)
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (hdl->is_valid == 1);
  TFTP_ASSERT (hdl->is_new_remote_addr_valid == 1);

  tftp_memscpy (&hdl->remote_addr, sizeof (hdl->remote_addr),
                &hdl->new_remote_addr, sizeof(hdl->remote_addr));

  tftp_connection_get_new_remote_addr (hdl, &hdl->con_remote_addr);
  hdl->is_remote_addr_valid = 1;

  return TFTP_CONNECTION_RESULT_SUCCESS;
}

enum tftp_connection_result
tftp_connection_set_timeout (struct tftp_connection_handle *handle,
                              uint32 timeout_in_ms)
{
  TFTP_ASSERT (handle != NULL);
  if (handle == NULL)
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (handle->is_valid == 1);

  handle->timeout_in_ms = timeout_in_ms;
  return TFTP_CONNECTION_RESULT_SUCCESS;
}

enum tftp_connection_result
tftp_connection_get_timeout (struct tftp_connection_handle *handle,
                             uint32 *timeout_in_ms)
{
  TFTP_ASSERT (handle != NULL);
  TFTP_ASSERT (timeout_in_ms != NULL);
  if ((handle == NULL) || (timeout_in_ms == NULL))
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }

  TFTP_ASSERT (handle->is_valid == 1);

  *timeout_in_ms = handle->timeout_in_ms;
  return TFTP_CONNECTION_RESULT_SUCCESS;
}

enum tftp_connection_result
tftp_connection_connect (struct tftp_connection_handle *handle)
{
  enum tftp_connection_result connect_res = TFTP_CONNECTION_RESULT_FAILED;
  enum tftp_socket_result_type sock_res;

  TFTP_ASSERT (handle != NULL);
  if (handle == NULL)
  {
    return TFTP_CONNECTION_RESULT_INVALID_ARGS;
  }
  TFTP_ASSERT (handle->is_valid == 1);

  if (tftp_connection_enable_debug_logs)
  {
    TFTP_LOG_DBG ("connect. sd = %d [%d, %d] -> [%d, %d]",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  handle->con_local_addr.u.port_addr.proc_id,
                  handle->con_local_addr.u.port_addr.port_id,
                  handle->con_remote_addr.u.port_addr.proc_id,
                  handle->con_remote_addr.u.port_addr.port_id);
  }

  sock_res = tftp_socket_connect(&handle->socket_handle, &handle->remote_addr);
  if (sock_res == TFTP_SOCKET_RESULT_SUCCESS)
  {
    handle->is_connected = 1;
    connect_res = TFTP_CONNECTION_RESULT_SUCCESS;
  }
  else
  {
    int err_num = tftp_socket_get_last_errno (&handle->socket_handle);

    connect_res = TFTP_CONNECTION_RESULT_FAILED;
    handle->last_errno = err_num;
    TFTP_LOG_ERR ("Connect failed. sd = %d : errno = %d, msg = [%s]",
                  tftp_socket_get_socket_desc (&handle->socket_handle),
                  err_num, strerror (err_num));
  }
  return connect_res;
}
