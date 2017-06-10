/***********************************************************************
 * tftp_log.c
 *
 * The TFTP Log module.
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP Log module
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
2014-07-28   rp    Move log-buffer from global to stack.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-26   rp    Add tftp-client module.
2013-12-05   rp    Add new debug-log interface.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-24   rp    Support TFTP extension options.
2013-11-21   nr    Abstract the OS layer across the OS's.
2013-11-15   dks   Fix printf warnings.
2013-11-19   nr    Make string manipulation functions OS agnostic.
2013-11-14   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_log.h"
#include "tftp_assert.h"
#include "tftp_pkt.h"
#include "tftp_msg.h"
#include "tftp_socket.h"
#include "tftp_utils.h"
#include "tftp_string.h"
#include <stdarg.h>
#include <stdio.h>

static struct tftp_log_config_type  tftp_log_config_inst;

void
tftp_log_init (void)
{
  tftp_msg_init ();

  memset (&tftp_log_config_inst, 0, sizeof (tftp_log_config_inst));

#ifdef TFTP_LOG_CONFIG_ENABLE_ERR_LOGGING
  tftp_log_config_inst.enable_err_logging = 1;
#endif

#ifdef TFTP_LOG_CONFIG_ENABLE_WARN_LOGGING
  tftp_log_config_inst.enable_warn_logging = 1;
#endif

#ifdef TFTP_LOG_CONFIG_ENABLE_DEBUG_LOGGING
  tftp_log_config_inst.enable_debug_logging = 1;
#endif

#ifdef TFTP_LOG_CONFIG_ENABLE_INFO_LOGGING
  tftp_log_config_inst.enable_info_logging = 1;
#endif

#ifdef TFTP_LOG_CONFIG_ENABLE_LINE_NO_LOGGING
  tftp_log_config_inst.enable_line_no_logging = 1;
#endif

#ifdef TFTP_LOG_CONFIG_ENABLE_CONSOLE_LOGGING
  tftp_log_config_inst.enable_console_logging = 1;
#endif
}

static const char*
tftp_log_get_msg_level_string (enum tftp_log_msg_level_type msg_level)
{
  const char *msg_level_str = "??? :";

  switch (msg_level)
  {
    case TFTP_LOG_MSG_LEVEL_ERROR:
      msg_level_str = "ERR :";
      break;

    case TFTP_LOG_MSG_LEVEL_WARN:
      msg_level_str = "WARN :";
      break;

    case TFTP_LOG_MSG_LEVEL_DEBUG:
      msg_level_str = "DBG :";
      break;

    case TFTP_LOG_MSG_LEVEL_INFO:
      msg_level_str = "INF :";
      break;

    default:
      TFTP_ASSERT (0);
      break;
  }

  return msg_level_str;
}

char *tftp_locate_filename (const char *filepath)
{
  char *filename;

  if (filepath == NULL)
  {
    return NULL;
  }

  filename = strrchr (filepath, '/');
  if (filename == NULL)
  {
    filename = strrchr (filepath, '\\');
  }

  return filename;
}

void
tftp_log_msg (const char *filepath, uint32 line_no,
              enum tftp_log_msg_level_type msg_level,
              const char *msg_format, ...)
{
  va_list args_fmt;
  char line_buffer_array[TFTP_LOG_LINE_BUF_SIZE_IN_BYTES], *line_buffer;
  const char *msg_level_str;
  int line_buffer_size, str_len;

  line_buffer_size = sizeof (line_buffer_array);
  memset (line_buffer_array, 0, line_buffer_size);
  line_buffer = line_buffer_array;

  msg_level_str = tftp_log_get_msg_level_string (msg_level);
  tftp_strlcpy (line_buffer, msg_level_str, line_buffer_size);
  str_len = strlen (line_buffer);
  TFTP_ASSERT (line_buffer_size >= str_len);
  if (line_buffer_size < str_len)
  {
    return;
  }
  line_buffer_size -= str_len;
  line_buffer += str_len;

  /* If required, log the filename and line-number. */
  if (tftp_log_config_inst.enable_line_no_logging)
  {
    const char *filename = tftp_locate_filename (filepath);
    if (filename != NULL)
    {
      filename++;
    }
    else
    {
      filename = filepath;
    }

    snprintf (line_buffer, line_buffer_size, "[%s, %d] ",
              filename, (int)line_no);

    str_len = strlen (line_buffer);
    TFTP_ASSERT (line_buffer_size >= str_len);
    if (line_buffer_size < str_len)
    {
      return;
    }
    line_buffer_size -= str_len;
    line_buffer += str_len;
  }

  /* Now append the supplied message itself. */
  va_start (args_fmt, msg_format);
  vsnprintf (line_buffer, line_buffer_size - 1, msg_format, args_fmt);
  va_end (args_fmt);

  /* If required, log the message to console. */
  if (tftp_log_config_inst.enable_console_logging)
  {
    tftp_msg_print (msg_level, line_buffer_array);
  }
}

