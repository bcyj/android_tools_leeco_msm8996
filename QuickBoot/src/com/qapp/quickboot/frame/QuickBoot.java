/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.frame;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.qapp.quickboot.poweroff.PowerOffTask;
import com.qapp.quickboot.poweron.PowerOnTask;
import com.qapp.quickboot.poweron.RestoreTask;
import com.qapp.quickboot.utils.Values;

public class QuickBoot extends Activity {

    private Context mContext = null;

    void init() {
        logd("");
        mContext = getApplicationContext();
    }

    @Override
    protected void onCreate(Bundle arg0) {

        super.onCreate(arg0);
        init();

        Intent intent = getIntent();
        int mode = 0;
        if (intent != null && intent.hasExtra(Values.KEY_INTENT_LAUNCH_MODE))
            mode = intent.getIntExtra(Values.KEY_INTENT_LAUNCH_MODE, 0);

        switch (mode) {
        case Values.MODE_POWER_ON:
            new PowerOnTask(mContext).execute();
            break;
        case Values.MODE_POWER_OFF:
            new PowerOffTask(mContext).execute();
            break;
        case Values.MODE_RESTORE:
            new RestoreTask(mContext).execute();
            break;
        default:
            new PowerOffTask(mContext).execute();
            break;
        }

    }

    private static void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(Values.TAG, s + "");
    }
}
