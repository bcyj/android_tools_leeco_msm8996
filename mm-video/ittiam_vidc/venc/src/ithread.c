/******************************************************************************
*
*                                 OMX Ittiam
*
*                     ITTIAM SYSTEMS PVT LTD, BANGALORE
*                             COPYRIGHT(C) 2011
*
*  This program is proprietary to ittiam systems pvt. ltd.,and is protected
*  under indian copyright act as an unpublished work.its use and disclosure
*  is limited by the terms and conditions of a license agreement.it may not
*  be copied or otherwise  reproduced or disclosed  to persons  outside the
*  licensee's   organization  except  in  accordance   with  the  terms and
*  conditions  of such  an agreement. all copies and reproductions shall be
*  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
*  its entirety.
*
******************************************************************************/
#include <string.h>
#include "ittiam_datatypes.h"

#include <pthread.h>
#include <semaphore.h>
#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>
#include <OMX_Ittiam.h>
#if defined(WIN32) || defined(IOS)
#else
#include <sys/prctl.h>
#endif
#include<utils/Log.h>
#include "ithread.h"



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
    return(sizeof(pthread_cond_t));
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

void ithread_yield(void)
{
#ifdef PTHREAD_AVAILABLE
    pthread_yield();
#else //PTHREAD_AVAILABLE
    usleep(0);
#endif //PTHREAD_AVAILABLE
}

void ithread_sleep(UWORD32 u4_time_in_us)
{
    usleep(u4_time_in_us);
}

UWORD32 ithread_get_sem_strcut_size(void)
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
#if defined(WIN32) || defined(IOS)
#else
    prctl(PR_SET_NAME, (unsigned long)pu1_thread_name, 0, 0, 0);
#endif
}

void ithread_condition_init(void *condition)
{
    pthread_cond_init((pthread_cond_t *)condition ,NULL);
}

void ithread_condition_signal(void * condition)
{
    pthread_cond_signal((pthread_cond_t *)condition);
}


void ithread_condition_wait(void *condition,void *mutex)
{
    pthread_cond_wait((pthread_cond_t *)condition,(pthread_mutex_t *)mutex);
}
