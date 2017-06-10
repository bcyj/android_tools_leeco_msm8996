/*============================================================================
@file StreamingActivity.java

@brief
Fragment to display and test streaming from sensors.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.stream;

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import android.annotation.SuppressLint;
import android.app.Fragment;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;

import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.qsensortest.TabControl;
import com.qualcomm.sensors.sensortest.SensorTest;

public class StreamingActivity extends Fragment {
  static final public String TAG = TabControl.TAG;
  static final private int REDRAW_TIMER_MS = 300;
  static public SensorListView sensorListView = null;
  static public boolean autoRetryTrigger = true;
  static public boolean beepEnabled = false;
  static private boolean updateUI = true;

  private SensorDialog activeDialog;
  private Timer redrawTimer;
  private Handler drawHandler;

  public StreamingActivity() {
    this.redrawTimer = null;
    this.drawHandler = null;
    this.activeDialog = null;

    String updateScreenDef = SettingsDatabase.getSettings().getProperty("update_screen_def");
    StreamingActivity.updateUI = null != updateScreenDef && updateScreenDef.equals("1");

    this.drawHandler = new Handler() {
      public void handleMessage(Message msg) {
        for(final SensorView sensorView : StreamingActivity.sensorListView.sensorList())
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
    if(null == StreamingActivity.sensorListView)
      StreamingActivity.sensorListView = new SensorListView(this.getActivity(), TabControl.sensorManager);

    this.setHasOptionsMenu(true);

    return StreamingActivity.sensorListView;
  }

  @Override
  public void onResume() {
    super.onResume();

    if(null != this.redrawTimer) {
      this.redrawTimer.cancel();
      this.redrawTimer = null;
    }
    if(StreamingActivity.updateUI)
      this.startRedrawTimer();
  }

  private class RedrawTask extends TimerTask {
    @Override
    public void run() {
      if(null != StreamingActivity.sensorListView &&
         null != StreamingActivity.sensorListView.sensorList())
        StreamingActivity.this.drawHandler.obtainMessage(1).sendToTarget();
    }
  }

  private void startRedrawTimer() {
    this.redrawTimer = new Timer();
    this.redrawTimer.scheduleAtFixedRate(new RedrawTask(), 0, REDRAW_TIMER_MS);
  }

  @Override
  public void onPause() {
    super.onPause();

    if(null != this.redrawTimer) {
      this.redrawTimer.cancel();
      this.redrawTimer = null;
    }
  }

  /**
   * Load the options menu (defined in xml)
   *
   * @see android.app.Fragment#onOptionsItemSelected(android.view.MenuItem, MenuInflater)
   */
  @Override
  public void onCreateOptionsMenu (Menu menu, MenuInflater inflater) {
    inflater.inflate(R.menu.streaming_menu, menu);

    int menuIDs[] = {R.id.all_listeners_add, R.id.raw_data_mode_toggle, R.id.all_listeners_add,
        R.id.all_listeners_remove, R.id.ui_data_update_toggle, R.id.retrigger_toggle,
        R.id.all_listeners_add
    };

    for(int menuID : menuIDs) {
      boolean menuItemEnabled = SettingsDatabase.getSettings().getMenuItemEnabled(
          TabControl.getContext().getResources().getResourceEntryName(menuID));

      MenuItem menuItem = menu.findItem(menuID);
      menuItem.setEnabled(menuItemEnabled);

      if(R.id.raw_data_mode_toggle == menuID) {
        try {
          menuItem.setEnabled(true);
          menuItem.setChecked(SensorTest.getRawDataMode());
        } catch(LinkageError e) {
          menuItem.setEnabled(false);
        }
      }
      else if(R.id.retrigger_toggle == menuID) {
        String autoRetrigger = SettingsDatabase.getSettings().getProperty("auto_retrigger");
        menuItem.setChecked(null != autoRetrigger && autoRetrigger.equals("1"));
      }
      else if(R.id.ui_data_update_toggle == menuID) {
        menuItem.setChecked(StreamingActivity.updateUI);
      }
      else if(R.id.beep_toggle == menuID) {
        menuItem.setChecked(StreamingActivity.beepEnabled);
      }
    }
  }

  /**
   * Defines what occurs when the user selects one of the menu options.
   *
   * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
   */
  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
    case R.id.raw_data_mode_toggle: // Check the current value, change to the opp., check again, then report the results
      boolean enabled = SensorTest.getRawDataMode();

      try { SensorTest.setRawDataMode( !enabled );
      } catch (Exception e) {
        Toast.makeText(this.getActivity(), e.getMessage(), Toast.LENGTH_LONG);
      }

      boolean newEnabled = SensorTest.getRawDataMode();

      if(newEnabled != enabled)
        Toast.makeText(this.getActivity(), "Raw Mode setting will be applied on new streams", Toast.LENGTH_SHORT).show();
      else
        Toast.makeText(this.getActivity(), "Unable to change Raw Data Mode", Toast.LENGTH_LONG).show();

      item.setChecked(newEnabled);
      return true;
    case R.id.all_listeners_add:
      boolean batchSupport = true;
      this.activeDialog = new SensorDialog(new StreamingActivity.AllSensorController(TabControl.sensorManager), -1, -1, batchSupport);
      this.activeDialog.show();
      return true;
    case R.id.all_listeners_remove:
      this.clearAllListeners(TabControl.sensorManager);
      return true;
    case R.id.ui_data_update_toggle:
      if(null != this.redrawTimer) {
        this.redrawTimer.cancel();
        this.redrawTimer = null;
      }

      StreamingActivity.updateUI = !StreamingActivity.updateUI;
      item.setChecked(StreamingActivity.updateUI);
      if(StreamingActivity.updateUI)
        this.startRedrawTimer();
      return true;
    case R.id.retrigger_toggle:
      StreamingActivity.autoRetryTrigger = !StreamingActivity.autoRetryTrigger;
      item.setChecked(StreamingActivity.autoRetryTrigger);
      return true;
    case R.id.beep_toggle:
      StreamingActivity.beepEnabled = !StreamingActivity.beepEnabled;
      item.setChecked(beepEnabled);
      return true;
    case R.id.all_listeners_flush:
      this.flushAllListeners(TabControl.sensorManager);
      return true;
    default:
      return super.onOptionsItemSelected(item);
    }
  }

  protected void clearAllListeners(SensorManager sensorManager) {
    List<SensorView> sensorList = StreamingActivity.sensorListView.sensorList();

    for(SensorView sensorView : sensorList) {
      sensorView.sensorAdapter().streamRateIs(-1, -1, false);
      sensorView.updateView();
    }
  }

  protected void flushAllListeners(SensorManager sensorManager) {
    List<SensorView> sensorList = StreamingActivity.sensorListView.sensorList();

    for(SensorView sensorView : sensorList)
      sensorView.sensorAdapter().flush(sensorManager);
  }

  protected void setAllListeners(SensorManager sensorManager,
      int streamRate, int reportRate, boolean clear) {
    List<SensorView> sensorList = StreamingActivity.sensorListView.sensorList();

    for(SensorView sensorView : sensorList)
      sensorView.sensorAdapter().streamRateIs(streamRate, reportRate, clear);
  }

  private class AllSensorController implements OnClickListener {
    private SensorManager sensorManager;
    public AllSensorController(SensorManager sensorManager) {
      this.sensorManager = sensorManager;
    }

    @SuppressLint("NewApi")
    @Override
    public void onClick(View view) {
      if(R.id.delay_button_normal == view.getId())
        StreamingActivity.this.setAllListeners(sensorManager, SensorManager.SENSOR_DELAY_NORMAL, -1, true);
      else if(R.id.delay_button_ui == view.getId())
        StreamingActivity.this.setAllListeners(sensorManager, SensorManager.SENSOR_DELAY_UI, -1, true);
      else if(R.id.delay_button_game == view.getId())
        StreamingActivity.this.setAllListeners(sensorManager, SensorManager.SENSOR_DELAY_GAME, -1, true);
      else if(R.id.delay_button_fastest == view.getId())
        StreamingActivity.this.setAllListeners(sensorManager, SensorManager.SENSOR_DELAY_FASTEST, -1, true);
      else if(R.id.delay_button_submit == view.getId()){
        EditText sampleField = (EditText) activeDialog.findViewById(R.id.delay_field_sample);
        EditText reportField = (EditText) activeDialog.findViewById(R.id.delay_field_report);
        int batchRate;

        try{
          batchRate = Integer.parseInt(reportField.getText().toString());
        } catch(NumberFormatException e){
          batchRate = -1;
        }

        try{
          int rate = Integer.parseInt(sampleField.getText().toString());
          StreamingActivity.this.setAllListeners(sensorManager, rate, batchRate, true);
        } catch(NumberFormatException e){
          Toast.makeText(TabControl.getContext(), "Invalid number entry", Toast.LENGTH_LONG).show();
        }
      }
      else if(R.id.delay_button_cancel == view.getId()){ }
      else if(R.id.delay_button_flush == view.getId()) {
        StreamingActivity.this.flushAllListeners(this.sensorManager);
      }
      StreamingActivity.this.activeDialog.cancel();
    }
  }
}
