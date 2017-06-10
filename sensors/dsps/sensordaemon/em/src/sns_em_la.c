/*============================================================================
  @file sns_em_la.c

  @brief
  This implements the Event Manager (EM) for the Linux Android platform.
  It manages timers, and converting time units between apps/dsps ticks and
  local apps time.

  <br><br>

  DEPENDENCIES: Must be initialized using sns_em_init().

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "fixed_point.h"
#include "sns_common.h"
#include "sns_debug_str.h"
#include "sns_debug_api.h"
#include "sns_em.h"
#include "sns_init.h"
#include "sns_memmgr.h"
#include "sns_osa.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define SNS_TS_CLOCK CLOCK_REALTIME
#define SNS_TIMER_CLOCK CLOCK_MONOTONIC

#define USEC_PER_SEC 1000000L
#define NSEC_PER_SEC 1000000000L

#define SEC_TO_USEC(sec)   ((sec)*USEC_PER_SEC)
#define NSEC_TO_USEC(nsec) ((nsec)/1000L)

#define USEC_TO_NSEC(usec) ((usec)*1000L)
#define USEC_TO_SEC(usec)  ((usec)/USEC_PER_SEC)

#define DSPS_HZ 32768LL

/*============================================================================
  Type Declarations
  ============================================================================*/

struct sns_em_tmr_s {
  void  *callback_arg;                    // Argument to the callback routine
  void  (*CallBack)(void * callback_arg); // callback routine of the client
  struct timespec timer_expry;            // Time when the timer should expire
  uint32_t period_ticks;                  /* 0 if not periodic, otherwise number
                                           * of ticks between periodic
                                           * intervals */
  struct sns_em_tmr_s *link;              // queue next link pointer
};

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

static struct sns_em_tmr_s *tmr_head_ptr = NULL;
static OS_EVENT *sns_em_mutex = (OS_EVENT*)NULL;
static bool sns_em_initialized = false;

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================

  FUNCTION:   compare_ts

  ===========================================================================*/
