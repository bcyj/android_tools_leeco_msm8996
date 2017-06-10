/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.   *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIDBG_H__
#define __QIDBG_H__

#include <stdio.h>

/** QIDBG_LOG_LEVEL:
 *
 *  defines the loglevel
 *  0 - Logs are disabled
 *  1 - only error logs are enabled
 *  2 - error logs and high priority logs are enabled
 *  3 - medium, high priority and error logs are enabled
 *  4 - all logs are enabled
 **/
#define QIDBG_LOG_LEVEL 2

#undef QIDBG
#if (QIDBG_LOG_LEVEL > 0)
    #ifdef _ANDROID_
        #undef LOG_NIDEBUG
        #undef LOG_TAG
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mm-still"
        #include <utils/Log.h>
        #define QIDBG(fmt, args...) ALOGE(fmt, ##args)
        #define QINDBG(fmt, args...) do{}while(0)
    #else
        #define QIDBG(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#else
    #define QIDBG(fmt, args...) do{}while(0)
    #define QINDBG(fmt, args...) do{}while(0)
#endif

#if (QIDBG_LOG_LEVEL >= 4)
  #define QIDBG_ERROR(...)  QIDBG(__VA_ARGS__)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QIDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QIDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 3)
  #define QIDBG_ERROR(...)  QIDBG(__VA_ARGS__)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QIDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 2)
  #define QIDBG_ERROR(...)  QIDBG(__VA_ARGS__)
  #define QIDBG_HIGH(...)   QIDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#elif (QIDBG_LOG_LEVEL == 1)
  #define QIDBG_ERROR(...)  QIDBG(__VA_ARGS__)
  #define QIDBG_HIGH(...)   QINDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#else
  #define QIDBG_ERROR(...)  QINDBG(__VA_ARGS__)
  #define QIDBG_HIGH(...)   QINDBG(__VA_ARGS__)
  #define QIDBG_MED(...)    QINDBG(__VA_ARGS__)
  #define QIDBG_LOW(...)    QINDBG(__VA_ARGS__)
#endif
#endif /* __QIDBG_H__ */
