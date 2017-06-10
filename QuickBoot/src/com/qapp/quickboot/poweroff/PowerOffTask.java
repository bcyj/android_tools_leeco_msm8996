/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.poweroff;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.AsyncTask;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.SystemService;
import android.os.UserHandle;
import android.util.Log;

import com.qapp.quickboot.utils.Utils;
import com.qapp.quickboot.utils.UtilsProcess;
import com.qapp.quickboot.utils.Values;

public class PowerOffTask extends AsyncTask {

    private Context mContext = null;
    private WakeLock mWakeLock = null;
    private static final Object mSyncLock = new Object();

    public PowerOffTask(Context context) {
        mContext = context;
        mWakeLock = ((PowerManager) mContext.getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, Values.TAG);
    }

    @Override
    protected void onPreExecute() {

        logd("PowerOffTask start");
        acquireWakeLock();
        Utils.setOngoingFlag(mContext);
        Utils.syncStorage(mContext);
        SystemProperties.set(Values.KEY_QUICKBOOT_ENABLE, "1");
        SystemProperties.set(Values.KEY_QUICKBOOT_POWEROFF, "1");
        super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... arg0) {

        Utils.startBootAnimation();

        SystemProperties.set("sys.shutdown.requested", "QuickBoot");
        Utils.enableButtonLight(false);
        Utils.saveStatus(mContext);

        // Tell music playback service to pause
        Intent intent = new Intent(AudioManager.ACTION_AUDIO_BECOMING_NOISY);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

        BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                synchronized (mSyncLock) {
                    logd("done");
                    mSyncLock.notifyAll();
                }
            }
        };
        // send shutdown broadcast
        intent = new Intent(Intent.ACTION_SHUTDOWN);
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        intent.putExtra("from_quickboot", true);

        mContext.sendOrderedBroadcastAsUser(intent, UserHandle.ALL, null, broadcastReceiver, null,
                0, null, null);

        synchronized (mSyncLock) {
            try {
                // wait apps to exit
                mSyncLock.wait(2000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // set device states
        Utils.setAirplaneMode(mContext, true);

        // clear apps and their records
        Utils.clearNotification();
        Utils.clearRecentApps(mContext);
        UtilsProcess.killApplications(mContext);

        // put device into sleep mode
        Utils.sleep(mContext);
        // sleep some time for screen off
        SystemClock.sleep(300);
        Utils.vibrate(mContext, Values.POWEROFF_VIBRATE_TIME);
        SystemClock.sleep(Values.POWEROFF_VIBRATE_TIME);
        Utils.stopBootAnimation();
        Utils.startQbCharger();
        return null;
    }

    @Override
    protected void onPostExecute(Object result) {

        SystemProperties.set(Values.KEY_QUICKBOOT_POWEROFF, "0");
        releaseWakeLock();
        logd("PowerOffTask exit");
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
