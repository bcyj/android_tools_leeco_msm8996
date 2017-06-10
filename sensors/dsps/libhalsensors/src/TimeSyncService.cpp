/*============================================================================
  @file TimeSyncService.cpp

  @brief
  TimeSyncService class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>
#include <utils/SystemClock.h>

#include "TimeSyncService.h"
#include "Utility.h"

TimeSyncService* TimeSyncService::self = NULL;

/*===========================================================================
                   PREPROCESSOR DEFINTIONS
===========================================================================*/
#define MAX_CLOCK_DIFF_NS 10000

/*============================================================================
  FUNCTION getTimeSyncService
    Get the TimeSyncService static object;
============================================================================*/
TimeSyncService* TimeSyncService::getTimeSyncService()
{
    if (NULL == self)
        self = new TimeSyncService;
    return self;
}
/*============================================================================
  FUNCTION getCb
    Get the TimeSyncService sensor1 handle;
============================================================================*/
sensor1_handle_s* TimeSyncService::getCb()
{
    return sensor1_handle;
}

/*============================================================================
  TimeSyncService Constructor
============================================================================*/
TimeSyncService::TimeSyncService()
    :enabled(0),
    sensor1_handle(NULL),
    timestamp_offset_apps_dsps(0),
    dsps_ts_last(0),
    dsps_rollover_cnt(0),
    dsps_rollover_cnt_rcv(0),
    boot_ts_last_rollover(0)
{
    int i;
    pthread_mutexattr_t attr;
    sensor1_error_e error;

    for(i = 0; i < MAX_NUM_SENSORS; i++) {
        timestamp_last_sent[i] = 0;
    }
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&time_mutex, &attr);
    pthread_cond_init(&time_cond, NULL);
    pthread_mutexattr_destroy(&attr);
    /* open the sensor1 connection for time_service */
    error = sensor1_open(&sensor1_handle, &time_service_sensor1_cb, (intptr_t)this);
    if(SENSOR1_SUCCESS != error) {
        HAL_LOG_ERROR("%s:sensor1 open failed for time_service!", __FUNCTION__);
    }
    HAL_LOG_VERBOSE("Sensor1 opened for time_service");
}

/*============================================================================
  TimeSyncService Destructor
============================================================================*/
TimeSyncService::~TimeSyncService()
{
    HAL_LOG_INFO("%s: closing time_service sensor1...", __FUNCTION__);
    sensor1_close(sensor1_handle);
    pthread_mutex_destroy(&time_mutex);
    pthread_cond_destroy(&time_cond);
}

/*===========================================================================
  FUNCTION:  timeServiceStart
    Start the time_service service
===========================================================================*/
int TimeSyncService::timeServiceStart()
{
    sensor1_msg_header_s msgHdr;
    sns_time_timestamp_req_msg_v02 *msg_ptr = NULL;
    int err = -1;
    sensor1_error_e error;

    pthread_mutex_lock(&time_mutex);
    /* if the time_service is not started, start the time_service */
    if (0 == enabled) {
        error = sensor1_alloc_msg_buf(sensor1_handle,
                sizeof(sns_time_timestamp_req_msg_v02), (void**)&msg_ptr );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf returned(get) %d",
                       __FUNCTION__, error);
            err = -1;
        }
        else {
            msgHdr.service_number = SNS_TIME2_SVC_ID_V01;
            msgHdr.msg_id = SNS_TIME_TIMESTAMP_REQ_V02;
            msgHdr.msg_size = sizeof(sns_time_timestamp_req_msg_v02);
            msgHdr.txn_id = 1;
            msg_ptr->reg_report_valid = true;
            msg_ptr->reg_report = true;

            error = sensor1_write(sensor1_handle, &msgHdr, msg_ptr);
            if (SENSOR1_SUCCESS != error) {
                HAL_LOG_ERROR("%s: sensor1_write returned %d", __FUNCTION__, error);
                sensor1_free_msg_buf(sensor1_handle, msg_ptr);
                err = -1;
            }
            else if (false == Utility::waitForResponse(TIME_OUT_MS,
                                                    &time_mutex,
                                                    &time_cond,
                                                    &is_resp_arrived)) {
                HAL_LOG_ERROR("%s: ERROR: No response from request %d",
                        __FUNCTION__, SNS_TIME_TIMESTAMP_REQ_V02);
                err = -1;
            }
        }
    }
    enabled = 1;
    pthread_mutex_unlock(&time_mutex);
    return err;
}

