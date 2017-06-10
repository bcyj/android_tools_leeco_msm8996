/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __GPSONE_DAEMON_DBG_H__
#define __GPSONE_DAEMON_DBG_H__

#include <stdio.h>

#define LOG_DEBUG

/* enable to debug call */
#define LOG_DEBUG_CALL
#define LOG_DEBUG_TIMESTAMP

/* This defined by makefile */
// #define _ANDROID_

#undef GPSONE_DMN_DBG
#undef GPSONE_DMN_PR_ERR

#ifdef LOG_DEBUG_TIMESTAMP
#include <sys/time.h>

#define TS_SPRINT_HHMMSS(str)                                  \
{                                                              \
  struct timeval tv;                                           \
  struct timezone tz;                                          \
  int hh, mm, ss;                                              \
  gettimeofday(&tv, &tz);                                      \
  hh = tv.tv_sec/3600%24;                                      \
  mm = (tv.tv_sec%3600)/60;                                    \
  ss = tv.tv_sec%60;                                           \
  sprintf(str, "%02d:%02d:%02d.%06ld ", hh, mm, ss, tv.tv_usec);    \
}

#define TS_PRINT_HHMMSS()                                      \
{                                                              \
  struct timeval tv;                                           \
  struct timezone tz;                                          \
  int hh, mm, ss;                                              \
  gettimeofday(&tv, &tz);                                      \
  hh = tv.tv_sec/3600%24;                                      \
  mm = (tv.tv_sec%3600)/60;                                    \
  ss = tv.tv_sec%60;                                           \
  fprintf(stderr, "%02d:%02d:%02d.%06ld ", hh, mm, ss, tv.tv_usec);    \
}
#define FPRINTF TS_PRINT_HHMMSS(); fprintf

#else

#define FPRINTF fprintf

#endif

#ifndef LOG_DEBUG

#ifdef _ANDROID_
#define LOG_NIDEBUG 0
#define LOG_TAG "gpsone_dmn"
#include <utils/Log.h>
#define GPSONE_DMN_DBG(fmt, args...) do{}while(0)
#define GPSONE_DMN_MSG(fmt, args...) do{}while(0)
#define GPSONE_DMN_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
#else /* _ANDROID_ */
#define GPSONE_DMN_DBG(fmt, args...) do{}while(0)
#define GPSONE_DMN_MSG(fmt, args...) do{}while(0)
#define GPSONE_DMN_PR_ERR(fmt, args...) FPRINTF(stderr, fmt, ##args)
#endif /* _ANDROID_ */

#else /* LOG_DEBUG */

#ifdef _ANDROID_
#define LOG_NIDEBUG 0
#define LOG_TAG "gpsone_dmn"
#include <utils/Log.h>
#ifdef LOG_DEBUG_CALL
#define GPSONE_DMN_DBG(fmt, args...) ALOGE(fmt, ##args)
#else
#define GPSONE_DMN_DBG(fmt, args...) do{}while(0)
#endif
#define GPSONE_DMN_MSG(fmt, args...) ALOGE(fmt, ##args)
#define GPSONE_DMN_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
#else /* _ANDROID_ */
#ifdef LOG_DEBUG_CALL
#define GPSONE_DMN_DBG(fmt, args...) FPRINTF(stderr, fmt, ##args)
#else
#define GPSONE_DMN_DBG(fmt, args...) do{}while(0)
#endif
#define GPSONE_DMN_MSG(fmt, args...) FPRINTF(stderr, fmt, ##args)
#define GPSONE_DMN_PR_ERR(fmt, args...) FPRINTF(stderr, fmt, ##args)
#endif /* _ANDROID_ */

#endif /* LOG_DEBUG */

#endif /* __GPSONE_DAEMON_DBG_H__ */
