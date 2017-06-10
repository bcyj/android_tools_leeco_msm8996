/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Spinner;
import android.widget.TextView;

public class Settings extends Activity {
    final String TAG = "Settings";

    int FIELD_TAG = 0;
    int FIELD_VALUE = 1;
    Context mContext;
    AsyncTask mNotifyFormatTask;
    AsyncTask mEventReportTask;

    public static class EventReport {
        int publishTriggerInd;
        int notifyInd;
        int enablerInd;

        @Override
            public String toString() {
                return "EventReport [publishTriggerInd=" + publishTriggerInd
                    + ", notifyInd=" + notifyInd + ", enablerInd=" + enablerInd
                    + "]";
            }

        public EventReport(int publishTriggerInd, int notifyInd, int enablerInd) {
            this.publishTriggerInd = publishTriggerInd;
            this.notifyInd = notifyInd;
            this.enablerInd = enablerInd;
        }

        public EventReport() {
            // TODO Auto-generated constructor stub
        }
    }

    int formMap[][] = {

        { R.string.set_notify_fmt_text, R.id.notify_fmt_Spinner },

        { R.string.enable_publish_trigger_ind_text,
            R.id.enable_publish_trigger_ind_checkbox },
        { R.string.enable_enabler_state_ind_text,
            R.id.enable_enabler_state_ind_checkbox },
        { R.string.enable_notify_ind_text,
            R.id.enable_notify_ind_checkbox },

        { R.string.availability_cache_exp_text,
            R.id.availability_cache_exp_value },
        { R.string.capability_cache_exp_text,
            R.id.capability_cache_exp_value },
        { R.string.capability_poll_interval_text,
            R.id.capability_poll_interval_value },
        { R.string.capability_poll_list_exp_text,
            R.id.capability_poll_list_exp_value },
        { R.string.source_throttle_publish_text,
            R.id.source_throttle_publish_value },
        { R.string.subscribe_retry_text, R.id.subscribe_retry_value },
        { R.string.publish_retry_text, R.id.publish_retry_value },
        { R.string.max_list_entries_text, R.id.max_list_entries_value },

    };


    class SettingsMainThreadHandler extends Handler {

        final static int SET_NOTIFY_FORMAT_RESPONSE = 0;
        final static int SET_EVENT_REPORT_RESPONSE = 1;

        @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case SET_NOTIFY_FORMAT_RESPONSE:
                    {
                        String val = (String) msg.obj;
                        Log.d(TAG,
                                "Received msg in Setting activity main thread. msg.obj="
                                + val);
                        updateNotifyFmtValue(val);
                        break;
                    }
                    case SET_EVENT_REPORT_RESPONSE :
                    {
                        EventReport rep = (EventReport) msg.obj;
                        Log.d(TAG, rep.toString());
                        updateEventReportValue(rep);
                        break;
                    }

                    default:
                        Log.e(TAG, "Unknown msg");
                }

