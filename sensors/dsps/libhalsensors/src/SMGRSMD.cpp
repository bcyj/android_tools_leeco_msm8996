/*============================================================================
  @file SMGRSMD.cpp

  @brief
  SMGRSMD class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "SMGRSMD.h"

/*============================================================================
  SMGRSMD Constructor
============================================================================*/
SMGRSMD::SMGRSMD(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_TRIG;
    bWakeUp = true;
}

/*============================================================================
  SMGRSMD Destructor
============================================================================*/
SMGRSMD::~SMGRSMD()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
    @info : sensor information to be reported to the framework
===========================================================================*/
void SMGRSMD::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: Sig Motion DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_SIGNIFICANT_MOTION);
    setFlags(SENSOR_FLAG_ONE_SHOT_MODE|SENSOR_FLAG_WAKE_UP);
    setResolution(1);
    setMaxRange(1);
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void SMGRSMD::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    UNREFERENCED_PARAMETER(smgr_data);
    sns_smgr_buffering_req_msg_v01* smgr_req;
    sensor1_error_e error;
    sensor1_msg_header_s  req_hdr;
    sensor_data.type = SENSOR_TYPE_SIGNIFICANT_MOTION;
    sensor_data.sensor = HANDLE_SMGR_SMD;
    sensor_data.data[0] = 1;
    HAL_LOG_DEBUG("%s: Sig Motion detected", __FUNCTION__);
    /* SMD is one-shot. Manually delete SMD report here */
    freq = 0;
    report_rate = 0;
    last_event.timestamp = 0;
    error = sensor1_alloc_msg_buf(smgr_sensor1_cb->sensor1_handle,
                sizeof(sns_smgr_buffering_req_msg_v01),
                (void**)&smgr_req);
    if (SENSOR1_SUCCESS != error) {
        HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf() failed: %u", __FUNCTION__, error );
        return;
    }
    /* Message header */
    req_hdr.service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
    req_hdr.msg_size = sizeof( sns_smgr_buffering_req_msg_v01 );
    /* Set txn_id to TXN_ID_NO_RESP_SIGNALLED so that a response is not signaled */
    req_hdr.txn_id = TXN_ID_NO_RESP_SIGNALLED;
    /* Message body */
    smgr_req->ReportId = HANDLE_SMGR_SMD;
    smgr_req->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
    /* Send Request */
    smgr_sensor1_cb->error = false;
    if ((error = sensor1_write(smgr_sensor1_cb->sensor1_handle, &req_hdr,
            smgr_req )) != SENSOR1_SUCCESS) {
        sensor1_free_msg_buf(smgr_sensor1_cb->sensor1_handle, smgr_req );
        HAL_LOG_ERROR("%s: sensor1_write() error: %u", __FUNCTION__, error);
    }
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void SMGRSMD::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_SMD_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
