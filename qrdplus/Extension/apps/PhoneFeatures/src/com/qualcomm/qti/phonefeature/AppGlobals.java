/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.Application;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

public class AppGlobals extends Application {

    public static final String TAG = "phone_features";

    ServiceStateMonitor mServiceMonitor;
    CardStateMonitor mCardMonitor;

    Phone[] mPhones;

    QcRilHook mQcRilHook;
    private boolean mQcRilHookLoaded = false;
    private RegistrantList mProxyLoadedRegistrants = new RegistrantList();

    private QcRilHookCallback mQcRilHookCallback = new QcRilHookCallback() {
        @Override
        public void onQcRilHookReady() {
            mQcRilHookLoaded = true;
            synchronized (mProxyLoadedRegistrants) {
                mProxyLoadedRegistrants.notifyRegistrants();
            }
        }
        @Override
        public void onQcRilHookDisconnected() {
            mQcRilHookLoaded = false;
        }
    };

    private static AppGlobals mApp;

    public static AppGlobals getInstance() {
        if (mApp == null) {
            throw new IllegalStateException("No AppGlobals here!");
        }
        return mApp;
    }

    public AppGlobals() {
        mApp = this;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mPhones = PhoneFactory.getPhones();
        mServiceMonitor = new ServiceStateMonitor();
        mCardMonitor = new CardStateMonitor(this);
        mQcRilHook = new QcRilHook(this, mQcRilHookCallback);
    }

    public void registerQcRilHookLoaded(Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mProxyLoadedRegistrants) {
            mProxyLoadedRegistrants.add(r);
            if (mQcRilHookLoaded) {
                r.notifyRegistrant();
            }
        }
    }

    public void unregisterQcRilHookLoaded(Handler handler) {
        synchronized (mProxyLoadedRegistrants) {
            mProxyLoadedRegistrants.remove(handler);
        }
    }

    public boolean isQcRilHookLoaded() {
        return mQcRilHookLoaded;
    }
}
