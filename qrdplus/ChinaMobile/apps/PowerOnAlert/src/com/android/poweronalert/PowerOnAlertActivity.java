/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution. Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.poweronalert;

import android.app.Activity;
import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;

public class PowerOnAlertActivity  extends Activity{

    private KeyguardManager mKeyguardManager;
    private KeyguardManager.KeyguardLock mKeyguardLock;
    private static final int TIME_EXPIRED_NO_OPERATOR = 30;
    private static final int NO_OPERATOR_ALL_TIME = 100;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case NO_OPERATOR_ALL_TIME:
                    powerOff(PowerOnAlertActivity.this);
                    finish();
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.poweron_alert);
        mKeyguardManager = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        Button BtnYes = (Button) findViewById(R.id.poweron_yes);
        BtnYes.requestFocus();
        BtnYes.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                finish();
            }
        });

        Button BtnNo = (Button) findViewById(R.id.poweron_no);
        BtnNo.requestFocus();
        BtnNo.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                finish();
                powerOff(PowerOnAlertActivity.this);
            }
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        mHandler.removeMessages(NO_OPERATOR_ALL_TIME);
        mHandler.sendEmptyMessageDelayed(NO_OPERATOR_ALL_TIME,1000 * TIME_EXPIRED_NO_OPERATOR);
        disableKeyguard();

    }

    @Override
    protected void onStop() {
        super.onStop();
        mHandler.removeMessages(NO_OPERATOR_ALL_TIME);
        enableKeyguard();

    }
    private synchronized void enableKeyguard() {
        if (mKeyguardLock != null) {
            mKeyguardLock.reenableKeyguard();
            mKeyguardLock = null;
        }
    }

    private synchronized void disableKeyguard() {
        if (mKeyguardLock == null) {
            mKeyguardLock = mKeyguardManager.newKeyguardLock("PowerOnAlert");
            mKeyguardLock.disableKeyguard();
        }
    }

    /**
     * Implement power off function immediately.
     */
    private static void powerOff(Context context) {
        Intent requestShutdown = new Intent(
                Intent.ACTION_REQUEST_SHUTDOWN);
        requestShutdown.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
        requestShutdown.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(requestShutdown);
    }

    /**
     * ShutDown receiver,responsible for making sure that implement to power off
     * when power on due to alarm, and no user operation
     */
    public static class ShutDownReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(final Context context, Intent intent) {
            powerOff(context);
        }
    }

}
