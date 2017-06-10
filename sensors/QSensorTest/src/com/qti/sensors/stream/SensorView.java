/*============================================================================
@file QSensorView.java

@brief
The view area associated with a particular QSensor.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Rect;
import android.hardware.SensorManager;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.TabControl;

@SuppressLint("ViewConstructor")
public class SensorView {
  protected SensorAdapter sensorAdapter;
  protected View childView;
  protected SensorController controller;
  protected TextView streamRateView, reportRateView, columnData1, columnData2,
    streamTSView, streamAccuracyView;
  protected ImageButton removeListenerButton;

  // Sensor data saved here for performance reasons
  protected long lastTimestamp;
  protected int streamRate;

  public SensorView(SensorAdapter sensorAdapter, SensorManager sensorManager) {
    this.sensorAdapter = sensorAdapter;
    this.lastTimestamp = 0;
    this.streamRate = -2;

    LayoutInflater inflater = (LayoutInflater)TabControl.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    this.childView = inflater.inflate(this.itemViewLayout(), null);

    this.controller = this.initController(sensorManager);
    this.initView(sensorManager);

    TextView titleView = (TextView) this.childView.findViewById(R.id.stream_sensor_title);
    titleView.setText(this.sensorAdapter.title());
    titleView.setOnClickListener(this.controller);
    titleView.setOnLongClickListener(this.controller);

    this.updateView();
  }

  public View childView() { return this.childView; }

  protected int itemViewLayout(){ return R.layout.sensor_list_item; }

  protected SensorController initController(SensorManager sensorManager) {
    return new SensorController(sensorManager, this.sensorAdapter);
  }

  protected void initView(SensorManager sensorManager) {
    SensorController clickListener = this.controller;

    this.streamRateView = (TextView) this.childView.findViewById(R.id.stream_sensor_rate);
    this.reportRateView = (TextView) this.childView.findViewById(R.id.stream_batch_rate);
    this.columnData1 = (TextView) this.childView.findViewById(R.id.stream_data_column1);
    this.columnData2 = (TextView) this.childView.findViewById(R.id.stream_data_column2);
    this.streamTSView = (TextView) this.childView.findViewById(R.id.stream_data_ts);
    this.streamAccuracyView = (TextView) this.childView.findViewById(R.id.stream_data_accuracy);
    this.removeListenerButton = (ImageButton) this.childView.findViewById(R.id.stream_button_cancel);

    this.columnData1.setOnClickListener(clickListener);
    this.columnData1.setOnLongClickListener(clickListener);
    this.columnData2.setOnClickListener(clickListener);
    this.columnData2.setOnLongClickListener(clickListener);
    this.streamAccuracyView.setOnClickListener(clickListener);
    this.streamAccuracyView.setOnLongClickListener(clickListener);
    this.streamRateView.setOnClickListener(clickListener);
    this.streamRateView.setOnLongClickListener(clickListener);

    this.removeListenerButton.setOnClickListener(clickListener);
  }

  public SensorAdapter sensorAdapter() { return this.sensorAdapter; }

  /*
   * The components of the view that may require updating when sensor data changes, or other events occur.
   */
  public void updateView(){
    Rect scrollBounds = new Rect();
    this.childView.getHitRect(scrollBounds);

    if((this.childView.getLocalVisibleRect(scrollBounds) &&
        this.lastTimestamp != this.sensorAdapter.lastTimestamp()) ||
       this.streamRate != this.sensorAdapter.streamRate()) {
      String streamRate = this.sensorAdapter.streamRateString();
      String reportRate = this.sensorAdapter.reportRateString();
      String sensorData[] = this.sensorAdapter.sensorData();
      String accuracy = this.sensorAdapter.accuracyString();
      String timestamp = this.sensorAdapter.timestamp();

      this.streamRateView.setText(streamRate);
      if(!this.reportRateView.getText().equals(reportRate))
        this.reportRateView.setText(reportRate);
      if(!this.streamAccuracyView.getText().equals(accuracy))
        this.streamAccuracyView.setText(accuracy);
      this.columnData1.setText(sensorData[0]);
      this.columnData2.setText(sensorData[1]);
      this.streamTSView.setText(timestamp);

      if(-1 == this.sensorAdapter.streamRate())
        this.removeListenerButton.setVisibility(LinearLayout.INVISIBLE);
      else
        this.removeListenerButton.setVisibility(LinearLayout.VISIBLE);

      this.lastTimestamp = this.sensorAdapter.lastTimestamp();
      this.streamRate = this.sensorAdapter.streamRate();
    }
  }
}
