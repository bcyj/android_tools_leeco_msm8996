/*============================================================================
  FILE:         slim_internal.c

  OVERVIEW:     SLIM processor specific utils implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "slim_internal.h"

/**
@brief Returns current processor type.

Function returns current processor type.

@return Current processor type.
*/
slim_ProcessorEnumType slim_CurrentProcessor
(
  void
)
{
  return SLIM_PROCESSOR_APSS;
}

/**
@brief Size bounded memory copy.

Function copies bytes from the source buffer to the destination buffer.

@param dst:       Destination buffer.
@param dst_size:  Size of the destination buffer in bytes.
@param src:       Source buffer.
@param src_size:  Number of bytes to copy from source buffer.
@return The number of bytes copied to the destination buffer.
*/
size_t slim_Memscpy
(
  void *dst,
  size_t dst_size,
  const void *src,
  size_t src_size
)
{
    memcpy(dst, src, src_size);

    return src_size;
}

/**
@brief Creates a mutex which can be associated with a resource.

Function creates a mutex which can be associated with a resource. Mutexes are
used to allow only one thread to enter the critical section of code that is
accessing shared data.

@param pz_Mutex: Pointer to the mutex structure
@return TRUE if initialized with no errors, otherwise FALSE.
*/
boolean slim_MutexInit
(
  slim_MutexStructType *pz_Mutex
)
{
    //retained the comments in case needed fro debugging in phase 2
    /* SLIM_MSG_HIGH("[SLIM] slim_MutexInit: Entering slim_MutexInit with pz_Mutex %08X and d_mutex %08X", */
    /*               pz_Mutex, pz_Mutex->d_mutex, 0); */
    bool returnValue = false;
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    if(pthread_mutex_init(&(pz_Mutex->d_mutex), &mta) == 0)
    {
        /* SLIM_MSG_HIGH("[SLIM] slim_MutexInit: Leaving slim_MutexInit", */
        /*               0, 0, 0); */
        pthread_mutexattr_destroy(&mta);
        returnValue = true;
    }

    return returnValue;
}

/**
@brief Locks the resource associated with pz_Mutex if it is not already locked.

Function locks the resource associated with pz_Mutex if it is not already
locked.

@param pz_Mutex: Pointer to the mutex structure
*/
void slim_MutexLock
(
  slim_MutexStructType *pz_Mutex
)
{
    /* SLIM_MSG_HIGH("[SLIM] slim_MutexLock: Entering slim_MutexLock with pz_Mutex %08X and d_mutex %08X", */
    /*               pz_Mutex, pz_Mutex->d_mutex, 0); */

    pthread_mutex_lock(&pz_Mutex->d_mutex);

    /* SLIM_MSG_HIGH("[SLIM] slim_MutexLock: Leaving slim_MutexLock with pz_Mutex %08X and d_mutex %08X", */
    /*               pz_Mutex, pz_Mutex->d_mutex, 0); */

}

/**
@brief Unlocks the resource associated with pz_Mutex.

Function unlocks the resource associated with pz_Mutex.

@param pz_Mutex: Pointer to the mutex structure
*/
void slim_MutexUnlock
(
  slim_MutexStructType *pz_Mutex
)
{
    /* SLIM_MSG_HIGH("[SLIM] slim_MutexUnLock: Entering slim_MutexUnLock with pz_Mutex %08X and d_mutex %08X", */
    /*               pz_Mutex, pz_Mutex->d_mutex, 0); */

    pthread_mutex_unlock(&pz_Mutex->d_mutex);

    /* SLIM_MSG_HIGH("[SLIM] slim_MutexUnLock: Leaving slim_MutexUnLock with pz_Mutex %08X and d_mutex %08X", */
    /*               pz_Mutex, pz_Mutex->d_mutex, 0); */
}

/**
@brief Returns the timetick count in milliseconds.

Function returns the current timetick count in milliseconds.

@return Current timetick count in milliseconds.
*/
uint64 slim_TimeTickGetMilliseconds
(
  void
)
{
  struct timespec ts;
  uint64 time_ms = 0;
  clock_gettime(CLOCK_BOOTTIME, &ts);

  time_ms += (ts.tv_sec * 1000LL);     /* Seconds to milliseconds */
  time_ms += ts.tv_nsec / 1000000LL;   /* Nanoseconds to milliseconds */

  return time_ms;

}
