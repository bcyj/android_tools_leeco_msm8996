/*============================================================================
  @file ar_hal.h

  @brief
  AR HAL header.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef AR_HAL_H
#define AR_HAL_H

/*===========================================================================
                           INCLUDE FILES
===========================================================================*/
/* ADB logcat */
#define LOG_TAG "qti_ar_hal"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NVDEBUG 0

#include <cutils/properties.h>
#include <cutils/log.h>
#include <activity_recognition.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include "log.h"
#include "sensor1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
                   PREPROCESSOR DEFINTIONS
===========================================================================*/

/* max number of activities */
#define MAX_NUM_ACTIVITIES 6

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof((x)[0]))

#ifndef UNREFERENCED_PARAMETER
# define UNREFERENCED_PARAMETER(x) (void)x;
#endif /* UNREFERENCED_PARAMETER */

/* timeout in ms used in mutex wait */
#define TIME_OUT_MS 1000

/* max number of SAM service ID's */
#define MAX_SAM_SERVICES 60

/* Unit conversion between sensor1 and android */
#define UNIT_Q16                     (65536.0f)
#define UNIT_CONVERT_Q16             (1.0/65536.0)

#define RAD_Q16_TO_DEG_FLT  (UNIT_CONVERT_Q16 * RAD2DEG)

#define INVALID_INSTANCE_ID   0xFF

/* Timing related macros */
#define NSEC_PER_SEC       1000000000LL
#define USEC_PER_SEC       1000000LL
#define USEC_TO_MSEC(usec) ((usec)/1000L)
#define USEC_TO_NSEC(usec) ((usec)*1000L)
#define NSEC_TO_MSEC(nsec) ((nsec)/1000000LL)
#define NSEC_TO_SEC(nsec)  ((nsec)/1000000000LL)
#define HZ_TO_USEC(hz)     (1000000LL/(hz))
#define HZ_TO_NSEC(hz)     (1000000000LL/(hz))
#define MSEC_TO_HZ(ms)     (1000.0/(ms))
#define NSEC_TO_HZ(ns)     (1000000000.0/((float)ns))
#define DSPS_HZ            32768LL

/* If two consecutive samples have non-increasing timestamps:
 * If the second is within X of the first, edit the second. */
#define TS_CORRECT_THRESH 200000

/* If a new sample contains a timestamp that is within X of 0, while
 * the previous sample contained a timestamp within X of UINT32_MAX, the SSC
 * clock may have rolled-over */
#define TS_ROLLOVER_THRESH 983040

/* If a DSPS clock-rollover is detected, ensure that the last rollover event
 * happened more than X ns ago. This prevents false rollover detections from
 * jitter in the incoming SSC timestamps. */
#define BOOT_TS_ROLLOVER_THRESH 5000000000 // in ns (5 seconds)

#ifndef LOGV_IF
#define LOGV_IF ALOGV_IF
#endif /* LOG_IF */
#ifndef LOGD_IF
#define LOGD_IF ALOGD_IF
#endif /* LOG_IF */
#ifndef LOGI_IF
#define LOGI_IF ALOGI_IF
#endif /* LOG_IF */
#ifndef LOGW_IF
#define LOGW_IF ALOGW_IF
#endif /* LOG_IF */
#ifndef LOGE_IF
#define LOGE_IF ALOGE_IF
#endif /* LOG_IF */

#define HAL_LOG_VERBOSE(...) LOGV_IF((g_hal_log_level <= HAL_LOG_LEVEL_VERBOSE), __VA_ARGS__ )
#define HAL_LOG_DEBUG(...) LOGD_IF((g_hal_log_level <= HAL_LOG_LEVEL_DEBUG), __VA_ARGS__ )
#define HAL_LOG_INFO(...) LOGI_IF((g_hal_log_level <= HAL_LOG_LEVEL_INFO), __VA_ARGS__ )
#define HAL_LOG_WARN(...) LOGW_IF((g_hal_log_level <= HAL_LOG_LEVEL_WARN), __VA_ARGS__ )
#define HAL_LOG_ERROR(...) LOGE_IF((g_hal_log_level <= HAL_LOG_LEVEL_ERROR), __VA_ARGS__ )

/* System properties to enable features */
#define HAL_PROP_DEBUG          "persist.debug.ar.hal"

/* Maximum data fields per SAM algorithm report */
#define SAM_MAX_DATA_LENGTH 16

/*===========================================================================
                   DATA TYPES
===========================================================================*/
/* Definitions for logging */
typedef enum {
    HAL_LOG_LEVEL_ALL,
    HAL_LOG_LEVEL_VERBOSE,
    HAL_LOG_LEVEL_DEBUG,
    HAL_LOG_LEVEL_INFO,
    HAL_LOG_LEVEL_WARN,
    HAL_LOG_LEVEL_ERROR,
    HAL_LOG_LEVEL_DISABLED
} hal_log_level_e;

typedef struct hal_sam_service_info {
    uint32_t      freq;
    uint8_t       instance_id;   /* Used to store instance IDs for SAM req/resp */
    uint8_t       ref_count;
} hal_sam_service_info_t;

typedef struct hal_sam_sample {
    uint32_t data[SAM_MAX_DATA_LENGTH];
    uint32_t timestamp;
} hal_sam_sample_t;

typedef struct hal_ar_activity {
    bool enter;
    bool exit;
    int64_t batch_rate;
} hal_ar_activity_t;

