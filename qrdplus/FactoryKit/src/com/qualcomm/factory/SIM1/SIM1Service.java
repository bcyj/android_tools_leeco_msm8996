/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.SIM1;

import java.util.Map;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp;

public class SIM1Service extends Service {

    String TAG = "SIM1Service";
    int simState = TelephonyManager.SIM_STATE_UNKNOWN;
    boolean result = false;
    final int SUB_ID = 0;
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

    public static final int SIM_STATE_UNKNOWN = 0;
    public static final int SIM_STATE_ABSENT = 1;
    public static final int SIM_STATE_PIN_REQUIRED = 2;
    public static final int SIM_STATE_PUK_REQUIRED = 3;
    public static final int SIM_STATE_NETWORK_LOCKED = 4;
    public static final int SIM_STATE_READY = 5;
    public static final int SIM_STATE_CARD_IO_ERROR = 6;

    private void init() {
        simState = TelephonyManager.SIM_STATE_UNKNOWN;
        result = false;
    }

    private void finishTest() {
        Map<String, String> item = (Map<String, String>) MainApp.getInstance().mItemList.get(index);
        if (result) {
            item.put("result", Utilities.RESULT_PASS);
            Utilities.saveStringValue(getApplicationContext(), item.get("title"), Utilities.RESULT_PASS);
            Utilities.writeCurMessage(TAG, Utilities.RESULT_PASS);
        } else {
            item.put("result", Utilities.RESULT_FAIL);
            Utilities.saveStringValue(getApplicationContext(), item.get("title"), Utilities.RESULT_FAIL);
            Utilities.writeCurMessage(TAG, Utilities.RESULT_FAIL);
        }

        sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
    }

    private void startTest() {
        logd("");
        String IMSI = null;
        int simState = TelephonyManager.SIM_STATE_ABSENT;
        boolean isSubActive = false;
        TelephonyManager mTelephonyManager = (TelephonyManager)getSystemService(
                Service.TELEPHONY_SERVICE);
        if (mTelephonyManager == null) {
            return;
        }
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            simState = mTelephonyManager.getSimState(SUB_ID);
            IMSI = mTelephonyManager.getSubscriberId(SUB_ID);
            isSubActive = Utilities.isSimSubscriptionStatusActive(SUB_ID);
        } else {
            simState = mTelephonyManager.getSimState();
            IMSI = mTelephonyManager.getSubscriberId();
        }

        if (IMSI != null && !IMSI.equals("")) {
            result = true;
        } else if (simState != TelephonyManager.SIM_STATE_ABSENT) {
            result = true;
        } else if (isSubActive){
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
