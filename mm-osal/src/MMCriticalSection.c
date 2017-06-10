/*===========================================================================
                          V i d e o   W r a p p e r
               f o r   C r i t i c a l  S e c t i o n  O b j e c t

*//** @file MMCriticalSectionPosix.c
  This file defines a methods that can be used to synchronize access
  to data in multithreaded environment

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Common/OSAbstraction/POSIX/main/latest/src/VideoCriticalSectionPosix.c#1 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/15/09   rvontedd    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/
#include "MMCriticalSection.h"
#include "MMMalloc.h"
#include "stddef.h"
#include <pthread.h>
#include <errno.h>


//#define LOG_NDEBUG 0
#define LOG_TAG "MMCriticalsection"
#include <utils/Log.h>

typedef struct __MM_CritSection
{
  pthread_mutex_t m_mutex;
  pthread_mutexattr_t m_attr;
}__MM_CritSection;

/* -----------------------------------------------------------------------
** Function prototypes
** ----------------------------------------------------------------------- */

/*
 * Initializes the critical section
 *
 * @param[out] pHandle - a reference to the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Create
(
  MM_HANDLE  *pHandle
)
{
    int status = 1; // Failure
    __MM_CritSection *pCritSect = NULL;

    pCritSect = (__MM_CritSection *) MM_Malloc( sizeof(__MM_CritSection) );

    if(pCritSect)
    {
      status = pthread_mutexattr_init(&(pCritSect->m_attr));
      if (!status)
      {
        pthread_mutexattr_settype(&(pCritSect->m_attr), PTHREAD_MUTEX_RECURSIVE);

        // pthread_mutex_init returns zero for success
        status = pthread_mutex_init(&(pCritSect->m_mutex), &(pCritSect->m_attr));
        if(!status)
        {
           *pHandle = (MM_HANDLE) pCritSect;
        }
        else
        {
          (void)pthread_mutexattr_destroy(&(pCritSect->m_attr));
           MM_Free(pCritSect);
        }
      }
      else
      {
        MM_Free(pCritSect);
      }
    }
    return status;
}

/*
 * Releases the resources associated with the critical section
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Release
(
  MM_HANDLE  handle
)
{
    int status = 1; // Failure
    __MM_CritSection *pCritSect = (__MM_CritSection *) handle;

    if (pCritSect)
    {
      pthread_mutexattr_destroy(&(pCritSect->m_attr));
      status = pthread_mutex_destroy(&(pCritSect->m_mutex));
      MM_Free(pCritSect);
    }

    return status;
}

/*
 * Enter a critical section, blocks if another thread is inside the critical
 * section until the other thread leaves the critical section.
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Enter
(
  MM_HANDLE  handle
)
{
    int status = 1; // Failure
    __MM_CritSection *pCritSect = (__MM_CritSection *) handle;
    if(pCritSect)
    {
      status = pthread_mutex_lock(&(pCritSect->m_mutex));
    }
    return status;
}

/*
 * Leave a critical section (releases the lock).
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Leave
(
  MM_HANDLE  handle
)
{
    int status = 1; // Failure
    __MM_CritSection *pCritSect = (__MM_CritSection *) handle;
    if (pCritSect)
    {
      status = pthread_mutex_unlock(&(pCritSect->m_mutex));
    }
    return status;
}

/*
 * Tries to acquire the lock, return with failure immediately if not able to
 * acquire the lock unlike Enter which blocks until the lock is acquired.
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value is success else failure
 */
int MM_CriticalSection_TryEnter
(
  MM_HANDLE  handle
)
{
    int status = 1; // Failure
    __MM_CritSection *pCritSect = (__MM_CritSection *) handle;

    if(pCritSect)
    {
      status = pthread_mutex_trylock(&(pCritSect->m_mutex));
    }

    return status;
}

