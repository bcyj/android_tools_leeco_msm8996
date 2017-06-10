/*============================================================================
@file QSensor.java

@brief
Manages all sensor data and current listener parameters.

Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.stream;

import java.util.Date;
import java.util.LinkedList;

import android.annotation.SuppressLint;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.TriggerEvent;

import com.qualcomm.sensors.qsensortest.SettingsDatabase;

public class QSensor {
  static protected int effectiveRateCount;
  static protected int eventLength;

  static{
    QSensor.effectiveRateCount = Integer.parseInt(SettingsDatabase.getSettings().getProperty("stream_eff_rate_cnt"));
    QSensor.eventLength = Integer.parseInt(SettingsDatabase.getSettings().getProperty("stream_event_len"));
  }

  private Sensor sensor;  // Associated Android sensor
  private LinkedList<SensorSample> sensorEvents;
  private int streamRate; // Streaming rate requested by the user
  private int reportRate; // Report (aka batch) rate requested by the user
  private int accuracy;   // Accuracy values received by onAccuracyChanged callback

  public QSensor(Sensor sensor) {
    super();
    this.sensor = sensor;
    this.sensorEvents = new LinkedList<SensorSample>();
    this.accuracy = -1;
    this.streamRateIs(-1);
    this.reportRateIs(-1);
    this.sensorEventsClear();
  }

  public Sensor sensor() { return this.sensor; }
  protected int streamRate() { return this.streamRate; }
  protected int reportRate() { return this.reportRate; }
  protected int accuracy() { return this.accuracy; }
  protected LinkedList<SensorSample> sensorEvents() { return this.sensorEvents; }

  protected void reportRateIs(int reportRate) { this.reportRate = reportRate; }
  protected void streamRateIs(int streamRate) { this.streamRate = streamRate; }
  protected void accuracyIs(int accuracy) { this.accuracy = accuracy; }
  protected void sensorEventIs(SensorSample sensorEvent) {
    this.sensorEvents.addFirst(sensorEvent);

    if(this.sensorEvents().size() > QSensor.effectiveRateCount &&
       this.sensorEvents().size() > QSensor.eventLength)
      this.sensorEvents.removeLast();
  }
  protected void sensorEventsClear() {
    this.sensorEvents.clear();

    for(int i = 0; i < QSensor.eventLength || i < QSensor.effectiveRateCount; i++)
      this.sensorEventIs(new SensorSample(sensor));
  }

  /**
   * Since SensorEvent was poorly written by Android, this class acts as
   * a proxy.
   */
  protected class SensorSample {
    private final float[] values;
    private Sensor sensor;
    private int accuracy;
    private long timestamp;
    private Date rcvTime;

    public SensorSample(SensorEvent sensorEvent) {
      this.rcvTime = new Date();
      this.values = sensorEvent.values.clone();
      this.sensor = sensorEvent.sensor;
      this.accuracy = sensorEvent.accuracy == 0 ?
          QSensor.this.accuracy() :
            sensorEvent.accuracy;
          this.timestamp = sensorEvent.timestamp;
    }

    @SuppressLint("NewApi")
    public SensorSample(TriggerEvent triggerEvent) {
      this.rcvTime = new Date();
      this.values = triggerEvent.values.clone();
      this.sensor = triggerEvent.sensor;
      this.timestamp = triggerEvent.timestamp;
    }

    public SensorSample(Sensor sensor) {
      this.rcvTime = new Date();
      this.values = new float[3];
      this.values[0] = 0;
      this.values[1] = 0;
      this.values[2] = 0;
      this.sensor = sensor;
      this.accuracy = -1;
      this.timestamp = 0;
    }

    public float[] values() { return this.values; }
    public Sensor sensor() { return this.sensor; }
    public int accuracy() { return this.accuracy; }
    public long timestamp() { return this.timestamp; }
    public Date rcvTime() { return this.rcvTime; }
  }
}