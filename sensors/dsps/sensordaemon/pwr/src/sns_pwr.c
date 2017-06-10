/*============================================================================
  @file sns_pwr.c

  @brief
    This implements the sensor1 APIs, and contains the Power Manager

  <br><br>

  DEPENDENCIES: This uses OS services defined in sns_osa.h.
    It uses events/timers defined in sns_em.h.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/

#include "sns_common.h"
#include "sns_debug_api.h"
#include "sns_debug_str.h"
#include "sns_em.h"
#include "sns_fsa.h"
#include "sns_init.h"
#include "sns_log_api.h"
#include "sns_log_types.h"
#include "sns_pwr.h"

#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#ifdef SNS_LA
#include <linux/msm_dsps.h>
#endif /* SNS_LA */


/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define INVALID_FILE_DESCRIPTOR -1

#define SNS_MODULE SNS_MODULE_APPS_PWR

#define SNS_PWR_DSPS_PATH  _PATH_DEV"msm_dsps"
#define SNS_PWR_DSPS_MAJOR 251
#define SNS_PWR_DSPS_MINOR 0

/* Time between the last power off vote, and turning off the DSPS */
#define SNS_PWR_OFF_DELAY_USEC 1000000 /* 1 second */
/* Initial multiplier of the delay for the first time DSPS is powered off */
#define SNS_PWR_OFF_DELAY_INIT_MULTIPLIER 10

/* Macro to program DSPS power states */
#ifdef SNS_LA
#define SNS_PWR_CMD(fd,cmd)                                 \
  ioctl( (fd),                                              \
         DSPS_IOCTL_ ## cmd ,                               \
         0 );                                               \
  SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_PWR,           \
                            "Sent "#cmd" ioctl" );
#else
#define SNS_PWR_CMD(fd,cmd) 0
#endif /* SNS_LA */

#define SNS_CPU_LATENCY_FILE    "/dev/cpu_dma_latency"
#define SNS_WAKELOCK_FILE       "/sys/power/wake_lock"
#define SNS_WAKEUNLOCK_FILE     "/sys/power/wake_unlock"
#define SNS_WAKELOCK_NAME       "sns_periodic_wakelock"
#define SNS_ASYNC_WAKELOCK_NAME "sns_async_ev_wakelock"

/* Default behavior is to not hold wakelocks for periodic reports */
#define SNS_PERIODIC_WL_DIS_PROP         "ro.qc.sensors.wl_dis"
#define SNS_PERIODIC_WL_DIS_PROP_DEFAULT "true"


/*============================================================================
  Forward Declarations
  ============================================================================*/
extern void smr_set_qmi_service_obj( void );

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/** File descriptor for SNS_PWR_DSPS_PATH */
int g_pil_fd = INVALID_FILE_DESCRIPTOR;

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/** File descriptor for SNS_WAKELOCK_FILE */
static int g_wl_fd = INVALID_FILE_DESCRIPTOR;
/** File descriptor for SNS_WAKEUNLOCK_FILE */
static int g_wul_fd = INVALID_FILE_DESCRIPTOR;
/** File descriptor for SNS_CPU_LATENCY_FILE */
static int g_latency_fd = INVALID_FILE_DESCRIPTOR;

/** Current voting mask */
static volatile uint32_t g_vote_mask;

/** Timer. Used to set a delay before powering off the DSPS. */
static sns_em_timer_obj_t g_timer = (sns_em_timer_obj_t)NULL;

/** Mutex protecting the vote mask */
static OS_EVENT *sns_pwr_mutex = (OS_EVENT*)NULL;

/** Processor ID Daemon is running on */
static sns_msm_type g_msm_type = {SNS_MSM_UNKNOWN, SNS_PLATFORM_UNKNOWN};

#ifdef SNS_LA
/* Latency adjustment variable : set to 2ms */
static int32_t sns_pwr_latency_adjustment = 2000;
#endif /* SNS_LA */

/* Hold wakelock for periodic reports? */
static boolean sns_pwr_periodic_wl_enabled = false;

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/


