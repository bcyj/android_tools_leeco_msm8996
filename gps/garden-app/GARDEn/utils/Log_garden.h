/* Copyright (c) 2011-2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef _LIBS_UTILS_LOG_H
#define _LIBS_UTILS_LOG_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <log_util.h>
#include <string.h>

#define TS_PRINTF(format, x...)                                \
{                                                              \
  struct timeval tv;                                           \
  struct timezone tz;                                          \
  int hh, mm, ss;                                              \
  gettimeofday(&tv, &tz);                                      \
  hh = tv.tv_sec/3600%24;                                      \
  mm = (tv.tv_sec%3600)/60;                                    \
  ss = tv.tv_sec%60;                                           \
  fprintf(stdout,"%02d:%02d:%02d.%06ld]" format "\n", hh, mm, ss, tv.tv_usec,##x);    \
}

#ifdef GARDEN_DEBUG

#define ALOGE(format, x...) TS_PRINTF("%s: %d] " format , __func__, __LINE__, ##x)
#define ALOGW(format, x...) TS_PRINTF("%s: %d] " format , __func__, __LINE__, ##x)
#define ALOGI(format, x...) TS_PRINTF("%s: %d] " format , __func__, __LINE__, ##x)
#define ALOGD(format, x...) TS_PRINTF("%s: %d] " format , __func__, __LINE__, ##x)
#define ALOGV(format, x...) TS_PRINTF("%s: %d] " format , __func__, __LINE__, ##x)

#else  // GARDEN_DEBUG

#define ALOGE(format, x...) TS_PRINTF("E/:(%d) " format , getpid(),  ##x)
#define ALOGW(format, x...) TS_PRINTF("W/:(%d) " format , getpid(),  ##x)
#define ALOGI(format, x...) TS_PRINTF("I/:(%d) " format , getpid(),  ##x)
#define ALOGD(format, x...) TS_PRINTF("D/:(%d) " format , getpid(),  ##x)
#define ALOGV(format, x...) TS_PRINTF("V/:(%d) " format , getpid(),  ##x)

#endif // GARDEN_DEBUG


#endif // _LIBS_UTILS_LOG_H
