#ifndef MMTHREAD_H
#define MMTHREAD_H
/*===========================================================================
                          M M    W r a p p e r
                      f o r   T h r e a d   S e r v i c e s

*//** @file MMThread.h
  This file defines a utility class that can be used to create and control
  threads.

Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMThread.h#5 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Converted comments to doxygen style.
07/27/09   rmandal     Added MM_Thread_CreateEx to support thread name.
07/02/08   gkapalli    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifndef _MM_HANDLE
typedef void* MM_HANDLE;
#define _MM_HANDLE
#endif



#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/* -----------------------------------------------------------------------
** To support basic priority mapping for OSAL clients.
** ----------------------------------------------------------------------- */
extern const int MM_Thread_DefaultPriority;
extern const int MM_Thread_ComponentPriority;
extern const int MM_Thread_ModulePriority;
extern const int MM_Thread_RealtimePriority;
extern const int MM_Thread_BackgroundPriority;

/**
 * Creates a thread and begins execution
 *
 * The thread handle needs to be released by calling MM_Thread_Exit
 *
 * @param[in] nPriority - thread priority
 * @param[in] nSuspend -  suspend count a value greater than 0 results in a
                          suspended thread
 * @param[in] pfnStart - start routine at which thread execution starts
 * @param[in] pvStartArg - data pointer passed to start routine
 * @param[in] dwStackSize - size of thread stack
 * @param[in] pstrName - Null terminated Name of the thread
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
);

/**
 * Creates a thread and begins execution
 *
 * The thread handle needs to be released by calling MM_Thread_Exit
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
);

/**
 * Releases the resources associated with the thread
 *
 * @param[in] handle - the thread handle
 *
 * @return zero value on success else failure
 */
int MM_Thread_Release
(
  MM_HANDLE handle
);

/**
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
  int         *pnExitCode
);

/**
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
);

/**
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
);

/**
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
);

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
);

/**
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
);

/**
 * Gets the thread handle of the calling context
 *
 * @param[out] pHandle - the thread handle on success
 *
 * @return value 0 is success else failure
 */
int MM_Thread_GetCurrent
(
  MM_HANDLE *pHandle
);

/**
 * Compares two thread handles
 *
 * @param[in] handle1 - the thread handle on compare
 * @param[in] handle2 - the thread handle on compare
 *
 * @return value 0 is same else the handles are different
 */
int MM_Thread_Compare
(
  MM_HANDLE handle1,
  MM_HANDLE handle2

);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMTHREAD_H
