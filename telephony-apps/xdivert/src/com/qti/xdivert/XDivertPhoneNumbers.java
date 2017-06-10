/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.xdivert;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.text.Selection;
import android.text.Spannable;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import android.content.Context;
// This class handles the user entry for phone numbers.

public class XDivertPhoneNumbers extends Activity {
    private static final String LOG_TAG = "XDivertPhoneNumbers";
    private static final boolean DBG = false;

    private EditText[] mLine1Numbers;
    private Button mButton;
    int mNumPhones;
    XDivertUtility mXDivertUtility;
    Intent resultIntent;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
       Intent intent = getIntent();
        resultIntent = new Intent();
        setResult(RESULT_CANCELED, resultIntent);
        mNumPhones = TelephonyManager.getDefault().getPhoneCount();
        mXDivertUtility = XDivertUtility.getInstance();
        setContentView(R.layout.xdivert_phone_numbers);
        // If the sim has been changed & user immediately enters XDivert screen after configuring
        // the new sim (but before SIM_RECORDS_LOADED event), check the imsi's again &
        // update the shared pref & numbers accordingly.
        boolean isImsiReady = mXDivertUtility.checkImsiReady();

        Log.d(LOG_TAG,"onCreate isImsiReady = " + isImsiReady);
        if (!isImsiReady) {
            displayAlertDialog(R.string.xdivert_imsi_not_read);
        } else {
            setupView();
        }
    }

    public void processPhoneNumbers() {
       Intent intent = getIntent();
        // If the sim has been changed & user immediately enters XDivert screen after configuring
        // the new sim (but before SIM_RECORDS_LOADED event), check the imsi's again &
        // update the shared pref & numbers accordingly.
        boolean isImsiReady = mXDivertUtility.checkImsiReady();
        Log.d(LOG_TAG,"onCreate isImsiReady = " + isImsiReady);
        if (!isImsiReady) {
            displayAlertDialog(R.string.xdivert_imsi_not_read);
        } else {
            setupView();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    private void displayAlertDialog(int resId) {
        new AlertDialog.Builder(this).setMessage(resId)
            .setTitle(R.string.xdivert_title)
            .setIcon(android.R.drawable.ic_dialog_alert)
            .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(LOG_TAG, "XDivertPhoneNumbers onClick");
                    }
                })
            .show()
            .setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        Log.d(LOG_TAG, "XDivertPhoneNumbers onDismiss");
                        finish();
                    }
            });
    }

    private void setupView() {
        int numberEditTextId[] = {R.id.sub1_number, R.id.sub2_number};

        mLine1Numbers = new EditText[mNumPhones];
        // Get the lineNumbers from XDivertUtility
        // lineNumbers will be returned if they were previously stored in shared preference
        // else null will be returned.
        String[] subLine1Number = mXDivertUtility.getLineNumbers();
        for (int i = 0; i < mNumPhones; i++) {
            Log.d(LOG_TAG,"setupView sub" + (i+1) + " line number = " + subLine1Number[i]);
            mLine1Numbers[i] = (EditText) findViewById(numberEditTextId[i]);
            if (mLine1Numbers[i] != null) {
                mLine1Numbers[i].setText(subLine1Number[i]);
                mLine1Numbers[i].setOnFocusChangeListener(mOnFocusChangeHandler);
                mLine1Numbers[i].setOnClickListener(mClicked);
            }
        }

        mButton = (Button) findViewById(R.id.button);
        if (mButton != null) {
            mButton.setOnClickListener(mClicked);
        }
    }

    private String[] getLine1Numbers() {
        String[] line1Numbers = new String[mNumPhones];
        for (int i = 0; i < mNumPhones; i++) {
            line1Numbers[i] = mLine1Numbers[i].getText().toString();
        }
        return line1Numbers;
    }

    private void processXDivert() {
        Log.d(LOG_TAG," processXDivert () ");
        resultIntent.putExtra(XDivertUtility.LINE1_NUMBERS, getLine1Numbers());
        setResult(RESULT_OK, resultIntent);
        finish();
    }

    private boolean isValidNumbers() {
        Log.d(LOG_TAG," isValidNumbers  ");
        for (int i = 0; i < mNumPhones; i++) {
            String num = mLine1Numbers[i].getText().toString();
            if (android.text.TextUtils.isEmpty(num)) return false;
        }
        return true;
    }

    private View.OnClickListener mClicked = new View.OnClickListener() {
        public void onClick(View v) {
            Log.d(LOG_TAG," onClick  ");
            if (v == mLine1Numbers[0]) {
                Log.d(LOG_TAG," onClick1  ");
                mLine1Numbers[1].requestFocus();
            } else if (v == mLine1Numbers[1]) {
                Log.d(LOG_TAG," onClick2  ");
                mButton.requestFocus();
            } else if (v == mButton) {
                Log.d(LOG_TAG," onClick 3 ");
                if (!isValidNumbers()) {
                    Log.d(LOG_TAG," !isValidNumbers  ");
                    Toast toast = Toast.makeText(getApplicationContext(),
                            R.string.xdivert_enternumber_error,
                            Toast.LENGTH_LONG);
                    toast.show();
                } else {
                    Log.d(LOG_TAG," processXDivert  ");
                    processXDivert();
                }
            }
        }
    };

    View.OnFocusChangeListener mOnFocusChangeHandler =
            new View.OnFocusChangeListener() {
        public void onFocusChange(View v, boolean hasFocus) {
            if (hasFocus) {
                TextView textView = (TextView) v;
                Selection.selectAll((Spannable) textView.getText());
            }
        }
    };
}
