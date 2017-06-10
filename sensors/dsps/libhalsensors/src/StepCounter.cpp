/*============================================================================
  @file StepCounter.cpp

  @brief
  StepCounter class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "TimeSyncService.h"
#include "StepCounter.h"

/*============================================================================
  StepCounter Constructor
============================================================================*/
StepCounter::StepCounter(int handle)
    :SAMSensor(handle),
    step_counter_running_total(0),
    step_counter_running_instance(0),
    step_counter_current_instance(0),
    step_counter_last_timestamp(0)
{
    if(getAttribOK() == true) {
        svc_num = SNS_SAM_PED_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_EVENT;
        HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
        setName("Step Counter");
        setVendor("QTI");
        setType(SENSOR_TYPE_STEP_COUNTER);

        if(handle == HANDLE_SAM_STEP_COUNTER_WAKE_UP) {
            bWakeUp = true;
            setName("Step Counter -Wakeup");
            setFlags(SENSOR_FLAG_ON_CHANGE_MODE|SENSOR_FLAG_WAKE_UP);
        } else if(handle == HANDLE_SAM_STEP_COUNTER) {
            bWakeUp = false;
            setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
        }

        setMaxRange(1);
        setResolution(1);

        /* Send Algo Attributes Request */
        sendAlgoAttribReq();
    }
}

/*============================================================================
  StepCounter Destructor
============================================================================*/
StepCounter::~StepCounter()
{

}

/*===========================================================================
  setSensorInfo
===========================================================================*/
void StepCounter::setSensorInfo()
{
    HAL_LOG_INFO("%s: handle=%d", __FUNCTION__, handle);
    setMaxBufferedSamples(0);
}

/*============================================================================
  enable
============================================================================*/
int StepCounter::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_ped_enable_req_msg_v01 *sam_req;
    step_counter_current_instance = -1;

    if (enabled == en) {
        HAL_LOG_INFO("StepCounter is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d, freq=%f report_rate=%d batch_rate=%u \
                    batched=%d wakeup %d",__FUNCTION__, handle, freq,
                    report_rate, batch_rate, batching, bWakeUp);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                 sizeof(sns_sam_ped_enable_req_msg_v01),
                                 (void**)&sam_req );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s:sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_PED_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof(sns_sam_ped_enable_req_msg_v01);
        req_hdr.txn_id = 0;

        /* asynchronous reporting */
        sam_req->report_period = 0;

        /* set default behaviour for indications during suspend */
        sam_req->notify_suspend_valid = true;
        sam_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
        sam_req->notify_suspend.send_indications_during_suspend = bWakeUp;

        /* Send Enable Request */
        err = sendEnableReq(&req_hdr, (void *)sam_req);
        if (err) {
            HAL_LOG_ERROR("send the SAM sensor Enable message failed!");
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }
        HAL_LOG_DEBUG("%s: Received Response: %d",  __FUNCTION__, sensor1_cb->error);
        pthread_mutex_unlock(&sensor1_cb->cb_mutex);

        /* Get the last report data after enabling the sensor */
        err = sendGetReportReq();
        if (err) {
            HAL_LOG_ERROR("Get the last sensor data return error!");
        }
    } else {
        /* Disable sensor */
        HAL_LOG_DEBUG("%s: Disabling sensor handle=%d", __FUNCTION__, handle);
        sendCancel();
    };
    return 0;
}

