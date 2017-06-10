/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdarg.h>
#include "utils/Log.h"

#define FAKE_BUF_SIZE 1024

int __android_log_print(int prio, const char *tag, const char *fmt, ...)
{
    va_list ap;
    char buf[FAKE_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, FAKE_BUF_SIZE, fmt, ap);
    va_end(ap);
    ALOGI("%s",buf);
}


void __android_log_assert(const char *cond, const char *tag,
                          const char *fmt, ...)
{
    va_list ap;
    char buf[FAKE_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, FAKE_BUF_SIZE, fmt, ap);
    va_end(ap);
    ALOGI("%s",buf);
}

