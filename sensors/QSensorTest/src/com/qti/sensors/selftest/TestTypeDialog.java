/*============================================================================
@file TestTypeDialog.java

@brief
Dialog box for the user to select which sensor test type they wish to perform
while on the "Self Test" tab.

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.selftest;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.database.SQLException;
import android.os.Bundle;
import android.widget.Toast;

import com.qualcomm.sensors.qsensortest.R;
import com.qualcomm.sensors.qsensortest.SettingsDatabase;
import com.qualcomm.sensors.qsensortest.TabControl;
import com.qualcomm.sensors.sensortest.SensorTest;
import com.qualcomm.sensors.sensortest.SensorTest.TestType;

public class TestTypeDialog extends DialogFragment {
    public static TestTypeDialog newInstance() {
       TestTypeDialog frag = new TestTypeDialog();
       Bundle args = new Bundle();
       args.putInt("title", R.string.selftest_type_dialog_title);
       frag.setArguments(args);
       return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
       int title = getArguments().getInt("title");

       final int testType = Integer.parseInt(SettingsDatabase.getSettings().getProperty("selftest_testType"));
       final TestType[] testTypes = SensorTest.TestType.values();
       final CharSequence[] testTypeNames = new CharSequence[testTypes.length];
       for(int i = 0; i < testTypes.length; i++)
          testTypeNames[i] = testTypes[i].toString();

       AlertDialog.Builder builder = new AlertDialog.Builder(TabControl.getContext());
       builder.setTitle(title);
       builder.setSingleChoiceItems((CharSequence[]) testTypeNames, testType,
             new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int item) {
                   Toast.makeText(TabControl.getContext(),
                                  testTypeNames[item],
                                  Toast.LENGTH_SHORT).show();
                   try{
                      SettingsDatabase.getSettings().setProperty("selftest_testType",
                            Integer.toString(testTypes[item].ordinal()));
                   } catch(SQLException e) {
                      Toast.makeText(TabControl.getContext(),
                                     "Unable to change setting",
                                     Toast.LENGTH_LONG).show();
                   }
                }
             });
       AlertDialog alert = builder.create();

       return alert;
    }
}
