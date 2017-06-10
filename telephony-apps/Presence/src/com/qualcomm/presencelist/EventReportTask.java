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

public class EventReportTask extends AsyncTask<Integer, Integer, Object> {

    final String TAG = "EventReportTask";

    Context mContext;
    ProgressDialog dialog;
    EventReportTask me;

    /* True for setNotifyReq */
    int mSetEventReportFlag;

    public EventReportTask() {}

    public EventReportTask(Context appContext) {
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
            this.mSetEventReportFlag = flag[0];

            if (mSetEventReportFlag == 1 ) { //SET
                return sendSetEventReportRequest();
            } else {
                return sendGetEventReportRequest(); //GET

            }

        }

    private Object sendGetEventReportRequest() {
        Log.d(TAG, "sendGetEventReportRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();
        return p.imsp_get_event_report_req();
    }



    private Object  sendSetEventReportRequest() {
        Log.d(TAG, "sendSetEventReportRequest");

        PresenceOemHook p = AppGlobalState.getPresenceOemHook();

        int mask = getMaskValue();
        return p.imsp_set_event_report_req(mask);
    }

    private int getMaskValue() {

        final int PUBLISH_TRIGGER_IND_ON = 0x01;
        final int ENABLER_STATE_IND_ON = 0x02;
        final int NOTIFY_IND_ON = 0x04;

        int mask = 0x00;
        Boolean tempBool;
        String tempString;
        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_SETTINGS);
        tempString  = setting.getString(mContext.getString(
                    R.string.enable_publish_trigger_ind_text), "false");
        tempBool = new Boolean(tempString);
        mask |= (tempBool)?PUBLISH_TRIGGER_IND_ON:mask;

        tempString  = setting.getString(mContext.getString(
                    R.string.enable_enabler_state_ind_text), "false");
        tempBool = new Boolean(tempString);
        mask |= (tempBool)?ENABLER_STATE_IND_ON:mask;

        tempString  = setting.getString(mContext.getString(
                    R.string.enable_notify_ind_text), "false");
        tempBool = new Boolean(tempString);
        mask |= (tempBool)?NOTIFY_IND_ON:mask;

        Log.d(TAG, "SetEventReport mask="+mask);

        return mask;
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
            Log.d(TAG, "onPostExecute(), Thread="
                    + Thread.currentThread().getName());

            dialog.dismiss();

            PresenceSolResponse response = (PresenceSolResponse) obj;
            int data = (Integer) response.data;
            int result = response.result;


            if(result != 0) {
                Toast.makeText(mContext,
                        "Event report request failed ="+
                        result,
                        Toast.LENGTH_SHORT).show();
                return;
            }

            if (mSetEventReportFlag == 0) { // getNotifyRequest response.
                SharedPreferences.Editor editor = getSharedPrefEditor(
                        AppGlobalState.IMS_PRESENCE_SETTINGS);

                int publishTriggerInd;
                int enablerInd;
                int notifyInd;


                publishTriggerInd = data & 0x01;
                enablerInd = data & 0x02;
                notifyInd = data & 0x04;

                String s;
                String val;
                Settings.EventReport rep = new Settings.EventReport();

                s = mContext.getString(R.string.enable_publish_trigger_ind_text);
                val = (publishTriggerInd == 0x01) ? ("true") : ("false");
                rep.publishTriggerInd = (publishTriggerInd == 0x01) ? (1) : (0);
                editor.putString(s, val);

                s = mContext.getString(R.string.enable_enabler_state_ind_text);
                val = (enablerInd == 0x02) ? ("true") : ("false");
                rep.enablerInd = (enablerInd == 0x02) ? (1) : (0);
                editor.putString(s, val);

                s = mContext.getString(R.string.enable_notify_ind_text);
                val = (notifyInd == 0x04) ? ("true") : ("false");
                rep.notifyInd = (notifyInd == 0x04) ? (1) : (0);
                editor.putString(s, val);

                editor.commit();

                updateSettingsActivity(rep);

                Toast.makeText(mContext, "Get event report done ",
                        Toast.LENGTH_SHORT).show();

            } else { //SET
                Toast.makeText(
                        mContext,
                        "Set event report request status"
                        + response.result,
                        Toast.LENGTH_SHORT).show();
            }
        }

    private void updateSettingsActivity(Settings.EventReport rep) {
        SettingsMainThreadHandler settingsHandler = AppGlobalState
            .getSettingsHandler();
        Message m = settingsHandler.obtainMessage(
                SettingsMainThreadHandler.SET_EVENT_REPORT_RESPONSE, rep);
        settingsHandler.sendMessage(m);

    }


}
