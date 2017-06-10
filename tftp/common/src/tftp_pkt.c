/***********************************************************************
 * tftp_pkt.c
 *
 * The TFTP Packet module.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP Packet module
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-08-26   rp    Bring in changes from target-unit-testing.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-26   rp    Add tftp-client module.
2013-12-06   rp    Improve OPACK logic.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-24   rp    Support TFTP extension options.
2013-11-21   nr    Abstract the OS layer across the OS's.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_pkt.h"
#include "tftp_assert.h"
#include "tftp_string.h"
#include "tftp_socket.h"
#include "tftp_utils.h"

#include <stdio.h>
#include <stdlib.h>


struct tftp_pkt_option_id_name_map_type
{
  enum tftp_pkt_option_id_type option_id;
  const char *option_name;
};


static struct tftp_pkt_option_id_name_map_type
              tftp_pkt_option_id_name_map[] =
{
  { TFTP_PKT_OPTION_ID_BLOCK_SIZE,        "blksize"},
  { TFTP_PKT_OPTION_ID_TIMEOUT_IN_SECS,   "timeout"  },
  { TFTP_PKT_OPTION_ID_TSIZE,             "tsize"},
  { TFTP_PKT_OPTION_ID_SEEK,              "seek"},
  { TFTP_PKT_OPTION_ID_APPEND,            "append"},
  { TFTP_PKT_OPTION_ID_UNLINK,            "unlink"},
  { TFTP_PKT_OPTION_ID_WINDOW_SIZE,       "wsize"},
  { TFTP_PKT_OPTION_ID_TIMEOUT_IN_MS,     "timeoutms"},
  { TFTP_PKT_OPTION_ID_MAX,               "null"},
};

static uint32 tftp_pkt_option_id_map_count =
                (sizeof (tftp_pkt_option_id_name_map) /
                 sizeof (tftp_pkt_option_id_name_map[0]));

static const char*
tftp_pkt_get_option_name_from_id (enum tftp_pkt_option_id_type option_id)
{
  const char *option_name = NULL;
  uint32 i;

  TFTP_ASSERT (option_id < TFTP_PKT_OPTION_ID_MAX);

  for ( i = 0; i < tftp_pkt_option_id_map_count; ++i )
  {
    if (tftp_pkt_option_id_name_map[i].option_id == option_id)
    {
      option_name = tftp_pkt_option_id_name_map[i].option_name;
      break;
    }
  }

  return option_name;
}

static enum tftp_pkt_option_id_type
tftp_pkt_get_option_id_from_name (const char *option_name)
{
  enum tftp_pkt_option_id_type option_id;
  uint32 i;
  unsigned int len1, len2;
  int result;

  option_id = TFTP_PKT_OPTION_ID_MAX;

  TFTP_ASSERT (option_name != NULL);
  if (option_name == NULL)
  {
    goto End;
  }

  len1 = strlen (option_name);

  for ( i = 0; i < tftp_pkt_option_id_map_count; ++i )
  {
    len2 = strlen (tftp_pkt_option_id_name_map[i].option_name);

    if (len1 == len2)
    {
      result = memcmp (tftp_pkt_option_id_name_map[i].option_name,
                       option_name, len1);
      if (result == 0)
      {
        option_id = tftp_pkt_option_id_name_map[i].option_id;
        break;
      }
    }
  }

 End:
  return option_id;
}

//todo : handle the debug-asserts disabled case.
static int
tftp_pkt_option_parse_helper (struct tftp_pkt_type *tftp_pkt,
                               uint32 start_idx)
{
  struct tftp_pkt_options_type *options;
  uint32 idx, option_val, option_idx, str_len;
  const char *option_name, *option_val_str;
  char *end_of_value;
  enum tftp_pkt_option_id_type option_id;
  int result = 0;

  if (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_RRQ ||
      tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_WRQ)
  {
    options = &(tftp_pkt->pkt_fields.req_pkt.options);
  }
  else if (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_OACK)
  {
    options = &(tftp_pkt->pkt_fields.oack_pkt.options);
  }
  else
  {
    TFTP_ASSERT (0); /* This opcode shouldnt be calling this function */
    return -1;
  }

  idx = start_idx;
  option_idx = 0;
  while (idx < tftp_pkt->raw_pkt_buff_data_size)
  {
    option_name = (const char *)&tftp_pkt->raw_pkt_buff[idx];
    str_len = strlen (option_name);

    idx += str_len;
    TFTP_DEBUG_ASSERT (tftp_pkt->raw_pkt_buff[idx] == 0);
    ++idx;

    option_id = tftp_pkt_get_option_id_from_name (option_name);

    /* The idx should still be within the buffer. */
    if (idx >= tftp_pkt->raw_pkt_buff_data_size)
    {
      result = -1;
      break;
    }

    option_val_str = (const char *)&tftp_pkt->raw_pkt_buff[idx];
    str_len = strlen (option_val_str);
    idx += str_len;
    TFTP_DEBUG_ASSERT (tftp_pkt->raw_pkt_buff[idx] == 0);
    ++idx;

    /* The idx would just cross the buffer for last option */
    if (idx > tftp_pkt->raw_pkt_buff_data_size)
    {
      result = -1;
      break;
    }

    if (option_id >= TFTP_PKT_OPTION_ID_MAX)
    {
      continue; /* Unsupported option so skip it. */
    }

    option_val = strtol (option_val_str, &end_of_value, 0);

    if (*end_of_value != '\0')
    {
      result = -1;
      break;
    }

    options->options[option_idx].option_id = option_id;
    options->options[option_idx].option_value = option_val;
    options->options[option_idx].is_valid = 1;
    ++option_idx;
  }

  options->extended_options_count = option_idx;

  if (result == 0)
  {
    options->are_all_options_valid = 1;
  }
  else
  {
    options->are_all_options_valid = 0;
  }

  return result;
}


