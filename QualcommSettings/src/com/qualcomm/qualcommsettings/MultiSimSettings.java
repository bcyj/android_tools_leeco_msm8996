/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qualcommsettings;

import com.qualcomm.qcrilhook.IQcRilHook;

import com.qualcomm.qcrilhook.QcRilHook;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Looper;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.RadioButton;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import android.util.Log;

import java.util.List;

public class MultiSimSettings extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "MultiSimSettings";

    private static final String TUNE_AWAY_RECEIVER = "tune_away_receiver";
    private static final int TUNE_AWAY_RECEIVER_DISABLED = 0;
    private static final int TUNE_AWAY_RECEIVER_ENABLED = 1;
    private CheckBoxPreference mTuneAwayReceiver;
    private boolean mTuneAway;

    private static final String PRIORITY_SUBSCRIPTION = "priority_subscription";
    private ListPreference mPrioritySubscription;
    private CharSequence[] entries; // ListPrefrences requires entry array
    private CharSequence[] entryValues; // ListPrefrences requires entry values array
    private CharSequence[] mPrioritySubscriptionSummaryText;
    private CharSequence[] mPrioritySubscriptionEntries;
    private CharSequence[] mPrioritySubscriptionEntryValues;
    private int mSubscriptionIndex;

    private static final int EVENT_SET_TUNE_AWAY = 10;
    private static final int EVENT_SET_TUNE_AWAY_DONE = 11;
    private static final int EVENT_SET_PRIORITY_SUBSCRIPTION = 12;
    private static final int EVENT_SET_PRIORITY_SUBSCRIPTION_DONE = 13;

    private QcRilHook mQcRilHook;
    private Context mContext;

    private HandlerThread qcRilHookHandlerThread;
    private QcRilHandler qcRilHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.multi_sim_settings);
        Log.d(TAG, "MultiSimSettings onCreate");

        mContext = getApplicationContext();
        mQcRilHook = new QcRilHook(mContext);

        mTuneAwayReceiver = (CheckBoxPreference) findPreference(TUNE_AWAY_RECEIVER);
        mTuneAwayReceiver.setOnPreferenceChangeListener(this);

        mPrioritySubscription = (ListPreference) findPreference(PRIORITY_SUBSCRIPTION);
        mPrioritySubscription.setOnPreferenceChangeListener(this);

        initializePrioritySubcriptionListPreferences();

        qcRilHookHandlerThread = new HandlerThread("QcRilHookHandlerThread");
        qcRilHookHandlerThread.start();
        qcRilHandler = new QcRilHandler(qcRilHookHandlerThread.getLooper());
    }

    @Override
    protected void onResume() {
        super.onResume();

        mPrioritySubscription.setEntries(mPrioritySubscriptionEntries);
        mPrioritySubscription.setEntryValues(mPrioritySubscriptionEntryValues);

        updateTuneAwayReceiverSummary();
        updatePrioritySubscriptionSummary();
    }

    private String getDisplayNameFromSub(int subId) {
        SubscriptionInfo subInfo =
                SubscriptionManager.from(mContext).getActiveSubscriptionInfo(subId);
        return subInfo == null ? "" : subInfo.getDisplayName().toString();
    }

    private void initializePrioritySubcriptionListPreferences() {
        Log.d(TAG, "Initialize mPrioritySubscription ListPrefrences entry and entry values");

        List<SubscriptionInfo> activeSubscriptions = SubscriptionManager
                .from(this).getActiveSubscriptionInfoList();
        int numberOfActiveSubscriptions = activeSubscriptions == null ? 0 : activeSubscriptions
                .size();
        Log.d(TAG, "Number of Active Subscriptions: " + numberOfActiveSubscriptions);

        mPrioritySubscriptionEntries = new CharSequence[numberOfActiveSubscriptions];
        mPrioritySubscriptionEntryValues = new CharSequence[numberOfActiveSubscriptions];
        mPrioritySubscriptionSummaryText = new CharSequence[numberOfActiveSubscriptions];

        for (int i = 0; i < numberOfActiveSubscriptions; i++) {
            int id = activeSubscriptions.get(i).getSubscriptionId();
            String displayName = getDisplayNameFromSub(id);
            Log.d(TAG, "Active Subscription id = " + id + " displayName = " + displayName);

            mPrioritySubscriptionEntries[i] = displayName;
            mPrioritySubscriptionEntryValues[i] = Integer.toString(id);
            mPrioritySubscriptionSummaryText[i] = displayName;
        }

        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        entries = new CharSequence[numPhones];
        entryValues = new CharSequence[numPhones];

        for (int i = 0; i < numPhones; i++) {
            Log.d(TAG, "initPreferences index " + i);
            SubscriptionInfo subscription = SubscriptionManager.from(mContext)
                    .getActiveSubscriptionInfoForSimSlotIndex(i);
            entries[i] = subscription == null ? "" :
                    subscription.getDisplayName().toString();
            entryValues[i] = Integer.toString(i);
            Log.d(TAG, "Subscription id = " + entryValues[i] + " displayName = " + entries[i]);
        }
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        final String key = preference.getKey();

        Log.d(TAG, "onPreferenceChange: " + key + " changed to " + newValue.toString());

        if (key.equals(TUNE_AWAY_RECEIVER)) {
            mTuneAway = ((Boolean) newValue).booleanValue();
            qcRilHandler.sendMessage(qcRilHandler.obtainMessage(EVENT_SET_TUNE_AWAY, mTuneAway));
        }

        if (PRIORITY_SUBSCRIPTION.equals(key)) {
            mSubscriptionIndex = Integer.parseInt((String) newValue);
            qcRilHandler.sendMessage(qcRilHandler.obtainMessage(EVENT_SET_PRIORITY_SUBSCRIPTION,
                    mSubscriptionIndex));
        }

        return true;
    }

    private void updateTuneAwayReceiverSummary() {
        boolean tuneAway = (Settings.Global.getInt(getContentResolver(),
                Settings.Global.TUNE_AWAY_STATUS, 0) == 1);
        Log.d(TAG, " updateTuneAwayReceiverSummary to " + tuneAway);

        mTuneAwayReceiver.setChecked(tuneAway);
        mTuneAwayReceiver.setSummary(tuneAway ?
                getResources().getString(R.string.tune_away_receiver_enabled) :
                getResources().getString(R.string.tune_away_receiver_disabled));
    }

    private void updatePrioritySubscriptionSummary() {
        mPrioritySubscription.setEntries(entries);
        mPrioritySubscription.setEntryValues(entryValues);

        try {
            int prioritySubIndex = Settings.Global.getInt(getContentResolver(),
                    Settings.Global.MULTI_SIM_PRIORITY_SUBSCRIPTION);
            Log.d(TAG, " updatePrioritySubscriptionSummary setting value to " + prioritySubIndex);

            mPrioritySubscription.setValue(Integer.toString(prioritySubIndex));
            mPrioritySubscription.setSummary(mPrioritySubscriptionSummaryText[prioritySubIndex]);
        } catch (SettingNotFoundException snfe) {
            Log.e(TAG, "Settings Exception Reading MSim Priority Subscription Values");
        }
    }

     private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg != null) {
                Log.d(TAG, "handleMessage " + msg.what);
            }

            switch (msg.what) {

                case EVENT_SET_TUNE_AWAY_DONE:
                    boolean qcRilTuneAwayResult = (boolean) msg.obj;
                    if (qcRilTuneAwayResult) {
                        if (mTuneAway) {
                            Settings.Global.putInt(getContentResolver(),
                                    Settings.Global.TUNE_AWAY_STATUS,
                                    TUNE_AWAY_RECEIVER_ENABLED);
                        } else {
                            Settings.Global.putInt(getContentResolver(),
                                    Settings.Global.TUNE_AWAY_STATUS,
                                    TUNE_AWAY_RECEIVER_DISABLED);
                        }

                        updateTuneAwayReceiverSummary();
                    } else {
                        Log.e(TAG, "QCRIL TUNEAWAY Value NOT SET");
                    }
                    break;

                case EVENT_SET_PRIORITY_SUBSCRIPTION_DONE:
                    boolean qcRilPrioritySubResult = (boolean) msg.obj;
                    if (qcRilPrioritySubResult) {
                        Settings.Global.putInt(getContentResolver(),
                                        Settings.Global.MULTI_SIM_PRIORITY_SUBSCRIPTION,
                                        mSubscriptionIndex);

                        updatePrioritySubscriptionSummary();
                    } else {
                        Log.e(TAG, "QCRIL PrioritySubscription Value NOT SET");
                    }
                    break;
            }
        }
    };

    private class QcRilHandler extends Handler {

        QcRilHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage (Message msg) {
            switch (msg.what) {
                case EVENT_SET_TUNE_AWAY:
                    boolean tuneAway = (boolean) msg.obj;
                    boolean qcRilSetTuneAwayResult = mQcRilHook.qcRilSetTuneAway(tuneAway);
                    Log.d(TAG, "QCRIL TUNEAWAY  result: " + qcRilSetTuneAwayResult);
                    mHandler.sendMessage(
                            mHandler.obtainMessage(EVENT_SET_TUNE_AWAY_DONE,
                                                    qcRilSetTuneAwayResult));
                    break;

                case EVENT_SET_PRIORITY_SUBSCRIPTION:
                    int subscriptionIndex = ((int) msg.obj);
                    boolean result = mQcRilHook.qcRilSetPrioritySubscription(subscriptionIndex);
                    Log.d(TAG, "QCRIL PrioritySubscription  result: " + result);
                    mHandler.sendMessage(
                            mHandler.obtainMessage(EVENT_SET_PRIORITY_SUBSCRIPTION_DONE, result));
                    break;
            }
        }
     }

}