/*===========================================================================

  FUNCTION:   sns_detect_msm

  ===========================================================================*/
/*!
  @brief Detects the msm type

*/
/*=========================================================================*/
static void sns_detect_msm()
{
  FILE *fp;
  char line[100];
  char board_platform[PROPERTY_VALUE_MAX];

  g_msm_type.msm_id = SNS_MSM_UNKNOWN;
  g_msm_type.platform = SNS_PLATFORM_UNKNOWN;

  property_get("ro.board.platform", board_platform, "");
  if (!strcmp("msm8660", board_platform))
  {
    g_msm_type.msm_id = SNS_MSM_8660;
  }
  else if (!strcmp("msm8960", board_platform))
  {
    g_msm_type.msm_id = SNS_MSM_8960;
  }
  else if (!strcmp("msm8226", board_platform))
  {
    g_msm_type.msm_id = SNS_MSM_8226;
  }
  else if (!strcmp("msm8974", board_platform))
  {
    g_msm_type.msm_id = SNS_MSM_8974;
  }
  else if (!strcmp("apq8084", board_platform))
  {
    g_msm_type.msm_id = SNS_APQ_8084;
  }
  else if (!strcmp("msm8962", board_platform))
  {
    g_msm_type.msm_id = SNS_MSM_8962;
  }
  else if (!strcmp("msm8994", board_platform))
  {
    g_msm_type.msm_id = SNS_PLUTONIUM;
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
      "device platform not handled '%s'", board_platform);
  }

  fp = sns_fsa_open("/sys/devices/soc0/hw_platform", "r" );
  if( fp == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
      "hw_platform fopen failed %i", errno );
  }
  else if( fgets(line, sizeof(line), fp) == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
      "hw_platform fgets failed %i: %s", ferror( fp ) );
  }
  else
  {
    if( strstr( line, "FFA" ) != NULL ||
        strstr( line, "MTP" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_MTP;
    }
    else if( strstr( line, "Surf" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_CDP;
    }
    else if( strstr( line, "Fluid" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_FLUID;
    }
    else if( strstr( line, "Liquid" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_LIQUID;
    }
    else if( strstr( line, "QRD" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_QRD;
    }
    else if( strstr( line, "SKUF" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_SKUF;
    }
  }

  if( SNS_SUCCESS != sns_fsa_close( fp ) )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_PWR, "sns_fsa_close failed" );
  }

  /* check the subtype */
  fp = sns_fsa_open("/sys/devices/soc0/platform_subtype", "r" );
  if( fp == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
      "platform_subtype fopen failed %d", errno );
  }
  else if( fgets(line, sizeof(line), fp) == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
      "platform_subtype fgets failed %d", ferror( fp ) );
  }
  else if( SNS_PLATFORM_QRD == g_msm_type.platform )
  {
    if( strstr( line, "SKUF" ) != NULL )
    {
      g_msm_type.platform = SNS_PLATFORM_SKUF;
    }
  }

  if( SNS_SUCCESS != sns_fsa_close( fp ) )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_PWR, "sns_fsa_close failed" );
  }
}


/*===========================================================================

  FUNCTION:   sns_pwr_wake_lock

  ===========================================================================*/
/*!
  @brief Grabs the wakelock to prevent system suspend

  @param[i] wl_name: Name of wakelock to grab
*/
/*=========================================================================*/
static void sns_pwr_wake_lock( const char * wl_name )
{
  int err;

  if( g_wl_fd >= 0 ) {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_PWR,
                              "acquiring wakelock" );

    err = write( g_wl_fd, wl_name, strlen(wl_name) );
    if( err != (int)strlen(wl_name) ) {
      SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_PWR,
                                 "sns_pwr_on: wakelock write error: %d, %d",
                                 err, errno );
    }
  }
}


/*===========================================================================

  FUNCTION:   sns_pwr_wake_unlock

  ===========================================================================*/
