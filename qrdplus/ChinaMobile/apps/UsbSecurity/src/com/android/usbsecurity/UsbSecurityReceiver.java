/*
Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*/

package com.android.usbsecurity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.usb.UsbManager;
import android.os.SystemProperties;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

public class UsbSecurityReceiver extends BroadcastReceiver {
    private static final String TAG = "UsbSecurity";
    private static boolean mIsLastConnected = false;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (UsbManager.ACTION_USB_STATE.equals(action)) {
            boolean isConnected = intent.getExtras().getBoolean(UsbManager.USB_CONNECTED, false);
            boolean isSimCardInserted = SystemProperties.getBoolean(
                    "persist.sys.sim.activate", false);
            boolean isUsbSecurityEnable = SystemProperties.getBoolean(
                    "persist.sys.usb.security", false);
            Log.d(TAG, "ACTION_USB_STATE" + !isSimCardInserted + isUsbSecurityEnable + isConnected
                    + mIsLastConnected);
            // 1 Pre-conditions: User never insert any sim card, user plug in usb cable.
            // 2 Expect result: Pop up a dialog to notify user.
            // 3 When persist.sys.usb.security is true, this feature is enable.
            // 4 When plug in usb cable, check and uncheck "Charge only", will cause DUT send
            // USB_CONNECTED intent, dialog will pop up again when user click OK. Use
            // mIsLastConnected to avoid it.
            if ((!isSimCardInserted) && isUsbSecurityEnable && isConnected && !mIsLastConnected) {
                Intent UsbSecurityActivityIntent = new Intent();
                UsbSecurityActivityIntent.setClass(context, UsbSecurityActivity.class);
                UsbSecurityActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(UsbSecurityActivityIntent);
            }
            mIsLastConnected = isConnected;
        } else if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            boolean isSimCardInserted = IccCardConstants.INTENT_VALUE_ICC_READY.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_IMSI.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra);
            if (isSimCardInserted) {
                SystemProperties.set("persist.sys.sim.activate", "true");
            }
        }
    }
}
