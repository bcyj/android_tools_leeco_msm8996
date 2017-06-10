/*============================================================================
@file SensorListItemView.java

@brief
This class defines how each entry in the list of sensors will look,
and provides functions to edit its content.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.selftest;

import android.annotation.SuppressLint;
import android.content.Context;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qti.sensors.stream.SensorAdapter;

/**
 * This class defines how each entry in the list of sensors will look,
 * and provides functions to edit its content.
 *
 */
@SuppressLint("ViewConstructor")
public class SensorListItemView extends LinearLayout {
  private TextView sensorNameView;
  private TextView sensorStatusView;
  private SensorStatus sensorStatus;

  public SensorListItemView(Context context, SensorStatus sensorStatus) {
    super(context);
    this.setOrientation(VERTICAL);
    this.sensorStatus = sensorStatus;

    this.sensorNameView = new TextView(context);
    this.sensorNameView.setText(SensorAdapter.sensorTypeString(sensorStatus.getSensor())
        + ": " + sensorStatus.getSensor().getName());
    addView(this.sensorNameView, new LinearLayout.LayoutParams(
        android.view.ViewGroup.LayoutParams.MATCH_PARENT,
        android.view.ViewGroup.LayoutParams.WRAP_CONTENT));

    this.sensorStatusView = new TextView(context);
    this.sensorStatusView.setText(sensorStatus.getStatus());
    addView(this.sensorStatusView, new LinearLayout.LayoutParams(
        android.view.ViewGroup.LayoutParams.MATCH_PARENT,
        android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
    this.sensorStatusView.setTextColor(sensorStatus.getColor());
  }

  public void setStatus(String status, int color) {
    this.sensorStatusView.setText(status);
    this.sensorStatusView.setTextColor(color);

    this.sensorStatus.setStatus(status);
    this.sensorStatus.setColor(color);
  }
  public SensorStatus getSensorStatus(){
    return this.sensorStatus;
  }
}