static int
tftp_pkt_parse_req_pkt (struct tftp_pkt_type *tftp_pkt)
{
  uint32 idx;

  /* move after OPCODE field. */
  TFTP_DEBUG_ASSERT (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_RRQ ||
                     tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_WRQ);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  /* The idx should still be within the buffer. */
  if (idx >= tftp_pkt->raw_pkt_buff_data_size)
  {
    return -1;
  }

  /* get file name */
  tftp_pkt->pkt_fields.req_pkt.filename =
             (const char *)&tftp_pkt->raw_pkt_buff[idx];
  idx += strlen (tftp_pkt->pkt_fields.req_pkt.filename);
  TFTP_DEBUG_ASSERT (tftp_pkt->raw_pkt_buff[idx] == 0);
  ++idx;

 /* The idx should still be within the buffer. */
  if (idx >= tftp_pkt->raw_pkt_buff_data_size)
  {
    return -1;
  }

  /* get mode */
  tftp_pkt->pkt_fields.req_pkt.mode =(const char*)&tftp_pkt->raw_pkt_buff[idx];
  idx += strlen (tftp_pkt->pkt_fields.req_pkt.mode);
  TFTP_DEBUG_ASSERT (tftp_pkt->raw_pkt_buff[idx] == 0);
  ++idx;

  /* The idx should still be within the buffer. */
  if (idx > tftp_pkt->raw_pkt_buff_data_size)
  {
    return -1;
  }

  return tftp_pkt_option_parse_helper (tftp_pkt, idx);
}

