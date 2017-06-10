/*============================================================================
@file ThresholdSensorController.java

@brief
Handler for all click-events related to the Threshold activity.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.threshold;

import android.hardware.SensorManager;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.qti.sensors.stream.SensorController;
import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.TabControl;

public class ThresholdSensorController extends SensorController  {
  private ThresholdSensorAdapter sensorAdapter;
  private ThresholdDialog activeDialog;

  public ThresholdSensorController(SensorManager sensorManager, ThresholdSensorAdapter sensorAdapter) {
    super(sensorManager, sensorAdapter);
    this.sensorAdapter = sensorAdapter;
    this.activeDialog = null;
  }

  @Override
  public boolean onLongClick(View view) {
    return true;
  }

  @Override
  public void onClick(View view) {
    if(R.id.stream_data_column1 == view.getId() || R.id.stream_data_column2 == view.getId() ||
        R.id.stream_sensor_title == view.getId() || R.id.stream_sensor_rate == view.getId()){
      this.activeDialog = new ThresholdDialog(this);
      this.activeDialog.show();
    }
    else if(R.id.stream_button_cancel == view.getId()){
      this.sensorAdapter.thresholdIs(-1, 0, 0, 0);
    }
    else if(R.id.thresh_button_submit == view.getId()){
      EditText rateField = (EditText) activeDialog.findViewById(R.id.thresh_delay_field);
      EditText threshXField = (EditText) activeDialog.findViewById(R.id.thresh_x_field);
      EditText threshYField = (EditText) activeDialog.findViewById(R.id.thresh_y_field);
      EditText threshZField = (EditText) activeDialog.findViewById(R.id.thresh_z_field);

      try{
        int rate = Integer.parseInt(rateField.getText().toString());
        int threshXValue = Integer.parseInt(threshXField.getText().toString());
        int threshYValue = Integer.parseInt(threshYField.getText().toString());
        int threshZValue = Integer.parseInt(threshZField.getText().toString());
        this.sensorAdapter.thresholdIs(rate, threshXValue, threshYValue, threshZValue);
        activeDialog.cancel();
      } catch(NumberFormatException e){
        Toast.makeText(TabControl.getContext(), "Invalid number entry", Toast.LENGTH_LONG).show();
      }
    }
    else if(R.id.thresh_button_cancel == view.getId()){
      this.activeDialog.cancel();
    }
  }
}