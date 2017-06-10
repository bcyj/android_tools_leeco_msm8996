#ifndef QMI_CSI_TARGET_H
#define QMI_CSI_TARGET_H
/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <sys/select.h>

typedef pthread_mutex_t qmi_csi_lock_type;

//#define LOCK_INIT(ptr)

#define LOCK_INIT(ptr) \
  do { \
    pthread_mutexattr_t   mta; \
    pthread_mutexattr_init(&mta);  \
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP); \
    pthread_mutex_init(ptr, &mta); \
    pthread_mutexattr_destroy(&mta);  \
  } while(0)

#define LOCK_DEINIT(ptr) pthread_mutex_destroy(ptr)
#define LOCK(ptr) pthread_mutex_lock(ptr)
#define UNLOCK(ptr) pthread_mutex_unlock(ptr)

#define MALLOC malloc
#define CALLOC calloc
#define FREE free

#endif
