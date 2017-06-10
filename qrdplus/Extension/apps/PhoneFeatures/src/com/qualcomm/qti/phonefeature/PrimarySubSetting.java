/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout.LayoutParams;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

public class PrimarySubSetting extends Activity implements View.OnClickListener {

    private TextView mRecognizeText;
    private RadioGroup mRadioGroup;
    private Button mOKbutton;
    private CheckBox mDdsChecBox;
    private ProgressDialog mProgressDialog;
    private PrimarySubPolicy mPrimarySubPolicy;

    private static final int SET_LTE_SUB_MSG = 1;

    private static final String CONFIG_ACTION = "com.qualcomm.qti.phonefeature.LTE_CONFIGURE";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_sub_select);
        mPrimarySubPolicy = PrimarySubPolicy.getInstance(this);
        mRecognizeText = (TextView) findViewById(R.id.recognize_text);

        mRadioGroup = (RadioGroup) findViewById(R.id.radiogroup);
        for (int i = 0; i < Constants.PHONE_COUNT; i++) {
            RadioButton radioButton = new RadioButton(this);
            mRadioGroup.addView(radioButton, new LayoutParams(LayoutParams.WRAP_CONTENT,
                    LayoutParams.WRAP_CONTENT));
            radioButton.setTag(i);
            radioButton.setText(Constants.getSimName(this, i));
            radioButton.setOnClickListener(this);
        }
        mDdsChecBox = (CheckBox) findViewById(R.id.lte_checkBox);
        if (CONFIG_ACTION.equals(getIntent().getAction())) {
            setTitle(R.string.lte_select_title);
            mRecognizeText.setVisibility(View.GONE);
        } else {
            setTitle(R.string.lte_recognition_title);
            mDdsChecBox.setVisibility(View.GONE);
        }

        mOKbutton = (Button) findViewById(R.id.select_ok_btn);
        mOKbutton.setOnClickListener(this);

        mProgressDialog = new ProgressDialog(this);
        mProgressDialog.setMessage(this.getString(R.string.lte_setting));
        mProgressDialog.setCancelable(false);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateState();
    }

    private void updateState() {
        boolean primaryEnabled = mPrimarySubPolicy.isPrimaryEnabled()
                && mPrimarySubPolicy.isPrimarySetable()
                && mPrimarySubPolicy.getPrefPrimarySub() == -1;
        int current = mPrimarySubPolicy.getPrimarySub();
        mRadioGroup.clearCheck();
        for (int i = 0; i < mRadioGroup.getChildCount(); i++) {
            RadioButton radioButton = (RadioButton) mRadioGroup.getChildAt(i);
            radioButton.setChecked(current == i);
            radioButton.setEnabled(primaryEnabled);
        }
        mDdsChecBox.setEnabled(primaryEnabled);
        mOKbutton.setEnabled(primaryEnabled && current != -1);
        mOKbutton.setTag(current);
        if (!primaryEnabled) {
            Toast.makeText(
                    this, getString(R.string.lte_switch_unavailable), Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public boolean onNavigateUp() {
        finish();
        return true;
    }

    public void onClick(View v) {
        if (v instanceof RadioButton) {
            int sub = (Integer) v.getTag();
            mOKbutton.setTag(sub);
            mOKbutton.setEnabled(true);
        } else if (v == mOKbutton) {
            if (mPrimarySubPolicy.setPrimarySub((Integer) mOKbutton.getTag(),
                    mHandler.obtainMessage(SET_LTE_SUB_MSG))) {
                mProgressDialog.show();
            }
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_LTE_SUB_MSG:
                    int targetSub = (Integer) mOKbutton.getTag();
                    if (targetSub != mPrimarySubPolicy.getPrimarySub()) {
                        showFailedDialog(targetSub);
                    } else {
                        if (mDdsChecBox.isChecked()) {
                            SubscriptionManager.from(PrimarySubSetting.this)
                                    .setDefaultDataSubId(targetSub);
                        }
                        Toast.makeText(PrimarySubSetting.this, getString(R.string.reg_suc),
                                Toast.LENGTH_LONG).show();
                    }
                    updateState();
                    mProgressDialog.dismiss();
                    break;
                default:
                    break;
            }
        }
    };

    private void showFailedDialog(int sub) {
        if (CONFIG_ACTION.equals(getIntent().getAction())) {
            AlertDialog alertDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.reg_failed)
                    .setMessage(getString(R.string.reg_failed_msg, Constants.getSimName(this, sub)))
                    .setNeutralButton(R.string.select_ok, null)
                    .create();
            alertDialog.show();
        } else {
            AlertDialog alertDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.reg_failed)
                    .setMessage(getString(R.string.reg_failed_msg, Constants.getSimName(this, sub)))
                    .setNeutralButton(R.string.lte_set, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            String action = "com.android.settings.MULTI_SIM_SETTINGS";
                            Intent intent = new Intent(action);
                            startActivity(intent);
                            finish();
                        }
                    })
                    .setNegativeButton(R.string.select_cancel,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    finish();
                                }
                            })
                    .create();
            alertDialog.show();
        }
    }
}
