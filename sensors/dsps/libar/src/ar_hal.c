/*============================================================================
  @file ar_hal.c

  @brief
  Activity Recognition HAL.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/
#include <cutils/native_handle.h>
#include <stdlib.h>
#include <hardware.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#include "ar_hal.h"
#include "sns_sam_cmc_v02.h"
#include "sns_sam_tilt_detector_v01.h"
#include "log_codes.h"
#include "sns_log_types.h"
#include "sns_reg_common.h"

/*============================================================================
                   INTERNAL DEFINITIONS AND TYPES
============================================================================*/
#define CMC_SVC_NUM SNS_SAM_CMC_SVC_ID_V01
#define TILT_DETECTOR_SVC_NUM SNS_SAM_TILT_DETECTOR_SVC_ID_V01

/* Activity Handles */
#define HANDLE_IN_VEHICLE 0
#define HANDLE_ON_BICYCLE 1
#define HANDLE_WALKING 2
#define HANDLE_RUNNING 3
#define HANDLE_STILL 4
#define HANDLE_TILTING 5

/*============================================================================
                    FUNCTION DECLARATIONS
============================================================================*/
/* functions defined by AR HAL */
static int hal_ar_close( struct hw_device_t *dev );
static int hal_ar_enable_event(const struct activity_recognition_device* dev,
                               uint32_t activity_handle, uint32_t event_type,
                               int64_t max_batch_report_latency_ns);
static int hal_ar_disable_event(const struct activity_recognition_device* dev,
                                uint32_t activity, uint32_t event_type);
static int hal_ar_flush(const struct activity_recognition_device* dev);
static void hal_ar_register_cb(const struct activity_recognition_device* dev,
                               const activity_recognition_callback_procs_t* callback);

/*============================================================================
  Static Variable Definitions
  ===========================================================================*/

/*===========================================================================
                         GLOBAL VARIABLES
===========================================================================*/
hal_log_level_e g_hal_log_level = HAL_LOG_LEVEL_WARN;
static hal_ar_ctl_t* g_ar_control = NULL;

/* List of supported activities */
const char *g_activity_list[MAX_NUM_ACTIVITIES] = {
    "android.activity_recognition.in_vehicle",
    "android.activity_recognition.on_bicycle",
    "android.activity_recognition.walking",
    "android.activity_recognition.running",
    "android.activity_recognition.still",
    "android.activity_recognition.tilting"
};

/*============================================================================
  Static Function Definitions and Documentation
  ==========================================================================*/
static void enableLogging()
{
    int debug_prop_len;
    char debug_prop[PROPERTY_VALUE_MAX];

    /* Get the debug level from the property */
    debug_prop_len = property_get(HAL_PROP_DEBUG, debug_prop, "");
    if (debug_prop_len == 1) {
        switch (debug_prop[0]) {
            case '0':
                g_hal_log_level = HAL_LOG_LEVEL_DISABLED;
                break;
            case '1':
                g_hal_log_level = HAL_LOG_LEVEL_ALL;
                break;
            case 'v':
            case 'V':
                g_hal_log_level = HAL_LOG_LEVEL_VERBOSE;
                break;
            case 'd':
            case 'D':
                g_hal_log_level = HAL_LOG_LEVEL_DEBUG;
                break;
            case 'i':
            case 'I':
                g_hal_log_level = HAL_LOG_LEVEL_INFO;
                break;
            case 'w':
            case 'W':
                g_hal_log_level = HAL_LOG_LEVEL_WARN;
                break;
            case 'e':
            case 'E':
                g_hal_log_level = HAL_LOG_LEVEL_ERROR;
                break;
            default:
                break;
        }
        LOGI("%s: Setting log level to %d", __FUNCTION__, g_hal_log_level);
    }
    else if (debug_prop_len > 1) {
        LOGE("%s: invalid value for %s: %s. Enabling all logs", __FUNCTION__,
            HAL_PROP_DEBUG, debug_prop);
        g_hal_log_level = HAL_LOG_LEVEL_ALL;
    }
}

/*===========================================================================
  FUNCTION:  hal_sam_send_cancel
===========================================================================*/
/*!
*/
static void hal_sam_send_cancel( sensor1_handle_s *sensor1_hndl, uint32_t svc_num )
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_common_cancel_req_msg_v01 *cancel_msg;

    HAL_LOG_DEBUG( "%s: Sending cancel to %d", __FUNCTION__, svc_num );

    error = sensor1_alloc_msg_buf( sensor1_hndl,
                                   sizeof(sns_common_cancel_req_msg_v01),
                                   (void**)&cancel_msg );

    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
    }
    else {
        req_hdr.service_number = svc_num;
        req_hdr.msg_id = 0; /* Message ID for Cancel Request is 0 for all services */
        req_hdr.msg_size = sizeof( sns_common_cancel_req_msg_v01 );
        req_hdr.txn_id = 0;

        if( SENSOR1_SUCCESS !=
            (error = sensor1_write( sensor1_hndl, &req_hdr, cancel_msg )) ) {
            sensor1_free_msg_buf( sensor1_hndl, cancel_msg );
            HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
        }
    }
}

/*===========================================================================
  FUNCTION:  hal_wait_for_response
===========================================================================*/
/*!
*/
bool hal_wait_for_response( int timeout,
                            pthread_mutex_t* cb_mutex_ptr,
                            pthread_cond_t*  cond_ptr,
                            bool *cond_var )
{
    bool    ret_val = false;               /* the return value of this function */
    int     rc = 0;                        /* return code from pthread calls */
    struct timeval    present_time;
    struct timespec   expire_time;

    HAL_LOG_DEBUG("%s: timeout=%d", __FUNCTION__, timeout );

    /* special case where callback is issued before the main function
       can wait on cond */
    if (*cond_var == true) {
        HAL_LOG_DEBUG("%s: cb has arrived without waiting", __FUNCTION__ );
        ret_val = true;
    }
    else {
        /* Calculate absolute expire time */
        gettimeofday(&present_time, NULL);

        /* Convert from timeval to timespec */
        expire_time.tv_sec  = present_time.tv_sec;
        expire_time.tv_nsec = present_time.tv_usec * 1000;
        expire_time.tv_sec += timeout / 1000;

        /* calculate carry over */
        if ( (present_time.tv_usec + (timeout % 1000) * 1000) >= 1000000) {
            expire_time.tv_sec += 1;
        }
        expire_time.tv_nsec = (expire_time.tv_nsec + (timeout % 1000) * 1000000) % 1000000000;

        while( *cond_var != true && rc != ETIMEDOUT ) {
            if( 0 == timeout ) {
                rc = pthread_cond_wait( cond_ptr, cb_mutex_ptr );
            }
            else {
                /* Wait for the callback until timeout expires */
                rc = pthread_cond_timedwait( cond_ptr, cb_mutex_ptr,
                                             &expire_time);
            }
            if( 0 != rc ) {
                HAL_LOG_ERROR( "%s: pthread_cond_timedwait() rc=%d (cond: %i)",
                               __FUNCTION__, rc, *cond_var );
            }
            ret_val = ( rc == 0 || *cond_var ) ? true : false;
        }
    }

    *cond_var = false;

    return ret_val;
}

/*===========================================================================
  FUNCTION:  hal_signal_response()
===========================================================================*/
/*!
*/
static void hal_signal_response( bool error, pthread_cond_t* cond_ptr )
{
    g_ar_control->error = error;
    g_ar_control->is_resp_arrived = true;

    pthread_cond_signal( cond_ptr );
}

/*===========================================================================
  FUNCTION:  hal_signal_ind()
===========================================================================*/
/*!
*/
static void hal_signal_ind( pthread_cond_t* cond_ptr )
{
    g_ar_control->is_ind_arrived = true;
    pthread_cond_signal( cond_ptr );
}

/*===========================================================================
  FUNCTION:  hal_flush_send_cmplt
===========================================================================*/
/*!
  @brief
  Sends ACTIVITY_EVENT_TYPE_FLUSH_COMPLETE event.  Must hold data_mutex.
 */
static void hal_flush_send_cmplt()
{
    HAL_LOG_INFO("%s", __FUNCTION__);

    activity_event_t flush_evt = {
        .event_type = ACTIVITY_EVENT_FLUSH_COMPLETE,
        .activity = 0,
        .timestamp = 0,
        .reserved = {0}
    };

    if(true == g_ar_control->flush_requested ) {
        g_ar_control->flush_requested = false;
        g_ar_control->hal_ar_cb->activity_callback( g_ar_control->hal_ar_cb, &flush_evt, 1 );
        hal_signal_ind( &g_ar_control->data_arrived_cond );
    }
}

