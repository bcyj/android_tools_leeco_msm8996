/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_LOG_H
#define CPP_LOG_H

#include "camera_dbg.h"

#define CPP_LOG_SILENT     0
#define CPP_LOG_NORMAL     1
#define CPP_LOG_DEBUG      2
#define CPP_LOG_VERBOSE    3

/* ------------- change this macro to change the logging level -------------*/

#define CPP_LOG_LEVEL      0

/* -------------- change this macro to enable profiling logs  --------------*/
#define CPP_DEBUG_PROFILE  0

/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define CPP_DEBUG_MUTEX    0

/* -------------------------------------------------------------------------*/
volatile extern uint32_t gCamCppLogLevel;

#ifdef CDBG
#undef CDBG
#define CDBG(...) do{} while(0)
#endif
#define CDBG(fmt, args...) ALOGD_IF(gCamCppLogLevel >= 2, fmt, ##args)

#ifdef CDBG_LOW
#undef CDBG_LOW
#endif //#ifdef CDBG_LOW
#define CDBG_LOW(fmt, args...) ALOGD_IF(gCamCppLogLevel >= 3, fmt, ##args)

#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif //#ifdef CDBG_HIGH
#define CDBG_HIGH(fmt, args...) ALOGD_IF(gCamCppLogLevel >= 1, fmt, ##args)

/* macro for profiling logs */
#undef CDBG_PROFILE
#if (CPP_DEBUG_PROFILE == 1)
  #define CDBG_PROFILE CDBG_HIGH
#else
  #define CDBG_PROFILE(fmt, args...) do {} while(0)
#endif

#undef PTHREAD_MUTEX_LOCK
#undef PTHREAD_MUTEX_UNLOCK

#if (CPP_DEBUG_MUTEX == 1)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    CDBG_HIGH("%s:%d [cpp_mutex_log] before pthread_mutex_lock(%p)\n", \
      __func__, __LINE__, m); \
    pthread_mutex_lock(m); \
    CDBG_HIGH("%s:%d [cpp_mutex_log] after pthread_mutex_lock(%p)\n", \
      __func__, __LINE__, m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    CDBG_HIGH("%s:%d [cpp_mutex_log] before pthread_mutex_unlock(%p)\n", \
      __func__, __LINE__, m); \
    pthread_mutex_unlock(m); \
    CDBG_HIGH("%s:%d [cpp_mutex_log] after pthread_mutex_unlock(%p)\n", \
      __func__, __LINE__, m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif
#endif
