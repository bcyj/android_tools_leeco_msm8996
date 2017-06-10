/*============================================================================
  @file MagneticUncalibratedSAM.cpp

  @brief
  MagneticUncalibratedSAM class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "MagneticUncalibratedSAM.h"
#include "SensorsContext.h"

/*============================================================================
  MagneticUncalibratedSAM Constructor
============================================================================*/
MagneticUncalibratedSAM::MagneticUncalibratedSAM(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
}

/*============================================================================
  MagneticUncalibratedSAM Destructor
============================================================================*/
MagneticUncalibratedSAM::~MagneticUncalibratedSAM()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void MagneticUncalibratedSAM::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    UNREFERENCED_PARAMETER(sensor_datatype);
    SensorsContext *context = SensorsContext::getInstance();
    Sensor *mSensor = context->getSensor(HANDLE_MAGNETIC_FIELD);
    if (mSensor != NULL) {
        setName(mSensor->getName());
        strlcat(name, " Uncalibrated", SNS_MAX_SENSOR_NAME_SIZE);
        setVendor(mSensor->getVendor());
        setVersion(mSensor->getVersion());
        setType(SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED);
        if (bWakeUp == false) {
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
        setAttribOK(true);
        setMaxBufferedSamples(0);
        HAL_LOG_DEBUG("%s:, Added Uncal Mag from SAM", __FUNCTION__);
    }
    else {
        HAL_LOG_ERROR("The mSensor handle %d is NULL!", handle);
    }
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void MagneticUncalibratedSAM::processReportInd(
                        sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
                        sns_smgr_data_item_s_v01* smgr_data,
                        sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    HAL_LOG_DEBUG("MagneticUncalibratedSAM %s", __FUNCTION__);
    if (!(smgr_data->ItemFlags & SNS_SMGR_ITEM_FLAG_AUTO_CAL_V01)) {
        /* Convert from SAE to Android co-ordinates and scale
        x' = y; y' = x; z' = -z; */
        sensor_data.type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;

        if(bWakeUp == false) {
            sensor_data.sensor = HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM;
            HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
        } else {
            sensor_data.sensor = HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM_WAKE_UP;
            HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
        }
        mag_cal_cur_sample.sample.x_uncalib =
            (float)(smgr_data->ItemData[1]) * UNIT_CONVERT_MAGNETIC_FIELD;
        mag_cal_cur_sample.sample.y_uncalib =
            (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_MAGNETIC_FIELD;
        mag_cal_cur_sample.sample.z_uncalib =
            (float)(-smgr_data->ItemData[2]) * UNIT_CONVERT_MAGNETIC_FIELD;
        mag_cal_cur_sample.smgr_ts = smgr_data->TimeStamp;
        HAL_LOG_VERBOSE("%s: raw_X: %f raw_Y: %f raw_Z: %f", __FUNCTION__,
                        mag_cal_cur_sample.sample.x_uncalib,
                        mag_cal_cur_sample.sample.y_uncalib,
                        mag_cal_cur_sample.sample.z_uncalib );
    } else {
        HAL_LOG_ERROR("%s:Incorrect flag %d", __FUNCTION__, smgr_data->ItemFlags);
    }

    HAL_LOG_VERBOSE("%s: smgr_ts: %u sam_ts: %u apps_ts: %"PRId64, __FUNCTION__,
            mag_cal_cur_sample.smgr_ts,
            mag_cal_cur_sample.sam_ts,
            sensor_data.timestamp );
    sensor_data.uncalibrated_magnetic = mag_cal_cur_sample.sample;
    return;
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void MagneticUncalibratedSAM::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId    = SNS_SMGR_ID_MAG_V01;
    (*buff_req)->Item[0].Calibration = SNS_SMGR_CAL_SEL_FACTORY_CAL_V01;
    (*buff_req)->Item[0].Decimation  = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
}
