/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Framework;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.CountDownTimer;
import android.os.IBinder;
import android.util.Log;

import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp.ServiceInfo;

public class AutoService extends IntentService {
    String TAG = "AutoService";
    private static Context mContext = null;
    
    public AutoService() {
        super("AutoService");
    }
    
    private int taskCounter = 0;
    private long INTERVAL_TIME = 500;
    private long FINISH_TIME = 30 * 1000;
    CountDownTimer taskCountDownTimer = new CountDownTimer(FINISH_TIME, INTERVAL_TIME) {
        @Override
        public void onTick(long arg0) {
            ArrayList<ServiceInfo> intentArrayList = MainApp.getInstance().intentArrayList;
            if (intentArrayList.size() > 0) {
                for (int i = 0; i < intentArrayList.size(); i++) {
                    int delay = 0;
                    if (intentArrayList.get(i).paraMap.get(Values.KEY_SERVICE_DELAY) != null)
                        delay = Integer.valueOf(intentArrayList.get(i).paraMap.get(Values.KEY_SERVICE_DELAY));
                    if (delay == taskCounter * INTERVAL_TIME) {
                        Intent intent = intentArrayList.get(i).intentService;
                        if (intent != null &&
                                (intent.getComponent() != null || intent.getPackage() != null)) {
                            startService(intent);
                            logd(taskCounter + " " + intent.getAction());
                        }
                    }
                }
            }
            taskCounter++;
        }
        
        @Override
        public void onFinish() {
        }
    };
    
    private void init(Context context) {
        
        mContext = context;
        taskCounter = 0;
        
        // To save test time, enable some devices first
        Utilities.enableWifi(mContext, true);
        Utilities.enableBluetooth(true);
        Utilities.enableGps(mContext, true);
        Utilities.enableCharging(true);
        Utilities.configScreenTimeout(mContext, 1800000); // 1 min
        Utilities.configMultiSim(mContext);
    }
    
    @Override
    protected void onHandleIntent(Intent intent) {
        
        logd("RunningService=" + MainApp.getInstance().getServiceCounter());
        if (MainApp.getInstance().getServiceCounter() > 0)
            return;
        init(getApplicationContext());
        
        List<Map<String, ?>> list = (List<Map<String, ?>>) MainApp.getInstance().mItemList;
        for (int i = 0; i < list.size(); i++) {
            Map<String, ?> item = list.get(i);
            if ("true".equals(item.get("auto")))
                logd(item.get("title") + " " + item.get("result"));
            if ("true".equals(item.get("auto")))
                if (Utilities.RESULT_FAIL.equals(item.get("result")) || "NULL".equals(item.get("result"))) {
                    
                    logd("Add " + item.get("title") + " to service list");
                    String service = ((HashMap<String, String>) item.get("parameter")).get("service");
                    Intent serviceIntent = new Intent(service);
                    serviceIntent.putExtra(Values.KEY_SERVICE_INDEX, i);
                    MainApp.getInstance().addServiceList(new ServiceInfo(serviceIntent, item.get("parameter")));
                }
        }
        
        taskCountDownTimer.start();
    }
    
    @Override
    public void onDestroy() {
        logd("");
        // if (taskCountDownTimer != null)
        // taskCountDownTimer.cancel();
        super.onDestroy();
    }
    
    private void logd(Object s) {
        
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        
        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
    
    private void loge(Object s) {
        
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        
        s = "[" + mMethodName + "] " + s;
        Log.e(TAG, s + "");
    }
    
    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }
    
}