/*===========================================================================
  FUNCTION:  hal_sam_cmc_send_algo_attrib_req
===========================================================================*/
/*!
*/
static bool hal_sam_cmc_send_algo_attrib_req()
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_get_algo_attrib_req_msg_v01 *sam_req;

    HAL_LOG_INFO("%s", __FUNCTION__);

    error = sensor1_alloc_msg_buf( g_ar_control->hndl,
                                   sizeof(sns_sam_get_algo_attrib_req_msg_v01),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return false;
    }

    req_hdr.service_number = CMC_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_CMC_GET_ATTRIBUTES_REQ_V02;
    req_hdr.msg_size = sizeof( sns_sam_get_algo_attrib_req_msg_v01 );
    req_hdr.txn_id = 0;

    /* Send request */
    g_ar_control->error = false;
    if( (error = sensor1_write( g_ar_control->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
        /* free the message buffer */
        sensor1_free_msg_buf( g_ar_control->hndl, sam_req );
        HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
        return false;
    }

    /* waiting for response */
    if (hal_wait_for_response( TIME_OUT_MS,
                               &g_ar_control->cb_mutex,
                               &g_ar_control->cb_arrived_cond,
                               &g_ar_control->is_resp_arrived ) == false ) {
        HAL_LOG_ERROR( "%s: ERROR: No response from the request", __FUNCTION__ );
        return false;
    }

    HAL_LOG_DEBUG( "%s: Received Response: %d", __FUNCTION__, g_ar_control->error );
    /* received response */
    if( g_ar_control->error == true ) {
        return false;
    }

    return true;
}

/*===========================================================================
  FUNCTION:  hal_sam_cmc_send_batch_req
===========================================================================*/
/*!
*/
static int hal_sam_cmc_send_batch_req( hal_ar_ctl_t* ar_ctl, uint32_t batch_rate)
{
    sensor1_error_e  error;
    sensor1_msg_header_s req_hdr;
    sns_sam_cmc_batch_req_msg_v02 *sam_req;
    float batch_rate_in_hz;

    HAL_LOG_DEBUG( "%s: batch_rate=%f (Hz), IID: %d",
                   __FUNCTION__, FX_FIXTOFLT_Q16( batch_rate ),
                   ar_ctl->sam_service[ CMC_SVC_NUM ].instance_id );

    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_cmc_batch_req_msg_v02),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return -1;
    }
    req_hdr.service_number = CMC_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_CMC_BATCH_REQ_V02;
    req_hdr.msg_size = sizeof( sns_sam_cmc_batch_req_msg_v02 );
    req_hdr.txn_id = 0;

    sam_req->instance_id = ar_ctl->sam_service[ CMC_SVC_NUM ].instance_id;

    /* Wake client from suspend when buffer is full */
    sam_req->req_type_valid = true;
    sam_req->req_type = 1;

    /* convert batch rate from Hz in Q16 to batch period in seconds in Q16 */
    batch_rate_in_hz = FX_FIXTOFLT_Q16( batch_rate );
    sam_req->batch_period = FX_FLTTOFIX_Q16( 1.0 / batch_rate_in_hz );

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
      /* free the message buffer */
      sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
      HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
      return -1;
    }

    return ar_ctl->error ? -1 : 0;
}

/*===========================================================================
  FUNCTION:  hal_sam_cmc_send_batch_update_req
===========================================================================*/
/*!
*/
static int hal_sam_cmc_send_batch_update_req( hal_ar_ctl_t* ar_ctl, uint32_t batch_rate )
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_cmc_update_batch_period_req_msg_v02 *sam_req;
    float batch_rate_in_hz;

    HAL_LOG_DEBUG( "%s: batch_rate=%f (Hz)", __FUNCTION__, FX_FIXTOFLT_Q16( batch_rate ) );

    /* Message Body */
    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_cmc_update_batch_period_req_msg_v02),
                                   (void**)&sam_req );

    if( SENSOR1_SUCCESS != error )
    {
      HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
      return -1;
    }

    req_hdr.service_number = CMC_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_CMC_UPDATE_BATCH_PERIOD_REQ_V02;
    req_hdr.msg_size = sizeof( sns_sam_cmc_update_batch_period_req_msg_v02 );
    req_hdr.txn_id = 0;

    sam_req->instance_id = ar_ctl->sam_service[ CMC_SVC_NUM ].instance_id;

    /* convert batch rate from Hz in Q16 to batch period in seconds in Q16 */
    batch_rate_in_hz = FX_FIXTOFLT_Q16( batch_rate );
    sam_req->active_batch_period = FX_FLTTOFIX_Q16( 1.0 / batch_rate_in_hz );

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS )
    {
      /* free the message buffer */
      sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
      HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
      return -1;
    }

    return ar_ctl->error ? -1 : 0;
}

/*===========================================================================
FUNCTION: hal_sam_cmc_send_enable_req
===========================================================================*/
/*!
*/
static bool hal_sam_cmc_send_enable_req( hal_ar_ctl_t* ar_ctl )
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_cmc_enable_req_msg_v02* sam_req;

    HAL_LOG_INFO("%s", __FUNCTION__);

    /* Always cancel first, just in case */
    hal_sam_send_cancel( ar_ctl->hndl, CMC_SVC_NUM );

    /* Message Body */
    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_cmc_enable_req_msg_v02),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return (false);
    }

    req_hdr.service_number = CMC_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_CMC_ENABLE_REQ_V02;
    req_hdr.msg_size = sizeof(sns_sam_cmc_enable_req_msg_v02);
    req_hdr.txn_id = 0;

    /* set defualt behavior for indications during suspend */
    sam_req->notify_suspend_valid = true;
    sam_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
    sam_req->notify_suspend.send_indications_during_suspend = true;

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
        /* free the message buffer */
        sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
        HAL_LOG_ERROR("%s: sensor1_write() error: %d", __FUNCTION__, error );
        return (false);
    }

    /* waiting for response */
    if (hal_wait_for_response( TIME_OUT_MS,
                               &ar_ctl->cb_mutex,
                               &ar_ctl->cb_arrived_cond,
                               &g_ar_control->is_resp_arrived ) == false ) {
        HAL_LOG_ERROR("%s: ERROR: No reponse from CMC enable request %d",
                       __FUNCTION__, SNS_SAM_CMC_ENABLE_REQ_V02);
        return (false);
    }

    HAL_LOG_DEBUG("%s: Received CMC enable response: %d", __FUNCTION__, ar_ctl->error);

    /* received response */
    if( ar_ctl->error == true ) {
        return (false);
    }

    return (true);
}