/*===========================================================================
  FUNCTION:  timeServiceStop
    Stop the time_service service
===========================================================================*/
int TimeSyncService::timeServiceStop()
{
    sensor1_msg_header_s msgHdr;
    sns_time_timestamp_req_msg_v02 *msg_ptr = NULL;
    int err = -1;
    sensor1_error_e error;

    pthread_mutex_lock(&time_mutex);
    /* if the time_service is started, stop the time_service */
    if (1 == enabled) {
        error = sensor1_alloc_msg_buf(sensor1_handle,
                sizeof(sns_time_timestamp_req_msg_v02), (void**)&msg_ptr );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf returned(get) %d",
                       __FUNCTION__, error);
            err = -1;
        }
        else {
            HAL_LOG_DEBUG("send stop time_service command");
            msgHdr.service_number = SNS_TIME2_SVC_ID_V01;
            msgHdr.msg_id = SNS_TIME_CANCEL_REQ_V02;
            msgHdr.msg_size = sizeof(sns_time_timestamp_req_msg_v02);
            msgHdr.txn_id = 1;
            msg_ptr->reg_report_valid = true;
            msg_ptr->reg_report = true;

            error = sensor1_write(sensor1_handle, &msgHdr, msg_ptr);
            if (SENSOR1_SUCCESS != error) {
                HAL_LOG_ERROR("%s: sensor1_write returned %d", __FUNCTION__, error);
                sensor1_free_msg_buf(sensor1_handle, msg_ptr);
                err = -1;
            }
            else if (false == Utility::waitForResponse(TIME_OUT_MS,
                                                    &time_mutex,
                                                    &time_cond,
                                                    &is_resp_arrived)) {
                HAL_LOG_ERROR("%s: ERROR: No response from request %d",
                        __FUNCTION__, SNS_TIME_CANCEL_REQ_V02);
                err = -1;
            }
        }
    }
    enabled = 0;
    pthread_mutex_unlock(&time_mutex);
    return err;
}

/*===========================================================================
  FUNCTION:  getTimeSyncServiceStatus
    Get the time_service status
    Return valuses
        1 : time_service is started
        0 : time_service is stopped
===========================================================================*/
int TimeSyncService::getTimeSyncServiceStatus()
{
    return enabled;
}

/*===========================================================================
  FUNCTION:  processTimeResp
    Process response message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
void TimeSyncService::processTimeResp(sensor1_msg_header_s const *msg_hdr,
        sns_time_timestamp_resp_msg_v02 const *msg_ptr)
{
    UNREFERENCED_PARAMETER(msg_hdr);
    pthread_mutex_lock(&time_mutex);
    if (0 == msg_ptr->resp.sns_result_t) {
        if(true == msg_ptr->dsps_rollover_cnt_valid) {
            rolloverCntIs(msg_ptr->dsps_rollover_cnt);
        }
        if (true == msg_ptr->timestamp_dsps_valid &&
            true == msg_ptr->timestamp_apps_boottime_valid) {
                tsOffsetIs(msg_ptr->timestamp_dsps, msg_ptr->timestamp_apps_boottime);
        }
        else if(true == msg_ptr->error_code_valid) {
            HAL_LOG_ERROR("%s: Error in RESP: %i", __FUNCTION__, msg_ptr->error_code);
        }
        else {
            HAL_LOG_ERROR("%s: Unknown error in RESP. DSPS ts valid: %i; APPS: %i APPS boottime: %i",
                     __FUNCTION__, msg_ptr->timestamp_dsps_valid, msg_ptr->timestamp_apps_valid,
                     msg_ptr->timestamp_apps_boottime_valid);
        }
    }
    else {
        HAL_LOG_ERROR("%s: Received 'Failed' in response result", __FUNCTION__);
    }

    is_resp_arrived = true;
    pthread_cond_signal(&time_cond);
    pthread_mutex_unlock(&time_mutex);
    return;
}

/*===========================================================================
  FUNCTION:  processCancerResp
    Process Cancer response to the time_service service.
===========================================================================*/
void TimeSyncService::processCancerResp()
{
        pthread_mutex_lock(&time_mutex);
        is_resp_arrived = true;
        pthread_cond_signal(&time_cond);
        pthread_mutex_unlock(&time_mutex);
}

