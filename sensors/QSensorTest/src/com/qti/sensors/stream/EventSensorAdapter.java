/*============================================================================
@file EventSensorAdapter.java

@brief
Wrapper API for View and Controller to access or modify the QSensor Model.  To
be used for event-based sensors.

Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.stream;

import android.hardware.Sensor;
import android.hardware.SensorManager;

public class EventSensorAdapter extends SensorAdapter {

  public EventSensorAdapter(Sensor sensor, SensorManager sensorManager) {
    super(sensor, sensorManager);
  }

  public synchronized String[] sensorData(){
    String outputString[] = {"", ""};

    for(int i = 0; i < QSensor.eventLength && i < this.sensorEvents().size(); i++){
      float data[] = this.sensorEvents().get(i).values();

      outputString[0] += "[" + 0 + "]:" + Float.toString(data[0]) + "\n";

      outputString[1] += "ts:" +
          Long.toString(this.sensorEvents().get(i).timestamp() / 1000) +
          "  ";
      outputString[1] += this.accuracy()+ "\n";
    }
    for(int i = 0; i < outputString.length && outputString[i].length() > 0; i++)
      outputString[i] = outputString[i].substring(0, outputString[i].length()-1);

    return outputString;
  }
}