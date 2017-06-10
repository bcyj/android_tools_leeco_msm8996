/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.quicinc.cne.settings.controllers;

import com.quicinc.cne.settings.common.CneCommon;
import com.quicinc.cneapiclient.CNEManager;
import com.quicinc.cneapiclient.CNEManagerException;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceScreen;
import android.preference.TwoStatePreference;
import android.preference.EditTextPreference;
import android.os.SystemProperties;

import android.util.Log;

import android.content.IntentFilter;

import com.quicinc.cne.settings.R;

public class WQEController extends BaseController implements DialogInterface.OnDismissListener,
   DialogInterface.OnClickListener {

   protected static String TAG = "CneWQEController";

   private PreferenceScreen mPreferenceScreen;
   private TwoStatePreference mTwoStatePreference;
   private EditTextPreference mPolicyFilePathPreference;
   private Preference mPolicyVersionPreference;
   private Preference mPolicyUpdatePreference;
   private boolean mFeatureEnabled;
   private boolean mSwitchEnabled;
   private boolean mLastClickProcessed;
   private boolean mUserAction;
   private boolean mEnableAction;
   private boolean mPositivePressed;
   private String andsfVersion;
   private String andsfFilePath;

   private IntentFilter mIntentFilter;

   private BroadcastReceiver mReceiver = new BroadcastReceiver() {
       public void onReceive(Context context, Intent intent) {
           final String action = intent.getAction();
           int feature = intent.getIntExtra(CNEManager.EXTRA_FEATURE_ID,
                                            CNEManager.EXTRA_VALUE_UNKNOWN);
           if (action.equals(CNEManager.CNE_PREFERENCE_CHANGED_ACTION) &&
               feature == CNEManager.WQE_FEATURE) {
               int parameter = intent.getIntExtra(CNEManager.EXTRA_FEATURE_PARAMETER,
                                                  CNEManager.EXTRA_VALUE_UNKNOWN);
               switch(parameter) {
               case CNEManager.WQE_FEATURE_ENABLED:
                   int value = intent.getIntExtra(CNEManager.EXTRA_PARAMETER_VALUE,
                                                  CNEManager.EXTRA_VALUE_UNKNOWN);
                   if (value == CNEManager.WQE_FEATURE_ON) {
                       mFeatureEnabled = true;
                   } else if (value == CNEManager.WQE_FEATURE_OFF) {
                       mFeatureEnabled = false;
                   } else {
                       Log.d(TAG,"Received state unknown.");
                   }
                   mLastClickProcessed = true;
                   setSwitchEnabled(true);
                   setSwitchChecked(mFeatureEnabled);
                   mPositivePressed = false;
                   break;
               default:
                   Log.d(TAG, "Received parameter unknown");
               }
               Log.d(TAG, "Received preference update mFeatureEnabled = " + mFeatureEnabled);
           }
       }
    };

   public WQEController(Context context, PreferenceScreen preferencesScreen) {
       super(context, preferencesScreen);

       mFeatureEnabled = false;
       mSwitchEnabled = true;
       mLastClickProcessed = true;
       mUserAction = true;
       mEnableAction = false;
       mPositivePressed = false;
       andsfVersion = "";
       andsfFilePath = "";
       mPreferenceScreen = preferencesScreen;
       mTwoStatePreference =
           (TwoStatePreference) preferencesScreen.findPreference(CneCommon.WQE_TOGGLE);
       if(mTwoStatePreference == null) {
           Log.e(TAG,"TwoStatePreference is missing.");
           //TBD
       }

       mPolicyFilePathPreference =
           (EditTextPreference) preferencesScreen.findPreference(CneCommon.ANDSF_FILE_PATH_KEY);
       mPolicyVersionPreference =
           preferencesScreen.findPreference(CneCommon.ANDSF_VERSION_KEY);
       mPolicyUpdatePreference =
           preferencesScreen.findPreference(CneCommon.ANDSF_UPDATE_KEY);
       mPolicyUpdatePreference.setOnPreferenceClickListener(
               new OnPreferenceClickListener() {
                   public boolean onPreferenceClick(Preference pref) {
                     new AlertDialog.Builder(mContext)
                         .setTitle("Policy Update")
                         .setMessage("Updating policy at "+ andsfFilePath)
                         .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                //continue to update policy
                                new UpdatePolicyTask().execute();
                            }
                         })
                         .setNegativeButton("No", new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                // do nothing
                            }
                         })
                         .show();
                     return false;
                   }
               });

       SharedPreferences sp = mPreferenceScreen.getSharedPreferences();
       andsfFilePath = sp.getString(CneCommon.ANDSF_FILE_PATH_KEY, "");

       int value = SystemProperties.getInt("persist.cne.feature", 0);
       mSwitchEnabled = (value == 3 || value == 6);
       setSwitchEnabled(mSwitchEnabled);
       mIntentFilter = new IntentFilter(CNEManager.CNE_PREFERENCE_CHANGED_ACTION);
       try {
           mFeatureEnabled = mCNEManager.getWQEEnabled();
           andsfVersion = mCNEManager.getPolicyVersion(CNEManager.POLICY_TYPE_ANDSF);
           setSwitchChecked(mFeatureEnabled);
       } catch ( CNEManagerException cne ){
           Log.e(TAG,""+cne);
       }

       mPolicyVersionPreference.setSummary(andsfVersion);
       mPolicyFilePathPreference.setSummary(andsfFilePath);
   }

   protected String processUpdatePolicy() {
       if (!"".equals(andsfFilePath)) {
           try {
               mCNEManager.updatePolicy(CNEManager.POLICY_TYPE_ANDSF,
                       andsfFilePath);
           } catch ( CNEManagerException cne ){
               Log.e(TAG,""+cne);
               return cne.toString();
           }
       }
       else {
           Log.e(TAG,"file path cannot be NULL");
           return "Error: file path cannot be empty";
       }
       return "Updated policy successfully";
   }

   /**
     * @hide
     */
   public void onClick(DialogInterface dialog, int which) {
       if (which == DialogInterface.BUTTON_POSITIVE) {
           mPositivePressed = true;
           try {
             mCNEManager.setWQEEnabled(true);
           } catch( CNEManagerException cne ){
             setSwitchChecked(false);
             Log.e(TAG,""+cne);
           }
       } else {
           mPositivePressed = false;
           mTwoStatePreference.setChecked(false);
       }
   }

    /**
     * @hide
     */
    public void onDismiss(DialogInterface dialog) {
        if (!mPositivePressed) {
           mLastClickProcessed = true;
           setSwitchEnabled(true);
           setSwitchChecked(mFeatureEnabled);
        }
    }

    protected void processOnSharedPreferenceChanged(
            SharedPreferences sharedPref, String key) {
        if (key.equals(CneCommon.ANDSF_FILE_PATH_KEY)) {
            mPolicyFilePathPreference = (EditTextPreference) mPreferenceScreen.findPreference(key);
            andsfFilePath = sharedPref.getString(key,"");
            mPolicyFilePathPreference.setSummary(andsfFilePath);
            Log.i(TAG,"mPolicyFilePathPref updated "+ andsfFilePath);
        }
    }

    protected boolean processOnPreferenceChange(Preference preference, Object newValueObj) {
        Log.i(TAG,"processOnPreferenceChange()");
        boolean result = false;
        String key = preference.getKey();
        if(CneCommon.WQE_TOGGLE.equals(key)){
            Log.i(TAG,"processOnPreferenceChange() wqe toggle clicked");
            if (mEnableAction) {
                Log.i(TAG,"processOnPreferenceChange() enable action.");
                mEnableAction = false;
                return true;
            }
            if (!mUserAction) {
                Log.i(TAG,"processOnPreferenceChange() not an user action.");
                mUserAction = true;
                return true;
            }
            if (!mLastClickProcessed) {
                Log.i(TAG,"processOnPreferenceChange() previous request is not yet processed.");
                return false;
            }
            setSwitchEnabled(false);
            boolean newValue = ((Boolean)newValueObj).booleanValue();
            mLastClickProcessed = false;
            if(newValue) {
                new AlertDialog.Builder(mContext).setMessage(
                        mContext.getResources().getString(R.string.wqe_enable_warning))
                        .setTitle(android.R.string.dialog_alert_title)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setPositiveButton(android.R.string.yes, this)
                        .setNegativeButton(android.R.string.no, this)
                        .show()
                        .setOnDismissListener(this);
            } else {
                try {
                  mCNEManager.setWQEEnabled(false);
                } catch( CNEManagerException cne ){
                  Log.e(TAG,""+cne);
                }
            }
            result = true;
        } else {
            Log.i(TAG,"processOnPreferenceChange() preference key: " + key);
            result = false;
        }
       return result;
   }

    private void setSwitchChecked(boolean value){
        if(mTwoStatePreference.isChecked() != value) {
                mUserAction = false;
                mTwoStatePreference.setChecked(value);
        }
    }

    private void setSwitchEnabled(boolean value){
        if(mTwoStatePreference.isEnabled() != value) {
                mEnableAction = !value;
                mTwoStatePreference.setEnabled(value);
        }
    }

    protected void processOnPause(){
        mPreferenceScreen.getSharedPreferences()
                .unregisterOnSharedPreferenceChangeListener(this);
        if (mTwoStatePreference != null){
            mTwoStatePreference.setOnPreferenceChangeListener(null);
        }

        mContext.unregisterReceiver(mReceiver);
    }

    protected void processOnResume(){
        mPolicyVersionPreference.setSummary(andsfVersion);
        andsfFilePath = mPolicyFilePathPreference.getSummary().toString();
        mPolicyFilePathPreference.setSummary(andsfFilePath);

        if (mTwoStatePreference != null){
            mTwoStatePreference.setOnPreferenceChangeListener(this);
        }
        try {
          mFeatureEnabled = mCNEManager.getWQEEnabled();
          setSwitchChecked(mFeatureEnabled);
        } catch ( CNEManagerException cne ){
          Log.e(TAG,""+cne);
        }
        mContext.registerReceiver(mReceiver, mIntentFilter);
        mPreferenceScreen.getSharedPreferences()
                .registerOnSharedPreferenceChangeListener(this);

    }
}
