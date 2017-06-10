/*============================================================================
  @file Humidity.cpp

  @brief
  Humidity class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "Humidity.h"

/*============================================================================
  Humidity Constructor
============================================================================*/
Humidity::Humidity(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_EVENT;
    (handle == HANDLE_RELATIVE_HUMIDITY_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  Humidity Destructor
============================================================================*/
Humidity::~Humidity()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void Humidity::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: HUMIDITY DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_RELATIVE_HUMIDITY);
    if(bWakeUp == false) {
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
    } else {
        strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE|SENSOR_FLAG_WAKE_UP);
    }

    setResolution((float)((float)sensor_datatype->Resolution *
            UNIT_CONVERT_RELATIVE_HUMIDITY));
    setMaxRange((float)((float)sensor_datatype->MaxRange *
            UNIT_CONVERT_RELATIVE_HUMIDITY));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void Humidity::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    sensor_data.type = SENSOR_TYPE_RELATIVE_HUMIDITY;
    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_RELATIVE_HUMIDITY;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_RELATIVE_HUMIDITY_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
    }

    sensor_data.relative_humidity =
    (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_RELATIVE_HUMIDITY;
    HAL_LOG_DEBUG("%s: x: P: %f", __FUNCTION__,
                sensor_data.relative_humidity);
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void Humidity::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
   (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_HUMIDITY_V01;
}
