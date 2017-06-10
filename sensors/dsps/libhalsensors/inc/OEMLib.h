/*============================================================================
  @file OEMLib.h

  @brief
  OEMLib class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_OEM_LIB_H
#define ANDROID_OEM_LIB_H

#include "Sensor.h"

class OEMLib {
private:
    /* OEM sensors module  */
    struct sensors_module_t *OEM_module;
    /* OEM device */
    static poll_dev_t *OEM_device;
    /* To call OEM poll function */
    pthread_t OEM_poll_thread;
    /* To ensure thread is created only once */
    int thread_counter;
    /* Store the OEM library's own handle*/
    int OEM_handle[MAX_NUM_SENSORS];
    /* OEM light sensor object */
    Sensor *OEM_light;
    /* OEM proximity sensor object*/
    Sensor *OEM_proximity;
public:
/*============================================================================
  FUNCTION Constructor
============================================================================*/
    OEMLib();
/*============================================================================
  FUNCTION Destructor
============================================================================*/
    ~OEMLib();
    int OEMLoadLib();
/*============================================================================
  FUNCTION OEMAddSensors
    Register the OEM sensors to the HAL.
============================================================================*/
    void OEMAddSensors();
/*============================================================================
  FUNCTION OEMActivate
    Enable or disable the sensor of handle.
    Parameter
        @handle : the sensor handle
        @enabled : 1 - enable the sensor; 0 - disable the sensor.
    Return value
        @int : return 0 on success, negative errno code otherwise.
============================================================================*/
    int OEMActivate(int handle, int enabled);
/*============================================================================
  FUNCTION OEMSetDelay
    This function is used to set the delay between sensor events for OEM sensor.
    Parameter
        @handle : the sensor handle
        @ns : data sampling rate in ns
    Return value
        @int : return 0 on success, negative errno code otherwise.
============================================================================*/
    int OEMSetDelay(int handle, int64_t ns);
/*============================================================================
  FUNCTION OEMBatch
    This function is used to set the delay between sensor events for OEM sensor.
    Parameter
        @handle : the sensor handle
        @flags : batching flags
        @ns : data sampling rate in ns
        @timeout : batch report rate latency
    Return value
        @int : return 0 on success, negative errno code otherwise.
============================================================================*/
    int OEMBatch(int handle, int flags, int64_t ns, int64_t timeout);
/*============================================================================
  FUNCTION OEMFlush
    This function is used to flush FIFO of an OEM sensor.
    Parameter
        @handle : the sensor handle
    Return value
        @int : return 0 on success, negative errno code otherwise.
============================================================================*/
    int OEMFlush(int handle);
/*============================================================================
  FUNCTION OEMDataPoll
    This function is used to get the data from the OEM sensors. This is a static
    function so it could be used as a callback of the pthread_create function.
============================================================================*/
    static void* OEMDataPoll(void *ptr);
/*============================================================================
  FUNCTION OEMDataPoll
    This function is used to create one thread for the OEM sensor to poll the
    sensor data.
============================================================================*/
    void OEMCreateThread();
/*============================================================================
  FUNCTION getOEMProximity
    Return value
        @Sensor* : the OEM proximity sensor object
============================================================================*/
    Sensor* getOEMProximity();
/*============================================================================
  FUNCTION getOEMProximity
    Return value
        @Sensor* : the OEM light sensor object
============================================================================*/
    Sensor* getOEMLight();
/*============================================================================
  FUNCTION getOEMSensorHandle
    Find the OEM lib sensor handle by the general sensor handle. The OEM lib may
    use a different handle, it is stored in the OEM_handle[] array.
    Parameter
        @handle : the HAL sensor handle
    Return value
        @Sensor* : the OEM lib sensor handle
============================================================================*/
    int getOEMSensorHandle(int handle);
};

#endif