static int
tftp_pkt_parse_data_pkt (struct tftp_pkt_type *tftp_pkt)
{
  uint32 idx;
  uint16 block_number_n, block_number_h;

  /* move after OPCODE field. */
  TFTP_DEBUG_ASSERT (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_DATA);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  /* get block-number */
  tftp_memscpy (&block_number_n, sizeof (block_number_n),
                &tftp_pkt->raw_pkt_buff[idx],
                TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
  block_number_h = tftp_ntohs (block_number_n);
  tftp_pkt->pkt_fields.data_pkt.block_number = block_number_h;
  idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;

  /* get data */
  TFTP_DEBUG_ASSERT (idx <= tftp_pkt->raw_pkt_buff_data_size);
  tftp_pkt->pkt_fields.data_pkt.data = &tftp_pkt->raw_pkt_buff[idx];
  tftp_pkt->pkt_fields.data_pkt.data_size =
                             tftp_pkt->raw_pkt_buff_data_size - idx;

  return 0;
}

static int
tftp_pkt_parse_ack_pkt (struct tftp_pkt_type *tftp_pkt)
{
  uint32 idx;
  uint16 block_number_n, block_number_h;

  /* move after OPCODE field. */
  TFTP_DEBUG_ASSERT (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ACK);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  /* get block-number */
  tftp_memscpy (&block_number_n, sizeof (block_number_n),
                &tftp_pkt->raw_pkt_buff[idx],
                TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
  block_number_h = tftp_ntohs (block_number_n);
  tftp_pkt->pkt_fields.ack_pkt.block_number = block_number_h;
  idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;

  return 0;
}

static int
tftp_pkt_parse_error_pkt (struct tftp_pkt_type *tftp_pkt)
{
  uint32 idx;
  uint16 error_code_n, error_code_h;

  /* move after OPCODE field. */
  TFTP_DEBUG_ASSERT (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_ERROR);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  /* get error code */
  tftp_memscpy (&error_code_n, sizeof (error_code_n),
                &tftp_pkt->raw_pkt_buff[idx],
                TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES);
  error_code_h = tftp_ntohs (error_code_n);
  tftp_pkt->pkt_fields.error_pkt.error_code = error_code_h;
  idx += TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES;

  /* get error string */
  tftp_pkt->pkt_fields.error_pkt.error_string =
                   (const char *)&tftp_pkt->raw_pkt_buff[idx];
  idx += strlen (tftp_pkt->pkt_fields.error_pkt.error_string);
  TFTP_DEBUG_ASSERT (tftp_pkt->raw_pkt_buff[idx] == 0);
  ++idx;

  return 0;
}

static int
tftp_pkt_parse_oack_pkt (struct tftp_pkt_type *tftp_pkt)
{
  uint32 idx;

  /* move after OPCODE field. */
  TFTP_DEBUG_ASSERT (tftp_pkt->opcode == TFTP_PKT_OPCODE_TYPE_OACK);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  /* The idx should still be within the buffer. */
  if (idx >= tftp_pkt->raw_pkt_buff_data_size)
  {
    return -1;
  }
  return tftp_pkt_option_parse_helper (tftp_pkt, idx);
}


int
tftp_pkt_interpret (struct tftp_pkt_type *tftp_pkt)
{
  uint16 opcode_n, opcode_h;
  int result;

  TFTP_DEBUG_ASSERT (tftp_pkt->pkt_processed == 0);
  tftp_pkt->pkt_processed = 1;

  /* Extract the OP-Code. */
  tftp_memscpy (&opcode_n, sizeof (opcode_n), &tftp_pkt->raw_pkt_buff[0],
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  opcode_h = tftp_ntohs(opcode_n);

  switch (opcode_h)
  {
    case TFTP_PKT_OPCODE_TYPE_RRQ:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_RRQ;
      result = tftp_pkt_parse_req_pkt (tftp_pkt);
      break;

    case TFTP_PKT_OPCODE_TYPE_WRQ:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_WRQ;
      result = tftp_pkt_parse_req_pkt (tftp_pkt);
      break;

    case TFTP_PKT_OPCODE_TYPE_DATA:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_DATA;
      result = tftp_pkt_parse_data_pkt (tftp_pkt);
      break;

    case TFTP_PKT_OPCODE_TYPE_ACK:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_ACK;
      result = tftp_pkt_parse_ack_pkt (tftp_pkt);
      break;

    case TFTP_PKT_OPCODE_TYPE_ERROR:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_ERROR;
      result = tftp_pkt_parse_error_pkt (tftp_pkt);
      break;

    case TFTP_PKT_OPCODE_TYPE_OACK:
      tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_OACK;
      result = tftp_pkt_parse_oack_pkt (tftp_pkt);
      break;

    default :
        result = -1;
  }

  return result;
}


int
tftp_form_data_packet (struct tftp_pkt_type *tftp_pkt, uint16 block_number_h)
{
  uint16 opcode_n, block_number_n;
  unsigned int idx, pkt_len;

  memset (tftp_pkt, 0, sizeof (struct tftp_pkt_type));

  idx = 0;
  pkt_len = sizeof (tftp_pkt->raw_pkt_buff);

  /* store DATA opcode. */
  opcode_n = tftp_htons (TFTP_PKT_OPCODE_TYPE_DATA);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &opcode_n,
          TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_DATA;

  /* store block-number. */
  block_number_n = tftp_htons (block_number_h);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &block_number_n,
      TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
  idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;
  tftp_pkt->pkt_fields.data_pkt.block_number = block_number_h;

  /* Hookup data-pointer to store data. */
  tftp_pkt->pkt_fields.data_pkt.data = &tftp_pkt->raw_pkt_buff[idx];
  tftp_pkt->pkt_fields.data_pkt.data_size = sizeof (tftp_pkt->raw_pkt_buff) -
                                            idx;

  tftp_pkt->raw_pkt_buff_data_size = idx;

  return 0;
}

int
tftp_form_error_packet (struct tftp_pkt_type *tftp_pkt,
                        uint16 error_code_h, const char *error_msg)
{
  uint16 opcode_n, error_code_n;
  unsigned int idx, error_msg_len, pkt_len;

  memset (tftp_pkt, 0, sizeof (struct tftp_pkt_type));

  idx = 0;
  pkt_len = sizeof (tftp_pkt->raw_pkt_buff);

  /* store ERROR opcode. */
  opcode_n = tftp_htons (TFTP_PKT_OPCODE_TYPE_ERROR);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &opcode_n,
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_ERROR;

  /* store error-code. */
  error_code_n = tftp_htons (error_code_h);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &error_code_n,
                TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES);
  idx += TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->pkt_fields.error_pkt.error_code = error_code_h;

  /* store error-message */
  error_msg_len = strlen (error_msg);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, error_msg,
                error_msg_len);
  tftp_pkt->pkt_fields.error_pkt.error_string =
                       (const char *)&tftp_pkt->raw_pkt_buff[idx];
  idx += error_msg_len;
  tftp_pkt->raw_pkt_buff[idx] = 0;
  ++idx;

  tftp_pkt->raw_pkt_buff_data_size = idx;

  return 0;
}

int
tftp_form_ack_packet (struct tftp_pkt_type *tftp_pkt, uint16 block_number_h)
{
  uint16 opcode_n, block_number_n;
  unsigned int idx, pkt_len;

  memset (tftp_pkt, 0, sizeof (struct tftp_pkt_type));

  idx = 0;
  pkt_len = sizeof (tftp_pkt->raw_pkt_buff);

  /* store DATA opcode. */
  opcode_n = tftp_htons (TFTP_PKT_OPCODE_TYPE_ACK);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &opcode_n,
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_ACK;

  /* store block-number. */
  block_number_n = tftp_htons (block_number_h);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &block_number_n,
                TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
  idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;
  tftp_pkt->pkt_fields.ack_pkt.block_number = block_number_h;

  tftp_pkt->raw_pkt_buff_data_size = idx;

  return 0;
}


static int
tftp_form_rrq_or_wrq_packet (struct tftp_pkt_type *tftp_pkt,
                              const char *filename,
                              enum tftp_pkt_opcode_type opcode,
                              struct tftp_pkt_options_type *options)
{
  uint16 opcode_h, opcode_n;
  unsigned int idx, str_len, remaining_buff_size;
  uint32 i, option_idx, pkt_len;
  int result = -1;
  enum tftp_pkt_option_id_type option_id;
  const char *option_name;
  struct tftp_pkt_req_pkt_fields *req_pkt;

  TFTP_ASSERT (opcode == TFTP_PKT_OPCODE_TYPE_RRQ ||
               opcode == TFTP_PKT_OPCODE_TYPE_WRQ);
  opcode_h = (uint16)opcode;

  memset (tftp_pkt, 0, sizeof (struct tftp_pkt_type));

  idx = 0;
  pkt_len = sizeof (tftp_pkt->raw_pkt_buff);

  /* store RRQ/WRQ opcode. */
  opcode_n = tftp_htons (opcode_h);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &opcode_n,
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->opcode = opcode;

  /* store filename */
  str_len = strlen (filename);
  tftp_memscpy(&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, filename, str_len);
  tftp_pkt->pkt_fields.req_pkt.filename =
                       (const char *)&tftp_pkt->raw_pkt_buff[idx];
  idx += str_len;
  tftp_pkt->raw_pkt_buff[idx] = 0;
  ++idx;

  /* store filemode */
  str_len = strlen ("octet");
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, "octet", str_len);
  tftp_pkt->pkt_fields.req_pkt.mode =
                       (const char *)&tftp_pkt->raw_pkt_buff[idx];
  idx += str_len;
  tftp_pkt->raw_pkt_buff[idx] = 0;
  ++idx;

  req_pkt = &tftp_pkt->pkt_fields.req_pkt;

  option_idx = 0;
  for ( i = 0; i < options->extended_options_count; ++i, ++option_idx)
  {
    option_id = options->options[i].option_id;
    TFTP_ASSERT (option_id < TFTP_PKT_OPTION_ID_MAX);
    option_name = tftp_pkt_get_option_name_from_id (option_id);
    if (option_name == NULL)
    {
      goto End;
    }

    /* store option name */
    str_len = strlen (option_name);
    tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, option_name,
                  str_len);
    req_pkt->options.options[option_idx].option_id = option_id;
    idx += str_len;
    tftp_pkt->raw_pkt_buff[idx] = 0;
    ++idx;

    /* store option value */
    remaining_buff_size = sizeof (tftp_pkt->raw_pkt_buff) - idx;

    str_len = snprintf ((char*)&tftp_pkt->raw_pkt_buff[idx],
                        remaining_buff_size, "%d",
                        (int)options->options[i].option_value);

    if (str_len >= remaining_buff_size)
    {
      goto End;
    }

    idx += str_len;
    tftp_pkt->raw_pkt_buff[idx] = 0;
    ++idx;

    req_pkt->options.options[option_idx].option_value =
        options->options[i].option_value;
  }

  req_pkt->options.extended_options_count = option_idx;
  req_pkt->options.are_all_options_valid = 1;
  // todo : handle are_all_options_valid properly

  tftp_pkt->raw_pkt_buff_data_size = idx;
  result = 0;