/*===========================================================================
  FUNCTION:  processResp
===========================================================================*/
void StepCounter::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
    bool                          error = false;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    if (crsp_ptr->sns_result_t != 0) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
        error = true;

        if (msg_hdr->msg_id == SNS_SAM_PED_BATCH_RESP_V01 &&
            (crsp_ptr->sns_err_t == SENSOR1_EBAD_SVC_ID ||
            crsp_ptr->sns_err_t == SENSOR1_EUNKNOWN)) {
            /* Proceed if batching is not supported */
            error = false;
        }
    }

    if (SNS_SAM_PED_CANCEL_RESP_V01 == msg_hdr->msg_id ||
        SNS_SAM_PED_DISABLE_RESP_V01 == msg_hdr->msg_id) {
        /* Note, it's possible that we continue to receive/process indications after this.
        * However, they will update the running_instance, and should not be lost in
        * subsequent iterations */
        step_counter_running_total = step_counter_running_instance;
    }

    if(true != error ) {
        switch (msg_hdr->msg_id) {
        case SNS_SAM_PED_ENABLE_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_PED_ENABLE_RESP_V01", __FUNCTION__);
            instance_id = ((sns_sam_ped_enable_resp_msg_v01 *)msg_ptr)->instance_id;
            break;
        case SNS_SAM_PED_GET_REPORT_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_PED_GET_REPORT_RESP_V01", __FUNCTION__);
            processInd(msg_hdr, msg_ptr);
            break;
        case SNS_SAM_PED_CANCEL_RESP_V01:
        case SNS_SAM_PED_DISABLE_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_PED_CANCEL/DISABLE_RESP_V01", __FUNCTION__);
            /* Reset instance ID */
            instance_id = 0xFF;
            break;
        case SNS_SAM_PED_GET_ATTRIBUTES_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_PED_GET_ATTRIBUTES_RESP_V01", __FUNCTION__);
            processAlgoAttribResp(msg_hdr, msg_ptr);
            break;
        case SNS_SAM_PED_BATCH_RESP_V01:
            /* StepCounter is not allowed the batch */
            HAL_LOG_DEBUG("%s: Received SNS_SAM_PED_BATCH_RESP_V01", __FUNCTION__);
            break;
        default:
            HAL_LOG_ERROR("%s: Unknown msg id: %d", __FUNCTION__, msg_hdr->msg_id );
            return;
        }
    }

    if (msg_hdr->txn_id != TXN_ID_NO_RESP_SIGNALLED) {
            pthread_mutex_lock(&sensor1_cb->cb_mutex);
            Utility::signalResponse(error, sensor1_cb);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
    }
}

