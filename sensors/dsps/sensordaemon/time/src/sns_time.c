/*============================================================================
  @file sns_time.c

  @brief
    This is the implementation of the Sensors Time Service. It allows clients
    request the current time stamp values from several sourcesm.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#include "sensor1.h"
#include "sns_common.h"
#include "sns_time_priv.h"
#include "fixed_point.h"
#include "sns_debug_str.h"
#include "sns_osa.h"
#include "sns_smr_util.h"
#include "sns_memmgr.h"
#include "sns_init.h"
#include "sns_pwr.h"
#include "sns_em.h"
#include "sns_log_api.h"
#include "sns_log_types.h"

#ifdef SNS_LA
#include <linux/msm_dsps.h>
#endif /* SNS_LA */

#include <time.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

/*============================================================================
                   INTERNAL DEFINITIONS AND TYPES
============================================================================*/

/* Timing related macros */
#define NSEC_PER_SEC       1000000000LL

/* At a MINimum, offsets should be calculated this often (ns) */
#define OFFSET_CALC_RATE_MIN 10000000000LL

/* At a MAXimum, offsets should be calculated this often (ns) */
#define OFFSET_CALC_RATE_MAX 100000000LL

/* Ideally, log when the current offset is X ns different than last log */
#define OFFSET_CALC_DIFF 90000

/* Number of iterations to request timestamps to calculate the average */
#define OFFSET_CALC_ITER 10

/* Drop outliers that are more than X ns away from the median */
#define OFFSET_CALC_OUTLIER 200

/* Consider the DSPS clock to have rolled-over when one timestamp is less than X
 * fewer ns than the previous */
#define TS_ROLLOVER_THRESH 1000000

/* The maximum number of times the Time Service will attempt to recalculate
 * the adsp-apps offset upon predicting an adsp timestamp rollover event */
#define MAX_OFFSET_RECALC_ITERATIONS 4

/* Number of samples to average over for filtering */
#ifdef _WIN32
#define OFFSET_FILTER_COUNT (uint32_t)(1000 / 31) /* Numbers are from WP8 1000us system timer resolution & 31us DSPS clock resolution */
#else /* _WIN32 */
#define OFFSET_FILTER_COUNT 1
#endif /* else _WIN32 */

/* Sensors device */
#define SENSORS_DEV "/dev/sensors"

/*============================================================================
  Type Declarations
  ==========================================================================*/

typedef struct sns_time_client
{
  intptr_t client_hndl;
  struct sns_time_client *next;
} sns_time_client;

typedef struct
{
  int64_t  apps_dsps_offset;  /* Offset between apps timestamp
                                  and DSPS ticks (in ns). */
  int64_t  offset_calc_rate_curr; /* Current rate to calculate offsets */
  sns_em_timer_obj_t sns_time_calc_tmr; /* Timer indication the next
                                           calculation is due */
  sns_time_client *time_clients;

  int64_t  dsps_last_ts;  /* Last DSPS timestamp acquired (in ns) */
  uint32_t dsps_rollover_cnt; /* Number of times the DSPS clock has
                                 "rolled over" and restarted at 0 */
  OS_EVENT *sns_time_mutex; /* Mutex protecting sns_time_control_s members */

  int64_t offset_history[OFFSET_FILTER_COUNT]; /* Used for filtering offsets */
  uint8_t offset_history_count;
  uint8_t offset_history_idx;
  int fd; /* device handle */
} sns_time_control_s;

/*============================================================================
  Static Variable Definitions
  ===========================================================================*/
static sns_time_control_s *g_time_control = (sns_time_control_s*)NULL;

/*============================================================================
  Static Function Definitions
  ==========================================================================*/

/*!
  @brief Searches the client list for the specified client.

  @param[i] client_hndl Opaque and unique client handle

  @return Client struct if found, NULL otherwise.
*/
static sns_time_client*
sns_time_client_lookup( intptr_t client_hndl )
{
  sns_time_client *client = g_time_control->time_clients;
  while( NULL != client )
  {
    if( client->client_hndl == client_hndl )
    {
      break;
    }
    client = client->next;
  }

  return client;
}

