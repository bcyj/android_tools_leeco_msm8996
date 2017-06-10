/*============================================================================
  @file GyroscopeUncalibrated.cpp

  @brief
  GyroscopeUncalibrated class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "GyroscopeUncalibrated.h"
#include "SensorsContext.h"

/*============================================================================
  GyroscopeUncalibrated Constructor
============================================================================*/
GyroscopeUncalibrated::GyroscopeUncalibrated(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_GYRO_UNCALIBRATED_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  GyroscopeUncalibrated Destructor
============================================================================*/
GyroscopeUncalibrated::~GyroscopeUncalibrated()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void GyroscopeUncalibrated::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* datatype)
{
    UNREFERENCED_PARAMETER(datatype);
    SensorsContext *context = SensorsContext::getInstance();
    Sensor *mSensor = context->getSensor(HANDLE_GYRO);
    if (mSensor != NULL) {
        setName(mSensor->getName());
        strlcat(name, " Uncalibrated", SNS_MAX_SENSOR_NAME_SIZE);
        setVendor(mSensor->getVendor());
        setVersion(mSensor->getVersion());
        setType(SENSOR_TYPE_GYROSCOPE_UNCALIBRATED);

        if(bWakeUp == false) {
            setFlags(SENSOR_FLAG_CONTINUOUS_MODE);
         } else {
            strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
            setFlags(SENSOR_FLAG_CONTINUOUS_MODE|SENSOR_FLAG_WAKE_UP);
         }
        setMaxRange(mSensor->getMaxRange());
        setResolution(mSensor->getResolution());
        setPower(mSensor->getPower());
        setMaxFreq(mSensor->getMaxFreq());
        setMinFreq(mSensor->getMinFreq());
        setAttribOK(mSensor->getAttribOK());
        setMaxBufferedSamples(mSensor->getMaxBufferedSamples());
    }
    else {
        HAL_LOG_ERROR("The mSensor handle %d is NULL!", HANDLE_GYRO);
    }
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void GyroscopeUncalibrated::processReportInd(
                        sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
                        sns_smgr_data_item_s_v01* smgr_data,
                        sensors_event_t &sensor_data)
{
    HAL_LOG_DEBUG("GyroscopeUncalibrated::processReportInd");
    if (smgr_ind->Item_len == 2) {
        float x_bias, y_bias, z_bias;
        sns_smgr_data_item_s_v01* factory_smgr_data;
        sns_smgr_data_item_s_v01* calibrated_smgr_data;
        if(smgr_ind->Item[0].ItemFlags & SNS_SMGR_ITEM_FLAG_AUTO_CAL_V01) {
            calibrated_smgr_data = &smgr_ind->Item[0];
            factory_smgr_data = &smgr_ind->Item[1];
        } else {
            factory_smgr_data = &smgr_ind->Item[0];
            calibrated_smgr_data = &smgr_ind->Item[1];
        }
        sensor_data.type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;

        if(bWakeUp == false) {
            sensor_data.sensor = HANDLE_GYRO_UNCALIBRATED;
            HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
        } else {
            sensor_data.sensor = HANDLE_GYRO_UNCALIBRATED_WAKE_UP;
            HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                            Utility::SensorTypeToSensorString(getType()));
        }

        /* Convert from SAE to Android co-ordinates and scale
        x' = y; y' = x; z' = -z;*/
        sensor_data.uncalibrated_gyro.x_uncalib =
            (float)(factory_smgr_data->ItemData[1]) * UNIT_CONVERT_GYRO;
        sensor_data.uncalibrated_gyro.y_uncalib =
            (float)(factory_smgr_data->ItemData[0]) * UNIT_CONVERT_GYRO;
        sensor_data.uncalibrated_gyro.z_uncalib =
            (float)(-factory_smgr_data->ItemData[2]) * UNIT_CONVERT_GYRO;

        x_bias = (float)(factory_smgr_data->ItemData[1] -
            calibrated_smgr_data->ItemData[1]) * UNIT_CONVERT_GYRO;
        y_bias = (float)(factory_smgr_data->ItemData[0] -
            calibrated_smgr_data->ItemData[0]) * UNIT_CONVERT_GYRO;
        z_bias = (float)-(factory_smgr_data->ItemData[2] -
            calibrated_smgr_data->ItemData[2]) * UNIT_CONVERT_GYRO;

        sensor_data.uncalibrated_gyro.x_bias = x_bias;
        sensor_data.uncalibrated_gyro.y_bias = y_bias;
        sensor_data.uncalibrated_gyro.z_bias = z_bias;

        HAL_LOG_VERBOSE( "%s: Uncal gyro x:%f y:%f z:%f xb:%f yb:%f zb:%f",
            __FUNCTION__,
            sensor_data.uncalibrated_gyro.x_uncalib,
            sensor_data.uncalibrated_gyro.y_uncalib,
            sensor_data.uncalibrated_gyro.z_uncalib,
            sensor_data.uncalibrated_gyro.x_bias,
            sensor_data.uncalibrated_gyro.y_bias,
            sensor_data.uncalibrated_gyro.z_bias );
    } else {
        HAL_LOG_ERROR("%s: Incorrect item len %d", __FUNCTION__,
            smgr_ind->Item_len);
    }

    /* accuracy .. is this good ?? */
    if (SNS_SMGR_ITEM_QUALITY_CURRENT_SAMPLE_V01 == smgr_data->ItemQuality) {
        ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_HIGH;
    } else {
        ((sensors_vec_t*)(sensor_data.data))->status = SENSOR_STATUS_ACCURACY_MEDIUM;
    }
    return;
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void GyroscopeUncalibrated::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    HAL_LOG_DEBUG("GyroscopeUncalibrated::prepareAddMsg");
    /* Request for both cal & uncal gyro in one report. Both are needed
    to report gyro bias offsets */
    (*buff_req)->Item_len = 2;
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_GYRO_V01;
    (*buff_req)->Item[0].Calibration = SNS_SMGR_CAL_SEL_FACTORY_CAL_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
    (*buff_req)->Item[1].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
    (*buff_req)->Item[1].SensorId = SNS_SMGR_ID_GYRO_V01;
    (*buff_req)->Item[1].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
    (*buff_req)->Item[1].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
