/*============================================================================
  @file sns_osa_linux.c

  @brief
  Implements the OS abstraction layer for linux.

  <br><br>

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*=====================================================================
                              INCLUDES
=======================================================================*/
#include "sensor1.h"
#include "sns_em.h"
#include "sns_osa.h"

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

/*=====================================================================
                    INTERNAL DEFINITIONS AND TYPES
=======================================================================*/

#define NSEC_PER_USEC 1000
#define NSEC_PER_SEC  1000000000

#undef REALTIME_SCHED
#undef MUTEX_TIMEDLOCK
#undef CONDATTR_SETCLOCK
#undef PTHREAD_SET_INHERIT_SCHED

#ifndef MUTEX_TIMEDLOCK
#define MUTEX_POLL_USEC 1000
#endif

/* Determine the clock source to use for condition variable waits */
#ifdef CONDATTR_SETCLOCK
#  if defined CLOCK_MONOTONIC_HR
#    define CLOCK CLOCK_MONOTONIC_HR
#  elif defined CLOCK_REALTIME_HR
#    define CLOCK CLOCK_REALTIME_HR
#  elif defined CLOCK_THREAD_CPUTIME_ID
#    define CLOCK CLOCK_THREAD_CPUTIME_ID
#  elif defined CLOCK_PROCESS_CPUTIME_ID
#    define CLOCK CLOCK_PROCESS_CPUTIME_ID
#  else
#    define CLOCK CLOCK_REALTIME
#  endif
#else /*CONDATTR_SETCLOCK*/
#  define CLOCK CLOCK_REALTIME
#endif /*CONDATTR_SETCLOCK*/


/*=====================================================================
                    EXERNAL GLOBALS
=======================================================================*/

/*=====================================================================
                    STATIC GLOBALS
=======================================================================*/

/*=====================================================================
                    INTERNAL FUNCTIONS
=======================================================================*/
/*===========================================================================

  FUNCTION: sns_os_sigs_ready

  ===========================================================================*/
/*!
  @brief Returns true if the flags match the requested criteria

  @param[i] set_flags: OS flags indicating which flags are set

  @param[i] pend_flags: OS flags indicating which flags are being queried

  @param[i] wait_type: The condition on which the flags are being queried:
    OS_FLAG_WAIT_CLR_ALL: all pend_flags must be cleared in set_flags
    OS_FLAG_WAIT_CLR_ANY: One pend_flag must be cleared in set_flags
    OS_FLAG_WAIT_SET_ALL: all pend_flags must be set in set_flags
    OS_FLAG_WAIT_SET_ANY: One pend_flags must be set in set_flags

  @return
  true if the conditions are met.

  @sideeffects
  none

  @note
*/
/*=========================================================================*/
static int8_t sns_os_sigs_ready( OS_FLAGS set_flags,
                                 OS_FLAGS pend_flags,
                                 uint8_t  wait_type )
{
  int8_t rv = FALSE; // Return value
  switch( (wait_type & 0xF) ) {
    case OS_FLAG_WAIT_CLR_ALL:
      if( (~set_flags & pend_flags) == pend_flags ) {
        rv = TRUE;
      }
      break;
    case OS_FLAG_WAIT_CLR_ANY:
      if( ~set_flags & pend_flags ) {
        rv = TRUE;
      }
      break;
    case OS_FLAG_WAIT_SET_ALL:
      if( (set_flags & pend_flags) == pend_flags ) {
        rv = TRUE;
      }
      break;
    default:
    case OS_FLAG_WAIT_SET_ANY:
      if( set_flags & pend_flags ) {
        rv = TRUE;
      }
      break;
  }
  return rv;
}

/*=====================================================================
                     EXTERNAL FUNCTIONS
=======================================================================*/
/* Task */

/*===========================================================================

  FUNCTION: sns_os_task_create_ext

  ===========================================================================*/
