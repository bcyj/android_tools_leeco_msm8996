/*
 * Copyright (c) 2011-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.SIM1;

import android.app.Activity;
import android.os.Bundle;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.qti.factory.Utils;
import com.qti.factory.Values;

public class SIM1 extends Activity {

    String TAG = "SIM1";
    int SLOT_ID = 0;
    String resultString = "Failed";
    String toastString = "";
    boolean result = false;

    @Override
    public void finish() {

        Utils.writeCurMessage(TAG, resultString);

        logd(resultString);
        super.finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        logd("");

        super.onCreate(savedInstanceState);
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
            toastString = "iccid: " + iccid;
        }

        logd(iccid);

        if (result) {
            setResult(RESULT_OK);
            resultString = Utils.RESULT_PASS;
            toast(toastString);

        } else {
            setResult(RESULT_CANCELED);
            resultString = Utils.RESULT_FAIL;
        }
        finish();
    }

    public void toast(Object s) {

        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }

    private void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
}
