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
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.SystemService;
import android.os.UserHandle;
import android.util.Log;

import com.qapp.quickboot.utils.Utils;
import com.qapp.quickboot.utils.Values;

public class PowerOnTask extends AsyncTask {

    private static final String BOOT_ANIMATION_SERVICE = "bootanim";

    private Context mContext = null;
    private WakeLock mWakeLock = null;
    private IActivityManager mActivityManager = null;

    public PowerOnTask(Context context) {
        mContext = context;
        mWakeLock = ((PowerManager) mContext.getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, Values.TAG);
        mActivityManager = ActivityManagerNative.getDefault();
    }

    @Override
    protected void onPreExecute() {

        logd("PowerOnTask start");
        acquireWakeLock();
        SystemProperties.set(Values.KEY_QUICKBOOT_POWERON, "1");
        super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... arg0) {

        // tell framework poweron process started
        Intent intent = new Intent(Values.INTENT_QUICKBOOT_START);
        mContext.sendBroadcast(intent, "android.permission.DEVICE_POWER");

        if (SystemService.isRunning("qbcharger"))
            Utils.stopQbCharger();
        // ensure quickboot charger has stopped
        while (SystemService.isRunning("qbcharger")) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
            }
            logd("waiting qbcharger stopped...");
        }

        Utils.startBootAnimation();
        // waiting for starting boot animation
        int retry = 10;
        do {
            try {
                Thread.sleep(50);
            } catch (InterruptedException e) {
            }
            retry--;
        } while (retry > 0 && !SystemService.isRunning(BOOT_ANIMATION_SERVICE));

        Utils.vibrate(mContext, Values.POWERON_VIBRATE_TIME);
        SystemClock.sleep(Values.POWERON_VIBRATE_TIME);

        // wakeup screen
        Utils.wakeup(mContext);
        Utils.restoreStatus(mContext);

        SystemProperties.set("sys.shutdown.requested", "");
        intent = new Intent(Intent.ACTION_BOOT_COMPLETED, null);
        intent.putExtra(Intent.EXTRA_USER_HANDLE, 0);
        intent.putExtra("from_quickboot", true);
        try {
            mActivityManager.broadcastIntent(null, intent, null, null, 0, null, null,
                    android.Manifest.permission.RECEIVE_BOOT_COMPLETED, AppOpsManager.OP_NONE,
                    false, false, UserHandle.USER_ALL);

        } catch (RemoteException e) {
            e.printStackTrace();
        }

        // disable touch input during poweron process
        Utils.enableWindowInput(false);
        SystemClock.sleep(2000);
        Utils.stopBootAnimation();
        // enable touch input
        Utils.enableWindowInput(true);
        return null;
    }

    @Override
    protected void onPostExecute(Object result) {

        SystemProperties.set(Values.KEY_QUICKBOOT_ENABLE, "0");
        SystemProperties.set(Values.KEY_QUICKBOOT_POWERON, "0");
        Utils.clearOngoingFlag(mContext);
        releaseWakeLock();
        logd("PowerOnTask exit");
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