/*!
  @brief Release the wakelock to allow system suspend

*/
/*=========================================================================*/
static void sns_pwr_wake_unlock( const char * wl_name )
{
  int err;

  if( g_wul_fd >= 0 ) {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_PWR,
                              "releasing wakelock" );

    err = write( g_wul_fd, wl_name, strlen(wl_name) );
    if( err != (int)strlen(wl_name) ) {
      SNS_PRINTF_STRING_HIGH_2( SNS_DBG_MOD_APPS_PWR,
                                "sns_pwr_wake_unlock: wakeunlock write error: %d, %d",
                                err, errno );
    }
  }
}

/*===========================================================================

  FUNCTION:   sns_pwr_timer_cb

  ===========================================================================*/
/*!
  @brief Turns off the DSPS based on voting mask.

*/
/*=========================================================================*/
static void
sns_pwr_timer_cb( void* cb_arg )
{
  int err;
  uint8_t os_err;

  UNREFERENCED_PARAMETER(cb_arg);

  sns_os_mutex_pend( sns_pwr_mutex, 0, &os_err );
  if( 0 == os_err &&
      0 == g_vote_mask ) {
    err = SNS_PWR_CMD( g_pil_fd, OFF );
    if( err < 0 ) {
#if defined(SNS_LA) || defined(SNS_LA_SIM)
      perror("sns_pwr_off: ioctl");
#endif
    } else {
      sns_err_code_e sns_err;
      sns_log_dsps_pwr_s *dsps_pwr_p;

      sns_err = sns_logpkt_malloc( SNS_LOG_DSPS_PWR,
                                   sizeof( sns_log_dsps_pwr_s ),
                                   (void**)(&dsps_pwr_p) );
      if( (SNS_SUCCESS == sns_err) && (NULL != dsps_pwr_p) ) {
        dsps_pwr_p->version  = SNS_LOG_DSPS_PWR_VERSION;
        sns_err = sns_em_get_timestamp64( &(dsps_pwr_p->timestamp) );
        if( SNS_SUCCESS != sns_err ) {
          // Fall back to 32-bit timestamp
          dsps_pwr_p->timestamp = sns_em_get_timestamp();
        }
        dsps_pwr_p->powerstate = SNS_LOG_DSPS_PWR_ST_OFF;
        sns_logpkt_commit( SNS_LOG_DSPS_PWR, dsps_pwr_p );
      }
    }
  }
  sns_os_mutex_post( sns_pwr_mutex );
}

/*============================================================================
  Externalized Function Definitions
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_pwr_get_msm_type

  ===========================================================================*/
sns_msm_type
sns_pwr_get_msm_type()
{
  return g_msm_type;
}

/*===========================================================================

  FUNCTION:   sns_pwr_get_pil_fd

  ===========================================================================*/
int
sns_pwr_get_pil_fd( )
{
  return g_pil_fd;
}

/*===========================================================================

  FUNCTION:   sns_pwr_set_wake_lock

  ===========================================================================*/
void
sns_pwr_set_wake_lock( boolean lock )
{
  static bool lock_held = false;

  if( lock == true ) {
    if( lock_held == false ) {
      sns_pwr_wake_lock( SNS_ASYNC_WAKELOCK_NAME );
      lock_held = true;
    }
  } else {
    if( lock_held == true ) {
      lock_held = false;
      sns_pwr_wake_unlock( SNS_ASYNC_WAKELOCK_NAME );
    }
  }
}

