/*============================================================================
@file ThresholdListView.java

@brief
View corresponding to the list of Sensors on the Threshold tab.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.threshold;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.widget.LinearLayout;

import com.qti.sensors.stream.SensorController;
import com.qti.sensors.stream.SensorListView;
import com.qti.sensors.stream.SensorView;
import com.qualcomm.sensors.qsensortest.SettingsDatabase.SensorSetting;

public class ThresholdListView extends SensorListView {
  public ThresholdListView(Context context, SensorManager sensorManager) {
    super(context, sensorManager);
  }

  @Override
  protected SensorView createView(SensorSetting settings, Sensor sensor) {
    ThresholdSensorAdapter sensorAdapter = null;
    ThresholdSensorView sensorView = null;

    if(settings.getEnableThresh()) {
      sensorAdapter = new ThresholdSensorAdapter(sensor, sensorManager);
      sensorView = new ThresholdSensorView(sensorAdapter, sensorManager);
    }

    return sensorView;
  }

  public class ThresholdSensorView extends SensorView {
    private ThresholdSensorAdapter sensorAdapter;
    public ThresholdSensorView(ThresholdSensorAdapter sensorAdapter, SensorManager sensorManager) {
      super(sensorAdapter, sensorManager);
      this.sensorAdapter = sensorAdapter;
      this.controller = initController(sensorManager);
      this.initView(sensorManager);

      this.reportRateView.setVisibility(LinearLayout.INVISIBLE);
    }

    @Override
    protected SensorController initController(SensorManager sensorManager) {
      if(null != this.sensorAdapter)
        return new ThresholdSensorController(sensorManager, this.sensorAdapter);
      else
        return null;
    }
  }
}