/*===========================================================================
  FUNCTION:  processTimeInd
    Process indication message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
void TimeSyncService::processTimeInd(sensor1_msg_header_s const *msg_hdr,
    sns_time_timestamp_ind_msg_v02 const *msg_ptr)
{
    UNREFERENCED_PARAMETER(msg_hdr);
    rolloverCntIs(msg_ptr->dsps_rollover_cnt);
    if (msg_ptr->timestamp_apps_boottime_valid) {
        tsOffsetIs(msg_ptr->timestamp_dsps, msg_ptr->timestamp_apps_boottime);
    }
}

/*===========================================================================
  FUNCTION:  rolloverCntIs
    Update the rollover count
    parameters
        @rollover_cnt : the rollover count in the ssc side
===========================================================================*/
void TimeSyncService::rolloverCntIs(uint32_t rollover_cnt)
{
    pthread_mutex_lock(&time_mutex);
    if (0 == dsps_rollover_cnt && 0 == dsps_ts_last) {
        dsps_rollover_cnt = rollover_cnt;
    }
    dsps_rollover_cnt_rcv = rollover_cnt;
    pthread_mutex_unlock(&time_mutex);
}

/*===========================================================================
  FUNCTION:  tsOffsetIs
    Update local time stamp offset state.
    parameters
        @timestamp_ssc : the time stamp in the SSC side
        @timestamp_ap: the times tamp in the AS side
===========================================================================*/
void TimeSyncService::tsOffsetIs(uint32_t timestamp_ssc, uint64_t timestamp_ap)
{
    uint64_t ssc_ns;
    uint64_t elapsed_realtime_ns;
    struct timespec ts_start, ts_end;
    uint64_t boottime_ns, boottime_start, boottime_end;

    /* Figure out the difference between the CLOCK_BOOTTIME time and
     * elapsedRealtimeNano */
    do {
        clock_gettime( CLOCK_BOOTTIME, &ts_start );
        elapsed_realtime_ns = android::elapsedRealtimeNano();
        clock_gettime( CLOCK_BOOTTIME, &ts_end );
        boottime_start = ts_start.tv_sec * NSEC_PER_SEC + ts_start.tv_nsec;
        boottime_end = ts_end.tv_sec * NSEC_PER_SEC + ts_end.tv_nsec;
        HAL_LOG_DEBUG("%s: boottime_end - boottime_start: %llu", __FUNCTION__, boottime_end - boottime_start);
    } while (boottime_end - boottime_start > MAX_CLOCK_DIFF_NS);

    boottime_ns = (boottime_start + boottime_end) / 2;

    ssc_ns = (uint64_t)(((uint64_t)timestamp_ssc * NSEC_PER_SEC) / DSPS_HZ);
    timestamp_offset_apps_dsps = timestamp_ap - ssc_ns - boottime_ns + elapsed_realtime_ns;

    HAL_LOG_DEBUG("%s: Apps: %"PRIu64"; DSPS: %u; Offset : %"PRId64,
                __FUNCTION__,  timestamp_ap, timestamp_ssc, timestamp_offset_apps_dsps);
}

