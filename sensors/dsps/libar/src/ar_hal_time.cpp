/*============================================================================
  @file ar_hal_time.cpp

  @brief
  Time synchronization component of the AR HAL.  Ensures HAL samples
  contain correct LA timestamps, given input data from SSC.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*===========================================================================
                           INCLUDE FILES
===========================================================================*/
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <utils/SystemClock.h>

#define __bool_true_false_are_defined 1

#include "ar_hal.h"
#include "sns_time_api_v02.h"
#include "sensor1.h"

/*===========================================================================
                   DATA TYPES
===========================================================================*/

typedef struct hal_time_control_t {
  sensor1_handle_s  *sensor1_hndl;
  int64_t           timestamp_offset_apps_dsps;     /* Offset (in ns) between apps and dsps timestamps */
  uint32_t          dsps_ts_last;                   /* Last DSPS timestamp received */
  uint32_t          dsps_rollover_cnt;              /* # of times the DSPS clock has "rolled over" and restarted at 0 */
  uint32_t          dsps_rollover_cnt_rcv;          /* Rollover count as received in last time service report */
  pthread_mutex_t   time_mutex;                     /* mutex lock for time service sensor1 callback */
  pthread_cond_t    time_cond;                      /* cond variable to signal time svc resp has arrived */
  int64_t           timestamp_last_sent[MAX_NUM_ACTIVITIES]; /* The timestamp on the last sample sent to Android */
  bool              is_resp_arrived;                /* flag to indicate callback has arrived */
  int64_t           boot_ts_last_rollover;          /* Boottime timestamp of the last predicted rollover event */
} hal_time_control_t;

/*===========================================================================
                   PREPROCESSOR DEFINTIONS
===========================================================================*/
#define MAX_CLOCK_DIFF_NS 10000

/*===========================================================================
                         STATIC VARIABLES
===========================================================================*/
static hal_time_control_t     *g_time_control = NULL;    /* Time service-related data */

/*==========================================================================
                         STATIC FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  hal_rollover_cnt_is
===========================================================================*/
/*!
 * Update local rollover count state.
*/
static void
hal_rollover_cnt_is( uint32_t rollover_cnt )
{
  if( 0 == g_time_control->dsps_rollover_cnt &&
      0 == g_time_control->dsps_ts_last )
  {
    g_time_control->dsps_rollover_cnt = rollover_cnt;
  }
  g_time_control->dsps_rollover_cnt_rcv = rollover_cnt;
}

/*===========================================================================
  FUNCTION:  hal_ts_offset_is
===========================================================================*/
/*!
 * Update local timestamp offset state.
*/
static void
hal_ts_offset_is( uint32_t timestamp_ssc, uint64_t timestamp_ap )
{
  uint64_t ssc_ns;
  uint64_t elapsed_realtime_ns;
  struct timespec ts_start, ts_end;
  uint64_t boottime_ns, boottime_start, boottime_end;

  /* Figure out the difference between the CLOCK_BOOTTIME time and
     elapsedRealtimeNano */
  do
  {
    clock_gettime( CLOCK_BOOTTIME, &ts_start );
    elapsed_realtime_ns = android::elapsedRealtimeNano();
    clock_gettime( CLOCK_BOOTTIME, &ts_end );
    boottime_start = ts_start.tv_sec * NSEC_PER_SEC + ts_start.tv_nsec;
    boottime_end = ts_end.tv_sec * NSEC_PER_SEC + ts_end.tv_nsec;
    HAL_LOG_DEBUG( "%s: boottime_end - boottime_start: %llu", __FUNCTION__, boottime_end - boottime_start );
  }while( boottime_end - boottime_start > MAX_CLOCK_DIFF_NS );

  boottime_ns = (boottime_start + boottime_end) / 2;

  ssc_ns = (uint64_t)(((uint64_t)timestamp_ssc * NSEC_PER_SEC)/DSPS_HZ);
  g_time_control->timestamp_offset_apps_dsps = timestamp_ap - ssc_ns - boottime_ns + elapsed_realtime_ns;

  HAL_LOG_DEBUG( "%s: Apps: %"PRIu64"; DSPS: %u; Offset: %"PRId64,
                 __FUNCTION__,  timestamp_ap, timestamp_ssc,
                 g_time_control->timestamp_offset_apps_dsps );
}

