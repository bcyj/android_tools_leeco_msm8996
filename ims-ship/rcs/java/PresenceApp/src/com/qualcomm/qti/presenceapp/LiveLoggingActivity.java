/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import com.qualcomm.qti.presenceapp.R;

import android.app.Activity;
import android.os.Bundle;


public class LiveLoggingActivity extends Activity {
    LogTextBox mTextBox;

    /** Called when the activity is first created. */
    @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            this.setContentView(R.layout.livelogging);

            mTextBox = (LogTextBox) findViewById(R.id.liveloggingtextView);
            LogTextBox.setInstance(mTextBox);

            InputStream instream;
            try {
                instream = openFileInput(AppGlobalState.IMS_PRESENCE_LIVE_LOGGING);
                if (instream != null) {
                    InputStreamReader inputreader = new InputStreamReader(instream);
                    BufferedReader buffreader = new BufferedReader(inputreader);

                    String line;

                    while ((line = buffreader.readLine()) != null) {
                        mTextBox.append(line);
                    }
                    instream.close();
                }

            } catch (FileNotFoundException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

        }

    @Override
    protected void onStop() {
        super.onStop();
        finish();
    }

    public void onWindowFocusChanged(boolean hasFocus) {

        if(hasFocus == true) {
            if(mTextBox != null) {
                mTextBox.invalidate();
            }
        }
        super.onWindowFocusChanged(hasFocus);

    }
}
