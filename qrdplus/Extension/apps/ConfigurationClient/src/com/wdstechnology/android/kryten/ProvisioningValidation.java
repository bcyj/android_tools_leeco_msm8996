/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
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

package com.wdstechnology.android.kryten;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;

import com.wdstechnology.android.kryten.R.string;
import com.wdstechnology.android.kryten.security.OmaSigner;
import com.wdstechnology.android.kryten.security.UnknownSecurityMechanismException;
import com.wdstechnology.android.kryten.utils.HexConvertor;

public class ProvisioningValidation extends Activity {

    private Button mCancel;
    private Button mOk;
    private EditText mUserSuppliedPin;
    private TextView mPin;
    private OmaSigner mSigner;
    private byte[] mMac;
    private byte[] mDocument;
    private Dialog mBadUserPinDialog;
    private String from;
    private static int pinCounter = 0;
    private long id;
    private String TAG = "ProvisioningValidation";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreate");
        // requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.enter_pin);

        mPin = (TextView) findViewById(R.id.please_enter_pin);

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP, ActionBar.DISPLAY_HOME_AS_UP);
            actionBar.setCustomView(R.layout.banner);
            actionBar.setTitle(getString(R.string.banner_title));
        }

        mBadUserPinDialog = new AlertDialog.Builder(ProvisioningValidation.this).setIcon(
                R.drawable.icon).setTitle(R.string.bad_pin_title).setPositiveButton(R.string.ok,
                null).setMessage(R.string.bad_pin_message).create();

        mUserSuppliedPin = (EditText) findViewById(R.id.user_supplied_pin);
        mOk = (Button) findViewById(R.id.pin_ok);

        mOk.setEnabled(false);
        mUserSuppliedPin.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence text, int arg1, int arg2, int arg3) {
                // TODO Auto-generated method stub
                if (text != null && text.length() >= 1)
                {
                    mOk.setEnabled(true);
                }
                else
                    mOk.setEnabled(false);
            }

            @Override
            public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                    int arg3) {
                // TODO Auto-generated method stub

            }

            @Override
            public void afterTextChanged(Editable arg0) {
                // TODO Auto-generated method stub

            }
        });
        mUserSuppliedPin.setOnKeyListener(new View.OnKeyListener() {

            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_ENTER && KeyEvent.ACTION_DOWN == event.getAction()) {
                    pinEntered();
                    return true;
                } else {
                    return false;
                }
            }
        });

        mCancel = (Button) findViewById(R.id.pin_cancel);
        mCancel.setOnClickListener(new View.OnClickListener() {

            AlertDialog.Builder about;

            @Override
            public void onClick(View v) {
                about = new AlertDialog.Builder(ProvisioningValidation.this);

                about.setTitle(getString(R.string.app_name));
                about.setMessage(getString(R.string.ignore_message));
                about.setPositiveButton(getString(R.string.ok),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                                ProvisioningNotification
                                        .clearNotification(ProvisioningValidation.this);
                                finish();
                            }
                        });
                about.show();
            }
        });

        mOk.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                pinEntered();
            }

        });
    }

    @Override
    protected void onStart() {
        super.onStart();

        Intent myIntent = getIntent();
        String secString = myIntent.getStringExtra("com.wdstechnology.android.kryten.SEC");
        mDocument = myIntent
                .getByteArrayExtra("com.wdstechnology.android.kryten.provisioning-data");
        String macString = myIntent.getStringExtra("com.wdstechnology.android.kryten.MAC");
        from = myIntent.getStringExtra("from");
        id = myIntent.getLongExtra("id", -1);
        Log.i(TAG, "secString " + secString + " macString " + macString);
        Log.i(TAG, "from " + from + " id " + id);
        if ((secString == null) || (secString.length() == 0)) {
            ProvisioningNotification.clearNotification(ProvisioningValidation.this);
            StoreProvisioning.provision(ProvisioningValidation.this, mDocument, from, id);
            finish();
            return;
        }

        int sec = -1;
        if (secString != null && !secString.equals(""))
            sec = Integer.valueOf(secString);

        if (macString != null) {
            mMac = HexConvertor.convert(macString);
        } else {
            mMac = null;
        }

        Log.i(TAG, "sec " + sec + " mMac " + mMac);

        TelephonyManager tm = (TelephonyManager) ProvisioningValidation.this
                .getSystemService(Context.TELEPHONY_SERVICE);

        try {
            if (sec != -1)
                mSigner = OmaSigner.signerFor(sec, mDocument, tm.getSubscriberId());
        } catch (UnknownSecurityMechanismException e) {
            throw new RuntimeException(e);
        }

        if (mSigner != null && !mSigner.isUserPinRequired()) {
            boolean valid = mSigner.isDocumentValid(mUserSuppliedPin.getText().toString(), mMac);
            if (valid || secString == null || secString.equals("")) {
                ProvisioningNotification.clearNotification(ProvisioningValidation.this);
                StoreProvisioning.provision(ProvisioningValidation.this, mDocument, from, id);

                finish();
                return;
            } else {
                ProvisioningFailed.fail(this, R.string.bad_network_pin_message);
                finish();
                return;
            }
        }

    }

    private void pinEntered() {
        Log.i(TAG, "mSigner " + mSigner + " mUserSuppliedPin " + mUserSuppliedPin);
        if (mSigner != null && mSigner.isDocumentValid(mUserSuppliedPin.getText().toString(), mMac)) {
            ProvisioningNotification.clearNotification(ProvisioningValidation.this);
            StoreProvisioning.provision(ProvisioningValidation.this, mDocument, from, id);
            finish();
            return;
        } else {
            mPin.setText(getString(R.string.please_enter_pin));
            mUserSuppliedPin.setText("");
            mBadUserPinDialog.show();
        }
    }

    public static PendingIntent createPendingValidationActivity(Context context, String sec,
            String mac, byte[] document, String from) {

        Intent validationIntent = new Intent(context, ProvisioningValidation.class);
        validationIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (sec != null) {
            validationIntent.putExtra("com.wdstechnology.android.kryten.SEC", sec);
        }
        if (mac != null) {
            validationIntent.putExtra("com.wdstechnology.android.kryten.MAC", mac);
        }
        validationIntent.putExtra("com.wdstechnology.android.kryten.provisioning-data", document);
        validationIntent.putExtra("from", from);

        return PendingIntent.getActivity(context, 0, validationIntent,
                PendingIntent.FLAG_CANCEL_CURRENT);

    }

}
