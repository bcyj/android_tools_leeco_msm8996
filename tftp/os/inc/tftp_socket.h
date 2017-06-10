/***********************************************************************
 * tftp_socket.h
 *
 * TFTP socket abstraction layer.
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP socket abstraction layer.
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
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-04   rp    Switch to IPCRouter sockets.
2012-12-05   nr    Add support for netlink sockets.
2012-12-05   nr    Add support to abstract LWIP sockets.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-12-03   nr    Create

===========================================================================*/

#ifndef __TFTP_SOCKET_H__
#define __TFTP_SOCKET_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

#if defined (TFTP_USE_IPCR_MODEM_SOCKETS)
  #include "tftp_socket_ipcr_modem.h"
#elif defined (TFTP_USE_IPCR_LA_SOCKETS)
  #include "tftp_socket_ipcr_la.h"
#elif defined (TFTP_USE_IPCR_WINDOWS_SOCKETS)
  #include "tftp_socket_ipcr_windows.h"
#else
  #error "Sockets : Unknown config"
#endif

enum tftp_socket_result_type
{
  TFTP_SOCKET_RESULT_SUCCESS,
  TFTP_SOCKET_RESULT_FAILED,
  TFTP_SOCKET_RESULT_TIMEOUT,
  TFTP_SOCKET_RESULT_HANGUP,
};


enum tftp_socket_result_type tftp_socket_open (tftp_socket *socket_handle,
                                               int do_bind);

enum tftp_socket_result_type tftp_socket_close (tftp_socket *socket_handle);

int tftp_socket_get_last_errno (tftp_socket *socket_handle);

enum tftp_socket_result_type tftp_socket_bind (tftp_socket *socket_handle,
                                               tftp_sockaddr *local_addr);

enum tftp_socket_result_type tftp_socket_recvfrom (
           tftp_socket *socket_handle, void *buffer, int32 buffer_size,
           tftp_sockaddr *remote_addr, uint32 timeout_in_ms,
           int32 *recd_size);

enum tftp_socket_result_type tftp_socket_recv (
           tftp_socket *socket_handle, void *buffer, int32 buffer_size,
           uint32 timeout_in_ms, int32 *recd_size);

enum tftp_socket_result_type tftp_socket_sendto (
          tftp_socket *socket_handle, void *buffer, int32 buffer_size,
          tftp_sockaddr *remote_addr, int32 *sent_size);

enum tftp_socket_result_type tftp_socket_send (
          tftp_socket *socket_handle, void *buffer, int32 buffer_size,
          int32 *sent_size);

enum tftp_socket_result_type tftp_socket_set_addr_by_name (
          tftp_sockaddr *sockaddr, uint32 service_id, uint32 instance_id);

enum tftp_socket_result_type tftp_socket_set_addr_by_port (
          tftp_sockaddr *sockaddr, uint32 proc_id, uint32 port_id);

enum tftp_socket_result_type
tftp_socket_get_addr_by_port (tftp_socket *socket_handle,
                              tftp_sockaddr *sockaddr,
                              uint32 *proc_id, uint32 *port_id);


int tftp_socket_poll(tftp_socket_pollfd *pfd, uint32 pfd_count,
                      int32 timeout_in_ms);

enum tftp_socket_result_type
tftp_socket_interpret_poll_result(tftp_socket_pollfd *pfd, int poll_res);

enum tftp_socket_result_type tftp_socket_connect(tftp_socket *socket_handle,
                                                  tftp_sockaddr *remote_addr);

int tftp_socket_get_socket_desc (tftp_socket *socket_handle);

enum tftp_socket_result_type tftp_socket_invalidate_socket_handle (
                             tftp_socket *socket_handle);

#endif /* not __TFTP_SOCKET_H__ */
