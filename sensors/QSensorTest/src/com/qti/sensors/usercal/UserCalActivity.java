/*============================================================================
@file UserCalActivity.java

@brief
A simple Android activity which demonstrated to other developers how to use
the associated User Cal functions.

Ensure that libsensor_user_cal.so has been loaded onto the device.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.usercal;

import java.util.Iterator;
import java.util.List;

import android.app.ListFragment;
import android.content.Context;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

import com.qti.sensors.selftest.SensorListAdapter;
import com.qti.sensors.selftest.SensorListItemView;
import com.qti.sensors.selftest.SensorStatus;
import com.qti.sensors.stream.SensorAdapter;
import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.sensortest.SensorID.SensorType;
import com.qualcomm.sensors.sensortest.SensorUserCal;

public class UserCalActivity extends ListFragment  {
  private static final String TAG = "UserCalApp";
  private List<SensorStatus> sensorList;

  /**
   * Called when the activity is first created.
   */
  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    SensorManager sensorManager = (SensorManager)this.getActivity().getSystemService(Context.SENSOR_SERVICE);
    if(this.sensorList == null)
      this.sensorList = SensorStatus.createSensorList(sensorManager.getSensorList(Sensor.TYPE_ALL));

    for(Iterator<SensorStatus> itr = sensorList.iterator(); itr.hasNext();)
      if(!SettingsDatabase.getSettings().getSensorSetting(itr.next().getSensor()).getEnableUserCal())
        itr.remove();

    this.setListAdapter(new SensorListAdapter(this.getActivity(), this.sensorList));

    ListView lv = getListView();
    lv.setOnItemClickListener(new SensorListClickListener());
  }

  /**
   * This class allows calibration operations to be run in a separate thread, which
   * leaves the UI responsive.  We extend from AsyncTask instead of
   * simply using a new Thread, since the UI can only be modified from the
   * UI thread (which AsyncTask partially runs within).
   */
  private class UserCalTask extends AsyncTask<Object, Void, Integer> {
    private SensorListItemView view;
    private Sensor sensor;

    /**
     * Runs the appropriate self-test
     *
     * @param sensor The Android sensor on which to run the self-test
     * @param view This object is used to update the status/result of the test
     *
     * @return The result of the self-test
     *
     * @see android.os.AsyncTask#doInBackground(Params[])
     */
    @Override
    protected Integer doInBackground(Object... params) {
      this.sensor = (Sensor) params[0];
      this.view = (SensorListItemView) params[1];
      SensorType type = SensorType.getSensorType(this.sensor);

      if(null == type)
        return -2;
      else
        return SensorUserCal.performUserCal((byte)SensorType.getSensorType(this.sensor).typeValue(),
            (byte)SensorAdapter.sensorDataType(this.sensor).ordinal());
    }

    /**
     * Updates the UI and logs the result
     *
     * @param result The result of the calibration as passed from doInBackground()
     *
     * @see android.os.AsyncTask#onPostExecute(java.lang.Object)
     */
    @Override
    protected void onPostExecute(Integer result) {
      if(result == 0){
        this.view.setStatus("User Cal finished", Color.GREEN);
      }
      else{
        this.view.setStatus("User Cal FAILED. Error: " + getError(result), Color.RED);
      }
    }
  }

  /**
   * @param error Code as returned from SensorUserCal.performUserCal
   * @return String description of error
   */
  static private String getError(int error){
    switch(error){
    case -1:
      return "Response error - timeout";
    case -2:
      return "Sensor1 error";
    case -3:
      return "Bias error";
    default:
      return "Unknown Error " + String.valueOf(error);
    }
  }

  /**
   * When a sensor name in the list is "clicked", this function will look-up
   * the sensor and call performUserCal.
   */
  private class SensorListClickListener implements OnItemClickListener{
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
      SensorListAdapter listAdapter = (SensorListAdapter) UserCalActivity.this.getListAdapter();
      SensorStatus testSensor = (SensorStatus) listAdapter.getItem(position);

      SensorListItemView sensorView = (SensorListItemView) view;
      sensorView.setStatus("User Cal Begun", Color.YELLOW);
      new UserCalTask().execute(testSensor.getSensor(), sensorView);
    }
  }
}
