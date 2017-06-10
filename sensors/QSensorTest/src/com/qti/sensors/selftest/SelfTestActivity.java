/*============================================================================
@file SelfTestActivity.java

@brief
A simple Android application which demonstrated to other developers how to use
the associated SensorTest functions.

Ensure that libsensor_test.so has been loaded onto the device.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.selftest;

import java.util.Iterator;
import java.util.List;

import android.app.DialogFragment;
import android.app.ListFragment;
import android.content.Context;
import android.database.SQLException;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.Toast;

import com.qti.sensors.stream.SensorAdapter;
import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.sensortest.SensorID;
import com.qualcomm.sensors.sensortest.SensorID.SensorType;
import com.qualcomm.sensors.sensortest.SensorTest;

public class SelfTestActivity extends ListFragment  {
  private static final String TAG = "SelfTestApp";
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
      if(!SettingsDatabase.getSettings().getSensorSetting(itr.next().getSensor()).getEnableSelfTest())
        itr.remove();

    this.setListAdapter(new SensorListAdapter(this.getActivity(), this.sensorList));

    ListView lv = getListView();
    lv.setOnItemClickListener(new SensorListClickListener());
    this.setHasOptionsMenu(true);
  }

  /**
   * Load the options menu (defined in xml)
   *
   * @see android.app.Fragment#onOptionsItemSelected(android.view.MenuItem, MenuInflater)
   */
  @Override
  public void onCreateOptionsMenu (Menu menu, MenuInflater inflater) {
    inflater.inflate(R.menu.selftest_menu, menu);

    String applyCalNow = SettingsDatabase.getSettings().getProperty("selftest_applyCalNow");
    String saveToRegistry = SettingsDatabase.getSettings().getProperty("selftest_saveToRegistry");

    if(null == applyCalNow || null == saveToRegistry){
      Log.e(TAG, "Unable to find selftest_applyCalNow or selftest_saveToRegistry in sqlite database");
      return ;
    }

    menu.findItem(R.id.selftest_menu_apply_cal_now).setChecked(applyCalNow.contentEquals("1"));
    menu.findItem(R.id.selftest_menu_save_to_registry).setChecked(saveToRegistry.contentEquals("1"));
  }


  /**
   * Defines what occurs when the user selects one of the menu options.
   *
   * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
   */
  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
    case R.id.selftest_menu_apply_cal_now:
      try{
        SettingsDatabase.getSettings().setProperty(
            "selftest_applyCalNow", item.isChecked() ? "0" : "1");
        item.setChecked(!item.isChecked());
      } catch(SQLException e) {
        Toast.makeText(this.getActivity(), "Unable to change setting", Toast.LENGTH_LONG).show();
      }

      return true;
    case R.id.selftest_menu_save_to_registry:
      try{
        SettingsDatabase.getSettings().setProperty(
            "selftest_saveToRegistry", item.isChecked() ? "0" : "1");
        item.setChecked(!item.isChecked());
      } catch(SQLException e) {
        Toast.makeText(this.getActivity(), "Unable to change setting", Toast.LENGTH_LONG).show();
      }
      return true;
    case R.id.selftest_menu_test_type:
      DialogFragment newFragment = TestTypeDialog.newInstance();
      newFragment.show(getFragmentManager(), "testTypeDialog");
    default:
      return super.onOptionsItemSelected(item);
    }
  }

  /**
   * This class allows self tests to be run in a separate thread, which
   * leaves the UI responsive.  We extend from AsyncTask instead of
   * simply using a new Thread, since the UI can only be modified from the
   * UI thread (which AsyncTask partially runs within).
   */
  private class SensorTestTask extends AsyncTask<Object, Void, Integer> {
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
      int rv;
      SensorType type = SensorType.getSensorType(this.sensor);

      String applyCalNow = SettingsDatabase.getSettings().getProperty("selftest_applyCalNow");
      String saveToRegistry = SettingsDatabase.getSettings().getProperty("selftest_saveToRegistry");
      String testType = SettingsDatabase.getSettings().getProperty("selftest_testType");

      if(null == type)
        rv = -2;
      else if(null == applyCalNow || null == saveToRegistry || null == testType)
        rv = -50;
      else
        rv = SensorTest.runSensorTest(new SensorID(type, 0),
            SensorAdapter.sensorDataType(this.sensor), SensorTest.TestType.values()[Integer.parseInt(testType)],
            saveToRegistry.contentEquals("1"), applyCalNow.contentEquals("1"));

      return rv;
    }

    /**
     * Updates the UI and logs the result
     *
     * @param testResult The result of the test as passed from doInBackground()
     *
     * @see android.os.AsyncTask#onPostExecute(java.lang.Object)
     */
    @Override
    protected void onPostExecute(Integer testResult) {
      if(testResult == 0){
        this.view.setStatus("Test PASSED", Color.GREEN);
      }
      else{
        this.view.setStatus("Test FAILED. Error: " + getTestError(testResult), Color.RED);
      }
    }
  }

  /**
   * @param error Code as returned from SensorTest.runSensorTest
   * @return String description of error
   */
  static private String getTestError(int error){
    switch(error){
    case -1:
      return "Sensor Test Native Error";
    case -2:
      return "Invalid Sensor ID";
    case -3:
      return "Test Timed-out";
    case -12:
      return "Device Busy";
    case -13:
      return "Invalid Test";
    case -14:
      return "Invalid Test Parameter";
    case -15:
      return "Received 'failed' response";
    case -16:
      return "Another test is running";
    case -21:
      return "Broken Message Pipe";
    case -22:
      return "Internal Error";
    case -50:
      return "Settings Database items not found";
    default:
      return "Sensor Specific error: " + String.valueOf(error);
    }
  }

  /**
   * When a sensor name in the list is "clicked", this function will look-up
   * the sensor and call runSensorTest.
   */
  private class SensorListClickListener implements OnItemClickListener{
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
      SensorListAdapter listAdapter = (SensorListAdapter) SelfTestActivity.this.getListAdapter();
      SensorStatus testSensor = (SensorStatus) listAdapter.getItem(position);

      SensorListItemView sensorView = (SensorListItemView) view;
      sensorView.setStatus("Test Started", Color.YELLOW);
      new SensorTestTask().execute(testSensor.getSensor(), sensorView);
    }
  }
}