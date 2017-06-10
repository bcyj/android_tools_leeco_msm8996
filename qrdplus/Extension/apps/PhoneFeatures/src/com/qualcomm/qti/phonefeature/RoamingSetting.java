/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.AlertDialog;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.view.WindowManager;

import com.android.internal.telephony.PhoneFactory;

public class RoamingSetting extends Handler {

    private final Context mContext;

    private static final int EVENT_ROAMING_CHANGED = 1;

    public RoamingSetting(Context context) {
        mContext = context;
        AppGlobals.getInstance().mServiceMonitor.registerRoamingStateChanged(this,
                EVENT_ROAMING_CHANGED, null);
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_ROAMING_CHANGED:
                onRoamingStateChanged((AsyncResult) msg.obj);
                break;
        }
    }

    public void onRoamingStateChanged(AsyncResult ar) {
        int slotId = (Integer) ar.result;
        boolean roaming = AppGlobals.getInstance().mServiceMonitor.isRoaming(slotId);
        boolean mobileDataEnabled = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.MOBILE_DATA, 0) != 0;
        if (!roaming || !mobileDataEnabled) {
            return;
        }
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.DATA_ROAMING, 1);
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.DATA_ROAMING
                + slotId, 1);
        if ((!Constants.MULTI_MODE || SubscriptionManager.getSlotId(PhoneFactory.
                getDataSubscription()) == slotId)) {
            alertRoaming();
        }
    }

    protected void alertRoaming() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        AlertDialog dialog = builder.setMessage(R.string.alert_roaming_messgae)
                .setNegativeButton(android.R.string.ok, null).create();
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    }

    public void dispose() {
        AppGlobals.getInstance().mServiceMonitor.unregisterRoamingStateChanged(this);
    }
}
