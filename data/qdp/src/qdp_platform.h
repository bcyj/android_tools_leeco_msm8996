/*!
  @file
  qdp_platform.h

  @brief
  contains platform specific definitions for qdp (qualcomm
  data profiles) module.

*/

/*===========================================================================

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_data.c#17 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/27/10   js      created file

===========================================================================*/
#include <signal.h>

#ifdef FEATURE_DATA_LOG_QXDM
  #include "msg.h"
#endif

#include "ds_string.h"
#include "ds_sl_list.h"

#ifdef FEATURE_DATA_LOG_QXDM

#define QDP_MAX_DIAG_LOG_MSG_SIZE 512
extern void qdp_format_log_msg(char * buf, int buf_size, char * fmt, ...);
#define QDP_LOG_MSG_DIAG(...) \
  { \
    char buf[QDP_MAX_DIAG_LOG_MSG_SIZE]; \
    qdp_format_log_msg(buf, QDP_MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__); \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf); \
  }
#define QDP_LOG_DEBUG(fmt, ...) QDP_LOG_MSG_DIAG(fmt, __VA_ARGS__)
#define QDP_LOG_ERROR(fmt, ...) QDP_LOG_MSG_DIAG(fmt, __VA_ARGS__)
#define QDP_LOG_FATAL(fmt, ...) QDP_LOG_MSG_DIAG(fmt, __VA_ARGS__)
#define QDP_LOG_VERBOSE(fmt, ...) QDP_LOG_MSG_DIAG(fmt, __VA_ARGS__)
#define QDP_LOG_INFO(fmt, ...) QDP_LOG_MSG_DIAG(fmt, __VA_ARGS__)

#elif defined(FEATURE_DATA_LOG_STDERR)

  #define QDP_LOG_DEBUG(...)    fprintf(stderr, "%s %d:", __FILE__, __LINE__);\
                                fprintf(stderr, __VA_ARGS__)
  #define QDP_LOG_ERROR(...)    fprintf(stderr, "%s %d:", __FILE__, __LINE__);\
                                fprintf(stderr, __VA_ARGS__)
  #define QDP_LOG_FATAL(...)    fprintf(stderr, "%s %d:", __FILE__, __LINE__);\
                                fprintf(stderr, __VA_ARGS__)
  #define QDP_LOG_VERBOSE(...)  fprintf(stderr, "%s %d:", __FILE__, __LINE__);\
                                fprintf(stderr, __VA_ARGS__)
  #define QDP_LOG_INFO(...)     fprintf(stderr, "%s %d:", __FILE__, __LINE__);\
                                fprintf(stderr, __VA_ARGS__)
#else

  #define QDP_LOG_DEBUG(...)
  #define QDP_LOG_ERROR(...)
  #define QDP_LOG_FATAL(...)
  #define QDP_LOG_VERBOSE(...)
  #define QDP_LOG_INFO(...)

#endif

typedef __sighandler_t sighandler_t;

typedef struct qdp_sig_handler_s
{
  int sig;
  sighandler_t handler;
} qdp_sig_handler_t;

extern qdp_sig_handler_t qdp_sig_handler_tbl[];

/*===========================================================================
  FUNCTION:  qdp_signal_handler
===========================================================================*/
/*!
    @brief
    processes signals sent to this process. see full list of signals
    in qdp_init

    @return
    void
*/
/*=========================================================================*/
extern void qdp_signal_handler
(
  int sig
);

/*===========================================================================
  FUNCTION:  qdp_platform_init
===========================================================================*/
/*!
    @brief
    platform specific init

    @return
    void
*/
/*=========================================================================*/
void qdp_platform_init(void);
