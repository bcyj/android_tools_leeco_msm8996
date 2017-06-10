/***********************************************************************
 * tftp_protocol.h
 *
 * The module that implements the TFTP protocol to send and receive data.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The module that implements the TFTP protocol to send and receive data.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-09-19   dks   Add hooks to extract performance numbers.
2014-07-28   rp    Move log-buffer from global to stack.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   rp    Add support to do partial-file reads.
2013-12-26   rp    Add tftp-client module.
2013-11-27   nr    Add support to receive data.
2013-11-25   nr    Create

===========================================================================*/

#ifndef __TFTP_PROTOCOL_H__
#define __TFTP_PROTOCOL_H__

#include "tftp_config_i.h"
#include "tftp_pkt.h"
#include "tftp_connection.h"

enum tftp_protocol_result
{
  TFTP_PROTOCOL_INVALID = -1,
  TFTP_PROTOCOL_SUCCESS = 0,
  TFTP_PROTOCOL_EOT,
  TFTP_PROTOCOL_TIMEOUT,
  TFTP_PROTOCOL_WRONG_PACKET,
  TFTP_PROTOCOL_CONN_ERROR,
  TFTP_PROTOCOL_REMOTE_HANGUP,
  TFTP_PROTOCOL_PKT_ERROR,
  TFTP_PROTOCOL_ERROR_PKT_RECD,
  TFTP_PROTOCOL_ENOSPC,
  TFTP_PROTOCOL_INSUFFICIENT_BUFFER,
  TFTP_PROTOCOL_UNDEFINED_ERROR,
  TFTP_PROTOCOL_FILE_NOT_FOUND,
  TFTP_PROTOCOL_ACCESS_VIOLATION,
  TFTP_PROTOCOL_DISK_FULL,
  TFTP_PROTOCOL_ILLEGAL_FTP_OPERATION,
  TFTP_PROTOCOL_UNKNOWN_TRANSFER_ID,
  TFTP_PROTOCOL_FILE_ALREADY_EXISTS,
  TFTP_PROTOCOL_NO_SUCH_USER,
  TFTP_PROTOCOL_INVALID_OPTIONS,
};

enum tftp_protocol_request_type
{
  TFTP_REQUEST_TYPE_INVALID = 0,
  TFTP_REQUEST_TYPE_RRQ,
  TFTP_REQUEST_TYPE_WRQ,
};

struct tftp_protocol_debug_info_type
{
  uint32 total_blocks;
  uint32 total_bytes;
  uint32 total_timeouts;
  uint32 total_wrong_pkts;
};

struct tftp_protocol_info
{
  enum tftp_protocol_request_type type; /* Request type for server. */

  uint32 block_size;
  uint32 timeout_in_ms;
  uint32 last_ack_timeout_ms;
  uint32 current_timeout_ms;
  uint64 begin_wait_pkt_timestamp;
  uint64 current_wait_pkt_timestamp;
  uint32 remaining_timeout_ms;
  uint8  is_seek_offset_valid;
  uint32 seek_offset;
  uint8  do_append;
  uint8  do_unlink;
  uint32 window_size;
  uint32 tsize;
  uint32 retry_count;
  uint32 max_pkt_retry_count;
  uint32 max_wrong_pkt_count;

  uint16 curr_block_number;
  uint32 actual_block_number; /* The real block number after rollover. */
  int    insufficient_recv_buffer;
  int    final_pkt_received;
  int    final_pkt_sent;
  uint32 retransmitted_pkt_count;


  int is_connection_open;
  struct tftp_connection_handle connection_hdl;
  struct tftp_connection_ipcr_addr remote_addr;

  struct tftp_pkt_options_type options;

  struct tftp_pkt_type tx_pkt;
  struct tftp_pkt_type rx_pkt;

  struct tftp_protocol_debug_info_type debug_info;
};



enum tftp_protocol_result
tftp_protocol_open_connection (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_close_connection (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_update_remote_addr (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_connect_remote_addr (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_send_wrq_helper (struct tftp_protocol_info *proto,
                                const char *filename);
enum tftp_protocol_result
tftp_protocol_send_rrq_helper (struct tftp_protocol_info *proto,
                                const char *filename);

enum tftp_protocol_result
tftp_protocol_send_ack_wait_for_data (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_server_send_oack_wait_for_data(struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_server_send_oack_wait_for_ack (struct tftp_protocol_info *proto);

enum tftp_protocol_result
tftp_protocol_server_send_oack_no_wait (struct tftp_protocol_info *proto);

void tftp_protocol_send_error_packet (struct tftp_protocol_info *proto,
                                      enum tftp_pkt_error_code error_code,
                                      const char *error_msg);

void tftp_protocol_send_errno_as_error_packet(struct tftp_protocol_info *proto,
                                               int32 error_value);


typedef int32 (*tftp_protocol_send_data_cb_type) (void *param,
               uint16 block_number, uint8 **data_buf);

enum tftp_protocol_result
tftp_protocol_send_data (struct tftp_protocol_info *proto,
                          const uint8 *data_buf, uint32 data_buf_size,
                          uint32 *sent_data_size,
                          tftp_protocol_send_data_cb_type cb_func,
                          void *cb_func_param);

typedef int32 (*tftp_protocol_recv_data_cb_type) (void *param,
               uint16 block_number, uint8 *data_buf, uint32 data_buf_size);

enum tftp_protocol_result
tftp_protocol_recv_data (struct tftp_protocol_info *proto,
                          uint8 *data_buf, uint32 data_buf_size,
                          uint32 *recd_data_buf_size,
                          tftp_protocol_recv_data_cb_type cb_func,
                          tftp_protocol_recv_data_cb_type close_cb_func,
                          void *cb_func_param);


#endif /* not __TFTP_PROTOCOL_H__ */
