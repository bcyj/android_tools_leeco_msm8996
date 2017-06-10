/*===========================================================================
                           SmarterStandSensorListener.java

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import android.hardware.SensorManager;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import java.util.List;
import android.util.Log;
import java.lang.Math;

import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;

public class SmarterStandSensorListener {
    private static final String TAG = "SmarterStandSensorListener";

    public interface AccelerometerChangeCallback {
        void onNewPosition(double accelerometerAngleReading);
    }

    private final AccelerometerChangeCallback callback;
    private final SensorManager sensorManager;
    private final int supportedOrientation;
    private double prevReportedAngle = 0;
    /**
     * Report angles that differ by this threshold
     */
    private static final double REPORT_ANGLE_THRESHOLD = 1.5;
    /**
     * Create a sensor event listener to listen to accelerometer changes and
     * store angle.
     */
    private final SensorEventListener sensorEventListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            double rotationAngle = getRotationAngle(event.values[0], event.values[1],
                    event.values[2]);
            if (-1 != rotationAngle
                    && REPORT_ANGLE_THRESHOLD < diff(rotationAngle, prevReportedAngle)) {
                callback.onNewPosition(rotationAngle);
                Log.d(TAG, "Rotation angle: " + rotationAngle);
                prevReportedAngle = rotationAngle;
                return;
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // TODO Auto-generated method stub
        }

        double getRotationAngle(float x, float y, float z) {
            if (0 == z) {
                return -1;
            }

            double rotationAngle = getRotationAngleOfSupportedOrientation(x, y, z);
            if (0 > rotationAngle) {
                return 360 + rotationAngle;
            } else {
                return rotationAngle;
            }
        }

        private double getRotationAngleOfSupportedOrientation(float x, float y, float z) {
            double atanArgument = 0;
            switch (supportedOrientation) {
                case DigitalPenConfig.DP_SMARTER_STAND_ORIENTATION_LANDSCAPE:
                    atanArgument = (y / z);
                    break;
                case DigitalPenConfig.DP_SMARTER_STAND_ORIENTATION_PORTRAIT:
                    atanArgument = (x / z);
                    break;
                case DigitalPenConfig.DP_SMARTER_STAND_ORIENTATION_LANDSCAPE_REVERSED:
                    atanArgument = (-y / z);
                    break;
                case DigitalPenConfig.DP_SMARTER_STAND_ORIENTATION_PORTRAIT_REVERSED:
                    atanArgument = (-x / z);
                    break;
                default:
                    Log.w(TAG, "Invalid orientation for accelerometer angle");
                    break;
            }
            return Math.toDegrees(Math.atan(atanArgument));
        }
    };

    public SmarterStandSensorListener(AccelerometerChangeCallback callback,
            SensorManager sensorManager, int supportedOrientation) {
        this.callback = callback;
        this.sensorManager = sensorManager;
        this.supportedOrientation = supportedOrientation;
    }

    public void start() {
        // Register to accelerometer sensor events
        Sensor sensor;
        List<Sensor> sensors = sensorManager.getSensorList(Sensor.TYPE_ACCELEROMETER);
        if (0 < sensors.size()) {
            sensor = sensors.get(0);
            sensorManager.registerListener(sensorEventListener, sensor,
                    SensorManager.SENSOR_DELAY_NORMAL);
        } else {
            // Notify the user that there's no accelerometer sensor
            Log.w(TAG, "There's no accelerometer sensor, cannot track angle changes");
        }
    }

    public void stop() {
        if (null != sensorManager) {
            sensorManager.unregisterListener(sensorEventListener);
        }
    }

    private double diff(double d1, double d2) {
        return Math.abs(d1 - d2);
    }
}
