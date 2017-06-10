/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;

public class ConfirmDialog extends Activity implements OnClickListener, OnCancelListener {

    private static final String TAG = "ConfirmDialog";

    private static final String ACTION_DISABLE_TDD_LTE =
            "com.qualcomm.qti.phonefeature.DISABLE_TDD_LTE";

    private static final int DISABLE_LTE_DATA_ONLY = 1;

    private AlertDialog mDialogToDisableTDD;

    private ProgressDialog mProgressDialog;
    private int mSlot;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case DISABLE_LTE_DATA_ONLY:
                    finish();
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSlot = getIntent().getIntExtra(PhoneConstants.SLOT_KEY, 0);
        if (ACTION_DISABLE_TDD_LTE.equals(getIntent().getAction())) {
            alertTDDDataOnly();
        } else {
            finish();
        }
    }

    private void alertTDDDataOnly() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        AlertDialog dialog = builder.setTitle(R.string.alert_title)
                .setOnCancelListener(this)
                .setMessage(R.string.message_tdd_data_only)
                .setNegativeButton(R.string.choose_no, this)
                .setPositiveButton(R.string.choose_yes, this).create();
        dialog.show();
        mDialogToDisableTDD = dialog;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (mDialogToDisableTDD == dialog && which == DialogInterface.BUTTON_POSITIVE) {
            disableTddOnly();
        } else {
            finish();
        }
    }

    private void disableTddOnly() {
        int prefNetwork = -1;
        try {
            prefNetwork = TelephonyManager.getIntAtIndex(getContentResolver(),
                    Constants.SETTING_DEFAULT_PREF_NETWORK_MODE, mSlot);
        } catch (SettingNotFoundException e) {
            Log.e(AppGlobals.TAG, "disableTddOnly: can not previous network", e);
            finish();
            return;
        }
        showProgressDialog();
        PrefNetworkRequest request = new PrefNetworkRequest(this, mSlot, prefNetwork,
                mHandler.obtainMessage(DISABLE_LTE_DATA_ONLY));
        request.setBand(Constants.NW_BAND_LTE_FDD);
        try {
            request.setAcqOrder(TelephonyManager.getIntAtIndex(getContentResolver(),
                    Constants.SETTING_ACQ, mSlot));
        } catch (SettingNotFoundException e) {
        }
        request.loop();
    }

    private void showProgressDialog() {
        mProgressDialog = new ProgressDialog(this);
        mProgressDialog.setIndeterminate(true);
        mProgressDialog.setCancelable(false);
        mProgressDialog.show();
    }

    private void dismissProgressDialog() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        dismissProgressDialog();
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        finish();
    }
}
