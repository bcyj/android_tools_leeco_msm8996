/******************************************************************************
 * @file    PPReceiver.java
 * @brief   Bootup Broadcast Receiver
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.display;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class PPReceiver extends BroadcastReceiver {

    private static final String TAG = "PPReceiver";

    private static final String ACTION_BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";

    @Override
    public void onReceive(Context context, Intent intent) {

        Intent j = new Intent(context, PPService.class);
        // Pass-through the intent extras to the service
        String action = intent.getAction();

        if (null != action && action.equals(ACTION_BOOT_COMPLETED)) {
            j.setAction(PPService.INTENT_PP_BOOT);
            context.startService(j);
        } else {
            Log.e(TAG,"Couldnot set hsic values");
        }
    }
}
