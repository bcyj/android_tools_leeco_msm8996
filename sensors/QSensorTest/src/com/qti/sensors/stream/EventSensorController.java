/*============================================================================
@file SensorEventController.java

@brief
Handles button-clicks for event-sensors.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.annotation.SuppressLint;
import android.hardware.SensorManager;
import android.view.View;

import com.qualcomm.sensors.qsensortest.R;

public class EventSensorController extends SensorController{
  public EventSensorController(SensorManager sensorManager, SensorAdapter sensorAdapter) {
    super(sensorManager, sensorAdapter);
  }

  @SuppressLint("NewApi")
  @Override
  public boolean onLongClick(View view) {
     if(R.id.stream_event_column1 == view.getId() || R.id.stream_event_column2 == view.getId() ||
        R.id.stream_sensor_title == view.getId()) {
        boolean batchSupport = this.sensorAdapter.sensor().getFifoMaxEventCount() > 0;
        this.activeDialog = new SensorDialog(this, this.sensorAdapter.streamRate(),
            this.sensorAdapter.reportRate(), batchSupport);
        this.activeDialog.show();
     }

     return true;
  }

  @SuppressLint("NewApi")
  @Override
  public void onClick(View view) {
    if(R.id.stream_event_column1 == view.getId() || R.id.stream_event_column2 == view.getId() ||
        R.id.stream_sensor_title == view.getId()){
       boolean batchSupport = this.sensorAdapter.sensor().getFifoMaxEventCount() > 0;
       this.activeDialog = new SensorDialog(this, this.sensorAdapter.streamRate(),
           this.sensorAdapter.reportRate(), batchSupport);
       this.sensorAdapter.streamRateIs(SensorManager.SENSOR_DELAY_NORMAL, -1, true);
    }
    else if(R.id.stream_button_cancel == view.getId()){
      this.sensorAdapter.streamRateIs(-1, -1, false);
    }
    else {
       super.onClick(view);
    }
  }
}