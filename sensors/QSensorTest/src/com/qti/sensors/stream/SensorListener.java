/*============================================================================
@file QSensorListener.java

@brief
SensorEvent listener.  Updates the associated QSensor upon event receipt.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.annotation.SuppressLint;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.TriggerEvent;

import com.qualcomm.sensors.qsensortest.QSensorEventListener;
import com.qualcomm.sensors.qsensortest.TabControl;

@SuppressLint("NewApi")
public class SensorListener extends QSensorEventListener implements SensorEventListener {
  protected SensorAdapter sensorAdapter;

  public SensorListener(SensorAdapter sensorAdapter) {
    this.sensorAdapter = sensorAdapter;
  }

  @Override
  public void onAccuracyChanged(Sensor sensor, int accuracy) {
    if(!TabControl.optimizePower)
      this.sensorAdapter.accuracyIs(accuracy);
  }

  @Override
  public void onSensorChanged(SensorEvent event) {
    if(!TabControl.optimizePower)
      this.sensorAdapter.eventIs(this.sensorAdapter.new SensorSample(event));
  }

  @Override
  public void onTrigger(TriggerEvent event) {
    if(!TabControl.optimizePower)
      this.sensorAdapter.eventIs(this.sensorAdapter.new SensorSample(event));
  }
}