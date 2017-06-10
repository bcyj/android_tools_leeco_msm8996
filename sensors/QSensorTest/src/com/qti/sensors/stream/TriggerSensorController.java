/*============================================================================
@file SensorTriggerController.java

@brief
Handles button-clicks for trigger-mode sensors.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.hardware.SensorManager;
import android.view.View;

public class TriggerSensorController extends EventSensorController {

  public TriggerSensorController(SensorManager sensorManager,
      SensorAdapter sensorAdapter) {
    super(sensorManager, sensorAdapter);
    // TODO Auto-generated constructor stub
  }

  @Override
  public boolean onLongClick(View view) {
     return true;
  }
}
