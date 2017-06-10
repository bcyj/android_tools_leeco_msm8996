/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.vtremoteservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.SystemProperties;

public class VTRemoteService extends Service {

    private String val;
    private String key;

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startID) {
        return Service.START_STICKY;
    }

    private final IRemoteService.Stub mBinder = new IRemoteService.Stub() {
        @Override
        public void setProperty(String key, String val) {
            SystemProperties.set(key, val);
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}