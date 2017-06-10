/*============================================================================
@file SensorThresh.java

@brief
Java Native interface to enable the SAM threshold algorithm for various sensors.

@attention
Threshold JNI does not support SSR and Sensors Daemon recovery.  Active
Threshold algorithm instances in such situations must be manually restarted.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.sensortest;

import android.hardware.Sensor;

import com.qualcomm.sensors.qsensortest.QSensorEventListener;

public class SensorThresh {
   /**
    * Load the native functions.  The library will be named on the file system "libsensor_thresh.so",
    * and must be pushed to the device before use (to system/lib, or [app_data]/lib).
    */
   static {
       System.loadLibrary("sensor_thresh");
   }

   /**
    * Registers a listener for the threshold algorithm.
    *
    * @param sensor The physical (not virtual) sensor to run threshold on.
    * @param streamRate Streaming rate of the underlying sensor (in Hz).
    * @param threshXValue Threshold X value.  Note this will likely not correspond to the LA X axis.
    * @param threshYValue Threshold Y value.
    * @param threshZValue Threshold Z value.
    * @param tListener Callback object which implements the onSensorChanged function.
    * @return ThresholdInstance ID, or null on an error.
    */
   public static synchronized SensorThresh.ThresholdInstanceID registerThreshold(Sensor sensor,
         int streamRate, float threshXValue, float threshYValue, float threshZValue,
         QSensorEventListener listener) throws Exception {
      if(0 > streamRate)
         throw new Exception("Stream rate out of bounds: " + streamRate);
      if(0 > threshXValue || 0 > threshYValue || 0 > threshZValue )
         throw new Exception("Threhold value out of bounds: (" + threshXValue + "," +
               threshYValue + "," + threshZValue + ")");

      int instanceID = registerThresholdNative(SensorID.SensorType.getSensorType(sensor).typeValue(),
            SensorID.SensorType.getDataType(sensor).ordinal(),
            streamRate, threshXValue, threshYValue, threshZValue, listener);

      return -1 == instanceID ? null : new SensorThresh.ThresholdInstanceID(instanceID);
   }

   /**
    * Disables a particular instance of the threshold algorithm.  Will throw
    * an exception upon failure.
    *
    * @param tIID The instance ID returned from registerThreshold.
    */
   public static synchronized void unregisterThreshold(SensorThresh.ThresholdInstanceID tIID) throws Exception {
      int rv = unregisterThresholdNative(tIID.instanceID());

      if(0 != rv)
         throw new Exception("Unregister Failed: " + tIID.instanceID());
   }

   /**
    * Registers a listener for the threshold algorithm.
    *
    * @param sensorID The SMGR sensor ID.
    * @param dataType The SMGR sensor data type.
    * @param streamRate Streaming rate of the underlying sensor (in Hz).
    * @param threshXValue Threshold X value.  Note this will likely not correspond to the LA X axis.
    * @param threshYValue Threshold Y value.
    * @param threshZValue Threshold Z value.
    * @param tListener Callback object which implements the onSensorChanged function.
    * @return Instance ID, or < 0 upon error
    */
   private static native int registerThresholdNative(int sensorID, int dataType,
         int streamRate, float threshXValue, float threshYValue,
         float threshZValue, QSensorEventListener tListener);

   /**
    * Disables a particular instance of the threshold algorithm.  Will throw
    * an exception upon failure.
    *
    * @param tIID The instance ID returned from registerThreshold.
    * @return 0 upon success, or < 0 upon error.
    */
   private static native int unregisterThresholdNative(int instanceID);

   /**
    * Value-type class to store the instace ID returned from threshold algorithm.
    */
   public static class ThresholdInstanceID {
      private int instanceID;
      public ThresholdInstanceID(int instanceID) {
         this.instanceID = instanceID;
      }
      public int instanceID() { return this.instanceID; }
  }
}