/*!
  @brief Compares two timespecs

  This function says that 2 timers are equal if they are within 1usec of
  each other.

  @param[i] t1_ptr: pointer to timer1
  @param[i] t2_ptr: pointer to timer2

  @return
  -1 : t1 < t2
   0 : t1 == t2
   1 : t1 > t2


  @sideeffects
  none
*/
/*=========================================================================*/
static int_fast8_t compare_ts( const struct timespec *t1_ptr,
                               const struct timespec *t2_ptr )
{
  int_fast8_t rv;
  if( t1_ptr->tv_sec < t2_ptr->tv_sec ) {
    rv = -1;
  } else if( t1_ptr->tv_sec > t2_ptr->tv_sec ) {
    rv = 1;
  } else if( abs(t1_ptr->tv_nsec - t2_ptr->tv_nsec) < USEC_TO_NSEC(1)) {
    rv = 0;
  } else {
    rv = (t1_ptr->tv_nsec > t2_ptr->tv_nsec)?1:-1;
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   add_usec_to_ts

  ===========================================================================*/
/*
  @brief Adds microsecnd count to a timespec

  @param[i] t1_ptr: pointer to timer1
  @param[i] usec: Number of microseconds to add to t1

  @return
  Normalized t1 + usec

  @sideeffects
  none
*/
/*=========================================================================*/
static struct timespec add_usec_to_ts( const struct timespec *t1_ptr,
                                       uint32_t usec )
{
  struct timespec rv;

  rv.tv_sec = t1_ptr->tv_sec;

  /* Compute number of seconds in the usec value provided
   * to avoid rollover issues
   */
  rv.tv_sec += usec/USEC_PER_SEC;
  usec = usec % USEC_PER_SEC;
  rv.tv_nsec = t1_ptr->tv_nsec + USEC_TO_NSEC(usec);

  /* Check if the nano seconds component is greater than 1 second
   * If it is more than a second then the setitimer functions will
   * fail
   */
  if (rv.tv_nsec > (NSEC_PER_SEC-1))
  {
    rv.tv_nsec = rv.tv_nsec - 1000000000;
    rv.tv_sec = rv.tv_sec + 1;
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sub_ts

  ===========================================================================*/
/*
  @brief Subtracts two timespecs

  @param[i] t1_ptr: pointer to timer1
  @param[i] t2_ptr: pointer to timer2

  @return
  Normalized t1 - t2

  @sideeffects
  none
*/
/*=========================================================================*/
static struct timespec sub_ts( const struct timespec *t1_ptr,
                               const struct timespec *t2_ptr )
{
  struct timespec rv;

  rv.tv_sec = t1_ptr->tv_sec - t2_ptr->tv_sec;

  rv.tv_nsec = t1_ptr->tv_nsec - t2_ptr->tv_nsec;

  if( 0 > rv.tv_nsec ) {
    rv.tv_nsec += NSEC_PER_SEC;
    rv.tv_sec -= 1;
  }

  return rv;
}

/*===========================================================================

  FUNCTION:   unlink_tmr

  ===========================================================================*/
/*!
  @brief Removes timer from the timer list

  @param[i] t_ptr: pointer to the timer

  @return
  None

  @dependencies
  Requires that the mutex "sns_em_mutex" be locked before calling.

  @sideeffects
  Modifies the global timer list. May set it to NULL if there are no other
  timers.
  Clears the posix "itimer" if there are no other timers.
*/
/*=========================================================================*/
static void unlink_tmr( struct sns_em_tmr_s *t_ptr )
{
  struct sns_em_tmr_s *index_ptr;
  if( tmr_head_ptr == t_ptr ) {
    tmr_head_ptr = t_ptr->link;
  } else {
    index_ptr = tmr_head_ptr;
    while( index_ptr && index_ptr->link != t_ptr ) {
      index_ptr = index_ptr->link;
    }
    if( index_ptr != NULL ) {
      index_ptr->link = t_ptr->link;
    }
  }
  if( tmr_head_ptr == NULL ) {
    struct itimerval value;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    setitimer( ITIMER_REAL, &value, NULL );
  }
  t_ptr->link = NULL;
}


/*===========================================================================

  FUNCTION:   set_next_timer

  ===========================================================================*/
/*!
  @brief Sets the POSIX "itimer" to expire for the next timer.

  @dependencies
  Requires the sns_em_mutex to be held before entering.

  @return
  None

  @sideeffects
  None
*/
/*=========================================================================*/
static void set_next_timer( void )
{
  struct itimerval timer;
  struct timespec ts;
  int err;

  if( NULL != tmr_head_ptr ) {
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    clock_gettime( SNS_TIMER_CLOCK, &ts );
    if( -1 == compare_ts( &ts, &tmr_head_ptr->timer_expry ) ) {
      ts = sub_ts( &tmr_head_ptr->timer_expry, &ts );

      timer.it_value.tv_sec = ts.tv_sec;
      timer.it_value.tv_usec = NSEC_TO_USEC(ts.tv_nsec);
      timer.it_value.tv_sec += (timer.it_value.tv_usec / USEC_PER_SEC);
      timer.it_value.tv_usec %= USEC_PER_SEC;
    } else {
      timer.it_value.tv_sec = 0;
      timer.it_value.tv_usec = 1;
    }

    err = setitimer( ITIMER_REAL, &timer, NULL );
    if( -1 == err ) {
      SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_EM,
                                "setitimer err %d",
                                errno );
    }
  }
}

/*===========================================================================

  FUNCTION:   link_tmr

  ===========================================================================*/
/*!
  @brief Adds a new timer to the timer linked list. This will also remove the
  timer from the list, if it's already on it.

  @param[i] t_ptr: Pointer to the timer to add.

  @return
  None

  @dependencies
  Requires that the mutex "sns_em_mutex" be locked before calling.

*/
/*=========================================================================*/
static void link_tmr( struct sns_em_tmr_s *t_ptr )
{
  struct sns_em_tmr_s *index_ptr, *prev_ptr;

  unlink_tmr( t_ptr );

  if( NULL == tmr_head_ptr ) {
    tmr_head_ptr = t_ptr;
  } else {

    if( 1 == compare_ts( &tmr_head_ptr->timer_expry, &t_ptr->timer_expry ) ) {
      t_ptr->link = tmr_head_ptr;
      tmr_head_ptr = t_ptr;
    } else {
      /* We can safely say that we are going to add this pointer at least after the head */
      prev_ptr = tmr_head_ptr;
      index_ptr = prev_ptr->link;

      while( index_ptr != NULL &&  // 'this' is not the last element
             ( 1 != compare_ts( &index_ptr->timer_expry, &t_ptr->timer_expry ) ) )  // the new timer expires later than 'this'
      {
          prev_ptr  = index_ptr;
          index_ptr = prev_ptr->link;
      }
      t_ptr->link = index_ptr;
      prev_ptr->link = t_ptr;

    }
  }

}

/*===========================================================================

  FUNCTION:   call_tmrs

  ===========================================================================*/
/*!
  @brief Calls all timer callbacks which have expired.

  @return
  None

  @sideeffects
  Removes the called timer from the timer list.
  Resets the SIGALRM timer if there are more timers in the list.
*/
/*=========================================================================*/
static void call_tmrs( void )
{
  struct sns_em_tmr_s *index_ptr;
  uint8_t  err;
  struct timespec now;
  void  (*cb)(void *);
  void *cb_arg;

  sns_os_mutex_pend( sns_em_mutex, 0, &err );

  clock_gettime( SNS_TIMER_CLOCK, &now );

  while( (NULL != tmr_head_ptr) &&
         0 <= compare_ts( &now, &tmr_head_ptr->timer_expry ) ) {

    index_ptr = tmr_head_ptr;
    unlink_tmr( index_ptr );
    if( 0 != index_ptr->period_ticks ) {
      index_ptr->timer_expry = add_usec_to_ts( &index_ptr->timer_expry,
                                               index_ptr->period_ticks );
      link_tmr( index_ptr );
    }
    cb = index_ptr->CallBack;
    cb_arg = index_ptr->callback_arg;
    sns_os_mutex_post( sns_em_mutex );

    cb( cb_arg );

    sns_os_mutex_pend( sns_em_mutex, 0, &err );
  }
  set_next_timer();
  sns_os_mutex_post( sns_em_mutex );
}

/*===========================================================================

  FUNCTION:   sns_em_task

  ===========================================================================*/
/*!
  @brief Main thread loop for EM. Handles the SIGALRM signal by calling pending
  timer callback functions.

  @param[i] unused: unused.

  @return
  None

  @sideeffects
  None
*/
/*=========================================================================*/
// This function runs in a separate thread
void sns_em_task( void *unused )
{
  sigset_t set;
  int      sig;
  uint8_t  err;

  UNREFERENCED_PARAMETER(unused);

  sigemptyset( &set );
  sigaddset( &set, SIGALRM );

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_EM,
                           "EM thread started" );

  sns_em_mutex = sns_os_mutex_create( SNS_MODULE_PRI_APPS_EM_MUTEX,
                                      &err );
  sns_em_initialized = true;
  sns_init_done();

  while( true ) {
    sigwait( &set, &sig );
    if( sns_em_initialized == false ) {
      break;
    }
    call_tmrs();
  }
  // TODO: explicitly delete thread?
}

/*============================================================================
  Externalized Function Definitions
  ===========================================================================*/

/*===========================================================================

  FUNCTION:   sns_em_get_timestamp

  ===========================================================================*/
uint32_t sns_em_get_timestamp(void)
{
  struct timespec ts;

  if( 0 == clock_gettime( SNS_TS_CLOCK, &ts ) ) {
    return( SEC_TO_USEC((uint32_t)ts.tv_sec) + NSEC_TO_USEC(ts.tv_nsec) );
  } else {
    return 0;
  }
}

/*===========================================================================

  FUNCTION:   sns_em_get_timestamp64

  ===========================================================================*/
sns_err_code_e
sns_em_get_timestamp64( uint64_t* timestamp_p )
{
  struct timespec ts;
  uint64_t time;

  if( NULL == timestamp_p || 0 != clock_gettime( SNS_TS_CLOCK, &ts ) ) {
    return SNS_ERR_FAILED;
  }

  time = ( SEC_TO_USEC((uint64_t)ts.tv_sec) +
           NSEC_TO_USEC((uint64_t)ts.tv_nsec) );

  SNS_OS_MEMCOPY( timestamp_p, &time, sizeof(uint64_t) );

  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_em_create_timer_obj

  ===========================================================================*/
sns_err_code_e sns_em_create_timer_obj( void (*callback)(void*),
                                        void *callback_arg,
                                        sns_em_timer_type_e timer_type,
                                        sns_em_timer_obj_t* timer_obj_ptr)
{
  struct sns_em_tmr_s *tmr_ptr;

  tmr_ptr = malloc(sizeof(struct sns_em_tmr_s));
  *timer_obj_ptr = tmr_ptr;

  if( NULL != tmr_ptr ) {
    tmr_ptr->CallBack = callback;
    tmr_ptr->callback_arg = callback_arg;
    if( SNS_EM_TIMER_TYPE_PERIODIC == timer_type ) {
      // Set period_ticks to any non-zero value
      tmr_ptr->period_ticks = 1;
    } else {
      tmr_ptr->period_ticks = 0;
    }
    tmr_ptr->link = NULL;
    return SNS_SUCCESS;
  }
  return SNS_ERR_NOMEM;
}

/*===========================================================================

  FUNCTION:   sns_em_register_timer

  ===========================================================================*/
sns_err_code_e sns_em_register_timer(struct sns_em_tmr_s * timer_obj_ptr,
                                     uint32_t delta_tick_time) /* microseconds */
{
  uint8_t err;
  struct timespec ts;

  clock_gettime( SNS_TIMER_CLOCK, &ts );

  sns_os_mutex_pend( sns_em_mutex, 0, &err );

  timer_obj_ptr->timer_expry = add_usec_to_ts( &ts, delta_tick_time );
  if( 0 != timer_obj_ptr->period_ticks ) {
    // Set period_ticks to the delta time
    timer_obj_ptr->period_ticks = delta_tick_time;
  }

  link_tmr( timer_obj_ptr );
  set_next_timer();

  sns_os_mutex_post( sns_em_mutex );

  return SNS_SUCCESS;
}


/*===========================================================================

  FUNCTION:   sns_em_cancel_timer

  ===========================================================================*/
sns_err_code_e sns_em_cancel_timer (struct sns_em_tmr_s * timer_obj_ptr)
{
  uint8_t err;

  sns_os_mutex_pend( sns_em_mutex, 0, &err );
  unlink_tmr( timer_obj_ptr );
  sns_os_mutex_post( sns_em_mutex );

  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_em_delete_timer

  ===========================================================================*/
sns_err_code_e sns_em_delete_timer_obj (struct sns_em_tmr_s * timer_obj_ptr)
{
  uint8_t err;

  sns_os_mutex_pend( sns_em_mutex, 0, &err );
  unlink_tmr( timer_obj_ptr );
  sns_os_mutex_post( sns_em_mutex );

  free( timer_obj_ptr );
  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_em_convert_dspstick_to_usec

  ===========================================================================*/
uint32_t sns_em_convert_dspstick_to_usec (uint32_t dsps_tick)
{
  int64_t rv; // return value

  rv = ((int64_t)dsps_tick * USEC_PER_SEC)/DSPS_HZ;

  return (uint32_t)rv;
}

/*===========================================================================

  FUNCTION:   sns_em_convert_usec_to_dspstick

  ===========================================================================*/
uint32_t sns_em_convert_usec_to_dspstick (uint32_t usec_time)
{
  int64_t rv; // Return value

  rv = ((int64_t)usec_time * DSPS_HZ)/USEC_PER_SEC;

  return (uint32_t)rv;
}

/*===========================================================================

  FUNCTION:   sns_em_convert_dspstick_to_appstime

  ===========================================================================*/
uint32_t sns_em_convert_dspstick_to_appstime (uint32_t dsps_tick);

/*===========================================================================

  FUNCTION:   sns_em_convert_localtick_to_usec

  ===========================================================================*/
uint32_t sns_em_convert_localtick_to_usec( uint32_t local_tick )
{
  return local_tick;
}

/*===========================================================================

  FUNCTION:   sns_em_convert_usec_to_localtick

  ===========================================================================*/
uint32_t sns_em_convert_usec_to_localtick( uint32_t usec_time )
{
  return usec_time;
}

/*===========================================================================

  FUNCTION:   sns_em_init

  ===========================================================================*/
sns_err_code_e sns_em_init( void )
{
  sns_os_task_create( sns_em_task, NULL, NULL, SNS_MODULE_PRI_APPS_EM );

  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_em_deinit

  ===========================================================================*/
sns_err_code_e sns_em_deinit( void )
{
    sns_em_initialized = false;
    raise( SIGALRM );
    sns_os_task_del_req(SNS_MODULE_PRI_APPS_EM);
    return SNS_SUCCESS;
}

/*=====================================================================================
  FUNCTION:  sns_em_convert_sec_in_q16_to_localtick
=====================================================================================*/
uint32_t sns_em_convert_sec_in_q16_to_localtick( uint32_t time_sec_q16 )
{
  static const uint64_t usecPerSecQ16 = FX_CONV_Q16(((uint64_t)USEC_PER_SEC), 0);
  uint64_t repPeriodQ32 = (uint64_t)time_sec_q16 * usecPerSecQ16;
  uint32_t repPerioduSec =
    (uint32_t)(FX_CONV(repPeriodQ32, (FX_QFACTOR + FX_QFACTOR), 0));

  return repPerioduSec;
}

/*=====================================================================================
  FUNCTION:  sns_em_create_utimer_obj
=====================================================================================*/
sns_err_code_e sns_em_create_utimer_obj( void (*timer_cb)(void *),
                                         void *timer_cb_arg,
                                         sns_em_timer_type_e timer_category,
                                         sns_em_timer_obj_t* timer_obj_ptr )
{
  return(sns_em_create_timer_obj(timer_cb,
                                 timer_cb_arg,
                                 timer_category,
                                 timer_obj_ptr));
}

/*=====================================================================================
  FUNCTION:  sns_em_delete_utimer
=====================================================================================*/
sns_err_code_e sns_em_delete_utimer_obj( sns_em_timer_obj_t timer_obj )
{
  return(sns_em_delete_timer_obj(timer_obj));
}

/*=====================================================================================
  FUNCTION:  sns_em_register_utimer
=====================================================================================*/
sns_err_code_e sns_em_register_utimer( sns_em_timer_obj_t  timer_obj,
                                      uint32_t            delta_tick_time )
{
  return(sns_em_register_timer(timer_obj, delta_tick_time));
}

/*=====================================================================================
  FUNCTION:  sns_em_cancel_utimer
=====================================================================================*/
sns_err_code_e sns_em_cancel_utimer( sns_em_timer_obj_t timer_obj )
{
  return(sns_em_cancel_timer(timer_obj));
}

