/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import com.qapp.secprotect.Configs;

public class UtilsLog {

    private static final boolean ENABLE_LOG = false;

    public static void logd() {

        if (!ENABLE_LOG)
            return;
        Thread thread = Thread.currentThread();
        StackTraceElement[] mStackTrace = thread.getStackTrace();
        String methodName = mStackTrace[3].getMethodName();
        String className = mStackTrace[3].getClassName();
        className = className.substring(className.lastIndexOf('.') + 1);
        long threadId = thread.getId();

        String s = "[" + threadId + ": " + className + "." + methodName + "] ";
        Log.d(Configs.TAG, s + "");
    }

    public static void logd(Object s) {

        if (!ENABLE_LOG)
            return;
        Thread thread = Thread.currentThread();
        StackTraceElement[] mStackTrace = thread.getStackTrace();
        String methodName = mStackTrace[3].getMethodName();
        String className = mStackTrace[3].getClassName();
        className = className.substring(className.lastIndexOf('.') + 1);
        long threadId = thread.getId();

        s = "[" + threadId + ": " + className + "." + methodName + "] " + s;
        Log.d(Configs.TAG, s + "");
    }

    public static void loge(Object s) {

        Thread thread = Thread.currentThread();
        StackTraceElement[] mStackTrace = thread.getStackTrace();
        String methodName = mStackTrace[3].getMethodName();
        String className = mStackTrace[3].getClass().getSimpleName();
        long threadId = thread.getId();

        s = "[" + threadId + ": " + className + "." + methodName + "] " + s;
        Log.e(Configs.TAG, s + "");
    }

    public static void toast(final Context context, final Object s) {

        if (s == null)
            return;
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable() {
            public void run() {
                Toast.makeText(context, s + "", Toast.LENGTH_SHORT).show();
            }
        });
    }

}
