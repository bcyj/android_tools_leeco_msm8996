/*============================================================================
@file TabControl.java

@brief
Main launch point for the QSensorTest application.  Manages all tabs.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.qsensortest;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.database.SQLException;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.PowerManager;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.widget.Toast;

import com.qti.sensors.attributes.SensorAttributesActivity;
import com.qti.sensors.selftest.SelfTestActivity;
import com.qti.sensors.snsregistry.SNSRegistryActivity;
import com.qti.sensors.stream.StreamingActivity;
import com.qti.sensors.threshold.ThresholdActivity;
import com.qti.sensors.usercal.UserCalActivity;
import com.qualcomm.sensors.sensortest.SensorTest;
import com.qualcomm.sensors.sensortest.SensorThresh;
import com.qualcomm.sensors.sensortest.SensorsReg;

public class TabControl extends Activity {
  static final public String TAG = "QSensorTest";
  static private Context context;
  static private Activity activity;
  static private PowerManager.WakeLock wakeLock = null;
  static public SensorManager sensorManager;
  static public boolean optimizePower = false;
  static public Context getContext() { return TabControl.context; }
  static public Activity getActivity() { return TabControl.activity; }

  private SuspendMonitor suspendMonitor = null;
  private CommandReceiver commandReceiver;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    // Hack to work around TabControl issue during monkey testing
    try {Thread.sleep(1000);} catch (InterruptedException e) {e.printStackTrace();}

    super.onCreate(savedInstanceState);
    TabControl.sensorManager = (SensorManager)this.getSystemService(Context.SENSOR_SERVICE);
    TabControl.context = this;
    TabControl.activity = this;

    String optimizePower = SettingsDatabase.getSettings().getProperty("optimize_power_def");
    TabControl.optimizePower = null != optimizePower && optimizePower.equals("1");

    this.requestWindowFeature(Window.FEATURE_ACTION_BAR);
    ActionBar actionBar = getActionBar();
    actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
    actionBar.setDisplayShowTitleEnabled(true);

    Tab tab = actionBar.newTab()
        .setText(R.string.fragment_name_stream)
        .setTabListener(new TabListener<StreamingActivity>(
            this, "streaming", StreamingActivity.class));
    if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_stream)))
      actionBar.addTab(tab);

    /*tab = actionBar.newTab()
            .setText(R.string.fragment_name_graph)
            .setTabListener(new TabListener<SensorGraphActivity>(
                    this, "graph", SensorGraphActivity.class));
      if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_graph)))
          actionBar.addTab(tab);*/

    tab = actionBar.newTab()
        .setText(R.string.fragment_name_cal)
        .setTabListener(new TabListener<UserCalActivity>(
            this, "user_cal", UserCalActivity.class));
    if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_cal)))
      actionBar.addTab(tab);

    tab = actionBar.newTab()
        .setText(R.string.fragment_name_test)
        .setTabListener(new TabListener<SelfTestActivity>(
            this, "self_test", SelfTestActivity.class));
    try {
      new SensorTest();
      if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_test)))
        actionBar.addTab(tab);
    } catch(LinkageError e) {
      Toast.makeText(this, "Self Test tab disabled", Toast.LENGTH_LONG).show();
    }

    tab = actionBar.newTab()
        .setText(R.string.fragment_name_registry)
        .setTabListener(new TabListener<SNSRegistryActivity>(
            this, "registry", SNSRegistryActivity.class));
    try {
      new SensorsReg();
      if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_registry)))
        actionBar.addTab(tab);
    } catch(LinkageError e) {
      Toast.makeText(this, "Registry tab disabled", Toast.LENGTH_LONG).show();
    }

    tab = actionBar.newTab()
        .setText(R.string.fragment_name_thresh)
        .setTabListener(new TabListener<ThresholdActivity>(
            this, "thresh", ThresholdActivity.class));
    try {
      new SensorThresh();
      if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_thresh)))
        actionBar.addTab(tab);
    } catch(LinkageError e) {
      Toast.makeText(this, "Threshold tab disabled", Toast.LENGTH_LONG).show();
    }

    tab = actionBar.newTab()
        .setText(R.string.fragment_name_info)
        .setTabListener(new TabListener<SensorAttributesActivity>(
            this, "info", SensorAttributesActivity.class));
    if(SettingsDatabase.getSettings().getFragmentEnabled(this.getResources().getResourceEntryName(R.string.fragment_name_info)))
      actionBar.addTab(tab);

    if(null == this.suspendMonitor )
      this.suspendMonitor = new SuspendMonitor();

    if(null == TabControl.wakeLock) {
      PowerManager pm = (PowerManager)TabControl.getContext().getSystemService(Context.POWER_SERVICE);
      TabControl.wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE, "qsensortest_wl");

      String holdWL = SettingsDatabase.getSettings().getProperty("hold_wake_lock");
      if(null != holdWL && holdWL.equals("1"))
        TabControl.wakeLock.acquire();
    }
  // Hack to work around TabControl issue during monkey testing
  try {Thread.sleep(1000);} catch (InterruptedException e) {e.printStackTrace();}
  }

  /**
   * Load the options menu (defined in xml)
   *
   * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
   */
  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    MenuInflater inflater = this.getMenuInflater();
    inflater.inflate(R.menu.qsensortest_menu, menu);

    MenuItem wlMenuItem = menu.findItem(R.id.wake_lock_toggle);
    String holdWL = SettingsDatabase.getSettings().getProperty("hold_wake_lock");
    wlMenuItem.setChecked(null != holdWL && holdWL.equals("1"));

    MenuItem smMenuItem = menu.findItem(R.id.suspend_monitor_toggle);
    smMenuItem.setChecked(this.suspendMonitor.active());

    MenuItem opMenuItem = menu.findItem(R.id.optimize_power);
    opMenuItem.setChecked(TabControl.optimizePower );

    return super.onCreateOptionsMenu(menu);
  }

  /**
   * Defines what occurs when the user selects one of the menu options.
   *
   * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
   */
  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
    case R.id.exit:
      boolean closeEnabled = SettingsDatabase.getSettings().getMenuItemEnabled(
          TabControl.context.getResources().getResourceEntryName(R.id.exit));
      if(closeEnabled){
        this.finish();
      }
      return true;
    case R.id.suspend_monitor_toggle:
      boolean monitorEnabled = SettingsDatabase.getSettings().getMenuItemEnabled(
          TabControl.context.getResources().getResourceEntryName(R.id.suspend_monitor_toggle));
      if(monitorEnabled){
        if(this.suspendMonitor.active())
          this.suspendMonitor.stop();
        else
          this.suspendMonitor.start();
        item.setChecked(this.suspendMonitor.active());
      }
      return true;
    case R.id.wake_lock_toggle:
      boolean wlEnabled = SettingsDatabase.getSettings().getMenuItemEnabled(
          TabControl.getContext().getResources().getResourceEntryName(R.id.wake_lock_toggle));
      if(wlEnabled)
      {
        try{
          SettingsDatabase.getSettings().setProperty(
              "hold_wake_lock", item.isChecked() ? "0" : "1");
          item.setChecked(!item.isChecked());
        } catch(SQLException e) {
          Toast.makeText(TabControl.getActivity(), "Unable to change setting", Toast.LENGTH_LONG).show();
        }

        if(item.isChecked())
          TabControl.wakeLock.acquire();
        else
          TabControl.wakeLock.release();
      }
      return true;
    case R.id.optimize_power:
      boolean menuItemEnabled = SettingsDatabase.getSettings().getMenuItemEnabled(
          TabControl.getContext().getResources().getResourceEntryName(R.id.optimize_power));
      if(menuItemEnabled) {
        TabControl.optimizePower = !TabControl.optimizePower;
        item.setChecked(TabControl.optimizePower);
      }
      return true;
    default:
      return super.onOptionsItemSelected(item);
    }
  }

  @Override
  public void onResume() {
    super.onResume();
    if(null == this.commandReceiver)
      this.commandReceiver = new CommandReceiver();
    TabControl.getContext().registerReceiver(this.commandReceiver, this.commandReceiver.getIntentFilter());
  }

  @Override
  public void onPause() {
    super.onPause();
    if(null != this.commandReceiver)
      TabControl.getContext().unregisterReceiver(this.commandReceiver);
  }

  private static class TabListener<T extends Fragment> implements ActionBar.TabListener {
    private Fragment fragment;
    private final Activity activity;
    private final String tag;
    private final Class<T> fragmentClass;

    /** Constructor used each time a new tab is created.
     *
     * @param activity  The host Activity, used to instantiate the fragment
     * @param tag  The identifier tag for the fragment
     * @param clz  The fragment's Class, used to instantiate the fragment
     */
    public TabListener(Activity activity, String tag, Class<T> fragmentClass) {
      this.activity = activity;
      this.tag = tag;
      this.fragmentClass = fragmentClass;
    }
    @Override
    public void onTabSelected(Tab tab, FragmentTransaction ft) {
      if(null == this.fragment) {
        this.fragment = Fragment.instantiate(this.activity, this.fragmentClass.getName());
        ft.add(android.R.id.content, this.fragment, this.tag);
      }
      else
        ft.attach(this.fragment);
    }
    @Override
    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
      if (this.fragment != null)
        ft.detach(this.fragment);
    }
    @Override
    public void onTabReselected(Tab tab, FragmentTransaction ft) {}
  }
}
