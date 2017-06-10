/***********************************************************************
 * tftp_log.h
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
2014-01-05   rp    In logs do not print full-paths just file-name portion.
2014-07-28   rp    Move log-buffer from global to stack.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-05   rp    Add new debug-log interface.
2013-11-24   rp    Support TFTP extension options.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_LOG_H__
#define __TFTP_LOG_H__

#include "tftp_config_i.h"
#include "tftp_pkt.h"

/*----------------------------------------------------------------------------
  Log Configuration.
----------------------------------------------------------------------------*/
struct tftp_log_config_type
{
  int enable_err_logging;
  int enable_warn_logging;
  int enable_debug_logging;
  int enable_info_logging;

  int enable_line_no_logging;
  int enable_ip_addr_logging;

  int enable_console_logging;
};

void tftp_log_init (void);

/*----------------------------------------------------------------------------
 * Internal functions. Do not use them directly, Instead use the macros
 *--------------------------------------------------------------------------*/

enum tftp_log_msg_level_type
{
  TFTP_LOG_MSG_LEVEL_ERROR,
  TFTP_LOG_MSG_LEVEL_WARN,
  TFTP_LOG_MSG_LEVEL_DEBUG,
  TFTP_LOG_MSG_LEVEL_INFO,
};

void tftp_log_msg (const char *filename, uint32 line_no,
                   enum tftp_log_msg_level_type msg_level,
                   const char *msg_format, ...);


void tftp_log_raw_tftp_buf (const char *filename, uint32 line_no,
                            const uint8 *pkt_buf, uint32 pkt_buf_size,
                            const char *msg_format, ...);

/*----------------------------------------------------------------------------
 * Macros to log a Message.
 *--------------------------------------------------------------------------*/

/* Log a ERROR-message */
#define TFTP_LOG_ERR(msg_format, ...)                           \
  do                                                            \
  {                                                             \
    tftp_log_msg (TFTP_SOURCE_FILE_NAME, __LINE__,              \
                  TFTP_LOG_MSG_LEVEL_ERROR,                     \
                  msg_format, ##__VA_ARGS__);                   \
  } while (0)

/* Log a WARNING-message */
#define TFTP_LOG_WARN(msg_format, ...)                          \
  do                                                            \
  {                                                             \
    tftp_log_msg (TFTP_SOURCE_FILE_NAME, __LINE__,              \
                  TFTP_LOG_MSG_LEVEL_WARN,                      \
                  msg_format, ##__VA_ARGS__);                   \
  } while (0)


/* Log a DEBUG-message */
#define TFTP_LOG_DBG(msg_format, ...)                           \
  do                                                            \
  {                                                             \
    tftp_log_msg (TFTP_SOURCE_FILE_NAME, __LINE__,              \
                  TFTP_LOG_MSG_LEVEL_DEBUG,                     \
                  msg_format, ##__VA_ARGS__);                   \
  } while (0)

/* Log a INFO-message */
#define TFTP_LOG_INFO(msg_format, ...)                          \
  do                                                            \
  {                                                             \
    tftp_log_msg (TFTP_SOURCE_FILE_NAME, __LINE__,              \
                  TFTP_LOG_MSG_LEVEL_INFO,                      \
                  msg_format, ##__VA_ARGS__);                   \
  } while (0)


/*----------------------------------------------------------------------------
 * Macros to log a TFTP-Packets.
 *--------------------------------------------------------------------------*/

/* Log a raw-TFTP packet */
#define TFTP_LOG_RAW_TFTP_BUF(pkt_buf, pkt_buf_size,            \
                              msg_format, ...)                  \
  do                                                            \
  {                                                             \
    tftp_log_raw_tftp_buf (TFTP_SOURCE_FILE_NAME, __LINE__,     \
                           pkt_buf, pkt_buf_size,               \
                           msg_format, ##__VA_ARGS__);          \
  } while (0)

#endif /* __TFTP_LOG_H__ */
