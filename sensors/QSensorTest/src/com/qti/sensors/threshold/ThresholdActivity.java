/*============================================================================
@file ThresholdActivity.java

@brief
Direct access to the SAM threshold algorithm.  Only available for physical
sensors.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.threshold;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Fragment;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import com.qti.sensors.stream.SensorListView;
import com.qti.sensors.stream.SensorView;
import com.qti.sensors.stream.StreamingActivity;
import com.qualcomm.sensors.qsensortest.TabControl;

public class ThresholdActivity extends Fragment {
  static public final String TAG = TabControl.TAG;
  static private final int REDRAW_TIMER_MS = 400;
  static public SensorListView sensorListView = null;

  private Timer redrawTimer;
  private Handler drawHandler;

  public ThresholdActivity() {
    this.redrawTimer = null;
    this.drawHandler = null;

    this.drawHandler = new Handler() {
      public void handleMessage(Message msg) {
        for(final SensorView sensorView : ThresholdActivity.sensorListView.sensorList())
          sensorView.updateView();
        this.removeMessages(1);
      }
    };
  }

  /**
   * Called when the activity is first created.
   */
  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
    if(null == ThresholdActivity.sensorListView)
      ThresholdActivity.sensorListView = new ThresholdListView(this.getActivity(), TabControl.sensorManager);

    this.setHasOptionsMenu(true);
    return ThresholdActivity.sensorListView;
  }

  @Override
  public void onResume() {
    super.onResume();
    this.startRedrawTimer();
  }

  private class RedrawTask extends TimerTask {
    @Override
    public void run() {
      if(null != StreamingActivity.sensorListView &&
         null != StreamingActivity.sensorListView.sensorList())
        ThresholdActivity.this.drawHandler.obtainMessage(1).sendToTarget();
    }
  }

  private void startRedrawTimer() {
    this.redrawTimer = new Timer();
    this.redrawTimer.scheduleAtFixedRate(new RedrawTask(), 0, REDRAW_TIMER_MS);
  }

  @Override
  public void onPause() {
    super.onPause();
    this.redrawTimer.cancel();
  }

  /**
   * Load the options menu (defined in xml)
   *
   * @see android.app.Fragment#onOptionsItemSelected(android.view.MenuItem, MenuInflater)
   */
  @Override
  public void onCreateOptionsMenu (Menu menu, MenuInflater inflater) {
  }

  /**
   * Defines what occurs when the user selects one of the menu options.
   *
   * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
   */
  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
    default:
      return super.onOptionsItemSelected(item);
    }
  }
}