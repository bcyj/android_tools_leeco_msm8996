/******************************************************************************
 * @file    OverlayService.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em.model;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;

public abstract class OverlayService extends Service {

    private static Context mContext;
    private static WindowManager mWindowManager;

    protected View mView;
    protected WindowManager.LayoutParams mParams;

    protected static LayoutInflater sLayoutInflater;

    @Override
    public void onCreate() {
        super.onCreate();

        mContext = getApplicationContext();
        mWindowManager = (WindowManager)mContext.getSystemService(WINDOW_SERVICE);
        sLayoutInflater = (LayoutInflater)mContext.getSystemService(LAYOUT_INFLATER_SERVICE);

        mView = setView();
        mParams = new WindowManager.LayoutParams();
        mWindowManager.addView(mView, setLayoutParams());
    }

    /**
     * Override this method and return a Layout for display
     */
    protected abstract View setView();

    protected abstract WindowManager.LayoutParams setLayoutParams();

    /**
     * This method here implements a switcher to open/close display
     */
    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);

        Bundle bundle = intent.getExtras();
        if (!bundle.getBoolean("startOverlay")) {
            mWindowManager.removeViewImmediate(mView);
            stopSelf();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
