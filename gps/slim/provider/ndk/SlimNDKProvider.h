/******************************************************************************
  @file:  SlimNDKProvider.h
  @brief: GNSS / Sensor Interface Framework Support

  DESCRIPTION
    This file defines the interface to accessing sensor data through Native Android
    NDK API.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
05/01/11   jb       Initial version
08/01/11   jb       1. Misc typedef changes

======================================================================*/
#ifndef __SLIM_NDK_PROVIDER_H__
#define __SLIM_NDK_PROVIDER_H__

#include <stdbool.h>
#include "slim_client_types.h"
#include "SlimDaemonMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
FUNCTION    slim_ndk_init

DESCRIPTION
   Initializes internal structures for sensor provider.

   p_msg_q: Message Queue to place sensor/spi/msi data in using
   daemon msg structures.

   Note: Function should not be called from a time sensitive thread as this
         function may block for sensor initialization.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_ndk_init();

/*===========================================================================
FUNCTION    slim_ndk_init_after_request

DESCRIPTION
   Initializes internal structures for sensor provider.

   Note: Function should not be called from a time sensitive thread as this
         function may block for sensor initialization.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_ndk_init_after_request();

/*===========================================================================
FUNCTION    slim_ndk_update_sensor_status

DESCRIPTION
  Function updates the current running status of the accelerometer from sensor
  provider. If sensor is already reporting sensor data and it is requested to
  start again, this function is a nop. If sensor is already stopped and it is
  requested to stop this function is a nop.

  running:       TRUE - start sensor reporting, FALSE - stop sensor reporting
  sampling_rate: Sampling rate in Hz
  batching_rate: Batch samples at this frequency in Hz
  sensor_type:   What kind of sensor are we updating

  Note: Function should not be called from a time sensitive thread as this
        function may block for some time to start up sensors.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A


===========================================================================*/
bool slim_ndk_update_sensor_status(bool running, uint32_t sampling_rate, uint32_t batching_rate, slimServiceEnumT sensor_type);


/*===========================================================================
FUNCTION    slim_ndk_destroy

DESCRIPTION
  Function cleans up any resources the sensor provider may have been using.
  It will also stop reporting sensor data if it was not stopped previously.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_ndk_destroy();

/*===========================================================================
FUNCTION    slim_ndk_get_sensor_time

DESCRIPTION
  Function gets the current sensor time in milliseconds.

  Note: Function should not be called from a time sensitive thread as this
        function may block for some time to query the sensor time.

DEPENDENCIES
   N/A

RETURN VALUE
   1 : Success
   0 : Failure to read sensor time

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_ndk_get_sensor_time(int32 l_ServiceTxnId);


/*===========================================================================
FUNCTION    sp_send_sensor_data_batch

DESCRIPTION
   Helper function to look at the given QMI TLV. It will inspect the amount of
   data in the batch and will send to SLIM data task if the total number of
   samples exceeds the max number of samples or if the interval of samples in
   ms exceeds the batching interval in ms.

   p_msg_q:            Message queue to send message to.
   sensor_str:         String name of sensor to use in logging
   p_slim_data_msg:   SLIM message with sensor data message to send to SLIM data task if batch exceeded.
   p_sensor_data_tlv:  TLV tested for either max number of samples or exceeding batch interval for these samples (accel or gyro).
   p_sensor_temp_tlv:  TLV tested for either max number of samples or exceeding batch interval for these samples (accel or gyro temperature).
   p_baro_data:        TLV tested for either max number of samples or exceeding batch interval for these samples (baro)
   sample_interval:    Length of a sample in milliseconds. (1000 / sampling rate)
   batching_interval:  Length of a batch of samples in milliseconds (1000 / batching rate)

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
void sp_send_sensor_data_batch( const char* sensor_str,
        SlimDaemonMessage* p_slim_data_msg,
        slimSensorDataStructT*  p_sensor_data_tlv,
        slimServiceEnumT e_service,
        double sample_interval,
        double batching_interval);

/*===========================================================================
FUNCTION    sp_process_raw_sensor_data

DESCRIPTION
   Helper function to process the raw sensor samples, log the sample and place
   the sample with timestamp into the proper QMI TLV location.

   Note: Assumes samples are in order based on timestamp for sample and the previous
         samples found in p_sensor_data_tlv. Make sure they are inserted in order
         based on a monotonically increasing timestamp.

   sensor_str:         String name of sensor to use in logging
   data:               Array of 3 data points in sensor sample (0=x, 1=y, 2=z)
   timestamp_ms:       Timestamp of the sensor sample in milliseconds
   p_sensor_data_tlv:  TLV of where to add the sensor sample to the end of.
   p_sensor_temp_tlv:  TLV of where to add the sensor temperature sample to the end of.
   p_baro_data:        TLV tested for either max number of samples or exceeding batch interval for these samples (baro)
DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
void sp_process_raw_sensor_data(const char* sensor_str, float data[3], uint32_t timestamp_ms,
                                slimSensorDataStructT* p_sensor_data_tlv);

/*===========================================================================
FUNCTION    sp_get_sensor_time_offset_ms

DESCRIPTION
   Function to return the current sensor time offset. Needed to implement
   testing to fake time jumps for specified durations.

DEPENDENCIES
   N/A

RETURN VALUE
   Current sensor time offset in milliseconds

SIDE EFFECTS
   N/A

===========================================================================*/
int32_t sp_get_sensor_time_offset_ms();

/*===========================================================================
FUNCTION    sp_read_sys_time_ms

DESCRIPTION
   Utility function to return the current system time in milliseconds.

DEPENDENCIES
   N/A

RETURN VALUE
   Current system time in milliseconds.

SIDE EFFECTS
   N/A

===========================================================================*/
uint32_t sp_read_sys_time_ms();

/*===========================================================================
FUNCTION    sp_read_sys_time_us

DESCRIPTION
   Utility function to return the current system time in microseconds.

DEPENDENCIES
   N/A

RETURN VALUE
   Current system time in milliseconds.

SIDE EFFECTS
   N/A

===========================================================================*/
uint64_t sp_read_sys_time_us();

#ifdef __cplusplus
}
#endif

#endif /* __SLIM_NDK_PROVIDER_H__ */
