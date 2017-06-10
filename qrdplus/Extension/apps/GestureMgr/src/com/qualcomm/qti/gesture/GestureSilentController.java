/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import android.app.Service;
import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class GestureSilentController extends Handler {

    private static GestureSilentController sInstance;
    private final Context mContext;

    private static final int MSG_START_LISTEN_SENSOR_EVENT = 100;
    private static final int MSG_STOP_LISTEN_SENSOR_EVENT = 101;
    private static final int MSG_SILENT_RINGER = 102;

    private static final String KEY_SILENT_GRESTURE = "key_silent_gesture";

    private TelephonyManager mTelephonyManager;
    private SensorEventListener mSensorListener;

    // Indicate whether the device starts rolling
    private boolean mStartRolling;

    public static void init(Context context) {
        synchronized (GestureSilentController.class) {
            if (sInstance == null) {
                sInstance = new GestureSilentController(context);
            }
        }
    }

    private GestureSilentController(Context context) {
        mContext = context;
        mTelephonyManager = (TelephonyManager) mContext.
                getSystemService(Context.TELEPHONY_SERVICE);
        listenPhoneState();
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_START_LISTEN_SENSOR_EVENT:
                startListeningSensorEvent();
                break;
            case MSG_SILENT_RINGER:
                mTelephonyManager.silenceRinger();
            case MSG_STOP_LISTEN_SENSOR_EVENT:
                stopListeningSensorEvent();
                break;
            default:
                break;
        }
    }

    private void listenPhoneState() {
        if (mTelephonyManager != null) {
            mTelephonyManager.listen(mPhoneStateListener,
                    PhoneStateListener.LISTEN_CALL_STATE);
        }
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            switch (state) {
                case TelephonyManager.CALL_STATE_RINGING:
                    if (isFlipMuteEnabled()) {
                        sendEmptyMessage(MSG_START_LISTEN_SENSOR_EVENT);
                    }
                    break;
                case TelephonyManager.CALL_STATE_OFFHOOK:
                case TelephonyManager.CALL_STATE_IDLE:
                    sendEmptyMessage(MSG_STOP_LISTEN_SENSOR_EVENT);
                    break;
                default:
                    break;
            }
        }
    };

    private boolean isFlipMuteEnabled() {
        SharedPreferences preferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return preferences.getBoolean(KEY_SILENT_GRESTURE, false);
    }

    private void startListeningSensorEvent() {
        SensorManager sm = (SensorManager) mContext.getSystemService(
                Service.SENSOR_SERVICE);
        Sensor sensor = sm.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mSensorListener = new SensorEventListener() {

            private float mSensorEventZ;

            // Indicate whether the device is resupinate
            private boolean mIsRollingUp;

            @Override
            public void onSensorChanged(SensorEvent event) {
                mSensorEventZ = event.values[2];
                if (!mStartRolling) {
                    if (mSensorEventZ > 0) {
                        mIsRollingUp = true;
                    } else {
                        mIsRollingUp = false;
                    }
                    mStartRolling = true;
                } else {
                    if (isTurnedOver()) {
                        if (mTelephonyManager != null && mTelephonyManager.isRinging()) {
                            sendEmptyMessage(MSG_SILENT_RINGER);
                        }
                    }
                }
             }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }

            private boolean isTurnedOver() {
                return (mSensorEventZ > 0 && !mIsRollingUp)
                        || (mSensorEventZ < 0 && mIsRollingUp);
            }
        };
        sm.registerListener(mSensorListener, sensor, SensorManager.SENSOR_DELAY_GAME);
    }

    private void stopListeningSensorEvent() {
        if (mSensorListener != null) {
            SensorManager sm = (SensorManager) mContext.getSystemService(
                    Service.SENSOR_SERVICE);
            sm.unregisterListener(mSensorListener);
            mStartRolling = false;
            mSensorListener = null;
        }
    }
}
