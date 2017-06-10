/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.autoregistration;

import com.android.internal.telephony.TelephonyIntents;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AutoRegReceiver extends BroadcastReceiver {

    private static final String TAG = "AutoRegReceiver";
    private static final boolean DBG = false;
    private static final String BOOT_COMPLETE_FLAG = "boot_complete";
    private static final String MANUAL_REGISTRATION_FLAG = "manual";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (DBG) {
            Log.d(TAG, "onReceived action:" + intent.getAction());
        }
        Intent i = new Intent(context, RegistrationService.class);
        String receivedAction = intent.getAction();
        if (receivedAction.equals(Intent.ACTION_BOOT_COMPLETED)) {
            // start service to do the work.
            if (DBG) {
                Log.d(TAG, "Action boot completed received..");
            }
            i.putExtra(BOOT_COMPLETE_FLAG, true);
            context.startService(i);
        } else if (receivedAction.equals(TelephonyIntents.SECRET_CODE_ACTION)) {
            if (DBG) {
                Log.d(TAG, "Action secret code received..");
            }
            i.putExtra(MANUAL_REGISTRATION_FLAG, true);
            context.startService(i);
        } else if (receivedAction.equals(RegistrationService.ACTION_AUTO_REGISTERATION)) {
            if (DBG) {
                Log.d(TAG, "Action reschedual received..");
            }
            context.startService(i);
        }
    }

}
