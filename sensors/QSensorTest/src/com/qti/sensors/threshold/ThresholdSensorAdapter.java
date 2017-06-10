/*============================================================================
@file ThresholdSensorAdapter.java

@brief
Data backing an item in the sensor list in the Threshold tab.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.threshold;

import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.util.Log;
import android.widget.Toast;

import com.qti.sensors.stream.SensorAdapter;
import com.qti.sensors.stream.SensorListener;
import com.qualcomm.sensors.qsensortest.TabControl;
import com.qualcomm.sensors.sensortest.SensorThresh;

public class ThresholdSensorAdapter extends SensorAdapter {
   private SensorThresh.ThresholdInstanceID instanceID;

   public ThresholdSensorAdapter(Sensor sensor, SensorManager sensorManager) {
      super(sensor, sensorManager);
      this.instanceID = null;
      this.sensorListener = new SensorListener(this);
   }

   public SensorThresh.ThresholdInstanceID instanceID() { return this.instanceID; }
   public void instanceIDIs(SensorThresh.ThresholdInstanceID instanceID) { this.instanceID = instanceID; }

   public void thresholdIs(int streamRate, int threshXValue, int threshYValue, int threshZValue) {
     if(null != this.instanceID()) {
       try {
         SensorThresh.unregisterThreshold(this.instanceID());
         this.instanceIDIs(null);
         this.streamRateIs(-1);
       } catch (Exception e) {
         e.printStackTrace();
       }
     }

     // If we just enabled or changed a sensor
     if(-1 != streamRate)
       this.sensorEventsClear();

     Log.d(ThresholdActivity.TAG, "Change threshold sensor rate for " + this.sensor().getType() +
         " from " + this.streamRate() + " to " + streamRate);

     if(-1 != streamRate) {
       try {
         SensorThresh.ThresholdInstanceID instanceID = SensorThresh.registerThreshold(
             this.sensor(), streamRate,
             threshXValue, threshYValue, threshZValue,
             this.sensorListener);
         this.instanceIDIs(instanceID);
         this.streamRateIs(streamRate);
       } catch (Exception e) {
         e.printStackTrace();
         Toast.makeText(TabControl.getContext(),
             "Error registering threshold", Toast.LENGTH_LONG).show();
         Log.e(ThresholdActivity.TAG, "Error registering threshold");
       }
     }
   }
}