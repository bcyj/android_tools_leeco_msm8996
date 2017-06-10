/***********************************************************************
 * tftp_protocol.c
 *
 * The module that implements the TFTP protocol to send and receive data.
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2015-01-05   vm    Compiling server for TN Apps.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-09-19   dks   Add hooks to extract performance numbers.
2014-08-26   rp    Bring in changes from target-unit-testing.
2014-07-28   rp    Add debug variable to catch pkt-loss.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-20   dks   Fix compilation on windows builds.
2014-01-20   rp    Add support to do partial-file reads.
2013-12-26   rp    Add tftp-client module.
2013-12-05   rp    Add new debug-log interface.
2013-12-05   nr    Bug fixes to the send and receive buffer functions.
2013-11-27   nr    Add support to receive data.
2013-11-25   nr    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_protocol.h"
#include "tftp_assert.h"
#include "tftp_log.h"
#include "tftp_string.h"
#include "stdio.h"
#include "tftp_os.h"


volatile int tftp_crash_on_timeout = 0;


/*-----------------------------------------------------------------------------
 * TFTP protocol
 *
 * Read-Request
 *         Server                 Client
 *        -----------------------------------
 *                    <--           RRQ
 *           DATA-1   -->
 *
 * Read-Request with OACK
 *         Server                  Client
 *        -----------------------------------
 *                    <--          RRQ-EXTNS
 *           OACK     -->
 *                    <--          ACK-0
 *           DATA-1   -->
 *
 *=============================================================================
 *
 * Write-Request
 *         Server                 Client
 *        -----------------------------------
 *                    <--           WRQ
 *           ACK-0    -->
 *                    <--           DATA-1
 *
 * Write-Request with OACK
 *         Server                  Client
 *        -----------------------------------
 *                    <--          WRQ-EXTNS
 *           OACK     -->
 *                    <--          DATA-1
 *
-----------------------------------------------------------------------------*/

static enum tftp_protocol_result
tftp_protocol_map_con_res_to_prot_res (enum tftp_connection_result con_res)
{
  enum tftp_protocol_result prot_res = TFTP_PROTOCOL_INVALID;

  switch (con_res)
  {
    case TFTP_CONNECTION_RESULT_SUCCESS:
      prot_res = TFTP_PROTOCOL_SUCCESS;
      break;

    case TFTP_CONNECTION_RESULT_FAILED:
      prot_res = TFTP_PROTOCOL_CONN_ERROR;
      break;

    case TFTP_CONNECTION_RESULT_TIMEOUT:
      prot_res = TFTP_PROTOCOL_TIMEOUT;
      break;

    case TFTP_CONNECTION_RESULT_HANGUP:
      prot_res = TFTP_PROTOCOL_REMOTE_HANGUP;
      break;

    case TFTP_CONNECTION_RESULT_INVALID_ARGS:
      prot_res = TFTP_PROTOCOL_INVALID;
      break;

    default:
      TFTP_ASSERT (0);
      break;
  }

  return prot_res;
}

static enum tftp_protocol_result
tftp_protocol_map_tftp_err_to_proto_res (
     struct tftp_pkt_error_pkt_fields *error_pkt)
{
  enum tftp_protocol_result prot_res = TFTP_PROTOCOL_INVALID;

  TFTP_ASSERT (error_pkt != NULL);
  if (error_pkt == NULL)
  {
    goto End;
  }

  switch (error_pkt->error_code)
  {
    case TFTP_PKT_ERROR_CODE_NOT_DEFINED:
      prot_res = TFTP_PROTOCOL_UNDEFINED_ERROR;
      break;

    case TFTP_PKT_ERROR_CODE_FILE_NOT_FOUND:
      prot_res = TFTP_PROTOCOL_FILE_NOT_FOUND;
      break;

    case TFTP_PKT_ERROR_CODE_ACCESS_VIOLATION:
      prot_res = TFTP_PROTOCOL_ACCESS_VIOLATION;
      break;

    case TFTP_PKT_ERROR_CODE_DISK_FULL:
      prot_res = TFTP_PROTOCOL_DISK_FULL;
      break;

    case TFTP_PKT_ERROR_CODE_ILLEGAL_FTP_OPERATION:
      prot_res = TFTP_PROTOCOL_ILLEGAL_FTP_OPERATION;
      break;

    case TFTP_PKT_ERROR_CODE_UNKNOWN_TRANSFER_ID:
      prot_res = TFTP_PROTOCOL_UNKNOWN_TRANSFER_ID;
      break;

    case TFTP_PKT_ERROR_CODE_FILE_ALREADY_EXISTS:
      prot_res = TFTP_PROTOCOL_FILE_ALREADY_EXISTS;
      break;

    case TFTP_PKT_ERROR_CODE_NO_SUCH_USER:
      prot_res = TFTP_PROTOCOL_NO_SUCH_USER;
      break;

    case TFTP_PKT_ERROR_CODE_INVALID_OPTIONS:
      prot_res = TFTP_PROTOCOL_INVALID_OPTIONS;
      break;

    case TFTP_PKT_ERROR_CODE_END_OF_TRANSFER:
      prot_res = TFTP_PROTOCOL_EOT;
      break;

    default:
      prot_res = TFTP_PROTOCOL_INVALID;
      break;
  }

 End:
  return prot_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_open_connection (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    prot_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  proto->current_timeout_ms = proto->timeout_in_ms;

  con_res = tftp_connection_open (&proto->connection_hdl, &proto->remote_addr,
                                  proto->current_timeout_ms);
  if (con_res == TFTP_CONNECTION_RESULT_SUCCESS)
  {
    proto->is_connection_open = 1;
  }

  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);
End:
  return prot_res;
}

/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_close_connection (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    prot_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  if (!proto->is_connection_open)
  {
    prot_res = TFTP_PROTOCOL_SUCCESS;
    goto End;
  }

  con_res = tftp_connection_close (&proto->connection_hdl);
  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);

