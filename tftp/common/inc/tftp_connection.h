/***********************************************************************
 * tftp_connection.h
 *
 * Short description.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2014-07-28   rp    Add debug_info to measure timings.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-04   rp    Switch to IPCRouter sockets.
2013-11-24   nr    Support to set timeout and remote addr for an open conn.
2013-11-24   rp    Support TFTP extension options.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_CONNECTION_H__
#define __TFTP_CONNECTION_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "tftp_connection_addr.h"
#include "tftp_socket.h"
#include "tftp_utils.h"

struct tftp_connection_debug_info_type
{
  uint64 last_send_time_tick;
  uint64 last_recv_time_tick;

  uint64 last_send_time_delta;
  uint64 max_send_time_delta;
  uint64 min_send_time_delta;
  uint64 total_send_time_delta;

  uint64 last_recv_time_delta;
  uint64 max_recv_time_delta;
  uint64 min_recv_time_delta;
  uint64 total_recv_time_delta;
};

struct tftp_connection_handle
{
  int is_valid;
  int is_connected;

  struct tftp_connection_ipcr_addr con_local_addr;
  struct tftp_connection_ipcr_addr con_remote_addr;

  tftp_socket socket_handle;
  uint32 timeout_in_ms;
  int is_remote_addr_valid;
  int is_new_remote_addr_valid;
  uint32 recieve_packet_count;
  int last_errno;

  tftp_sockaddr local_addr;
  tftp_sockaddr remote_addr;
  tftp_sockaddr new_remote_addr;

  struct tftp_connection_debug_info_type debug_info;
};

enum tftp_connection_result
{
  TFTP_CONNECTION_RESULT_SUCCESS,
  TFTP_CONNECTION_RESULT_INVALID_ARGS,
  TFTP_CONNECTION_RESULT_FAILED,
  TFTP_CONNECTION_RESULT_TIMEOUT,
  TFTP_CONNECTION_RESULT_HANGUP,
};

void tftp_connection_init (void);

enum tftp_connection_result tftp_connection_open (
          struct tftp_connection_handle *handle,
          struct tftp_connection_ipcr_addr *con_remote_addr,
          uint32 timeout_in_ms);


enum tftp_connection_result tftp_connection_send_data (
          struct tftp_connection_handle *handle,
          void *buffer, uint32 buffer_size, uint32 *out_buffer_size);

enum tftp_connection_result tftp_connection_get_data (
          struct tftp_connection_handle *handle,
          void *buffer, uint32 buffer_size, uint32 *out_buffer_size);

enum tftp_connection_result tftp_connection_get_new_remote_addr (
          struct tftp_connection_handle *handle,
          struct tftp_connection_ipcr_addr *remote_addr);

enum tftp_connection_result tftp_connection_set_timeout (
          struct tftp_connection_handle *handle,
          uint32 timeout_in_ms);

enum tftp_connection_result tftp_connection_get_timeout (
          struct tftp_connection_handle *handle,
          uint32 *timeout_in_ms);

enum tftp_connection_result tftp_connection_close (
          struct tftp_connection_handle *handle);

enum tftp_connection_result tftp_connection_switch_to_new_remote_addr (
          struct tftp_connection_handle *handle);

enum tftp_connection_result tftp_connection_connect (
          struct tftp_connection_handle *handle);

#endif /* __TFTP_CONNECTION_H__ */