/*===========================================================================
  FUNCTION:  hal_process_time_resp
===========================================================================*/
/*!
 * Process response message from the time sync service.
*/
static void
hal_process_time_resp( sensor1_msg_header_s const *msg_hdr,
    sns_time_timestamp_resp_msg_v02 const *msg_ptr )
{
  UNREFERENCED_PARAMETER(msg_hdr);
  bool error = false;

  if( 0 == msg_ptr->resp.sns_result_t )
  {
    if( true == msg_ptr->dsps_rollover_cnt_valid )
    {
      hal_rollover_cnt_is( msg_ptr->dsps_rollover_cnt );
    }

    if( true == msg_ptr->timestamp_dsps_valid &&
        true == msg_ptr->timestamp_apps_boottime_valid )
    {
      hal_ts_offset_is( msg_ptr->timestamp_dsps, msg_ptr->timestamp_apps_boottime );
    }
    else if( true == msg_ptr->error_code_valid )
    {
      HAL_LOG_ERROR( "%s: Error in RESP: %i", __FUNCTION__, msg_ptr->error_code );
      error = true;
    }
    else
    {
      HAL_LOG_ERROR( "%s: Unknown error in RESP. DSPS ts valid: %i; APPS: %i APPS boottime: %i",
                     __FUNCTION__, msg_ptr->timestamp_dsps_valid, msg_ptr->timestamp_apps_valid,
                     msg_ptr->timestamp_apps_boottime_valid);
      error = true;
    }
  }
  else
  {
      HAL_LOG_ERROR( "%s: Received 'Failed' in response result", __FUNCTION__ );
      error = true;
  }

  g_time_control->is_resp_arrived = true;
  pthread_cond_signal( &g_time_control->time_cond );
}

/*===========================================================================
  FUNCTION:  hal_process_time_ind
===========================================================================*/
/*!
 * Process indication message from the time sync service.
*/
static void
hal_process_time_ind( sensor1_msg_header_s const *msg_hdr,
    sns_time_timestamp_ind_msg_v02 const *msg_ptr )
{
  UNREFERENCED_PARAMETER(msg_hdr);
  hal_rollover_cnt_is( msg_ptr->dsps_rollover_cnt );
  if( msg_ptr->timestamp_apps_boottime_valid )
  {
    hal_ts_offset_is( msg_ptr->timestamp_dsps, msg_ptr->timestamp_apps_boottime );
  }
}

/*===========================================================================
  FUNCTION:  hal_time_data_cb
===========================================================================*/
/*!
*/
static void
hal_time_data_cb( intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
    sensor1_msg_type_e msg_type, void *msg_ptr )
{
  UNREFERENCED_PARAMETER(cb_data);
  sensor1_error_e error;
  HAL_LOG_DEBUG( "%s: msg_type %d", __FUNCTION__, msg_type );

  if( msg_hdr != NULL )
  {
    HAL_LOG_DEBUG( "%s: Sn %d, msg Id %d, txn Id %d", __FUNCTION__,
         msg_hdr->service_number, msg_hdr->msg_id, msg_hdr->txn_id );
  }
  else
  {
    HAL_LOG_DEBUG( "%s: Ignoring message (type %u)", __FUNCTION__, msg_type );
    return ;
  }

  pthread_mutex_lock( &g_time_control->time_mutex );

  if( SENSOR1_MSG_TYPE_BROKEN_PIPE == msg_type )
  {
    HAL_LOG_VERBOSE("%s: SENSOR1_MSG_TYPE_BROKEN_PIPE", __FUNCTION__);

    // Re-init these values from from hal_time_stop()
    // The pipe is already broken so we only need to clean up
    g_time_control->dsps_rollover_cnt = 0;
    g_time_control->dsps_ts_last = 0;
    g_time_control->boot_ts_last_rollover = 0;
    g_time_control->sensor1_hndl = NULL;
  }
  else if (SENSOR1_MSG_TYPE_RETRY_OPEN == msg_type )
  {
    HAL_LOG_VERBOSE("%s: SENSOR1_MSG_TYPE_RETRY_OPEN", __FUNCTION__);

    // Re-try sensor1_open()
    error = sensor1_open( &g_time_control->sensor1_hndl, &hal_time_data_cb, (intptr_t)NULL );
    HAL_LOG_VERBOSE( "%s: sensor1 open: %d %"PRIuPTR, __FUNCTION__, error, (uintptr_t)g_time_control->sensor1_hndl );
  }
  else if( SENSOR1_MSG_TYPE_RESP == msg_type &&
             SNS_TIME_TIMESTAMP_RESP_V02 == msg_hdr->msg_id )
  {
    hal_process_time_resp( msg_hdr, (sns_time_timestamp_resp_msg_v02*)msg_ptr );
  }
  else if( SENSOR1_MSG_TYPE_IND == msg_type )
  {
    hal_process_time_ind( msg_hdr, (sns_time_timestamp_ind_msg_v02*)msg_ptr );
  }
  else
  {
    HAL_LOG_WARN( "%s: Received unknown message type %i, id %i",
                  __FUNCTION__, msg_type, msg_hdr->msg_id );
  }

  if( NULL != msg_ptr && g_time_control->sensor1_hndl )
  {
    sensor1_free_msg_buf( g_time_control->sensor1_hndl, msg_ptr );
  }
  pthread_mutex_unlock( &g_time_control->time_mutex );
}

