/*
 * Copyright (c) 2012-2013,Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.android.qrdreader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.telephony.MSimTelephonyManager;
import android.util.Log;

public class ReaderActivity extends Activity {
    /** Called when the activity is first created. */

    private final Boolean DBG = false;
    private final String LOG_TAG = "ReaderActivity";
    private final String OPERATOR_NUMERIC = "46001";// china union operator no.

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        if (isNetworkConnected()) {
            if (DBG) {
                Log.i(LOG_TAG, "china union data source or wifi");
            }
            startBrowser();
        }
        /* if not china union data,pop up a dialog */
        else {
            AlertDialog.Builder confirmDialog = new AlertDialog.Builder(this);
            confirmDialog.setTitle(getResources().getString(R.string.confirm_title));
            confirmDialog.setMessage(getResources().getString(R.string.confirm_info));
            confirmDialog.setPositiveButton(getResources().getString(R.string.PositiveButton),
                new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                    startBrowser();
                }
            });
            confirmDialog.setNegativeButton(getResources().getString(R.string.NegativeButton),
                new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                    finish();
                }
            });
            confirmDialog.show();
        }
    }

    private void startBrowser() {
        Uri uri = Uri.parse(getResources().getString(R.string.reader_homepage));
        Intent intent  = new Intent(Intent.ACTION_VIEW,uri);
        try{
            startActivity(intent);
        } catch (ActivityNotFoundException e){
            Log.w("ReaderActivity",e);
        }
    }

    private Boolean isNetworkConnected() {
        MSimTelephonyManager tm = MSimTelephonyManager.getDefault();
        int dataSubscription = tm.getPreferredDataSubscription();
        String networkOperator = tm.getNetworkOperator(dataSubscription);
        if (DBG) {
            Log.i(LOG_TAG, "OPERATOR_NUMERIC == " + networkOperator);
        }
        ConnectivityManager connManager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo wifiInfo = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if ((OPERATOR_NUMERIC.equals(tm.getNetworkOperator(dataSubscription)) && (TelephonyManager.DATA_CONNECTED == tm
                .getDataState())) || wifiInfo.isConnected()) {
            return true;
        } else {
            return false;
        }
    }
}
