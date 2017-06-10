/*
 * Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef RADISH_H_INCLUDED
#define RADISH_H_INCLUDED

#define LOG_TAG "radish"
#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#ifndef RADISH_OFFLINE
#include <cutils/log.h>
#include "common_log.h"
#endif

#define RADISH_VERSION "1.1"
#define RAD_DBG 0

#define RAD_LOGI(...) \
  if (RAD_DBG) \
    LOGI(__VA_ARGS__)
#define RAD_LOGV(...) \
  if (RAD_DBG) \
    LOGV(__VA_ARGS__)

#ifndef FEATURE_DATA_LINUX_LE
#include "list.h"
#endif

#define IOV_OWNER(the_iovec) OWNER(struct buffer_item,iov,the_iovec)

#define PREFIX(list) (LIST_OWNER(list, PrefixItem, the_list))
#define IFACECFG(list) (LIST_OWNER(list, IfaceCfg, the_list))
#define BUFF_ITM(list) (LIST_OWNER(list, struct buffer_item, the_list))

#define RADISH_LOG_FUNC_ENTRY RAD_LOGI("%s>", __func__)
#define RADISH_LOG_FUNC_EXIT RAD_LOGI("%s<", __func__)

#define RADISH_ERROR    -1
#define RADISH_SUCCESS   0

#ifdef RADISH_OFFLINE
#include <stdio.h>
#include <signal.h>
#include <time.h>

#ifdef RADISH_LOG_QXDM
#include "ds_util.h"

#undef  DS_LOG_TAG
#define DS_LOG_TAG "RADISH-LIB"

#define  LOGE           ds_log_err
#define  LOGD           ds_log_high
#define  LOGI           ds_log_med
#define  LOGV           ds_log_low

#else
#define LOGD(...) \
  do \
  { \
  fprintf(stdout, "DEBUG:"); \
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  } while(0)

#define LOGE(...) \
  do \
  { \
  fprintf(stderr, "ERROR:"); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  } while(0)

#define LOGI(...)  \
  do \
  { \
  fprintf(stdout, "INFO:"); \
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  } while(0)

#define LOGV(...)  \
  do \
  { \
  fprintf(stdout, "VERBOSE:"); \
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  } while(0)
#endif /* RADISH_LOG_QXDM */

#define strlcpy strncpy
#endif

#endif /* RADISH_H_INCLUDED */