/*!
  @brief Determines the number of active clients to the time service.

  @return Number of clients
*/
static int
sns_time_client_count()
{
  int count = 0;
  sns_time_client *client = g_time_control->time_clients;
  while( NULL != client )
  {
    count++;
    client = client->next;
  }

  return count;
}

/*!
  @brief Converts a timespec structure to a nanoseconds timestamp.

  @param[i] ts_apps Time to convert
  @return Apps timestamp; nanoseconds since last epoch.
*/
static int64_t
sns_time_conv_apps( struct timespec ts_apps )
{
  return ((int64_t)ts_apps.tv_sec * 1000000000) + ts_apps.tv_nsec;
}

/*!
  @brief Converts a timestamp (in ns) from the apps processor to an equivalent
         timestamp on the DSPS (in clock ticks).

  @param[i] ts_apps Time stamp from Apps (ns)
  @return Number of DSPS ticks since reboot at the given apps time.
*/
static uint32_t
sns_time_conv_dsps( int64_t ts_apps )
{
  int64_t dsps_ns = (g_time_control->apps_dsps_offset != 0)
    ? ts_apps - g_time_control->apps_dsps_offset
    : 0;
  return (uint32_t)((dsps_ns * DSPS_SLEEP_CLK) / NSEC_PER_SEC);
}

/*!
  @brief Standard compare function for use in qsort.

  @param [i] a First value to compare
  @param [i] a Second value to compare
  @return The difference from a to b.
*/
static int
sns_time_compare_int64(const void *a, const void *b)
{
  return (int)(*(int64_t*)a - *(int64_t*)b);
}

/*!
  @brief Removes outliers and calculates the average (mean) value

  @param [i/o] apps_dsps_offset Array of offsets (in ns)
  @param [i] len Number of entries in apps_dsps_offset
  @return The calculated offset average (in ns).
*/
static int64_t
sns_time_average( int64_t *apps_dsps_offset, uint8_t len )
{
  int64_t median, mean;
  uint8_t i;

  // Sort the values
  qsort( apps_dsps_offset, len, sizeof(int64_t), sns_time_compare_int64 );

  // Choose the median
  median = apps_dsps_offset[ len / 2 ];

  // Remove items more than OFFSET_CALC_OUTLIER away from the median
  for( i = 0 ; i < len; )
  {
    if( median + OFFSET_CALC_OUTLIER > apps_dsps_offset[ i ]
        && median - OFFSET_CALC_OUTLIER < apps_dsps_offset[ i ] )
    {
      i++;
    }
    else
    {
      apps_dsps_offset[ i ] = apps_dsps_offset[ len - 1 ];
      len--;
    }
  }

  // Calculate and return the mean
  // We could just add all the numbers and divide, but this would
  // overflow after 548/len years from the unix epoch.
  mean = apps_dsps_offset[ 0 ]/len;
  for( i = 1; i < len; i++ )
  {
    mean = mean + apps_dsps_offset[ i ]/len;
  }

  // Return the average of previous offsets.
  // Average size (ie, OFFSET_FILTER_COUNT) should be set based on the resolution
  // of the system clock.
  g_time_control->offset_history[g_time_control->offset_history_idx++] = mean;
  if(g_time_control->offset_history_count < g_time_control->offset_history_idx)
  {
      g_time_control->offset_history_count = g_time_control->offset_history_idx;
  }
  if(g_time_control->offset_history_idx == OFFSET_FILTER_COUNT)
  {
      g_time_control->offset_history_idx = 0;
  }

  len = g_time_control->offset_history_count;
  mean = g_time_control->offset_history[ 0 ]/len;
  for( i = 1; i < g_time_control->offset_history_count; i++ )
  {
    mean += g_time_control->offset_history[ i ]/len;
  }

  return mean;
}

