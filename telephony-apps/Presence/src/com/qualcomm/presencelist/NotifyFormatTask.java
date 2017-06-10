/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.AsyncTask;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;


import com.qualcomm.presencelist.R;
import com.qualcomm.presencelist.Settings.SettingsMainThreadHandler;
import com.qualcomm.qcrilhook.PresenceOemHook;
import com.qualcomm.qcrilhook.PresenceOemHook.PresenceSolResponse;

public class NotifyFormatTask extends AsyncTask<Integer, Integer, Object> {

    final String TAG = "NotifyFormatTask";

    Context mContext;
    ProgressDialog dialog;
    NotifyFormatTask me;

    /* True for setNotifyReq */
    int mSetNotifyFlag;

    public NotifyFormatTask() {}

    public NotifyFormatTask(Context appContext) {
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
        protected Object doInBackground(Integer... flag) {
            Log.d(TAG, "doInBackground(), Thread="+
                    Thread.currentThread().getName());

            if(Looper.myLooper() == null ) {
                Looper.prepare();
            }
            this.mSetNotifyFlag = flag[0];

            if (flag[0] == 1 ) {
                return sendSetNotifyRequest();
            } else {
                return sendGetNotifyRequest();
            }

        }

    private Object sendGetNotifyRequest() {
        Log.d(TAG, "sendGetNotifyRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        return p.imsp_get_notify_fmt_req();
    }



    private Object  sendSetNotifyRequest() {
        Log.d(TAG, "sendSetNotifyRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_SETTINGS);

        String fmt  = setting.getString(
                mContext.getString(R.string.set_notify_fmt_text),
                mContext.getString(R.string.fmt_struct_text));

        Log.d(TAG, "sendSetNotifyRequest ="+fmt);

        int flag = (fmt.equals(mContext.getString(R.string.fmt_struct_text)))? 1 : 0;
        return p.imsp_set_notify_fmt_req(flag);
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext.getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private Editor getSharedPrefEditor(String imsPresencePref) {
        SharedPreferences settings = getSharedPrefHandle(imsPresencePref);
        SharedPreferences.Editor editor = settings.edit();
        return editor;
    }


    @Override
        protected void onPostExecute(Object obj) {
            super.onPostExecute(obj);
            Log.d(TAG, "onPostExecute(), Thread="+Thread.currentThread().getName());

            dialog.dismiss();

            PresenceSolResponse response = (PresenceSolResponse) obj;
            int result = response.result;
            int data = (Integer) response.data;

            if (result != 0) {
                Toast.makeText(mContext, "Notify fmt request failed ="+
                        result, Toast.LENGTH_SHORT).show();
                return;
            }


            if (mSetNotifyFlag == 0) { //getNotifyRequest response.
                SharedPreferences.Editor editor = getSharedPrefEditor(
                        AppGlobalState.IMS_PRESENCE_SETTINGS);

                switch (data) {
                case 0:
                { //XML Format
                    String val = mContext.getString(R.string.fmt_xml_text);
                    editor.putString(mContext.getString(R.string.set_notify_fmt_text), val );
                    editor.commit();

                    Toast.makeText(mContext, "Current Notify Format ="+val,
                            Toast.LENGTH_SHORT).show();

                    updateSettingsActivity(val);

                    break;
                }
                case 1:
                { //STRUCT format
                    String val = mContext.getString(R.string.fmt_struct_text);
                    editor.putString(mContext.getString(R.string.set_notify_fmt_text), val );
                    editor.commit();

                    Toast.makeText(mContext, "Current Notify Format ="+val,
                            Toast.LENGTH_SHORT).show();

                    updateSettingsActivity(val);
                    break;
                }

                default:
                    Log.e(TAG, "Unknown value");
                }
            } else {
                Toast.makeText(mContext, "Set notify request status"+
                        response.result, Toast.LENGTH_SHORT).show();
            }
        }

    private void updateSettingsActivity(String val) {
        SettingsMainThreadHandler settingsHandler = AppGlobalState.getSettingsHandler();
        Message m = settingsHandler.obtainMessage(
                SettingsMainThreadHandler.SET_NOTIFY_FORMAT_RESPONSE, val);
        settingsHandler.sendMessage(m);

    }
}