End:
  return result;
}

int tftp_form_rrq_packet (struct tftp_pkt_type *tftp_pkt,
                           const char *filename,
                           struct tftp_pkt_options_type *options)

{
  return tftp_form_rrq_or_wrq_packet (tftp_pkt, filename,
                                       TFTP_PKT_OPCODE_TYPE_RRQ,
                                       options);
}

int
tftp_form_wrq_packet (struct tftp_pkt_type *tftp_pkt,
                       const char *filename,
                       struct tftp_pkt_options_type *options)
{
  return tftp_form_rrq_or_wrq_packet (tftp_pkt, filename,
                                       TFTP_PKT_OPCODE_TYPE_WRQ,
                                       options);
}


int
tftp_form_oack_packet (struct tftp_pkt_type *tftp_pkt,
                        struct tftp_pkt_options_type *options)
{
  uint16 opcode_n;
  unsigned int idx, str_len, remaining_buff_size;
  uint32 i, option_idx, pkt_len;
  int result = -1;
  enum tftp_pkt_option_id_type option_id;
  const char *option_name;
  struct tftp_pkt_oack_pkt_fields *oack_pkt;

  TFTP_ASSERT (tftp_pkt != NULL);
  TFTP_ASSERT (options != NULL);
  if ((tftp_pkt == NULL) ||
      (options == NULL))
  {
    goto End;
  }

