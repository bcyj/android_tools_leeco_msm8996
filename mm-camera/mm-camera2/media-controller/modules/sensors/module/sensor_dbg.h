/* sensor_dbg.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_DBG_H__
#define __SENSOR_DBG_H__

#undef SLOG_HIGH
#undef SLOG_LOW
#undef SERR
#undef SLOW
#undef SHIGH

/* Enable SLOG_HIGH to print SERR and SHIGH */
//#define SLOG_HIGH

/* Enable SLOG_LOW to print all log levels - SERR, SLOW and SHIGH */
#define SLOG_LOW

/* ------- change this macro to enable mutex debugging for deadlock --------*/
#define SENSOR_DEBUG_MUTEX    0

#ifndef __cplusplus
uint32_t sensordebug_mask;
#endif

typedef enum {
  SENSOR_DEBUG_MASK_LOW  = (1 << 0),
  SENSOR_DEBUG_MASK_HIGH = (1 << 1),
  SENSOR_DEBUG_MASK_MAX  = 3,
}sensor_dbg_mask_type;


#if defined(SLOG_LOW)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-sensor"
    #include <utils/Log.h>
    #define SERR(fmt, args...) \
      ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
    #define SLOW(fmt, args...) \
      if (sensordebug_mask & SENSOR_DEBUG_MASK_LOW) \
        ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
    #define SHIGH(fmt, args...) \
      if (sensordebug_mask & SENSOR_DEBUG_MASK_HIGH) \
        ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #else
    #include <stdio.h>
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SHIGH(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#elif defined(SLOG_HIGH)
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-sensor"
    #include <utils/Log.h>
    #define SERR(fmt, args...) \
      ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) \
      ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
  #else
    #include <stdio.h>
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) fprintf(stderr, fmt, ##args)
  #endif
#else
  #ifdef _ANDROID_
    #undef LOG_NIDEBUG
    #undef LOG_TAG
    #define LOG_NIDEBUG 0
    #define LOG_TAG "mm-camera-sensor"
    #include <utils/Log.h>
    #define SERR(fmt, args...) \
      ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) do{}while(0)
  #else
    #include <stdio.h>
    #define SERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #define SLOW(fmt, args...) do{}while(0)
    #define SHIGH(fmt, args...) do{}while(0)
  #endif
#endif

#if (SENSOR_DEBUG_MUTEX == 1)
  #define PTHREAD_MUTEX_LOCK(m) do { \
    SHIGH("[sensor_mutex_log] before pthread_mutex_lock(%p)\n", m); \
    pthread_mutex_lock(m); \
    SHIGH("[sensor_mutex_log] after pthread_mutex_lock(%p)\n", m); \
  } while(0)

  #define PTHREAD_MUTEX_UNLOCK(m) do { \
    SHIGH("[sensor_mutex_log] before pthread_mutex_unlock(%p)\n", m); \
    pthread_mutex_unlock(m); \
    SHIGH("[sensor_mutex_log] after pthread_mutex_unlock(%p)\n", m); \
  } while(0)
#else
  #define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
  #define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif

#endif /* __SENSOR_DBG_H__ */
