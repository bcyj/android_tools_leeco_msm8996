/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import static com.android.internal.telephony.TelephonyIntents.SECRET_CODE_ACTION;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class StartActivityReceiver extends BroadcastReceiver {
    private static final String SECRET_CODE = "3648665";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (SECRET_CODE_ACTION.equals(intent.getAction())) {
            String host = intent.getData() != null ? intent.getData().getHost() : null;
            if (SECRET_CODE.equals(host)) {
                Intent i = new Intent(context, EngineerToolActivity.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(i);
            }
        }
    }
}
