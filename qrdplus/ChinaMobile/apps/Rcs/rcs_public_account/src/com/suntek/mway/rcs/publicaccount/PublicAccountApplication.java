/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount;

import com.suntek.mway.rcs.publicaccount.http.service.CommonHttpRequest;

import android.app.Application;
import android.content.Context;
import android.os.Vibrator;

public class PublicAccountApplication extends Application {

    private static PublicAccountApplication sInstance;

    private static long nowThreadId;

    public long getNowThreadId() {
        return nowThreadId;
    }

    public void setNowThreadId(long threadId) {
        nowThreadId = threadId;
    }

    public static PublicAccountApplication getInstance() {
        return sInstance;
    }

    private Vibrator vibrator;

    @Override
    public void onCreate() {
        super.onCreate();
        sInstance = this;
        RcsApiManager.init(this);
        CommonHttpRequest.getInstance();
    }

    public void vibrator(long milliseconds) {
        if (vibrator == null) {
            vibrator = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
        }
        vibrator.vibrate(milliseconds);
    }
}