                super.handleMessage(msg);
            }

        private void updateEventReportValue(EventReport rep) {

            SharedPreferences setting = getSharedPrefHandle(
                    AppGlobalState.IMS_PRESENCE_SETTINGS);

            Boolean val;
            CheckBox c;

            val = (rep.publishTriggerInd == 1)? true: false;
            c = (CheckBox)findViewById(R.id.enable_publish_trigger_ind_checkbox);
            c.setChecked(val);

            val = (rep.notifyInd == 1)? true: false;
            c = (CheckBox)findViewById(R.id.enable_notify_ind_checkbox);
            c.setChecked(val);

            val = (rep.enablerInd == 1)? true: false;
            c = (CheckBox)findViewById(R.id.enable_enabler_state_ind_checkbox);
            c.setChecked(val);

        }

        private void updateNotifyFmtValue(String val) {
            Spinner spinner = (Spinner) findViewById(R.id.notify_fmt_Spinner);
            ArrayAdapter adapter = (ArrayAdapter) spinner.getAdapter();
            int pos = adapter.getPosition(val);
            spinner.setSelection(pos);
        }
    }

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mContext = this;
            this.setContentView(R.layout.settings);

            SettingsMainThreadHandler settingsHandler = new SettingsMainThreadHandler();
            AppGlobalState.setSettingsHandler(settingsHandler);

            initButtons();

            populateInitialValues();
        }

    @Override
    protected void onStop() {
        super.onStop();
        finish();
    }

    private void initButtons() {
        Button okButton = (Button) findViewById(R.id.ok);
        handleOkButtonClick(okButton);

        Button cancelButton = (Button) findViewById(R.id.cancel);
        handleCancelButtonClick(cancelButton);

        Button setNotifyFmtButton = (Button) findViewById(R.id.set_notify_fmt_button);
        handleSetNotifyButtonClick(setNotifyFmtButton);
        Button getNotifyFmtButton = (Button) findViewById(R.id.get_notify_current_fmt_button);
        handleGetNotifyButtonClick(getNotifyFmtButton);

        Button setEventReporttButton = (Button) findViewById(R.id.set_event_report_button);
        handleSetEventReportButtonClick(setEventReporttButton);
        Button getEventReportButton = (Button) findViewById(R.id.get_event_report_button);
        handleGetEventReportButtonClick(getEventReportButton);
    }

    private void handleGetEventReportButtonClick(Button button) {
        button.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mEventReportTask = (EventReportTask) new EventReportTask(
                    mContext).execute(0); //GET
            }
        });

    }

    private void handleSetEventReportButtonClick(Button button) {
        button.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                SharedPreferences.Editor editor = getSharedPrefEditor(
                    AppGlobalState.IMS_PRESENCE_SETTINGS);
                storeFormValuesToSharedPreferences(editor);

                mEventReportTask = (EventReportTask) new EventReportTask(
                    mContext).execute(1); //SET
            }
        });

    }

    private void handleSetNotifyButtonClick(Button setNotifyFmtButton) {
        setNotifyFmtButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                SharedPreferences.Editor editor = getSharedPrefEditor(
                    AppGlobalState.IMS_PRESENCE_SETTINGS);
                storeFormValuesToSharedPreferences(editor);

                mNotifyFormatTask = (NotifyFormatTask) new NotifyFormatTask(
                    mContext).execute(1);
            }
        });
    }

    private void handleGetNotifyButtonClick(Button getNotifyFmtButton) {
        getNotifyFmtButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mNotifyFormatTask = (NotifyFormatTask) new NotifyFormatTask(
                    mContext).execute(0);
            }
        });
    }

    private void handleOkButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

                SharedPreferences.Editor editor = getSharedPrefEditor(
                    AppGlobalState.IMS_PRESENCE_SETTINGS);
                storeFormValuesToSharedPreferences(editor);
                finish();
            }
        });
    }

    private void handleCancelButtonClick(Button cancelButton) {
        cancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                finish();
            }
        });
    }

    private void populateInitialValues() {
        SharedPreferences setting = getSharedPrefHandle(
                AppGlobalState.IMS_PRESENCE_SETTINGS);

        for (int i = 0; i < formMap.length; i++) {
            String uriValue = setting.getString(
                    getString(formMap[i][FIELD_TAG]), "");

            if (formMap[i][FIELD_VALUE] == R.id.notify_fmt_Spinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                ArrayAdapter adapter = (ArrayAdapter) spinner.getAdapter();
                int pos = adapter.getPosition(uriValue);
                spinner.setSelection(pos);

            } else if  (formMap[i][FIELD_VALUE] == R.id.enable_publish_trigger_ind_checkbox ||
                    formMap[i][FIELD_VALUE] == R.id.enable_enabler_state_ind_checkbox ||
                    formMap[i][FIELD_VALUE] == R.id.enable_notify_ind_checkbox) {
                CheckBox c =  (CheckBox) findViewById(formMap[i][FIELD_VALUE]);
                Boolean val = new Boolean(uriValue);
                c.setChecked(val);

            }else {
                TextView t = (TextView) findViewById(formMap[i][FIELD_VALUE]);
                t.setText(uriValue);
            }
        }
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private Editor getSharedPrefEditor(String imsPresencePref) {
        SharedPreferences settings = getSharedPrefHandle(imsPresencePref);
        SharedPreferences.Editor editor = settings.edit();
        return editor;
    }

    private void storeFormValuesToSharedPreferences(Editor editor) {
        for (int i = 0; i < formMap.length; i++) {
            if (formMap[i][FIELD_VALUE] == R.id.notify_fmt_Spinner) {

                Spinner spinner = (Spinner) findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]), spinner
                        .getSelectedItem().toString());

            } else if  (formMap[i][FIELD_VALUE] == R.id.enable_publish_trigger_ind_checkbox ||
                    formMap[i][FIELD_VALUE] == R.id.enable_enabler_state_ind_checkbox ||
                    formMap[i][FIELD_VALUE] == R.id.enable_notify_ind_checkbox) {
                CheckBox c =  (CheckBox) findViewById(formMap[i][FIELD_VALUE]);
                Boolean val = c.isChecked();

                editor.putString(getString(formMap[i][FIELD_TAG]), val.toString());

            }
            else {
                TextView t = (TextView) findViewById(formMap[i][FIELD_VALUE]);
                editor.putString(getString(formMap[i][FIELD_TAG]), t.getText()
                        .toString());
            }
        }
        editor.commit();
    }

}
