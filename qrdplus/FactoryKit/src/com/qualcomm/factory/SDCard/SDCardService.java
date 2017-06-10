/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.SDCard;

import java.util.Map;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class SDCardService extends Service {
    
    private final static String TAG = "SDCardService";
    private static boolean result = false;
    private static Context mContext = null;
    private static int index = -1;
    private final static Handler mHandler = new Handler() {
        public void dispatchMessage(android.os.Message msg) {
            Bundle bundle = msg.getData();
            String output = (String) bundle.get(Values.KEY_OUTPUT);
            
            if (output.contains("/storage/sdcard1"))
                pass();
            else
                fail(null);
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
    }

    private void start() {
        logd("");
        Utilities.exec("mount", mHandler);
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
        
    };
    
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