/*===========================================================================
FUNCTION: hal_sam_cmc_send_update_reporting_req
===========================================================================*/
/*!
*/
static bool hal_sam_cmc_send_update_reporting_req( hal_ar_ctl_t* ar_ctl,
                                            uint32_t activity_handle, uint32_t event_type,
                                            int64_t max_batch_report_latency_ns, bool enable,
                                            bool single_event )
{
    uint32_t handle;
    uint32_t evt_type;
    float report_rate_f = 0;
    uint32_t report_rate_Q16 = 0;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_cmc_update_reporting_req_msg_v02* sam_req;

    HAL_LOG_INFO("%s: activity_handle:%d event_type:%d enable:%d max_batch_report_latency_ns:%"PRId64,
                  __FUNCTION__, activity_handle, event_type, enable, max_batch_report_latency_ns);

    /* Message Body */
    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_cmc_update_reporting_req_msg_v02),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return (false);
    }

    req_hdr.service_number = CMC_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_CMC_UPDATE_REPORTING_REQ_V02;
    req_hdr.msg_size = sizeof(sns_sam_cmc_update_reporting_req_msg_v02);
    req_hdr.txn_id = 0;

    switch(activity_handle) {
        case HANDLE_IN_VEHICLE:
             handle = SNS_SAM_CMC_MS_VEHICLE_V02;
             break;
        case HANDLE_ON_BICYCLE:
             handle = SNS_SAM_CMC_MS_BIKE_V02;
             break;
        case HANDLE_WALKING:
             handle = SNS_SAM_CMC_MS_WALK_V02;
             break;
        case HANDLE_RUNNING:
             handle = SNS_SAM_CMC_MS_RUN_V02;
             break;
        case HANDLE_STILL:
             handle = SNS_SAM_CMC_MS_STATIONARY_V02;
             break;
        default:
             HAL_LOG_ERROR("%s: Unknown Activity %d", __FUNCTION__, activity_handle);
             return (false);
    }

    switch(event_type) {
        case ACTIVITY_EVENT_FLUSH_COMPLETE:
             evt_type = ACTIVITY_EVENT_FLUSH_COMPLETE;
             break;
        case ACTIVITY_EVENT_ENTER:
             evt_type = SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_ENTER_V02;
             break;
        case ACTIVITY_EVENT_EXIT:
             evt_type = SNS_SAM_CMC_MS_EVENT_REPORT_TYPE_EXIT_V02;
             break;
        default:
             HAL_LOG_ERROR("%s: Unknown Event %d", __FUNCTION__, event_type);
             return (false);
    }

    HAL_LOG_DEBUG("%s: handle=%d evt_type=%d", __FUNCTION__, handle, evt_type);
    sam_req->instance_id = ar_ctl->sam_service[ CMC_SVC_NUM ].instance_id;
    sam_req->report_motion_state = handle;
    sam_req->enable = enable;
    sam_req->report_event_type = evt_type;

    if( single_event == true ) {
        sam_req->report_ms_type = SNS_SAM_CMC_MS_REPORT_TYPE_SINGLE_V02;
    }
    else {
        sam_req->report_ms_type = SNS_SAM_CMC_MS_REPORT_TYPE_ALL_V02;
    }

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
        /* free the message buffer */
        sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
        HAL_LOG_ERROR("%s: sensor1_write() error: %d", __FUNCTION__, error );
        return (false);
    }

    /* waiting for response */
    if (hal_wait_for_response( TIME_OUT_MS,
                               &ar_ctl->cb_mutex,
                               &ar_ctl->cb_arrived_cond,
                               &g_ar_control->is_resp_arrived ) == false ) {
        HAL_LOG_ERROR("%s: ERROR: No reponse for request %d",
                       __FUNCTION__, SNS_SAM_CMC_UPDATE_REPORTING_REQ_V02);
        return (false);
    }

    HAL_LOG_DEBUG("%s: Received response: %d", __FUNCTION__, ar_ctl->error);

    /* Send batch request */
    if( max_batch_report_latency_ns > 0 ) {
        report_rate_f = NSEC_TO_HZ(max_batch_report_latency_ns);
        report_rate_Q16 = FX_FLTTOFIX_Q16(report_rate_f);

        hal_sam_cmc_send_batch_req(ar_ctl, report_rate_Q16);
        hal_sam_cmc_send_batch_update_req(ar_ctl, report_rate_Q16);
    }

    /* received response */
    if( ar_ctl->error == true ) {
        return (false);
    }

    return (true);
}

/*===========================================================================
  FUNCTION:  hal_process_sam_cmc_resp
===========================================================================*/
/*!
*/
static void hal_process_sam_cmc_resp( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01*) msg_ptr;
    bool                          error = false;

    HAL_LOG_DEBUG("%s: %d", __FUNCTION__,  msg_hdr->msg_id );

    if( crsp_ptr->sns_result_t != 0 &&
        msg_hdr->msg_id != SNS_SAM_CMC_CANCEL_RESP_V02) {
        HAL_LOG_ERROR( "%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                       msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t );
    }

    if( true != error ) {
        switch( msg_hdr->msg_id ) {
            case SNS_SAM_CMC_ENABLE_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_ENABLE_RESP_V02", __FUNCTION__);
                g_ar_control->sam_service[ CMC_SVC_NUM ].instance_id =
                    ((sns_sam_cmc_enable_resp_msg_v02*) msg_ptr)->instance_id;
                break;
            case SNS_SAM_CMC_CANCEL_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_CANCEL_RESP_V02", __FUNCTION__);
                /* Reset instance ID */
                g_ar_control->sam_service[ CMC_SVC_NUM ].instance_id = 0xFF;
                break;
            case SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02", __FUNCTION__);
                g_ar_control->cmc_version =
                    ((sns_sam_get_algo_attrib_resp_msg_v01*) msg_ptr)->algorithm_revision;
                HAL_LOG_DEBUG( "%s: version %d", __FUNCTION__, g_ar_control->cmc_version);
                break;
            case SNS_SAM_CMC_BATCH_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_BATCH_RESP_V02", __FUNCTION__);
                break;
            case SNS_SAM_CMC_UPDATE_BATCH_PERIOD_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_UPDATE_BATCH_PERIOD_RESP_V02", __FUNCTION__);
                break;
            case SNS_SAM_CMC_UPDATE_REPORTING_RESP_V02:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_UPDATE_REPORTING_RESP_V02", __FUNCTION__);
                break;
            default:
                HAL_LOG_ERROR("%s: Unknown msg id: %d", __FUNCTION__, msg_hdr->msg_id );
                return;
        }
    }

    hal_signal_response( error, &g_ar_control->cb_arrived_cond );
}

