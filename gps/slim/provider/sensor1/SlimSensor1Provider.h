/******************************************************************************
  @file:  SlimSensor1Provider.h
  @brief: GNSS / Sensor Interface Framework Support

  DESCRIPTION
    This file defines the interface to accessing sensor data through Sensor1
    API.

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
#ifndef __SLIM_SENSOR1_PROVIDER_H__
#define __SLIM_SENSOR1_PROVIDER_H__

#include <stdbool.h>
#include "slim_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
FUNCTION    slim_sensor1_open

DESCRIPTION
   Opens connection to sensor1 API. Returns true if connection is opened
   successfully.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_sensor1_open();

/*===========================================================================
FUNCTION    slim_sensor1_close

DESCRIPTION
   Closes connection to sensor1 API.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
void slim_sensor1_close();

/*===========================================================================
FUNCTION    slim_sensor1_init

DESCRIPTION
   Initializes internal structures for sensor provider.

   p_msg_q: Message Queue to place sensor/spi/msi data in using slim msg
   structures.

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
bool slim_sensor1_init();

/*===========================================================================
FUNCTION    slim_sensor1_update_motion_data

DESCRIPTION
  Function updates the current running status of the Motion Data (MD)
  algorithm from sensor provider. If MD is already reporting
  and it is requested to start again, this function is a nop. If MD is already
  stopped and it is requested to stop this function is a nop.

  running:       TRUE - start SPI reporting, FALSE - stop SPI reporting

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
bool slim_sensor1_update_motion_data(bool running);

/*===========================================================================
FUNCTION    slim_sensor1_update_pedometer

DESCRIPTION
  Function updates the current running status of pedometer updates from this
  sensor provider.

  running:                TRUE - start pedometer reportig,
                          FALSE - stop pedometer reporting
  reset_step_count:       1 - reset step count,
                          0 - do not reset step count
  step_count_threshold:   0 - keep the previous value or use default value 1
                              if there is no previous value,
                          N - report after every Nth step

  Note: Function should not be called from a time sensitive thread as this
        function may block for some time to start up spi reporting.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
bool slim_sensor1_update_pedometer(bool running, uint8_t reset_step_count, uint32_t step_count_threshold);

/*===========================================================================
FUNCTION    slim_sensor1_update_spi_status

DESCRIPTION
  Function updates the current running status of the Stationary Position
  Indicator (SPI) algorithm from sensor provider. If SPI is already reporting
  and it is requested to start again, this function is a nop. If SPI is already
  stopped and it is requested to stop this function is a nop.

  running:       TRUE - start SPI reporting, FALSE - stop SPI reporting

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
bool slim_sensor1_update_spi_status(bool running);

/*===========================================================================
FUNCTION    slim_sensor1_destroy

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
bool slim_sensor1_destroy();

/*===========================================================================
FUNCTION    slim_sensor1_get_sensor_time

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
bool slim_sensor1_get_sensor_time(int32 l_ServiceTxnId);

/*===========================================================================
FUNCTION    slim_sensor1_update_sensor_status_buffering

DESCRIPTION
  Function updates the current running status of the particular sensor from sensor
  provider.

  running:       TRUE - start sensor reporting, FALSE - stop sensor reporting
  sample_count:  Batch sample count
  report_rate:   Batch reporting rate
  sensor_type:   Sensor type - ACCEL/GYRO/ACCEL TEMP/GYRO TEMP/BARO

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
bool slim_sensor1_update_sensor_status_buffering
(
   bool running,
   uint32_t sample_count,
   uint32_t report_rate,
   slimServiceEnumT sensor_type
);

#ifdef __cplusplus
}
#endif

#endif /* __SLIM_SENSOR1_PROVIDER_H__ */
