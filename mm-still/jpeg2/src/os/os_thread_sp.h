/*========================================================================

*//** @file os_thread_sp.h

This is the os-specific portion of the OS abstraction layer regarding
threading, specific for linux. For general usage of the API, please
see os_thread.h.

The linux implementation of the OS abstraction layer for threading relies
on the pthread implementation and is almost a one to one mapping to the
pthread API.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/07/08   vma     Created file.

========================================================================== */

#ifndef _OS_THREAD_SP_H
#define _OS_THREAD_SP_H

// Include the Linux pthread header
#include <pthread.h>

// Abstract the return type of the function to be run as a thread
#define OS_THREAD_FUNC_RET_T            void *

// Abstract the argument type to the thread function
#define OS_THREAD_FUNC_ARG_T            void *

// Abstract the function modifier placed in the beginning of the thread declaration (empty for Linux)
#define OS_THREAD_FUNC_MODIFIER

// Helpful constants for return values in the thread functions
#define OS_THREAD_FUNC_RET_SUCCEEDED    (OS_THREAD_FUNC_RET_T)0
#define OS_THREAD_FUNC_RET_FAILED       (OS_THREAD_FUNC_RET_T)1
#define OS_THREAD_DEFAULT_PRIORITY      0

// Map the os_thread_t type to pthread_t
typedef pthread_t        os_thread_t;

// Map the os_mutex_t type to pthread_mutex_t
typedef pthread_mutex_t  os_mutex_t;

// Map the os_cond_t type to pthread_cond_t
typedef pthread_cond_t   os_cond_t;

// The broadcasting of condintional variables is removed intentional from the
// OS-generic abstraction layer but is kept here for use by linux-only
// upper layer software as pthread natively supports it.
int os_cond_broadcast(os_cond_t *p_cond);

#endif // #ifndef _OS_THREAD_SP_H