/*==========================================================================
                         PUBLIC FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  hal_timestamp_calc
===========================================================================*/
int64_t
hal_timestamp_calc( uint64_t dsps_timestamp, uint32_t activity )
{
  int64_t rv = g_time_control->timestamp_offset_apps_dsps +
               ((dsps_timestamp * NSEC_PER_SEC)/DSPS_HZ);
  int32_t rollover_diff;

  pthread_mutex_lock( &g_time_control->time_mutex );

  if( (dsps_timestamp < TS_ROLLOVER_THRESH) &&
      (UINT32_MAX - g_time_control->dsps_ts_last < TS_ROLLOVER_THRESH) )
  {
    /* If a roll-over is predicted, check the boottime timestamp of the last
     * predicted roll-over against the current boottime timestamp. If the
     * difference between the two times is not greater than
     * BOOT_TS_ROLLOVER_THRESH, then there most likely was just jitter in the
     * incoming DSPS timestamps instead of a real clock roll-over event. */
    struct timespec current_time;
    int64_t current_boot_ts;

    int time_err = clock_gettime( CLOCK_BOOTTIME, &current_time );
    if( 0 != time_err )
    {
      time_err = errno;
      HAL_LOG_ERROR("%s: Error with clock_gettime: %i", __FUNCTION__, time_err );
    }
    else
    {
      current_boot_ts = ((int64_t)current_time.tv_sec * 1000000000) + current_time.tv_nsec;

      HAL_LOG_WARN( "%s: potential TS rollover detected. DSPS TS: %"PRIu64", last DSPS: %u, boot TS: %"PRIi64", last boot: %"PRIi64"",
            __FUNCTION__, dsps_timestamp, g_time_control->dsps_ts_last,
            current_boot_ts, g_time_control->boot_ts_last_rollover );

      if( current_boot_ts - g_time_control->boot_ts_last_rollover > BOOT_TS_ROLLOVER_THRESH )
      {
        /* If a roll-over has likely occurred */
        g_time_control->dsps_rollover_cnt++;

        /* Record the boottime timestamp */
        g_time_control->boot_ts_last_rollover = current_boot_ts;

        HAL_LOG_WARN( "%s: TS rollover confirmed. cnt: %u, rcv: %u",
              __FUNCTION__, g_time_control->dsps_rollover_cnt,
              g_time_control->dsps_rollover_cnt_rcv );
      }
    }
  }

  /* If the # of rollovers determined by the HAL is different than in the
   * last message received from the time service, adjust the timestamp accordingly */
  rollover_diff = g_time_control->dsps_rollover_cnt - g_time_control->dsps_rollover_cnt_rcv;
  if( (0 < rollover_diff && dsps_timestamp < TS_ROLLOVER_THRESH) ||
      (0 > rollover_diff && dsps_timestamp > UINT32_MAX - TS_ROLLOVER_THRESH) )
  {
    HAL_LOG_WARN( "%s: Adjusting timestamp for rollover: %"PRIu64", %i", __FUNCTION__,
        rv, rollover_diff );
    rv += (rollover_diff * UINT32_MAX) * NSEC_PER_SEC / DSPS_HZ;
  }

  /* Ensure samples are sent to LA with increasing timestamps */
  if( g_time_control->timestamp_last_sent[ activity ] > rv &&
      abs(g_time_control->timestamp_last_sent[ activity ] - rv) < TS_CORRECT_THRESH )
  {
    HAL_LOG_WARN( "%s: Adjusting timestamp to maintain ascension: %"PRIu64", %"PRIu64, __FUNCTION__,
        rv, g_time_control->timestamp_last_sent[ activity ] );
    rv = g_time_control->timestamp_last_sent[ activity ] + 1;
  }

  g_time_control->dsps_ts_last = dsps_timestamp;
  g_time_control->timestamp_last_sent[ activity ] = rv;

  pthread_mutex_unlock( &g_time_control->time_mutex );

  return rv;
}