End:
  return prot_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_update_remote_addr (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    prot_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  //todo: validate the server address
  con_res = tftp_connection_switch_to_new_remote_addr (&proto->connection_hdl);
  TFTP_ASSERT (con_res == TFTP_CONNECTION_RESULT_SUCCESS);
  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);

 End:
  return prot_res;
}

enum tftp_protocol_result
tftp_protocol_connect_remote_addr (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    prot_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  con_res = tftp_connection_connect (&proto->connection_hdl);
  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);

 End:
  return prot_res;
}


/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_send_pkt (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;
  struct tftp_pkt_type *tx_pkt;
  uint32 sent_buff_size;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    prot_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;

  sent_buff_size = 0;
  con_res = tftp_connection_send_data (&proto->connection_hdl,
                  tx_pkt->raw_pkt_buff, tx_pkt->raw_pkt_buff_data_size,
                  &sent_buff_size);
  if (con_res == TFTP_CONNECTION_RESULT_SUCCESS)
  {
    if (sent_buff_size != tx_pkt->raw_pkt_buff_data_size)
    {
      con_res = TFTP_CONNECTION_RESULT_FAILED;
    }
  }

  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);

 End:
  return prot_res;
}

/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_recv_pkt (struct tftp_protocol_info *proto)
{
  enum tftp_connection_result con_res;
  enum tftp_protocol_result prot_res;
  struct tftp_pkt_type *rx_pkt;

  rx_pkt = &proto->rx_pkt;

  memset (rx_pkt, 0, sizeof (*rx_pkt));

  con_res = tftp_connection_get_data (&proto->connection_hdl,
                  rx_pkt->raw_pkt_buff, sizeof (rx_pkt->raw_pkt_buff),
                  &rx_pkt->raw_pkt_buff_data_size);

  prot_res = tftp_protocol_map_con_res_to_prot_res (con_res);
  return prot_res;
}

static void
tftp_protocol_wait_pkt_begin (struct tftp_protocol_info *proto)
{
  proto->begin_wait_pkt_timestamp = tftp_timetick_get();
  proto->remaining_timeout_ms = proto->current_timeout_ms;
}

static int
tftp_protocol_wait_pkt_is_timeout (struct tftp_protocol_info *proto)
{
  uint64 elapsed_time_us, elapsed_time_ms;
  int is_timeout = 0;

  proto->current_wait_pkt_timestamp = tftp_timetick_get();

  elapsed_time_us =
              tftp_timetick_diff_in_usec (proto->begin_wait_pkt_timestamp,
                                          proto->current_wait_pkt_timestamp);

  /* Add 1 ms to the time that has elapsed so that
     every retry would reduce the timeout. */
  elapsed_time_ms = (elapsed_time_us / 1000) + 1;

  if (elapsed_time_ms < proto->remaining_timeout_ms)
  {
    proto->remaining_timeout_ms -= elapsed_time_ms;
  }
  else
  {
    proto->remaining_timeout_ms = 0;
  }

  if (proto->remaining_timeout_ms == 0)
  {
    is_timeout = 1;
  }

  return is_timeout;
}

