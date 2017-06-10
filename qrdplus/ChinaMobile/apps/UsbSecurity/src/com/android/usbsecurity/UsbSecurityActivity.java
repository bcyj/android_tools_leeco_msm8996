/*
Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*/

package com.android.usbsecurity;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

public class UsbSecurityActivity extends AlertActivity {

    private static final String TAG = "UsbSecurity";
    private IntentFilter mFilter;
    private BroadcastReceiver mReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        final AlertController.AlertParams mParams = mAlertParams;
        LayoutInflater vi = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mParams.mView = vi.inflate(R.layout.usb_security_dialog, null);
        mParams.mViewSpacingSpecified = true;
        mParams.mViewSpacingLeft = 15;
        mParams.mViewSpacingRight = 15;
        mParams.mViewSpacingTop = 5;
        mParams.mViewSpacingBottom = 5;
        mParams.mPositiveButtonText = getString(android.R.string.yes);
        mParams.mPositiveButtonListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        };
        setupAlert();
        mFilter = new IntentFilter();
        mFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                    String stateExtra = intent
                            .getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                    boolean isSimCardInserted = IccCardConstants.INTENT_VALUE_ICC_READY
                            .equals(stateExtra)
                            || IccCardConstants.INTENT_VALUE_ICC_IMSI.equals(stateExtra)
                            || IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra);
                    if (isSimCardInserted) {
                        Log.d(TAG, "finish by intent");
                        finish();
                    }
                }
            }
        };
        registerReceiver(mReceiver, mFilter);
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }
}
