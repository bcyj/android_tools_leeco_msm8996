#ifndef QMI_CCI_TARGET_H
#define QMI_CCI_TARGET_H
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
#include <errno.h>
#include <sys/time.h>

typedef pthread_mutex_t qmi_cci_lock_type;
#define LOCK_INIT(ptr)
#define LOCK_DEINIT(ptr) pthread_mutex_destroy(ptr)
#define LOCK(ptr) pthread_mutex_lock(ptr)
#define UNLOCK(ptr) pthread_mutex_unlock(ptr)

#define MALLOC malloc
#define CALLOC calloc
#define FREE free

#endif
