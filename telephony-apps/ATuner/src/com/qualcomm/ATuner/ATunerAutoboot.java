/*******************************************************************************
@file    ATunerAutoboot.java
@brief   Android Tuner Service Auto Boot

DESCRIPTION
Used for booting ATunerService once kernel boots up

---------------------------------------------------------------------------
Copyright (C) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.atuner;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class ATunerAutoboot extends BroadcastReceiver {

    /**
     * Variables
     */
    private static final String TAG = "ATuner Auto Boot";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        boolean mTunerSupport = SystemProperties.getBoolean("persist.atel.tuner.support",
                                                            false);

        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) && mTunerSupport ) {
            ComponentName comp = new ComponentName(context.getPackageName(),
                   ATunerService.class.getName());
            if (comp != null) {
                ComponentName service = context.startService(new Intent().setComponent(comp));
                if (service == null) {
                    Log.e(TAG, "Could Not Start Service " + comp.toString());
                } else {
                    Log.e(TAG, "ATuner Auto Boot Started Successfully");
                }
            } else {
                Log.e(TAG, "ATuner Auto Boot Not Started Successfully");
            }
        } else {
            Log.e(TAG, "Received Intent: " + intent.toString() +
                       "Tuner Support :" + mTunerSupport);
        }

    }

}