  TFTP_ASSERT (options->extended_options_count > 0);
  TFTP_ASSERT (options->are_all_options_valid == 1);

  memset (tftp_pkt, 0, sizeof (struct tftp_pkt_type));

  idx = 0;
  pkt_len = sizeof (tftp_pkt->raw_pkt_buff);

  /* store OACK opcode. */
  opcode_n = tftp_htons (TFTP_PKT_OPCODE_TYPE_OACK);
  tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, &opcode_n,
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;
  tftp_pkt->opcode = TFTP_PKT_OPCODE_TYPE_OACK;

  oack_pkt = &tftp_pkt->pkt_fields.oack_pkt;

  option_idx = 0;
  for ( i = 0; i < options->extended_options_count; ++i, ++option_idx )
  {
    option_id = options->options[i].option_id;
    TFTP_ASSERT (option_id < TFTP_PKT_OPTION_ID_MAX);
    option_name = tftp_pkt_get_option_name_from_id (option_id);
    if (option_name == NULL)
    {
      goto End;
    }

    /* store option name */
    str_len = strlen (option_name);
    tftp_memscpy (&tftp_pkt->raw_pkt_buff[idx], pkt_len - idx, option_name,
                  str_len);
    oack_pkt->options.options[option_idx].option_id = option_id;
    idx += str_len;
    tftp_pkt->raw_pkt_buff[idx] = 0;
    ++idx;

    /* store option value */
    oack_pkt->options.options[option_idx].option_value =
                                        options->options[i].option_value;
    remaining_buff_size = sizeof (tftp_pkt->raw_pkt_buff) - idx;
    str_len = snprintf ((char*)&tftp_pkt->raw_pkt_buff[idx],
                        remaining_buff_size, "%d",
                        (int)options->options[i].option_value);
    if (str_len >= remaining_buff_size)
    {
      goto End;
    }

    idx += str_len;
    tftp_pkt->raw_pkt_buff[idx] = 0;
    ++idx;
  }

  oack_pkt->options.extended_options_count = option_idx;

  tftp_pkt->raw_pkt_buff_data_size = idx;
  result = 0;

End:
  return result;
}

