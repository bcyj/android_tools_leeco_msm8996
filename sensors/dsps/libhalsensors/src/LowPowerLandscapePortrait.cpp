/*============================================================================
  @file LowPowerLandscapePortrait.cpp

  @brief
  LowPowerLandscapePortrait class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <cutils/properties.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include "TimeSyncService.h"
#include "LowPowerLandscapePortrait.h"
#include "sns_smgr_api_v01.h"
#include "sns_sam_rmd_v01.h"
#include "sns_sam_qmd_v01.h"

/*============================================================================
  LowPowerLandscapePortrait Constructor
============================================================================*/
LowPowerLandscapePortrait::LowPowerLandscapePortrait(int handle)
    :SAMSensor(handle)
{
    if(getAttribOK() == true) {
        svc_num = SNS_SAM_EVENT_GATED_SENSOR_SVC_ID_V01;
        trigger_mode = SENSOR_MODE_CONT;
        HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
        setName("Motion Accel");
        setVendor("QTI");
        setType(SENSOR_TYPE_SCREEN_ORIENTATION);
        setFlags(SENSOR_FLAG_CONTINUOUS_MODE);
        setMaxRange(1);
        setResolution(1);

        /* Send Algo Attributes Request */
        sendAlgoAttribReq();
    }
}

/*============================================================================
  LowPowerLandscapePortrait Destructor
============================================================================*/
LowPowerLandscapePortrait::~LowPowerLandscapePortrait()
{

}

