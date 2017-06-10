/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (C) 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.fastdormancy;

import android.os.Bundle;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class FastDormancyReceiver extends BroadcastReceiver {
    private static final String TAG = "DormancyReceiver";
    private static final String PROPERTY_FAST_DORMANCY_KEY = "persist.env.fastdorm.enabled";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;

        if (SystemProperties.getBoolean(PROPERTY_FAST_DORMANCY_KEY, false)
                && Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            ComponentName comp = new ComponentName(context.getPackageName(), FastDormancyService
                    .class.getName());
            if (comp != null) {
                ComponentName service = context.startService(new Intent().setComponent(comp));
                if (service == null) {
                    Log.e(TAG, "Could Not Start Service " + comp.toString());
                } else {
                    Log.d(TAG, "Fast Dormancy Auto Boot Started Successfully");
                }
            } else {
                Log.d(TAG, "Can't find FD service,not Started Successfully");
            }
        } else {
            Log.d(TAG, "Received Unexpected Intent " + intent.toString());
        }
    }
}
