/*============================================================================
  @file Pedometer.cpp

  @brief
  Pedometer class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "TimeSyncService.h"
#include "Pedometer.h"

/*============================================================================
  Pedometer Constructor
============================================================================*/
Pedometer::Pedometer(int handle)
    :SAMSensor(handle)
{
    if(getAttribOK() == true) {
        svc_num = SNS_SAM_PED_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_EVENT;
        HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
        setName("Pedometer");
        setVendor("QTI");
        setType(SENSOR_TYPE_PEDOMETER);
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
        setMaxRange(1);
        setResolution(1);

        /* Send Algo Attributes Request */
        sendAlgoAttribReq();
    }
}

/*============================================================================
  Pedometer Destructor
============================================================================*/
Pedometer::~Pedometer()
{

}

/*============================================================================
  enable
============================================================================*/
int Pedometer::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_ped_enable_req_msg_v01 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("Pedometer is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

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

        /* set default behavior for indications during suspend */
        sam_req->notify_suspend_valid = true;
        sam_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
        sam_req->notify_suspend.send_indications_during_suspend = false;

        /* Send Enable Request */
        err = sendEnableReq(&req_hdr, (void *)sam_req);
        if (err) {
            HAL_LOG_ERROR("send the SAM sensor Enable message failed!");
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        HAL_LOG_DEBUG("%s: Received Response:%d", __FUNCTION__, sensor1_cb->error);
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
void Pedometer::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
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
void Pedometer::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    hal_sam_sample_t *sample_list = NULL;
    hal_sam_sample_t *curr_sample = NULL;
    sensors_event_t la_sample;
    uint32_t i = 0;
    uint32_t count = 0;

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
    else {
        HAL_LOG_ERROR("%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
    }

    for (i = 0; i < count; i++) {
        HAL_LOG_DEBUG("%s: handle %d, count=%d", __FUNCTION__, handle, count);
        curr_sample = &sample_list[i];

        la_sample.type = SENSOR_TYPE_PEDOMETER;
        la_sample.sensor = HANDLE_PEDOMETER;

        la_sample.data[0] = curr_sample->data[0];
        la_sample.data[1] = curr_sample->data[1];
        la_sample.data[2] = curr_sample->data[2];

        la_sample.version = sizeof(sensors_event_t);
        la_sample.timestamp = time_service->timestampCalc(
                (uint64_t)curr_sample->timestamp, la_sample.sensor);

        HAL_LOG_VERBOSE("%s: PEDOMETER: step count %f",
                        __FUNCTION__, la_sample.data[0]);

        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&la_sample)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
    free(sample_list);
}