/*!
  @brief Determines the current offset between the apps and dsps processors.

  @param[i] set_tmr Whether to register a timer callback, causing the offset
                    to be calculated again.
  @return The calculated offset: (apps time in ns) - (dsps time in ns)
*/
static int64_t
sns_time_calc_apps_dsps_offset( bool set_tmr )
{
  int64_t apps_dsps_offset[OFFSET_CALC_ITER];
  int64_t apps_dsps_offset_avg;
  int64_t timestamp_apps, timestamp_dsps[OFFSET_CALC_ITER];
  int i;
  sns_err_code_e sns_err;
  sns_log_time_report_s *log_pkt;
  uint8_t os_err;

  sns_os_mutex_pend( g_time_control->sns_time_mutex, 0, &os_err );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                               "Error acquiring mutex %i", os_err );
    return 0;
  }

  for( i = 0; i < OFFSET_CALC_ITER; i++ )
  {
    struct timespec current_time;
    int apps_err;
    int dsps_err = 0;

#if defined(SNS_LA)
    unsigned int dsps_clk_ticks;
    dsps_err = ioctl( g_time_control->fd, DSPS_IOCTL_READ_SLOW_TIMER, &dsps_clk_ticks);
#elif defined(_WIN32)
    unsigned int dsps_clk_ticks;
    dsps_err = read_dsps_timer((uint32_t*)&dsps_clk_ticks);
#endif

    apps_err = clock_gettime( CLOCK_BOOTTIME, &current_time );
    if( 0 != apps_err )
    {
#ifndef _WIN32
      apps_err = errno;
#endif /* !_WIN32 */
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                 "Apps time error %i",
                                 apps_err );
      timestamp_apps = 0;
    }
    else
    {
      timestamp_apps = sns_time_conv_apps( current_time );
    }

    if( 0 != dsps_err )
    {
#ifndef _WIN32
      dsps_err = errno;
#endif /* !_WIN32 */
      SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_TIME,
                                 "Error reading timer (fd=%d) err=%d",
                                 g_time_control->fd, dsps_err );
      timestamp_dsps[ i ] = 0;
    }
    else
    {
#if defined(SNS_LA_SIM)
      timestamp_dsps[ i ] = timestamp_apps;
#else
      timestamp_dsps[ i ] = (int64_t)(((int64_t)dsps_clk_ticks * NSEC_PER_SEC)/DSPS_SLEEP_CLK);
#endif
    }

    apps_dsps_offset[ i ] = timestamp_apps - timestamp_dsps[ i ];

    /* If the DSPS clock rolls-over during the calculation, start again */
    if( i > 0 && timestamp_dsps[ i ] < timestamp_dsps[ i - 1] )
    {
      i = 0;
      SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_TIME,
                                  "DSPS rollover, resetting" );
    }
  }

  apps_dsps_offset_avg = sns_time_average( apps_dsps_offset, (uint8_t)OFFSET_CALC_ITER );
  if( timestamp_dsps[ OFFSET_CALC_ITER - 1 ] < g_time_control->dsps_last_ts )
  {
    g_time_control->dsps_rollover_cnt++;
    SNS_PRINTF_STRING_MEDIUM_3( SNS_DBG_MOD_APPS_TIME,
                                "DSPS Clock Rollover! DSPS rollover cnt: %i, dsps_last_ts: %li, timestamp_dsps: %li",
                                g_time_control->dsps_rollover_cnt,
                                g_time_control->dsps_last_ts,
                                timestamp_dsps[ OFFSET_CALC_ITER - 1 ] );

  }
  g_time_control->dsps_last_ts = timestamp_dsps[ OFFSET_CALC_ITER - 1 ];

  sns_err = sns_logpkt_malloc( SNS_LOG_TIME_REPORT,
                               sizeof( sns_log_time_report_s ),
                               (void**)(&log_pkt) );
  if( (SNS_SUCCESS == sns_err) && (NULL != log_pkt) )
  {
    log_pkt->version = SNS_LOG_TIME_REPORT_VERSION;
    log_pkt->apps_dsps_offset = apps_dsps_offset_avg;
    log_pkt->apps_mdm_offset = 0;
    sns_err = sns_logpkt_commit( SNS_LOG_TIME_REPORT, log_pkt );
    if( sns_err != SNS_SUCCESS )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                 "Error logging time diag message %i",
                                 sns_err );
    }
  }

  SNS_PRINTF_STRING_LOW_3( SNS_DBG_MOD_APPS_TIME,
                           "sns_time offset difference %d, new_offset %d, old_offset %d",
                           (int32_t)(apps_dsps_offset_avg - g_time_control->apps_dsps_offset),
                           apps_dsps_offset_avg,
                           g_time_control->apps_dsps_offset);

  if( g_time_control->apps_dsps_offset + OFFSET_CALC_DIFF > apps_dsps_offset_avg
      && g_time_control->apps_dsps_offset - OFFSET_CALC_DIFF < apps_dsps_offset_avg )
  {
    g_time_control->offset_calc_rate_curr *= 2;

    if( g_time_control->offset_calc_rate_curr > OFFSET_CALC_RATE_MIN)
    {
      g_time_control->offset_calc_rate_curr = OFFSET_CALC_RATE_MIN;
    }
  }
  else
  {
    g_time_control->offset_calc_rate_curr /= 2;

    if( g_time_control->offset_calc_rate_curr < OFFSET_CALC_RATE_MAX)
    {
      g_time_control->offset_calc_rate_curr = OFFSET_CALC_RATE_MAX;
    }
  }
  g_time_control->apps_dsps_offset = apps_dsps_offset_avg;

  if( set_tmr )
  {
    sns_em_register_timer( g_time_control->sns_time_calc_tmr,
                           (uint32_t)(g_time_control->offset_calc_rate_curr / 1000) );
  }

  sns_os_mutex_post( g_time_control->sns_time_mutex );

  return apps_dsps_offset_avg;
}

