/*============================================================================
@file TriggerSensorView.java

@brief
The view area associated with a trigger-mode SensorListItem

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.annotation.SuppressLint;
import android.hardware.SensorManager;

@SuppressLint("ViewConstructor")
public class TriggerSensorView extends EventSensorView {
  public TriggerSensorView(SensorAdapter sensorAdapter, SensorManager sensorManager) {
    super(sensorAdapter, sensorManager);
  }
}