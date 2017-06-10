/*============================================================================
  @file CoarseMotionClassifier.cpp

  @brief
  CoarseMotionClassifier class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include "TimeSyncService.h"
#include "CoarseMotionClassifier.h"

/*============================================================================
  CoarseMotionClassifier Constructor
============================================================================*/
CoarseMotionClassifier::CoarseMotionClassifier(int handle)
    :SAMSensor(handle)
{
    if(getAttribOK() == true) {
        svc_num = SNS_SAM_CMC_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_EVENT;
        HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
        setName("Coarse Motion Classifier");
        setVendor("QTI");
        setType(SENSOR_TYPE_CMC);
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
        setMaxRange(SNS_SAM_CMC_MS_E_MAX_ENUM_VAL_V02); /* range of enum values */
        setResolution(1);

        /* Send Algo Attributes Request */
        sendAlgoAttribReq();
    }
}

/*============================================================================
  CoarseMotionClassifier Destructor
============================================================================*/
CoarseMotionClassifier::~CoarseMotionClassifier()
{

}

/*============================================================================
  enable
============================================================================*/
int CoarseMotionClassifier::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_cmc_enable_req_msg_v02 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("CMC is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                 sizeof(sns_sam_cmc_enable_req_msg_v02),
                                 (void**)&sam_req );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_CMC_ENABLE_REQ_V02;
        req_hdr.msg_size = sizeof(sns_sam_cmc_enable_req_msg_v02);
        req_hdr.txn_id = HANDLE_CMC;

        /* set default behavior for indications during suspend */
        sam_req->notify_suspend_valid = true;
        sam_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
        sam_req->notify_suspend.send_indications_during_suspend = true;

        /* Send Enable Request */
        err = sendEnableReq(&req_hdr, (void *)sam_req);
        if (err) {
            HAL_LOG_ERROR("send the SAM sensor Enable message failed!");
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        HAL_LOG_DEBUG( "%s: Received response: %d", __FUNCTION__, sensor1_cb->error );
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
void CoarseMotionClassifier::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
    bool                          error = false;

    HAL_LOG_INFO("%s: handle:%d %d", __FUNCTION__, handle, msg_hdr->msg_id);
    if (crsp_ptr->sns_result_t != 0 &&
        msg_hdr->msg_id != SNS_SAM_CMC_CANCEL_RESP_V02) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
        error = true;
    }

    if(true != error ) {
        switch (msg_hdr->msg_id) {
        case SNS_SAM_CMC_ENABLE_RESP_V02:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_ENABLE_RESP_V02", __FUNCTION__);
            instance_id = ((sns_sam_cmc_enable_resp_msg_v02 *)msg_ptr)->instance_id;
            break;
        case SNS_SAM_CMC_CANCEL_RESP_V02:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_CANCEL_RESP_V02", __FUNCTION__);
            /* Reset instance ID */
            instance_id = 0xFF;
            break;
        case SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02", __FUNCTION__);
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
void CoarseMotionClassifier::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    sns_sam_cmc_report_ind_msg_v02*  sam_cmc_rpt_ptr =
                                     (sns_sam_cmc_report_ind_msg_v02*) msg_ptr;
    bool                             error = false;
    uint32_t                         timestamp;
    sensors_event_t                  sensor_data;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    switch(msg_hdr->msg_id) {
        case SNS_SAM_CMC_REPORT_IND_V02:
            HAL_LOG_DEBUG("%s: SNS_SAM_CMC_REPORT_IND_V02", __FUNCTION__);
            sensor_data.type = SENSOR_TYPE_CMC;
            sensor_data.sensor = HANDLE_CMC;
            sensor_data.data[0] = sam_cmc_rpt_ptr->report_data.motion_state;
            sensor_data.data[1] = sam_cmc_rpt_ptr->report_data.motion_state_probability;
            HAL_LOG_VERBOSE("%s: sensor %d, motion state %f, probability %f", __FUNCTION__,
                            sensor_data.type, sensor_data.data[0], sensor_data.data[1]);
            timestamp = sam_cmc_rpt_ptr->timestamp;
            break;
        case SNS_SAM_CMC_ERROR_IND_V02:
            HAL_LOG_ERROR("%s: SNS_SAM_CMC_ERROR_IND_V02", __FUNCTION__);
            error = true;
            break;
        default:
            HAL_LOG_ERROR("%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
            error = true;
            break;
    }

    /* No error */
    if (error == false) {
        sensor_data.version = sizeof(sensors_event_t);
        sensor_data.timestamp = time_service->timestampCalc(
                (uint64_t)timestamp, sensor_data.sensor);

        HAL_LOG_VERBOSE("%s: CMC data[0]: %f data[1]: %f ts: %u", __FUNCTION__,
                         sensor_data.data[0], sensor_data.data[1], timestamp);
        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&sensor_data)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
}