uint8_t sns_os_task_create_ext (void           (*task)(void *p_arg),
                                void            *p_arg,
                                OS_STK          *ptos,
                                uint8_t          prio,
                                uint16_t         id,
                                OS_STK          *pbos,
                                uint32_t         stk_size,
                                void            *pext,
                                uint16_t         opt,
                                uint8_t         *name)
{
  UNREFERENCED_PARAMETER(id);
  UNREFERENCED_PARAMETER(pbos);
  UNREFERENCED_PARAMETER(stk_size);
  UNREFERENCED_PARAMETER(pext);
  UNREFERENCED_PARAMETER(opt);
  UNREFERENCED_PARAMETER(name);
  return sns_os_task_create( task, p_arg, ptos, prio );
}

/*===========================================================================

  FUNCTION: sns_os_task_create

  ===========================================================================*/
uint8_t sns_os_task_create (void           (*task)(void *p_arg),
                            void            *p_arg,
                            OS_STK          *ptos,
                            uint8_t          prio)
{
  pthread_t thread;
  uint8_t error;
  pthread_attr_t attr;

  UNREFERENCED_PARAMETER(ptos);

  error = pthread_attr_init( &attr );

#ifdef _POSIX_THREAD_ATTR_STACKADDR
  // TODO: use the newer stack address functions
  //  error = pthread_attr_setstackaddr( &attr, (void*)ptos );
#endif /* _POSIX_THREAD_ATTR_STACKADDR */

  error |= pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

#ifdef REALTIME_SCHED
  {
    struct sched_param sched;
#ifdef PTHREAD_SET_INHERIT_SCHED
    error = pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    if( 0 != error ) {
      printf("setinheritsched failed\n");
    }
#endif /* PTHREAD_SET_INHERIT_SCHED */
    error = pthread_attr_setschedpolicy( &attr, SCHED_FIFO );
    if( 0 != error ) {
      printf("setschedpolicy failed %d\n", error);
    }
    sched.sched_priority = prio;
    error = pthread_attr_setschedparam( &attr, &sched );
    if( 0 != error ) {
      printf("setschedparam failed %d\n", error);
    }
  }
#else
  UNREFERENCED_PARAMETER(prio);
#endif /* REALTIME_SCHED */

  // This is a scary cast -- but it should work as long as tasks don't return

  error |= pthread_create( &thread, &attr, (void*(*)(void*))task, p_arg );

  pthread_attr_destroy( &attr );

  return error;
}

/*===========================================================================

  FUNCTION: sns_os_task_del

  ===========================================================================*/
uint8_t sns_os_task_del     (uint8_t prio)
{
#ifdef REALTIME_SCHED
  /* Currently only works if the calling thread is deleting itself */
  pthread_attr_t attr;
  struct sched_param sched;
  int error;

  error = pthread_attr_getschedparam( &attr, &sched );

  if( 0 == error ) {
    if( sched.sched_priority != prio ) {
      return 1;
    }
  } else {
    return 1;
  }
#else
  UNREFERENCED_PARAMETER(prio);
#endif /* REALTIME_SCHED */
  /* If realtime scheduling is not used, there's currently no way to ensure
   * that the requsted "prio" is the current thread. Just assume that it is... */
  pthread_exit( NULL );
  return 0;
}

/*===========================================================================

  FUNCTION: sns_os_task_del_req

  ===========================================================================*/
uint8_t sns_os_task_del_req (uint8_t prio)
{
  // TODO: Implement
  UNREFERENCED_PARAMETER(prio);
  return 1;
}


/* Flag */

/*===========================================================================

  FUNCTION: sns_os_sigs_create

  ===========================================================================*/
OS_FLAG_GRP  *sns_os_sigs_create (OS_FLAGS         flags,
                                  uint8_t         *perr)
{
  pthread_mutexattr_t attr;
  pthread_condattr_t cond_attr;
  OS_FLAG_GRP *grp;
  int error;

  grp = (OS_FLAG_GRP*)malloc(sizeof(OS_FLAG_GRP));

  if( NULL == grp )
  {
    *perr = OS_ERR_MEM_FULL;
    return NULL;
  }

  pthread_condattr_init( &cond_attr );
#ifdef CONDATTR_SETCLOCK
  pthread_condattr_setclock( &cond_attr, CLOCK );
#endif

  error = pthread_cond_init( &(grp->cond), &cond_attr );

  if( 0 != error ) {
    pthread_condattr_destroy( &cond_attr );
    *perr = OS_ERR_FLAG_GRP_DEPLETED;
    return NULL;
  }
  grp->flags = flags;

  error = pthread_mutexattr_init( &attr );
  //error |= pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
  error |= pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
  error |= pthread_mutex_init( &(grp->mutex), &attr );
  if( 0 != error ) {
    pthread_mutexattr_destroy( &attr );
    pthread_cond_destroy( &(grp->cond) );
    *perr = OS_ERR_FLAG_GRP_DEPLETED;
    return NULL;
  }

  *perr = OS_ERR_NONE;
  return grp;
}

