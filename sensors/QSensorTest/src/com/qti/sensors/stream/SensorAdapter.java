/*============================================================================
@file QSensorAdapter.java

@brief
Wrapper API for View and Controller to access or modify the QSensor Model.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.stream;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.NoSuchElementException;

import android.annotation.SuppressLint;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.qsensortest.SettingsDatabase.SensorSetting;
import com.qualcomm.sensors.qsensortest.TabControl;
import com.qualcomm.sensors.sensortest.SensorTest;

public class SensorAdapter extends QSensor {
  static protected String streamLogName;
  static protected BufferedWriter logFileWriter;
  static protected HandlerThread handlerThread;
  static protected boolean logcatLoggingEnabled;
  static protected ToneGenerator toneGenerator;
  static {
    SensorAdapter.streamLogName = SettingsDatabase.getSettings().getProperty("stream_log_name");
    if(!SensorAdapter.streamLogName.equals("")){
      int logFileBufferSize = Integer.parseInt(SettingsDatabase.getSettings().getProperty("stream_log_buffer"));
      try{
        FileWriter fstream =
            new FileWriter(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS ) +
                "/" + SensorAdapter.streamLogName, false);
        SensorAdapter.logFileWriter = new BufferedWriter(fstream, logFileBufferSize);
      } catch (IOException e) { e.printStackTrace(); }
    }

    SensorAdapter.logcatLoggingEnabled = SettingsDatabase.getSettings().getProperty("logcat_logging_enabled").equals("1");
    SensorAdapter.toneGenerator = new ToneGenerator(AudioManager.STREAM_NOTIFICATION, 100);

    SensorAdapter.handlerThread = new HandlerThread("sensorThread");
    SensorAdapter.handlerThread.start();
  }

  protected SensorListener sensorListener;
  protected final SensorManager sensorManager;
  protected final SensorSetting databaseSettings;
  protected final SensorSetting sensorSetting;

  public SensorAdapter(Sensor sensor, SensorManager sensorManager) {
    super(sensor);
    this.sensorListener = null;
    this.sensorManager = sensorManager;
    this.sensorSetting = SettingsDatabase.getSettings().getSensorSetting(this.sensor());

    this.databaseSettings = SettingsDatabase.getSettings().getSensorSetting(this.sensor());
  }

  public synchronized void eventIs(SensorSample sensorSample) {
    Long timestamp = (long) 0;
    String logInfo = "";

    if(this.sensorSetting.getEventBeep() && StreamingActivity.beepEnabled)
      SensorAdapter.toneGenerator.startTone(ToneGenerator.TONE_PROP_BEEP);

    try{
      timestamp = this.sensorEvents().getFirst().timestamp();
    } catch(NoSuchElementException e){}

    if(sensorSample.timestamp() < timestamp)
      Log.e(StreamingActivity.TAG, "Log misorder: " + SensorAdapter.sensorTypeString(this.sensor()) +
          " (" + timestamp + ","+ sensorSample.timestamp() + ")");

    if(0 != QSensor.effectiveRateCount || 0 != QSensor.eventLength)
      this.sensorEventIs(sensorSample);

    if(SensorAdapter.logcatLoggingEnabled || SensorAdapter.logFileExists())
      logInfo = this.sensorLogInfo(sensorSample);

    if(SensorAdapter.logcatLoggingEnabled)
      Log.d(StreamingActivity.TAG, logInfo);

    if(SensorAdapter.logFileExists())
      SensorAdapter.writeToLogFile(logInfo + "\n");
  }

  public synchronized void streamRateIs(int streamRate, int reportRate, boolean clear) {
    if(streamRate == this.streamRate() && reportRate == this.reportRate())
      return ; // No need to change anything

    this.sensorManager.unregisterListener(this.sensorListener, this.sensor());
    this.sensorListener = null;

    if(-1 != streamRate) {
      Handler handler = new Handler(SensorAdapter.handlerThread.getLooper());

      if(-1 == reportRate) {
        this.sensorListener = new SensorListener(this);
        if(!sensorManager.registerListener(this.sensorListener,
            this.sensor(), streamRate < 4 ? streamRate : streamRate * 1000, handler)) {
          streamRate = -1;
          Toast.makeText(TabControl.getContext(),
              "Unable to register new listener", Toast.LENGTH_LONG).show();
        }
      }
      else {
        this.sensorListener = new SensorFlushListener(this);
        if(!sensorManager.registerListener(this.sensorListener, this.sensor(),
            streamRate < 4 ? streamRate : streamRate * 1000, reportRate * 1000, handler)) {
          streamRate = -1;
          reportRate = -1;
          Toast.makeText(TabControl.getContext(),
            "Unable to register new listener", Toast.LENGTH_LONG).show();
        }
      }
    }

    // If we just enabled or changed a sensor
    if(-1 != streamRate && clear)
      this.sensorEventsClear();

    Log.d(StreamingActivity.TAG, "Change sensor rate for " + this.sensor().getType() +
        " from " + this.streamRate() + " (" + this.reportRate() + ") to " +
        streamRate + " (" + reportRate + ")");

    this.streamRateIs(streamRate);
    this.reportRateIs(reportRate);
  }

  @Override
  protected synchronized void accuracyIs(int accuracy) {
    if(SensorAdapter.logcatLoggingEnabled)
      Log.d(StreamingActivity.TAG, "Received accuracy update.  Sensor: " +
            this.sensor().getName() + " accuracy: " + accuracy);

    super.accuracyIs(accuracy);
  }

  /**
   * Calls flush on the sensor listener
   */
  protected void flush(SensorManager sensorManager) {
    if(null == this.sensorListener)
      Toast.makeText(TabControl.getContext(), "Cannot flush inactive sensor:\n" + this.sensor().getName(),
          Toast.LENGTH_LONG).show();
    else
      sensorManager.flush(this.sensorListener);
  }

  /**
   * @return The title of the sensor with form "SensorType: SensorName" or "SensorType: Vendor"
   */
  public String title() {
     return SensorAdapter.sensorTypeString(this.sensor()) + ": " + this.sensor().getName() + ": " + this.sensor().getVendor();
  }

  /**
   * @return String containing all of the attributes of the given sensor, as
   * contained within the Sensor class.
   */
  @SuppressLint("NewApi")
  public String sensorAttributes() {
    float   maximumRange;
    float   power;
    float   resolution;
    String  vendor;
    int     version;
    int     minDelay;
    String  stringtype;
    int     maxDelay;
    String  permissions;
    boolean bwakeup;

    maximumRange = this.sensor().getMaximumRange();  /* maximum range of the sensor in the sensor's unit */
    power = this.sensor().getPower();            /* the power in mA used by this sensor while in use */
    resolution = this.sensor().getResolution();  /* resolution of the sensor in the sensor's unit */
    vendor = this.sensor().getVendor();          /* vendor string of this sensor */
    version = this.sensor().getVersion();        /* version of the sensor's module */
    minDelay = this.sensor().getMinDelay();
    maxDelay = this.sensor().getMaxDelay();
    permissions = this.sensor().getRequiredPermission();
    stringtype  = this.sensor().getStringType();
    bwakeup = this.sensor().isWakeUpSensor();

    String output = this.sensor().getType() + ": " +
        this.sensor().getName() + "\n" +
        "Max Range: " + Float.toString(maximumRange) + "\n" +
        "Power: " + Float.toString(power) + "mA\n" +
        "Resolution: " + Float.toString(resolution) + "\n" +
        "Vendor: " + vendor + "\n" +
        "Version: " + Integer.toString(version) + "\n" +
        "Minimum Delay: " + Integer.toString(minDelay) + "\n" +
        "Maximum Dealy: " + Integer.toString(maxDelay) + "\n" +
        "String type: " + stringtype + "\n" +
        "Wake Up: " + bwakeup + "\n" +
        "Permission: " + permissions + "\n";

    if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT)
      output +=    "Max fifo event count: " + this.sensor().getFifoMaxEventCount() + "\n" +
          "Fifo reserved event count: " + this.sensor().getFifoReservedEventCount();

    return output;
  }

  @SuppressLint("SimpleDateFormat")
  public synchronized String timestamp() {
    Long timestamp = (long) 0;

    try{
      timestamp = this.sensorEvents().getFirst().timestamp();
    } catch(NoSuchElementException e){}

    return "ts: " + Long.toString(timestamp) + " ~ " +
    new SimpleDateFormat("HH:mm:ss.SSS").format( // "yyyy-MM-dd HH:mm:ss.SSS"
        new Date(timestamp / 1000000));
  }

  public synchronized String accuracyString() {
    int accuracy = 0;
    String outputString;

    try{
      accuracy = this.sensorEvents().getFirst().accuracy();
    } catch(NoSuchElementException e){}

    outputString = "Acc:" +
        SensorAdapter.sensorAccuracy(this.accuracy()) +
        "(" + Integer.toString(accuracy) + ")";

    return this.databaseSettings.getShowAccuracy()
        ? outputString : "";
  }

  static private final String[] logIndices = {"[0]:", "[1]:", "[2]:", "[3]:", "[4]:", "[5]:",
    "[6]:", "[7]:", "[8]:", "[9]:", "[10]:", "[11]:", "[12]:", "[13]:", "[14]:", "[15]:" };
  /**
   * @return A string containing the contents of the last update.
   */
  public synchronized String[] sensorData() {
    String outputString[] = {"", ""};
    float data[] = {(float)0.0,(float)0.0,(float)0.0};

    try{
      data = this.sensorEvents().getFirst().values();
    } catch(NoSuchElementException e){}

    for(int i = 0; i < data.length && i < 6; i++ ) {
      outputString[i / 3] += "[" + i + "]:" + Float.toString(data[i]) + "\n";
    }

    for(int i = 0; i < outputString.length && outputString[i].length() > 0; i++)
      outputString[i] = outputString[i].substring(0, outputString[i].length()-1);

    return outputString;
  }

  /**
   * @return String to be used in a logcat entry; contents specified by I&T
   */
  protected String sensorLogInfo(SensorSample sensorSample) {
    StringBuilder output = new StringBuilder(50);
    output.append(SensorAdapter.sensorTypeString(this.sensor()));
    output.append(":");

    for(int i = 0; i < sensorSample.values().length; i++ ) {
      output.append(SensorAdapter.logIndices[i]);
      output.append(sensorSample.values()[i]);
      output.append(",");
    }
    output.append("acc:");
    output.append(sensorSample.accuracy());
    output.append(",");
    output.append("ts:");
    output.append(sensorSample.timestamp());

    return output.toString();
  }

  /**
   * @return A string containing the sample rate of the sensor stream
   */
  public synchronized String streamRateString() {
    String outputString = "";

    outputString += "Rate: " + SensorAdapter.streamRateString(this.streamRate());
    outputString += "(" + new DecimalFormat("#.##").format(this.effectiveStreamRate()) + ")";

    return outputString;
  }

  /**
   * @return A string containing the batch rate of the sensor stream
   */
  public synchronized String reportRateString() {
    String outputString = "";

    outputString += "Batch: " + SensorAdapter.streamRateString(this.reportRate());

    return outputString;
  }

  /**
   * @return The effective stream rate for the given sensor.  0 if no
   *         data exists.
   */
  protected Double effectiveStreamRate() {
    if(0 == this.sensorEvents().size() || 1 == this.sensorEvents().size())
      return 0.0;
    else
      return 1000 / ((double)(this.sensorEvents().getFirst().rcvTime().getTime() -
          this.sensorEvents().getLast().rcvTime().getTime()) /
          (this.sensorEvents().size() - 1));
  }

  public synchronized long lastTimestamp() {
    SensorSample first = this.sensorEvents().getFirst();
    return null == first ? 0 : first.timestamp();
  }

  /**
   * Returns String value for the sensor type
   *
   * @param type the sensor type
   * @return String String value for the sensor type
   */
  @SuppressWarnings("deprecation")
  static public String sensorTypeString(Sensor sensor)
  {
    String  sensorTypeString;
    switch (sensor.getType())
    {
    case Sensor.TYPE_ACCELEROMETER:
      sensorTypeString = "ACCEL";
      break;
    case Sensor.TYPE_GYROSCOPE:
      sensorTypeString = "GYRO";
      break;
    case Sensor.TYPE_LIGHT:
      sensorTypeString = "LIGHT";
      break;
    case Sensor.TYPE_MAGNETIC_FIELD:
      sensorTypeString = "MAG";
      break;
    case Sensor.TYPE_ORIENTATION:
      sensorTypeString = "ORI";
      break;
    case Sensor.TYPE_PRESSURE:
      sensorTypeString = "PRESS";
      break;
    case Sensor.TYPE_PROXIMITY:
      sensorTypeString = "PROX";
      break;
    case Sensor.TYPE_TEMPERATURE:
      sensorTypeString = "TEMP";
      break;
    case Sensor.TYPE_LINEAR_ACCELERATION:
      sensorTypeString = "L.ACCEL";
      break;
    case Sensor.TYPE_GRAVITY:
      sensorTypeString = "GRAVITY";
      break;
    case Sensor.TYPE_ROTATION_VECTOR:
      sensorTypeString = "ROT VEC";
      break;
    case Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED:
      sensorTypeString = "UNCAL MAG";
      break;
    case Sensor.TYPE_GYROSCOPE_UNCALIBRATED:
      sensorTypeString = "UNCAL GYRO";
      break;
    case Sensor.TYPE_SIGNIFICANT_MOTION:
      sensorTypeString = "SIG MOT";
      break;
    case Sensor.TYPE_STEP_COUNTER:
      sensorTypeString = "S COUNT";
      break;
    case Sensor.TYPE_STEP_DETECTOR:
      sensorTypeString = "S DETECT";
      break;
    case Sensor.TYPE_GEOMAGNETIC_ROTATION_VECTOR:
      sensorTypeString = "GM RV";
      break;
    case Sensor.TYPE_GAME_ROTATION_VECTOR:
      sensorTypeString = "GAME RV";
      break;
    default:
      //sensorTypeString = "UNKNOWN";
      sensorTypeString = sensor.getName();
      break;
    }

    return sensorTypeString;
  }

  /**
   * @return A string representing what the current stream rate means in the Android framework
   */
  static public String streamRateString(int streamRate){
    if(streamRate == -1)
      return "Off";
    else if(streamRate == SensorManager.SENSOR_DELAY_FASTEST)
      return "Fastest";
    else if(streamRate == SensorManager.SENSOR_DELAY_GAME)
      return "Game";
    else if(streamRate == SensorManager.SENSOR_DELAY_NORMAL)
      return "Normal";
    else if(streamRate== SensorManager.SENSOR_DELAY_UI)
      return "UI";
    else
      return String.valueOf(streamRate);
  }

  static public boolean logFileExists() {
    return !SensorAdapter.streamLogName.equals("");
  }

  /**
   * @param text String to be written into the log file.
   *
   * Log file name is determined by database property, and will be located
   * within the device's downloads folder.
   */
  static synchronized public void writeToLogFile(String text){
    if(SensorAdapter.logFileExists()){
      try {
        SensorAdapter.logFileWriter.write(text);
      } catch (IOException e) { e.printStackTrace(); }
    }
  }

  /**
   * Returns String value for the rate value
   *
   * @param rate The sensor rate
   * @return String value for the rate
   */
  static public String sensorAccuracy(int rate)
  {
    String  sensorAccuracyString;
    switch (rate)
    {
    case SensorManager.SENSOR_STATUS_UNRELIABLE:
      sensorAccuracyString = "UNR";
      break;
    case SensorManager.SENSOR_STATUS_ACCURACY_LOW:
      sensorAccuracyString = "LOW";
      break;
    case SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM:
      sensorAccuracyString = "MED";
      break;
    case SensorManager.SENSOR_STATUS_ACCURACY_HIGH:
      sensorAccuracyString = "HI";
      break;
    default:
      sensorAccuracyString = "UNK";
      break;
    }
    return sensorAccuracyString;
  }

  /**
   * Most sensor types simply use the PRIMARY data-type.  But for those that don't,
   * we must perform the translation here.
   *
   * @param sensor The sensor we wish to translate
   * @return The data-type to request
   */
  static public SensorTest.DataType sensorDataType(Sensor sensor){
    if(Sensor.TYPE_PROXIMITY == sensor.getType())
      return SensorTest.DataType.PRIMARY;
    else if(Sensor.TYPE_LIGHT == sensor.getType())
      return SensorTest.DataType.SECONDARY;
    else
      return SensorTest.DataType.PRIMARY;
  }
}
