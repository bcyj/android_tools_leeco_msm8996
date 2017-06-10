/******************************************************************************
 * @file    EmbmsBootReceiver.java
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.embms;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemProperties;

public class EmbmsBootReceiver extends BroadcastReceiver {
    private static final String LOG_TAG = "EmbmsBootReceiver";
    private boolean setUtcTimeProperty ;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(LOG_TAG, "onReceive");

        setUtcTimeProperty = SystemProperties.getBoolean("persist.radio.sib16_support", false);

        if(setUtcTimeProperty) {
            Log.i(LOG_TAG, "starting EmbmsService on Boot");
            context.startService(new Intent(context, EmbmsService.class));
        }
        else {
            Log.i(LOG_TAG, "persist.radio.sib16_support is not set, "
                            + "EmbmsService not started on Boot");
        }
    }
}
