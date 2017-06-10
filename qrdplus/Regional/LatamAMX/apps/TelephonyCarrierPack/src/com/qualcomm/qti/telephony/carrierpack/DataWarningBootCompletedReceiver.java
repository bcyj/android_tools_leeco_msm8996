/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.carrierpack;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

/**
 * BroadcastReceiver that starts the service to perform one time initialization
 * at bootup time.
 */
public class DataWarningBootCompletedReceiver extends BroadcastReceiver {
    private static final String TAG = "DWBCompletedReceiver";
    private static final boolean DBG = Log.isLoggable(TAG, Log.DEBUG);

    @Override
    public void onReceive(Context context, Intent intent) {

        if (DBG) Log.v(TAG, "DataWarningBootCompleted.onReceive");
        SharedPreferences sharedPref = PreferenceManager
                .getDefaultSharedPreferences(context);

        boolean isDataWarningDialogRequired =
                sharedPref.getBoolean("disable_data_warning_dialog", false);
        boolean isDataWarningDialogFeatureEnabled = context.getResources()
                .getBoolean(R.bool.config_enable_data_warning_dialog);

        if (DBG) {
            Log.d(TAG, "isDataWarningDialogRequired = " + isDataWarningDialogRequired);
            Log.d(TAG, "isDataWarningDialogFeatureEnabled = " + isDataWarningDialogFeatureEnabled);
        }

        if (isDataWarningDialogFeatureEnabled) {
            if (isDataWarningDialogRequired) {
                if (DBG) Log.d(TAG, "onCreate : data warning dialog is disabled by checkbox");
                return;
            } else {
                Intent dataWarningIntent = new Intent(context,
                        DataWarningActivity.class);
                dataWarningIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(dataWarningIntent);
            }
        } else {
            if (DBG) Log.d(TAG, "onCreate : data warning dialog is disabled via config");
        }
    }
}
