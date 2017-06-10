/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import com.qualcomm.qcrilhook.PresenceOemHook;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.qcrilhook.PresenceOemHook;
import com.qualcomm.qcrilhook.PresenceOemHook.PresenceSolResponse;

public class ImsEnablerTask extends AsyncTask<Void, Integer, Object> {

    final String TAG = "ImsEnablerTask";

    Context mContext;
    ProgressDialog dialog;

    public ImsEnablerTask() {}

    public ImsEnablerTask(Context appContext) {
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

        dialog.show();

    }

    @Override
        protected Object doInBackground(Void... params) {
            Log.d(TAG, "doInBackground(), Thread="+
                    Thread.currentThread().getName());

            if(Looper.myLooper() == null ) {
                Looper.prepare();
            }

            return sendImsEnablerRequest();

        }


    private Object sendImsEnablerRequest() {
        Log.d(TAG, "sendImsEnablerRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        return p.imsp_get_enabler_state_req();

    }

    @Override
        protected void onProgressUpdate(Integer... values) {

            super.onProgressUpdate(values);

            dialog.incrementProgressBy(values[0]);
        }

    @Override
        protected void onPostExecute(Object obj) {
            super.onPostExecute(obj);
            Log.d(TAG, "onPostExecute(), Thread="+
                    Thread.currentThread().getName());

            dialog.dismiss();

            PresenceSolResponse response = (PresenceSolResponse) obj;
            int result = response.result;
            int data = (Integer) response.data;

            Toast.makeText(mContext, "IMS Enabler State Result ="+
                    PresenceOemHook.IMS_ENABLER_RESPONSE[data],
                    Toast.LENGTH_SHORT).show();
        }
}