/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_wait_for_pkt_helper (struct tftp_protocol_info *proto,
                                    enum tftp_pkt_opcode_type pkt_type_1,
                                    enum tftp_pkt_opcode_type pkt_type_2,
                                    uint16 block_num_for_pkt)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;
  enum tftp_protocol_result timeout_proto_res = TFTP_PROTOCOL_INVALID;
  enum tftp_connection_result con_res;
  int is_timeout = 0;

  struct tftp_pkt_type *rx_pkt;
  int result;
  uint32 rx_pkt_count;


  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tftp_protocol_wait_pkt_begin (proto);
  rx_pkt = &proto->rx_pkt;

  con_res = tftp_connection_set_timeout (&proto->connection_hdl,
                                          proto->remaining_timeout_ms);
  timeout_proto_res = tftp_protocol_map_con_res_to_prot_res (con_res);

  if (timeout_proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    TFTP_LOG_ERR ("Couldn't set new timeout to %u",
                   proto->remaining_timeout_ms);
    proto_res = timeout_proto_res;
    goto End;
  }

  for (rx_pkt_count = 0; rx_pkt_count < proto->max_wrong_pkt_count;
       ++rx_pkt_count)
  {
    proto_res = tftp_protocol_recv_pkt (proto);

    is_timeout = tftp_protocol_wait_pkt_is_timeout (proto);

    if ((proto_res != TFTP_PROTOCOL_SUCCESS) &&
        (proto_res != TFTP_PROTOCOL_TIMEOUT))
    {
      break;
    }

    if (proto_res == TFTP_PROTOCOL_TIMEOUT)
    {
      if ((is_timeout == 1) || (proto->remaining_timeout_ms == 0))
      {
        break;
      }

      /* Set timeout to remaining time. */
      con_res = tftp_connection_set_timeout (&proto->connection_hdl,
                                              proto->remaining_timeout_ms);
      timeout_proto_res = tftp_protocol_map_con_res_to_prot_res (con_res);

      if (timeout_proto_res != TFTP_PROTOCOL_SUCCESS)
      {
        TFTP_LOG_ERR ("Couldn't set new timeout to %u",
                       proto->remaining_timeout_ms);
        proto_res = timeout_proto_res;
        goto End;
      }
      continue;
    }

    // todo: ID the sender

    result = tftp_pkt_interpret (rx_pkt);
    if (result != 0)
    {
      continue;
    }

    if (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR)
    {
      struct tftp_pkt_error_pkt_fields *err_pkt;

      err_pkt = &rx_pkt->pkt_fields.error_pkt;
      if (err_pkt->error_code == TFTP_PKT_ERROR_CODE_END_OF_TRANSFER)
      {
        proto_res = TFTP_PROTOCOL_EOT;
      }
      else
      {
        proto_res = TFTP_PROTOCOL_ERROR_PKT_RECD;
      }
      break;
    }

    proto_res = TFTP_PROTOCOL_WRONG_PACKET;

    if ((rx_pkt->opcode == pkt_type_1) || (rx_pkt->opcode == pkt_type_2))
    {
      switch (rx_pkt->opcode)
      {
        case TFTP_PKT_OPCODE_TYPE_ACK:
        {
          struct tftp_pkt_ack_pkt_fields *ack_pkt;
          ack_pkt = &rx_pkt->pkt_fields.ack_pkt;
          if (ack_pkt->block_number == block_num_for_pkt)
          {
            proto_res = TFTP_PROTOCOL_SUCCESS;
          }
          else
          {
            TFTP_LOG_ERR ("Received wrong ACK.. expecting %u got %u",
                          block_num_for_pkt, ack_pkt->block_number);
          }
        }
        break;

        case TFTP_PKT_OPCODE_TYPE_DATA:
        {
          struct tftp_pkt_data_pkt_fields *data_pkt;
          data_pkt = &rx_pkt->pkt_fields.data_pkt;
          if (data_pkt->block_number == block_num_for_pkt)
          {
            proto_res = TFTP_PROTOCOL_SUCCESS;
          }
          else
          {
            TFTP_LOG_ERR ("Received wrong DATA.. expecting %u got %u",
                          block_num_for_pkt, data_pkt->block_number);
          }
        }
        break;

        case TFTP_PKT_OPCODE_TYPE_OACK:
        {
          proto_res = TFTP_PROTOCOL_SUCCESS;
        }
        break;

        default:
        {
          proto_res = TFTP_PROTOCOL_WRONG_PACKET;
        }
        break;
      }
    }
    else
    {
      TFTP_LOG_ERR ("Received unexpected packet type.");
    }

    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }

     /* We got a wrong packet. */
    /* Set timeout to 1ms to help clear the pending queue. */
      con_res = tftp_connection_set_timeout (&proto->connection_hdl, 1);
      timeout_proto_res = tftp_protocol_map_con_res_to_prot_res (con_res);

      if (timeout_proto_res != TFTP_PROTOCOL_SUCCESS)
      {
        TFTP_LOG_ERR ("Couldn't set new timeout to %u", 1);
        proto_res = timeout_proto_res;
        goto End;
      }
  }

 End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_wait_for_data_helper (struct tftp_protocol_info *proto,
                                     uint16 block_number)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;
  struct tftp_pkt_type *rx_pkt;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  rx_pkt = &proto->rx_pkt;

  for (proto->retry_count = 0;
       proto->retry_count < proto->max_pkt_retry_count;
       proto->retry_count++)
  {
    proto_res = tftp_protocol_send_pkt (proto);
    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      goto End;
    }

    proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                     TFTP_PKT_OPCODE_TYPE_DATA,
                     TFTP_PKT_OPCODE_TYPE_UNKNOWN, block_number);
    switch (proto_res)
    {
      case TFTP_PROTOCOL_SUCCESS:
      {
        struct tftp_pkt_data_pkt_fields *data_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_DATA);
        data_pkt = &rx_pkt->pkt_fields.data_pkt;
        TFTP_ASSERT (data_pkt->block_number == block_number);
      }
      break;

      case TFTP_PROTOCOL_ERROR_PKT_RECD:
      case TFTP_PROTOCOL_EOT:
      {
        struct tftp_pkt_error_pkt_fields *error_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
        error_pkt = &rx_pkt->pkt_fields.error_pkt;
        TFTP_LOG_ERR ("Recd error-pkt. Code = %d, Msg = %s",
                      error_pkt->error_code, error_pkt->error_string);
        goto End;
      }

      //TODO: Handle errors from connection_get_data as exit condition instead
      //      of as a timeout.
      case TFTP_PROTOCOL_TIMEOUT:
      case TFTP_PROTOCOL_WRONG_PACKET:
      default:
      {
        if (proto_res == TFTP_PROTOCOL_TIMEOUT)
        {
          TFTP_LOG_ERR ("Timeout while wating for data packet block=%u",
                        block_number);
          ++proto->debug_info.total_timeouts;
          if (tftp_crash_on_timeout)
          {
            TFTP_LOG_ERR ("Timeout : %d", proto_res);
            TFTP_ASSERT (proto_res != TFTP_PROTOCOL_TIMEOUT);
          }
        }
        else
        {
          ++proto->debug_info.total_wrong_pkts;
        }
      }
      break;
    }

    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }
  }

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_wait_for_ack_helper (struct tftp_protocol_info *proto,
                                    uint16 block_number)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_SUCCESS;
  struct tftp_pkt_type *rx_pkt;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  rx_pkt = &proto->rx_pkt;

  for (proto->retry_count = 0;
       proto->retry_count < proto->max_pkt_retry_count;
       proto->retry_count++)
  {
    proto_res = tftp_protocol_send_pkt (proto);
    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      goto End;
    }

    proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                     TFTP_PKT_OPCODE_TYPE_ACK,
                     TFTP_PKT_OPCODE_TYPE_UNKNOWN, block_number);
    switch (proto_res)
    {
      case TFTP_PROTOCOL_SUCCESS:
      {
        struct tftp_pkt_ack_pkt_fields *ack_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ACK);
        ack_pkt = &rx_pkt->pkt_fields.ack_pkt;
        TFTP_ASSERT (ack_pkt->block_number == block_number);
      }
      break;

      case TFTP_PROTOCOL_EOT:
      {
        struct tftp_pkt_error_pkt_fields *error_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
        error_pkt = &rx_pkt->pkt_fields.error_pkt;
        TFTP_LOG_DBG ("Recd END OF TRANSFER pkt. Code = %d, Msg = %s",
                      error_pkt->error_code, error_pkt->error_string);
        goto End;
      }

      case TFTP_PROTOCOL_ERROR_PKT_RECD:
      {
        struct tftp_pkt_error_pkt_fields *error_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
        error_pkt = &rx_pkt->pkt_fields.error_pkt;
        TFTP_LOG_ERR ("Recd error-pkt. Code = %d, Msg = %s",
                      error_pkt->error_code, error_pkt->error_string);
        goto End;
      }

      case TFTP_PROTOCOL_TIMEOUT:
      case TFTP_PROTOCOL_WRONG_PACKET:
      default:
      {
        if (proto_res == TFTP_PROTOCOL_TIMEOUT)
        {
          TFTP_LOG_ERR ("Timeout while RRQ");
          ++proto->debug_info.total_timeouts;
          if (tftp_crash_on_timeout)
          {
            TFTP_LOG_ERR ("Timeout : %d", proto_res);
            TFTP_ASSERT (proto_res != TFTP_PROTOCOL_TIMEOUT);
          }
        }
        else
        {
          ++proto->debug_info.total_wrong_pkts;
        }
      }
      break;
    }

    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }
  }

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_send_wrq_helper (struct tftp_protocol_info *proto,
                                const char *filename)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;
  struct tftp_pkt_type *tx_pkt, *rx_pkt;
  int result;

  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (filename != NULL);
  if ((proto == NULL) ||
      (filename == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;
  rx_pkt = &proto->rx_pkt;

  result = tftp_form_wrq_packet (tx_pkt, filename, &proto->options);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    proto_res = TFTP_PROTOCOL_PKT_ERROR;
    goto End;
  }

  result = -1;
  for (proto->retry_count = 0;
       proto->retry_count < proto->max_pkt_retry_count;
       proto->retry_count++)
  {
    proto_res = tftp_protocol_send_pkt (proto);
    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      goto End;
    }

    proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                     TFTP_PKT_OPCODE_TYPE_ACK,
                     TFTP_PKT_OPCODE_TYPE_OACK, 0);
    switch (proto_res)
    {
      case TFTP_PROTOCOL_SUCCESS:
      {
        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ACK ||
                     rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_OACK);
      }
      break;

      case TFTP_PROTOCOL_ERROR_PKT_RECD:
      case TFTP_PROTOCOL_EOT:
      {
        struct tftp_pkt_error_pkt_fields *error_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
        error_pkt = &rx_pkt->pkt_fields.error_pkt;
        TFTP_LOG_ERR ("Recd error-pkt. Code = %d, Msg = %s",
                      error_pkt->error_code, error_pkt->error_string);
        proto_res = tftp_protocol_map_tftp_err_to_proto_res (error_pkt);
        goto End;
      }


      case TFTP_PROTOCOL_TIMEOUT:
      case TFTP_PROTOCOL_WRONG_PACKET:
      default:
      {
        if (proto_res == TFTP_PROTOCOL_TIMEOUT)
        {
          TFTP_LOG_ERR ("Timeout while WRQ");
          ++proto->debug_info.total_timeouts;
            if (tftp_crash_on_timeout)
            {
              TFTP_LOG_ERR ("Timeout : %d", proto_res);
              TFTP_ASSERT (proto_res != TFTP_PROTOCOL_TIMEOUT);
            }
        }
        else
        {
          ++proto->debug_info.total_wrong_pkts;
        }
      }
      break;
    }

    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }
  }

