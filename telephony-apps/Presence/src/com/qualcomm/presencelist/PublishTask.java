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


import com.qualcomm.presencelist.R;
import com.qualcomm.qcrilhook.PresenceOemHook;

public class PublishTask extends AsyncTask<Void, Integer, Integer> {

    final String TAG = "PublishTask";

    Context mContext;
    ProgressDialog dialog;
    PublishTask me;

    public PublishTask() {
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
            return sendPublishRequest();
        }


    private int sendPublishRequest() {
        Log.d(TAG, "sendPublishRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        SharedPreferences setting = getSharedPrefHandle(AppGlobalState.IMS_PRESENCE_MY_INFO);

        String myNum = setting.getString(
                mContext.getString(R.string.myNumtext), "");
        String uri1 = setting.getString(
                mContext.getString(R.string.uri1text), "");
        String uri2 = setting.getString(
                mContext.getString(R.string.uri2text), "");

        String status = setting.getString(
                mContext.getString(R.string.basicstatustext),
                mContext.getString(R.string.bs_open_text));
        int statusValue = (status.equals(
                    mContext.getString(R.string.bs_open_text)))?1:0;

        String description = setting.getString(
                mContext.getString(R.string.descriptiontext), "");
        String ver = setting.getString(
                mContext.getString(R.string.vertext), "");
        String serviceId = setting.getString(
                mContext.getString(R.string.serviceIdtext), "");

        String isAudioSupported = setting.getString(
                mContext.getString(R.string.isAudioSupportedtext), "");
        int audioSupported = isAudioSupported.equals(
                mContext.getString(R.string.sc_supported_text))?1:0;

        String isVideoSupported = setting.getString(
                mContext.getString(R.string.isVideoSupportedtext), "");
        int videoSupported = isVideoSupported.equals(
                mContext.getString(R.string.sc_supported_text))?1:0;


        String myNumUri = uri1+myNum+uri2;

        return p.imsp_send_publish_req(statusValue,
                myNumUri, description, ver, serviceId,
                audioSupported, 0,videoSupported,0);
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }


    @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Log.d(TAG, "onPostExecute(), Thread="+Thread.currentThread().getName());

            dialog.dismiss();

            Toast.makeText(mContext, "Publish Rich Result ="+
                    result, Toast.LENGTH_SHORT).show();
        }
}