void
sns_pwr_set_cpu_latency( int32_t hz )
{
#if defined(SNS_LA)
  int32_t usec;

  SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_PWR,
                            "sns_pwr_set_cpu_latency: hz %d", hz );

  if( g_latency_fd < 0 ) {
    /* this should never happen but just in case .. */
    g_latency_fd = open( SNS_CPU_LATENCY_FILE, O_WRONLY );
  }


  if( g_latency_fd >= 0 ) {
    int err;

    if( hz > 0 ) {
      usec = (1000000LL/hz);

      /* Adjust the latency to account for sensor data processing time */
      /* Subtracting only 2ms now, based on rough estimates. Needs tuning,
       * possibly will require different values for different frequencies (based on
       * latency analysis)
       */
      usec = usec - sns_pwr_latency_adjustment;


      if( true == sns_pwr_periodic_wl_enabled ) {
        /* Acquire wake lock */
        sns_pwr_wake_lock( SNS_WAKELOCK_NAME );
      }

    } else {
      usec = -1; /* just write -1 when there is no latency requirement */

      /* Also release wake lock as there is no wake lock requirement */
      sns_pwr_wake_unlock( SNS_WAKELOCK_NAME );
    }

    err = write( g_latency_fd, &usec, sizeof(int32_t) );
    if( err != sizeof(int32_t) ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                                 "sns_pwr_set_cpu_latency: write error : %d",
                                 err );
    }

  }
  else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                               "sns_pwr_set_cpu_latency: open error: %d",
                               errno );
  }
#else /* defined(SNS_LA) */
  UNREFERENCED_PARAMETER(hz);
#endif /* else defined(SNS_LA) */
}


/*===========================================================================

  FUNCTION:   sns_pwr_boot

  ===========================================================================*/
sns_err_code_e
sns_pwr_boot( void )
{
  sns_err_code_e rv = SNS_SUCCESS;
#if defined(SNS_LA)
  if( ( sns_pwr_get_msm_type().msm_id == SNS_MSM_8960 ||
        sns_pwr_get_msm_type().msm_id == SNS_MSM_8660 )
      && 0 > ( g_pil_fd = open( SNS_PWR_DSPS_PATH, O_RDONLY, 0 ) ) )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                               "sns_pwr_boot: DSPS device open failed err %d",
                               errno );
    rv = SNS_ERR_FAILED;
  }
  else
#endif /* SNS_LA */
  {
    sns_err_code_e sns_err;
    sns_log_dsps_pwr_s *dsps_pwr_p;
    sns_err = sns_logpkt_malloc( SNS_LOG_DSPS_PWR,
                                 sizeof( sns_log_dsps_pwr_s ),
                                 (void**)(&dsps_pwr_p) );
    if( (SNS_SUCCESS == sns_err) && (NULL != dsps_pwr_p) ) {
      dsps_pwr_p->version  = SNS_LOG_DSPS_PWR_VERSION;
      sns_err = sns_em_get_timestamp64( &(dsps_pwr_p->timestamp) );
      if( SNS_SUCCESS != sns_err ) {
        // Fall back to 32-bit timestamp
        dsps_pwr_p->timestamp = sns_em_get_timestamp();
      }
      dsps_pwr_p->powerstate = SNS_LOG_DSPS_PWR_ST_ON;
      sns_logpkt_commit( SNS_LOG_DSPS_PWR, dsps_pwr_p );
    }

#ifdef SNS_LA
    /* Open wakelock files: */
    g_wl_fd = open( SNS_WAKELOCK_FILE, O_WRONLY|O_APPEND );
    if( g_wl_fd < 0 )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                                 "sns_pwr_boot: wakelock open error: %d",
                                 errno );
      g_wul_fd = INVALID_FILE_DESCRIPTOR;
    } else {
      g_wul_fd = open( SNS_WAKEUNLOCK_FILE, O_WRONLY|O_APPEND );
      if( g_wul_fd < 0 ) {
        SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                                   "sns_pwr_boot: wakeunlock open error: %d",
                                   errno );
        close( g_wl_fd );
        g_wl_fd = INVALID_FILE_DESCRIPTOR;
      }
    }

     /* It is not necessary to grab wake lock here as DSPS boot process can still continue
     * even if Apps processor enters suspend mode.
     */

    /* open cpu latency file */
    g_latency_fd = open( SNS_CPU_LATENCY_FILE, O_WRONLY );
    if( g_latency_fd < 0 )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                                 "sns_pwr_boot: cpu latency open error: %d",
                                 errno );
    }
#endif /* SNS_LA */

  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_pwr_on

  ===========================================================================*/
