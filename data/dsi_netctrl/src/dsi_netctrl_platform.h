/*!
  @file
  dsi_netctrl_plaform.h

  @brief
  contains platform specific definitions for dsi_netctrl module

*/

/*===========================================================================

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/19/10   js      created

===========================================================================*/
#ifndef _DSI_NETCTRL_PLATFORM_
#define _DSI_NETCTRL_PLATFORM_

#include <sys/types.h>
#include "ds_util.h"

//#define boolean int
#define DSI_FALSE 0
#define DSI_TRUE 1

#define DSI_INLINE inline

#ifdef FEATURE_QMI_TEST
#include "tf_log.h"
#define DSI_LOG_DEBUG   TF_MSG_INFO
#define DSI_LOG_ERROR   TF_MSG_ERROR
#define DSI_LOG_FATAL   TF_MSG_ERROR
#define DSI_LOG_VERBOSE TF_MSG_INFO
#define DSI_LOG_INFO    TF_MSG_INFO

#elif defined(FEATURE_DSI_TEST) || defined(FEATURE_DATA_LOG_STDERR)
#include <stdio.h>
#define DSI_LOG_DEBUG(...) \
  printf("%s:%d:%s(): ",__FILE__, __LINE__, __func__); \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_ERROR(...) \
  printf("%s:%d:%s(): ",__FILE__, __LINE__, __func__); \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_FATAL(...) \
  printf("%s:%d:%s(): ",__FILE__, __LINE__, __func__); \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_VERBOSE(...) \
  printf("%s:%d:%s(): ",__FILE__, __LINE__, __func__); \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_INFO(...) \
  printf("%s:%d:%s(): ",__FILE__, __LINE__, __func__); \
  printf(__VA_ARGS__); \
  printf("\n")
#else
#define DSI_MAX_DIAG_LOG_MSG_SIZE 512
#include "msg.h"
extern void dsi_format_log_msg(char * buf, int buf_size, char * fmt, ...);
#define DSI_LOG_MSG_DIAG_HIGH(...) \
  { \
    char buf[DSI_MAX_DIAG_LOG_MSG_SIZE]; \
    dsi_format_log_msg(buf, DSI_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__); \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf); \
  }
#define DSI_LOG_MSG_DIAG_VERBOSE(...) \
  { \
    char buf[DSI_MAX_DIAG_LOG_MSG_SIZE]; \
    dsi_format_log_msg(buf, DSI_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__); \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, "%s", buf); \
  }
#define DSI_LOG_MSG_DIAG_ERROR(...) \
  { \
    char buf[DSI_MAX_DIAG_LOG_MSG_SIZE]; \
    dsi_format_log_msg(buf, DSI_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__); \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, "%s", buf); \
  }
#define DSI_LOG_MSG_DIAG_FATAL(...) \
  { \
    char buf[DSI_MAX_DIAG_LOG_MSG_SIZE]; \
    dsi_format_log_msg(buf, DSI_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__); \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_FATAL, "%s", buf); \
  }
#define DSI_LOG_DEBUG(fmt, ...) \
  { \
  DSI_LOG_MSG_DIAG_HIGH(fmt, __VA_ARGS__); \
  DS_LOG_MULTICAST_HIGH(fmt, __VA_ARGS__); \
  }
#define DSI_LOG_ERROR(fmt, ...) \
  { \
  DSI_LOG_MSG_DIAG_ERROR(fmt, __VA_ARGS__); \
  DS_LOG_MULTICAST_ERR(fmt, __VA_ARGS__); \
  }
#define DSI_LOG_FATAL(fmt, ...) \
  { \
  DSI_LOG_MSG_DIAG_FATAL(fmt, __VA_ARGS__); \
  DS_LOG_MULTICAST_ERR(fmt, __VA_ARGS__); \
  }
#define DSI_LOG_VERBOSE(fmt, ...) \
  { \
  DSI_LOG_MSG_DIAG_VERBOSE(fmt, __VA_ARGS__); \
  DS_LOG_MULTICAST_LOW(fmt, __VA_ARGS__); \
  }
#define DSI_LOG_INFO(fmt, ...) \
  { \
  DSI_LOG_MSG_DIAG_HIGH(fmt, __VA_ARGS__); \
  DS_LOG_MULTICAST_MED(fmt, __VA_ARGS__); \
  }
#endif

void * dsi_malloc(size_t memsize);
void dsi_free(void * ptr);

#endif /* _DSI_NETCTRL_PLATFORM_ */
