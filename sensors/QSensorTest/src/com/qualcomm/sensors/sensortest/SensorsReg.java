/*============================================================================
@file SensorsReg.java

@brief
Provides access to the Sensors Registry.

Ensure that libsensor_reg.so has been loaded onto the device, or is
included with the application.

Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qualcomm.sensors.sensortest;

public class SensorsReg {
   // Tag to use when logging messages to the android device (viewable through logcat).
   private static final String TAG = "SensorReg";

   /**
    * Load the native functions.  The library will be named on the file system "libsensor_reg.so",
    * and must be pushed to the device before use (to system/lib, or [app_data]/lib).
    */
   static {
      System.loadLibrary("sensor_reg");
   }

   /**
    * Set the value of entry in the sensors registry.  This new value will persist future
    * calls to close() and device reboots.
    *
    * @param itemID The ID of the item to write.  See sns_reg_api_v02.h.
    * @param data Array of data to be written
    * @param len Number of bytes in data
    * @return  0: Success
    *         -1: Internal error
    *         -2: Response error - Timeout
    *         -3: Unable to process response message
    *         -4: Sensor1 error
    *         -5: Range Error: len
    *         -6: Did not call open() first
    */
   static public native int setRegistryValue(int itemID, byte[] data, byte len);

   /**
    * Request a value from the sensors registry.
    *
    * @param itemID The ID of the item to read.  See sns_reg_api_v02.h.
    * @return The value in the registry, separated into its component bytes.
    * @throws Exception Upon error, includes error code:
    *         -1: Internal error
    *         -2: Response error - Timeout
    *         -3: Unable to process response message
    *         -4: Sensor1 error
    *         -6: Did not call open() first
    */
   static public native byte[] getRegistryValue(int itemID);

   /**
    * Initializes the internal data structures used to read/write registry values.
    *
    * @return 0 upon success; <0 upon error
    */
   static public native int open();

   /**
    * Closes and/or destroys the internal data structures created by open().
    *
    * @return 0 upon success; <0 upon error
    */
   static public native int close();
}
