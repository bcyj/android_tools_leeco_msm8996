/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

public class Smsc extends Handler {

    private static final int EVENT_SET_SMSC_DONE = 1;
    private static final int EVENT_GET_SMSC_DONE = 2;
    private static final int EVENT_SET_SMSC = 3;
    private static final int EVENT_GET_SMSC = 4;

    private final int mSlot;
    private final Message mCallback;

    public Smsc(int slot, Message message) {
        super(Looper.getMainLooper());
        mSlot = slot;
        mCallback = message;
    }

    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_SET_SMSC_DONE:
                responseSetDone(((AsyncResult) msg.obj).exception);
                break;
            case EVENT_GET_SMSC_DONE:
                responseGetDone((String) ((AsyncResult) msg.obj).result,
                        ((AsyncResult) msg.obj).exception);
                break;
            case EVENT_GET_SMSC:
                AppGlobals.getInstance().mPhones[mSlot].getSmscAddress(
                        obtainMessage(EVENT_GET_SMSC_DONE));
                break;
            case EVENT_SET_SMSC:
                AppGlobals.getInstance().mPhones[mSlot].setSmscAddress((String) msg.obj,
                        obtainMessage(EVENT_SET_SMSC_DONE));
                break;
        }
    }

    public void set(final String smsc) {
        obtainMessage(EVENT_SET_SMSC, smsc).sendToTarget();
    }

    public void get() {
        obtainMessage(EVENT_GET_SMSC).sendToTarget();
    }

    private void responseSetDone(Throwable exception) {
        Bundle bundle = new Bundle();
        bundle.putSerializable(FeatureProvider.EXTRA_EXCEPTION, exception);
        Constants.response(bundle, mCallback);
    }

    private void responseGetDone(String smsc, Throwable exception) {
        Bundle bundle = new Bundle();
        bundle.putString(FeatureProvider.EXTRA_SMSC, smsc);
        bundle.putSerializable(FeatureProvider.EXTRA_EXCEPTION, exception);
        Constants.response(bundle, mCallback);
    }
}