/*===========================================================================

  FUNCTION: sns_os_sigs_del

  ===========================================================================*/
OS_FLAG_GRP  *sns_os_sigs_del    (OS_FLAG_GRP     *pgrp,
                                  uint8_t          opt,
                                  uint8_t         *perr)
{
  // TODO: check options
  UNREFERENCED_PARAMETER(opt);

  pthread_mutex_destroy( &(pgrp->mutex) );
  pthread_cond_destroy( &(pgrp->cond) );
  free( pgrp );
  pgrp = NULL;
  *perr = OS_ERR_NONE;
  return pgrp;
}

/*===========================================================================

  FUNCTION: sns_os_sigs_pend

  ===========================================================================*/
OS_FLAGS sns_os_sigs_pend (OS_FLAG_GRP     *pgrp,
                           OS_FLAGS         flags,
                           uint8_t          wait_type,
                           uint32_t         timeout,
                           uint8_t         *perr)
{
  uint32_t set_flags;
  struct timespec ts;
  int error = 0;

  if( 0 != timeout ) {
    uint64_t ns_time;
    clock_gettime( CLOCK, &ts );
    ns_time = ts.tv_nsec
            + (sns_em_convert_localtick_to_usec(timeout) * NSEC_PER_USEC);
    ts.tv_sec += (time_t)(ns_time / NSEC_PER_SEC);
    ts.tv_nsec = (long)(ns_time % NSEC_PER_SEC);
  }

  pthread_mutex_lock( &pgrp->mutex );
  set_flags = pgrp->flags & flags;
  if( !sns_os_sigs_ready(set_flags,flags,wait_type) ) {
    do {
      if( 0 == timeout ) {
        error = pthread_cond_wait( &pgrp->cond, &pgrp->mutex );
      } else {
        error = pthread_cond_timedwait( &pgrp->cond, &pgrp->mutex, &ts );
      }
      set_flags = pgrp->flags & flags;
      if( ETIMEDOUT == error ) {
        break;
      }
    } while( !sns_os_sigs_ready(set_flags,flags,wait_type) );
  }

  if( OS_FLAG_CONSUME & wait_type ) {
    pgrp->flags &= ~flags;
  }
  pthread_mutex_unlock( &pgrp->mutex );

  if( 0 == error ) {
    *perr = OS_ERR_NONE;
  } else {
    // TODO: set a proper error
    *perr = OS_ERR_TIMEOUT;
  }
  return set_flags;
}

/*===========================================================================

  FUNCTION: sns_os_sigs_post

  ===========================================================================*/
OS_FLAGS sns_os_sigs_post (OS_FLAG_GRP     *pgrp,
                           OS_FLAGS         flags,
                           uint8_t          opt,
                           uint8_t         *perr)
{
  pthread_mutex_lock( &pgrp->mutex );
  if( OS_FLAG_CLR == opt ) {
    pgrp->flags &= ~flags;
  } else {
    pgrp->flags |= flags;
  }
  flags = pgrp->flags;
  // TODO: make this better by only signalling if someone is waiting
  // on these flags.
  pthread_cond_signal( &pgrp->cond );
  pthread_mutex_unlock( &pgrp->mutex );

  *perr = OS_ERR_NONE;
  return flags;
}

/*===========================================================================

  FUNCTION: sns_os_sigs_add

  ===========================================================================*/
void *sns_os_sigs_add (OS_FLAG_GRP     *pgrp,
                       OS_FLAGS         flags)
{
  // Not implemented
  UNREFERENCED_PARAMETER(pgrp);
  UNREFERENCED_PARAMETER(flags);
  return NULL;
}

/* Mutex */


