/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.provider.CalendarContract;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

public class NoAccountAlert extends Activity implements OnClickListener {

    private Button mCancel;
    private Button mAddAccount;

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.no_account);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.WRAP_CONTENT);

        mCancel = (Button) findViewById(R.id.cancel);
        mCancel.setOnClickListener(this);
        mAddAccount = (Button) findViewById(R.id.add_account);
        mAddAccount.setOnClickListener(this);
    }

    /**
     * Caused by we didn't want it could go to background. So we will finish it
     * if it is onPause.
     */
    @Override
    protected void onPause() {
        super.onPause();
        finish();
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.add_account) {
            Intent nextIntent = new Intent(Settings.ACTION_ADD_ACCOUNT);
            nextIntent.putExtra(Settings.EXTRA_AUTHORITIES,
                    new String[] { CalendarContract.AUTHORITY });
            nextIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(nextIntent);
        }
        finish();
    }
}