/*===========================================================================
  FUNCTION:  hal_time_init
===========================================================================*/
int hal_time_init()
{
  pthread_mutexattr_t attr;
  int i;
  int rv = 0;

  if( NULL == g_time_control )
  {
    g_time_control = (hal_time_control_t*)malloc( sizeof(hal_time_control_t) );
    if( NULL == g_time_control )
    {
      HAL_LOG_ERROR( "%s: ERROR: malloc error", __FUNCTION__ );
      rv = -1;
    }
    else
    {
      for( i = 0; i < MAX_NUM_ACTIVITIES; i++ )
      {
        g_time_control->timestamp_last_sent[ i ] = 0;
      }

      g_time_control->sensor1_hndl = NULL;
      g_time_control->dsps_ts_last = 0;
      g_time_control->dsps_rollover_cnt = 0;
      g_time_control->dsps_rollover_cnt_rcv = 0;
      g_time_control->timestamp_offset_apps_dsps = 0;
      g_time_control->boot_ts_last_rollover = 0;

      pthread_mutexattr_init( &attr );
      pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
      pthread_mutex_init( &(g_time_control->time_mutex), &attr );
      pthread_cond_init( &(g_time_control->time_cond), NULL );
      pthread_mutexattr_destroy( &attr );
    }
  }

  return rv;
}

/*===========================================================================
  FUNCTION:  hal_time_stop
===========================================================================*/
int hal_time_stop()
{
  pthread_mutex_lock( &g_time_control->time_mutex );

  if( NULL != g_time_control->sensor1_hndl )
  {
    HAL_LOG_INFO( "%s: closing sensor1...", __FUNCTION__ );
    sensor1_close( g_time_control->sensor1_hndl );
    g_time_control->sensor1_hndl = NULL;
  }

  g_time_control->dsps_rollover_cnt = 0;
  g_time_control->dsps_ts_last = 0;
  g_time_control->boot_ts_last_rollover = 0;

  pthread_mutex_unlock( &g_time_control->time_mutex );

  return 0;
}

/*===========================================================================
  FUNCTION:  hal_time_start
===========================================================================*/
int
hal_time_start()
{
  sensor1_error_e error;
  sensor1_msg_header_s msgHdr;
  sns_time_timestamp_req_msg_v02 *msg_ptr = NULL;
  bool rv = 0;

  pthread_mutex_lock( &g_time_control->time_mutex );

  if( NULL == g_time_control->sensor1_hndl )
  {
    sensor1_init();
    error = sensor1_open( &g_time_control->sensor1_hndl, &hal_time_data_cb, (intptr_t)NULL );
    HAL_LOG_VERBOSE( "%s: sensor1 open: %d %"PRIuPTR, __FUNCTION__, error, (uintptr_t)g_time_control->sensor1_hndl );
    if( SENSOR1_SUCCESS != error )
    {
      rv = -1;
    }
    else
    {
      g_time_control->dsps_rollover_cnt = 0;
      g_time_control->dsps_ts_last = 0;
      g_time_control->boot_ts_last_rollover = 0;

      error = sensor1_alloc_msg_buf( g_time_control->sensor1_hndl,
          sizeof(sns_time_timestamp_req_msg_v02), (void**)&msg_ptr );
      if( SENSOR1_SUCCESS != error )
      {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf returned(get) %d",
                       __FUNCTION__, error );
      }
      else
      {
        msgHdr.service_number = SNS_TIME2_SVC_ID_V01;
        msgHdr.msg_id = SNS_TIME_TIMESTAMP_REQ_V02;
        msgHdr.msg_size = sizeof(sns_time_timestamp_req_msg_v02);
        msgHdr.txn_id = 1;

        msg_ptr->reg_report_valid = true;
        msg_ptr->reg_report = true;

        error = sensor1_write( g_time_control->sensor1_hndl, &msgHdr, msg_ptr );
        if( SENSOR1_SUCCESS != error )
        {
          HAL_LOG_ERROR( "%s: sensor1_write returned %d", __FUNCTION__, error );
          sensor1_free_msg_buf( g_time_control->sensor1_hndl, msg_ptr );
          rv = -1;
        }
        else if( false == hal_wait_for_response(
              TIME_OUT_MS, &g_time_control->time_mutex,
              &g_time_control->time_cond, &g_time_control->is_resp_arrived ) )
        {
          HAL_LOG_ERROR( "%s: ERROR: No response from request %d",
                         __FUNCTION__, SNS_TIME_TIMESTAMP_REQ_V02 );
          rv = -1;
        }
      }
    }
  }

  if( 0 != rv )
  {
    hal_time_stop();
  }
  pthread_mutex_unlock( &g_time_control->time_mutex );

  return rv;
}
