/*============================================================================
@file UserCal.java

@brief
Provides LA-layer access to the Sensors User Cal functionality.

Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qualcomm.sensors.sensortest;

public class SensorUserCal {
   // Tag to use when logging messages to the android device (viewable through logcat).
   private static final String TAG = "SensorUserCal";

   /**
    * Load the native functions.  The library will be named on the file system "libsensor_reg.so",
    * and must be pushed to the device before use (to system/lib, or [app_data]/lib).
    */
   static {
      System.loadLibrary("sensor_user_cal");
   }

   /**
    * Performs calibration on the applicable sensor and immediately applies the results.
    *
    * @param sensorType Defines the sensor that this configuration pertains to. The sensor can be one of following:
    *        00 - SNS_SMGR_ID_ACCEL
    *        10 - SNS_SMGR_ID_GYRO
    *        20 - SNS_SMGR_ID_MAG
    *        30 - SNS_SMGR_ID_PRESSURE
    *        40 - SNS_SMGR_ID_PROX_LIGHT
    * @param dataType Defines sensor data type which classifies if the data type is primary or secondary.
    *        00 - SNS_SMGR_DATA_TYPE_PRIMARY
    *        01 - SNS_SMGR_DATA_TYPE_SECONDARY
    * @return
    *        0: No error
    *       -1: Response error - timeout
    *       -2: Sensor1 Error
    *       -3: Error using bias
    *       -4: Error writing to registry
    */
   static public native synchronized int performUserCal(byte sensorType, byte dataType);
}
