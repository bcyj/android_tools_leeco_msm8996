/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import java.util.ArrayList;

import com.qualcomm.presencelist.MainActivity.ContactArrayAdapter;
import com.qualcomm.qcrilhook.PresenceOemHook;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


public class UnSubscribeTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "UnSubscribeTask";


    int mContactIndex;
    Context mContext;
    ProgressDialog dialog;


    public UnSubscribeTask() {}

    public UnSubscribeTask(Context appContext, int contactIndex) {
        mContext = appContext;
        mContactIndex = contactIndex;
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

            String uri = prepareCompleteUri(phone);
            return sendUnSubscribeRequest(uri);
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

    private Integer sendUnSubscribeRequest(String uri) {
        Log.d(TAG, "sendUnSubscribeRequest for " + uri);

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        return p.imsp_send_unsubscribe_req(uri);
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

            updateContactSubscriptionFlag(result);

            dialog.dismiss();

            Toast.makeText(mContext, "UnSubscribe Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }

    private void updateContactSubscriptionFlag(int result) {

        ArrayList<Contact> contacts =AppGlobalState.getContacts();

        Contact temp = contacts.get(mContactIndex);
        if(result == 0) { //success
            temp.setSubscriptionOnFlag(false);
        } else if (result == 1) { //failure
            temp.setSubscriptionOnFlag(true);
        }

    }
}