/*===========================================================================

  FUNCTION: sns_os_mutex_create

  ===========================================================================*/
OS_EVENT *sns_os_mutex_create (uint8_t          prio,
                               uint8_t         *perr)
{
  pthread_mutex_t *mutex;
  pthread_mutexattr_t attr;

  UNREFERENCED_PARAMETER(prio);

  mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

  if( mutex == NULL ) {
    *perr = OS_ERR_MEM_FULL;
    return NULL;
  }

  pthread_mutexattr_init(&attr);
#ifdef REALTIME_SCHED
  //pthread_mutexattr_setprioceiling( &attr, prio );
#endif /* REALTIME_SCHED */


  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
  //pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
  *perr = pthread_mutex_init( mutex, &attr );

  pthread_mutexattr_destroy(&attr);

  return (OS_EVENT*)mutex;
}


/*===========================================================================

  FUNCTION: sns_os_mutex_del

  ===========================================================================*/
OS_EVENT *sns_os_mutex_del (OS_EVENT        *pevent,
                            uint8_t          opt,
                            uint8_t         *perr)
{
  int error;

  UNREFERENCED_PARAMETER(opt);

  error = pthread_mutex_destroy( &pevent->mutex );
  if( 0 == error ) {
    pevent = NULL;
    *perr = OS_ERR_NONE;
  } else if ( EBUSY == error ) {
    *perr =  OS_ERR_TASK_WAITING;
  } else {
    *perr = OS_ERR_EVENT_TYPE;
  }
  return pevent;
}


/*===========================================================================

  FUNCTION: sns_os_mutex_pend

  ===========================================================================*/
void sns_os_mutex_pend (OS_EVENT        *pevent,
                        uint32_t         timeout,
                        uint8_t         *perr)
{
  struct timespec ts;
  int error;

  if( 0 == timeout ) {
    *perr = pthread_mutex_lock( &pevent->mutex );
  } else {
#ifdef MUTEX_TIMEDLOCK
    clock_gettime( CLOCK_REALTIME, &ts );
    // hard-coded to CLOCK_REALTIME, as pec spec
    ts.tv_nsec += sns_em_convert_localtick_to_usec(timeout) * NSEC_PER_USEC;
    ts.tv_sec += ts.tv_nsec / NSEC_PER_SEC;
    ts.tv_nsec %= NSEC_PER_SEC;

    error = pthread_mutex_timedlock( &pevent->mutex, &ts );
    if( ETIMEDOUT == error ) {
      *perr = OS_ERR_TIMEOUT;
    } else {
      *perr = OS_ERR_EVENT_TYPE;
    }
#else

    /* If this posix implementation doesn't support timed locks, implement
     * a polling mechanism for checking the mutex periodically */

    struct timespec current_ts;
    int past_time = 0;
    clock_gettime( CLOCK_REALTIME, &ts );
    ts.tv_nsec += sns_em_convert_localtick_to_usec(timeout) * NSEC_PER_USEC;
    ts.tv_sec += ts.tv_nsec / NSEC_PER_SEC;
    ts.tv_nsec %= NSEC_PER_SEC;
    while( !past_time ) {
      error = pthread_mutex_trylock( &pevent->mutex );
      if( 0 == error ) {
        *perr = OS_ERR_NONE;
        return;
      }
      clock_gettime( CLOCK_REALTIME, &current_ts );
      if( (current_ts.tv_sec >= ts.tv_sec) &&
          (current_ts.tv_nsec > ts.tv_nsec) )  {
        *perr = OS_ERR_TIMEOUT;
        return;
      }
      if( EBUSY == error ) {
        usleep(MUTEX_POLL_USEC);
      } else {
        *perr = OS_ERR_EVENT_TYPE;
        return;
      }
    }
#endif
  }
}

/*===========================================================================

  FUNCTION: sns_os_mutex_post

  ===========================================================================*/
uint8_t sns_os_mutex_post (OS_EVENT        *pevent)
{
  pthread_mutex_unlock( &pevent->mutex );
  return OS_ERR_NONE;
}

OS_EVENT *sns_os_mutex_create_uimg (uint8_t          prio,
                                    uint8_t         *perr)
{
  return (sns_os_mutex_create(prio, perr));
}

