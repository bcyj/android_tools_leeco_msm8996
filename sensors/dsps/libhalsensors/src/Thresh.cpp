/*============================================================================
  @file Thresh.cpp

  @brief
  Thresh class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include "TimeSyncService.h"
#include "Thresh.h"
#include "SensorsContext.h"

/*============================================================================
  Thresh Constructor
============================================================================*/
Thresh::Thresh(int handle)
    :SAMSensor(handle)
{
    if(getAttribOK() == true) {
        sensor1_error_e err;
        svc_num = SNS_SAM_SENSOR_THRESH_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_EVENT;
        HAL_LOG_INFO("%s: handle=%d", __FUNCTION__, handle);
        setType(SENSOR_TYPE_PROXIMITY);

        if(handle == HANDLE_PROXIMITY_NON_WAKE_UP) {
            setName("Proximity Non Wakeup");
            setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
            bWakeUp = false;
        } else if (handle == HANDLE_PROXIMITY) {
            setName("Proximity");
            setFlags(SENSOR_FLAG_ON_CHANGE_MODE|SENSOR_FLAG_WAKE_UP);
            bWakeUp = true;
        }
        setVendor("QTI");
        setMaxRange(5);
        setResolution(5);
        setVersion(1);
        setPower(0.1);
        setMaxFreq(0);
        setMinFreq(1);
        setMaxBufferedSamples(0);
    }
}

/*============================================================================
  Thresh Destructor
============================================================================*/
Thresh::~Thresh()
{

}

/*============================================================================
  enable
============================================================================*/
int Thresh::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_sensor_thresh_enable_req_msg_v01 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("THRESH is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                 sizeof(sns_sam_sensor_thresh_enable_req_msg_v01),
                                 (void**)&sam_req );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s:sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_SENSOR_THRESH_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof(sns_sam_sensor_thresh_enable_req_msg_v01);
        req_hdr.txn_id = 0;

        /* Report Request */
        sam_req->sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
        sam_req->data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
        sam_req->sample_rate = (5 << 16); /* ignore report_rate, use 5Hz */
        sam_req->threshold[0] = 0xFFFF;   /* threshold for binary states in Q16 */
        sam_req->threshold[1] = sam_req->threshold[2] = 0;  /* for completeness */

        /* set default behavior for indications during suspend */
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

        HAL_LOG_DEBUG("%s: Received response: %d", __FUNCTION__, sensor1_cb->error );
        pthread_mutex_unlock(&sensor1_cb->cb_mutex);
    } else {
        /* Disable sensor */
        HAL_LOG_DEBUG("%s: Disabling sensor handle=%d", __FUNCTION__, handle);
        sendCancel();
    }
    return 0;
}

/*===========================================================================
  FUNCTION:  processResp
===========================================================================*/
void Thresh::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
    bool                          error = false;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
#if 0
    if (crsp_ptr->sns_result_t != 0) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
        error = true;
    }
#endif

    if(true != error ) {
        switch (msg_hdr->msg_id) {
        case SNS_SAM_SENSOR_THRESH_ENABLE_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_SENSOR_THRESH_ENABLE_RESP_V01", __FUNCTION__);
            instance_id = ((sns_sam_sensor_thresh_enable_resp_msg_v01 *)msg_ptr)->instance_id;
            break;
        case SNS_SAM_SENSOR_THRESH_DISABLE_RESP_V01:
        case SNS_SAM_SENSOR_THRESH_CANCEL_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_SENSOR_THRESH_DISABLE/CANCEL_RESP_V01", __FUNCTION__);
            /* Reset instance ID */
            instance_id = 0xFF;
            break;
        default:
            HAL_LOG_ERROR("%s: Unknown msg id: %d", __FUNCTION__, msg_hdr->msg_id);
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
void Thresh::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    hal_sam_sample_t *sample_list = NULL;
    hal_sam_sample_t *curr_sample = NULL;
    sensors_event_t la_sample;
    uint32_t i = 0;
    uint32_t count = 0;
    SensorsContext *context = SensorsContext::getInstance();
    Sensor *mSensor = context->getSensor(HANDLE_PROXIMITY);

    if(bWakeUp == false) mSensor = context->getSensor(HANDLE_PROXIMITY_NON_WAKE_UP);
    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    if (SNS_SAM_SENSOR_THRESH_REPORT_IND_V01 == msg_hdr->msg_id) {
        HAL_LOG_DEBUG("%s: SNS_SAM_SENSOR_THRESH_REPORT_IND_V01", __FUNCTION__);
        sns_sam_sensor_thresh_report_ind_msg_v01* sam_ind =
            (sns_sam_sensor_thresh_report_ind_msg_v01*)msg_ptr;

        sample_list = (hal_sam_sample_t *)malloc(sizeof(hal_sam_sample_t));
        if (NULL == sample_list) {
            HAL_LOG_ERROR( "%s: Malloc error", __FUNCTION__ );
        } else {
            count = 1;
            sample_list->data[0] = sam_ind->sample_value[0];
            sample_list->data[1] = sam_ind->sample_value[1];
            sample_list->data[2] = sam_ind->sample_value[2];
            sample_list->timestamp = sam_ind->timestamp;
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
        la_sample.type = SENSOR_TYPE_PROXIMITY;

        if(bWakeUp == false) {
            la_sample.sensor = HANDLE_PROXIMITY_NON_WAKE_UP;
            HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
        } else {
            la_sample.sensor = HANDLE_PROXIMITY;
            HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                            Utility::SensorTypeToSensorString(getType()));
        }

#ifndef HAL_SUPPORT_DISTANCE
        if (0 == curr_sample->data[0] * UNIT_CONVERT_Q16) {
            /* far */
            if (mSensor != NULL)
                la_sample.distance = mSensor->getMaxRange();
            else
                la_sample.distance = (float)(curr_sample->data[1]) * UNIT_CONVERT_PROXIMITY;
        }
        else {
            /* near */
            la_sample.distance = 0;
        }
#else
        la_sample.distance = (float)(curr_sample->data[1]) * UNIT_CONVERT_PROXIMITY;
#endif

        la_sample.version = sizeof(sensors_event_t);
        la_sample.timestamp = time_service->timestampCalc(
                        (uint64_t)curr_sample->timestamp, la_sample.sensor);

        HAL_LOG_VERBOSE( "%s: prox data: %f %f %f", __FUNCTION__,
                         curr_sample->data[0], curr_sample->data[1],
                         la_sample.distance );

        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&la_sample)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
    free(sample_list);
}