/*!
  @brief Helper thread for sns_time_calc_cb

  @param p_arg Not used.
  @return None
*/
static void
sns_time_calc_thread( void *p_arg )
{
  UNREFERENCED_PARAMETER(p_arg);

  sns_time_calc_apps_dsps_offset( true );

  sns_time_client *client = g_time_control->time_clients;
  while( NULL != client )
  {
    sns_time_mr_send_ind( client->client_hndl );
    client = client->next;
  }
}

/*!
  @brief Callback indicating that the apps-dsps offset should be calculated
         again.

  @return None
*/
/*==========================================================================*/
static void
sns_time_calc_cb( void *cb_data )
{
  uint8_t os_err;
  static bool cleanup_old_thread = false;

  UNREFERENCED_PARAMETER(cb_data);

  /* The time service creates a thread from this callback.
   * If the service is stopped & restarted, the old thread
   * needs to be deleted. So delete the old thread before
   * starting a new one */
  if( cleanup_old_thread == true )
  {
      sns_os_task_del_req( SNS_MODULE_PRI_APPS_TIME2 );
  }

  os_err = sns_os_task_create( sns_time_calc_thread,
      NULL, NULL, SNS_MODULE_PRI_APPS_TIME2 );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_TIME,
                               "task create failed %i", os_err );
  }
  else
  {
    cleanup_old_thread = true;
  }
}

/*============================================================================
  Externalized Function Definitions
  ==========================================================================*/

sns_err_code_e
sns_time_client_add( intptr_t client_hndl )
{
  sns_time_client *client = sns_time_client_lookup( client_hndl );
  if( NULL == client )
  {
    client = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME, sizeof(sns_time_client) );
    if( NULL == client )
    {
      return SNS_ERR_NOMEM;
    }
    client->client_hndl = client_hndl;
    client->next = g_time_control->time_clients;
    g_time_control->time_clients = client;
  }

  return SNS_SUCCESS;
}

