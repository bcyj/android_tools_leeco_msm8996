/* ==============================================================================
 * WfdService.java
 *
 * Copyright (c) 2012,2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd.service;

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.util.Log;

/**
 * Main service entry point for Session Manager Service
 *
 * @author sachins
 */
public class WfdService extends Service {

    private static final String TAG = "WfdService";

    private ISessionManagerService.Stub mBinder;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStart()");
        onBind(intent);
        Notification n = new Notification.Builder(getApplicationContext())
            .setContentTitle("Wireless Display")
            .setContentText("Screen Mirroring is On")
            .setSmallIcon(R.drawable.icon).build();
        startForeground(Process.myPid(), n);
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        if (mBinder == null) {
            Context c = getApplicationContext();
            Log.d(TAG, "Creating SessionManagerService with context:" + c);
            mBinder = new SessionManagerService(c);
        }
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
        if (mBinder != null && mBinder instanceof SessionManagerService) {
            Log.d(TAG, "Unregister callbacks");
            ((SessionManagerService) mBinder).destroyService();
            mBinder = null;
        }
    }

}
