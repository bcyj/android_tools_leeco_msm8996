/******************************************************************************
 * @file    MainActivity.java
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.antitheftdemo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;

public class MainActivity extends Activity implements OnEditorActionListener {

    private String password;

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            setContentView(R.layout.activity_main);
            configureEditTextView();
            showAlertDialog();
        }

    @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                hideKeyboard((EditText) v);
                String value = v.getText().toString();
                switch(v.getId()) {
                    case R.id.askConfirmationCode:
                        if(!value.equals(password)) {
                            Toast.makeText(this, R.string.text_wrong_confirmationCode_submit, Toast.LENGTH_SHORT).show();
                        } else {
                            startConfigurationActivity();
                        }
                        v.setText(null);
                        break;
                }

                return true;
            }
            return false;
        }


    private void showAlertDialog() {
        if (SettingsEditor.getPermission(MainActivity.this) == false) {
        Resources res = getResources();
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        // set title
        alertDialogBuilder.setTitle(res.getString(R.string.app_name));

        // set dialog message
        alertDialogBuilder.setMessage(res.getString(R.string.text_permission));
        alertDialogBuilder.setCancelable(false);
        alertDialogBuilder.setNegativeButton(res.getString(R.string.text_denied),new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog,int id) {
                // if this button is clicked, close
                // current activity
                SettingsEditor.setPermission(MainActivity.this, false);
                MainActivity.this.finish();
                }
                });
        alertDialogBuilder.setPositiveButton(res.getString(R.string.text_allowed),new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog,int id) {
                // if this button is clicked, just close
                // the dialog box and do nothing
                SettingsEditor.setPermission(MainActivity.this, true);
                dialog.cancel();
                chooseActivity();
                }
                });

        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();

        // show it
        alertDialog.show();
        } else {
            chooseActivity();
        }
    }

    private void chooseActivity() {
        password = SettingsEditor.getConfirmationCode(this);
        if(password == null) {
            startConfigurationActivity();
        }
    }

    private void startConfigurationActivity() {
        Intent i = new Intent(this, ConfigurationActivity.class);
        startActivity(i);
        finish();
    }

    private void configureEditTextView() {
        EditText askConfrimationCodeEditText = (EditText) this.findViewById(R.id.askConfirmationCode);
        askConfrimationCodeEditText.setOnEditorActionListener(this);
    }

    private void hideKeyboard(EditText editText) {
        InputMethodManager imm= (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
    }
}