/*============================================================================
  enable
============================================================================*/
int LowPowerLandscapePortrait::enable(int en)
{
    int err;
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_sam_event_gated_sensor_enable_req_msg_v01 *sam_req;

    if (enabled == en) {
        HAL_LOG_INFO("LP2/MA is already enabled/disabled %d", enabled);
        return 0;
    }

    /* store the en value */
    enabled = en;
    HAL_LOG_DEBUG("%s: handle=%d", __FUNCTION__, handle);

    if (en) {
        pthread_mutex_lock(&sensor1_cb->cb_mutex);
        error = sensor1_alloc_msg_buf(sensor1_cb->sensor1_handle,
                                 sizeof(sns_sam_event_gated_sensor_enable_req_msg_v01),
                                 (void**)&sam_req );
        if (SENSOR1_SUCCESS != error) {
            HAL_LOG_ERROR("%s:sensor1_alloc_msg_buf error:%d", __FUNCTION__, error);
            pthread_mutex_unlock(&sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }

        req_hdr.service_number = svc_num;
        req_hdr.msg_id = SNS_SAM_EVENT_GATED_SENSOR_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof(sns_sam_event_gated_sensor_enable_req_msg_v01);
        req_hdr.txn_id = 0;

        /* Event Sensor SUID for LP2/MA */
        sam_req->event_suid = SNS_SAM_RMD_SUID_V01;
        /* Gating condition for LP2/MA */
        sam_req->gating_condition = SNS_SAM_MOTION_REST_V01;

        sam_req->streaming_item_len = 1;
        sam_req->streaming_item[0].SensorId = SNS_SMGR_ID_ACCEL_V01;
        sam_req->streaming_item[0].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
        sam_req->streaming_item[0].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;
        sam_req->streaming_item[0].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
        sam_req->streaming_item[0].SamplingRate = MOTION_ACCEL_SAMPLE_RATE; /* 15hz */
        sam_req->streaming_item[1].SamplingRate = MOTION_ACCEL_SAMPLE_RATE; /* 15hz */

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
void LowPowerLandscapePortrait::processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
    bool                          error = false;

    HAL_LOG_DEBUG("%s: handle:%d %d", __FUNCTION__, handle, msg_hdr->msg_id);
    if (crsp_ptr->sns_result_t != 0 &&
        msg_hdr->msg_id != SNS_SAM_EVENT_GATED_SENSOR_CANCEL_RESP_V01) {
        HAL_LOG_ERROR("%s: Msg %i; Result: %u, Error: %u", __FUNCTION__,
                    msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
        error = true;
    }

    if(true != error ) {
        switch (msg_hdr->msg_id) {
        case SNS_SAM_EVENT_GATED_SENSOR_ENABLE_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_EVENT_GATED_SENSOR_ENABLE_RESP_V01", __FUNCTION__);
            instance_id = ((sns_sam_event_gated_sensor_enable_resp_msg_v01 *)msg_ptr)->instance_id;
            break;
        case SNS_SAM_EVENT_GATED_SENSOR_CANCEL_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_EVENT_GATED_SENSOR_CANCEL_RESP_V01", __FUNCTION__);
            /* Reset instance ID */
            instance_id = 0xFF;
            break;
        case SNS_SAM_EVENT_GATED_SENSOR_GET_ATTRIBUTES_RESP_V01:
            HAL_LOG_DEBUG("%s: Received SNS_SAM_EVENT_GATED_SENSOR_GET_ATTRIBUTES_RESP_V01", __FUNCTION__);
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
void LowPowerLandscapePortrait::processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    sns_sam_event_gated_sensor_ind_msg_v01*  sam_lp2_rpt_ptr =
                                     (sns_sam_event_gated_sensor_ind_msg_v01*) msg_ptr;
    bool                             error = false;
    sensors_event_t                  sensor_data;
    uint32_t i,j;
    /* This array keeps track of the current index to the item requested
     * in the buffering request */
    uint32_t ts_offset[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01] = {0,};
    uint_fast16_t max_reports_per_index = 0;
    sns_smgr_periodic_report_ind_msg_v01 report_msg;
    sns_smgr_data_item_s_v01* smgr_data;

    HAL_LOG_INFO("%s: handle:%d", __FUNCTION__, handle);
    memset(&sensor_data, 0, sizeof(sensors_event_t));
    switch(msg_hdr->msg_id) {
      case SNS_SAM_EVENT_GATED_SENSOR_REPORT_IND_V01:
          HAL_LOG_DEBUG("%s: SNS_SAM_EVENT_GATED_SENSOR_REPORT_IND_V01", __FUNCTION__);

        for (i = 0; i < sam_lp2_rpt_ptr->Indices_len; i++) {
            max_reports_per_index = MAX(max_reports_per_index,
                    sam_lp2_rpt_ptr->Indices[i].SampleCount);
        }
        HAL_LOG_DEBUG("%s: Samples_len=%d Items=%d max_reports_per_index=%d",
            __FUNCTION__, sam_lp2_rpt_ptr->Samples_len, sam_lp2_rpt_ptr->Indices_len,
            max_reports_per_index);

        for (i = 0; i < max_reports_per_index; i++) {
          report_msg.ReportId = sam_lp2_rpt_ptr->ReportId;
          report_msg.status = SNS_SMGR_REPORT_OK_V01;
          /* The CurrentRate isn't used for much -- just a debug printout.
          * This conversion may be wrong if there's more than one sampling rate,
          * but the only side effect is an incorrect debug print */
          report_msg.CurrentRate = sam_lp2_rpt_ptr->Indices[0].SamplingRate;
          report_msg.Item_len = 0;
          for (j = 0; j < sam_lp2_rpt_ptr->Indices_len; j++) {
            if (i < sam_lp2_rpt_ptr->Indices[j].SampleCount) {
              report_msg.Item_len++;
              report_msg.Item[j].SensorId = sam_lp2_rpt_ptr->Indices[j].SensorId;
              report_msg.Item[j].DataType = sam_lp2_rpt_ptr->Indices[j].DataType;
              report_msg.Item[j].ItemData[0] = sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].Data[0];
              report_msg.Item[j].ItemData[1] = sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].Data[1];
              report_msg.Item[j].ItemData[2] = sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].Data[2];
              ts_offset[j] += sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].TimeStampOffset;
              report_msg.Item[j].TimeStamp = (sam_lp2_rpt_ptr->Indices[j].FirstSampleTimestamp +
                              ts_offset[j]);
              report_msg.Item[j].ItemFlags = sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].Flags;
              report_msg.Item[j].ItemQuality = sam_lp2_rpt_ptr->Samples[sam_lp2_rpt_ptr->Indices[j].FirstSampleIdx+i].Quality;
              report_msg.Item[j].ItemSensitivity = 0;
            }
          }

        /* the HAL requests one item per report */
        smgr_data = &report_msg.Item[0];

        if (smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01) {
            HAL_LOG_DEBUG( "%s: Received invalid/not ready sample for sensor ID %i",
                __FUNCTION__, smgr_data->SensorId);
            return;
        }
        if (smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 ||
            smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 ||
            smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01) {
            HAL_LOG_ERROR("%s: Bad item quality: %u ", __FUNCTION__, smgr_data->ItemQuality);
            return;
        }

                sensor_data.type = SENSOR_TYPE_SCREEN_ORIENTATION;
                sensor_data.sensor = HANDLE_MOTION_ACCEL;
                sensor_data.version = sizeof(sensors_event_t);
                sensor_data.timestamp = time_service->timestampCalc((uint64_t)smgr_data->TimeStamp,
                                                                    sensor_data.sensor);

        /* Convert from SAE to Android co-ordinates and scale
        x' = y; y' = x; z' = -z; */
        sensor_data.acceleration.x = (float)(smgr_data->ItemData[1]) *
                                                UNIT_CONVERT_ACCELERATION;
        sensor_data.acceleration.y = (float)(smgr_data->ItemData[0]) *
                                                UNIT_CONVERT_ACCELERATION;
        sensor_data.acceleration.z = (float)(-smgr_data->ItemData[2]) *
                                                UNIT_CONVERT_ACCELERATION;
        HAL_LOG_VERBOSE("%s: X: %f Y: %f Z: %f ", __FUNCTION__,
                    sensor_data.acceleration.x,
                    sensor_data.acceleration.y,
                    sensor_data.acceleration.z);

        /* accuracy .. is this good ?? */
        if (SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE_V01 == smgr_data->ItemQuality) {
            ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_HIGH;
        } else {
            ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_MEDIUM;
        }
      }

        break;
      case SNS_SAM_EVENT_GATED_SENSOR_ERROR_IND_V01:
          HAL_LOG_ERROR("%s: SNS_SAM_EVENT_GATED_SENSOR_ERROR_IND_V01", __FUNCTION__);
          error = true;
          break;
      default:
          HAL_LOG_ERROR("%s: Unknown message ID = %d", __FUNCTION__, msg_hdr->msg_id);
          error = true;
          break;
    }

    /* No error */
    if (error == false) {
        pthread_mutex_lock(&data_cb->data_mutex);
        if (Utility::insertQueue(&sensor_data)) {
            Utility::signalInd(data_cb);
        }
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
}