/*===========================================================================
  FUNCTION:  hal_process_sam_cmc_ind
===========================================================================*/
/*!
*/
static void hal_process_sam_cmc_ind( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
    hal_sam_sample_t *sample_list = NULL;
    hal_sam_sample_t *curr_sample = NULL;
    uint32_t i = 0;
    uint32_t items_len = 0;
    uint32_t count = 0;

    HAL_LOG_DEBUG("%s: msg_id=%d", __FUNCTION__,  msg_hdr->msg_id );

    if (SNS_SAM_CMC_REPORT_IND_V02 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_CMC_REPORT_IND_V02", __FUNCTION__);
        sns_sam_cmc_report_ind_msg_v02* sam_ind =
            (sns_sam_cmc_report_ind_msg_v02*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        } else {
            HAL_LOG_DEBUG("%s: ms state=%d ms event=%d timestamp=%d",
                          __FUNCTION__, sam_ind->report_data.motion_state,
                          sam_ind->ms_event, sam_ind->timestamp);
            if( (sam_ind->report_data.motion_state == SNS_SAM_CMC_MS_VEHICLE_V02 &&
                 ((sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_IN_VEHICLE].enter == true) ||
                  (sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_IN_VEHICLE].exit == true))) ||

                (sam_ind->report_data.motion_state == SNS_SAM_CMC_MS_BIKE_V02 &&
                 ((sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_ON_BICYCLE].enter == true) ||
                  (sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_ON_BICYCLE].exit == true))) ||

                (sam_ind->report_data.motion_state == SNS_SAM_CMC_MS_WALK_V02 &&
                 ((sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_WALKING].enter == true) ||
                  (sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_WALKING].exit == true))) ||

                (sam_ind->report_data.motion_state == SNS_SAM_CMC_MS_RUN_V02 &&
                 ((sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_RUNNING].enter == true) ||
                  (sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_RUNNING].exit == true))) ||

                (sam_ind->report_data.motion_state == SNS_SAM_CMC_MS_STATIONARY_V02 &&
                 ((sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_STILL].enter == true) ||
                  (sam_ind->ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                   g_ar_control->hal_track_activity[HANDLE_STILL].exit == true))) ) {
                count = 1;
                sample_list->timestamp = sam_ind->timestamp;
                sample_list->data[0] = sam_ind->report_data.motion_state;
                sample_list->data[1] = sam_ind->ms_event;
            }
        }
    }
    else if(SNS_SAM_CMC_BATCH_IND_V02 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_CMC_BATCH_IND_V02", __FUNCTION__);
        sns_sam_cmc_batch_ind_msg_v02* sam_ind =
            (sns_sam_cmc_batch_ind_msg_v02*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sam_ind->items_len * sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        }
        else {
            curr_sample = sample_list;
            items_len = sam_ind->items_len;

            for(i = 0; i < sam_ind->items_len; i++) {
                HAL_LOG_DEBUG("%s: ms state=%d ms event=%d timestamp=%d",
                              __FUNCTION__, sam_ind->items[ i ].report.motion_state,
                              sam_ind->items[ i ].ms_event, sam_ind->items[ i ].timestamp);
                if( (sam_ind->items[ i ].report.motion_state == SNS_SAM_CMC_MS_VEHICLE_V02 &&
                     ((sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_IN_VEHICLE].enter == true) ||
                      (sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_IN_VEHICLE].exit == true))) ||

                    (sam_ind->items[ i ].report.motion_state == SNS_SAM_CMC_MS_BIKE_V02 &&
                     ((sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_ON_BICYCLE].enter == true) ||
                      (sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_ON_BICYCLE].exit == true))) ||

                    (sam_ind->items[ i ].report.motion_state == SNS_SAM_CMC_MS_WALK_V02 &&
                     ((sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_WALKING].enter == true) ||
                      (sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_WALKING].exit == true))) ||

                    (sam_ind->items[ i ].report.motion_state == SNS_SAM_CMC_MS_RUN_V02 &&
                     ((sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_RUNNING].enter == true) ||
                      (sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_RUNNING].exit == true))) ||

                    (sam_ind->items[ i ].report.motion_state == SNS_SAM_CMC_MS_STATIONARY_V02 &&
                     ((sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_ENTER_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_STILL].enter == true) ||
                      (sam_ind->items[ i ].ms_event == SNS_SAM_CMC_MS_EVENT_EXIT_V02 &&
                       g_ar_control->hal_track_activity[HANDLE_STILL].exit == true))) ) {
                        curr_sample->data[0] = sam_ind->items[ i ].report.motion_state;
                        curr_sample->data[1] = sam_ind->items[ i ].ms_event;
                        curr_sample->timestamp = sam_ind->items[ i ].timestamp;
                        curr_sample++;
                        count++;
                }
                else {
                    HAL_LOG_ERROR("%s: Unknown Activity %d", __FUNCTION__, curr_sample->data[0]);
                }
            }
        }
    }
    else {
        HAL_LOG_ERROR( "%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id );
    }

    HAL_LOG_DEBUG("%s: items_len=%d count=%d", __FUNCTION__, items_len, count);
    activity_event_t activity_data[count];
    for (i = 0; i < count; i++) {
        curr_sample = &sample_list[i];
        HAL_LOG_DEBUG("%s: data[0]=%d data[1]=%d timestamp=%d",
                      __FUNCTION__, curr_sample->data[0], curr_sample->data[1], curr_sample->timestamp);

        switch(curr_sample->data[0]) {
            case SNS_SAM_CMC_MS_VEHICLE_V02:
                 activity_data[i].activity = HANDLE_IN_VEHICLE;
                 break;
            case SNS_SAM_CMC_MS_BIKE_V02:
                 activity_data[i].activity = HANDLE_ON_BICYCLE;
                 break;
            case SNS_SAM_CMC_MS_WALK_V02:
                 activity_data[i].activity = HANDLE_WALKING;
                 break;
            case SNS_SAM_CMC_MS_RUN_V02:
                 activity_data[i].activity = HANDLE_RUNNING;
                 break;
            case SNS_SAM_CMC_MS_STATIONARY_V02:
                 activity_data[i].activity = HANDLE_STILL;
                 break;
            default:
                 HAL_LOG_ERROR("%s: Unknown Activity %d", __FUNCTION__, curr_sample->data[0]);
                 continue;
        }

        switch(curr_sample->data[1]) {
            case SNS_SAM_CMC_MS_EVENT_ENTER_V02:
                 activity_data[i].event_type = ACTIVITY_EVENT_ENTER;
                 break;
            case SNS_SAM_CMC_MS_EVENT_EXIT_V02:
                 activity_data[i].event_type = ACTIVITY_EVENT_EXIT;
                 break;
            default:
                 HAL_LOG_ERROR("%s: Unknown Event %d", __FUNCTION__, curr_sample->data[1]);
                 continue;
        }
        struct timespec curr_time;
        int64_t curr_boot_ts;

        clock_gettime( CLOCK_BOOTTIME, &curr_time );
        curr_boot_ts = ((int64_t)curr_time.tv_sec * 1000000000) + curr_time.tv_nsec;
        HAL_LOG_DEBUG( "%s: boot TS: %"PRIi64"", __FUNCTION__, curr_boot_ts );

        activity_data[i].timestamp = hal_timestamp_calc( (uint64_t)curr_sample->timestamp,
                                                         activity_data[i].activity );

        HAL_LOG_DEBUG("%s: activity %d, event_type %d timestamp %"PRId64, __FUNCTION__,
                      activity_data[i].activity, activity_data[i].event_type, activity_data[i].timestamp);
    }

    if (count > 0) {
        g_ar_control->hal_ar_cb->activity_callback( g_ar_control->hal_ar_cb, activity_data, count );
        hal_signal_ind( &g_ar_control->data_arrived_cond );
    }

    free(sample_list);
}

/*===========================================================================
  FUNCTION:  hal_sam_tilt_detector_send_batch_req
===========================================================================*/
static int hal_sam_tilt_detector_send_batch_req( hal_ar_ctl_t* ar_ctl,
                                                 uint32_t batch_rate )
{
    sensor1_error_e  error;
    sensor1_msg_header_s req_hdr;
    sns_sam_tilt_detector_batch_req_msg_v01 *sam_req;
    float batch_rate_in_hz;

    HAL_LOG_DEBUG( "%s: batch_rate=%f (Hz), IID: %d",
                   __FUNCTION__, FX_FIXTOFLT_Q16( batch_rate ),
                   ar_ctl->sam_service[ TILT_DETECTOR_SVC_NUM ].instance_id );

    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_tilt_detector_batch_req_msg_v01),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return -1;
    }
    req_hdr.service_number = TILT_DETECTOR_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_TILT_DETECTOR_BATCH_REQ_V01;
    req_hdr.msg_size = sizeof( sns_sam_tilt_detector_batch_req_msg_v01 );
    req_hdr.txn_id = 0;

    sam_req->instance_id = ar_ctl->sam_service[ TILT_DETECTOR_SVC_NUM ].instance_id;

    /* Wake client from suspend when buffer is full */
    sam_req->req_type_valid = true;
    sam_req->req_type = 1;

    /* convert batch rate from Hz in Q16 to batch period in seconds in Q16 */
    batch_rate_in_hz = FX_FIXTOFLT_Q16( batch_rate );
    sam_req->batch_period = FX_FLTTOFIX_Q16( 1.0 / batch_rate_in_hz );

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
        /* free the message buffer */
        sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
        HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
        return -1;
    }

    return ar_ctl->error ? -1 : 0;
}

/*===========================================================================
  FUNCTION:  hal_sam_tilt_detector_send_batch_update_req
===========================================================================*/
static int hal_sam_tilt_detector_send_batch_update_req( hal_ar_ctl_t* ar_ctl,
                                                        uint32_t batch_rate )
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_tilt_detector_update_batch_period_req_msg_v01 *sam_req;
    float batch_rate_in_hz;

    HAL_LOG_DEBUG( "%s: batch_rate=%f (Hz)", __FUNCTION__, FX_FIXTOFLT_Q16( batch_rate ) );

    /* Message Body */
    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_tilt_detector_update_batch_period_req_msg_v01),
                                   (void**)&sam_req );

    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return -1;
    }

    req_hdr.service_number = TILT_DETECTOR_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_REQ_V01;
    req_hdr.msg_size = sizeof( sns_sam_tilt_detector_update_batch_period_req_msg_v01 );
    req_hdr.txn_id = 0;

    sam_req->instance_id = ar_ctl->sam_service[ TILT_DETECTOR_SVC_NUM ].instance_id;

    /* convert batch rate from Hz in Q16 to batch period in seconds in Q16 */
    batch_rate_in_hz = FX_FIXTOFLT_Q16( batch_rate );
    sam_req->active_batch_period = FX_FLTTOFIX_Q16( 1.0 / batch_rate_in_hz );

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS ) {
        /* free the message buffer */
        sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
        HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
        return -1;
    }

    return ar_ctl->error ? -1 : 0;
}

