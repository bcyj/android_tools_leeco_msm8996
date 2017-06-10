/*============================================================================
  @file SMGRStepDetector.cpp

  @brief
  SMGRStepDetector class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "SMGRStepDetector.h"

/*============================================================================
  SMGRStepDetector Constructor
============================================================================*/
SMGRStepDetector::SMGRStepDetector(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_SPECIAL;
    (handle == HANDLE_SMGR_STEP_DETECTOR_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  SMGRStepDetector Destructor
============================================================================*/
SMGRStepDetector::~SMGRStepDetector()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
    @info : sensor information to be reported to the framework
===========================================================================*/
void SMGRStepDetector::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: Step Event DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_STEP_DETECTOR);

    if(bWakeUp == true) {
        strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
        setFlags(SENSOR_FLAG_SPECIAL_REPORTING_MODE|SENSOR_FLAG_WAKE_UP);
    }
    else {
        setFlags(SENSOR_FLAG_SPECIAL_REPORTING_MODE);
    }

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
void SMGRStepDetector::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    UNREFERENCED_PARAMETER(smgr_data);
    sensor_data.type = SENSOR_TYPE_STEP_DETECTOR;
    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_SMGR_STEP_DETECTOR;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_SMGR_STEP_DETECTOR_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
    }

    sensor_data.data[0] = 1;
    HAL_LOG_DEBUG("%s: Step detected", __FUNCTION__);
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void SMGRStepDetector::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_STEP_EVENT_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
