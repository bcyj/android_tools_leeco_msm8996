/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Bluetooth;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.IBluetooth;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class BluetoothService extends Service {
    
    private final static String TAG = "BluetoothService";
    private static boolean result = false;
    private static Context mContext = null;
    private static int index = -1;
    private static BluetoothAdapter mBluetoothAdapter = null;
    private static List<DeviceInfo> mDeviceList = new ArrayList<DeviceInfo>();
    private static IBluetooth btService;
    private final static int MIN_COUNT = 1;
    private static IntentFilter mIntentFilter;
    
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
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        
        mIntentFilter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        mIntentFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        mIntentFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        mIntentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        
    }
    
    private static void start() {
        logd("");
        mContext.registerReceiver(mReceiver, mIntentFilter);
        if (mBluetoothAdapter.getState() == BluetoothAdapter.STATE_ON) {
            scanDevice();
        } else {
            if (mBluetoothAdapter.getState() != BluetoothAdapter.STATE_TURNING_ON) {
                mBluetoothAdapter.enable();
            }
        }
    }
    
    public static void finish(boolean writeResult) {
        
        cancelScan();
        try {
            mContext.unregisterReceiver(mReceiver);
        } catch (Exception e) {
            loge(e);
        }

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
    
    private static void scanDevice() {
        logd("");
        if (mBluetoothAdapter.isDiscovering()) {
            mBluetoothAdapter.cancelDiscovery();
        }
        mBluetoothAdapter.startDiscovery();
    }
    
    private static void cancelScan() {
        
        if (mBluetoothAdapter.isDiscovering()) {
            mBluetoothAdapter.cancelDiscovery();
        }
    }
    
    private static BroadcastReceiver mReceiver = new BroadcastReceiver() {
        
        @Override
        public void onReceive(Context context, Intent intent) {
            
            String action = intent.getAction();
            logd(action);
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                mDeviceList.add(new DeviceInfo(device.getName(), device.getAddress()));
                if (mDeviceList.size() >= MIN_COUNT)
                    pass();
                
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                if (mDeviceList.size() >= MIN_COUNT) {
                    // pass();
                } else
                    fail(null);
            } else if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action)) {
            } else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
                
                if (BluetoothAdapter.STATE_ON == intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0))
                    scanDevice();
                if (BluetoothAdapter.STATE_OFF == intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0))
                    mBluetoothAdapter.enable();
            } else if (BluetoothAdapter.STATE_TURNING_ON == intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0)) {

            }
            
        }// onReceive
        
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
        logd("");
        result = true;
        Utilities.enableBluetooth(false);
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
