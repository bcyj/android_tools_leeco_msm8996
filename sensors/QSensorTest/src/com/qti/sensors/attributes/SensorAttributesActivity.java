/*============================================================================
@file SensorAttributesActivity.java

@brief
Displays a list of the sensors on the device, along with the published
attributes of each.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.attributes;

import java.util.List;

import android.app.Fragment;
import android.hardware.Sensor;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.qti.sensors.stream.SensorAdapter;
import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.TabControl;

public class SensorAttributesActivity extends Fragment{
  private View view;

  /**
   * Called when the activity is first created.
   */
  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
    if(null == this.view){
      this.view = inflater.inflate(R.layout.activity_attributes_layout, null);

      String sensorsInfoText = new String();
      List<Sensor> sensorList = TabControl.sensorManager.getSensorList(Sensor.TYPE_ALL);

      for(Sensor sensor : sensorList){
        SensorAdapter sensorAdapter = new SensorAdapter(sensor, TabControl.sensorManager);
        sensorsInfoText += sensorAdapter.sensorAttributes() + "\n\n";
      }

      TextView sensorsInfo = (TextView) view.findViewById(R.id.sensor_attributes_info);
      sensorsInfo.setText(sensorsInfoText);
    }

    return this.view;
  }
}