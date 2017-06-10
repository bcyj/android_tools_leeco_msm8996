/*============================================================================
  @file Gyroscope.cpp

  @brief
  Gyroscope class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "Gyroscope.h"

/*============================================================================
  Gyroscope Constructor
============================================================================*/
Gyroscope::Gyroscope(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_GYRO_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  Gyroscope Destructor
============================================================================*/
Gyroscope::~Gyroscope()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void Gyroscope::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* datatype)
{
    HAL_LOG_DEBUG("%s: GYRO DTy: %d", __FUNCTION__, datatype->DataType);
    setType(SENSOR_TYPE_GYROSCOPE);

    if(bWakeUp == false) {
        setFlags(SENSOR_FLAG_CONTINUOUS_MODE);
    }  else {
       setFlags(SENSOR_FLAG_CONTINUOUS_MODE|SENSOR_FLAG_WAKE_UP);
       strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
    }

    setResolution((float)((float)datatype->Resolution *
                        UNIT_CONVERT_GYRO));
    setMaxRange((float)((float)datatype->MaxRange *
                        UNIT_CONVERT_GYRO));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void Gyroscope::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);

    sensor_data.type = SENSOR_TYPE_GYROSCOPE;
    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_GYRO;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_GYRO_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    }

    /* Convert from SAE to Android co-ordinates and scale
    x' = y; y' = x; z' = -z; */
    sensor_data.gyro.x = (float)(smgr_data->ItemData[1]) * UNIT_CONVERT_GYRO;
    sensor_data.gyro.y = (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_GYRO;
    sensor_data.gyro.z = (float)(-smgr_data->ItemData[2]) * UNIT_CONVERT_GYRO;

    HAL_LOG_VERBOSE("%s: X: %f Y: %f Z: %f ", __FUNCTION__,
            sensor_data.gyro.x,
            sensor_data.gyro.y,
            sensor_data.gyro.z);
    /* accuracy .. is this good ?? */
    if (SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE_V01 == smgr_data->ItemQuality) {
        ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_HIGH;
    } else {
        ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_MEDIUM;
    }
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void Gyroscope::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_GYRO_V01;
}