int
sns_time_client_delete( intptr_t client_hndl )
{
  sns_time_client *prev = NULL, *client = g_time_control->time_clients;

  while( NULL != client )
  {
    if( client->client_hndl == client_hndl )
    {
      if( NULL != prev )
      {
        prev->next = client->next;
      }
      else
      {
        g_time_control->time_clients = client->next;
      }
      SNS_OS_FREE( client );
      break;
    }
    prev = client;
    client = client->next;
  }

  if( 0 == sns_time_client_count() )
  {
    sns_em_cancel_timer( g_time_control->sns_time_calc_tmr );
    g_time_control->offset_history_count = 0;
    g_time_control->offset_history_idx = 0;
  }

  return 0;
}

int
sns_time_generate( uint64_t *timestamp_apps, uint32_t *timestamp_dsps,
                   uint32_t *dsps_rollover_cnt, uint64_t *timestamp_apps_boottime )
{
  int rv = 0;
  struct timespec current_time;
  struct timespec current_time_boottime;
  int time_err;
  int64_t dsps_ns;
  int i;

  /* If the time service was previously off, first recalculate */
  if( 0 == sns_time_client_count() )
  {
    sns_time_calc_apps_dsps_offset( true );
  }

  for (i = 0; i < MAX_OFFSET_RECALC_ITERATIONS; i++)
  {
    time_err = clock_gettime( CLOCK_REALTIME, &current_time );
    if( 0 != time_err )
    {
#ifndef _WIN32
      time_err = errno;
#endif /* !_WIN32 */
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                  "Error with clock_gettime: %i", time_err );
      rv = -1;
      break;
    }

    time_err = clock_gettime( CLOCK_BOOTTIME, &current_time_boottime );
    if( 0 != time_err )
    {
#ifndef _WIN32
      time_err = errno;
#endif /* !_WIN32 */
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                  "Error with clock_gettime: %i", time_err );
      rv = -1;
      break;
    }

    *timestamp_apps = sns_time_conv_apps( current_time );
    *timestamp_apps_boottime = sns_time_conv_apps( current_time_boottime );
    *timestamp_dsps = sns_time_conv_dsps( *timestamp_apps_boottime );
    dsps_ns = (int64_t)(((uint64_t)(*timestamp_dsps) * NSEC_PER_SEC)/DSPS_SLEEP_CLK);
    if( 0 == *timestamp_dsps )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME,
                                 "Offset uninitialized" );
      rv = -2;
      break;
    }
    else if( dsps_ns < g_time_control->dsps_last_ts )
    {
      SNS_PRINTF_STRING_HIGH_2( SNS_DBG_MOD_APPS_TIME,
                                "Rollover predicted. Old rollover count: %u, dsps ts: %lu",
                                g_time_control->dsps_rollover_cnt,
                                *timestamp_dsps );

      // Recalculate the apps-dsps offset to verify a rollover event
      sns_time_calc_apps_dsps_offset( false );
    }
    else
    {
      // Successful recalculation, break out of the for-loop
      break;
    }
  }

  if (MAX_OFFSET_RECALC_ITERATIONS == i)
  {
    SNS_PRINTF_STRING_ERROR_3( SNS_DBG_MOD_APPS_TIME,
                               "WARNING! DSPS-clock potentially stalled. apps-dsps_offset: %u, dsps ts: %lu, apps_ts: %llu",
                               g_time_control->apps_dsps_offset,
                               *timestamp_dsps,
                               *timestamp_apps );
    rv = -3;
  }

  *dsps_rollover_cnt = g_time_control->dsps_rollover_cnt;

  return rv;
}

/*============================================================================
  One time init function
  ==========================================================================*/

