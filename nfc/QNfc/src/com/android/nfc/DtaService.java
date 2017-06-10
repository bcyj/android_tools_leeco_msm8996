/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.nfc;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

final public class DtaService extends Service {
    private NfcService.DtaHelperService service = null;
    static final String TAG = "DtaService";

    public DtaService() {
        super();
        service = null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()'ed");
            super.onCreate();
            NfcService service = NfcService.getInstance();
            this.service = service.getDtaHelperService();
    }

    @Override
    public IBinder onBind(Intent intent) {
            Log.d(TAG, "onBind()'ed");
            return this.service;
    }

    @Override
    public boolean onUnbind(Intent intent) {
            Log.d(TAG, "onUnbind()'ed");
            return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
            Log.d(TAG, "onDestroy()'ed");
            this.service = null;
            super.onDestroy();
    }
}
