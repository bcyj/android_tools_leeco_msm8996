/******************************************************************************
 * @file    SetCdmsSubSrc.java
 * @brief   Provides option to set CDMA subscription source.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.qualcommsettings;

import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.RadioButton;
import android.util.Log;

public class SetCdmaSubSrc extends Activity {

    private static final String LOG_TAG = "SetCdmaSubSrc";

    // RadioButtons
    private static RadioButton radioBtnRuimSim = null;
    private static RadioButton radioBtnNv = null;
    private static RadioButton radioBtnAuto = null;
    // EditText
    private static EditText editTxtSpc = null;
    // Buttons
    private static Button btnOk = null;
    private static Button btnCancel = null;

    // Used for CDMA subscription source
    private static final int CDMA_SUBSCRIPTION_RUIM_SIM      = 0; // RUIM/SIM
    private static final int CDMA_SUBSCRIPTION_NV            = 1; // NV -> non-volatile memory
    private static final int CDMA_SUBSCRIPTION_RUIM_IF_AVAIL = 2; // Use RUIM if available

    private static final int PREFERRED_CDMA_SUBSCRIPTION = CDMA_SUBSCRIPTION_NV;

    // Current CDMA subscription source
    private static int mCurrCdmaSubsSrc = CDMA_SUBSCRIPTION_NV;

    private QcRilHook mQcRilHook;
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.set_cdma_sub_src);

        mContext = getApplicationContext();
        mQcRilHook = new QcRilHook(mContext);

        // Get reference to the radio buttons and the SPC EditText
        radioBtnRuimSim = (RadioButton) findViewById(R.id.cdma_sub_src_ruim_sim);
        radioBtnNv = (RadioButton) findViewById(R.id.cdma_sub_src_nv);
        radioBtnAuto = (RadioButton) findViewById(R.id.cdma_sub_src_ruim_if_avail);
        editTxtSpc = (EditText) findViewById(R.id.spcode);
        btnOk = (Button) findViewById(R.id.cdma_sub_src_ok_btn);
        btnCancel = (Button) findViewById(R.id.cdma_sub_src_cancel_btn);

        // Read the current CDMA subscription source
        mCurrCdmaSubsSrc = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.CDMA_SUBSCRIPTION_MODE, PREFERRED_CDMA_SUBSCRIPTION);
        Log.d(LOG_TAG, "Current Cdma subscription source value is " + mCurrCdmaSubsSrc);
        setSelectedCdmaSubSrc(mCurrCdmaSubsSrc);

        btnOk.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (!setCdmaSubSrcWithSPC()) {
                    DialogInterface.OnClickListener buttonListener =
                            new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            SetCdmaSubSrc.this.finish();
                        }
                    };
                    new AlertDialog.Builder(SetCdmaSubSrc.this)
                            .setTitle(R.string.unable_to_change_alert_title)
                            .setMessage(R.string.unable_to_change_cdma_sub_src)
                            .setNeutralButton(R.string.unable_to_change_button_ok, buttonListener)
                            .show();
                } else {
                    SetCdmaSubSrc.this.finish();
                }
            }
        });

        btnCancel.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                SetCdmaSubSrc.this.finish();
            }
        });
    }

    /**
     * Check a Radio Button with the given CDMA subscription source
     *
     * @param cdmaSubSrc - CDMA subscription source
     */
    private void setSelectedCdmaSubSrc(int cdmaSubSrc) {
        switch (cdmaSubSrc) {
            case CDMA_SUBSCRIPTION_RUIM_SIM:
                radioBtnRuimSim.setChecked(true);
                break;
            case CDMA_SUBSCRIPTION_NV:
                radioBtnNv.setChecked(true);
                break;
            case CDMA_SUBSCRIPTION_RUIM_IF_AVAIL:
                radioBtnAuto.setChecked(true);
                break;
            default:
                radioBtnNv.setChecked(true);
        }
    }

    /**
     * Return the selected CDMA subscription source
     *
     * @return CDMA subscription source : int
     */
    private int getSelectedCdmaSubSrc() {
        int result;

        if (radioBtnRuimSim.isChecked()) {
            result = CDMA_SUBSCRIPTION_RUIM_SIM;
        } else if (radioBtnAuto.isChecked()) {
            result = CDMA_SUBSCRIPTION_RUIM_IF_AVAIL;
        } else {
            result = CDMA_SUBSCRIPTION_NV;
        }

        return result;
    }

    /**
     * Change CDMA subscription source with SPC
     */
    public boolean setCdmaSubSrcWithSPC() {
        int cdmaSubSrc = getSelectedCdmaSubSrc();
        String spcode = editTxtSpc.getText().toString();
        boolean result = false;

        if (cdmaSubSrc != mCurrCdmaSubsSrc && !spcode.isEmpty()) {
            result = mQcRilHook.qcRilSetCdmaSubSrcWithSpc(cdmaSubSrc, spcode);
            if (result) {
                Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.CDMA_SUBSCRIPTION_MODE, cdmaSubSrc);
            } else {
                Log.e(LOG_TAG, "Failed to set Cdma subscription source.");
            }

        }

        return result;
    }
}
