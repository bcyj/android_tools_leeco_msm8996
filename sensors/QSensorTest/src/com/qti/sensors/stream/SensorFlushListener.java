/*============================================================================
@file QSensorFlushListener.java

@brief
Receives flush complete events from the Android Sensors Manager.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.hardware.Sensor;
import android.hardware.SensorEventListener2;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.sensors.qsensortest.TabControl;

public class SensorFlushListener extends SensorListener implements SensorEventListener2 {

  public SensorFlushListener(SensorAdapter sensorAdapter) {
    super(sensorAdapter);
  }

  @Override
  public void onFlushCompleted(Sensor sensor) {
    Toast.makeText(TabControl.getContext(), "Flush complete for:\n" + this.sensorAdapter.sensor().getName(),
          Toast.LENGTH_SHORT).show();
    Log.d(StreamingActivity.TAG, "Flush complete event received for: " + this.sensorAdapter.sensor().getName());
  }
}