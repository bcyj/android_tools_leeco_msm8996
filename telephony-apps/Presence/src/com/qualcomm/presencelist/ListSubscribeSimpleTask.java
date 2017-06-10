/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;


import java.util.ArrayList;


import com.qualcomm.qcrilhook.PresenceOemHook;
import com.qualcomm.qcrilhook.PresenceOemHook.SubscriptionType;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


public class ListSubscribeSimpleTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "ListSubscribeSimpleTask";

    Context mContext;
    ProgressDialog dialog;

    public ListSubscribeSimpleTask() {}

    public ListSubscribeSimpleTask(Context appContext) {

        mContext = appContext;
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

            ArrayList<String> uriList = getSelectedContactsList();

            if(uriList.size() == 0) {
                Log.d(TAG,"None of the contact selected for list subscription");
                return 0;
            }

            return sendListSubscribePollingRequest(uriList);
        }

    private ArrayList<String> getSelectedContactsList() {
        ArrayList<Contact> contacts =AppGlobalState.getContacts();
        ArrayList<String> uriList = new ArrayList();
        for(Contact c: contacts) {
            if(c.isMultiSelected()) {
                String phone = c.getPhone();
                uriList.add(prepareCompleteUri(phone));
            }
        }
        return uriList;
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private String prepareCompleteUri(String phone) {
        SharedPreferences setting = getSharedPrefHandle(AppGlobalState.IMS_PRESENCE_MY_INFO);

        String uri1Value = setting.getString(mContext.getString(R.string.uri1text), "");
        String uri2Value = setting.getString(mContext.getString(R.string.uri2text), "");

        phone = ""+(uri1Value!= null?uri1Value:"")+phone;
        phone = phone+(uri2Value !=null?uri2Value:"");

        return phone;
    }

    private Integer sendListSubscribePollingRequest(ArrayList<String> contactList) {
        Log.d(TAG, "sendListSubscribePollingRequest for " + contactList);

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        return p.imsp_send_subscribe_req(SubscriptionType.SIMPLE, contactList);
    }


    @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);
        }

    @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Log.d(TAG, "onPostExecute(), Thread="+Thread.currentThread().getName());


            dialog.dismiss();

            if(getSelectedContactsList().size() == 0) {
                Toast.makeText(mContext, "None of the contact selected for list subscription.",
                        Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(mContext, "List Subscribe Polling Result ="+
                        result, Toast.LENGTH_SHORT).show();
            }
        }
}