/*===========================================================================
  FUNCTION:  hal_sam_tilt_detector_send_enable_req
===========================================================================*/
/*!
*/
static bool hal_sam_tilt_detector_send_enable_req( hal_ar_ctl_t* ar_ctl,
                                                   int64_t max_batch_report_latency_ns )
{
    float report_rate_f = 0;
    uint32_t report_rate_Q16 = 0;
    sensor1_error_e         error;
    sensor1_msg_header_s    req_hdr;
    sns_sam_tilt_detector_enable_req_msg_v01*  sam_req;

    HAL_LOG_INFO("%s", __FUNCTION__);

    /* Always cancel first, just in case */
    hal_sam_send_cancel( ar_ctl->hndl, TILT_DETECTOR_SVC_NUM );

    /* Message Body */
    error = sensor1_alloc_msg_buf( ar_ctl->hndl,
                                   sizeof(sns_sam_tilt_detector_enable_req_msg_v01),
                                   (void**)&sam_req );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return (false);
    }

    /* Message header */
    req_hdr.service_number = TILT_DETECTOR_SVC_NUM;
    req_hdr.msg_id = SNS_SAM_TILT_DETECTOR_ENABLE_REQ_V01;
    req_hdr.msg_size = sizeof( sns_sam_tilt_detector_enable_req_msg_v01 );
    req_hdr.txn_id = 0;

    /* set default behavior for indications during suspend */
    sam_req->notify_suspend_valid = true;
    sam_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
    sam_req->notify_suspend.send_indications_during_suspend = true;
    sam_req->angle_thresh = 35;

    /* Send request */
    ar_ctl->error = false;
    if( (error = sensor1_write( ar_ctl->hndl, &req_hdr,
                                sam_req )) != SENSOR1_SUCCESS )
    {
        /* free the message buffer */
        sensor1_free_msg_buf( ar_ctl->hndl, sam_req );
        HAL_LOG_ERROR("%s: sensor1_write() error: %d", __FUNCTION__, error );
        return (false);
    }

    /* waiting for response */
    if (hal_wait_for_response( TIME_OUT_MS,
                               &ar_ctl->cb_mutex,
                               &ar_ctl->cb_arrived_cond,
                               &g_ar_control->is_resp_arrived ) == false ) {
        HAL_LOG_ERROR("%s: ERROR: No response from request %d", __FUNCTION__,
                       SNS_SAM_TILT_DETECTOR_ENABLE_REQ_V01);
        return (false);
    }

    HAL_LOG_DEBUG("%s: Received Response: %d", __FUNCTION__, ar_ctl->error);

    /* Send batch request */
    if( max_batch_report_latency_ns > 0 ) {
        report_rate_f = NSEC_TO_HZ(max_batch_report_latency_ns);
        report_rate_Q16 = FX_FLTTOFIX_Q16(report_rate_f);

        hal_sam_tilt_detector_send_batch_req(ar_ctl, report_rate_Q16);
        hal_sam_tilt_detector_send_batch_update_req(ar_ctl, report_rate_Q16);
    }

    /* received response */
    if( ar_ctl->error == true ) {
        return (false);
    }

    return (true);
}

/*===========================================================================
  FUNCTION:  hal_process_sam_tilt_detector_resp
===========================================================================*/
/*!
*/
static void hal_process_sam_tilt_detector_resp( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01*) msg_ptr;
    bool                          error = false;

    HAL_LOG_DEBUG("%s: msg_id=%d", __FUNCTION__,  msg_hdr->msg_id );

    if( crsp_ptr->sns_result_t != 0 &&
        msg_hdr->msg_id != SNS_SAM_TILT_DETECTOR_CANCEL_RESP_V01 ) {
        HAL_LOG_ERROR( "%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                       msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t );
        error = true;
    }

    if ( true != error ) {
        switch( msg_hdr->msg_id ) {
            /* Enable Responses */
            case SNS_SAM_TILT_DETECTOR_ENABLE_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_TILT_DETECTOR_ENABLE_RESP_V01", __FUNCTION__);
                g_ar_control->sam_service[ TILT_DETECTOR_SVC_NUM ].instance_id =
                  ((sns_sam_tilt_detector_enable_resp_msg_v01*) msg_ptr)->instance_id;
                break;
            /* Cancel Responses */
            case SNS_SAM_TILT_DETECTOR_CANCEL_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_TILT_DETECTOR_CANCEL_RESP_V01", __FUNCTION__);
                /* Reset instance ID */
                g_ar_control->sam_service[ TILT_DETECTOR_SVC_NUM ].instance_id = 0xFF;
                break;
            case SNS_SAM_TILT_DETECTOR_BATCH_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_TILT_DETECTOR_BATCH_RESP_V01", __FUNCTION__);
                break;
            case SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_TILT_DETECTOR_UPDATE_BATCH_PERIOD_RESP_V01", __FUNCTION__);
                break;
            default:
                HAL_LOG_ERROR("%s: Unknown msg id: %d", __FUNCTION__, msg_hdr->msg_id );
                return;
        }
    }

    hal_signal_response( error, &g_ar_control->cb_arrived_cond );
}

/*===========================================================================
  FUNCTION:  hal_process_sam_tilt_detector_ind
===========================================================================*/
/*!
*/
static void hal_process_sam_tilt_detector_ind( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
    hal_sam_sample_t *sample_list = NULL;
    hal_sam_sample_t *curr_sample = NULL;
    uint32_t i = 0;
    uint32_t count = 0;
    int64_t tilt_ts = 0;

    HAL_LOG_DEBUG("%s: msg_id=%d", __FUNCTION__,  msg_hdr->msg_id );

    if (SNS_SAM_TILT_DETECTOR_REPORT_IND_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_TILT_DETECTOR_REPORT_IND_V01", __FUNCTION__);
        sns_sam_tilt_detector_report_ind_msg_v01* sam_ind =
            (sns_sam_tilt_detector_report_ind_msg_v01*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        } else {
            /* To cover both enter & exit events */
            if( (g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) &&
                (g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) ) {
                count = 2;
            }
            else {
                count = 1;
            }
            sample_list->timestamp = sam_ind->tilt_timestamp;
        }
    }
    else if(SNS_SAM_TILT_DETECTOR_BATCH_IND_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_TILT_DETECTOR_BATCH_IND_V01", __FUNCTION__);
        sns_sam_tilt_detector_batch_ind_msg_v01* sam_ind =
            (sns_sam_tilt_detector_batch_ind_msg_v01*)msg_ptr;

        /* To cover both enter & exit events */
        if( (g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) &&
            (g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) ) {
            sample_list = (hal_sam_sample_t *)malloc(sam_ind->tilt_timestamps_len * 2 * sizeof(hal_sam_sample_t));
        }
        else {
            sample_list = (hal_sam_sample_t *)malloc(sam_ind->tilt_timestamps_len * sizeof(hal_sam_sample_t));
        }
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        }
        else {
            curr_sample = sample_list;

            /* To cover both enter & exit events */
            if( (g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) &&
                (g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) ) {
                count = sam_ind->tilt_timestamps_len * 2;
            }
            else {
                count = sam_ind->tilt_timestamps_len;
            }

            for(i = 0; i < count; i++) {
                if( (g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) &&
                    (g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) ) {
                    curr_sample->timestamp = sam_ind->tilt_timestamps[ i / 2 ];
                }
                else {
                    curr_sample->timestamp = sam_ind->tilt_timestamps[ i ];
                }
                curr_sample++;
            }
        }
    }
    else {
        HAL_LOG_ERROR( "%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id );
    }

    activity_event_t activity_data[count];
    HAL_LOG_DEBUG("%s: count=%d", __FUNCTION__, count);
    for (i = 0; i < count; i++) {
        curr_sample = &sample_list[i];

        activity_data[i].activity = HANDLE_TILTING;
        tilt_ts = hal_timestamp_calc( (uint64_t)curr_sample->timestamp,
                                                        activity_data[i].activity );
        activity_data[i].timestamp = tilt_ts;

        if( (g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) &&
            (g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) ) {
            activity_data[i].event_type = ACTIVITY_EVENT_ENTER;

            HAL_LOG_DEBUG("%s: activity %d, event_type %d timestamp %"PRId64, __FUNCTION__,
                          activity_data[i].activity, activity_data[i].event_type, activity_data[i].timestamp);

            /* Tilt doesn't generate enter & exit events.
             * Need to manage both enter & exit events in HAL to satisfy AR requirements.
             * Adding exit event after enter if both enter & exit events were requested. */
            i++;
            activity_data[i].activity = HANDLE_TILTING;
            activity_data[i].event_type = ACTIVITY_EVENT_EXIT;
            activity_data[i].timestamp = tilt_ts;

            HAL_LOG_DEBUG("%s: activity %d, event_type %d timestamp %"PRId64, __FUNCTION__,
                          activity_data[i].activity, activity_data[i].event_type, activity_data[i].timestamp);
        }
        else if(g_ar_control->hal_track_activity[HANDLE_TILTING].enter == true) {
            activity_data[i].event_type = ACTIVITY_EVENT_ENTER;

            HAL_LOG_DEBUG("%s: activity %d, event_type %d timestamp %"PRId64, __FUNCTION__,
                          activity_data[i].activity, activity_data[i].event_type, activity_data[i].timestamp);
        }
        else if(g_ar_control->hal_track_activity[HANDLE_TILTING].exit == true) {
            activity_data[i].event_type = ACTIVITY_EVENT_EXIT;

            HAL_LOG_DEBUG("%s: activity %d, event_type %d timestamp %"PRId64, __FUNCTION__,
                          activity_data[i].activity, activity_data[i].event_type, activity_data[i].timestamp);
        }
    }

    if(count > 0) {
        g_ar_control->hal_ar_cb->activity_callback( g_ar_control->hal_ar_cb, activity_data, count );
        hal_signal_ind( &g_ar_control->data_arrived_cond );
    }
    free(sample_list);
}

