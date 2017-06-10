/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.TelephonyIntents;

public class TriggerActionReceiver extends BroadcastReceiver {
    private static final String TAG = "TriggerActionReceiver";

    private static final String ACTION_TRIGGER = "com.qualcomm.qti.loadcarrier.trigger";
    private static final String ACTION_PHONE_READY = "com.android.phone.ACTION_PHONE_READY";

    private static final String HOST_TRIGGER_Y     = "87444379";
    private static final String HOST_TRIGGER_N     = "87444376";
    private static final String HOST_TRIGGER_VALUE = "874443782583";
    private static final String HOST_TRIGGER_START = "874443778278";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG, "Receive the action: " + action);

        if (ACTION_TRIGGER.equals(action)
                || ACTION_PHONE_READY.equals(action)) {
            // If the trigger function has been enabled, we will start the service.
            boolean defaultValue = context.getResources().getBoolean(R.bool.trigger_enabled);
            if (Utils.getValue(Utils.PROP_KEY_TRIGGER, defaultValue)
                    && Settings.Global.getInt(context.getContentResolver(),
                            Settings.Global.AIRPLANE_MODE_ON, 0) == 0) {
                if (Utils.DEBUG) Log.i(TAG, "Start the trigger service.");
                context.startService(new Intent(context, TriggerService.class));
            }
        } else if (TelephonyIntents.SECRET_CODE_ACTION.equals(action)) {
            Uri uri = intent.getData();
            String host = uri != null ? uri.getHost() : null;
            if (HOST_TRIGGER_Y.equals(host)) {
                // Enable the trigger function.
                Utils.setValue(Utils.PROP_KEY_TRIGGER, String.valueOf(true));
                Toast.makeText(context, R.string.toast_trigger_enabled, Toast.LENGTH_LONG).show();
            } else if (HOST_TRIGGER_N.equals(host)) {
                // Disable the trigger function.
                Utils.setValue(Utils.PROP_KEY_TRIGGER, String.valueOf(false));
                Toast.makeText(context, R.string.toast_trigger_disabled, Toast.LENGTH_LONG).show();
            } else if (HOST_TRIGGER_VALUE.equals(host)) {
                // Start the activity to edit the values used for trigger.
                Intent i = new Intent(Intent.ACTION_EDIT);
                i.setClass(context, EditTriggerValuesActivity.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(i);
            } else if (HOST_TRIGGER_START.equals(host)) {
                // As used the secret code to start trigger, do not check the prop value.
                context.startService(new Intent(context, TriggerService.class));
                Toast.makeText(context, R.string.toast_trigger_start, Toast.LENGTH_LONG).show();
            }
        }
    }

}
