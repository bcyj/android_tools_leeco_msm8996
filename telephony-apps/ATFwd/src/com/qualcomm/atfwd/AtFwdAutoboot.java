/*******************************************************************************
    Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class AtFwdAutoboot extends BroadcastReceiver {

    private static final String TAG = "AtFwd AutoBoot";
    @Override
    public void onReceive(Context context, Intent intent) {
        boolean canStartService = SystemProperties.getBoolean("persist.radio.atfwd.start", true);
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) && canStartService) {
            ComponentName comp = new ComponentName(context.getPackageName(),
                    AtFwdService.class.getName());
            ComponentName service = context.startService(new Intent().setComponent(comp));
            if (service == null) {
                Log.e(TAG, "Could Not Start Service " + comp.toString());
            } else {
                Log.e(TAG, "AtFwd Auto Boot Started Successfully");
            }
        } else {
            Log.e(TAG, "Received Intent: " + intent.toString() + " canStartService = "
                    + canStartService);
        }
    }
}