/*!
  @brief Initializes the Sensors Time service.

  Creates Time thread.
  Registers with SMR.

  @return
   - SNS_SUCCESS if initialization is successful.
   - All other values indicate an error has occurred.
*/
sns_err_code_e
sns_time_init()
{
  sns_err_code_e  smr_err;
  uint8_t         os_err;

  smr_err = sns_time_mr_init();
  if( SNS_SUCCESS != smr_err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_TIME,
                               "mr init failure %i", smr_err );
    return SNS_ERR_FAILED;
  }

  g_time_control = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME,
                                  sizeof(sns_time_control_s) );
  if( NULL == g_time_control )
  {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_APPS_TIME, "malloc error" );
    return SENSOR1_ENOMEM;
  }

  smr_err = sns_em_create_timer_obj( sns_time_calc_cb, NULL,
      SNS_EM_TIMER_TYPE_PERIODIC, &g_time_control->sns_time_calc_tmr);
  if( SNS_SUCCESS != smr_err ) {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_APPS_TIME,
                               "Can't create EM timer object" );
    SNS_OS_FREE( g_time_control );
    return SENSOR1_ENOMEM;
  }

  g_time_control->sns_time_mutex = sns_os_mutex_create( SNS_TIME_APPS_MUTEX, &os_err );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_APPS_TIME, "Mutex create failed" );
    sns_em_delete_timer_obj( g_time_control->sns_time_calc_tmr );
    SNS_OS_FREE( g_time_control );
    return SNS_ERR_FAILED;
  }

  g_time_control->apps_dsps_offset = 0;
  g_time_control->offset_calc_rate_curr = OFFSET_CALC_RATE_MIN;
  g_time_control->dsps_last_ts = 0;
  g_time_control->dsps_rollover_cnt = 0;
  g_time_control->time_clients = NULL;
  g_time_control->offset_history_count = 0;
  g_time_control->offset_history_idx = 0;

#ifdef SNS_LA
  /* open the kernel driver of the A-family first */
  g_time_control->fd = sns_pwr_get_pil_fd();
  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_TIME,
                           "A-family driver fd: %i",
                           g_time_control->fd );

  /* if failed then open kernel driver of the B-family */
  if( g_time_control->fd == -1 )
  {
    g_time_control->fd = open( SENSORS_DEV, O_RDONLY );
    SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_TIME,
                             "B-family driver fd: %i",
                             g_time_control->fd );
  }

  if( g_time_control->fd == -1 )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                               "Failed to open A and B family drivers, err=%d",
                               errno );
    return SNS_ERR_FAILED;
  }
#else
  g_time_control->fd = -1;
#endif /* SNS_LA */

  sns_time_calc_apps_dsps_offset( false );
  os_err = sns_os_task_create( sns_time_mr_thread, NULL, NULL,
                               SNS_MODULE_PRI_APPS_TIME );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_APPS_TIME, "task create failed" );
    sns_em_delete_timer_obj( g_time_control->sns_time_calc_tmr );
    SNS_OS_FREE( g_time_control );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}

/*!
  @brief Deinitializes the Sensors Time service.

  Destroys Time thread.
  Cleans up resources.

  @return
   - SNS_SUCCESS if initialization is successful.
   - All other values indicate an error has occurred.
*/
sns_err_code_e
sns_time_deinit( void )
{
  uint8_t err = 0;

  err = sns_time_mr_deinit();
  if( 0 != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_TIME, "Time deinit failed %i", err );
  }

  sns_os_task_del_req( SNS_MODULE_PRI_APPS_TIME2 );
  sns_os_task_del_req( SNS_MODULE_PRI_APPS_TIME );

  if( NULL != g_time_control )
  {
    sns_em_delete_timer_obj( g_time_control->sns_time_calc_tmr );

    // Delete any existing clients
    if( 0 != sns_time_client_count() )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                 "Deleting time data with %d clients",
                                 sns_time_client_count());

      while( NULL != g_time_control->time_clients )
      {
        sns_time_client *client = g_time_control->time_clients;
        g_time_control->time_clients = client->next;
        SNS_OS_FREE(client);
      }
    }

    // Delete mutex
    sns_os_mutex_del( g_time_control->sns_time_mutex, 0, &err );
    if( 0 != err )
    {
      SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_TIME, "can't delete mutex %i", err );
    }

    SNS_OS_FREE( g_time_control );
  }

  return SNS_SUCCESS;
}
