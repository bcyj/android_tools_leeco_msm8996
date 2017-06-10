/*============================================================================
  @file RGB.cpp

  @brief
  RGB class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "RGB.h"

/*============================================================================
  RGB Constructor
============================================================================*/
RGB::RGB(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_EVENT;
}

/*============================================================================
  RGB Destructor
============================================================================*/
RGB::~RGB()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
    @info : sensor information to be reported to the framework
===========================================================================*/
void RGB::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: RGB DTy: %d", __FUNCTION__, sensor_datatype->DataType );
    setType(SENSOR_TYPE_RGB);
    setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
    setResolution((float)((float)sensor_datatype->Resolution * UNIT_CONVERT_Q16));
    setMaxRange((float)((float)sensor_datatype->MaxRange * UNIT_CONVERT_Q16));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void RGB::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
   UNREFERENCED_PARAMETER(smgr_data);
   if (smgr_ind->Item_len == 2) {
        sns_smgr_data_item_s_v01* rgb_data = &smgr_ind->Item[0];
        sns_smgr_data_item_s_v01* ct_c_data = &smgr_ind->Item[1];
        sensor_data.type = SENSOR_TYPE_RGB;
        sensor_data.sensor = HANDLE_RGB;
        /* RGB data - irradiance in microW/cm2 */
        sensor_data.data[0] = (float)(rgb_data->ItemData[0]) * UNIT_CONVERT_Q16;
        sensor_data.data[1] = (float)(rgb_data->ItemData[1]) * UNIT_CONVERT_Q16;
        sensor_data.data[2] = (float)(rgb_data->ItemData[2]) * UNIT_CONVERT_Q16;
        /* Color temp - in kelvin */
        sensor_data.data[3] = (float)(ct_c_data->ItemData[0])* UNIT_CONVERT_COLOR_TEMP;
        /* clear data - irradiance in microW/cm2 */
        sensor_data.data[4] = (float)(ct_c_data->ItemData[1]) * UNIT_CONVERT_Q16;
        HAL_LOG_VERBOSE("%s: R: %f, G: %f, B: %f, CT: %f, C: %f", __FUNCTION__, sensor_data.data[0],
            sensor_data.data[1], sensor_data.data[2], sensor_data.data[3],
            sensor_data.data[4] );
    } else {
    HAL_LOG_ERROR( "%s: Incorrect item len %d", __FUNCTION__,
            smgr_ind->Item_len );
    }
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void RGB::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item_len = 2;
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_RGB_V01;
    (*buff_req)->Item[0].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
    (*buff_req)->Item[1].SensorId = SNS_SMGR_ID_RGB_V01;
    (*buff_req)->Item[1].DataType = SNS_SMGR_DATA_TYPE_SECONDARY_V01;
    (*buff_req)->Item[1].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
    (*buff_req)->Item[1].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