End:
  return proto_res;
}

/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_send_rrq_helper (struct tftp_protocol_info *proto,
                                const char *filename)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;
  struct tftp_pkt_type *tx_pkt, *rx_pkt;
  int result;

  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (filename != NULL);
  if ((proto == NULL) ||
      (filename == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;
  rx_pkt = &proto->rx_pkt;

  result = tftp_form_rrq_packet (tx_pkt, filename, &proto->options);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    proto_res = TFTP_PROTOCOL_PKT_ERROR;
    goto End;
  }

  for (proto->retry_count = 0;
       proto->retry_count < proto->max_pkt_retry_count;
       proto->retry_count++)
  {
    proto_res = tftp_protocol_send_pkt (proto);
    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      goto End;
    }

    proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                     TFTP_PKT_OPCODE_TYPE_DATA,
                     TFTP_PKT_OPCODE_TYPE_OACK, 1);
    switch (proto_res)
    {
      case TFTP_PROTOCOL_SUCCESS:
      {
        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_DATA||
                     rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_OACK);
      }
      break;

      case TFTP_PROTOCOL_ERROR_PKT_RECD:
      case TFTP_PROTOCOL_EOT:
      {
        struct tftp_pkt_error_pkt_fields *error_pkt;

        TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
        error_pkt = &rx_pkt->pkt_fields.error_pkt;
        TFTP_LOG_ERR ("Recd error-pkt. Code = %d, Msg = %s",
                      error_pkt->error_code, error_pkt->error_string);
        proto_res = tftp_protocol_map_tftp_err_to_proto_res (error_pkt);
        goto End;
      }

      case TFTP_PROTOCOL_REMOTE_HANGUP:
      case TFTP_PROTOCOL_TIMEOUT:
      case TFTP_PROTOCOL_WRONG_PACKET:
      default:
      {
        if (proto_res == TFTP_PROTOCOL_TIMEOUT)
        {
          TFTP_LOG_ERR ("Timeout while RRQ");
          ++proto->debug_info.total_timeouts;
          if (tftp_crash_on_timeout)
          {
            TFTP_LOG_ERR ("Timeout : %d", proto_res);
            TFTP_ASSERT (proto_res != TFTP_PROTOCOL_TIMEOUT);
          }
        }
        else if (proto_res == TFTP_PROTOCOL_REMOTE_HANGUP)
        {
          TFTP_LOG_ERR ("Remote host hangup while RRQ");
        }
        else
        {
          ++proto->debug_info.total_wrong_pkts;
        }
      }
      break;
    }

    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }
  }

End:
  return proto_res;
}

