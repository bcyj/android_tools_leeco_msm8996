/*============================================================================
@file EventSensorView.java

@brief
This class bundles the several attributes of how each event sensor will appear
to the user.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.annotation.SuppressLint;
import android.graphics.Rect;
import android.hardware.SensorManager;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qualcomm.sensors.qsensortest.R;

@SuppressLint("ViewConstructor")
public class EventSensorView extends SensorView {
  public EventSensorView(SensorAdapter sensorAdapter, SensorManager sensorManager) {
    super(sensorAdapter, sensorManager);
  }

  @Override
  protected int itemViewLayout(){ return R.layout.event_sensor_list_item; }

  protected SensorController initController(SensorManager sensorManager) {
    return new EventSensorController(sensorManager, this.sensorAdapter);
  }

  @Override
  protected void initView(SensorManager sensorManager) {
    SensorController clickListener = this.controller;

    this.columnData1 = (TextView) this.childView.findViewById(R.id.stream_event_column1);
    this.columnData2 = (TextView) this.childView.findViewById(R.id.stream_event_column2);

    this.columnData1.setOnClickListener(clickListener);
    this.columnData1.setOnLongClickListener(clickListener);
    this.columnData2.setOnClickListener(clickListener);
    this.columnData2.setOnLongClickListener(clickListener);

    this.removeListenerButton = (ImageButton) this.childView.findViewById(R.id.stream_button_cancel);
    this.removeListenerButton.setOnClickListener(this.controller);
  }

  @Override
  public void updateView(){
    Rect scrollBounds = new Rect();
    this.childView.getHitRect(scrollBounds);
    if(this.streamRate != this.sensorAdapter.streamRate() ||
       this.childView.getLocalVisibleRect(scrollBounds)) {
      this.columnData1.setText(this.sensorAdapter.sensorData()[0]);
      this.columnData2.setText(this.sensorAdapter.sensorData()[1]);

      if(-1 == this.sensorAdapter.streamRate())
        this.removeListenerButton.setVisibility(LinearLayout.INVISIBLE);
      else
        this.removeListenerButton.setVisibility(LinearLayout.VISIBLE);

      this.streamRate = this.sensorAdapter.streamRate();
    }
  }
}