/*===========================================================================

  FUNCTION:  hal_cleanup_resources
  Cleans up resources such as sensor1 connection, and internal state variables

  This function is called when the sensor daemon goes down, i.e. upon
  receiving BROKEN_PIPE message

  Return void

===========================================================================*/
static void hal_cleanup_resources()
{
    int i;
    hal_ar_ctl_t* ar_ctl = g_ar_control;
    HAL_LOG_DEBUG( "%s", __FUNCTION__ );

    /* close sensor1 connection */
    if( ar_ctl->hndl != (sensor1_handle_s*)-1) {
        sensor1_close( ar_ctl->hndl );
        ar_ctl->hndl = (sensor1_handle_s*)-1;
    }

    /* Signal any waiting request */
    hal_signal_response( false, &g_ar_control->cb_arrived_cond );

    /* reset some variables */
    ar_ctl->is_ind_arrived = false;
    ar_ctl->error = false;

    for( i=0; i<MAX_NUM_ACTIVITIES; i++ )
    {
        ar_ctl->last_event[i].timestamp = 0;
    }

    for( i=0; i<MAX_SAM_SERVICES; i++ )
    {
        ar_ctl->sam_service[ i ].instance_id = INVALID_INSTANCE_ID;
    }
}

/*===========================================================================
  FUNCTION:  hal_reinit

  Re-intializes the HAL. This is called when the sensor daemon
  has crashed and restarted.  Must hold cb_mutex

  Returns error code if error, otherwise SENSOR1_SUCCESS if success
===========================================================================*/
static int hal_reinit()
{
    hal_ar_ctl_t* ar_ctl = g_ar_control;
    int ret = SENSOR1_EFAILED;
    int i;

    HAL_LOG_DEBUG("%s", __FUNCTION__ );

    if( ar_ctl->hndl == (sensor1_handle_s*)-1 ) {
        ret = sensor1_open( &ar_ctl->hndl, &hal_sensor1_data_cb,
                              (intptr_t)ar_ctl );
        HAL_LOG_DEBUG("%s: sensor1_open() ret=%d hndl=%"PRIuPTR, __FUNCTION__, ret,
                      (uintptr_t)(ar_ctl->hndl) );

        /* re-acquire resources if the daemon is ready
         * otherwise callback will invoked with RETRY and we need to call this again */
        if( ret == SENSOR1_SUCCESS ) {
            if( g_ar_control->cmc_enabled == true ) {
                ret = hal_sam_cmc_send_enable_req( ar_ctl );
                if( !ret ) {
                    HAL_LOG_ERROR("%s: CMC enable request failed", __FUNCTION__);
                    return -1;
                }
                ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, 0, 0, 0, false, false);
                if( !ret ) {
                    HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                    return -1;
                }
                for( i = 0; i < MAX_NUM_ACTIVITIES; i++ ) {
                    if( i != HANDLE_TILTING ) {
                        if(g_ar_control->hal_track_activity[i].enter == true) {
                            ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, i, ACTIVITY_EVENT_ENTER,
                                                          g_ar_control->hal_track_activity[i].batch_rate, true, true);
                            if( !ret ) {
                                HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                                return -1;
                            }
                        }
                        else if(g_ar_control->hal_track_activity[i].exit == true) {
                            ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, i, ACTIVITY_EVENT_EXIT,
                                                          g_ar_control->hal_track_activity[i].batch_rate, true, true);
                            if( !ret ) {
                                HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                                return -1;
                            }
                        }
                    }
                }
            }

            if( g_ar_control->tilt_enabled == true ) {
                ret = hal_sam_tilt_detector_send_enable_req( ar_ctl,
                          g_ar_control->hal_track_activity[HANDLE_TILTING].batch_rate );
                if( !ret ) {
                    HAL_LOG_ERROR("%s: Tilt Detector enable request failed", __FUNCTION__);
                    return -1;
                }
            }
        }
        else if( ret == SENSOR1_EWOULDBLOCK ) {
            HAL_LOG_ERROR( "%s: sensor1_open returned EWOULDBLOCK. Daemon not ready, will try again",
                           __FUNCTION__ );
        }
        else {
          HAL_LOG_ERROR( "%s: sensor1_open() failed ret=%d", __FUNCTION__, ret );
        }
    }

    return ret;
}

/*===========================================================================
  FUNCTION:  hal_handle_broken_pipe
===========================================================================*/
/*!
 * @brief
 * This function is called when a broken pipe received
*/
static void hal_handle_broken_pipe()
{
    int ret = SENSOR1_EFAILED;
    HAL_LOG_DEBUG( "%s", __FUNCTION__ );

    /* clean up resources upon the daemon DOWN */
    hal_cleanup_resources();

    /* re-initialize and re-acquire resources upon the daemon UP
     * hal_reinit will be invoked again in the callback if the daemon is not ready */
    if( SENSOR1_SUCCESS != ( ret = hal_reinit() ) ) {
        HAL_LOG_ERROR( "%s: hal_reinit() failed ret=%d", __FUNCTION__, ret );
    }
}

/*===========================================================================
  FUNCTION:  hal_sensor1_data_cb
===========================================================================*/
/*!
*/
static void hal_sensor1_data_cb (intptr_t cb_data,
                                 sensor1_msg_header_s *msg_hdr,
                                 sensor1_msg_type_e msg_type,
                                 void *msg_ptr)
{
    UNREFERENCED_PARAMETER(cb_data);
    HAL_LOG_INFO("%s", __FUNCTION__);

    if( msg_hdr != NULL ) {
        HAL_LOG_VERBOSE("%s: msg_type %d, Sn %d, msg Id %d, txn Id %d", __FUNCTION__,
                        msg_type, msg_hdr->service_number, msg_hdr->msg_id, msg_hdr->txn_id );
    }
    else {
        if( msg_type != SENSOR1_MSG_TYPE_BROKEN_PIPE &&
            msg_type != SENSOR1_MSG_TYPE_REQ &&
            msg_type != SENSOR1_MSG_TYPE_RETRY_OPEN ) {
                HAL_LOG_ERROR("%s: Error - invalid msg type with NULL msg_hdr: %u",
                               __FUNCTION__, msg_type );
                return ;
        }
        else {
            HAL_LOG_VERBOSE("%s: msg_type %d", __FUNCTION__, msg_type);
        }
    }

    switch( msg_type ) {
        case SENSOR1_MSG_TYPE_RESP_INT_ERR:
            if ( ( msg_hdr->service_number == TILT_DETECTOR_SVC_NUM ) ||
                 ( msg_hdr->service_number == CMC_SVC_NUM ) ) {
                pthread_mutex_lock( &g_ar_control->cb_mutex );
                hal_signal_response( true, &g_ar_control->cb_arrived_cond );
                pthread_mutex_unlock( &g_ar_control->cb_mutex );
            }
            break;

        case SENSOR1_MSG_TYPE_RESP:
            pthread_mutex_lock( &g_ar_control->cb_mutex );
            if ( msg_hdr->service_number == TILT_DETECTOR_SVC_NUM ) {
                hal_process_sam_tilt_detector_resp( msg_hdr, msg_ptr );
            }
            else if ( ( msg_hdr->service_number == CMC_SVC_NUM ) ) {
                hal_process_sam_cmc_resp( msg_hdr, msg_ptr );
            }
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            break;

        case SENSOR1_MSG_TYPE_IND:
            pthread_mutex_lock( &g_ar_control->data_mutex );
            if ( ( msg_hdr->service_number == CMC_SVC_NUM ) ) {
                hal_process_sam_cmc_ind( msg_hdr, msg_ptr );
            }
            else if ( msg_hdr->service_number == TILT_DETECTOR_SVC_NUM ) {
                hal_process_sam_tilt_detector_ind( msg_hdr, msg_ptr );
            }
            pthread_mutex_unlock( &g_ar_control->data_mutex );
            break;

        case SENSOR1_MSG_TYPE_BROKEN_PIPE:
            HAL_LOG_WARN("%s: SENSOR1_MSG_TYPE_BROKEN_PIPE", __FUNCTION__);
            pthread_mutex_lock( &g_ar_control->cb_mutex );
            hal_handle_broken_pipe();
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            break;

        case SENSOR1_MSG_TYPE_RETRY_OPEN:
            HAL_LOG_WARN("%s: SENSOR1_MSG_TYPE_RETRY_OPEN", __FUNCTION__);
            pthread_mutex_lock( &g_ar_control->cb_mutex );
            hal_reinit();
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            break;

        case SENSOR1_MSG_TYPE_REQ:
        default:
            HAL_LOG_ERROR("%s: Error - invalid msg type in cb: %u", __FUNCTION__, msg_type );
            break;
    }

    pthread_mutex_lock( &g_ar_control->cb_mutex );
    if( NULL != msg_ptr && g_ar_control->hndl ) {
        sensor1_free_msg_buf( g_ar_control->hndl, msg_ptr );
    }
    pthread_mutex_unlock( &g_ar_control->cb_mutex );
}

