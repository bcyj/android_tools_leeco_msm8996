/******************************************************************************
  @file    client.c
  @brief   Android performance PerfLock library

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOG_TAG         "ANDR-PERF"
#include <cutils/log.h>
#include <cutils/properties.h>

#include "client.h"
#include "mp-ctl.h"

#if defined(ANDROID_JELLYBEAN)
#include "common_log.h"
#endif

#if QC_DEBUG
#  define QLOGE(...)    LOGE(__VA_ARGS__)
#  define QLOGW(...)    LOGW(__VA_ARGS__)
#  define QLOGI(...)    LOGI(__VA_ARGS__)
#  define QLOGV(...)    LOGV(__VA_ARGS__)
#else
#  define QLOGE(...)
#  define QLOGW(...)
#  define QLOGI(...)
#  define QLOGV(...)
#endif

#define FAILED                  -1
#define SUCCESS                 0

int perf_lock_acq(int handle, int duration, int list[], int numArgs)
{
    int rc = -1;

    if (duration < 0)
        return FAILED;

    if (numArgs > OPTIMIZATIONS_MAX) {
        numArgs = OPTIMIZATIONS_MAX;
    }

    rc = mpctl_send(MPCTL_CMD_PERFLOCKACQ, handle, duration, numArgs, list);
    return rc;
}

int perf_lock_rel(int handle)
{
    int rc = -1;

    if (handle == 0)
        return FAILED;

    rc = mpctl_send(MPCTL_CMD_PERFLOCKREL, handle);

    if(rc > 0)
        return SUCCESS;

    return FAILED;
}

void perf_lock_reset(void)
{
    mpctl_send(MPCTL_CMD_PERFLOCKRESET);
}