/*===========================================================================
  FUNCTION:  processInd
===========================================================================*/
void StepCounter::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    hal_sam_sample_t *sample_list = NULL;
    hal_sam_sample_t *curr_sample = NULL;
    sensors_event_t la_sample;
    uint32_t i = 0;
    uint32_t count = 0;
    uint64_t steps = 0;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    if (SNS_SAM_PED_REPORT_IND_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_PED_REPORT_IND_V01", __FUNCTION__);
        sns_sam_ped_report_ind_msg_v01* sam_ind =
            (sns_sam_ped_report_ind_msg_v01*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        } else {
            count = 1;
            sample_list->data[0] = sam_ind->report_data.step_count;
            sample_list->data[1] = sam_ind->report_data.step_rate;
            sample_list->data[2] = sam_ind->report_data.step_confidence;
            sample_list->data[3] = sam_ind->report_data.step_event;
            sample_list->data[4] = sam_ind->report_data.step_count_error;
            sample_list->accuracy = 0;
            sample_list->timestamp = sam_ind->timestamp;
        }
    }
    else if (SNS_SAM_PED_GET_REPORT_RESP_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_PED_GET_REPORT_RESP_V01", __FUNCTION__);
        sns_sam_ped_get_report_resp_msg_v01 *sam_ind =
            (sns_sam_ped_get_report_resp_msg_v01 *)msg_ptr;

        if (sam_ind->timestamp_valid && sam_ind->report_data_valid) {
            sample_list = (hal_sam_sample_t *)malloc( sizeof(hal_sam_sample_t) );
            if (NULL == sample_list) {
                HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
            } else {
                count = 1;
                sample_list->data[0] = sam_ind->report_data.step_count;
                sample_list->data[1] = sam_ind->report_data.step_rate;
                sample_list->data[2] = sam_ind->report_data.step_confidence;
                sample_list->data[3] = 0; /* We don't want to generate extra step detector events */
                sample_list->data[4] = sam_ind->report_data.step_count_error;
                sample_list->accuracy = 0;
                sample_list->timestamp = sam_ind->timestamp;
            }
        }
        else {
            HAL_LOG_WARN("%s: Received report with invalid data", __FUNCTION__);
        }
    }
    else if(SNS_SAM_PED_BATCH_IND_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_PED_BATCH_IND_V01", __FUNCTION__);
        sns_sam_ped_batch_ind_msg_v01* sam_ind =
            (sns_sam_ped_batch_ind_msg_v01*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sam_ind->items_len * sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        }
        else {
            curr_sample = sample_list;
            count = sam_ind->items_len;

            for(i = 0; i < sam_ind->items_len; i++) {
                curr_sample->data[0] = sam_ind->items[ i ].report.step_count;
                curr_sample->data[1] = sam_ind->items[ i ].report.step_rate;
                curr_sample->data[2] = sam_ind->items[ i ].report.step_confidence;
                curr_sample->data[3] = sam_ind->items[ i ].report.step_event;
                curr_sample->data[4] = sam_ind->items[ i ].report.step_count_error;
                curr_sample->accuracy = 0;
                curr_sample->timestamp = sam_ind->items[ i ].timestamp;
                curr_sample++;
            }
        }
    }
    else {
        HAL_LOG_ERROR("%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
    }

    if(count == 0) {
        pthread_mutex_lock(&data_cb->data_mutex);
        /* Release wakelock if held */
        if (data_cb->sensors_wakelock_held == true &&
            (getFlags() & SENSOR_FLAG_WAKE_UP)) {
            data_cb->sensors_wakelock_held = false;
            release_wake_lock( SENSORS_WAKE_LOCK );
            HAL_LOG_DEBUG("%s: released wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }

    for (i = 0; i < count; i++) {
        HAL_LOG_DEBUG("%s: handle %d, count=%d", __FUNCTION__, handle, count);
        curr_sample = &sample_list[i];

        /* As we update all sensors associated with an algo when SAM sends a response,
        * step counter needs a special handling to avoid spurious events generated
        * from step detector or pedometer when either of them are registered/de-registered.
        */
        if ((step_counter_running_total == 0) ||
            (step_counter_current_instance != sample_list->data[0])) {
            step_counter_current_instance = curr_sample->data[0];
            steps = step_counter_running_total + step_counter_current_instance;
            step_counter_running_instance = steps;
            la_sample.type = SENSOR_TYPE_STEP_COUNTER;

            if(bWakeUp == false) {
                la_sample.sensor = HANDLE_SAM_STEP_COUNTER;
                HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                            Utility::SensorTypeToSensorString(getType()));
            } else {
                la_sample.sensor = HANDLE_SAM_STEP_COUNTER_WAKE_UP;
                HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                                Utility::SensorTypeToSensorString(getType()));
            }
            la_sample.u64.step_counter = steps;
            la_sample.version = sizeof(sensors_event_t);
            if( step_counter_current_instance == 0) {
              /* Step count didn't change. Use the last saved timestamp. */
              la_sample.timestamp = step_counter_last_timestamp;
            } else {
              la_sample.timestamp = time_service->timestampCalc((uint64_t)curr_sample->timestamp, la_sample.sensor);
              step_counter_last_timestamp = la_sample.timestamp;
            }

            HAL_LOG_VERBOSE("%s: STEP COUNTER: step is %"PRIu64", running_total %"PRIu64"",
                            __FUNCTION__, steps, step_counter_running_total );

            pthread_mutex_lock(&data_cb->data_mutex);
            if (Utility::insertQueue(&la_sample)) {
                Utility::signalInd(data_cb);
            }
            pthread_mutex_unlock(&data_cb->data_mutex);
        }
    }
    free(sample_list);
}

/*===========================================================================
  FUNCTION:  sendGetReportReq
    Get the last report data after enabling the sensor.
===========================================================================*/
int StepCounter::sendGetReportReq()
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_ped_get_report_req_msg_v01 *sam_req;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                  sizeof(sns_sam_ped_get_report_req_msg_v01),
                                  (void**)&sam_req);
    if (SENSOR1_SUCCESS != error) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error );
        return -1;
    }
    req_hdr.service_number = svc_num;
    req_hdr.msg_id = SNS_SAM_PED_GET_REPORT_REQ_V01;
    req_hdr.msg_size = sizeof(sns_sam_ped_get_report_req_msg_v01);
    req_hdr.txn_id = TXN_ID_NO_RESP_SIGNALLED;
    sam_req->instance_id = instance_id;
    /* Send Request */
    sensor1_cb->error = false;

    error = sensor1_write(sensor1_cb->sensor1_handle, &req_hdr, sam_req);
    if (error != SENSOR1_SUCCESS) {
        /* free the message buffer */
        sensor1_free_msg_buf(sensor1_cb->sensor1_handle, sam_req);
        HAL_LOG_ERROR( "%s: sensor1_write() error: %d", __FUNCTION__, error );
        return -1;
    }
    return sensor1_cb->error ? -1 : 0;
}
