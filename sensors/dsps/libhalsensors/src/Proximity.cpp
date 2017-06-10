/*============================================================================
  @file Proximity.cpp

  @brief
  Proximity class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "Proximity.h"

/*============================================================================
  Proximity Constructor
============================================================================*/
Proximity::Proximity(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_EVENT;
    (handle == HANDLE_PROXIMITY)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  Proximity Destructor
============================================================================*/
Proximity::~Proximity()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void Proximity::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: Prox DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_PROXIMITY);
    if(bWakeUp == false) {
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
        strncat(name," -Non Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
    }
    else {
        setFlags(SENSOR_FLAG_ON_CHANGE_MODE|SENSOR_FLAG_WAKE_UP);
    }
    setResolution((float)((float)sensor_datatype->Resolution *
                                                        UNIT_CONVERT_PROXIMITY));
    setMaxRange((float)((float)sensor_datatype->MaxRange *
                                                        UNIT_CONVERT_PROXIMITY));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void Proximity::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    uint8_t temp;
    sensor_data.type = SENSOR_TYPE_PROXIMITY;
    if(bWakeUp == false) {
        HAL_LOG_VERBOSE("%s:(Prox non wake_up)",__FUNCTION__);
        sensor_data.sensor = HANDLE_PROXIMITY_NON_WAKE_UP;
    } else {
        HAL_LOG_VERBOSE("%s:(Prox wake_up)",__FUNCTION__);
        sensor_data.sensor = HANDLE_PROXIMITY;
    }
    /* The proximity driver does not support distance yet but reports near/far */
    temp = smgr_data->ItemData[0] * UNIT_CONVERT_Q16;
    if (0 == temp) {
        /* far */
        sensor_data.distance = max_range;
    }
    else {
        /* near */
        sensor_data.distance = 0;
    }
    HAL_LOG_VERBOSE("%s: prox data: %x %x %f", __FUNCTION__,
                    smgr_data->ItemData[0],
                    smgr_data->ItemData[1],
                    sensor_data.distance);
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void Proximity::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_PROX_LIGHT_V01;
}
