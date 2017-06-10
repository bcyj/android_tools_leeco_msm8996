/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.lanixcarrierpack;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.*;
import com.qualcomm.qcrilhook.QmiOemHook;
import com.qualcomm.qcrilhook.QmiOemHookConstants;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.InvalidParameterException;
import java.util.HashMap;

public class AutoPersoSwitchActivity extends Activity {
    private static final String TAG = "AutoPersoSwitchActivity";

    private static final short SERVICE_ID = 0x0B;
    private static final short MESSAGE_ID = 0x004C;

    QcRilHookInterface mQcrilHookInterface;
    QcRilHookInterface.QcrilHookInterfaceListener mListener
            = new QcRilHookInterface.QcrilHookInterfaceListener() {

        @Override
        public void onQcRilHookReady() {
            log("onQcRilHookReady");
            startSelection();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mQcrilHookInterface = new QcRilHookInterface(this, mListener);
    }

    private void startSelection() {
        boolean status = mQcrilHookInterface.isDeviceNetworkPersonalized();
        if (status) {
            startDepersonalizationPanel();
        } else {
            startPersonalizationPanel();
        }

        finish();
    }

    private void startDepersonalizationPanel() {
        Intent intent = new Intent(
                "org.codeaurora.carrier.ACTION_DEPERSO_PANEL");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        AutoPersoSwitchActivity.this.startActivity(intent);
    }

    private void startPersonalizationPanel() {
        Intent intent = new Intent("org.codeaurora.carrier.ACTION_PERSO_PANEL");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        AutoPersoSwitchActivity.this.startActivity(intent);
    }

    @Override
    public void finish() {
        try {
            mQcrilHookInterface.dispose();
        } catch (Exception e) {
            e.printStackTrace();
        }
        super.finish();
    }

    private void log(String log) {
        Log.i(TAG, log);
    }
}
