/*===========================================================================
                          V i d e o   W r a p p e r
                      f o r   T h r e a d   S e r v i c e s

*//** @file MMThreadPosix.c
  This file implements the thread interface that can be used to create and
  control threads.

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Common/OSAbstraction/POSIX/main/latest/src/VideoThreadPosix.c#1 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Added video_Thread_CreateEx to support thread name.
07/15/09   rvontedd    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/
#include "comdef.h"
#include "MMThread.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

#define UNION_CAST(x, destType) \
    (((union {__typeof__(x) a; destType b;})x).b)

#define UNUSED(x) ((void)x)

/*
 * Creates a thread and begins execution
 *
 * The thread handle needs to be released by calling video_Thread_Exit
 *
 * @param[in] nPriority - thread priority
 * @param[in] nSuspend -  suspend count a value greater than 0 results in a
                          suspended thread
 * @param[in] pfnStart - start routine at which thread execution starts
 * @param[in] pvStartArg - data pointer passed to start routine
 * @param[in] dwStackSize - size of thread stack
 * @param[in] name - Null terminated Name of the thread
 * @param[out] pHandle - returns a reference to the thread handle
 *
 * @return zero value on success else failure
 */
int MM_Thread_CreateEx
(
  int            nPriority,
  int            nSuspend,
  int           (*pfnStart)(void* ),
  void*          pvStartArg,
  unsigned int   dwStackSize,
  const char          *pstrName,
  MM_HANDLE  *pHandle
)
{
    UNUSED(nSuspend);
    UNUSED(pstrName);
    pthread_attr_t attr;
    int status = 1; // Failure

    if ((status = pthread_attr_init(&attr)) != 0)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "MM_Thread_CreateEx::attr init failed %d", status);
    }
    else
    {
      if ((status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "MM_Thread_CreateEx::set detach state failed %d", status);
      }
      else
      {
        struct sched_param prio;
        prio.sched_priority = nPriority;
        if ((status = pthread_attr_setschedparam(&attr, &prio)) != 0)
        {
            return status;
        }
        // Set stack size only if needed, else go with the default (1MB)
        unsigned int stacksize = 0;
        (void)pthread_attr_getstacksize(&attr, (void *)&stacksize);
        if (stacksize < dwStackSize)
        {
          stacksize = dwStackSize;
          if ((status = pthread_attr_setstacksize(&attr, stacksize)) != 0)
          {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "MM_Thread_CreateEx::set stack size failed %d", status);
          }
        }

        if (status == 0)
        {
        // Naming of threads is not supported by pthreads
        /*memcpy(attr.name, pstrName, PTHREAD_NAME_MAX_LEN);*/
          if ((status = pthread_create((pthread_t *)pHandle, &attr, (void *(*)(void *))pfnStart, pvStartArg)) != 0)
          {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "MM_Thread_CreateEx::create failed %d", status);
          }
          else
          {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "MM_Thread_CreateEx::thread 0x%lu created with stack %d bytes",
                         (unsigned long)*pHandle, stacksize);
          }
        }
      }

      pthread_attr_destroy(&attr);
    }

    return status;
}

/*
 * Creates a thread and begins execution
 *
 * The thread handle needs to be released by calling video_Thread_Exit
 *
 * @param[in] nPriority - thread priority
 * @param[in] nSuspend -  suspend count a value greater than 0 results in a
                          suspended thread
 * @param[in] pfnStart - start routine at which thread execution starts
 * @param[in] pvStartArg - data pointer passed to start routine
 * @param[in] dwStackSize - size of thread stack
 * @param[out] pHandle - returns a reference to the thread handle
 *
 * @return zero value on success else failure
 */
int MM_Thread_Create
(
  int            nPriority,
  int            nSuspend,
  int           (*pfnStart)(void* ),
  void*          pvStartArg,
  unsigned int   dwStackSize,
  MM_HANDLE  *pHandle
)
{
    // TODO:  Need to add support for automatic name like THREAD01, THREAD02 etc
    return MM_Thread_CreateEx(nPriority, nSuspend, pfnStart, pvStartArg, dwStackSize, NULL, pHandle);
}
/*
 * Releases the resources associated with the thread
 *
 * @param[in] handle - the thread handle
 *
 * @return zero value on success else failure
 */
int MM_Thread_Release
(
  MM_HANDLE handle
)
{
    int status = 1 ; // Failure
    pthread_t threadhandle = (pthread_t) handle;

    if(threadhandle)
        status = pthread_detach(threadhandle);

    return status;
}

