#ifndef QMI_CCI_TARGET_EXT_H
#define QMI_CCI_TARGET_EXT_H
/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sig_set;
    uint32_t timed_out;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} qmi_cci_os_signal_type;

typedef qmi_cci_os_signal_type qmi_client_os_params;

#define QMI_CCI_OS_SIGNAL qmi_cci_os_signal_type

#define QMI_CCI_OS_SIGNAL_INIT(ptr, os_params) \
  do { \
    (ptr)->sig_set = 0; \
    (ptr)->timed_out = 0; \
    pthread_cond_init(&(ptr)->cond, NULL); \
    pthread_mutex_init(&(ptr)->mutex, NULL); \
  } while(0)

#define QMI_CCI_OS_SIGNAL_DEINIT(ptr) \
  do {  \
    (ptr)->sig_set = 0; \
    (ptr)->timed_out = 0; \
    pthread_cond_destroy(&(ptr)->cond); \
    pthread_mutex_destroy(&(ptr)->mutex); \
  } while(0)

#define QMI_CCI_OS_EXT_SIGNAL_INIT(ptr, os_params) \
  do { \
    ptr = os_params; \
    QMI_CCI_OS_SIGNAL_INIT(ptr, NULL); \
  } while(0)

#define QMI_CCI_OS_SIGNAL_CLEAR(ptr) (ptr)->sig_set = 0
#define QMI_CCI_OS_SIGNAL_WAIT(ptr, timeout_ms) \
  do { \
    (ptr)->timed_out = 0; \
    if(timeout_ms) { \
      int rc; \
      struct timeval tv; \
      struct timespec ts; \
      gettimeofday(&tv, NULL); \
      ts.tv_sec = tv.tv_sec + timeout_ms / 1000; \
      ts.tv_nsec = tv.tv_usec * 1000 + (timeout_ms % 1000) * 1000 * 1000; \
      if (ts.tv_nsec >= 1000000000) \
      { \
        ts.tv_sec++; \
        ts.tv_nsec = (ts.tv_nsec % 1000000000); \
      } \
      pthread_mutex_lock(&(ptr)->mutex); \
      while(!(ptr)->sig_set) \
      { \
        rc = pthread_cond_timedwait(&(ptr)->cond, &(ptr)->mutex, &ts); \
        if(rc == ETIMEDOUT) \
        { \
          (ptr)->timed_out = 1; \
          break; \
        } \
      } \
      pthread_mutex_unlock(&(ptr)->mutex); \
    } else { \
      pthread_mutex_lock(&(ptr)->mutex); \
      while(!(ptr)->sig_set) \
        pthread_cond_wait(&(ptr)->cond, &(ptr)->mutex); \
      pthread_mutex_unlock(&(ptr)->mutex); \
    } \
  } while(0)
#define QMI_CCI_OS_SIGNAL_TIMED_OUT(ptr) (ptr)->timed_out
#define QMI_CCI_OS_SIGNAL_SET(ptr)  \
  do { \
    pthread_mutex_lock(&(ptr)->mutex); \
    (ptr)->sig_set = 1; \
    pthread_cond_signal(&(ptr)->cond); \
    pthread_mutex_unlock(&(ptr)->mutex); \
  } while(0)

#ifdef __cplusplus
}
#endif
#endif
