/*============================================================================
@file SensorListView.java

@brief
A scrolling view containing the child views associated with each of the sensors.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.ScrollView;

import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.qsensortest.SettingsDatabase.SensorSetting;
import com.qualcomm.sensors.qsensortest.TabControl;

@SuppressLint("ViewConstructor")
public class SensorListView extends LinearLayout {
   protected SensorManager sensorManager;
   protected ScrollView scrollView;
   protected List<SensorView> sensorList;

   public SensorListView(Context context, SensorManager sensorManager) {
      super(context);
      this.sensorManager = sensorManager;

      this.sensorList = new ArrayList<SensorView>();

      LayoutInflater inflater = (LayoutInflater)TabControl.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
      this.scrollView = (ScrollView) inflater.inflate(R.layout.activity_stream_list, null);

      LinearLayout viewList = (LinearLayout) this.scrollView.getChildAt(0);

      int index = 0;
      List<Sensor> sensors = sensorManager.getSensorList(Sensor.TYPE_ALL);
      for(Sensor sensor : sensors) {
        SensorSetting settings = SettingsDatabase.getSettings().getSensorSetting(sensor);
        if(settings.getEnableStreaming()) {
          SensorView sensorView = createView(settings, sensor);

          if(null != sensorView) {
            viewList.addView(sensorView.childView(), index);
            this.sensorList.add(sensorView);
            index++;
          }
        }
      }

      this.addView(this.scrollView);
   }

   protected SensorView createView(SensorSetting settings, Sensor sensor) {
     SensorAdapter sensorAdapter = null;
     SensorView sensorView = null;

     if(SettingsDatabase.TriggerMode.Continuous == settings.getTriggerMode()) {
       sensorAdapter = new SensorAdapter(sensor, sensorManager);
       sensorView = new SensorView(sensorAdapter, sensorManager);
     }
     else if(SettingsDatabase.TriggerMode.OnChange == settings.getTriggerMode()) {
       sensorAdapter = new EventSensorAdapter(sensor, sensorManager);
       sensorView = new EventSensorView(sensorAdapter, sensorManager);
     }
     else if(SettingsDatabase.TriggerMode.OneShot == settings.getTriggerMode()) {
       sensorAdapter = new TriggerSensorAdapter(sensor, sensorManager);
       sensorView = new TriggerSensorView(sensorAdapter, sensorManager);
     }

     return sensorView;
   }

   public List<SensorView> sensorList() { return this.sensorList; }
}