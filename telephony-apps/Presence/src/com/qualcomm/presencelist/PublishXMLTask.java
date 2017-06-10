/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


import com.qualcomm.qcrilhook.PresenceOemHook;

public class PublishXMLTask extends AsyncTask<Void, Integer, Integer> {

    final String TAG = "PublishXMLTask";

    Context mContext;
    ProgressDialog dialog;
    PublishXMLTask me;

    public PublishXMLTask( ) {
        mContext = AppGlobalState.getMainActivityContext();
        me= this;
    }

    @Override
        protected void onPreExecute() {
            super.onPreExecute();

            initProgressBar();
        }

    private void initProgressBar() {
        mContext = AppGlobalState.getMainActivityContext();
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");

        dialog.setCancelable(false);

        dialog.show();

    }

    @Override
        protected Integer doInBackground(Void... params) {
            Log.d(TAG, "doInBackground(), Thread="+
                    Thread.currentThread().getName());

            if(Looper.myLooper() == null ) {
                Looper.prepare();
            }
            String xml = Utility.readXMLFromFile(mContext, "publish.xml");
            return sendPublishXMLRequest(xml);
        }


    private int sendPublishXMLRequest(String xml) {
        Log.d(TAG, "sendPublishXMLRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        Utility.sendLogMesg(mContext, xml);
        return p.imsp_send_publish_xml_req(xml);
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(
                imsPresencePref, 0);

        return settings;
    }


    @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Log.d(TAG, "onPostExecute(), Thread="+Thread.currentThread().getName());

            dialog.dismiss();

            Toast.makeText(mContext, "PublishXML Rich Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }
}
