/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.quicinc.cne.settings.controllers;

import com.quicinc.cne.settings.common.CneCommon;
import com.quicinc.cneapiclient.CNEManager;
import com.quicinc.cneapiclient.CNEManagerException;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
//import android.net.FeatureConfig;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceScreen;
import android.preference.TwoStatePreference;
import android.os.SystemProperties;

import android.util.Log;

import android.content.IntentFilter;

import com.quicinc.cne.settings.R;

public class IWLANController extends BaseController implements DialogInterface.OnDismissListener,
   DialogInterface.OnClickListener {

   protected static String TAG = "CneIWLANController";

   private TwoStatePreference mTwoStatePreference;
   private boolean mFeatureEnabled = false;
   private boolean mSwitchEnabled = true;
   private boolean mLastClickProcessed = true;
   private boolean mUserAction = true;
   private boolean mEnableAction = false;
   private boolean mPositivePressed = false;

   private IntentFilter mIntentFilter;

   private BroadcastReceiver mReceiver = new BroadcastReceiver() {
       public void onReceive(Context context, Intent intent) {
           final String action = intent.getAction();
           int feature = intent.getIntExtra(CNEManager.EXTRA_FEATURE_ID, CNEManager.EXTRA_VALUE_UNKNOWN);
           if (action.equals(CNEManager.CNE_PREFERENCE_CHANGED_ACTION) &&
               feature == CNEManager.IWLAN_FEATURE) {
               int parameter = intent.getIntExtra(CNEManager.EXTRA_FEATURE_PARAMETER,
                                                  CNEManager.EXTRA_VALUE_UNKNOWN);
               switch(parameter) {
                   case CNEManager.IWLAN_FEATURE_ENABLED:
                       int value = intent.getIntExtra(CNEManager.EXTRA_PARAMETER_VALUE,
                                                      CNEManager.EXTRA_VALUE_UNKNOWN);
                       if (value == CNEManager.IWLAN_FEATURE_ON) {
                           mFeatureEnabled = true;
                       } else if (value == CNEManager.IWLAN_FEATURE_OFF) {
                           mFeatureEnabled = false;
                       } else {
                           Log.d(TAG,"Recieved state unknown.");
                       }
                       mLastClickProcessed = true;
                       setSwitchEnabled(true);
                       setSwitchChecked(mFeatureEnabled);
                       mPositivePressed = false;
                       break;
                   default:
                       Log.d(TAG, "Recieved parameter unknown");
               }
               Log.d(TAG, "Recieved preference update mFeatureEnabled = " + mFeatureEnabled);
           }
       }
    };

    public IWLANController(Context context, PreferenceScreen preferencesScreen) {
       super(context, preferencesScreen);

       mTwoStatePreference =
           (TwoStatePreference) preferencesScreen.findPreference(CneCommon.IWLAN_TOGGLE);
       if(mTwoStatePreference == null) {
           Log.e(TAG,"TwoStatePreference is missing.");
           //TBD
       }

       setSwitchEnabled(false);
       mIntentFilter = new IntentFilter(CNEManager.CNE_PREFERENCE_CHANGED_ACTION);

       mFeatureEnabled = mCNEManager.getIWLANEnabled();
       setSwitchChecked(mFeatureEnabled);
    }

   /**
     * @hide
     */
   public void onClick(DialogInterface dialog, int which) {
       if (which == DialogInterface.BUTTON_POSITIVE) {
           mPositivePressed = true;
           mCNEManager.setIWLANEnabled(true);
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

    protected String processUpdatePolicy() {
        return "OK";
    }

    protected void processOnSharedPreferenceChanged(
            SharedPreferences sharedpreference, String key) {
        //TODO:
    }

    protected boolean processOnPreferenceChange(Preference preference, Object newValueObj) {
        Log.i(TAG,"processOnPreferenceChange()");
        boolean result = false;
        String key = preference.getKey();
        if(CneCommon.IWLAN_TOGGLE.equals(key)){
            Log.i(TAG,"processOnPreferenceChange() iwlan toggle clicked");
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
                        mContext.getResources().getString(R.string.iwlan_enable_warning))
                        .setTitle(android.R.string.dialog_alert_title)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setPositiveButton(android.R.string.yes, this)
                        .setNegativeButton(android.R.string.no, this)
                        .show()
                        .setOnDismissListener(this);
            } else {
                mCNEManager.setIWLANEnabled(false);
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
                mEnableAction = true;
                mTwoStatePreference.setEnabled(value);
        }
    }

   protected void processOnPause(){
       if (mTwoStatePreference != null){
           mTwoStatePreference.setOnPreferenceChangeListener(null);
       }
       mContext.unregisterReceiver(mReceiver);
   }

   protected void processOnResume(){
       if (mTwoStatePreference != null){
           mTwoStatePreference.setOnPreferenceChangeListener(this);
       }
       mFeatureEnabled = mCNEManager.getIWLANEnabled();
       setSwitchChecked(mFeatureEnabled);
       mContext.registerReceiver(mReceiver, mIntentFilter);
   }
}
