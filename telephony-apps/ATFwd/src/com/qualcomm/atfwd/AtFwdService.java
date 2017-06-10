/*******************************************************************************
    Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

public class AtFwdService extends Service {

    private static final String TAG = "AtFwdService";
    private static AtCmdFwdService mAtCmdFwdIface;

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "Returning mAtCmdFwdIface for AtCmdFwdService binding.");
        return mAtCmdFwdIface;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate method");
        try {
            Log.i(TAG, "Instantiate AtCmdFwd Service");
            mAtCmdFwdIface = new AtCmdFwdService(this);
            ServiceManager.addService("AtCmdFwd", mAtCmdFwdIface);
        } catch (Throwable e) {
            Log.e(TAG, "Starting AtCmdFwd Service", e);
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "AtCmdFwdService Destroyed Successfully...");
        super.onDestroy();
    }
}