sns_err_code_e
sns_pwr_on( uint32_t vote_mask )
{
  sns_err_code_e rv = SNS_SUCCESS;
  int err;
  uint8_t os_err;
  uint32_t orig_vote_mask = g_vote_mask;

  SNS_PRINTF_STRING_HIGH_3( SNS_DBG_MOD_APPS_PWR,
                            "sns_pwr_on: vote 0x%x, g_vote_mask 0x%x, fd: %d",
                            vote_mask, g_vote_mask, g_pil_fd );

  if( g_pil_fd < 0 ) {
    return SNS_ERR_FAILED;
  }

  sns_os_mutex_pend( sns_pwr_mutex, 0, &os_err );
  if( 0 == os_err ) {
    g_vote_mask |= vote_mask;

    if( (0 == orig_vote_mask) && (0 != g_vote_mask) ) {
      err = SNS_PWR_CMD( g_pil_fd, ON );
      if( err < 0 ) {
        perror("sns_pwr_on: ioctl");
        rv = SNS_ERR_FAILED;
      } else {
        sns_err_code_e sns_err;
        sns_log_dsps_pwr_s *dsps_pwr_p;

        sns_err = sns_logpkt_malloc( SNS_LOG_DSPS_PWR,
                                     sizeof( sns_log_dsps_pwr_s ),
                                     (void**)(&dsps_pwr_p) );
        if( (SNS_SUCCESS == sns_err) && (NULL != dsps_pwr_p) ) {
          dsps_pwr_p->version  = SNS_LOG_DSPS_PWR_VERSION;
          sns_err = sns_em_get_timestamp64( &(dsps_pwr_p->timestamp) );
          if( SNS_SUCCESS != sns_err ) {
            // Fall back to 32-bit timestamp
            dsps_pwr_p->timestamp = sns_em_get_timestamp();
          }
          dsps_pwr_p->powerstate = SNS_LOG_DSPS_PWR_ST_ON;
          sns_logpkt_commit( SNS_LOG_DSPS_PWR, dsps_pwr_p );
        }
      }
    }
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                               "Error %d getting mutex", (int32_t)os_err );
  }
  sns_os_mutex_post( sns_pwr_mutex );
  return rv;
}


/*===========================================================================

  FUNCTION:   sns_pwr_off

  ===========================================================================*/
sns_err_code_e
sns_pwr_off( uint32_t vote_mask )
{
  sns_err_code_e rv = SNS_SUCCESS;
  sns_err_code_e sns_err;
  uint8_t os_err;
  static int init_multiplier_delay = SNS_PWR_OFF_DELAY_INIT_MULTIPLIER;

  SNS_PRINTF_STRING_HIGH_3( SNS_DBG_MOD_APPS_PWR,
                            "sns_pwr_off: vote mask 0x%x, "
                            "g_vote_mask 0x%x, fd: %d",
                            vote_mask, g_vote_mask, g_pil_fd );

  if( g_pil_fd < 0 ) {
    return SNS_ERR_FAILED;
  }

  sns_os_mutex_pend( sns_pwr_mutex, 0, &os_err );
  if( 0 == os_err ) {
    g_vote_mask &= (~vote_mask);

    if( 0 == g_vote_mask ) {
      uint32_t delay;
      SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_PWR,
                               "Registering timer to power down DSPS" );
      delay = sns_em_convert_usec_to_localtick( SNS_PWR_OFF_DELAY_USEC
                                                * init_multiplier_delay );
      sns_err = sns_em_register_timer( g_timer, delay );

      if( SNS_SUCCESS != sns_err ) {
        SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_PWR,
                                   "Can't register timer to power down DSPS!" );
      }
      init_multiplier_delay = 1;
    }
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_PWR,
                               "Error %d getting mutex", (int32_t)os_err );
  }
  sns_os_mutex_post( sns_pwr_mutex );
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_pwr_crash_shutdown

  ===========================================================================*/