/*===========================================================================
  FUNCTION:  timestampCalc
    Converts the DSPS clock ticks from a sensor sample to a LA timestamp (ns
    since epoch).  Adjusts return value based on dsps timestamp rollover
    and makes minor adjustments to ensure sensor samples are sent with
    ascending timestamps.
    Parameters
        @dsps_timestamp : the time stamp in the sensor event
        @sensor_handle : one type of sensor handle
    Return value
        int64_t : the calculated timestamp
===========================================================================*/
int64_t TimeSyncService::timestampCalc(uint64_t dsps_timestamp, int sensor_handle)
{
    int32_t rollover_diff;
    struct timespec current_time;
    int64_t current_boot_ts;
    int time_err;
    int64_t rv = timestamp_offset_apps_dsps +
                ((dsps_timestamp * NSEC_PER_SEC) / DSPS_HZ);

    pthread_mutex_lock(&time_mutex);

    if ((dsps_timestamp < TS_ROLLOVER_THRESH) &&
            (UINT32_MAX - dsps_ts_last < TS_ROLLOVER_THRESH)) {
        /* If a roll-over is predicted, check the boottime timestamp of the last
        * predicted roll-over against the current boottime timestamp. If the
        * difference between the two times is not greater than
        * BOOT_TS_ROLLOVER_THRESH, then there most likely was just jitter in the
        * incoming DSPS timestamps instead of a real clock roll-over event. */
        time_err = clock_gettime(CLOCK_BOOTTIME, &current_time);
        if (0 != time_err) {
            time_err = errno;
            HAL_LOG_ERROR("%s: Error with clock_gettime: %i", __FUNCTION__, time_err);
        }
        else {
            current_boot_ts = ((int64_t)current_time.tv_sec * 1000000000) +
                                                        current_time.tv_nsec;
            HAL_LOG_WARN("%s: potential TS rollover detected. \
                        DSPS TS: %"PRIu64", last DSPS: %u, boot \
                        TS: %"PRIi64", last boot: %"PRIi64"",
                        __FUNCTION__, dsps_timestamp, dsps_ts_last,
                        current_boot_ts, boot_ts_last_rollover);

            if ((current_boot_ts - boot_ts_last_rollover) > BOOT_TS_ROLLOVER_THRESH) {
                /* If a roll-over has likely occurred */
                dsps_rollover_cnt++;
                /* Record the boottime timestamp */
                boot_ts_last_rollover = current_boot_ts;

                HAL_LOG_WARN( "%s: TS rollover confirmed. cnt: %u, rcv: %u",
                        __FUNCTION__, dsps_rollover_cnt,
                        dsps_rollover_cnt_rcv );
            }
        }
    }

    /* If the # of rollovers determined by the HAL is different than in the
    * last message received from the time service, adjust the timestamp accordingly */
    rollover_diff = dsps_rollover_cnt - dsps_rollover_cnt_rcv;
    if ((0 < rollover_diff && dsps_timestamp < TS_ROLLOVER_THRESH) ||
        (0 > rollover_diff && dsps_timestamp > UINT32_MAX - TS_ROLLOVER_THRESH)) {
        HAL_LOG_WARN("%s: Adjusting timestamp for rollover: %"PRIu64", %i",
            __FUNCTION__, rv, rollover_diff );
        rv += (rollover_diff * UINT32_MAX) * NSEC_PER_SEC / DSPS_HZ;
    }

    /* Ensure sensor samples are sent to LA with increasing timestamps */
    if (timestamp_last_sent[sensor_handle] > rv &&
        abs(timestamp_last_sent[sensor_handle] - rv) < TS_CORRECT_THRESH) {
        HAL_LOG_WARN("%s: Adjusting timestamp to maintain ascension: %"PRIu64", %"PRIu64,
            __FUNCTION__, rv, timestamp_last_sent[sensor_handle]);
        rv = timestamp_last_sent[sensor_handle] + 1;
    }

    dsps_ts_last = dsps_timestamp;
    timestamp_last_sent[sensor_handle] = rv;
    pthread_mutex_unlock(&time_mutex);

    return rv;
}

/*===========================================================================
  FUNCTION:  time_service_sensor1_cb
    Parameters
        @cb_data : callback data pointer
        @msg_hdr : sensor1 message header
        @msg_type : sensor1 message type
        @msg_ptr : sensor1 message pointer
===========================================================================*/
void time_service_sensor1_cb(intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
        sensor1_msg_type_e msg_type, void *msg_ptr)
{
    TimeSyncService *time_service = (TimeSyncService *)cb_data;
    sensor1_handle_s  *sensor1_handle = time_service->getCb();

    HAL_LOG_DEBUG("%s: msg_type %d", __FUNCTION__, msg_type );
    if (msg_hdr != NULL) {
        HAL_LOG_DEBUG("%s: Sn %d, msg Id %d, txn Id %d", __FUNCTION__,
            msg_hdr->service_number, msg_hdr->msg_id, msg_hdr->txn_id);
    }
    else {
        HAL_LOG_DEBUG( "%s: Ignoring message (type %u)", __FUNCTION__, msg_type );
        return ;
    }

    if (SENSOR1_MSG_TYPE_RESP == msg_type &&
        SNS_TIME_TIMESTAMP_RESP_V02 == msg_hdr->msg_id) {
        time_service->processTimeResp(msg_hdr, (sns_time_timestamp_resp_msg_v02*)msg_ptr);
    }
    else if (SENSOR1_MSG_TYPE_RESP == msg_type &&
        SNS_TIME_CANCEL_RESP_V02 == msg_hdr->msg_id) {
        time_service->processCancerResp();
    }
    else if (SENSOR1_MSG_TYPE_IND == msg_type) {
        time_service->processTimeInd(msg_hdr, (sns_time_timestamp_ind_msg_v02*)msg_ptr);
    }
    else {
        HAL_LOG_WARN("%s: Received unknown message type %i, id %i",
                    __FUNCTION__, msg_type, msg_hdr->msg_id);
    }
    /* free the callback message buf */
    if (NULL != msg_ptr && sensor1_handle) {
        sensor1_free_msg_buf(sensor1_handle, msg_ptr);
    }
    return;
}
