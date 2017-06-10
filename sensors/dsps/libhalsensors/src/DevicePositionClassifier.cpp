/*============================================================================
  @file DevicePositionClassifier.cpp

  @brief
  DevicePositionClassifier class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "TimeSyncService.h"
#include "DevicePositionClassifier.h"
#include "SensorsContext.h"

/*============================================================================
  DevicePositionClassifier Constructor
============================================================================*/
DevicePositionClassifier::DevicePositionClassifier(int handle)
    :SAMSensor(handle)
{
    svc_num = SNS_SAM_DPC_SVC_ID_V01;
    trigger_mode = SENSOR_MODE_TRIG;
    bWakeUp = true;
    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);

    SensorsContext *context = SensorsContext::getInstance();
    Sensor *mSensor = context->getSensor(HANDLE_ACCELERATION);

    setName("Pick Up Gesture");
    setVendor("QTI");
    setType(SENSOR_TYPE_PICK_UP_GESTURE);
    setFlags(SENSOR_FLAG_ONE_SHOT_MODE|SENSOR_FLAG_WAKE_UP);
    setMaxRange(1);
    setResolution(1);
    setAttribOK(true);
    setVersion(1);
    setMaxFreq(1);
    setMinFreq(0);
    setMaxBufferedSamples(0);

    if (mSensor != NULL) {
        setPower(mSensor->getPower());
    }
}

/*============================================================================
  DevicePositionClassifier Destructor
============================================================================*/
DevicePositionClassifier::~DevicePositionClassifier()
{

}

/*============================================================================
  enable
============================================================================*/
int DevicePositionClassifier::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_dpc_enable_req_msg_v01 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("DPC is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                 sizeof(sns_sam_dpc_enable_req_msg_v01),
                                 (void**)&sam_req );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s:sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_DPC_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof(sns_sam_dpc_enable_req_msg_v01);
        req_hdr.txn_id = 0;

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
  FUNCTION:   processResp
