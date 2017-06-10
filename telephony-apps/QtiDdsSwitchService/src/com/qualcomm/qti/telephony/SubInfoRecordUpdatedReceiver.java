/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.util.Log;

public class SubInfoRecordUpdatedReceiver extends BroadcastReceiver {
    private static final String LOG_TAG = "QcDdsSwitchService";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(LOG_TAG, "onReceive hit");
            Log.d(LOG_TAG, "SUBINFO_RECORD_UPDATED. Start the service.");
            Intent startIntent = new Intent(context, QtiDdsSwitchService.class);
            // Only start the service as the owner of the device
            // Otherwise we would have permission problem
            context.startServiceAsUser(startIntent, UserHandle.OWNER);
    }
}