/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_server_form_oack_packet (struct tftp_protocol_info *proto)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;
  int result;
  struct tftp_pkt_type *tx_pkt;
  struct tftp_pkt_options_type *options;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  options = &proto->rx_pkt.pkt_fields.req_pkt.options;
  TFTP_ASSERT (options->are_all_options_valid == 1);

  /* Make sure options from RRQ are properly parsed and copied onto the
   * options-array in the protocol structure. */
  TFTP_ASSERT (proto->options.extended_options_count ==
               options->extended_options_count);
  TFTP_ASSERT (options->are_all_options_valid == 1);

  tx_pkt = &proto->tx_pkt;

  proto_res = TFTP_PROTOCOL_PKT_ERROR;

  result = tftp_form_oack_packet (tx_pkt, &proto->options);
  TFTP_ASSERT (result == 0);
  if (result == 0)
  {
    proto_res = TFTP_PROTOCOL_SUCCESS;
  }

 End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_server_send_oack_wait_for_ack (struct tftp_protocol_info *proto)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  proto_res = tftp_protocol_server_form_oack_packet (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    goto End;
  }

  proto_res = tftp_protocol_wait_for_ack_helper (proto, 0);

End:
  return proto_res;
}

enum tftp_protocol_result
tftp_protocol_server_send_oack_wait_for_data (struct tftp_protocol_info *proto)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  proto_res = tftp_protocol_server_form_oack_packet (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    goto End;
  }

  proto_res = tftp_protocol_wait_for_data_helper (proto, 1);

End:
  return proto_res;
}

enum tftp_protocol_result
tftp_protocol_server_send_oack_no_wait (struct tftp_protocol_info *proto)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_INVALID;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  proto_res = tftp_protocol_server_form_oack_packet (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    goto End;
  }

  proto_res = tftp_protocol_send_pkt (proto);

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_send_ack_wait_for_data (struct tftp_protocol_info *proto)
{
  enum tftp_protocol_result proto_res;
  struct tftp_pkt_type *tx_pkt;
  int result;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;

  result = tftp_form_ack_packet (tx_pkt, 0);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    proto_res = TFTP_PROTOCOL_PKT_ERROR;
    goto End;
  }

  proto_res = tftp_protocol_wait_for_data_helper (proto, 1);

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
void
tftp_protocol_send_error_packet (struct tftp_protocol_info *proto,
                                 enum tftp_pkt_error_code error_code,
                                 const char *error_msg)
{
  uint16 error_code_h;
  uint32 sent_buff_size;
  int result;
  struct tftp_pkt_type *tx_pkt;

  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (error_msg != NULL);
  if (proto == NULL)
  {
    TFTP_LOG_ERR ("Cant send error pkt: Proto is null");
    return;
  }

  if(error_msg == NULL)
  {
    error_msg = "Error message is NULL!";
  }

  tx_pkt = &proto->tx_pkt;

  error_code_h = (uint16)error_code;

  result = tftp_form_error_packet (tx_pkt, error_code_h, error_msg);
  TFTP_ASSERT (result == 0);

  if (result != 0)
  {
    TFTP_LOG_ERR ("Form error packet failed.\n");
    return;
  }

  (void) tftp_connection_send_data (&proto->connection_hdl,
               tx_pkt->raw_pkt_buff, tx_pkt->raw_pkt_buff_data_size,
               &sent_buff_size);

  if (error_code == TFTP_PKT_ERROR_CODE_END_OF_TRANSFER)
  {
    TFTP_LOG_DBG ("Sending End-Of-Transfer. Code = %d, Msg = %s",
                   error_code, error_msg);
  }
  else
  {
    TFTP_LOG_ERR ("sending error-pkt. Code = %d, Msg = %s",
                   error_code, error_msg);
  }
}

void
tftp_protocol_send_errno_as_error_packet (struct tftp_protocol_info *proto,
                                          int32 error_value)
{
  enum tftp_pkt_error_code error_code;
  char error_string[50];

  switch (error_value)
  {
    case ENOENT:
    {
      error_code = TFTP_PKT_ERROR_CODE_FILE_NOT_FOUND;
      break;
    }

    case EPERM:
    case EACCES:
    {
      error_code = TFTP_PKT_ERROR_CODE_ACCESS_VIOLATION;
      break;
    }

    case ENOSPC:
    {
      error_code = TFTP_PKT_ERROR_CODE_DISK_FULL;
      break;
    }

    case EEXIST:
    {
      error_code = TFTP_PKT_ERROR_CODE_FILE_ALREADY_EXISTS;
      break;
    }

    default:
    {
      error_code = TFTP_PKT_ERROR_CODE_NOT_DEFINED;
      break;
    }
  }

  snprintf (error_string, sizeof(error_string), "Err=%d String=%s",
            (int)error_value, strerror(error_value));
  tftp_protocol_send_error_packet (proto, error_code, error_string);
}

