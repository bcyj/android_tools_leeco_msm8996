/*******************************************************************************
@file    ATunerService.java
@brief   Receives shutdown broadcast event

---------------------------------------------------------------------------
Copyright (C) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.atuner;

import android.app.Service;

import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;

import android.content.Context;
import android.content.Intent;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;

import java.util.List;

import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.TunerOemHook;

public class ATunerService extends Service implements SensorEventListener {

    /**
     * Variables
     */
    private static final boolean DBG = true;

    private static final String TAG = "TunerService";
    private SensorManager mSensorManager;
    private Sensor mSensor;
    private Handler mProximityHandler;
    private long mLastProximityEventTime;
    private int mProximityPendingValue = -1; // -1 = NO OBJECT, 0 = FAR, 1 = NEAR
    private QcRilHook mQcRilHook;
    private Runnable mSystemPropertyUpdater = null;
    private TunerOemHook mTunerOemHook;

    private boolean mCalculateAngles = SystemProperties.getBoolean(
            "persist.atel.tuner.calc_angles", false);
    private final int mModemRevision = SystemProperties.getInt("persist.atel.tuner.modem_rev",
            1);

    // TODO: Make following values configurable!
    private static final float PROXIMITY_THRESHOLD = 3.0f;
    // If the Object is is within 3 - 10 cms, treat it as 'FAR' range
    private static final float PROXIMITY_FAR_MAX_RANGE = 10.0f;
    // Debounce timer for proximity sensor in milliseconds
    private static final int PROXIMITY_SENSOR_DELAY = 1000;

    // Failure reason codes
    private final String TUNER_SERVICE_PROXIMITY_SENSOR_NOT_SUPPORTED   =
                                               "Proximity Sensor support does not exist";
    private final String TUNER_SERVICE_MODEM_SANITY_CHECK_FAILURE =
                                          "Failure to establish authenticity with the modem";
    private final String TUNER_SERVICE_UNABLE_TO_RETRIEVE_MODEM_REV_FAILURE = "Unable to retrieve" +
                  "Modem Revision";
    private final String TUNER_SERVICE_GENERIC_FAILURE = "Generic Failure";

    // Handler events
    private final int EVENT_TUNER_UNSOL_REQ              = 1;
    private final int EVENT_TUNER_CMD_PERFORM_SANITY     = 2;
    private final int EVENT_TUNER_CMD_SANITY_DONE        = 3;
    private final int EVENT_TUNER_OEMHOOK_READY          = 4;

    // Retry variables
    private final int RETRY_TIME_MS = 5000;
    private final int RETRY_MAX_ATTEMPTS = 5;
    private int mRetryAttempts = 0;

    /**
     * Binder Function
     */
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {

    }

    @Override
    public void onStart(Intent intent, int startId) {

        super.onStart(intent, startId);

        mProximityHandler = new Handler();

        mTunerOemHook = TunerOemHook.getInstance(this, mMessageHandler);
        if (mTunerOemHook == null) {
            Log.e(TAG, "mTunerOemHook is null");
            stopService(TUNER_SERVICE_GENERIC_FAILURE);
            return;
        } else {
            if (DBG) Log.d(TAG, "mTunerOemHook created successfully");
            //Register with TunerOemHook for readiness
            mTunerOemHook.registerOnReadyCb(mMessageHandler, EVENT_TUNER_OEMHOOK_READY, null);
            Log.v(TAG, "Registered with TunerOemHook for readiness");
        }
    }

    @Override
    public void onDestroy() {

        Log.d(TAG, "Background Service Destroyed Successfully ...");
        super.onDestroy();
        if (null != mProximityHandler && null != mProximityTask)
            mProximityHandler.removeCallbacks(mProximityTask);
        unregisterFromSensors();
        if (null != mTunerOemHook)
            mTunerOemHook.unregisterOnReadyCb(mMessageHandler);
            mTunerOemHook.dispose();
  }

    @Override
    public void onSensorChanged(SensorEvent event) {

        switch (event.sensor.getType()) {
            case Sensor.TYPE_PROXIMITY:
                onProximityChanged (event);
                break;
            case Sensor.TYPE_GYROSCOPE:
                onGyroscopeChanged (event);
                break;
            case Sensor.TYPE_ACCELEROMETER:
                onAccelerometerChanged (event);
                break;
            default:
                Log.e(TAG, "Invalid Sensor type " + event.sensor.getType());
                break;
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // ignore
    }

    private Handler mMessageHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (DBG) Log.d(TAG, "handleMessage what=" + msg.what);

            switch (msg.what) {
                case EVENT_TUNER_UNSOL_REQ:
                    Log.d(TAG, "Message EVENT_TUNER_UNSOL_REQ received");
                    //UnsolObject obj = (UnsolObject)(((AsyncResult) msg.obj).result);
                    //handleUnsol(obj);
                    break;
                case EVENT_TUNER_CMD_PERFORM_SANITY:
                    int tunerRev = mTunerOemHook.tuner_get_provisioned_table_revision();
                    if (tunerRev == -1) {
                        stopService(TUNER_SERVICE_UNABLE_TO_RETRIEVE_MODEM_REV_FAILURE);
                        break;
                    } else {
                        Log.d(TAG, "Modem Tuner Rev " + tunerRev);
                    }

                    // just in case
                    removeMessages(EVENT_TUNER_CMD_PERFORM_SANITY);

                    if(mModemRevision == tunerRev) {
                        sendMessage(Message.obtain(mMessageHandler,EVENT_TUNER_CMD_SANITY_DONE));
                        break;
                    }

                    if (mRetryAttempts < RETRY_MAX_ATTEMPTS) {
                        if (DBG) Log.d(TAG, "Scheduling next attempt # " + mRetryAttempts);
                        sendMessageDelayed(Message.obtain(mMessageHandler,
                                          EVENT_TUNER_CMD_PERFORM_SANITY) ,
                                         (mRetryAttempts * RETRY_TIME_MS));
                        ++mRetryAttempts;
                        break;
                    }

                    stopService(TUNER_SERVICE_MODEM_SANITY_CHECK_FAILURE);
                    break;
                case EVENT_TUNER_CMD_SANITY_DONE:
                    registerForSensors();
                    break;
                case EVENT_TUNER_OEMHOOK_READY:
                    Log.d(TAG, "Message EVENT_QCRILHOOK_READY received");
                    performModemSanity();
                    break;
                default:
                    break;
            }
        }
    };

    private void stopService(String reason) {
        if (DBG) Log.d(TAG, "Stopping the service : reason - " + reason);
        stopSelf();
    }

    private void performModemSanity() {
        mMessageHandler.sendMessage(Message.obtain(mMessageHandler, EVENT_TUNER_CMD_PERFORM_SANITY));
    }

    private void handleUnsol (/*UnsolObject obj*/) {
        // For Future Use
    }

    private SensorManager getSensorManager() {
        if (mSensorManager == null) {
            mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        }
        return mSensorManager;
    }

    private void registerForSensors() {

        if (DBG) Log.d(TAG, "Registering for Sensors...");

        if (!registerForProximitySensor()) {
            Log.e(TAG, "Unable to register with Proximity Sensor");
            stopService(TUNER_SERVICE_PROXIMITY_SENSOR_NOT_SUPPORTED);
            return;
        }

        if (!registerFor_Gyro_Or_Accl_Sensor() && mCalculateAngles) {
            Log.e(TAG, "Unable to register with either Gyroscope or Accelerometer Sensor :"
                    + mCalculateAngles);
        }

    }

    private boolean registerForProximitySensor() {
        List<Sensor> sensors = getSensorManager().getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.isEmpty()) {
            Log.e(TAG, "No Proximity Sensors detected...");
            return false;
        }

        // Get the first sensor of a given sensor type
        return getSensorManager().registerListener(
                this, sensors.get(0), SensorManager.SENSOR_DELAY_FASTEST);
    }

    private boolean registerFor_Gyro_Or_Accl_Sensor() {
        List<Sensor> sensors = getSensorManager().getSensorList(Sensor.TYPE_GYROSCOPE);
        if (sensors.isEmpty()) {
            Log.d(TAG, " No Gyroscope sensor detected...");
            /*
             * Gyroscope sensors may produce less noise and reliable values
             * but they are not generally prevalent across different hardware.
             * Therefore relying on 'Accelerometer' sensor , if 'Gyroscope'
             * is not supported by the hardware in order to compute angles / orientation.
             */
            sensors = getSensorManager().getSensorList(Sensor.TYPE_ACCELEROMETER);
            if (sensors.isEmpty()) {
                Log.e(TAG, "No Accelerometer sensor detected either...");
                return false;
            }

        }
        // Get the first available sensor for now
        return getSensorManager().registerListener(
                this, sensors.get(0), SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void unregisterFromSensors() {
        if (DBG) Log.d(TAG, "Unregistering with all the Sensors");
        getSensorManager().unregisterListener(this);
    }

    private Runnable mProximityTask = new Runnable() {
        public void run() {
            if (mProximityPendingValue != -1) {
                sendProximityUpdate(mProximityPendingValue == 1);
                mProximityPendingValue = -1;
            }
        }
    };

    private void onProximityChanged(SensorEvent event) {
        long milliseconds = SystemClock.elapsedRealtime();
        long timeSinceLastEvent = milliseconds - mLastProximityEventTime;
        mLastProximityEventTime = milliseconds;
        // distance : Distance from object measured in centimeters
        float distance = event.values[0];
        if (DBG) Log.d(TAG, "distance: " + distance +
                " Max Range: " + event.sensor.getMaximumRange());

        /*
         * Some proximity sensors return binary values that
         * represent "near" or "far." In this case, the sensor
         * usually reports its maximum range value in the far state and a
         * lesser value in the near state. Typically, the far value is a value > 3 cm,
         * but this can vary from sensor to sensor.
         * Determine a sensor's maximum range by using the
         * getMaximumRange() method.
         */

        /*
         * NEAR : 1
         * FAR  : 0
         * NO OBJECT : -1 , perhaps
         */
        boolean proximity = (distance >= 0.0 && distance < PROXIMITY_THRESHOLD &&
                distance < event.sensor.getMaximumRange());

        if (DBG) Log.d(TAG, "proximity: " + proximity);

        if (distance > PROXIMITY_FAR_MAX_RANGE) {
            //proximity = 0;
            if (DBG) Log.d(TAG, "Sensor , Not detecting any object!");
        }

        mProximityHandler.removeCallbacks(mProximityTask);

        if (timeSinceLastEvent < PROXIMITY_SENSOR_DELAY) {
            // enforce delaying at-least by 'PROXIMITY_SENSOR_DELAY - timeSinceLastEvent'
            // before processing
            mProximityPendingValue = (proximity ? 1 : 0);
            mProximityHandler.postDelayed(mProximityTask,
                                          PROXIMITY_SENSOR_DELAY - timeSinceLastEvent);
        } else {
            // process the value immediately
            mProximityPendingValue = -1;
            /*
             * Say, If there are multiple proximity sensors
             * then there should be a way to distinguish
             * the events from different sensors.
             * Use event.sensor.getHandle() : Current Sensor_handle,
             * to distinguish.
             */
            sendProximityUpdate(proximity);
        }
        /*
         * Note:
         * Design Consideration: If proximity == FAR, it is a good idea to unregister
         * other CPU intensive registered listeners such as Gyroscopic (or) Accelerometer
         * and re-register back once the proximity == NEAR. In such scenarios, it is appropriate
         * to have two separate listeners as opposed to the current design of a single
         * registered listener for all sensors.
         * Are Accelerometer / Gyroscope implementations poll based unlike Proximity listener
         * which is interrupt based ? Maybe , having either Gyroscope / Accelerometer listeners
         * remain active forever, may have a drastic effect on Power consumption.
         */

        /*
         * Design Consideration: Per current requirements , the sensor AP is required to
         * refresh scenario updates every 10 seconds to the modem - This requirement needs to be
         * revisited as it may have effect again on power consumption.
         */
    }

    private void onGyroscopeChanged(SensorEvent event) {
        double x = event.values[0];
        double y = event.values[1];
        double z = event.values[2];
        // TODO: Angle to be computed using angular speeds
        return;
    }

    private void onAccelerometerChanged(SensorEvent event) {
        double x = event.values[0];
        double y = event.values[1];
        double z = event.values[2];

        if (DBG) Log.d(TAG, "onSensorEvent(" + x + ", " + y + ", " + z + ")");

        // If some values are exactly zero, then likely the sensor is not powered up yet.
        // ignore these events to avoid false horizontal positives.
        if (x == 0.0 || y == 0.0 || z == 0.0) return;

        // magnitude of the acceleration vector projected onto XY plane
        double xy = Math.sqrt(x*x + y*y);
        // compute the vertical angle
        double angle = Math.atan2(xy, z);
        // convert to degrees
        angle = angle * 180.0 / Math.PI;
        if (DBG) Log.d(TAG, "angle: " + angle);

        sendAngleUpdate(angle);
        return;
    }

    private void sendProximityUpdate(boolean active) {
        int[] val = {active ? 1 : 0};

        // Note this API shouldn't block for too long.
        Integer retValue = mTunerOemHook.tuner_send_proximity_updates(val);
        if (DBG) Log.d(TAG, "mTunerOemHook.tuner_send_proximity_updates : ret " + retValue);
    }

    private void sendAngleUpdate(double angle) {
        if (DBG) Log.d(TAG, "sendAngleUpdate - Not Supported");
    }
}
