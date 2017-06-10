/******************************************************************************
  @file    InputCriteria.java
  @brief   main screen for VZW GPS Location Provider test app

  DESCRIPTION

  test app for VZW GPS Location Provider test app

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.qvtester;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.text.DateFormat;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;
import com.qualcomm.location.vzw_library.VzwHalLocation;
import com.qualcomm.location.vzw_library.VzwHalSvInfo;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

public class InputCriteria extends Activity {

    private final String TAG = "QVTester_Criteria";

    Button btnRunVzwOnly;
    Button btnRunWithNormalStack;

    Button btnRunState1;
    Button btnRunState2;

    SharedPreferences mPreferences;
    Spinner spinnerMode;
    EditText NumOfFix;
    EditText TimeBetweenFix;
    EditText PdeAddr;
    EditText PdePort;
    EditText AccuracyH;
    EditText AccuracyV;
    EditText MaxResponseTime;
    EditText AppCredential;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mPreferences = getPreferences(MODE_PRIVATE);

        btnRunVzwOnly = (Button) findViewById(R.id.ButtonRunOne);
        btnRunVzwOnly.setOnClickListener(btn_OnClick);

        btnRunWithNormalStack = (Button) findViewById(R.id.ButtonRunTwo);
        btnRunWithNormalStack.setOnClickListener(btn_OnClick);

        btnRunState1 = (Button) findViewById(R.id.ButtonRunState1);
        btnRunState1.setOnClickListener(btn_OnClick);

        btnRunState2 = (Button) findViewById(R.id.ButtonRunState2);
        btnRunState2.setOnClickListener(btn_OnClick);

        spinnerMode = (Spinner)findViewById(R.id.SpinnerMode);
        NumOfFix = (EditText)findViewById(R.id.EditTextNumFix);
        TimeBetweenFix = (EditText)findViewById(R.id.EditTextRateFix);
        PdeAddr = (EditText)findViewById(R.id.EditTextPdeAddr);
        PdePort = (EditText)findViewById(R.id.EditTextPdePort);
        AccuracyH = (EditText)findViewById(R.id.EditTextAccuracyH);
        AccuracyV = (EditText)findViewById(R.id.EditTextAccuracyV);
        MaxResponseTime = (EditText)findViewById(R.id.EditTextMaxResponseTime);
        AppCredential = (EditText)findViewById(R.id.EditTextAppCredential);

        spinnerMode.setSelection( mPreferences.getInt("Mode", 0) );
        NumOfFix.setText( Integer.toString(mPreferences.getInt("NumOfFix", 20)) );
        TimeBetweenFix.setText( Integer.toString(mPreferences.getInt("TimeBetweenFix", 2)) );
        PdeAddr.setText( mPreferences.getString("PdeAddr", "127.0.0.1") );
        PdePort.setText( Integer.toString(mPreferences.getInt("PdePort", 1000)) );
        AccuracyH.setText( Integer.toString(mPreferences.getInt("AccuracyH", 50)) );
        AccuracyV.setText( Integer.toString(mPreferences.getInt("AccuracyV", 50)) );
        MaxResponseTime.setText( Integer.toString(mPreferences.getInt("MaxResponseTime", 120)) );

        try {
            Log.v(TAG,"Flushing contents in the log file");

            OutputStreamWriter mLogFile = new OutputStreamWriter(openFileOutput("location.log", MODE_PRIVATE));
            mLogFile.close();

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (NullPointerException npe) {
            npe.printStackTrace();
        }
    }


    private OnClickListener btn_OnClick = new OnClickListener ()
    {
        public void onClick(View v)
        {
            //Advance to the next screen.
            Intent iTrack = new Intent("com.qualcomm.location.qvtester.TRACKING");

            SharedPreferences.Editor editor = mPreferences.edit();

            //data exchange
            int iMode = spinnerMode.getSelectedItemPosition();

            iTrack.putExtra("Mode", iMode);
            editor.putInt("Mode", iMode);

            int iNumOfFix = Integer.parseInt(NumOfFix.getText().toString());

            iTrack.putExtra("NumOfFix", iNumOfFix);
            editor.putInt("NumOfFix", iNumOfFix);

            int iTimeBetweenFix = Integer.parseInt(TimeBetweenFix.getText().toString());

            iTrack.putExtra("TimeBetweenFix", iTimeBetweenFix);
            editor.putInt("TimeBetweenFix", iTimeBetweenFix);

            String strPdeAddr = PdeAddr.getText().toString();

            iTrack.putExtra("PdeAddr", strPdeAddr);
            editor.putString("PdeAddr", strPdeAddr);

            int iPdePort = Integer.parseInt(PdePort.getText().toString());

            iTrack.putExtra("PdePort", iPdePort);
            editor.putInt("PdePort", iPdePort);

            int iAccuracyH = Integer.parseInt(AccuracyH.getText().toString());

            iTrack.putExtra("AccuracyH", iAccuracyH);
            editor.putInt("AccuracyH", iAccuracyH);

            int iAccuracyV = Integer.parseInt(AccuracyV.getText().toString());

            iTrack.putExtra("AccuracyV", iAccuracyV);
            editor.putInt("AccuracyV", iAccuracyV);

            int iMaxResponseTime = Integer.parseInt(MaxResponseTime.getText().toString());

            iTrack.putExtra("MaxResponseTime", iMaxResponseTime);
            editor.putInt("MaxResponseTime", iMaxResponseTime);

            String strAppCredential = AppCredential.getText().toString();

            iTrack.putExtra("AppCredential", strAppCredential);
            editor.putString("AppCredential", strAppCredential);

            editor.commit();

            if(v.equals(btnRunVzwOnly))
            {
                Log.i (TAG, "Run Vzw Only");
                startActivity(iTrack);
            }
            else if(v.equals(btnRunWithNormalStack))
            {
                Log.i (TAG, "Run with normal stack");
                iTrack.putExtra("RunWithNormalStack", true);
                startActivity(iTrack);
            }
            else if(v.equals(btnRunState1))
            {
                Log.i (TAG, "Run state machine test suite 1");
                iTrack.putExtra("RunStateTest", 1);
                startActivity(iTrack);
            }
            else if(v.equals(btnRunState2))
            {
                Log.i (TAG, "Run state machine test suite 2");
                iTrack.putExtra("RunStateTest", 2);
                startActivity(iTrack);
            }
            else
            {
                // should never happen
                Log.e (TAG, "Button doesn't exist");
            }
            Log.i (TAG, "OnClick finished");
        }
    };

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add("Delete Aiding Data").setOnMenuItemClickListener(new MenuItem.OnMenuItemClickListener(){
            public boolean onMenuItemClick(MenuItem item) {
                deleteAidingData();
                return true;
            }});
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (AidingDataActivity.AIDING_SELECT_REQUEST == requestCode){
            int aidingSelected = 0;

            if (RESULT_OK == resultCode && null != data) {
                aidingSelected = data.getIntExtra(AidingDataActivity.AIDING_SELECTED, 0);
            }

            Log.v(TAG, "onActivityResult with resultCode "+Integer.toHexString(aidingSelected));


        }
    }

    private void deleteAidingData() {
        // now we have no more tests to go, process all the scores and launch report
        Intent intent = new Intent("com.qualcomm.location.qvtester.Aiding").setClassName(this, DeleteAidingDataActivity.class.getName());
        startActivityForResult(intent, DeleteAidingDataActivity.AIDING_DELETE_REQUEST);
    }
}
