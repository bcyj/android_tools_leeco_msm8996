/*============================================================================
@file SensorStatus.java

@brief
Class bundles the three attributes of how each sensor will appear to the user.

Copyright (c) 2011,2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

package com.qti.sensors.selftest;

import java.util.ArrayList;
import java.util.List;

import android.graphics.Color;
import android.hardware.Sensor;

/**
 * Class bundles the three attributes of how each sensor will appear to the user.
 */
public class SensorStatus {
    private Sensor sensor;
    private String status;
    private int color;

    /**
     * @param sensor The Sensor object itself (although all that is used is its name)
     * @param status The status of the test.  Eg. "Not Started", "PASSED", etc..
     * @param color What color the status text should appear as.
     */
    public SensorStatus(Sensor sensor, String status, int color) {
        super();
        this.sensor = sensor;
        this.status = status;
        this.color = color;
    }

    public Sensor getSensor() { return sensor; }
    public String getStatus() { return status; }
    public void setStatus(String status) { this.status = status; }
    public int getColor() { return color; }
    public void setColor(int color) { this.color = color; }

    public static List<SensorStatus> createSensorList(List<Sensor> sensorList){
        List<SensorStatus> testSensorList = new ArrayList<SensorStatus>(sensorList.size());
        for(Sensor sensor : sensorList){
            SensorStatus testSensor = new SensorStatus(sensor, "Operation not started", Color.BLACK);
            testSensorList.add(testSensor);
        }
        return testSensorList;
    }
}
