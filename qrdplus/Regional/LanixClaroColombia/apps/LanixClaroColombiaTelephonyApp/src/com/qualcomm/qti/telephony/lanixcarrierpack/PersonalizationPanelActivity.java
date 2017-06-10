/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.lanixcarrierpack;

import java.io.IOException;
import java.nio.ByteOrder;
import java.util.HashMap;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.Toast;

import com.qualcomm.qcrilhook.BaseQmiTypes.*;
import com.qualcomm.qcrilhook.QmiOemHook;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.*;

public class PersonalizationPanelActivity extends Activity {
    private static final String TAG = "Persopanel";
    // Emergency dialer activity action
    private static final String ACTION_EMERGENCY_DIAL = "com.android.phone.EmergencyDialer.DIAL";
    private final int SUCCESS = 0;
    private final int FAILURE = 1;

    private EditText mPinEditText;
    private View mLockBtn = null;

    private QcRilHookInterface mQcRilHookInterface;
    private QcRilHookInterface.QcrilHookInterfaceListener mListener
            = new QcRilHookInterface.QcrilHookInterfaceListener() {

        @Override
        public void onQcRilHookReady() {
            log("onQcRilHookReady");
            mLockBtn.setEnabled(true);
        }
    };

    private void setGWNWPersonalization(String pin) {
        AsyncTask<String, Void, Integer> personalizationTask
                = new AsyncTask<String, Void, Integer>() {
            @Override
            protected Integer doInBackground(String... params) {
                String pin = params[0];
                return mQcRilHookInterface.setGWNWPersonalization(pin);
            }

            protected void onPostExecute(Integer result) {
                int status = result.intValue();
                setMainViewVisible(true);
                if (status == Integer.MAX_VALUE) {// success
                    showDialog(SUCCESS,getString(R.string.perso_success_msg),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog,
                                        int which) {
                                    finish();
                                }
                            });
                } else if (status >= 0) {
                    // failure-with retry count
                    mPinEditText.getText().clear();
                    showDialog(FAILURE,
                            getString(R.string.perso_failure_msg, status),
                            null);
                    updateLockBtn(status);
                } else {
                    // Internal failure
                    Toast.makeText(PersonalizationPanelActivity.this,
                            getString(R.string.perso_internal_failure_msg),
                            Toast.LENGTH_LONG).show();
                }
            };
        };
        personalizationTask.execute(pin);
    }
    private OnClickListener mOnLockClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            String pin = mPinEditText.getText().toString();
            if (pin != null
                    && pin.length() == getResources()
                    .getInteger(R.integer.perso_unlock_max_chars)) {
                setMainViewVisible(false);
                setGWNWPersonalization(pin);
            } else {
                Toast.makeText(
                        PersonalizationPanelActivity.this,
                        getString(R.string.perso_text_length_message,
                                getResources().getInteger(R.integer.perso_unlock_max_chars)),
                        Toast.LENGTH_LONG).show();
            }
        }
    };

    private OnClickListener mOnDismissClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            finish();
        }
    };

    private OnClickListener mOnEmergencyBtnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Intent emergencyDialerIntent = new Intent(ACTION_EMERGENCY_DIAL);
            startActivity(emergencyDialerIntent);
            finish();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.personalization_layout);

        mPinEditText = (EditText) findViewById(R.id.pin_entry);

        mLockBtn = findViewById(R.id.np_lock);
        mLockBtn.setOnClickListener(mOnLockClickListener);

        View emergencyBtn = findViewById(R.id.ndp_emergency);
        emergencyBtn.setOnClickListener(mOnEmergencyBtnClickListener);

        View dismissBtn = findViewById(R.id.ndp_dismiss);
        dismissBtn.setOnClickListener(mOnDismissClickListener);

        mLockBtn.setEnabled(false);
        mQcRilHookInterface = new QcRilHookInterface(this, mListener);
    }

    private void updateLockBtn(int intExtra) {
        mLockBtn.setEnabled(intExtra > 0);
    }

    private void setMainViewVisible(boolean mainViewVisble) {
        findViewById(R.id.main_view).setVisibility(mainViewVisble
            ? View.VISIBLE : View.GONE);
        findViewById(R.id.processing_view).setVisibility(mainViewVisble
            ? View.GONE : View.VISIBLE);
    }

    @Override
    public void finish() {
        try {
            mQcRilHookInterface.dispose();
        } catch (Exception e) {
            e.printStackTrace();
        }
        super.finish();
    }

    private void showDialog(int type, String message, DialogInterface.OnClickListener listener) {
        AlertDialog.Builder builder = new AlertDialog.Builder(
                PersonalizationPanelActivity.this);
        builder.setCancelable(false);
        builder.setTitle((type == SUCCESS ? R.string.perso_lock : R.string.incorrect_code));
        builder.setMessage(message);
        builder.setPositiveButton(R.string.ok,(type == SUCCESS ? listener : null));
        builder.create().show();
    }

    private void log(String message) {
        Log.i(TAG, message);
    }
}
