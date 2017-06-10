/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.WiFi;

import java.util.List;
import java.util.Map;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class WiFiService extends Service {
    
    private final static String TAG = "WiFiService";
    private static boolean result = false;
    private static Context mContext = null;
    private static WifiManager mWifiManager = null;
    private static int index = -1;
    private static WifiLock mWifiLock;
    private static List<ScanResult> wifiScanResult;
    private final static int SCAN_INTERVAL = 4000;
    private final static int OUT_TIME = 30000;
    private static IntentFilter mFilter = new IntentFilter();

    private final static Handler mHandler = new Handler() {
        public void dispatchMessage(android.os.Message msg) {

            if (wifiScanResult != null && wifiScanResult.size() > 0) {
                pass();
            } else {
                fail(null);
            }
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
        mWifiManager = (WifiManager) this.getSystemService(Context.WIFI_SERVICE);
    }

    private static void start() {

        logd("");
        /** Keep Wi-Fi awake */
        mWifiLock = mWifiManager.createWifiLock(WifiManager.WIFI_MODE_SCAN_ONLY, TAG);
        if (false == mWifiLock.isHeld())
            mWifiLock.acquire();
        
        switch (mWifiManager.getWifiState()) {
        case WifiManager.WIFI_STATE_DISABLED:
            enableWifi(true);
            break;
        case WifiManager.WIFI_STATE_DISABLING:
            break;
        case WifiManager.WIFI_STATE_UNKNOWN:
            break;
        default:
            break;
        }
        
        mFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        mContext.registerReceiver(mReceiver, mFilter);
        if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED)
            mWifiManager.startScan();
    }
    
    public static void finish(boolean writeResult) {
        
        try {
            mContext.unregisterReceiver(mReceiver);
        } catch (Exception e) {
            loge(e);
        }
        
        if (true == mWifiLock.isHeld())
            mWifiLock.release();
        if (wifiScanResult != null && wifiScanResult.size() > 0)
            result = true;
        logd("Result=" + result);
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

    private static void enableWifi(boolean enable) {
        
        if (mWifiManager != null)
            mWifiManager.setWifiEnabled(enable);
    }

    private static BroadcastReceiver mReceiver = new BroadcastReceiver() {
        
        public void onReceive(Context c, Intent intent) {
            
            logd(intent.getAction() + " wifiState=" + mWifiManager.getWifiState());
            if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(intent.getAction())) {
                
                wifiScanResult = mWifiManager.getScanResults();
                mHandler.sendEmptyMessage(0);
            } else if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(intent.getAction())) {
                if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED)
                    mWifiManager.startScan();
            }
        }
        
    };

    public static void finish() {
        finish(true);
    }
    
    @Override
    public void onDestroy() {
        logd("");
        // finish(false);
        super.onDestroy();
    }

    static void fail(Object msg) {
        loge(msg);
        result = false;
        finish();
    }
    
    static void pass() {

        result = true;
        Utilities.enableWifi(mContext, false);
        finish();
    }

    private static void logd(Object s) {
        
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
