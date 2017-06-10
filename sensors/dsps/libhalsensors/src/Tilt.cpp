/*============================================================================
  @file Tilt.cpp

  @brief
  Tilt class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include "Tilt.h"
#include "TimeSyncService.h"

/*============================================================================
  Tilt Constructor
============================================================================*/
Tilt::Tilt(int handle)
    :SAMSensor(handle)
{
    if(getAttribOK() == true) {
        svc_num = SNS_SAM_INTEG_ANGLE_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_CONT;
        HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
        setName("Tilt");
        setVendor("QTI");
        setType(SENSOR_TYPE_TILT);
        setFlags(SENSOR_FLAG_CONTINUOUS_MODE);
        setMaxRange(180.0); /* unit in degree */
        setResolution(0.1); /* unit in degree */

        /* Send Algo Attributes Request */
        sendAlgoAttribReq();
    }
}

/*============================================================================
  Tilt Destructor
============================================================================*/
Tilt::~Tilt()
{

}

/*============================================================================
  enable
============================================================================*/
int Tilt::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_integ_angle_enable_req_msg_v01 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("TILT is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        HAL_LOG_DEBUG("%s: handle=%d, freq=%f report_rate=%d batched=%d",
                       __FUNCTION__, handle, freq, report_rate, batching);

        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                            sizeof(sns_sam_integ_angle_enable_req_msg_v01),
                            (void**)&sam_req);
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s:sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_INTEG_ANGLE_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof(sns_sam_integ_angle_enable_req_msg_v01);
        req_hdr.txn_id = 0;

        /* Set the sampling rate to the requested report rate */
        /* For tilt, the HAL is an intelligent HAL  - revisit */
        /* TODO: Normal & UI rates are broken with sample_rate.
                 If we remove sample_rate Game & Fastest would
                 be broken - maintaining status quo */
        sam_req->sample_rate_valid = true;
        sam_req->sample_rate       = (report_rate << 16);
        sam_req->report_period = (uint32_t)(UNIT_Q16/report_rate);

        /* Send Enable Request */
        err = sendEnableReq(&req_hdr, (void *)sam_req);
        if (err) {
            HAL_LOG_ERROR("send the SAM sensor Enable message failed!");
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        HAL_LOG_DEBUG("%s: Received response: %d", __FUNCTION__, sensor1_cb->error);
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
void Tilt::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    const sns_common_resp_s_v01* crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
    bool error = false;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    if (crsp_ptr->sns_result_t != 0 && msg_hdr->msg_id != SNS_SAM_INTEG_ANGLE_CANCEL_RESP_V01) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
        error = true;
    }
    if (true != error ) {
        switch (msg_hdr->msg_id) {
            case SNS_SAM_INTEG_ANGLE_ENABLE_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_INTEG_ANGLE_ENABLE_RESP_V01", __FUNCTION__);
                instance_id = ((sns_sam_integ_angle_enable_resp_msg_v01 *)msg_ptr)->instance_id;
                break;
            case SNS_SAM_INTEG_ANGLE_CANCEL_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_INTEG_ANGLE_CANCEL_RESP_V01", __FUNCTION__);
                /* Reset instance ID */
                instance_id = 0xFF;
                break;
            case SNS_SAM_INTEG_ANGLE_GET_ATTRIBUTES_RESP_V01:
                HAL_LOG_DEBUG("%s: Received SNS_SAM_INTEG_ANGLE_GET_ATTRIBUTES_RESP_V01", __FUNCTION__);
                processAlgoAttribResp(msg_hdr, msg_ptr);
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
void Tilt::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    sns_sam_integ_angle_report_ind_msg_v01 *rpt_ptr =
                            (sns_sam_integ_angle_report_ind_msg_v01 *)msg_ptr;
    bool error = false;
    uint32_t timestamp;
    sensors_event_t sensor_data;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    memset(&sensor_data, 0, sizeof(sensors_event_t));
    switch( msg_hdr->msg_id ) {
        case SNS_SAM_INTEG_ANGLE_REPORT_IND_V01:
            HAL_LOG_DEBUG("%s: SNS_SAM_INTEG_ANGLE_REPORT_IND_V01", __FUNCTION__);
            sensor_data.type = SENSOR_TYPE_TILT;
            sensor_data.sensor = HANDLE_GESTURE_TILT;
            /* Converting coordinate systems: QTI to Android */
            sensor_data.data[0] = rpt_ptr->angle[1] * RAD_Q16_TO_DEG_FLT;
            sensor_data.data[1] = rpt_ptr->angle[0] * RAD_Q16_TO_DEG_FLT;
            sensor_data.data[2] = -1 * rpt_ptr->angle[2] * RAD_Q16_TO_DEG_FLT;
            timestamp = rpt_ptr->timestamp;
            break;
        case SNS_SAM_INTEG_ANGLE_ERROR_IND_V01:
            HAL_LOG_ERROR("%s: SNS_SAM_INTEG_ANGLE_ERROR_IND_V01", __FUNCTION__);
            error = true;
            break;
        default:
            HAL_LOG_ERROR( "%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
            error = true;
            break;
    }

    /* No error */
    if (error == false) {
        sensor_data.version = sizeof(sensors_event_t);
        sensor_data.timestamp = time_service->timestampCalc((uint64_t)timestamp, sensor_data.sensor);
        HAL_LOG_VERBOSE("%s: TILT: %f, %f, %f, ts: %u", __FUNCTION__,
                         sensor_data.data[0], sensor_data.data[1],
                         sensor_data.data[0], timestamp);
        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&sensor_data)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
}
