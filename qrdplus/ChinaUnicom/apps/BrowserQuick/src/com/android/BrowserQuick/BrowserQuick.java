/*
 *Copyright (c) 2012-2013,Qualcomm Technologies, Inc.
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
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Window;

public class BrowserQuick extends Activity {

    private final String LOG_TAG = "BrowserQuick";
    private final String NUMERIC_NAME = "46001";// china union

    /**
     * Called when Activity is first created. Turns off the title bar, sets up
     * the content views, and fires up the SnakeView.
     *
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TelephonyManager TM = TelephonyManager.getDefault();
        String networkOperator =
                TM.getNetworkOperatorForSubscription(SubscriptionManager.getDefaultDataSubId());
        Log.i(LOG_TAG, "NUMERIC_NAME == " + networkOperator);

        ConnectivityManager connManager =
           (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo mWifi = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

        if ((NUMERIC_NAME.equals(networkOperator)
                && (TelephonyManager.DATA_CONNECTED == TM.getDataState())) || mWifi.isConnected()) {
            Log.i(LOG_TAG, "BrowserQuick:china union data source");
            startBrowser();
        }
        /* if not china union data,pop up a dialog */
        else {
            AlertDialog.Builder my_ADialog = new AlertDialog.Builder(this);
            my_ADialog.setTitle(getResources().getString(R.string.confirm_title));
            my_ADialog.setMessage(getResources().getString(R.string.confirm_info));
            my_ADialog.setPositiveButton(getResources().getString(R.string.PositiveButton),
                new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                    startBrowser();
                }
            });
            my_ADialog.setNegativeButton(getResources().getString(R.string.NegativeButton),
                new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                    finish();
                }
            });
            my_ADialog.show();
        }
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
        try{
            startActivity(intent);
        } catch (ActivityNotFoundException e){
            Log.w(LOG_TAG, "e: " + e);
        }
    }
}
