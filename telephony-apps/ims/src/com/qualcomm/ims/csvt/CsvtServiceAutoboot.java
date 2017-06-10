/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.csvt;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class CsvtServiceAutoboot extends BroadcastReceiver {

    private static final String TAG = "CsvtService AutoBoot";
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            boolean isEnabled =  CsvtUtils.isCsvtEnabled();
            Log.i(TAG, "Received boot complete.CsvtEnabled = " + isEnabled);
            if (isEnabled) {
                ComponentName comp = new ComponentName(context.getPackageName(),
                        CsvtService.class.getName());
                ComponentName service = context.startService(new Intent().setComponent(comp));
                if (service == null) {
                    Log.e(TAG, "Could Not Start Service " + comp.toString());
                } else {
                    Log.i(TAG, "CsvtService Auto Boot Started Successfully");
                }
            }
        } else {
            Log.e(TAG, "Received Intent: " + intent.toString());
        }
    }
}
