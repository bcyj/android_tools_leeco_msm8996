/*============================================================================
@file SensorID.java

@brief
A value-type class to wrap the value of the sensor ID.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.sensortest;

import com.qualcomm.sensors.sensortest.SensorTest.DataType;
import android.hardware.Sensor;

/**
 * A value-type class to wrap the value of the sensor ID.
 */
public class SensorID {
    private int sensorCount_;
    private SensorType sensorType_;
    private int sensorID_;

    /**
     * Available sensor types.  Values of each type are defined in "sns_smgr_api_v01.h"
     */
    public enum SensorType {
        ACCEL(0),
        GYRO(10),
        MAG(20),
        PRESSURE(30),
        PROX_LIGHT(40);

        private final int typeValue;

        SensorType(int typeValue){
          this.typeValue = typeValue;
        }
        public int typeValue(){
            return this.typeValue;
        }

        static public SensorType getSensorType(Sensor sensor) {
           switch(sensor.getType()){
              case Sensor.TYPE_ACCELEROMETER:
                 return ACCEL;
              case Sensor.TYPE_GYROSCOPE:
                 return GYRO;
              case Sensor.TYPE_MAGNETIC_FIELD:
                 return MAG;
              case Sensor.TYPE_PRESSURE:
                 return PRESSURE;
              case Sensor.TYPE_PROXIMITY:
              case Sensor.TYPE_LIGHT:
                 return PROX_LIGHT;
              default:
                 return null;
           }
        }

        static public DataType getDataType(Sensor sensor) {
           switch(sensor.getType()){
              case Sensor.TYPE_ACCELEROMETER:
              case Sensor.TYPE_GYROSCOPE:
              case Sensor.TYPE_MAGNETIC_FIELD:
              case Sensor.TYPE_PRESSURE:
              case Sensor.TYPE_PROXIMITY:
                 return DataType.PRIMARY;
              case Sensor.TYPE_LIGHT:
                 return DataType.SECONDARY;
              default:
                 return null;
           }
        }
    };

    /**
     * @param sensorType The type of sensor for which we are constructing an ID.
     * @param sensorCount If there are multiple of the sensor type on the device, the one we wish to query (0 for primary).
              As presently defined, at most 9 instances of any given sensor type can co-exist on a single device.
     * @throws IllegalArgumentException Range Exception on sensorCount, or unsupported sensor type
     */
    public SensorID(SensorType sensorType, int sensorCount) throws IllegalArgumentException {
        if(sensorCount > 9 || sensorCount < 0)
            throw new IllegalArgumentException("Range Exception: the value of sensor count must be between 0 and 9, inclusive");
        if(sensorType == null){
            throw new IllegalArgumentException("Range Exception: sensor type must not be null");
        }
        this.sensorCount_ = sensorCount;
        this.sensorType_ = sensorType;
        this.sensorID_ = sensorType.typeValue() + sensorCount;
    }

    /* Accessor Functions */
    /**
     * @return An ID which uniquely specifies the sensor on the device.
     */
    public int getSensorID() { return this.sensorID_; }
    public SensorType getSensorType() { return this.sensorType_; }
    public int getSensorCount() { return this.sensorCount_; }
}