typedef struct hal_ar_ctl {
    activity_recognition_device_t    device;
    sensor1_handle_s*                hndl;

    const activity_recognition_callback_procs_t* hal_ar_cb;

    /* Thread control mutex and cond variables */
    pthread_mutex_t                  cb_mutex;           /* mutex lock for sensor1 callback */
    pthread_cond_t                   cb_arrived_cond;    /* cond variable to signal callback has arrived */
    bool                             is_resp_arrived;    /* flag to indicate callback has arrived */
    bool                             error;

    pthread_mutex_t                  data_mutex;         /* mutex lock for data */
    bool                             is_ind_arrived;     /* flag to indicate callback has arrived */
    pthread_cond_t                   data_arrived_cond;  /* cond variable to signal data has arrived */

    pthread_mutex_t                  acquire_resources_mutex; /* Used to serialize hal_acquire_resources */
    timer_t                          acquire_resources_timer; /* Acquire resources is called when the timer expires */

    /* Protected by data_mutex */
    hal_ar_activity_t                hal_track_activity[MAX_NUM_ACTIVITIES];  /* Track events */
    activity_event_t                 last_event[MAX_NUM_ACTIVITIES];  /* Last event sent */
    bool                             flush_requested; /* true if a flush has been requested */

    /* Protected by cb_mutex */
    hal_sam_service_info_t           sam_service[MAX_SAM_SERVICES];

    bool                             cmc_enabled;  /* To know CMC Algo status */
    bool                             tilt_enabled; /* To know Tilt Algo status */
    bool                             time_service_enabled; /* To know time service status */
    int                              cmc_version;  /* CMC Algo Version */
} hal_ar_ctl_t;

/*===========================================================================
                    GLOBAL VARIABLES
===========================================================================*/
extern hal_log_level_e g_hal_log_level;

/*===========================================================================
                    FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  hal_time_init
===========================================================================*/
/*!
 * @brief
 * Must be called during HAL initialization.
 *
 * @return 0 Upon success, error otherwise
*/
int hal_time_init();

/*===========================================================================
  FUNCTION:  hal_time_start
===========================================================================*/
/*!
 * @brief
 * Initializes local state, and registers with the Time Sync service.
 *
 * @return 0 Upon success, error otherwise
*/
int hal_time_start();

/*===========================================================================
  FUNCTION:  hal_time_stop
===========================================================================*/
/*!
 * @brief
 * Close connections and reset local state.
 *
 * @return 0 Upon success, error otherwise
*/
int hal_time_stop();

/*===========================================================================
  FUNCTION:  hal_timestamp_calc
===========================================================================*/
/*!
  @brief
  Converts the DSPS clock ticks from a sample to a LA timestamp (ns
  since epoch).  Adjusts return value based on dsps timestamp rollover
  and makes minor adjustments to ensure samples are sent with
  ascending timestamps.

  @param[i] dsps_timestamp Timestamp as received from the DSPS
  @param[i] activity in question.
              Used to ensure timestamp ordering.

  @return The determined APPS-processor timestamp.
*/
int64_t hal_timestamp_calc( uint64_t dsps_timestamp, uint32_t activity );

/*===========================================================================
  FUNCTION:  hal_wait_for_response
===========================================================================*/
/*!
  @brief
  Blocks waiting for response, either sensor1 callback to arrvive or timeout

  Helper function

  @param [i] timeout timeout in ms
  @param [i] cb_mutex_ptr pointer to locked mutex
  @param [i] cond_ptr pointer to condition variable
  @param [i/o] cond_var boolean predicate.

  @return true if callback arrived, false if timeout

  @dependencies Caller needs to lock cb_mutex_ptr before
                calling this function. Another thread must set cond_var
                to true before signalling the condition variable.
*/
bool hal_wait_for_response( int timeout, pthread_mutex_t* cb_mutex_ptr,
                            pthread_cond_t*  cond_ptr, bool *cond_var );

/*===========================================================================
  FUNCTION:  hal_signal_response
===========================================================================*/
/*!
 * @brief
 * Signal any waiting request upon response
 *
 * @param[i] Either true or false
 * @param[i] Conditional pointer
*/
static void hal_signal_response( bool error, pthread_cond_t* cond_ptr );

/*===========================================================================
  FUNCTION:  hal_signal_ind
===========================================================================*/
static void hal_signal_ind( pthread_cond_t* cond_ptr );

/*===========================================================================
  FUNCTION:  hal_flush_send_cmplt
===========================================================================*/
/*!
 * @brief
 * Sends ACTIVITY_EVENT_FLUSH_COMPLETE event.  Must hold data_mutex.
*/
static void hal_flush_send_cmplt();

/*===========================================================================
  FUNCTION:  hal_sensor1_data_cb
===========================================================================*/
/*!
 * @brief
 * Data call back function. Processes response messages of sensor1_open.
 *
 * @param[i] call back data
 * @param[i] Message header
 * @param[i] Message type
 * @param[i] Message pointer
*/
static void
hal_sensor1_data_cb (intptr_t cb_data,
                     sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr);

/*===========================================================================
  FUNCTION:  hal_reinit
===========================================================================*/
/*!
 * @brief
 * Re-intializes the HAL. This is called when the sensor daemon
 * has crashed and restarted.  Must hold cb_mutex
 *
 * Returns error code if error, otherwise SENSOR1_SUCCESS if success
*/
static int hal_reinit();

/*===========================================================================
  FUNCTION:  hal_handle_broken_pipe
===========================================================================*/
/*!
 * @brief
 * This function is called when a broken pipe received
*/
static void hal_handle_broken_pipe();

#ifdef __cplusplus
}
#endif

#endif /* AR_HAL_H */
