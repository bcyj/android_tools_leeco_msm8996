/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import java.util.ArrayList;
import com.qualcomm.qcrilhook.PresenceOemHook;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


public class SubscribeSimpleXMLTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "SubscribeSimpleXMLTask";

    int mContactIndex;
    Context mContext;
    ProgressDialog dialog;
    boolean isBackground = false;

    SubscribeSimpleXMLTask me;

    public SubscribeSimpleXMLTask() {}

    public SubscribeSimpleXMLTask(Context appContext, int contactIndex, boolean bg) {
        mContext = appContext;
        mContactIndex = contactIndex;
        me= this;
        isBackground = bg;
    }

    @Override
        protected void onPreExecute() {
            super.onPreExecute();

            if(!isBackground ) {
                initProgressBar();
            }
        }

    private void initProgressBar() {
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");
        dialog.setCancelable(false);

        AppGlobalState.setProgressDialog(dialog);

        dialog.show();

    }

    @Override
        protected Integer doInBackground(Void... params) {
            Log.d(TAG, "doInBackground(), Thread="+Thread.currentThread().getName());

            if(Looper.myLooper() == null ) {
                Looper.prepare();
            }

            ArrayList<Contact> contacts =AppGlobalState.getContacts();
            String phone = contacts.get(mContactIndex).getPhone();


            String xml = Utility.readXMLFromFile(mContext, "subscribe.xml");

            return sendSubscribeSimpleXMLRequest(xml);
        }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private Integer sendSubscribeSimpleXMLRequest(String xml) {
        Log.d(TAG, "sendSubscribeSimpleXMLRequest");

        Utility.sendLogMesg(mContext, xml);

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        return p.imsp_send_subscribe_xml_req(xml);
    }

    @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);
        }

    @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Log.d(TAG, "onPostExecute(), Thread="+
                    Thread.currentThread().getName());

            if (result == 0) { //Success
                updateContactSubscriptionFlag();
            }

            if(!isBackground) {
                dialog.dismiss();
            }

            Toast.makeText(mContext, "SubscribeXML Simple Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }

    private void updateContactSubscriptionFlag() {

        ArrayList<Contact> contacts =AppGlobalState.getContacts();

        Contact temp = contacts.get(mContactIndex);
        temp.setSubscriptionOnFlag(true);
    }
}
