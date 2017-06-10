/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.poweron;

import android.app.ActivityManagerNative;
import android.app.AppOpsManager;
import android.app.IActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemService;
import android.os.UserHandle;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.util.Log;

import com.qapp.quickboot.utils.Utils;
import com.qapp.quickboot.utils.Values;

/**
 * This task is for booting up from poweroff alarm.
 *
 * */
public class AlarmBootTask extends AsyncTask {

    private Context mContext = null;
    private WakeLock mWakeLock = null;
    private IActivityManager mActivityManager = null;

    public AlarmBootTask(Context context) {
        mContext = context;
        mWakeLock = ((PowerManager) mContext.getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, Values.TAG);
        mActivityManager = ActivityManagerNative.getDefault();
    }

    @Override
    protected void onPreExecute() {

        if (!Utils.isUnderQuickBoot(mContext))
            Utils.exit(mContext);
        logd("AlarmBootTask start");
        acquireWakeLock();
        SystemProperties.set(Values.KEY_QUICKBOOT_POWERON, "1");
        super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... arg0) {

        if (SystemService.isRunning("qbcharger"))
            Utils.stopQbCharger();

        Utils.wakeup(mContext);
        Utils.restoreStatus(mContext);

        SystemProperties.set("sys.shutdown.requested", "");
        Intent intent = new Intent(Intent.ACTION_BOOT_COMPLETED, null);
        intent.putExtra(Intent.EXTRA_USER_HANDLE, 0);
        intent.putExtra("from_quickboot", true);
        try {
            mActivityManager.broadcastIntent(null, intent, null, null, 0, null, null,
                    android.Manifest.permission.RECEIVE_BOOT_COMPLETED, AppOpsManager.OP_NONE,
                    false, false, UserHandle.USER_ALL);

        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    protected void onPostExecute(Object result) {

        SystemProperties.set(Values.KEY_QUICKBOOT_ENABLE, "0");
        SystemProperties.set(Values.KEY_QUICKBOOT_POWERON, "0");
        Utils.clearOngoingFlag(mContext);
        releaseWakeLock();
        logd("AlarmBootTask exit");
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
