/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import com.qualcomm.presencelist.R;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

public class MyInfo extends Activity {

    int FIELD_TAG = 0;
    int FIELD_VALUE = 1;


    int formMap[][] = {
        {R.string.myNumtext,R.id.myNumValue},
        {R.string.uri1text, R.id.uri1Value},
        {R.string.uri2text, R.id.uri2Value},
        {R.string.basicstatustext, R.id.basicStatusSpinner},
        {R.string.descriptiontext, R.id.descriptionValue},
        {R.string.vertext, R.id.verValue},
        {R.string.serviceIdtext, R.id.serviceIdValue},
        {R.string.isAudioSupportedtext, R.id.audiospinner},
        {R.string.isVideoSupportedtext, R.id.videospinner},

    };

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            this.setContentView(R.layout.myinfo);

            initButtons();

            populateInitialValues();
        }

    @Override
    protected void onStop() {
        super.onStop();
        finish();
    }

    private void initButtons() {
        Button okButton = (Button)findViewById(R.id.ok);
        handleOkButtonClick(okButton);

        Button cancelButton = (Button)findViewById(R.id.cancel);
        handleCancelButtonClick(cancelButton);

    }

    private void handleOkButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

                SharedPreferences.Editor editor = getSharedPrefEditor(
                    AppGlobalState.IMS_PRESENCE_MY_INFO);
                storeFormValuesToSharedPreferences(editor);
                finish();
            }
        });
    }

    private void handleCancelButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                finish();
            }
        });
    }

    private void populateInitialValues() {
        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_MY_INFO);

        for(int i=0;i<formMap.length;i++) {
            String uriValue = setting.getString(getString(formMap[i][FIELD_TAG]), "");

            if(formMap[i][FIELD_VALUE] == R.id.basicStatusSpinner ||
                    formMap[i][FIELD_VALUE] == R.id.audiospinner ||
                    formMap[i][FIELD_VALUE] == R.id.videospinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                ArrayAdapter adapter  = (ArrayAdapter) spinner.getAdapter();
                int pos = adapter.getPosition(uriValue);

                spinner.setSelection(pos);

            } else {
                TextView t = (TextView) findViewById(formMap[i][FIELD_VALUE]);
                t.setText(uriValue);
            }
        }
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private Editor getSharedPrefEditor(String imsPresencePref) {
        SharedPreferences settings = getSharedPrefHandle(imsPresencePref);
        SharedPreferences.Editor editor = settings.edit();
        return editor;
    }

    private void storeFormValuesToSharedPreferences(Editor editor) {

        for(int i=0;i<formMap.length;i++) {
            if(formMap[i][FIELD_VALUE] == R.id.basicStatusSpinner ||
                    formMap[i][FIELD_VALUE] == R.id.audiospinner ||
                    formMap[i][FIELD_VALUE] == R.id.videospinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]),
                        spinner.getSelectedItem().toString());

            } else {
                TextView t = (TextView)findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]),
                        t.getText().toString());
            }
        }
        editor.commit();
    }
}
