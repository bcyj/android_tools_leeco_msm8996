/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.stk;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

/**
 * When STK app is not present in system, this app acts like a proxy STK app.
 */
public class StkLauncherActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Toast.makeText(this,
                getResources().getString(R.string.app_unavailable_msg),
                Toast.LENGTH_LONG).show();
        finish();
    }
}
