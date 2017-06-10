/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  log.h : Interface to functions to log output messages.
 *
 */
#ifndef LOG_H
#define LOG_H

#include "common.h"

#if defined(WINDOWSPC)
#include <time.h>
#endif
/* definitions for Android logging */
#if defined(LINUXPC) || defined(WINDOWSPC)
#define LOGE printf
#define LOGI printf
#else
#define LOG_TAG "mdmfiletransfer"
#include "cutils/log.h"
#endif

/*macro for logging */
#if defined(WINDOWSPC)
#define logmsg(log_level, fmt, ...) log_message (log_level, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#else
#define logmsg(log_level, fmt ...) log_message (log_level, __FUNCTION__, __LINE__, fmt)
#endif

enum log_level {
    LOG_ERROR,
    LOG_STATUS,
    LOG_WARN,
    LOG_EVENT,
    LOG_INFO
};

extern boolean verbose;

void log_message (enum log_level level, const char *function, int line_number, const char *format, ...);

#endif
