/*============================================================================
  @file Magnetic.cpp

  @brief
  Magnetic class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "Magnetic.h"

/*============================================================================
  Magnetic Constructor
============================================================================*/
Magnetic::Magnetic(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_MAGNETIC_FIELD_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  Magnetic Destructor
============================================================================*/
Magnetic::~Magnetic()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void Magnetic::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: MAG DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_MAGNETIC_FIELD);

    if(bWakeUp == false){
        setFlags(SENSOR_FLAG_CONTINUOUS_MODE);
    } else {
        setFlags(SENSOR_FLAG_CONTINUOUS_MODE|SENSOR_FLAG_WAKE_UP);
        strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
    }

    setResolution((float)((float)sensor_datatype->Resolution *
                        UNIT_CONVERT_MAGNETIC_FIELD));
    setMaxRange((float)((float)sensor_datatype->MaxRange *
                        UNIT_CONVERT_MAGNETIC_FIELD));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void Magnetic::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    uint8_t accuracy = 0x06; // bits 1,2 of Item Flag used for accuracy
    UNREFERENCED_PARAMETER(smgr_ind);
    sensor_data.type = SENSOR_TYPE_MAGNETIC_FIELD;
    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_MAGNETIC_FIELD;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_MAGNETIC_FIELD_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
    }

    /* Convert from SAE to Android co-ordinates and scale
    x' = y; y' = x; z' = -z; */
    sensor_data.magnetic.x =
                (float)(smgr_data->ItemData[1]) * UNIT_CONVERT_MAGNETIC_FIELD;
    sensor_data.magnetic.y =
                (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_MAGNETIC_FIELD;
    sensor_data.magnetic.z =
                (float)(-smgr_data->ItemData[2]) * UNIT_CONVERT_MAGNETIC_FIELD;
    HAL_LOG_VERBOSE( "%s: Calibrated Mag, %f, %f, %f",
                __FUNCTION__,
                sensor_data.magnetic.x,
                sensor_data.magnetic.y,
                sensor_data.magnetic.z);

    accuracy &= smgr_data->ItemFlags;
    accuracy >>= 1;
    ((sensors_vec_t*)(sensor_data.data))->status = accuracy;
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void Magnetic::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_MAG_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
