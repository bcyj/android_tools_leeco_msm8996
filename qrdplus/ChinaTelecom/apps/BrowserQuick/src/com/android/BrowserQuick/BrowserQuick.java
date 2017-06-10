/*
 *Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.BrowserQuick;

import com.android.BrowserQuick.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.view.Window;

public class BrowserQuick extends Activity {

   private final String LOG_TAG = "BrowserQuick";

    /**
     * Called when Activity is first created. Turns off the title bar, sets up
     * the content views, and fires up the SnakeView.
     *
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        startBrowser();
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        finish();
    }

    private void startBrowser() {
        Uri uri = Uri.parse(getResources().getString(R.string.homepage_base));
        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
        //Vnet will use system browser in default situation
        try{
            intent.setClassName("com.android.browser", "com.android.browser.BrowserActivity");
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            //When the system browser is disabled, Vnet will choose the other brower applications
            try {
                Intent newIntent = new Intent(Intent.ACTION_VIEW, uri);
                startActivity(newIntent);
            } catch (ActivityNotFoundException ex) {
                Log.w(LOG_TAG, "ex: " + ex);
            }
        }
    }
}
