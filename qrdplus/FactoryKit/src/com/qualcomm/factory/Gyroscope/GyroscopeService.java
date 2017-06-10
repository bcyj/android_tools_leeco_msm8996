/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Gyroscope;

import java.util.Map;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class GyroscopeService extends Service {
    
    private final static String TAG = "GyroscopeService";
    private static boolean result = false;
    private static Context mContext = null;
    private static int index = -1;
    private static SensorManager GyroscopeManager = null;
    private static Sensor mGyroscope = null;
    private static GyroscopeListener mGyroscopeListener;
    private final static String INIT_VALUE = "";
    private static String value = INIT_VALUE;
    private static String pre_value = INIT_VALUE;
    private final int MIN_COUNT = 4;
    private final static String i2C_CMD = "i2cdetect -y -r 0 0x48 0x48";
    private static boolean WORKROUND = false;
    private final static int SENSOR_TYPE = Sensor.TYPE_GYROSCOPE;
    private final static int SENSOR_DELAY = SensorManager.SENSOR_DELAY_FASTEST;

    private final static Handler mHandler = new Handler() {
        public void dispatchMessage(android.os.Message msg) {
            Bundle bundle = msg.getData();
            String output = (String) bundle.get(Values.KEY_OUTPUT);
            
            if (output.contains("--"))
                fail(null);
            else
                pass();
        };
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null)
            return -1;
        index = intent.getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        if (index < 0)
            return -1;
        
        init(getApplicationContext());
        start();
        // finish();
        
        return super.onStartCommand(intent, flags, startId);
    }
    
    private void init(Context context) {
        mContext = context;
        result = false;
        
        GyroscopeManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        if (GyroscopeManager == null) {
            fail(getString(R.string.service_get_fail));
        }
        
        mGyroscope = GyroscopeManager.getDefaultSensor(SENSOR_TYPE);
        if (mGyroscope == null) {
            fail(getString(R.string.sensor_get_fail));
        }
        
        mGyroscopeListener = new GyroscopeListener(this);

    }
    
    private void start() {
        logd("");
        if (WORKROUND) {
            Utilities.exec(i2C_CMD, mHandler);
        } else {
            if (!GyroscopeManager.registerListener(mGyroscopeListener, mGyroscope, SENSOR_DELAY)) {
                fail(getString(R.string.sensor_register_fail));
            }
        }

    }
    
    public static void finish(boolean writeResult) {
        
        if (writeResult) {
            Map<String, String> item = (Map<String, String>) MainApp.getInstance().mItemList.get(index);
            if (result) {
                item.put("result", Utilities.RESULT_PASS);
                Utilities.saveStringValue(mContext, item.get("title"), Utilities.RESULT_PASS);
                Utilities.writeCurMessage(TAG, Utilities.RESULT_PASS);
            } else {
                item.put("result", Utilities.RESULT_FAIL);
                Utilities.saveStringValue(mContext, item.get("title"), Utilities.RESULT_FAIL);
                Utilities.writeCurMessage(TAG, Utilities.RESULT_FAIL);
            }
            mContext.sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
        }
        
        if (!WORKROUND)
            try {
                GyroscopeManager.unregisterListener(mGyroscopeListener, mGyroscope);
            } catch (Exception e) {
            }

    };
    
    public class GyroscopeListener implements SensorEventListener {
        
        private int count = 0;
        
        public GyroscopeListener(Context context) {
            
            super();
        }
        
        public void onSensorChanged(SensorEvent event) {
            
            // Gyroscope event.value has 3 equal value.
            synchronized (this) {
                if (event.sensor.getType() == SENSOR_TYPE) {
                    logd(event.values.length + ":" + event.values[0] + " " + event.values[0] + " " + event.values[0]
                            + " ");
                    String value = "(" + event.values[0] + ", " + event.values[1] + ", " + event.values[2] + ")";
                    if (value != pre_value)
                        count++;
                    if (count >= MIN_COUNT)
                        pass();
                    pre_value = value;
                }
            }
        }
        
        public void onAccuracyChanged(Sensor arg0, int arg1) {
            
        }
    }

    public static void finish() {
        finish(true);
    }
    
    @Override
    public void onDestroy() {
        logd("");
        finish(false);
        super.onDestroy();
    }

    static void fail(Object msg) {
        loge(msg);
        result = false;
        finish();
    }
    
    static void pass() {
        
        result = true;
        finish();
    }
    
    private void logd(Object s) {
        
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        
        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
    
    private static void loge(Object e) {
        
        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

}
