/*******************************************************************************
  @file    Diag_OnBoot.java
  @brief   Diag On Boot

  DESCRIPTION
  Used for booting DiagServices once Android is up

  ---------------------------------------------------------------------------
  Copyright (C) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.diagservices;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import java.io.IOException;
import android.os.SystemProperties;

public class Diag_OnBoot extends BroadcastReceiver {

    /**
     * Variables
     */
    private static final String TAG = "Diag_OnBoot";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;

        if ("android.intent.action.BOOT_COMPLETED".equals(intent.getAction())) {
            if ((SystemProperties.get("ro.board.platform").startsWith("msm"))
                    || (SystemProperties.get("ro.board.platform").startsWith("apq"))
                    || (SystemProperties.get("ro.board.platform").startsWith("mpq"))) {
                ComponentName comp = new ComponentName(context.getPackageName(), QTIDiagServices.class
                        .getName());
                if (comp != null) {
                    ComponentName service = context.startService(new Intent().setComponent(comp));
                    if (service == null) {
                        Log.e(TAG, "Could Not Start Service " + comp.toString());
                    } else {
                        Log.d(TAG, "Started Successfully");
                    }
                } else {
                    Log.e(TAG, "Not Started Successfully");
                }
            } else {
                    Log.e(TAG, "QTIDiagServices not supported on this platform");
            }
        }
    }
}
