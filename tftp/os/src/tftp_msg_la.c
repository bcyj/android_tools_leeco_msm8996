/***********************************************************************
 * tftp_msg_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-07-28   rp    Remove unused MAX_LOG_LEVEL.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-30   nr    Support connected sockets and muti-poll.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "tftp_msg.h"
#include "tftp_assert.h"

#include <unistd.h>
#include <sys/syscall.h>

#if !defined (TFTP_LA_BUILD)
  #error "This file is to be included only in LA builds."
#endif

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_NIDEBUG 0

#define LOG_TAG "tftp_server"

#ifdef TFTP_LE_BUILD_ONLY
  #include <sys/syslog.h>

  #define LOGI(...) syslog(LOG_NOTICE, "INFO:"__VA_ARGS__)
  #define LOGV(...) syslog (LOG_NOTICE,"VERB:" __VA_ARGS__)
  #define LOGD(...) syslog (LOG_DEBUG,"DBG:"__VA_ARGS__)
  #define LOGE(...) syslog (LOG_ERR,"ERR:"__VA_ARGS__)
  #define LOGW(...) syslog (LOG_WARNING,"WRN:"__VA_ARGS__)
#else
  #include "cutils/log.h"
  #include "common_log.h"
#endif /* TFTP_LE_BUILD_ONLY */

void
tftp_msg_init (void)
{
  #ifdef TFTP_LE_BUILD_ONLY
    openlog(LOG_TAG, LOG_PID, LOG_USER);
  #endif /* TFTP_LE_BUILD_ONLY */

  LOGI("Starting...\n");
}

void
tftp_msg_print (enum tftp_log_msg_level_type msg_level, const char *msg_str)
{
  pid_t mypid, mytid;

  mypid = syscall(SYS_getpid);
  mytid = syscall(SYS_gettid);

  switch (msg_level)
  {
    case TFTP_LOG_MSG_LEVEL_ERROR:
      LOGE ("pid=%u tid=%u tftp-server : %s", mypid, mytid, msg_str);
      break;

    case TFTP_LOG_MSG_LEVEL_WARN:
      LOGW ("pid=%u tid=%u tftp-server : %s", mypid, mytid, msg_str);
      break;

    case TFTP_LOG_MSG_LEVEL_DEBUG:
      LOGD ("pid=%u tid=%u tftp-server : %s", mypid, mytid, msg_str);
      break;

    case TFTP_LOG_MSG_LEVEL_INFO:
      LOGI ("pid=%u tid=%u tftp-server : %s", mypid, mytid, msg_str);
      break;

    default:
      TFTP_ASSERT (0);
      break;
  }
}
