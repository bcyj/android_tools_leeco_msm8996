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


public class SubscribePollingXMLTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "SubscribePollingXMLTask";

    int mContactIndex;
    Context mContext;
    ProgressDialog dialog;

    SubscribePollingXMLTask me;

    public SubscribePollingXMLTask() {}

    public SubscribePollingXMLTask(Context appContext,int contactIndex) {

        mContext = appContext;
        mContactIndex = contactIndex;
        me= this;
    }

    @Override
        protected void onPreExecute() {
            super.onPreExecute();

            initProgressBar();
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

            return sendSubscribePollingXMLRequest(xml);
        }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private String prepareCompleteUri(String phone) {
        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_MY_INFO);

        String uri1Value = setting.getString(
                mContext.getString(R.string.uri1text), "");
        String uri2Value = setting.getString(
                mContext.getString(R.string.uri2text), "");

        phone = ""+(uri1Value!= null?uri1Value:"")+phone;
        phone = phone+(uri2Value !=null?uri2Value:"");

        return phone;
    }
    private Integer sendSubscribePollingXMLRequest(String xml) {
        Log.d(TAG, "sendSubscribePollingXMLRequest");

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

            dialog.dismiss();
            if (result != 0) { //failure
                //dialog.dismiss();
            } else { //success
                updateContactSubscriptionFlag();
            }

            Toast.makeText(mContext, "SubscribeXML Polling Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }

    private void updateContactSubscriptionFlag() {

        ArrayList<Contact> contacts =AppGlobalState.getContacts();

        Contact temp = contacts.get(mContactIndex);
        temp.setSubscriptionOnFlag(false);
    }
}
