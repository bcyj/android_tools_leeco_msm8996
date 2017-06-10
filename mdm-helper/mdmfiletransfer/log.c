/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  log.c : Implementations of functions to log output messages.
 *
 */
#include "log.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>

boolean verbose = FALSE;

void log_message (enum log_level level, const char *func_name, int line_number, const char *format, ...)
{
    va_list args;
    char *log_type;
    char log[1024];
    time_t current_time;
    struct tm *local_time;

    va_start (args, format);
    vsnprintf(log, sizeof(log), format, args);
    va_end (args);

    if ((level == LOG_ERROR) ||
        level == LOG_WARN ||
        level == LOG_EVENT ||
        level == LOG_STATUS ||
        verbose) {
        switch (level) {
            case LOG_ERROR:
                ALOGE("ERROR: function: %s:%d %s", func_name, line_number, log);
                break;

            case LOG_INFO:
                ALOGI("INFO: function: %s:%d %s", func_name, line_number, log);
                break;

            case LOG_WARN:
                ALOGI("WARNING: function: %s:%d %s", func_name, line_number, log);
                break;

            case LOG_EVENT:
                ALOGI("EVENT: %s", log);
                break;

            case LOG_STATUS:
                ALOGE("%s", log);
                break;
        }
    }
}
