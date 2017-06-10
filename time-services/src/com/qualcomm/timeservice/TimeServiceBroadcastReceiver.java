/*******************************************************************************
@file    TimeServiceBroadcastReceiver.java
@brief   Updates time-services user time offset when user changes time of
         the day and Android sends a TIME_CHANGED or DATE_CHANGED intents.
         time-services restores the time of the day after reboot using
         this offset.
---------------------------------------------------------------------------
Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.timeservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemClock;

public class TimeServiceBroadcastReceiver extends BroadcastReceiver {

    private static final String TAG = "TimeService";

    public native void setTimeServicesUserTime(long millis);
    static { System.loadLibrary("TimeService"); }

    @Override
    public void onReceive(Context context, Intent intent) {

        if ((Intent.ACTION_TIME_CHANGED.equals(intent.getAction())) ||
            (Intent.ACTION_DATE_CHANGED.equals(intent.getAction()))) {

            Log.d(TAG, "Received" + intent.getAction() + " intent. " +
                       "Current Time is " + System.currentTimeMillis());
            setTimeServicesUserTime(System.currentTimeMillis());
        } else {
            Log.w(TAG, "Received Unexpected Intent " + intent.toString());
        }
    }
}
