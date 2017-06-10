/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.carrierpack;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;

public class DataWarningActivity extends Activity {
    private static final String TAG = DataWarningActivity.class.getSimpleName();
    private static final boolean DBG = Log.isLoggable(TAG, Log.DEBUG);
    private Button mButton = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (DBG) Log.d(TAG, "onCreate : Inflate view");

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        View mView = getLayoutInflater().inflate(R.layout.data_usage_warning, null);
        setContentView(mView);

        // Text view to add the link to web url
        TextView alertDialogMessage = (TextView) mView
                .findViewById(R.id.data_usage_information);
        alertDialogMessage.setMovementMethod(LinkMovementMethod.getInstance());

        // ok button
        mButton = (Button) findViewById(R.id.button);
        mButton.setOnClickListener(mClicked);

        // Add checkbox listner
        CheckBox cb = (CheckBox) mView.findViewById(R.id.checkbox);
        cb.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                SharedPreferences sharedPref = PreferenceManager
                        .getDefaultSharedPreferences(getApplicationContext());
                final SharedPreferences.Editor sharedPrefEditor = sharedPref
                        .edit();

                if (DBG) Log.d(TAG, "onCheckedChanged : isChecked = " + isChecked);
                if (isChecked) {
                    sharedPrefEditor.putBoolean("disable_data_warning_dialog", true);
                    sharedPrefEditor.commit();
                } else {
                    sharedPrefEditor.putBoolean("disable_data_warning_dialog", false);
                    sharedPrefEditor.commit();
                }
            }
        });
    }

    private View.OnClickListener mClicked = new View.OnClickListener() {
        public void onClick(View v) {
            if (v == mButton) {
                if (DBG) Log.d(TAG, "onClick : finish activity");
                finish();
            }
        }
    };
} // end of class