/*!
  @brief Called in case of a crash shutdown.

  Cleans up all power-related activites before the sensor daemon shuts down.

  @dependencies
  None.

*/
/*=========================================================================*/
void
sns_pwr_crash_shutown( void )
{
  sns_pwr_wake_unlock( SNS_ASYNC_WAKELOCK_NAME );
  sns_pwr_wake_unlock( SNS_WAKELOCK_NAME );

  if( g_latency_fd != -1 ) {
    int32_t usec = -1;
    write( g_latency_fd, &usec, sizeof(int32_t) );
  }
}

/*===========================================================================

  FUNCTION:   sns_pwr_init

  ===========================================================================*/
/*!
  @brief Boot-time initilization of the pwr module.

  @dependencies
  None.

  @return
  SNS_SUCCESS: Booted the DSPS.
  Any other error code: DSPS not booted.
*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_init( void )
{
  sns_err_code_e rv;
  uint8_t        err;
  char           wl_dis_prop_value[PROPERTY_VALUE_MAX];

  g_vote_mask = 0;
  g_latency_fd = INVALID_FILE_DESCRIPTOR;

  sns_em_create_timer_obj( sns_pwr_timer_cb,
                           NULL,
                           SNS_EM_TIMER_TYPE_ONESHOT,
                           &g_timer );

  sns_pwr_mutex = sns_os_mutex_create( SNS_PWR_APPS_MUTEX,
                                       &err );

  sns_detect_msm();
  rv = sns_pwr_boot();

  /* Ensure wakelocks are released at the beginning of execution */
  sns_pwr_wake_unlock( SNS_ASYNC_WAKELOCK_NAME );
  sns_pwr_wake_unlock( SNS_WAKELOCK_NAME );

  // Must call this prior to any call to smr_get_qmi_service_obj()
  smr_set_qmi_service_obj();

  property_get( SNS_PERIODIC_WL_DIS_PROP, wl_dis_prop_value,
                SNS_PERIODIC_WL_DIS_PROP_DEFAULT );
  if( strncmp( wl_dis_prop_value, "false", sizeof( wl_dis_prop_value ) ) == 0 ||
      strncmp( wl_dis_prop_value, "0", sizeof( wl_dis_prop_value ) ) == 0 ) {
    sns_pwr_periodic_wl_enabled = true;
  }

  sns_init_done();
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_pwr_deinit

  ===========================================================================*/
/*!
  @brief Free resources used by the pwr module.

  @dependencies
  None.

  @return
  SNS_SUCCESS: PWR resources freed.
  Any other error code: PWR resources not freed.
*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_deinit( void )
{
  uint8_t err = 0;

  if(NULL != g_timer) {
    sns_em_cancel_timer(g_timer);
    sns_em_delete_timer_obj(g_timer);
    g_timer = (sns_em_timer_obj_t)NULL;
  }

  if(NULL != sns_pwr_mutex) {
    sns_os_mutex_del(sns_pwr_mutex, 0, &err);
    sns_pwr_mutex = (OS_EVENT*)NULL;
  }

#ifdef SNS_LA
  int fd;
  if( g_pil_fd != INVALID_FILE_DESCRIPTOR ) {
    fd = g_pil_fd;
    g_pil_fd = INVALID_FILE_DESCRIPTOR;
    close( fd );
  }

  if( g_wl_fd != INVALID_FILE_DESCRIPTOR ) {
    fd = g_wl_fd;
    g_wl_fd = INVALID_FILE_DESCRIPTOR;
    close( fd );
  }

  if( g_wul_fd != INVALID_FILE_DESCRIPTOR ) {
    fd = g_wul_fd;
    g_wul_fd = INVALID_FILE_DESCRIPTOR;
    close( fd );
  }

  if( g_latency_fd != INVALID_FILE_DESCRIPTOR ) {
    fd = g_latency_fd;
    g_latency_fd = INVALID_FILE_DESCRIPTOR;
    close( fd );
  }
#endif /* SNS_LA */

  return SNS_SUCCESS;
}