/*===========================================================================
  FUNCTION: hal_init
===========================================================================*/
static void hal_init( void ) __attribute__((constructor));
static void hal_init( void )
{
    pthread_mutexattr_t attr;
    hal_ar_ctl_t *ar_ctl;
    sensor1_error_e error;
    bool ret = false;

    /* Init sensor1 connection */
    sensor1_init();

    /* Enable logging */
    enableLogging();
    HAL_LOG_INFO("%s: Initializing AR HAL!", __FUNCTION__);

    ar_ctl = malloc(sizeof(*ar_ctl));
    if (ar_ctl == NULL)
    {
      HAL_LOG_ERROR("%s: ERROR: malloc error", __FUNCTION__ );
      return;
    }

    memset(ar_ctl, 0, sizeof(*ar_ctl));

    ar_ctl->cmc_enabled = false;
    ar_ctl->tilt_enabled = false;
    ar_ctl->time_service_enabled = false;

    ar_ctl->device.common.tag                 = HARDWARE_DEVICE_TAG;
    ar_ctl->device.common.version             = ACTIVITY_RECOGNITION_API_VERSION_0_1;
    ar_ctl->device.common.close               = hal_ar_close;
    ar_ctl->device.register_activity_callback = hal_ar_register_cb;
    ar_ctl->device.enable_activity_event      = hal_ar_enable_event;
    ar_ctl->device.disable_activity_event     = hal_ar_disable_event;
    ar_ctl->device.flush                      = hal_ar_flush;

    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(ar_ctl->cb_mutex), &attr );
    pthread_cond_init( &(ar_ctl->cb_arrived_cond), NULL );
    pthread_mutexattr_destroy(&attr);

    pthread_mutex_init( &(ar_ctl->data_mutex), NULL );
    pthread_cond_init( &(ar_ctl->data_arrived_cond), NULL );

    pthread_mutex_init( &(ar_ctl->acquire_resources_mutex), NULL );

    /* Time Service */
    hal_time_init();

    /* Open sensor1 connection */
    error = sensor1_open( &ar_ctl->hndl, &hal_sensor1_data_cb, (intptr_t)ar_ctl );
    if( SENSOR1_SUCCESS != error ) {
        HAL_LOG_ERROR("%s: Sensor1 open failed with error: %d %"PRIuPTR,
                       __FUNCTION__, error, (uintptr_t)ar_ctl->hndl);
        return;
    }

    g_ar_control = ar_ctl;
}

/*****************************************************************************/

/*===========================================================================
  FUNCTION: hal_ar_open
===========================================================================*/
/*!
  @brief
  Initialize the AR module

  HAL API - This function is called from AR's context

  @param module AR module
  @param name ACTIVITY_RECOGNITION_HARDWARE_INTERFACE
  @param device AR device

  @return 0 if successful, <0 if failed
 */
static int hal_ar_open(const struct hw_module_t *module, const char *name,
                       struct hw_device_t **device)
{
    int ret = -1;

    HAL_LOG_INFO("%s: name:%s", __FUNCTION__, name);

    if( !strcmp(name, ACTIVITY_RECOGNITION_HARDWARE_INTERFACE) ) {
        if( NULL == g_ar_control ) {
            HAL_LOG_ERROR("%s: HAL control init failed", __FUNCTION__ );
            return ret;
        }
        g_ar_control->device.common.module = (struct hw_module_t*) module;
        *device = &g_ar_control->device.common;
        ret = 0;
    }

    return ret;
}

/*===========================================================================
  FUNCTION:  hal_ar_close
===========================================================================*/
/*!
  @brief
  Closes the AR control device

  HAL API - This function is called from AR's context

  @param dev device
*/
static int hal_ar_close( struct hw_device_t *dev )
{
    hal_ar_ctl_t*  ar_ctl = (void*)dev;

    HAL_LOG_INFO("%s", __FUNCTION__);

    /* free up resources */
    if( ar_ctl != NULL ) {
        /* close sensor1 */
        sensor1_close( ar_ctl->hndl );

        /* clean up mutex and cond var  */
        pthread_mutex_destroy( &ar_ctl->cb_mutex );
        pthread_cond_destroy( &ar_ctl->cb_arrived_cond );
        pthread_mutex_destroy( &ar_ctl->data_mutex );
        pthread_cond_destroy( &ar_ctl->data_arrived_cond );

        /* Stop time service */
        hal_time_stop();

        /* free up memory */
        free(ar_ctl);
        g_ar_control = NULL;
    }
    return SENSOR1_SUCCESS;
}

/*===========================================================================
  FUNCTION:  hal_ar_register_cb
===========================================================================*/
static void hal_ar_register_cb(const struct activity_recognition_device* dev,
                               const activity_recognition_callback_procs_t* callback)
{
    hal_ar_ctl_t*   ar_ctl = (void*)dev;

    HAL_LOG_INFO("%s", __FUNCTION__);

    if( NULL == g_ar_control ) {
        HAL_LOG_ERROR("%s: HAL control init failed", __FUNCTION__ );
        return;
    }

    ar_ctl->hal_ar_cb = callback;
}

/*===========================================================================
  FUNCTION:  hal_ar_enable_event
===========================================================================*/
static int hal_ar_enable_event(const struct activity_recognition_device* dev,
                               uint32_t activity_handle, uint32_t event_type,
                               int64_t max_batch_report_latency_ns)
{
    int ret = -1;
    hal_ar_ctl_t* ar_ctl = (void*)dev;

    HAL_LOG_INFO("%s: activity_handle:%d event_type:%d max_batch_report_latency_ns:%"PRId64,
                  __FUNCTION__, activity_handle, event_type, max_batch_report_latency_ns);

    pthread_mutex_lock( &ar_ctl->cb_mutex );

    if(activity_handle > (MAX_NUM_ACTIVITIES - 1)) {
        HAL_LOG_ERROR("%s: Unknown activity! %d", __FUNCTION__, activity_handle);
        pthread_mutex_unlock( &ar_ctl->cb_mutex );
        return -1;
    }

    if(event_type == ACTIVITY_EVENT_ENTER) {
        g_ar_control->hal_track_activity[activity_handle].enter = true;
    }
    else if(event_type == ACTIVITY_EVENT_EXIT) {
        g_ar_control->hal_track_activity[activity_handle].exit = true;
    }
    else {
        HAL_LOG_ERROR("%s: Unknown event! %d", __FUNCTION__, event_type);
        pthread_mutex_unlock( &ar_ctl->cb_mutex );
        return -1;
    }

    g_ar_control->hal_track_activity[activity_handle].batch_rate = max_batch_report_latency_ns;
    /* Start time service if not enabled */
    if(g_ar_control->time_service_enabled != true) {
        hal_time_start();
        g_ar_control->time_service_enabled = true;
    }

    if( activity_handle == HANDLE_TILTING ) {
        if( !g_ar_control->tilt_enabled ) {
            ret = hal_sam_tilt_detector_send_enable_req(ar_ctl,
                                                        max_batch_report_latency_ns);
            if( !ret ) {
                HAL_LOG_ERROR("%s: TILT enable request failed", __FUNCTION__);
                pthread_mutex_unlock( &ar_ctl->cb_mutex );
                return -1;
            }
            g_ar_control->tilt_enabled = true;
        }
    }
    else {
        if( !g_ar_control->cmc_enabled ) {
            /* Send CMC enable Request */
            ret = hal_sam_cmc_send_enable_req( ar_ctl );
            if( !ret ) {
                HAL_LOG_ERROR("%s: CMC enable request failed", __FUNCTION__);
                pthread_mutex_unlock( &ar_ctl->cb_mutex );
                return -1;
            }
            /* Send CMC update reporting request to disable reporting of all events */
            ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, 0, 0, 0, false, false);
            if( !ret ) {
                HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                pthread_mutex_unlock( &ar_ctl->cb_mutex );
                return -1;
            }
            g_ar_control->cmc_enabled = true;
        }
        ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, activity_handle, event_type,
                                                    max_batch_report_latency_ns, true, true);
        if( !ret ) {
            HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
            pthread_mutex_unlock( &ar_ctl->cb_mutex );
            return -1;
        }
    }
    pthread_mutex_unlock( &ar_ctl->cb_mutex );

    return 0;
}

