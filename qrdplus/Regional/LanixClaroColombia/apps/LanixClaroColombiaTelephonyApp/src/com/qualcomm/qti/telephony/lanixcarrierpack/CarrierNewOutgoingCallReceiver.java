/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.lanixcarrierpack;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;

public class CarrierNewOutgoingCallReceiver extends BroadcastReceiver {

    private static final String TAG = "lanix_OCR";
    private static boolean DBG = Log.isLoggable(TAG, Log.DEBUG);

    private static final String ACTION_OEM_DEVICEINFO =
            "org.codeaurora.carrier.ACTION_OEM_DEVICEINFO";

    private Context mContext = null;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        if (intent != null && intent.getAction().equals(Intent.ACTION_NEW_OUTGOING_CALL)
                && intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER) != null) {
            if (intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                    .equals(context.getString(R.string.phone_software_code))) {
                // launch external software version activity
                log("launch external device info settings");
                launchExternalDeviceInfo();
            } else if(intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                    .equals(context.getString(R.string.phone_software_internal_code))) {
                // launch internal software version activity
                log("launch internal device info settings");
                launchInternalDeviceInfo();
            } else if (intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                    .equals(context.getString(R.string.field_test_mode_code))) {
                log("launch FTM");
                // launch field test mode activity
                launchFtm();
            } else if (intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                    .equals(context.getString(R.string.factory_mode_code))) {
                // initiate WipeUserData trigger
                log("start WipeUserData");
                startWipeUserData();
            } else if (intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                    .equals(context.getString(R.string.perso_code))) {
                log("start deperso/perso");
                startAutoSwitch();
            } else {
                // No code to handle, just return.
                return;
            }
            abortBroadcast();
            this.setResultData("");
            clearAbortBroadcast();

        }
    }

    private void startWipeUserData() {
        Intent intent = new Intent(Intent.ACTION_MASTER_CLEAR);
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        intent.putExtra(Intent.EXTRA_REASON, "MasterClearConfirm");
        mContext.sendBroadcast(intent);
    }

    private void launchFtm() {
        Intent intent = new Intent(TelephonyIntents.SECRET_CODE_ACTION,
                Uri.parse("android_secret_code://3878"));
        mContext.sendBroadcast(intent);
    }

    private void launchExternalDeviceInfo() {
        Intent oemDeviceInfointent = new Intent(ACTION_OEM_DEVICEINFO);
        oemDeviceInfointent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(oemDeviceInfointent);
    }

    private void launchInternalDeviceInfo() {
        Intent intent = new Intent(Settings.ACTION_DEVICE_INFO_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    private void startAutoSwitch() {
        Intent intent = new Intent("org.codeaurora.carrier.ACTION_AUTO_SWITCH_PERSO");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    private void log(String logMsg) {
        if (DBG) {
            Log.d(TAG, logMsg);
        }
    }

}
