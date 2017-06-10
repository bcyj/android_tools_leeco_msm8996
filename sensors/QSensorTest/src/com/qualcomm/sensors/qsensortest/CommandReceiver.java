/*============================================================================
@file CommandReceiver.java

@brief
Receives and processes broadcast commands.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.qsensortest;

import java.util.List;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.util.Log;

import com.qti.sensors.stream.SensorView;
import com.qti.sensors.stream.StreamingActivity;
import com.qti.sensors.threshold.ThresholdActivity;
import com.qti.sensors.threshold.ThresholdSensorAdapter;

public class CommandReceiver extends BroadcastReceiver {
  public CommandReceiver() {}

  @Override
  public void onReceive(Context context, Intent intent) {
    if(intent.getAction().contentEquals("com.qualcomm.sensors.qsensortest.intent.STREAM")) {
      CharSequence sensorName = intent.getCharSequenceExtra("sensorname");
      int sensorType = intent.getIntExtra("sensortype", 0);
      int rate = intent.getIntExtra("rate", 60);
      int batchRate = intent.getIntExtra("batch", -1);

      List<SensorView> sensorViews = StreamingActivity.sensorListView.sensorList();
      for(SensorView sensorView : sensorViews) {
        Sensor sensor = sensorView.sensorAdapter().sensor();
        if(sensor.getName().contentEquals(sensorName) &&
           sensor.getType() == sensorType &&
           SettingsDatabase.getSettings().getSensorSetting(sensor).getEnableStreaming()) {
          Log.d(TabControl.TAG, "CR setting stream rate for " + sensorName + ": " + rate + ", batch:" + batchRate);
          sensorView.sensorAdapter().streamRateIs(rate, batchRate, true);
          break;
        }
      }
    }
    if(intent.getAction().contentEquals("com.qualcomm.sensors.qsensortest.intent.SETTAB")) {
      CharSequence tabName = intent.getCharSequenceExtra("tab");
      for(int i = 0; i < TabControl.getActivity().getActionBar().getTabCount(); i++)
        if(0 == TabControl.getActivity().getActionBar().getTabAt(i).getText().toString().compareTo(tabName.toString()))
          TabControl.getActivity().getActionBar().setSelectedNavigationItem(i);
    }
    if(intent.getAction().contentEquals("com.qualcomm.sensors.qsensortest.intent.THRESH")) {
      CharSequence sensorName = intent.getCharSequenceExtra("sensorname");
      int sensorType = intent.getIntExtra("sensortype", 0);
      int streamRate = intent.getIntExtra("rate", 0);
      int threshXValue = intent.getIntExtra("x", 0);
      int threshYValue = intent.getIntExtra("y", 0);
      int threshZValue = intent.getIntExtra("z", 0);

      List<SensorView> sensorViews = ThresholdActivity.sensorListView.sensorList();
      for(SensorView sensorView : sensorViews) {
        Sensor sensor = sensorView.sensorAdapter().sensor();
        if(sensor.getType() == sensorType) {
          ThresholdSensorAdapter adapter = (ThresholdSensorAdapter)sensorView.sensorAdapter();
          Log.d(TabControl.TAG, "CR setting threshold for " + sensorType + ": " + streamRate +
                ", x:" + threshXValue + ", y:" + threshYValue + ", z:" + threshZValue);
          adapter.thresholdIs(streamRate, threshXValue, threshYValue, threshZValue);
          break;
        }
      }
    }
  }

  public IntentFilter getIntentFilter() {
    IntentFilter intentFilter = new IntentFilter();
    intentFilter.addAction("com.qualcomm.sensors.qsensortest.intent.STREAM");
    Log.d(TabControl.TAG, "Available: com.qualcomm.sensors.qsensortest.intent.STREAM");
    intentFilter.addAction("com.qualcomm.sensors.qsensortest.intent.THRESH");
    Log.d(TabControl.TAG, "Available: com.qualcomm.sensors.qsensortest.intent.THRESH");
    intentFilter.addAction("com.qualcomm.sensors.qsensortest.intent.SETTAB");
    Log.d(TabControl.TAG, "Available: com.qualcomm.sensors.qsensortest.intent.SETTAB");
    intentFilter.addCategory("com.qualcomm.sensors.qsensortest.intent.category.DEFAULT");

    return intentFilter;
  }
}