static void
tftp_log_append_string (char *buf, int buf_size, const char *new_str)
{
  TFTP_ASSERT (buf != NULL);
  TFTP_ASSERT (buf_size > 0);
  TFTP_ASSERT (new_str != NULL);

  if ((buf == NULL) ||
      (buf_size <= 0) ||
      (new_str == NULL))
  {
    return;
  }

  if ((new_str == NULL) || (buf == NULL) || (buf_size <= 0))
  {
    return;
  }

  tftp_strlcat (buf, new_str, buf_size);
}

static void
tftp_log_append_num (char *buf, int buf_size, uint32 num)
{
  char temp_str[20];

  TFTP_ASSERT (buf != NULL);
  TFTP_ASSERT (buf_size > 0);
  if ((buf == NULL) ||
      (buf_size <= 0))
  {
    return;
  }

  snprintf (temp_str, sizeof (temp_str), " %d", (int)num);

  tftp_log_append_string (buf, buf_size, temp_str);
}

//todo: remove asserts in this function
void
tftp_log_raw_tftp_buf (const char *filepath, uint32 line_no,
                       const uint8 *pkt_buf, uint32 pkt_buf_size,
                       const char *msg_format, ...)
{
  uint16 opcode_h, errcode_h, block_number_h;
  uint32 idx, i;
  uint32 str_len;
  const char *filename2, *str1, *str2;
  va_list args_fmt;
  char line_buf_array[TFTP_LOG_LINE_BUF_SIZE_IN_BYTES], *line_buf;
  uint32 line_buf_size;

  line_buf_size = sizeof (line_buf_array);
  memset (line_buf_array, 0, line_buf_size);
  line_buf = line_buf_array;

  /* If required, log the filename and line-number. */
  if (tftp_log_config_inst.enable_line_no_logging)
  {
    const char *filename = tftp_locate_filename (filepath);
    if (filename != NULL)
    {
      filename++;
    }
    else
    {
      filename = filepath;
    }

    snprintf (line_buf, line_buf_size, "[%s, %d] ", filename, (int)line_no);

    str_len = strlen (line_buf);
    TFTP_ASSERT (line_buf_size >= str_len);
    if (line_buf_size < str_len)
    {
      return;
    }
    line_buf_size -= str_len;
    line_buf += str_len;
  }

  if (msg_format != NULL)
  {
    va_start (args_fmt, msg_format);
    vsnprintf (line_buf, (line_buf_size - 1), msg_format, args_fmt);
    va_end (args_fmt);
  }

  line_buf_size = sizeof (line_buf_array);
  line_buf = line_buf_array;
  /* If required, log the message to console. */
  if (tftp_log_config_inst.enable_console_logging)
  {
    tftp_msg_print (TFTP_LOG_MSG_LEVEL_DEBUG, line_buf);
  }

  memset (line_buf_array, 0, line_buf_size);

  /* Extract the OP-Code. */
  tftp_memscpy (&opcode_h, sizeof (opcode_h), pkt_buf,
                TFTP_OPCODE_FIELD_SIZE_IN_BYTES);
  opcode_h = tftp_ntohs (opcode_h);
  idx = TFTP_OPCODE_FIELD_SIZE_IN_BYTES;

  switch (opcode_h)
  {
    case TFTP_PKT_OPCODE_TYPE_RRQ:
    case TFTP_PKT_OPCODE_TYPE_WRQ:
    {
      filename2 = (const char *)&pkt_buf[idx];
      str_len = strlen (filename2);
      idx += str_len;
      TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
      ++idx;

      str1 = (const char *)&pkt_buf[idx];
      str_len = strlen (str1);
      idx += str_len;
      TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
      ++idx;

      if (opcode_h == TFTP_PKT_OPCODE_TYPE_RRQ)
      {
        tftp_log_append_string (line_buf, line_buf_size, "RRQ pkt : ");
      }
      else
      {
        tftp_log_append_string (line_buf, line_buf_size, "WRQ pkt : ");
      }

      tftp_log_append_string (line_buf, line_buf_size, "File = ");
      tftp_log_append_string (line_buf, line_buf_size, filename2);
      tftp_log_append_string (line_buf, line_buf_size, " Mode = ");
      tftp_log_append_string (line_buf, line_buf_size, str1);

      while (idx < pkt_buf_size)
      {
        str1 = (const char *)&pkt_buf[idx];
        str_len = strlen (str1);
        idx += str_len;
        TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
        ++idx;

        str2 = (const char *)&pkt_buf[idx];
        str_len = strlen (str2);
        idx += str_len;
        TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
        ++idx;

        tftp_log_append_string (line_buf, line_buf_size, " ");
        tftp_log_append_string (line_buf, line_buf_size, str1);
        tftp_log_append_string (line_buf, line_buf_size, " = ");
        tftp_log_append_string (line_buf, line_buf_size, str2);
      }
      tftp_msg_print (TFTP_LOG_MSG_LEVEL_DEBUG, line_buf);
    }
    break;

    case TFTP_PKT_OPCODE_TYPE_DATA:
    {
      tftp_memscpy (&block_number_h, sizeof (block_number_h), &pkt_buf[idx],
                    TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
      block_number_h = tftp_ntohs (block_number_h);
      idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;
      tftp_log_append_string (line_buf, line_buf_size, "DATA pkt : Block = ");
      tftp_log_append_num (line_buf, line_buf_size, block_number_h);
      tftp_log_append_string (line_buf, line_buf_size, " PAYLOAD = ");

      if(pkt_buf_size > 4)
      {
        for ( i = 0; i < 10; ++i)
        {
          tftp_log_append_num (line_buf, line_buf_size, pkt_buf[idx]);
          ++idx;
          if (idx >= pkt_buf_size)
          {
            break;
          }
        }
      }
      else
      {
        tftp_log_append_string (line_buf, line_buf_size, " NO_PAYLOAD");
      }
      tftp_msg_print (TFTP_LOG_MSG_LEVEL_DEBUG, line_buf);
    }
    break;

    case TFTP_PKT_OPCODE_TYPE_ACK:
    {
      tftp_memscpy (&block_number_h, sizeof (block_number_h), &pkt_buf[idx],
                    TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES);
      block_number_h = tftp_ntohs (block_number_h);
      idx += TFTP_BLOCKNUMBER_FIELD_SIZE_IN_BYTES;
      tftp_log_append_string (line_buf, line_buf_size, "ACK pkt : Block = ");
      tftp_log_append_num (line_buf, line_buf_size, block_number_h);
      tftp_msg_print (TFTP_LOG_MSG_LEVEL_DEBUG, line_buf);
    }
    break;

    case TFTP_PKT_OPCODE_TYPE_ERROR:
    {
      tftp_memscpy (&errcode_h, sizeof (errcode_h), &pkt_buf[idx],
                    TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES);
      errcode_h = tftp_ntohs (errcode_h);
      idx += TFTP_ERRORCODE_FIELD_SIZE_IN_BYTES;
      TFTP_LOG_ERR ("ERROR pkt : Err-Code = [%d], Err-Msg = [%s]",
                    errcode_h, &pkt_buf[idx]);
    }
    break;

    case TFTP_PKT_OPCODE_TYPE_OACK:
    {
      tftp_log_append_string (line_buf, line_buf_size, "OACK pkt : ");

      while (idx < pkt_buf_size)
      {
        str1 = (const char *)&pkt_buf[idx];
        str_len = strlen (str1);
        idx += str_len;
        TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
        ++idx;

        str2 = (const char *)&pkt_buf[idx];
        str_len = strlen (str2);
        idx += str_len;
        TFTP_DEBUG_ASSERT (pkt_buf[idx] == 0);
        ++idx;

        tftp_log_append_string (line_buf, line_buf_size, " ");
        tftp_log_append_string (line_buf, line_buf_size, str1);
        tftp_log_append_string (line_buf, line_buf_size, " = ");
        tftp_log_append_string (line_buf, line_buf_size, str2);
      }
      tftp_msg_print (TFTP_LOG_MSG_LEVEL_DEBUG, line_buf);
    }
    break;

    default:
    {
      //todo : evaluate this
      //  TFTP_DEBUG_ASSERT (0);
      tftp_log_append_string (line_buf, line_buf_size, "Invalid opcode");
      tftp_log_append_num (line_buf, line_buf_size, opcode_h);
    }
    break;
  }
}
