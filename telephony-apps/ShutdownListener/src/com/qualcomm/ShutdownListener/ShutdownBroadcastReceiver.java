/*******************************************************************************
@file    ShutdownBroadcastReceiver.java
@brief   Receives shutdown broadcast event

---------------------------------------------------------------------------
Copyright (C) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.shutdownlistner;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.TelephonyProperties;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

public class ShutdownBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "ShutDownListener";

    private QcRilHook mQcRilHook;
    private boolean mCanInformShutdown = false;

    private static final int SUB1 = 0;
    private static final int SUB2 = 1;
    private static final int SUB3 = 2;

    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            performShutdown();
        }
        public void onQcRilHookDisconnected() {
            // TODO: Handle onQcRilHookDisconnected
        }
    };

    private void performShutdown() {
        if (mCanInformShutdown) {
            Log.d(TAG, "onQcRilHookReady callback, calling qcRilInformShutDown");
            mQcRilHook.qcRilInformShutDown(SUB1);

            String mSimConfig = SystemProperties.get(TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG);
            if (mSimConfig.equals("dsds") || mSimConfig.equals("dsda")) {
                mQcRilHook.qcRilInformShutDown(SUB2);
            } else if (mSimConfig.equals("tsts")) {
                mQcRilHook.qcRilInformShutDown(SUB2);
                mQcRilHook.qcRilInformShutDown(SUB3);
            }
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        mQcRilHook = new QcRilHook(context.getApplicationContext(), mQcrilHookCb);

        if ("android.intent.action.ACTION_SHUTDOWN".equals(intent.getAction())) {
            Log.d(TAG, "ACTION_SHUTDOWN Received, waiting for oemhook readiness callback");
            mCanInformShutdown = true;
        } else {
            Log.e(TAG, "Received Unexpected Intent " + intent.toString());
        }
    }
}
