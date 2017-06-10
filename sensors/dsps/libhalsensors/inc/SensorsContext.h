/*============================================================================
  @file SensorsContext.h

  @brief
  SensorsContext class definition.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef SENSORS_CONTEXT_H
#define SENSORS_CONTEXT_H

#include "sensors_hal.h"
#include "sns_smgr_api_v01.h"

typedef enum {
    HAL_MAG_CAL_SRC_UNKNOWN,
    HAL_MAG_CAL_SRC_SAM,
    HAL_MAG_CAL_SRC_SMGR,
    HAL_MAG_CAL_SRC_NONE
} hal_mag_cal_src_e;

class Sensor;
class TimeSyncService;
class OEMLib;

/*===========================================================================
                    CLASSES SensorsContext
===========================================================================*/
class SensorsContext {
public:
    /* must be first */
    sensors_poll_device_1_t device;
    ~SensorsContext();
    int activate(int handle, int en);
    int poll(sensors_event_t *data, int count);
    int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
    int flush(int handle);
/*===========================================================================
  FUNCTION:  getInstance
    Get the singleton SensorContext object.
    Return value
        @SensorsContext* : The SensorContext object.
===========================================================================*/
    static SensorsContext* getInstance();
/*===========================================================================
  FUNCTION:  getSensors
    Get all sensors objects.
    Return value
        @Sensor** : Return the mSensors data.
===========================================================================*/
    Sensor **getSensors();
/*===========================================================================
  FUNCTION:  getSensor
    Get the sensor handle object.
    Parameters
        @handle : The sensor handle.
    Return value
        @Sensor* : Return the mSensor[handle] data.
===========================================================================*/
    Sensor *getSensor(int handle);
/*===========================================================================
  FUNCTION:  processResp
    Process the response to the sensor1 SENSOR1_MSG_TYPE_RESP
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    void processResp(Sensor** msensors, sensor1_msg_header_s *msg_hdr, void *msg_ptr);
/*===========================================================================
  FUNCTION:  processAllSensorInfoResp
    Process the response to the sensor1 SNS_SMGR_ALL_SENSOR_INFO_RESP_V01
    Parameters
        @smgr_info_resp : sensor1 sensor info response message
===========================================================================*/
    void processAllSensorInfoResp(sns_smgr_all_sensor_info_resp_msg_v01* smgr_info_resp);
/*===========================================================================
  FUNCTION:  singleSensorInfoRequest
    Request to get the single sensor info
    Parameters
        @req_id : sensor1 transaction ID
        @sensor_id : the sensor ID
===========================================================================*/
    void singleSensorInfoRequest(int req_id, uint8_t sensor_id);
/*===========================================================================
  FUNCTION:  processSingleSensorInfoResp
    Process the response to the sensor1 SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01
    Parameters
        @msensors : the pointer to the sensor class
        @txn_id : sensor1 transaction ID
        @smgr_resp : the sensor1 response sensor info message
===========================================================================*/
    void processSingleSensorInfoResp(Sensor** mSensors, uint8_t txn_id,
                        sns_smgr_single_sensor_info_resp_msg_v01* smgr_resp);
/*===========================================================================
  FUNCTION:  getSensor1Cb
    Get the sensor_info_sensor1_cb pointer.
    Return value
        @hal_sensor1_cb_t* : The sensor_info_sensor1_cb pointer.
===========================================================================*/
    hal_sensor1_cb_t* getSensor1Cb();

private:
    /* only one SensorsContext */
    static SensorsContext self;
    SensorsContext();
    /* sub sensor class */
    Sensor *mSensors[MAX_NUM_SENSORS];
    /* time sync service object */
    TimeSyncService *time_service;
    /* mutex and cond for the sensor info report callback*/
    hal_sensor1_cb_t *sensor_info_sensor1_cb;
    /* mutex and cond for the SMGR callback*/
    hal_sensor1_cb_t *smgr_sensor1_cb;
    /* the data mutex and cond pointer for data sensor1 callback */
    hal_data_cb_t *data_cb;
    /* bit mask of active sensors */
    uint64_t active_sensors;
    /* the mutex to protect the active_sensors */
    pthread_mutex_t active_mutex;
    /* the source for calibrated mag data */
    hal_mag_cal_src_e mag_cal_src;
    /* determines Gyro availability */
    bool is_gyro_available;
    /* determines Mag availability */
    bool is_mag_available;
    /* number of SMGR sensors */
    uint32_t num_smgr_sensors;
    /* SMGR version */
    uint32_t smgr_version;
    /* keep tracking of sensor responses */
    uint32_t sensor_responses;
    /* class OEMlib object */
    OEMLib *oem;
/*===========================================================================
  FUNCTION:  getSensorList
    Get the SMGR sensor information by sending the sensor1 message.
    Return value
        @int : Successful or not.
===========================================================================*/
    int getSensorList();
/*===========================================================================
  FUNCTION:  sendSMGRVersionReq
    Get the SMGR version information by sending the sensor1 message.
    Return value
        @int : Successful or not.
===========================================================================*/
    int sendSMGRVersionReq();
/*===========================================================================
  FUNCTION:  enableLogging
    Enable the log level by getting the property.
===========================================================================*/
    void enableLogging();
/*===========================================================================
  FUNCTION:  addSensor
    Init the sensor handle class.
    Parameters:
        @handle: the sensor handle
===========================================================================*/
    void addSensor(int handle);
/*===========================================================================
  FUNCTION:  checkMagCalSource
    Check the NV3801 for SAM or SMGR Magnetic calibration.
===========================================================================*/
    void checkMagCalSource();
/*===========================================================================
  FUNCTION:  checkGyroAvailability
    Check the NV1918 for Gyro availability.
===========================================================================*/
    void checkGyroAvailability();
/*===========================================================================
  FUNCTION:  checkMagAvailability
    Check the NV1835 for Mag availability.
===========================================================================*/
    void checkMagAvailability();
};

/*===========================================================================

  FUNCTION:  context_sensor1_cb
    Handle the sensor1 callback for get SMGR sensors list.
    Parameters
    @cb_data : pointer of the callback data, SensorsContext is passed in
            this function
    @msg_hdr : sensor1 message header
    @msg_type : sensor1 message type, two major types are listed in the below:
            SENSOR1_MSG_TYPE_RESP
            SENSOR1_MSG_TYPE_IND
    @msg_ptr : sensor1 message pointer, do free this memory before return

===========================================================================*/
void context_sensor1_cb (intptr_t cb_data,
                     sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr);

#endif /* SENSORS_CONTEXT_H */
