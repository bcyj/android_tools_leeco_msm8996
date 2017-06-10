/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.SIM2;

import java.util.Map;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.qti.factory.Utils;
import com.qti.factory.Values;
import com.qti.factory.Framework.MainApp;

public class SIM2Service extends Service {

    String TAG = "SIM2Service";
    final int SLOT_ID = 1;
    boolean result = false;
    int index = -1;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null)
            return -1;
        index = intent.getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        if (index < 0)
            return -1;

        init();
        startTest();
        finishTest();

        return super.onStartCommand(intent, flags, startId);
    }

    private void init() {
        result = false;
    }

    private void finishTest() {
        Map<String, String> item = (Map<String, String>) MainApp.getInstance().mItemList
                .get(index);
        if (result) {
            item.put("result", Utils.RESULT_PASS);
            Utils.saveStringValue(getApplicationContext(), item.get("title"),
                    Utils.RESULT_PASS);
            Utils.writeCurMessage(TAG, Utils.RESULT_PASS);
        } else {
            item.put("result", Utils.RESULT_FAIL);
            Utils.saveStringValue(getApplicationContext(), item.get("title"),
                    Utils.RESULT_FAIL);
            Utils.writeCurMessage(TAG, Utils.RESULT_FAIL);
        }

        sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
    }

    private void startTest() {
        logd("");
        String iccid = null;
        if ("dsds"
                .equals(Utils.getSystemProperties(Values.PROP_MULTISIM, null))) {

            long[] subId = SubscriptionManager.getSubId(SLOT_ID);
            iccid = TelephonyManager.getDefault().getSimSerialNumber(subId[0]);
        } else {
            iccid = TelephonyManager.getDefault().getSimSerialNumber();
        }
        if (iccid != null && !iccid.equals("")) {
            result = true;
        }

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

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

}
