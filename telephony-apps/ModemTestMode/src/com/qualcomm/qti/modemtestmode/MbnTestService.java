/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import android.app.Service;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.content.Intent;
import android.os.SystemProperties;

import com.qualcomm.qti.modemtestmode.IMbnTestService;

public class MbnTestService extends Service {
    private static final String TAG = "MbnTestService";

    private final IMbnTestService.Stub mBinder = new IMbnTestService.Stub() {

        @Override
        public void setProperty(String property, String value)
                throws RemoteException {
            log ("set " + property + " :" + value);
            SystemProperties.set(property, value);
        }

    };

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_ " + msg);
    }
}
