/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.simcontacts;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class PhoneSyncService extends Service {
    private static final String TAG = "SyncService";
    private static final Object sLock = new Object();
    private static SyncAdapter sSyncAdapter = null;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        synchronized (sLock) {
            if (null == sSyncAdapter) {
                sSyncAdapter = new SyncAdapter(this, true);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return sSyncAdapter.getSyncAdapterBinder();
    }
}
