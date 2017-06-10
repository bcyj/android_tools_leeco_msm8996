/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Battery;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Map;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.qti.factory.Utils;
import com.qti.factory.Values;
import com.qti.factory.Framework.MainApp;

public class BatteryService extends Service {
    
    private final static String TAG = "BatteryService";
    private static boolean result = false;
    private static Context mContext = null;
    private static int index = -1;
    final String CAPACITY = "/sys/class/power_supply/battery/capacity";
    final String VOLTAGE_NOW = "/sys/class/power_supply/battery/voltage_now";
    final String STATUS = "/sys/class/power_supply/battery/status";

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
    }

    private void start() {
        logd("");
        String status = null;
        
        status = getBatteryInfo(STATUS);
        if (status != null) {

            if ("Charging".equals(status) || "Full".equals(status))
                result = true;
            else
                result = false;
        }

        if (result)
            pass();
        else
            fail(null);
    }

    public static void finish(boolean writeResult) {
        
        if (writeResult) {
            Map<String, String> item = (Map<String, String>) MainApp.getInstance().mItemList.get(index);
            if (result) {
                item.put("result", Utils.RESULT_PASS);
                Utils.saveStringValue(mContext, item.get("title"), Utils.RESULT_PASS);
                Utils.writeCurMessage(TAG, Utils.RESULT_PASS);
            } else {
                item.put("result", Utils.RESULT_FAIL);
                Utils.saveStringValue(mContext, item.get("title"), Utils.RESULT_FAIL);
                Utils.writeCurMessage(TAG, Utils.RESULT_FAIL);
            }
            mContext.sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
        }
        
    };
    
    private String getBatteryInfo(String path) {
        
        File mFile;
        FileReader mFileReader;
        mFile = new File(path);
        
        try {
            mFileReader = new FileReader(mFile);
            char data[] = new char[128];
            int charCount;
            String status[] = null;
            try {
                charCount = mFileReader.read(data);
                status = new String(data, 0, charCount).trim().split("\n");
                logd(status[0]);
                return status[0];
            } catch (IOException e) {
                loge(e);
            }
        } catch (FileNotFoundException e) {
            loge(e);
        }
        return null;
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
        
        if (Values.SERVICE_LOG) {
            Thread mThread = Thread.currentThread();
            StackTraceElement[] mStackTrace = mThread.getStackTrace();
            String mMethodName = mStackTrace[3].getMethodName();
            
            s = "[" + mMethodName + "] " + s;
            Log.d(TAG, s + "");
        }
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