/*===========================================================================
  FUNCTION:  hal_ar_disable_event
===========================================================================*/
static int hal_ar_disable_event(const struct activity_recognition_device* dev,
                                uint32_t activity_handle, uint32_t event_type)
{
    int i;
    int ret = -1;
    hal_ar_ctl_t* ar_ctl = (void*)dev;

    HAL_LOG_INFO("%s: activity_handle:%d event_type:%d",
                  __FUNCTION__, activity_handle, event_type);

    pthread_mutex_lock( &ar_ctl->cb_mutex );

    if(activity_handle > (MAX_NUM_ACTIVITIES - 1)) {
        HAL_LOG_ERROR("%s: Unknown activity! %d", __FUNCTION__, activity_handle);
        pthread_mutex_unlock( &ar_ctl->cb_mutex );
        return -1;
    }

    if(event_type == ACTIVITY_EVENT_ENTER) {
        g_ar_control->hal_track_activity[activity_handle].enter = false;
    }
    else if(event_type == ACTIVITY_EVENT_EXIT) {
        g_ar_control->hal_track_activity[activity_handle].exit = false;
    }
    else {
        HAL_LOG_ERROR("%s: Unknown event! %d", __FUNCTION__, event_type);
        pthread_mutex_unlock( &ar_ctl->cb_mutex );
        return -1;
    }

    if( activity_handle == HANDLE_TILTING ) {
        if(g_ar_control->hal_track_activity[HANDLE_TILTING].enter == false &&
           g_ar_control->hal_track_activity[HANDLE_TILTING].exit == false) {
            g_ar_control->tilt_enabled = false;
            HAL_LOG_DEBUG("%s: Disabling TILT due to no events to be reported", __FUNCTION__);
            /* Disable Tilt algo is no events to be reported */
            hal_sam_send_cancel( ar_ctl->hndl, TILT_DETECTOR_SVC_NUM );
        }
        else {
            g_ar_control->tilt_enabled = true;
        }
    }
    else {
        for( i = 0; i < MAX_NUM_ACTIVITIES; i++ ) {
            if( i != HANDLE_TILTING ) {
                if( (g_ar_control->hal_track_activity[i].enter == false) &&
                    (g_ar_control->hal_track_activity[i].exit == false) ) {
                    g_ar_control->cmc_enabled = false;
                    continue;
                }
                else {
                    g_ar_control->cmc_enabled = true;
                    break;
                }
            }
        }
        if(g_ar_control->cmc_enabled == false) {
            /* Disable CMC algo is no events to be reported */
            HAL_LOG_DEBUG("%s: Disabling CMC due to no events to be reported", __FUNCTION__);
            hal_sam_send_cancel( ar_ctl->hndl, CMC_SVC_NUM );
        }
        else {
            ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, activity_handle,
                                                        event_type, 0, false, true);
        }
    }

    /* Stop time service if no events to be reported */
    if(g_ar_control->cmc_enabled == false && g_ar_control->tilt_enabled == false) {
        hal_time_stop();
        g_ar_control->time_service_enabled = false;
    }
    pthread_mutex_unlock( &ar_ctl->cb_mutex );

    return 0;
}

/*===========================================================================
  FUNCTION:  hal_ar_flush
===========================================================================*/
static int hal_ar_flush(const struct activity_recognition_device* dev)
{
    int i;
    int ret = -1;
    hal_ar_ctl_t*  ar_ctl = (void*)dev;

    HAL_LOG_INFO("%s", __FUNCTION__);

    pthread_mutex_lock( &g_ar_control->data_mutex );
    g_ar_control->flush_requested = true;
    pthread_mutex_unlock( &g_ar_control->data_mutex );

    pthread_mutex_lock( &g_ar_control->cb_mutex );
    if( g_ar_control->cmc_enabled == true ) {
        ret = hal_sam_cmc_send_enable_req( ar_ctl );
        if( !ret ) {
            HAL_LOG_ERROR("%s: CMC enable request failed", __FUNCTION__);
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            return -1;
        }
        ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, 0, 0, 0, false, false);
        if( !ret ) {
            HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            return -1;
        }
        for( i = 0; i < MAX_NUM_ACTIVITIES; i++ ) {
            if( i != HANDLE_TILTING ) {
                if(g_ar_control->hal_track_activity[i].enter == true) {
                    ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, i, ACTIVITY_EVENT_ENTER,
                                                  g_ar_control->hal_track_activity[i].batch_rate, true, true);
                    if( !ret ) {
                        HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                        pthread_mutex_unlock( &g_ar_control->cb_mutex );
                        return -1;
                    }
                }
                else if(g_ar_control->hal_track_activity[i].exit == true) {
                    ret = hal_sam_cmc_send_update_reporting_req(ar_ctl, i, ACTIVITY_EVENT_EXIT,
                                                  g_ar_control->hal_track_activity[i].batch_rate, true, true);
                    if( !ret ) {
                        HAL_LOG_ERROR("%s: CMC update reporting request failed", __FUNCTION__);
                        pthread_mutex_unlock( &g_ar_control->cb_mutex );
                        return -1;
                    }
                }
            }
        }
    }

    if( g_ar_control->tilt_enabled == true ) {
        ret = hal_sam_tilt_detector_send_enable_req( ar_ctl,
                  g_ar_control->hal_track_activity[HANDLE_TILTING].batch_rate );
        if( !ret ) {
            HAL_LOG_ERROR("%s: Tilt Detector enable request failed", __FUNCTION__);
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            return -1;
        }
    }
    pthread_mutex_unlock( &g_ar_control->cb_mutex );

    pthread_mutex_lock( &g_ar_control->data_mutex );
    hal_flush_send_cmplt();
    pthread_mutex_unlock( &g_ar_control->data_mutex );

    return 0;
}

/*===========================================================================
  FUNCTION: hal_ar_get_supported_activities_list
===========================================================================*/
static int hal_ar_get_supported_activities_list(struct activity_recognition_module* module,
                                                char const* const** activity_list)
{
    UNREFERENCED_PARAMETER(module);
    static int ar_list_len = 0;
    int i;
    int ret;

    HAL_LOG_INFO("%s", __FUNCTION__);

    if( g_ar_control == NULL ) {
        HAL_LOG_ERROR("%s: HAL control init failed", __FUNCTION__ );
        return 0;
    }

    pthread_mutex_lock( &g_ar_control->cb_mutex );

    if( ar_list_len != 0 ) {
        HAL_LOG_INFO("%s: Already have the list of activities", __FUNCTION__ );
        *activity_list = g_activity_list;
    }
    else {
        ret = hal_sam_cmc_send_algo_attrib_req();
        if( !ret ) {
            HAL_LOG_ERROR("%s: CMC attribute request failed", __FUNCTION__);
            pthread_mutex_unlock( &g_ar_control->cb_mutex );
            return ar_list_len;
        }
        if( g_ar_control->cmc_version == 2 ) {
            /* Hard coding in the initial implementation as SAM doesn't support */
            /* Need to switch to algo attrib req once SAM supports */
            *activity_list = g_activity_list;
            ar_list_len = ARRAY_SIZE(g_activity_list);
        }
    }

    pthread_mutex_unlock( &g_ar_control->cb_mutex );
    HAL_LOG_VERBOSE( "%s: Number of activities: %d ", __FUNCTION__, ar_list_len );
    for( i = 0; i < ar_list_len; i++ ) {
        HAL_LOG_VERBOSE( "%s: handle:%d activity:%s", __FUNCTION__, i, g_activity_list[i] );
    }

    return ar_list_len;
}

/*****************************************************************************/

static struct hw_module_methods_t ar_module_methods = {
	.open = hal_ar_open
};

activity_recognition_module_t HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.module_api_version = (uint16_t)ACTIVITY_RECOGNITION_API_VERSION_0_1,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.id = ACTIVITY_RECOGNITION_HARDWARE_MODULE_ID,
		.name = "QTI AR Module",
		.author = "Qualcomm Technologies, Inc.",
		.methods = &ar_module_methods,
		.dso = NULL,
		.reserved = {0},
	},
	.get_supported_activities_list = hal_ar_get_supported_activities_list
};
