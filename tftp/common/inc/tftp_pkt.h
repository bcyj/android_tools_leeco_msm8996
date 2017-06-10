/***********************************************************************
 * tftp_pkt.h
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
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-26   rp    Add tftp-client module.
2013-12-06   rp    Improve OPACK logic.
2013-11-24   rp    Support TFTP extension options.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_PKT_H__
#define __TFTP_PKT_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

#define TFTP_OPCODE_FIELD_SIZE_IN_BYTES         2
#define TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES    2
#define TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES      2

enum tftp_pkt_opcode_type
{
  TFTP_PKT_OPCODE_TYPE_UNKNOWN = 0,
  TFTP_PKT_OPCODE_TYPE_RRQ     = 1,
  TFTP_PKT_OPCODE_TYPE_WRQ     = 2,
  TFTP_PKT_OPCODE_TYPE_DATA    = 3,
  TFTP_PKT_OPCODE_TYPE_ACK     = 4,
  TFTP_PKT_OPCODE_TYPE_ERROR   = 5,
  TFTP_PKT_OPCODE_TYPE_OACK    = 6,
};

enum tftp_pkt_option_id_type
{
  TFTP_PKT_OPTION_ID_BLOCK_SIZE = 0,
  TFTP_PKT_OPTION_ID_TIMEOUT_IN_SECS,
  TFTP_PKT_OPTION_ID_TSIZE,
  TFTP_PKT_OPTION_ID_SEEK,
  TFTP_PKT_OPTION_ID_APPEND,
  TFTP_PKT_OPTION_ID_UNLINK,
  TFTP_PKT_OPTION_ID_WINDOW_SIZE,
  TFTP_PKT_OPTION_ID_TIMEOUT_IN_MS,
  TFTP_PKT_OPTION_ID_MAX
};

struct tftp_pkt_option_item_type
{
  enum tftp_pkt_option_id_type option_id;
  int is_valid;
  uint32 option_value;
};

struct tftp_pkt_options_type
{
  struct tftp_pkt_option_item_type options[TFTP_PKT_OPTION_ID_MAX];
  uint32 extended_options_count;
  int are_all_options_valid;
};

struct tftp_pkt_req_pkt_fields
{
  const char *filename;
  const char *mode;

  struct tftp_pkt_options_type options;
};

struct tftp_pkt_data_pkt_fields
{
  uint16 block_number;
  uint8 *data;
  uint32 data_size;
};

struct tftp_pkt_ack_pkt_fields
{
  uint16 block_number;
};

struct tftp_pkt_error_pkt_fields
{
  uint16 error_code;
  const char *error_string;
};

struct tftp_pkt_oack_pkt_fields
{
  struct tftp_pkt_options_type options;
};


struct tftp_pkt_type
{
  uint8  raw_pkt_buff[TFTP_MAX_TFTP_PACKET_SIZE];
  uint32 raw_pkt_buff_data_size;

  int pkt_processed;
  enum tftp_pkt_opcode_type opcode;

  union
  {
    struct tftp_pkt_req_pkt_fields   req_pkt;
    struct tftp_pkt_data_pkt_fields  data_pkt;
    struct tftp_pkt_ack_pkt_fields   ack_pkt;
    struct tftp_pkt_error_pkt_fields error_pkt;
    struct tftp_pkt_oack_pkt_fields  oack_pkt;
  } pkt_fields;
};

enum tftp_pkt_error_code
{
  TFTP_PKT_ERROR_CODE_NOT_DEFINED             = 0,
  TFTP_PKT_ERROR_CODE_FILE_NOT_FOUND          = 1,
  TFTP_PKT_ERROR_CODE_ACCESS_VIOLATION        = 2,
  TFTP_PKT_ERROR_CODE_DISK_FULL               = 3,
  TFTP_PKT_ERROR_CODE_ILLEGAL_FTP_OPERATION   = 4,
  TFTP_PKT_ERROR_CODE_UNKNOWN_TRANSFER_ID     = 5,
  TFTP_PKT_ERROR_CODE_FILE_ALREADY_EXISTS     = 6,
  TFTP_PKT_ERROR_CODE_NO_SUCH_USER            = 7,
  TFTP_PKT_ERROR_CODE_INVALID_OPTIONS         = 8,

  TFTP_PKT_ERROR_CODE_END_OF_TRANSFER         = 9, //TODO: set to larger?
};


int tftp_pkt_interpret (struct tftp_pkt_type *tftp_pkt);
int tftp_form_data_packet (struct tftp_pkt_type *tftp_pkt,
                           uint16 block_number_h);
int tftp_form_error_packet (struct tftp_pkt_type *tftp_pkt,
                            uint16 error_code_h, const char *error_msg);
int tftp_form_ack_packet (struct tftp_pkt_type *tftp_pkt,
                          uint16 block_number_h);


int tftp_form_rrq_packet (struct tftp_pkt_type *tftp_pkt,
                           const char *filename,
                           struct tftp_pkt_options_type *options);


int tftp_form_wrq_packet (struct tftp_pkt_type *tftp_pkt,
                           const char *filename,
                           struct tftp_pkt_options_type *options);

int tftp_form_oack_packet (struct tftp_pkt_type *tftp_pkt,
                            struct tftp_pkt_options_type *options);


#endif /* __TFTP_PKT_H__ */
