/******************************************************************************
 * @file    DataUsage.java
 * @brief   Provides option to set data limit.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.qualcommsettings;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.util.Log;

public class DataUsage extends Activity {

    private static final String TAG = "DataUsage";
    private EditText mEdtName;
    private Button mBtnOk;
    private Button mBtnCancel;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.data_usage);
        mEdtName = (EditText) findViewById(R.id.edittext_limit);
        mEdtName.setText("0");

        mBtnOk = (Button) findViewById(R.id.ok_button);
        mBtnCancel = (Button) findViewById(R.id.cancel_button);

        mBtnOk.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                final long limitValue = Long.parseLong(mEdtName.getText().toString());
                Settings.Secure.putLong(getContentResolver(), "NETSTATS_GLOBAL_ALERT_BYTES",
                    limitValue);
                Log.i(TAG," Limit Value=" + limitValue);
                finish();
            }
        });

        mBtnCancel.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                finish();
            }
        });
    }
}
