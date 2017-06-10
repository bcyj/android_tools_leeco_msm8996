/*****************************************************************************/
/*                                                                           */
/*                              Thread Abstraction Layer                     */
/*                     ITTIAM SYSTEMS PVT LTD, BANGALORE                     */
/*                             COPYRIGHT(C) 2010                             */
/*                                                                           */
/*  This program  is  proprietary to  Ittiam  Systems  Private  Limited  and */
/*  is protected under Indian  Copyright Law as an unpublished work. Its use */
/*  and  disclosure  is  limited by  the terms  and  conditions of a license */
/*  agreement. It may not be copied or otherwise  reproduced or disclosed to */
/*  persons outside the licensee's organization except in accordance with the*/
/*  terms  and  conditions   of  such  an  agreement.  All  copies  and      */
/*  reproductions shall be the property of Ittiam Systems Private Limited and*/
/*  must bear this notice in its entirety.                                   */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : ithread.c                                            */
/*                                                                           */
/*  Description       : This file contains all the necessary structure and   */
/*                      enumeration definitions needed for the Application   */
/*                      Program Interface(API) of the                        */
/*                      Thread Abstraction Layer                             */
/*                                                                           */
/*  List of Functions :                                                      */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         06 09 2012   Ittiam          Initial Version                      */
/*                                                                           */
/*****************************************************************************/
#include <string.h>
#include "datatypedef.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "ithread.h"

#include <sched.h>

#ifndef WIN32

#include <sys/syscall.h>

#include <sys/prctl.h>

#endif

//#define ENABLE_CPU_AFFINITY

UWORD32 ithread_get_handle_size(void)
{
    return sizeof(pthread_t);
}

UWORD32 ithread_get_mutex_lock_size(void)
{
    return sizeof(pthread_mutex_t);
}

UWORD32 ithread_get_cond_size(void)
{
    return sizeof(pthread_cond_t);
}


WORD32 ithread_create(void *thread_handle, void *attribute, void *strt, void *argument)
{
    return pthread_create((pthread_t *)thread_handle, NULL,strt, argument);
}

WORD32 ithread_join(void *thread_handle, void ** val_ptr)
{
    pthread_t *pthread_handle   = (pthread_t *)thread_handle;
    return pthread_join(*pthread_handle, NULL);
}

void ithread_exit(void *val_ptr)
{
    return pthread_exit(val_ptr);
}

WORD32 ithread_mutex_init(void *mutex)
{
    return pthread_mutex_init((pthread_mutex_t *) mutex, NULL);
}

WORD32 ithread_mutex_destroy(void *mutex)
{
    return pthread_mutex_destroy((pthread_mutex_t *) mutex);
}

WORD32 ithread_mutex_lock(void *mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *)mutex);
}

WORD32 ithread_mutex_unlock(void *mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

WORD32 ithread_cond_init(void *cond)
{
    return pthread_cond_init((pthread_cond_t *) cond, NULL);
}

WORD32 ithread_cond_destroy(void *cond)
{
    return pthread_cond_destroy((pthread_cond_t *) cond);
}

WORD32 ithread_cond_wait(void *cond, void *mutex)
{
    return pthread_cond_wait((pthread_cond_t *)cond, (pthread_mutex_t *)mutex);
}

WORD32 ithread_cond_broadcast(void *cond)
{
    return pthread_cond_broadcast((pthread_cond_t *)cond);
}

WORD32 ithread_cond_signal(void *cond)
{
    return pthread_cond_signal((pthread_cond_t *)cond);
}

void ithread_yield(void)
{
    sched_yield();
}

void ithread_sleep(UWORD32 u4_time)
{
    usleep(u4_time * 1000 * 1000);
}

void ithread_msleep(UWORD32 u4_time_ms)
{
    usleep(u4_time_ms * 1000);
}

void ithread_usleep(UWORD32 u4_time_us)
{
    usleep(u4_time_us);
}

UWORD32 ithread_get_sem_struct_size(void)
{
    return(sizeof(sem_t));
}


WORD32 ithread_sem_init(void *sem,WORD32 pshared,UWORD32 value)
{
    return sem_init(sem,pshared,value);
}

WORD32 ithread_sem_post(void *sem)
{
    return sem_post(sem);
}


WORD32 ithread_sem_wait(void *sem)
{
    return sem_wait(sem);
}


WORD32 ithread_sem_destroy(void *sem)
{
    return sem_destroy(sem);
}

void ithread_set_name(UWORD8 *pu1_thread_name)

{

#ifndef WIN32

prctl(PR_SET_NAME, (unsigned long)pu1_thread_name, 0, 0, 0);

#endif

}

WORD32 ithread_set_affinity(WORD32 i4_mask)
{
#ifdef ENABLE_CPU_AFFINITY
    WORD32 i4_sys_res;

    pid_t pid = gettid();

    //LOG_DEBUG("Setting affinity of %d to %x\n",pid,i4_mask);

    i4_sys_res = syscall(__NR_sched_setaffinity, pid, sizeof(i4_mask), &i4_mask);
    if (i4_sys_res)
    {
        //WORD32 err;
        //err = errno;
        //perror("Error in setaffinity syscall PERROR : ");
        //LOG_ERROR("Error in the syscall setaffinity: mask=0x%x err=0x%x", i4_mask, i4_sys_res);
        return -1;
    }
    return 1;
#else
    return 1;
#endif
}

/*
#ifndef WIN32
static void ithread_set_affinity(size_t cpu)
{
      cpu_set_t set;
      int err;

      CPU_ZERO(&set);
      CPU_SET(cpu, &set);

      err = sched_setaffinity(getpid(), sizeof(set), &set);

      if (err == -1) {
              perror("Failed to set affinity");
              exit(0);
      }
}

#endif

*/
