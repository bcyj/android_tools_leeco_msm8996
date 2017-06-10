/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

package com.qualcomm.location;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.qualcomm.location.MonitorInterface.Monitor;

public class DeviceContext extends Monitor {
    private static final String TAG = "DeviceContext";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);
    private static final int MSG_START = 0;
    private static final int MSG_CHARGER_STATE_INJECT = 1;
    private static final int MSG_SHUTDOWN = 2;
    private static final int MSG_MAX = 3;
    private static final int CHARGER_ON = 1;
    private static final int CHARGER_OFF = 0;
    private int mChargeState;

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            logv("mBroadcastReceiver - " + action);

            if (action.equals(Intent.ACTION_POWER_DISCONNECTED)) {
                checkChargerIntent(CHARGER_OFF);
            } else if (action.equals(Intent.ACTION_POWER_CONNECTED)) {
                checkChargerIntent(CHARGER_ON);
            } else if (action.equals(Intent.ACTION_SHUTDOWN)) {
                sendMessage(MSG_SHUTDOWN, 0, 0, null);
            }
        }
    };

    private void checkChargerIntent(int chargerState) {
        logv("mChargeState - "+mChargeState+"; chargerState - "+chargerState);
        if (mChargeState != chargerState) {
            mChargeState = chargerState;

            sendMessage(MSG_CHARGER_STATE_INJECT, chargerState, 0, null);
        }
    }

    public DeviceContext(MonitorInterface service, int msgIdBase) {
        super(service, msgIdBase);
        mChargeState = CHARGER_OFF;
        sendMessage(MSG_START, 0, 0, null);
    }

    private void start() {
        // Get the current battery updates from sticky intent. This is why a null
        // broadcast receiver is allowed.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryIntent = mMoniterService.getContext().registerReceiver(null, intentFilter);

        int plugged = -1;
        plugged = batteryIntent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        boolean currentBatteryCharging = ((plugged == BatteryManager.BATTERY_PLUGGED_AC)
                             || (plugged == BatteryManager.BATTERY_PLUGGED_USB));
        mChargeState = currentBatteryCharging ? CHARGER_ON:CHARGER_OFF;
        sendMessage(MSG_CHARGER_STATE_INJECT, mChargeState, 0, null);
        logv("In start(). mChargeState - " + mChargeState);

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_POWER_CONNECTED);
        filter.addAction(Intent.ACTION_POWER_DISCONNECTED);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        mMoniterService.getContext().
            registerReceiver(mBroadcastReceiver, filter);
    }

    @Override
    public void handleMessage(Message msg) {
        int message = msg.what;
        Log.d(TAG, "handleMessage what - " + message);

        try {
            switch (message) {
            case MSG_START:
                start();
                native_dc_init();
                break;
            case MSG_CHARGER_STATE_INJECT:
                native_dc_charger_status_inject(msg.arg1);
                break;
            case MSG_SHUTDOWN:
                native_dc_shutdown();
                break;
            default:
                break;
            }
        } catch (ClassCastException cce) {
            Log.w(TAG, "ClassCastException on " + message);
        }
    }

    private static native void native_dc_class_init();
    private native void native_dc_init();
    public native void native_dc_charger_status_inject(int chargerStatus);
    public native void native_dc_shutdown();

    @Override
    public int getNumOfMessages() {
        return MSG_MAX;
    }

    static private void logv(String s) {
        if (VERBOSE_DBG) Log.v(TAG, s);
    }

}
