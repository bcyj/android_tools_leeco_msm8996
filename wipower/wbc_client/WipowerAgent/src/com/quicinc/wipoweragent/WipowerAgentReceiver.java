/*=========================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wipoweragent;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class WipowerAgentReceiver extends BroadcastReceiver {
    private static final boolean DBG = true;
    private static final String TAG = "WiPwrAg-Rcvr";

    public WipowerAgentReceiver() {

    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (SystemProperties.getBoolean("ro.bluetooth.wipower", false) == false) {
            if (DBG) Log.w(TAG, "wipower not supported, ignoring intent:" + intent.getAction());
            return;
        }

        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            if (DBG) Log.v(TAG, "Action: Boot completed");
            context.startService(new Intent(context, WipowerAgentService.class));
        } else if  (intent.getAction().equals("com.quicinc.wbc.action.SHOW_BLUETOOTH_NEEDED_UI_DIALOG")) {
            if (DBG) Log.v(TAG, "Action: SHOW_BLUETOOTH_NEEDED_UI_DIALOG");
            Intent btIntent = new Intent(context, WipowerAgentActivity.class);
            btIntent.setAction(intent.getAction());
            btIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(btIntent);
        } else if (intent.getAction().equals("com.quicinc.wbc.action.WIPOWER_ICON_ENABLE")) {
            if (DBG) Log.v(TAG, "Action: WIPOWER_ICON_ENABLE");
            Intent svcIntent = new Intent(context, WipowerAgentService.class);
            svcIntent.setAction(intent.getAction());
            context.startService(svcIntent);
        } else if (intent.getAction().equals("com.quicinc.wbc.action.WIPOWER_ICON_DISABLE")) {
            if (DBG) Log.v(TAG, "Action: WIPOWER_ICON_DISABLE");
            Intent svcIntent = new Intent(context, WipowerAgentService.class);
            svcIntent.setAction(intent.getAction());
            context.startService(svcIntent);
        }
    }
}
