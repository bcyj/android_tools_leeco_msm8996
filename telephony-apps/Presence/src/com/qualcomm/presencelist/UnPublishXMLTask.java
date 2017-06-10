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

public class UnPublishXMLTask extends AsyncTask<Void, Integer, Integer> {

    final String TAG = "UnPublishXMLTask";

    Context mContext;
    ProgressDialog dialog;
    UnPublishXMLTask me;

    public UnPublishXMLTask() {}

    public UnPublishXMLTask(Context appContext) {
        mContext = appContext;
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

        dialog.show();

    }

    @Override
        protected Integer doInBackground(Void... params) {
            Log.d(TAG, "doInBackground(), Thread="+
                    Thread.currentThread().getName());

            if(Looper.myLooper() == null ) {
                Looper.prepare();
            }
            return sendUnPublishXMLRequest();
        }

    private int sendUnPublishXMLRequest() {
        Log.d(TAG, "sendUnPublishXMLRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        return p.imsp_send_unpublish_req();
    }

    @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);

            dialog.incrementProgressBy(values[0]);
        }

    @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Log.d(TAG, "onPostExecute(), Thread="+
                    Thread.currentThread().getName());

            dialog.dismiss();

            Toast.makeText(mContext, "UnPublishXML Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }
}