/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_send_data_pkt (struct tftp_protocol_info *proto,
                             uint16 block_number, const uint8 *data_buf,
                             uint32 data_buf_size)
{
  enum tftp_protocol_result proto_res;
  struct tftp_pkt_type *tx_pkt;
  struct tftp_pkt_data_pkt_fields *data_pkt;
  int result;

  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (data_buf != NULL);
  if ((proto == NULL) ||
      (data_buf == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;
  data_pkt = &tx_pkt->pkt_fields.data_pkt;

  result = tftp_form_data_packet (tx_pkt, block_number);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    proto_res = TFTP_PROTOCOL_PKT_ERROR;
    goto End;
  }

  tftp_memscpy (data_pkt->data, data_pkt->data_size, data_buf, data_buf_size);
  tx_pkt->raw_pkt_buff_data_size += data_buf_size;

  proto_res = tftp_protocol_send_pkt (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    goto End;
  }

End:
  return proto_res;
}

/*---------------------------------------------------------------------------*/
static enum tftp_protocol_result
tftp_protocol_send_ack_pkt (struct tftp_protocol_info *proto,
                             uint16 block_number, int is_last_ack)
{
  enum tftp_protocol_result proto_res;
  enum tftp_connection_result con_res;
  struct tftp_pkt_type *tx_pkt;
  int result;
  uint32 new_timeout;

  TFTP_ASSERT (proto != NULL);
  if (proto == NULL)
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  tx_pkt = &proto->tx_pkt;

  /* Ideally only the server should care about waiting during a WRQ.
   * There is no need for a client to wait during an RRQ.
   * So the server would initialize the last_ack_timeout_ms with a reasonable
   * value and the client can set this toi zero unless a wait is really
   * required.
   */
  new_timeout = proto->last_ack_timeout_ms;

  if ((is_last_ack == 1) && (new_timeout > 0))
  {
    proto->current_timeout_ms = new_timeout;
    con_res = tftp_connection_set_timeout (&proto->connection_hdl,
                                            new_timeout);
    proto_res = tftp_protocol_map_con_res_to_prot_res (con_res);

    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      TFTP_LOG_ERR ("Couldn't set new timeout to %u", new_timeout);
    }
  }

  result = tftp_form_ack_packet (tx_pkt, block_number);
  TFTP_ASSERT (result == 0);
  if (result != 0)
  {
    proto_res = TFTP_PROTOCOL_PKT_ERROR;
    goto End;
  }

  proto_res = tftp_protocol_send_pkt (proto);
  if (proto_res != TFTP_PROTOCOL_SUCCESS)
  {
    TFTP_LOG_ERR ("Sending ACK Pkt failed!!\n");
    goto End;
  }

  if (is_last_ack == 1)
  {
    if(new_timeout == 0)
    {
      TFTP_LOG_DBG ("Sent last ACK: *NOT* waiting on reply.\n");
      goto End;
    }

    TFTP_LOG_DBG ("Sent last ACK: *Waiting* on reply.\n");
    proto_res = tftp_protocol_recv_pkt (proto);
    if (proto_res == TFTP_PROTOCOL_SUCCESS)
    {
      TFTP_LOG_DBG ("Received data resending last ACK. Pkt Loss ..resend!\n");
      (void) tftp_protocol_send_pkt (proto);
    }
    else
    {
      TFTP_LOG_DBG ("Success! No timeout detected on last ACK\n");
    }
    proto_res = TFTP_PROTOCOL_SUCCESS;
  }

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_send_data (struct tftp_protocol_info *proto,
                          const uint8 *data_buf, uint32 data_buf_size,
                          uint32 *sent_data_size,
                          tftp_protocol_send_data_cb_type cb_func,
                          void *cb_func_param)
{
  enum tftp_protocol_result proto_res;
  int run_tx_loop;
  uint32 retry_count;
  uint16 cur_block_number;
  const uint8 *cur_data_buf;
  uint8 *cb_data_buf;
  uint32 cur_window_size, window_tx_size;
  uint32 cur_data_buf_size, cur_buf_posn, pending_bytes;
  struct tftp_pkt_type *rx_pkt;

  proto_res = TFTP_PROTOCOL_INVALID;
  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (sent_data_size != NULL);
  if ((proto == NULL) || (sent_data_size == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  TFTP_ASSERT (data_buf != NULL || cb_func != NULL);
  if ((data_buf == NULL) &&
      (cb_func == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }
  if (data_buf == NULL)
  {
    TFTP_ASSERT (cb_func != NULL);
  }
  if (cb_func == NULL)
  {
    TFTP_ASSERT (data_buf != NULL);
  }

  rx_pkt = &proto->rx_pkt;

  proto->curr_block_number = 0;
  proto->debug_info.total_blocks = 0;
  proto->debug_info.total_bytes = 0;

  cur_buf_posn = 0;
  proto->final_pkt_sent = 0;
  window_tx_size = 0;
  cur_data_buf_size = 0;

  while (!proto->final_pkt_sent)
  {
    for (retry_count = 0; retry_count < proto->max_pkt_retry_count;
         ++retry_count)
    {
      cur_block_number = proto->curr_block_number;
      cur_window_size = 0;
      run_tx_loop = 1;
      if ((retry_count > 0) && (cb_func == NULL))
      {
        TFTP_ASSERT (cur_buf_posn >= window_tx_size);
        cur_buf_posn -= window_tx_size;
      }
      window_tx_size = 0;

      while ((cur_window_size < proto->window_size) || run_tx_loop)
      {
        ++cur_block_number;
        ++cur_window_size;

        cur_data_buf = NULL;
        cur_data_buf_size = 0;

        if (cb_func != NULL)
        {
          int32 result;
          result = cb_func (cb_func_param, cur_block_number,
                            &cb_data_buf);
          TFTP_DEBUG_ASSERT (result >= 0);
          TFTP_DEBUG_ASSERT ((uint32)result <= proto->block_size);
          if (result < 0)
          {
             tftp_protocol_send_errno_as_error_packet (proto, -result);
             proto_res = TFTP_PROTOCOL_UNDEFINED_ERROR;
             goto End;
          }
          cur_data_buf_size = result;
          cur_data_buf = cb_data_buf;
        }
        else
        {
          pending_bytes = data_buf_size - cur_buf_posn;
          cur_data_buf_size = (pending_bytes > proto->block_size) ?
                               proto->block_size : pending_bytes;
          cur_data_buf = &data_buf[cur_buf_posn];
          cur_buf_posn += cur_data_buf_size;
          TFTP_ASSERT (cur_buf_posn <= data_buf_size);
        }
        TFTP_ASSERT (cur_data_buf_size <= proto->block_size);
        TFTP_ASSERT (cur_data_buf != NULL);
        if (cur_data_buf == NULL)
        {
          proto_res = TFTP_PROTOCOL_INVALID;
          goto End;
        }

        ++proto->debug_info.total_blocks;
        proto->debug_info.total_bytes += cur_data_buf_size;

        proto_res = tftp_protocol_send_data_pkt (proto, cur_block_number,
                                         cur_data_buf, cur_data_buf_size);
        if (proto_res != TFTP_PROTOCOL_SUCCESS)
        {
          goto End;
        }

        window_tx_size += cur_data_buf_size;

        if (cur_data_buf_size < proto->block_size)
        {
          proto->final_pkt_sent = 1;
          run_tx_loop = 0;
          break;
        }

        if ((proto->window_size > 0) &&
            (cur_window_size == proto->window_size))
        {
          run_tx_loop = 0;
          break;
        }
      }

      if (proto->window_size == 0)
      {
        TFTP_ASSERT (proto->final_pkt_sent == 1);
      }
      else
      {
        TFTP_ASSERT (cur_window_size <= proto->window_size);
      }

      /* TODO: why not wait_for_ack_helper ? */
      proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                       TFTP_PKT_OPCODE_TYPE_ACK,
                       TFTP_PKT_OPCODE_TYPE_UNKNOWN, cur_block_number);
      switch (proto_res)
      {
        case TFTP_PROTOCOL_SUCCESS:
        {
          struct tftp_pkt_ack_pkt_fields *ack_pkt;

          ack_pkt = &rx_pkt->pkt_fields.ack_pkt;
          TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ACK);
          TFTP_ASSERT (ack_pkt->block_number == cur_block_number);
          proto->curr_block_number = cur_block_number;
          *sent_data_size += window_tx_size;
        }
        break;

        case TFTP_PROTOCOL_EOT:
        {
          struct tftp_pkt_error_pkt_fields *error_pkt;

          TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
          error_pkt = &rx_pkt->pkt_fields.error_pkt;
          TFTP_LOG_DBG ("Recd END OF TRANSFER pkt Code=%d, Msg=%s",
                        error_pkt->error_code, error_pkt->error_string);

          proto->curr_block_number = cur_block_number;
          *sent_data_size += window_tx_size;
          proto_res = TFTP_PROTOCOL_SUCCESS;
          goto End;
        }

        case TFTP_PROTOCOL_ERROR_PKT_RECD:
        {
          struct tftp_pkt_error_pkt_fields *error_pkt;

          TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
          error_pkt = &rx_pkt->pkt_fields.error_pkt;
          TFTP_LOG_ERR ("Recd err-pkt. Code=%d, Msg=%s",
                        error_pkt->error_code, error_pkt->error_string);
          proto_res = tftp_protocol_map_tftp_err_to_proto_res (error_pkt);
          goto End;
        }

        //TODO: Handle errors from connection_send as exit condition instead of
        //      as a timeout.
        case TFTP_PROTOCOL_TIMEOUT:
        case TFTP_PROTOCOL_WRONG_PACKET:
        default:
        {
          if (proto_res == TFTP_PROTOCOL_TIMEOUT)
          {
            TFTP_LOG_ERR ("Timeout while processing REQ");
            ++proto->debug_info.total_timeouts;
            if (tftp_crash_on_timeout)
            {
              TFTP_LOG_ERR ("Timeout : %d", proto_res);
              TFTP_ASSERT (proto_res != TFTP_PROTOCOL_TIMEOUT);
            }
          }
          else
          {
            proto_res = TFTP_PROTOCOL_TIMEOUT;
            ++proto->debug_info.total_wrong_pkts;
          }
        }
        break;
      }

      if (proto_res == TFTP_PROTOCOL_SUCCESS)
      {
        break;
      }
    }

    if (retry_count >= proto->max_pkt_retry_count)
    {
      break;
    }
  }

End:
  return proto_res;
}


/*---------------------------------------------------------------------------*/
enum tftp_protocol_result
tftp_protocol_recv_data (struct tftp_protocol_info *proto,
                          uint8 *data_buf, uint32 data_buf_size,
                          uint32 *recd_data_buf_size,
                          tftp_protocol_recv_data_cb_type cb_func,
                          tftp_protocol_recv_data_cb_type close_cb_func,
                          void *cb_func_param)
{
  enum tftp_protocol_result proto_res = TFTP_PROTOCOL_SUCCESS;
  int run_rx_loop, skip_first_read, is_last_ask = 0;
  uint32 retry_count;
  uint16 cur_block_number;
  uint8 *cur_data_buf;
  uint32 cur_window_size;
  uint32 cur_data_buf_size, cur_buf_posn, remainder_buf_size;
  struct tftp_pkt_type *rx_pkt;
  struct tftp_pkt_data_pkt_fields *data_pkt;

  TFTP_ASSERT (proto != NULL);
  TFTP_ASSERT (cb_func != NULL || data_buf != NULL);
  TFTP_ASSERT (recd_data_buf_size != NULL);
  if ((proto == NULL) ||
      (recd_data_buf_size == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  if ((cb_func == NULL) &&
      (data_buf == NULL))
  {
    proto_res = TFTP_PROTOCOL_INVALID;
    goto End;
  }

  rx_pkt = &proto->rx_pkt;
  data_pkt = &rx_pkt->pkt_fields.data_pkt;

  TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_DATA);
  TFTP_ASSERT (data_pkt->block_number == 1);
  proto->debug_info.total_blocks = 0;
  proto->debug_info.total_bytes = 0;

  proto->curr_block_number = 0;
  skip_first_read = 1;

  cur_data_buf_size = 0;
  cur_data_buf = NULL;
  cur_buf_posn = 0;
  proto->final_pkt_received = 0;
  proto->insufficient_recv_buffer = 0;
  *recd_data_buf_size = 0;

  while (!proto->final_pkt_received && !proto->insufficient_recv_buffer)
  {
    for ( retry_count = 0; retry_count < proto->max_pkt_retry_count;
          ++retry_count)
    {
      cur_window_size = 0;
      cur_block_number = proto->curr_block_number;
      run_rx_loop = 1;

      while ((cur_window_size < proto->window_size) || run_rx_loop)
      {
        ++cur_block_number;
        ++cur_window_size;

        if (!skip_first_read)
        {
          proto_res = tftp_protocol_wait_for_pkt_helper (proto,
                            TFTP_PKT_OPCODE_TYPE_DATA,
                            TFTP_PKT_OPCODE_TYPE_UNKNOWN,
                            cur_block_number);
          if (proto_res != TFTP_PROTOCOL_SUCCESS)
          {
            break;
          }
        }
        skip_first_read = 0;

        TFTP_ASSERT (data_pkt->block_number == cur_block_number);
        cur_data_buf = data_pkt->data;
        cur_data_buf_size = data_pkt->data_size;
        ++proto->debug_info.total_blocks;
        proto->debug_info.total_bytes += cur_data_buf_size;

        if (cb_func != NULL)
        {
          int32 result;
          result = cb_func (cb_func_param, cur_block_number, cur_data_buf,
                            cur_data_buf_size);

          if (result < 0 || (uint32)result != cur_data_buf_size)
          {
            proto_res = TFTP_PROTOCOL_UNDEFINED_ERROR;
            if (result >= 0)
            {
              *recd_data_buf_size = cur_buf_posn + result;
              result = -(ENOSPC);
              proto_res = TFTP_PROTOCOL_ENOSPC;
            }
            tftp_protocol_send_errno_as_error_packet (proto, -result);
            goto End;
          }

          cur_buf_posn += cur_data_buf_size;
        }
        else
        {
          TFTP_ASSERT (cur_buf_posn <= data_buf_size);
          remainder_buf_size = data_buf_size - cur_buf_posn;
          if (cur_data_buf_size > remainder_buf_size)
          {
            proto->insufficient_recv_buffer = 1;
            cur_data_buf_size = remainder_buf_size;
          }
          tftp_memscpy (&data_buf[cur_buf_posn], data_buf_size - cur_buf_posn,
                        cur_data_buf, cur_data_buf_size);
          cur_buf_posn += cur_data_buf_size;
          TFTP_ASSERT (cur_buf_posn <= data_buf_size);
        }

        if (data_pkt->data_size < proto->block_size)
        {
          proto->final_pkt_received = 1;
          run_rx_loop = 0;
          break;
        }

        if (proto->insufficient_recv_buffer == 1)
        {
          run_rx_loop = 0;
          break;
        }

        if ((proto->window_size > 0) &&
            (proto->window_size == cur_window_size))
        {
          run_rx_loop = 0;
          break;
        }
      }


      if (proto_res == TFTP_PROTOCOL_TIMEOUT)
      {
         TFTP_LOG_ERR ("Timeout while processing REQ");
        ++proto->debug_info.total_timeouts;
      }
      else if (proto_res == TFTP_PROTOCOL_WRONG_PACKET)
      {
         TFTP_LOG_ERR ("Wrong Pkt while processing REQ");
        ++proto->debug_info.total_wrong_pkts;
      }

      if ((proto_res == TFTP_PROTOCOL_TIMEOUT) ||
          (proto_res == TFTP_PROTOCOL_WRONG_PACKET))
      {
        // send ack for previous window
        proto_res = tftp_protocol_send_ack_pkt (proto,proto->curr_block_number,
                                                is_last_ask);

        if (proto_res == TFTP_PROTOCOL_SUCCESS)
        {
          continue;
        }
        else
        {
          break;
        }
      }
      if (proto->final_pkt_received == 1)
      {
        if (close_cb_func != NULL)
        {
          int32 result;
          result = close_cb_func (cb_func_param, cur_block_number,cur_data_buf,
                                  cur_data_buf_size);

          if (result != 0)
          {
            proto_res = TFTP_PROTOCOL_UNDEFINED_ERROR;
            tftp_protocol_send_errno_as_error_packet (proto, -result);
            goto End;
          }
        }
        is_last_ask = 1;
      }

      if (proto->insufficient_recv_buffer == 1)
      {
        break;
      }

      // send ack for this window
      proto_res = tftp_protocol_send_ack_pkt (proto, cur_block_number,
                                              is_last_ask);
      if (is_last_ask == 1)
      {
        break;
      }

      if (proto_res == TFTP_PROTOCOL_SUCCESS)
      {
        proto->curr_block_number = cur_block_number;
      }
      break;
    }

    if (proto_res != TFTP_PROTOCOL_SUCCESS)
    {
      break;
    }
  }

  if (proto_res == TFTP_PROTOCOL_SUCCESS)
  {
    *recd_data_buf_size = cur_buf_posn;
    if (proto->insufficient_recv_buffer == 1)
    {
      proto_res = TFTP_PROTOCOL_INSUFFICIENT_BUFFER;
    }
  }
  else if (proto_res == TFTP_PROTOCOL_EOT)
  {
    struct tftp_pkt_error_pkt_fields *error_pkt;

    TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
    error_pkt = &rx_pkt->pkt_fields.error_pkt;
    TFTP_LOG_DBG ("Recd END OF TRANSFER pkt Code=%d, Msg=%s",
                  error_pkt->error_code, error_pkt->error_string);

    *recd_data_buf_size = cur_buf_posn;
    proto_res = TFTP_PROTOCOL_SUCCESS;
  }
  else if (proto_res == TFTP_PROTOCOL_ERROR_PKT_RECD)
  {
    struct tftp_pkt_error_pkt_fields *error_pkt;

    TFTP_ASSERT (rx_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
    error_pkt = &rx_pkt->pkt_fields.error_pkt;
    TFTP_LOG_ERR ("Recd err-pkt. Code=%d, Msg=%s",
                  error_pkt->error_code, error_pkt->error_string);
    proto_res = tftp_protocol_map_tftp_err_to_proto_res (error_pkt);
  }

End:
  return proto_res;
}