/*
 * Blocks the calling thread until the specified thread terminates
 *
 * @param[in] handle - the thread handle
 * @param[out] pnExitCode - the exit code
 *
 * @return zero value on success else failure
 */
int MM_Thread_Join
(
  MM_HANDLE handle,
  int       *pnExitCode
)
{
    int status = 1; // Failure
    pthread_t phandle = (pthread_t)handle;

    if(phandle)
        status = pthread_join(phandle, (void **)pnExitCode);

    return status;
}

/*
 * Exits the thread with the given code. No return from this call.
 *
 * @param[in] handle - the thread handle
 * @param[in] nExitCode - the exit code
 *
 * @return zero value on success else failure
 */
int MM_Thread_Exit
(
  MM_HANDLE  handle,
  int           nExitCode
)
{
    UNUSED(handle);
    pthread_exit(&nExitCode);
    return 0;
}

/*
 * Increments the suspend count on a thread, suspending execution of the
 * thread if the previous count was 0.
 *
 * @param[in] handle - the thread handle
 * @param[out] pnSuspCount - the thread's previous suspend count
 *
 * @return zero value on success else failure
 */
int MM_Thread_Suspend
(
  MM_HANDLE handle,
  int         *pnSuspCount
)
{
 // TODO: Not implemented yet
    UNUSED(handle);
    UNUSED(pnSuspCount);
    return 1;
}

/*
 * Decrements the suspend count on a thread, resumes execution of the
 * thread if the resulting count was 0.
 *
 * @param[in] handle - the thread handle
 * @param[out] pnSuspCount - the thread's previous suspend count
 *
 * @return value 0 is success else failure
 */
int MM_Thread_Resume
(
  MM_HANDLE  handle,
  int          *pnSuspCount
)
{
  // TODO: Not implemented yet
  UNUSED(handle);
  UNUSED(pnSuspCount);
    return 1;
}

/*
 * Returns the priority of the current thread.
 *
 * @param[in] handle - the thread handle
 * @param[out] pnPriority - the thread priority
 *
 * @return value 0 is success else failure
 */
int MM_Thread_GetPriority
(
  MM_HANDLE  handle,
  int          *pnPriority
)
{
  // TODO: Not implemented yet
    UNUSED(handle);
    UNUSED(pnPriority);
    return 1;
}

/*
 * Sets the priority of the current thread.
 *
 * @param[in] handle - the thread handle
 * @param[in] nPriority - the thread priority
 *
 * @return value 0 is success else failure
 */
int MM_Thread_SetPriority
(
  MM_HANDLE handle,
  int          nPriority
)
{
  // TODO: Not implemented yet
  UNUSED(handle);
  UNUSED(nPriority);
    return 1;
}

/*
 * Gets the thread handle of the calling context
 *
 * @param[out] pHandle - the thread handle on success
 *
 * @return value 0 is success else failure
 */
int MM_Thread_GetCurrent
(
  MM_HANDLE *pHandle
)
{
    pthread_t thread_id = pthread_self();
    *pHandle = (MM_HANDLE)thread_id;

    return 0;
}

/*
 * Compares two thread handles
 *
 * @param[in] handle1 - the thread handle on compare
 * @param[in] handle2 - the thread handle on compare
 *
 * @return value 0 means same thread else the handles are different
 */
int MM_Thread_Compare
(
  MM_HANDLE handle1,
  MM_HANDLE handle2

)
{
    int status;
//    pthread_t * phandle1 = (pthread_t *) &handle1;
//    pthread_t * phandle2 = (pthread_t *) &handle2;

    pthread_t * phandle1 = UNION_CAST(&handle1,pthread_t *);
    pthread_t * phandle2 = UNION_CAST(&handle2,pthread_t *);
    status = pthread_equal(*phandle1, *phandle2);

    return status;
}

//Todo - someone to define something sensible when we pull in modules that are dependant on these values.
//const int MM_Thread_DefaultPriority = 158;
/*
   After reviewing the current thread priorities in android,
   determined that THREAD_PRIORITY_DEFAULT (0x0) is the appropriate value.
*/
const int MM_Thread_DefaultPriority = 0;

//const int MM_Thread_ComponentPriority = 158;
//const int MM_Thread_ModulePriority = 158;
//const int MM_Thread_RealtimePriority;
//const int MM_Thread_BackgroundPriority;



