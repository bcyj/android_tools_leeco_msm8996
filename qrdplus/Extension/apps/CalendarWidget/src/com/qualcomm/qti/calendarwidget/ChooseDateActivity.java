/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.format.Time;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.DatePicker.OnDateChangedListener;

public class ChooseDateActivity extends Activity implements OnClickListener, OnDateChangedListener {
    public static final String EXTRA_YEAR = "year";
    public static final String EXTRA_MONTH = "month";

    private DatePicker mPicker;
    private Button mCancel;
    private Button mOk;

    private int mYear;
    private int mMonth;

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        if (bundle == null) {
            Intent intent = getIntent();
            Time time = new Time();
            time.setToNow();
            mYear = intent.getIntExtra(EXTRA_YEAR, time.year);
            mMonth = intent.getIntExtra(EXTRA_MONTH, time.month);
        }

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.choose_date);
        getWindow().setLayout(WindowManager.LayoutParams.WRAP_CONTENT,
                              WindowManager.LayoutParams.WRAP_CONTENT);

        mCancel = (Button) findViewById(R.id.cancel);
        mCancel.setOnClickListener(this);
        mOk = (Button) findViewById(R.id.ok);
        mOk.setOnClickListener(this);

        mPicker = (DatePicker) findViewById(R.id.picker);
        mPicker.init(mYear, mMonth, 1, this);
    }

    /**
     * Caused by we didn't want it could go to background.
     * So we will finish it if it is onPause.
     */
    @Override
    protected void onPause() {
        super.onPause();
        finish();
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.ok) {
            // When click ok button, the user may be edit the
            // year or month, we should get the update year
            // and month and set to the widget.
            if (mPicker != null) {
                mPicker.clearFocus();
                mYear = mPicker.getYear();
                mMonth = mPicker.getMonth();
            }
            WidgetManager.setDate(this, mYear, mMonth);
        }
        finish();
    }

    @Override
    public void onDateChanged(DatePicker view, int year, int monthOfYear, int dayOfMonth) {
        mYear = year;
        mMonth = monthOfYear;
    }

}