===========================================================================*/
void DevicePositionClassifier::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    sns_sam_dpc_enable_resp_msg_v01* crsp_ptr = (sns_sam_dpc_enable_resp_msg_v01*) msg_ptr;
    bool                          error = false;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    if (crsp_ptr->resp.sns_result_t != 0 &&
        msg_hdr->msg_id != SNS_SAM_DPC_CANCEL_RESP_V01) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->resp.sns_result_t, crsp_ptr->resp.sns_err_t);

        /* Disregard DPC failures when algo is already disabled; avoid delete-retry cycles */
        if(SNS_SAM_DPC_DISABLE_RESP_V01 == msg_hdr->msg_id &&
            SENSOR1_ENOTALLOWED == crsp_ptr->resp.sns_err_t)
        {
            error = false;
        }
    }

    if(true != error ) {
        switch (msg_hdr->msg_id) {
        case SNS_SAM_DPC_ENABLE_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_DPC_ENABLE_RESP_V01", __FUNCTION__);
            instance_id = ((sns_sam_dpc_enable_resp_msg_v01 *)msg_ptr)->instance_id;
            break;
        case SNS_SAM_DPC_DISABLE_RESP_V01:
        case SNS_SAM_DPC_CANCEL_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_DPC_DISABLE/CANCEL_RESP_V01", __FUNCTION__);
            /* Reset instance ID */
            instance_id = 0xFF;
            if( msg_hdr->txn_id == TXN_ID_NO_RESP_SIGNALLED )
            {
              /* This disable response is because DPC was "auto-disabled" due to receiving
               * an DPC indication. Don't signal a response here, since the HAL isn't
               * expecting one for this case */
              HAL_LOG_VERBOSE("%s: DPC disable response. DPC auto-disabled due to indication",
                              __FUNCTION__ );
              return;
            }
            HAL_LOG_VERBOSE("%s: DPC disable response. DPC disabled due to HAL command",
                            __FUNCTION__ );
            break;
        case SNS_SAM_DPC_GET_ATTRIBUTES_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_DPC_GET_ATTRIBUTES_RESP_V01", __FUNCTION__);
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
  FUNCTION:   processInd
===========================================================================*/
void DevicePositionClassifier::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    sns_sam_dpc_report_ind_msg_v01*  sam_dpc_rpt_ptr =
                                     (sns_sam_dpc_report_ind_msg_v01*) msg_ptr;
    bool                             error = false;
    uint32_t                         timestamp = 0;
    sensors_event_t                  sensor_data;

    HAL_LOG_DEBUG("%s: DPC", __FUNCTION__);
    switch( msg_hdr->msg_id ) {
        case SNS_SAM_DPC_REPORT_IND_V01:
            HAL_LOG_DEBUG("%s: SNS_SAM_DPC_REPORT_IND_V01", __FUNCTION__);
            sensor_data.type = SENSOR_TYPE_PICK_UP_GESTURE;
            sensor_data.sensor = HANDLE_DPC;
            if (sam_dpc_rpt_ptr->report_data.dpc_state == 0)
            {
                sensor_data.data[0] = 1;
                HAL_LOG_VERBOSE("%s: sensor %d, dpc state %d", __FUNCTION__,
                                sensor_data.type, sensor_data.data[0]);
                timestamp = sam_dpc_rpt_ptr->timestamp;
            }
            else
            {
                error = true;
                HAL_LOG_DEBUG("%s: Invalid dpc state %d",__FUNCTION__,
                               sam_dpc_rpt_ptr->report_data.dpc_state);
            }
            break;
        case SNS_SAM_DPC_ERROR_IND_V01:
            HAL_LOG_ERROR("%s: SNS_SAM_DPC_ERROR_IND_V01", __FUNCTION__);
            error = true;
            break;
        default:
            HAL_LOG_ERROR("%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
            error = true;
            break;
    }

    /* No error */
    if (error == false) {
        /* Deactivate DPC since it's an one-shot sensor */
        deactivateDpc();
        sensor_data.version = sizeof(sensors_event_t);
        sensor_data.timestamp = time_service->timestampCalc(
                (uint64_t)timestamp, sensor_data.sensor);

        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&sensor_data)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
    else {
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
}

void DevicePositionClassifier::deactivateDpc()
{
    sensor1_error_e       error;
    sensor1_msg_header_s  req_hdr;
    sns_sam_dpc_disable_req_msg_v01*  sam_req;

    HAL_LOG_DEBUG("%s", __FUNCTION__ );
    pthread_mutex_lock( &sensor1_cb->cb_mutex );
    /* Disable DPC */
    enabled = 0;
    /* send request to disable algo */
    error = sensor1_alloc_msg_buf( sensor1_cb->sensor1_handle,
                                   sizeof(sns_sam_dpc_disable_req_msg_v01),
                                   (void**)&sam_req );
    if(SENSOR1_SUCCESS == error) {
        /* Message header */
        req_hdr.service_number = SNS_SAM_DPC_SVC_ID_V01;
        req_hdr.msg_id = SNS_SAM_DPC_DISABLE_REQ_V01;
        req_hdr.msg_size = sizeof( sns_sam_dpc_disable_req_msg_v01 );
        req_hdr.txn_id = TXN_ID_NO_RESP_SIGNALLED;
        sam_req->instance_id = instance_id;

        HAL_LOG_VERBOSE("%s: Sending DPC disable request. Instance ID %d",
                    __FUNCTION__, sam_req->instance_id );
        /* Send Request */
        sensor1_cb->error = false;
        if( (error = sensor1_write( sensor1_cb->sensor1_handle, &req_hdr,
                                    sam_req )) != SENSOR1_SUCCESS ) {
            sensor1_free_msg_buf( sensor1_cb->sensor1_handle, sam_req );
            HAL_LOG_ERROR("%s: sensor1_write() error: %u", __FUNCTION__, error );
        }

        /* waiting for response - 200ms */
        if (Utility::waitForResponse(TIME_OUT_MS_DPC_IND,
                                   &sensor1_cb->cb_mutex,
                                   &sensor1_cb->cb_cond,
                                   &sensor1_cb->is_resp_arrived) == false) {
            HAL_LOG_ERROR( "%s: ERROR: No response from the request", __FUNCTION__ );
        }
    }
    else {
        HAL_LOG_ERROR("%s: failed to allocated disable msg", __FUNCTION__);
    }
    pthread_mutex_unlock( &sensor1_cb->cb_mutex );
}
