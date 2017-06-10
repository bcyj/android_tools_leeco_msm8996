/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.framework;

import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import android.app.Application;
import android.content.Context;

public class MainApp extends Application {

    public ScheduledExecutorService scheduledExecutorService = Executors
            .newSingleThreadScheduledExecutor();
    private static MainApp mMainApp;
    private static Context mContext;
    private BlockingQueue<String> mBlockingQueue = new ArrayBlockingQueue<String>(
            1);
    public String mInternalKey = null;
    public String mSdcardKey = null;
    public String mInternalPassword = null;
    public String mSdcardPassword = null;

    private void init() {
        logd("");
        mContext = getApplicationContext();
    }

    public void clearPasswordCache() {
        mInternalKey = null;
        mSdcardKey = null;
        mInternalPassword = null;
        mSdcardPassword = null;
    }

    @Override
    public void onCreate() {
        logd("");
        init();
        super.onCreate();
    }

    @Override
    public void onLowMemory() {
        logd("");
        super.onLowMemory();
    }

    @Override
    public void onTerminate() {
        logd("");
        super.onTerminate();
    }

    public static MainApp getInstance() {
        if (mMainApp == null)
            mMainApp = new MainApp();
        return mMainApp;
    }

    public MainApp() {
    }

    public ScheduledExecutorService getScheduledExecutorService() {
        return scheduledExecutorService;
    }

    public Context getContext() {
        return mContext;
    }
}
