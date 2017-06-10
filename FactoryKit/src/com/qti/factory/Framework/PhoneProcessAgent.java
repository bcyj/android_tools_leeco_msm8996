/*
 * Copyright (c) 2011-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.Framework;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.qti.factory.Utils;
import com.qti.factory.Values;

public class PhoneProcessAgent extends Service {
    String TAG = "PhoneProcessAgent";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        configMultiSim();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    private void configMultiSim() {
        logd("");
        if (!"dsds".equals(Utils
                .getSystemProperties(Values.PROP_MULTISIM, null))) {
            return;
        }

        final TelephonyManager telephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        int numSlots = telephonyManager.getSimCount();
        for (int i = 0; i < numSlots; i++) {
            SubscriptionManager.activateSubId(i);
        }
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

}
