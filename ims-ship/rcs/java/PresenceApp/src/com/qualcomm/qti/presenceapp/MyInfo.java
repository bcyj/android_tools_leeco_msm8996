/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import com.qualcomm.qti.presenceapp.R;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MyInfo extends Activity {

    int FIELD_TAG = 0;
    int FIELD_VALUE = 1;
    public static MyInfo myInfoObject;
    public static boolean inMyInfoScreen = false;

    int formMap[][] = {
            {
                    R.string.myNumtext, R.id.myNumValue
            },
            {
                    R.string.uri1text, R.id.uri1Value
            },
            {
                    R.string.uri2text, R.id.uri2Value
            },
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        myInfoObject = this;
        inMyInfoScreen = true;
        this.setContentView(R.layout.myinfo);

        initButtons();

        populateInitialValues();
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        Log.d("PRESENCE_UI", "onStart making inMyInfoScreen true");
        inMyInfoScreen = true;
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("PRESENCE_UI", "ONPAUSE");
        inMyInfoScreen = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("PRESENCE_UI", "ON RESUME inMyInfoScreen true");
        inMyInfoScreen = true;
    }

    @Override
    protected void onStop() {
        super.onStop();
        finish();
    }

    private void initButtons() {
        Button okButton = (Button) findViewById(R.id.ok);
        handleOkButtonClick(okButton);

        Button cancelButton = (Button) findViewById(R.id.cancel);
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

        SharedPreferences.Editor editor = getSharedPrefEditor(AppGlobalState.IMS_PRESENCE_MY_INFO);
        editor.putString("Service Id",
                "org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel");
        editor.putString("Version", "1.0");
        editor.putString("Audio", "True");
        editor.commit();

        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_MY_INFO);

        for (int i = 0; i < formMap.length; i++) {
            String uriValue = setting.getString(getString(formMap[i][FIELD_TAG]), "");
            {
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

        for (int i = 0; i < formMap.length; i++) {
            {
                TextView t = (TextView) findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]),
                        t.getText().toString());
            }
        }
        editor.commit();
    }
}
