/*============================================================================
@file SettingsDatabase.java

@brief
Interface to access the sqlite database used by this application.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qualcomm.sensors.qsensortest;

import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.hardware.Sensor;
import android.util.Log;

public class SettingsDatabase extends Object {
  // Database version must be incremented for each change
  static private final int DATABASE_VERSION = 10;
  // Sensors Table
  static private final String TABLE_SENSOR = "Sensor";
  static private final String KEY_SENSOR_NAME = "sensor_name";
  static private final String KEY_ENABLE_STREAM = "enable_streaming";
  static private final String KEY_ENABLE_TEST = "enable_self_test";
  static private final String KEY_ENABLE_CAL = "enable_user_cal";
  static private final String KEY_ENABLE_INFO = "enable_info";
  static private final String KEY_ENABLE_THRESH = "enable_thresh";
  static private final String KEY_TRIGGER_MODE = "trigger_mode";
  static private final String KEY_EVENT_BEEP = "event_beep";
  static private final String KEY_SHOW_ACC = "show_acc";
  // Fragments table
  static private final String TABLE_FRAGMENT = "Fragment";
  static private final String KEY_FRAGMENT_NAME = "fragment_name";
  static private final String KEY_FRAGMENT_ENABLED = "fragment_enabled";
  // Menu Item table
  static private final String TABLE_MENU = "MenuItem";
  static private final String KEY_MENU_NAME = "menu_item_name";
  static private final String KEY_MENU_FRAGMENT = "menu_item_fragment";
  static private final String KEY_MENU_ENABLED = "menu_item_enabled";
  // Properties table
  static private final String TABLE_PROPERTY = "property";
  static private final String KEY_KEY = "key";
  static private final String KEY_VALUE = "value";
  // Database filename
  static private final String DATABASE_NAME = "qsensortest.db";

  /**
   * Array of key-value pairs to be stored in the database
   */
  static private final String[][] propertyList = new String[][]{
    new String[]{"selftest_applyCalNow", "1"},
    new String[]{"selftest_saveToRegistry", "1"},
    new String[]{"selftest_testType", "0"},
    new String[]{"stream_eff_rate_cnt", "5"},
    new String[]{"stream_log_name", ""},
    new String[]{"stream_log_buffer", "100"},
    new String[]{"logcat_logging_enabled", "1"},
    new String[]{"hold_wake_lock", "1"},
    new String[]{"stream_event_len", "4"},
    new String[]{"auto_retrigger", "1"},
    new String[]{"optimize_power_def", "0"},
    new String[]{"update_screen_def", "1"}
  };
  static private SettingsDatabase settings = null;
  static public SettingsDatabase getSettings(){
    if(null == SettingsDatabase.settings)
      SettingsDatabase.settings = new SettingsDatabase(TabControl.getContext());
    return SettingsDatabase.settings;
  }
  static public String[][] getDefaultPropertyList(){ return SettingsDatabase.propertyList; }

  private DatabaseOpenHelper dbOpenHelper;

  public SettingsDatabase(Context context){
    this.dbOpenHelper = new DatabaseOpenHelper(context);
  }

  /**
   * Looks-up the settings stored for a given sensor
   *
   * @param sensor Sensor to look-up.
   * @return SensorSetting object if found, null otherwise
   */
  public SensorSetting getSensorSetting(Sensor sensor){
    SensorSetting sensorSetting = null;

    // If a sensor cannot be found, perhaps the list of sensors has changed,
    // therefore try re-populating the database once
    for(int i = 0; i < 2; i++){
      Cursor resultCursor = dbOpenHelper.getReadableDatabase().query(
          TABLE_SENSOR,
          new String[]{KEY_SENSOR_NAME, KEY_ENABLE_STREAM,
              KEY_ENABLE_TEST, KEY_ENABLE_CAL,
              KEY_ENABLE_INFO, KEY_ENABLE_THRESH,
              KEY_TRIGGER_MODE, KEY_EVENT_BEEP, KEY_SHOW_ACC},
              KEY_SENSOR_NAME + " = '" + uniqueSensorID(sensor) + "'",
              new String[]{}, null, null, null);

      if(resultCursor.moveToFirst()){
        sensorSetting = new SensorSetting(
            resultCursor.getString(0),
            resultCursor.getString(1).contentEquals("true"),
            resultCursor.getString(2).contentEquals("true"),
            resultCursor.getString(3).contentEquals("true"),
            resultCursor.getString(4).contentEquals("true"),
            resultCursor.getString(5).contentEquals("true"),
            TriggerMode.values()[resultCursor.getInt(6)],
            resultCursor.getString(7).contentEquals("true"),
            resultCursor.getString(8).contentEquals("true"));
      }
      else if(0 == i) // If the sensor was not found in the database
        dbOpenHelper.populateSensorTable(dbOpenHelper.getWritableDatabase());

      resultCursor.close();
    }

    return sensorSetting;
  }

  /**
   * Determines if a specific fragment ought to be shown to the user.
   *
   * @param fragmentName Name of the fragment as per it's title's resource name in strings.xml
   * @return True if fragment is enabled, false otherwise.  Null if database info cannot be found.
   */
  public boolean getFragmentEnabled(String fragmentName){
    String output = null;
    Cursor resultCursor = dbOpenHelper.getReadableDatabase().query(
        TABLE_FRAGMENT, new String[]{KEY_FRAGMENT_ENABLED},
        KEY_FRAGMENT_NAME + " = '" + fragmentName + "'", null, null, null, null);

    if(resultCursor.moveToFirst()){
      output = resultCursor.getString(0);
    }

    resultCursor.close();
    return (null == output) ? null : output.contentEquals("true");
  }

  /**
   * Determines if a specific menu item ought to be functional/enabled.
   *
   * @param menuItemName Name of the menu item as per its id name in the menu xml file.
   * @return True if enabled, false otherwise.  Null if database info cannot be found.
   */
  public boolean getMenuItemEnabled(String menuItemName){
    String output = null;
    Cursor resultCursor = dbOpenHelper.getReadableDatabase().query(
        TABLE_MENU, new String[]{KEY_MENU_ENABLED},
        KEY_MENU_NAME + " = '" + menuItemName + "'", null, null, null, null);

    if(resultCursor.moveToFirst()){
      output = resultCursor.getString(0);
    }

    resultCursor.close();
    return (null == output) ? null : output.contentEquals("true");
  }

  /**
   * Looks-up a property value associated with the given key
   *
   * @param key String in DatabaseSettings.propertyList.
   * @return Value if key was found, null otherwise.
   */
  public String getProperty(String key){
    String output = null;
    Cursor resultCursor = dbOpenHelper.getReadableDatabase().query(
        TABLE_PROPERTY, new String[]{KEY_VALUE},
        KEY_KEY + " = '" + key + "'", null, null, null, null);

    if(resultCursor.moveToFirst()){
      output = resultCursor.getString(0);
    }

    resultCursor.close();
    return output;
  }

  /**
   * Updates a property (key-value pair) in the database.
   *
   * @throws SQLException
   */
  public void setProperty(String key, String value) throws SQLException {
    ContentValues cv = new ContentValues();
    cv.put(KEY_VALUE, value);
    dbOpenHelper.getWritableDatabase().update(
        TABLE_PROPERTY, cv, KEY_KEY + "=?", new String[]{key});
  }

  public enum TriggerMode {
    Continuous, OnChange, OneShot, Special;

    @SuppressWarnings("deprecation")
    public static TriggerMode getMode(Sensor sensor) {
      if(Sensor.TYPE_ACCELEROMETER == sensor.getType() ||
          Sensor.TYPE_AMBIENT_TEMPERATURE == sensor.getType() ||
          Sensor.TYPE_GRAVITY == sensor.getType() ||
          Sensor.TYPE_GYROSCOPE == sensor.getType() ||
          Sensor.TYPE_LINEAR_ACCELERATION == sensor.getType() ||
          Sensor.TYPE_MAGNETIC_FIELD == sensor.getType() ||
          Sensor.TYPE_ORIENTATION == sensor.getType() ||
          Sensor.TYPE_PRESSURE == sensor.getType() ||
          Sensor.TYPE_RELATIVE_HUMIDITY == sensor.getType() ||
          Sensor.TYPE_ROTATION_VECTOR == sensor.getType() ||
          Sensor.TYPE_TEMPERATURE == sensor.getType() ||
          Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED == sensor.getType() ||
          Sensor.TYPE_GAME_ROTATION_VECTOR == sensor.getType() ||
          Sensor.TYPE_GYROSCOPE_UNCALIBRATED == sensor.getType() ||
          20 == sensor.getType()) // GeoMag RV
        return TriggerMode.Continuous;
      else if(Sensor.TYPE_LIGHT == sensor.getType() ||
          Sensor.TYPE_PROXIMITY == sensor.getType() ||
          18 == sensor.getType() || 19 == sensor.getType())  // Step counter/detector)
        return TriggerMode.OnChange;
      else if(Sensor.TYPE_SIGNIFICANT_MOTION == sensor.getType() ||
              Sensor.TYPE_PICK_UP_GESTURE == sensor.getType())
        return TriggerMode.OneShot;
      else
        return TriggerMode.Continuous;
    }
  };

  /**
   * Helper class encapsulating all of the settings that may be applied
   * to each individual sensor.
   *
   */
  public class SensorSetting{
    private String sensorName;
    private boolean enableStreaming;
    private boolean enableSelfTest;
    private boolean enableUserCal;
    private boolean enableThresh;
    private boolean enableInfo;
    private boolean eventBeep;
    private boolean showAccuracy;
    private TriggerMode triggerMode;

    public SensorSetting(String sensorName, boolean enableStreaming,
        boolean enableSelfTest, boolean enableUserCal,
        boolean enableInfo, boolean enableThresh,
        TriggerMode triggerMode, boolean eventBeep,
        boolean showAccuracy){
      this.sensorName = sensorName;
      this.enableStreaming = enableStreaming;
      this.enableUserCal = enableUserCal;
      this.enableSelfTest = enableSelfTest;
      this.enableThresh = enableThresh;
      this.enableInfo = enableInfo;
      this.triggerMode = triggerMode;
      this.eventBeep = eventBeep;
      this.showAccuracy = showAccuracy;
    }

    public String getSensorName(){ return this.sensorName; }
    public boolean getEnableStreaming(){ return this.enableStreaming; }
    public boolean getEnableSelfTest(){ return this.enableSelfTest; }
    public boolean getEnableUserCal(){ return this.enableUserCal; }
    public boolean getEnableThresh(){ return this.enableThresh; }
    public boolean getEnableInfo(){ return this.enableInfo; }
    public boolean getEventBeep(){ return this.eventBeep; }
    public boolean getShowAccuracy(){ return this.showAccuracy; }
    public TriggerMode getTriggerMode(){ return this.triggerMode; }
  }

  /**
   * @return Whether the given sensor should be shown in the list of sensors in the self test tab.
   */
  private static boolean selfTestEnabled(Sensor sensor){
    if(sensor.getVendor().equals("Google Inc."))
      return false;

    switch(sensor.getType()){
    case Sensor.TYPE_ACCELEROMETER:
      return true;
    case Sensor.TYPE_GYROSCOPE:
      return true;
    case Sensor.TYPE_MAGNETIC_FIELD:
      return true;
    case Sensor.TYPE_LIGHT:
      return true;
    case Sensor.TYPE_PROXIMITY:
      return true;
    default:
      return false;
    }
  }

  @SuppressWarnings("deprecation")
  private static boolean showAccuracy(Sensor sensor){
    if(Sensor.TYPE_ACCELEROMETER == sensor.getType() ||
        Sensor.TYPE_MAGNETIC_FIELD == sensor.getType() ||
        Sensor.TYPE_ORIENTATION == sensor.getType() ||
        Sensor.TYPE_GYROSCOPE == sensor.getType())
      return true;

    return false;
  }

  /**
   * @return An unique identifier for the given sensor.
   *
   * Note: Potentially a problem if we have two of the same sensor attached to a device.
   */
  private static String uniqueSensorID(Sensor sensor){ return sensor.getName() + "." + sensor.getType(); }

  /**
   * Helper class which handles the creation and upgrade of the sqlite
   * database and tables.
   *
   */
  private class DatabaseOpenHelper extends SQLiteOpenHelper {
    private Resources resources;
    public DatabaseOpenHelper(Context context) {
      super(context, DATABASE_NAME, null, DATABASE_VERSION);
      this.resources = context.getResources();
      this.getWritableDatabase().close();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
      db.execSQL(
          "CREATE TABLE " + TABLE_PROPERTY + " (" +
              KEY_KEY + " TEXT, " +
              KEY_VALUE + " TEXT, " +
              "PRIMARY KEY('" + KEY_KEY + "'));"
          );
      db.execSQL(
          "CREATE TABLE " + TABLE_SENSOR + " (" +
              KEY_SENSOR_NAME + " TEXT, " +
              KEY_ENABLE_STREAM + " BOOLEAN, " +
              KEY_ENABLE_TEST + " BOOLEAN, " +
              KEY_ENABLE_CAL + " BOOLEAN, " +
              KEY_ENABLE_INFO + " BOOLEAN, " +
              KEY_ENABLE_THRESH + " BOOLEAN, " +
              KEY_TRIGGER_MODE + " INT, " +
              KEY_EVENT_BEEP + " BOOLEAN, " +
              KEY_SHOW_ACC + " BOOLEAN, " +
              "PRIMARY KEY('" + KEY_SENSOR_NAME + "'));"
          );
      db.execSQL(
          "CREATE TABLE " + TABLE_FRAGMENT + " (" +
              KEY_FRAGMENT_NAME + " TEXT, " +
              KEY_FRAGMENT_ENABLED + " BOOLEAN, " +
              "PRIMARY KEY('" + KEY_FRAGMENT_NAME + "'));"
          );
      db.execSQL(
          "CREATE TABLE " + TABLE_MENU + " (" +
              KEY_MENU_NAME + " TEXT, " +
              KEY_MENU_FRAGMENT + " TEXT, " +
              KEY_MENU_ENABLED + " BOOLEAN, " +
              "PRIMARY KEY('" + KEY_MENU_NAME + "')," +
              "FOREIGN KEY('" + KEY_MENU_FRAGMENT + "') REFERENCES " +
              TABLE_FRAGMENT + "(" + KEY_FRAGMENT_NAME + ") ON DELETE CASCADE);"
          );

      populateSensorTable(db);
      populateFragmentTable(db);
      populateMenuTable(db);
      populatePropertyTable(db);
    }

    public void populateSensorTable(SQLiteDatabase db){
      List<Sensor> sensorList = TabControl.sensorManager.getSensorList(Sensor.TYPE_ALL);
      for(Sensor sensor : sensorList){
        boolean enableStreaming = true;
        boolean enableSelfTest = selfTestEnabled(sensor);
        boolean enableUserCal = Sensor.TYPE_ACCELEROMETER == sensor.getType() ? true : false;
        boolean enableInfo = true;
        boolean eventBeep = (TriggerMode.getMode(sensor) != TriggerMode.Continuous);
        boolean enableThresh = !sensor.getVendor().contentEquals("Qualcomm") &&
            !sensor.getVendor().contentEquals("Google Inc.");
        boolean showAccuracy = SettingsDatabase.showAccuracy(sensor);
        TriggerMode triggerMode = TriggerMode.getMode(sensor);

        db.execSQL(
            "INSERT OR IGNORE INTO `" + TABLE_SENSOR + "` (`" +
                KEY_SENSOR_NAME + "` , `" + KEY_ENABLE_STREAM + "` , `" +
                KEY_ENABLE_TEST + "` , `" + KEY_ENABLE_CAL + "` , `" +
                KEY_ENABLE_INFO + "` , `" + KEY_ENABLE_THRESH + "` , `" +
                KEY_TRIGGER_MODE + "` , `" + KEY_EVENT_BEEP + "` , `" +
                KEY_SHOW_ACC + "`) VALUES " +
                "('" + uniqueSensorID(sensor) + "' , '" + enableStreaming + "', '" +
                enableSelfTest + "', '" + enableUserCal + "', '" + enableInfo + "', '" +
                enableThresh + "', '" + triggerMode.ordinal() + "', '" + eventBeep + "', '"
                + showAccuracy + "')"
            );
      }
    }

    private void populateFragmentTable(SQLiteDatabase db){
      String[][] fragments = new String[][]{
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream), "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_cal), "0"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_test), "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_registry), "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_info), "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_thresh), "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_graph), "1"}
      };
      for(String[] fragment : fragments)
        db.execSQL(
            "INSERT OR IGNORE INTO `" + TABLE_FRAGMENT + "` (`" + KEY_FRAGMENT_NAME +
            "` , `" + KEY_FRAGMENT_ENABLED + "`) VALUES " +
            "('" + fragment[0] + "' , '" + (fragment[1] == "1") + "')"
            );
    }

    private void populatePropertyTable(SQLiteDatabase db){
      for(String[] property : propertyList)
        db.execSQL(
            "INSERT OR IGNORE INTO `" + TABLE_PROPERTY + "` (`" + KEY_KEY +
            "` , `" + KEY_VALUE + "`) VALUES " +
            "('" + property[0] + "' , '" + property[1] + "')"
            );
    }

    private void populateMenuTable(SQLiteDatabase db){
      String[][] menuItems = new String[][]{
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.all_listeners_add),
              "1"},
              new String[]{"qsensortest",
              this.resources.getResourceEntryName(R.id.exit),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.all_listeners_remove),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.raw_data_mode_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.ui_data_update_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.wake_lock_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.retrigger_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.suspend_monitor_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.all_listeners_flush),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.beep_toggle),
          "1"},
          new String[]{this.resources.getResourceEntryName(R.string.fragment_name_stream),
              this.resources.getResourceEntryName(R.id.optimize_power),
          "1"}
      };
      for(String[] menuItem : menuItems)
        db.execSQL(
            "INSERT OR IGNORE INTO `" + TABLE_MENU + "` (`" + KEY_MENU_FRAGMENT +
            "` , `" + KEY_MENU_NAME + "` , `" + KEY_MENU_ENABLED + "`) VALUES " +
            "('" + menuItem[0] +  "' , '" + menuItem[1] + "' , '" + (menuItem[2] == "1") + "')"
            );
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      Log.w(TabControl.TAG, "Upgrading database from version "
          + oldVersion + " to " + newVersion);
      db.execSQL("DROP TABLE IF EXISTS " + TABLE_SENSOR);
      db.execSQL("DROP TABLE IF EXISTS " + TABLE_PROPERTY);
      db.execSQL("DROP TABLE IF EXISTS " + TABLE_FRAGMENT);
      db.execSQL("DROP TABLE IF EXISTS " + TABLE_MENU);
      onCreate(db);
    }
  }
}
