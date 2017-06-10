/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.poweron;

import android.content.Context;
import android.os.AsyncTask;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.util.Log;

import com.qapp.quickboot.utils.Utils;
import com.qapp.quickboot.utils.Values;

/**
 * This task is for restore device's state in case of any exception.
 *
 * */
public class RestoreTask extends AsyncTask {

    private Context mContext = null;
    private WakeLock mWakeLock = null;

    public RestoreTask(Context context) {
        mContext = context;
        mWakeLock = ((PowerManager) mContext.getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, Values.TAG);
    }

    @Override
    protected void onPreExecute() {

        logd("RestoreTask start");
        if (!Utils.isOngoing(mContext))
            Utils.exit(mContext);
        acquireWakeLock();
        super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... arg0) {
        Utils.restoreStatus(mContext);
        return null;
    }

    @Override
    protected void onPostExecute(Object result) {

        SystemProperties.set(Values.KEY_QUICKBOOT_ENABLE, "0");
        Utils.clearOngoingFlag(mContext);
        releaseWakeLock();
        logd("RestoreTask exit");
        Utils.exit(mContext);
        super.onPostExecute(result);
    }

    @Override
    protected void onCancelled() {
        super.onCancelled();
    }

    private void acquireWakeLock() {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
    }

    private void releaseWakeLock() {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();

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
