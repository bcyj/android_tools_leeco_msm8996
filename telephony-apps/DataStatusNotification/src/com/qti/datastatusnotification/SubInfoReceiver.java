/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.qualcomm.datastatusnotification;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.android.internal.telephony.TelephonyIntents;

public class SubInfoReceiver extends BroadcastReceiver {
    private static final String TAG = "QcDataStatusNotification SubInfoReceiver";
    private static final boolean DBG = false;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (DBG)
            Log.d(TAG, "onReceive: " + intent.toString());
        if (intent.getAction().equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
            Intent serviceIntent = new Intent(context, DataStatusNotificationService.class);
            ComponentName serviceComponent = context.startService(serviceIntent);
            if (serviceComponent == null) {
                Log.e(TAG, "Could not start service");
            } else {
                Log.d(TAG, "Successfully started service");
            }
        } else {
            Log.e(TAG, "Received unsupported intent");
        }
    }
}
