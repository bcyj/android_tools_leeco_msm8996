/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch.utils;

import java.text.SimpleDateFormat;
import java.util.Date;

public class Log {
    public final static String TAG = "TimerSwitch";

    public static final boolean LOGV = false;

    public static void d(String logInfo) {
        android.util.Log.d(TAG, logInfo);
    }

    public static void v(String logInfo) {
        android.util.Log.v(TAG, /* SystemClock.uptimeMillis() + " " + */ logInfo);
    }

    public static void i(String logInfo) {
        android.util.Log.i(TAG, logInfo);
    }

    public static void e(String logInfo) {
        android.util.Log.e(TAG, logInfo);
    }

    public static void e(String logInfo, Exception ex) {
        android.util.Log.e(TAG, logInfo, ex);
    }

    public static void w(String logInfo) {
        android.util.Log.w(TAG, logInfo);
    }

    public static void wtf(String logInfo) {
        android.util.Log.wtf(TAG, logInfo);
    }

    public static String formatTime(long millis) {
        return new SimpleDateFormat("HH:mm:ss.SSS/E").format(new Date(millis));
    }
}
