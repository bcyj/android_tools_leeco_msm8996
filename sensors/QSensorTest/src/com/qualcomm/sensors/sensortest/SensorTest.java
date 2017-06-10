/*============================================================================
@file SensorTestLib.java

@brief
Provides tools to initiate sensor-specific tests.

Ensure that libsensor_test.so and libsensor_reg.so have been loaded
 onto the device, or is included with the application.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qualcomm.sensors.sensortest;

import android.util.Log;

/**
 * Defines an interface to be used to request a particular sensor to perform a self-test.
 *
 */
public class SensorTest {
    // Tag to use when logging messages to the android device (viewable through logcat).
    private static final String TAG = "SensorTest";

    /**
     * Load the native functions.  The library will be named on the file system "libsensor_test.so",
     * and must be pushed to the device before use (to system/lib, or [app_data]/lib).
     */
    static {
        System.loadLibrary("sensor_test");
    }

    /**
     * Runs the specified sensor test.
     *
     * @param sensorID The ID of the sensor on which we wish to run a test
     * @param dataType The data type requested
     * @param testType If there are multiple tests available, which one to run (currently only "self test" available)
     * @param saveToRegistry If applicable, whether to save bias calculations to the registry as part of the test.
     * @param applyCalNow If applicable, whether to apply bias calculations immediately.
     * @return The error-code result of the self-test; (-10:-20 correspond to sns_smgr_test_status_e_v01)
     *         0: Success
     *         code > 0: Error meaning dependent on sensor
     *         -1: Sensor Test Native Error
     *         -2: Invalid Sensor ID
     *         -3: Test Timed-out
     *         -12: Device Busy
     *         -13: Invalid Test
     *         -14: Invalid Test Parameter
     *         -15: Received 'failed' response
     *         -16: Another test is running, try later.
     *         -21: Broken Message Pipe
     *         -22: Internal Error
     */
    static public synchronized int runSensorTest(SensorID sensorID, DataType
        dataType, TestType testType, boolean saveToRegistry, boolean applyCalNow){
        if(sensorID == null){
	      Log.e(TAG, "SensorID must not be NULL");
	      return -2;
        }

        int nativeTestResult = runNativeSensorTest(sensorID.getSensorID(), dataType.ordinal(), testType.ordinal(),
              saveToRegistry, applyCalNow);
        return nativeTestResult;
    }

    /**
     * Initiates a sensor test.  Will save bias values to the registry and
     * will apply them immediately.
     *
     * @deprecated  Replaced by {@link #runSensorTest(SensorID, DataType, TestType, Boolean, Boolean)}
     */
    static public int runSensorTest(SensorID sensorID, DataType dataType, TestType testType){
       return runSensorTest(sensorID, dataType, testType, true, true);
    }

    /**
     * Native function (ie. implemented in C) to run the self-test.
     *
     * Using primitive types through JNI is considerably simpler, less error-prone, and
     * requires less memory than passing complex objects.  Hence the reason for both a
     * non-native, compile-time argument type-enforcing function and a native, simple
     * JNI function.
     *
     * @param sensorID Sensor ID as defined by the SMGR API.
     * @param dataType Binary integer for primary/secondary data.  Int is used instead of
     *                 Boolean in case future data types are created.
     * @param testType The specific test to run.
     * @param saveToRegistry If applicable, whether to save bias calculations
     *                       to the registry as part of the test.
     * @param applyCalNow If applicable, whether to apply bias calculations immediately.
     * @return Error-code as returned from the sensor.  0 upon success.
     */
    static private native int runNativeSensorTest(int sensorID, int dataType, int testType,
          boolean saveToRegistry, boolean applyCalNow);

    /**
     * @deprecated Replaced by {@link #runNativeSensorTest(int, int, int, Boolean, Boolean)}
     */
    static private native int runNativeSensorTest(int sensorID, int dataType, int testType);

    /**
     * Enables or disables raw data mode.
     *
     * When raw data mode is enabled, requests for sensor data through the Android API will return uncalibrated and
     * unfiltered data.  When disabled, sensor data will be returned as normal.  Default value is disabled, and
     * will reset upon device start.
     *
     * @param enabled For streaming data from sensors, whether to send raw (true) or calibrated (false) data.
     * @throws Exception When an error has occurred within the native code.
     */
    static public synchronized void setRawDataMode(boolean enabled) throws Exception {
        int result = setNativeRawDataMode(enabled);

        // PENDING: Interpret error code and display message
        if(result != 0){
            throw new Exception("Unknown error occurred within native code: " + result);
        }
    }

    /**
     * Native function to set the raw data mode.
     *
     * @param enabled For streaming data from sensors, whether to send raw (true) or calibrated (false) data.
     * @return 0 if mode was set successfully, any other number upon failure.
     *
     * @see #setRawDataMode(boolean)
     */
    static private native int setNativeRawDataMode(boolean enabled);

    /**
     * @return True if raw data mode is currently enabled, false otherwise.
     */
    static public synchronized boolean getRawDataMode(){
        int enabled = getNativeRawDataMode();

        // PENDING: If enabled < 0, an error has occurred; interpret and print message
        return enabled == 1 ? true : false;
    }

    /**
     * @return True if raw data mode is currently enabled, false otherwise.
     *
     * @see #setNativeRawDataMode(boolean)
     * @see #getRawDataMode()
     */
    static private native int getNativeRawDataMode();

    /**
     * The data-type we wish to receive.
     *
     * All sensors offer primary, some sensor types, such as the Light/Proximity sensor, offer a secondary data-type option.
     */
    public enum DataType { PRIMARY, SECONDARY };

    /**
     * The types of tests than can be executed.
     *
     * The order of the values here must match the
     * numeric values associated with them (defined in sns_smgr_api_v01.idl).
     */
    public enum TestType { SELFTEST, IRQ, CONNECTIVITY, SELFTEST_HW, SELFTEST_SW, OEMTEST };
}

