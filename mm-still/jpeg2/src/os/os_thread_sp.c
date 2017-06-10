/*========================================================================

*//** @file os_thread_sp.c

The implementation of OS abstraction layer regarding threading specific for
linux (pthread).

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
10/16/14   vgaur   Set the names of threads.
05/12/09   vma     Created file.

========================================================================== */
#include "os_thread.h"
#include <sys/time.h>
#include "jpegerr.h"
#include <errno.h>

// thread functions
int os_thread_create(os_thread_t *p_thread, os_thread_func_t p_start_func, void *func_arg)
{
    return pthread_create(p_thread, NULL, p_start_func, func_arg);
}
int os_thread_create_ex(os_thread_t *p_thread, os_thread_func_t p_start_func, void *func_arg, int32_t priority, uint8_t *thread_name)
{
    // Other parameters are ignored for now, need to add priority and thread name
    int rc;
    rc = pthread_create(p_thread, NULL, p_start_func, func_arg);
    pthread_setname_np(*p_thread, (char *)thread_name);
    return rc;
}
int os_thread_join(os_thread_t *p_thread, OS_THREAD_FUNC_RET_T *p_ret_value)
{
    return pthread_join(*p_thread, p_ret_value);
}
int os_thread_detach(os_thread_t *p_thread)
{
    return pthread_detach(*p_thread);
}
os_thread_t os_thread_self()
{
    return pthread_self();
}

// mutex functions
int os_mutex_init(os_mutex_t *p_mutex)
{
    return pthread_mutex_init(p_mutex, NULL);
}

int os_recursive_mutex_init(os_mutex_t *p_mutex)
{
    int rc;
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    return pthread_mutex_init(p_mutex, &attr);
}
int os_mutex_destroy(os_mutex_t *p_mutex)
{
    return pthread_mutex_destroy(p_mutex);
}
int os_mutex_lock(os_mutex_t *p_mutex)
{
    return pthread_mutex_lock(p_mutex);
}
int os_mutex_unlock(os_mutex_t *p_mutex)
{
    return pthread_mutex_unlock(p_mutex);
}

// conditional variable functions
int os_cond_init(os_cond_t *p_cond)
{
    return pthread_cond_init(p_cond, NULL);
}
int os_cond_destroy(os_cond_t *p_cond)
{
    return pthread_cond_destroy(p_cond);
}
int os_cond_signal(os_cond_t *p_cond)
{
    return pthread_cond_signal(p_cond);
}
int os_cond_wait(os_cond_t *p_cond, os_mutex_t *p_mutex)
{
    return pthread_cond_wait(p_cond, p_mutex);
}
int os_cond_timedwait(os_cond_t *p_cond, os_mutex_t *p_mutex, uint32_t ms)
{
    struct timespec ts;
    int rc = clock_gettime(CLOCK_REALTIME, &ts);
    if (rc < 0) return rc;

    if (ms >= 1000) {
       ts.tv_sec += (ms/1000);
       ts.tv_nsec += ((ms%1000) * 1000000);
    } else {
        ts.tv_nsec += (ms * 1000000);
    }

    rc = pthread_cond_timedwait(p_cond, p_mutex, &ts);
    if (rc == ETIMEDOUT)
    {
        rc = JPEGERR_ETIMEDOUT;
    }
    return rc;
}

int os_cond_broadcast(os_cond_t *p_cond)
{
    return pthread_cond_broadcast(p_cond);
}
