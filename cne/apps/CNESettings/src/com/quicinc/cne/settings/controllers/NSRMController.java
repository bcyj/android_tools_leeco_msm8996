/*
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.quicinc.cne.settings.controllers;


import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.preference.Preference;
import android.content.SharedPreferences;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceScreen;
import android.preference.ListPreference;
import android.preference.EditTextPreference;
import android.util.Log;
import android.content.SharedPreferences;

import com.quicinc.cne.settings.R;
import com.quicinc.cne.settings.common.CneCommon;
import com.quicinc.cneapiclient.CNEManager;
import com.quicinc.cneapiclient.CNEManager.NSRMFeatureGetType;
import com.quicinc.cneapiclient.CNEManager.NSRMFeatureSetType;
import com.quicinc.cneapiclient.CNEManagerException;

public class NSRMController extends BaseController {

   protected static String TAG = "CneNSRMController";

   private PreferenceScreen mPreferenceScreen;
   private ListPreference mListPreference;
   private EditTextPreference mPolFilePathPref;
   private Preference mPolVersionPref;
   private Preference mPolUpdatePref;
   private boolean mFeatureEnabled = false;
   private boolean mSwitchEnabled = false;
   private String polVersion;
   private String polFilePath;

   private IntentFilter mIntentFilter;

   private BroadcastReceiver mReceiver = new BroadcastReceiver() {
       public void onReceive(Context context, Intent intent) {
           final String action = intent.getAction();
           int feature = intent.getIntExtra(CNEManager.EXTRA_FEATURE_ID,
                                            CNEManager.EXTRA_VALUE_UNKNOWN);
           if (action.equals(CNEManager.CNE_PREFERENCE_CHANGED_ACTION) &&
               feature == CNEManager.NSRM_FEATURE) {
               int parameter = intent.getIntExtra(CNEManager.EXTRA_FEATURE_PARAMETER,
                                                  CNEManager.EXTRA_VALUE_UNKNOWN);
               switch(parameter) {
                   case CNEManager.NSRM_FEATURE_ENABLED:
                       int value = intent.getIntExtra(CNEManager.EXTRA_PARAMETER_VALUE,
                               CNEManager.EXTRA_VALUE_UNKNOWN);
                       nsrmUpdateListPreference(value);
                       break;

                   default:
                       Log.d(TAG, "Received parameter unknown");
                       break;
               }
           }
       }
   };

    public NSRMController(Context context, PreferenceScreen preferencesScreen) {
       super(context, preferencesScreen);

       mFeatureEnabled = false;
       mSwitchEnabled = false;
       polVersion = "";
       polFilePath = "";
       mPreferenceScreen = preferencesScreen;

       mListPreference =
           (ListPreference) preferencesScreen.findPreference(CneCommon.NSRM_LIST);
       if(mListPreference == null) {
           Log.e(TAG,"ListPreference is missing.");
       }

       mPolFilePathPref =
            (EditTextPreference) preferencesScreen.findPreference(CneCommon.NSRM_FILE_PATH_KEY);
       mPolVersionPref =
            preferencesScreen.findPreference(CneCommon.NSRM_VERSION_KEY);
       mPolUpdatePref =
            preferencesScreen.findPreference(CneCommon.NSRM_UPDATE_KEY);
       mPolUpdatePref.setOnPreferenceClickListener(
                new OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference pref) {
                      new AlertDialog.Builder(mContext)
                        .setTitle("Policy Update")
                        .setMessage("Updating policy at "+ polFilePath)
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
       polFilePath = sp.getString(CneCommon.NSRM_FILE_PATH_KEY, "");

       int value = SystemProperties.getInt("persist.cne.feature", 0);
       mSwitchEnabled = (value == 4 || value == 6);
       mIntentFilter = new IntentFilter(CNEManager.CNE_PREFERENCE_CHANGED_ACTION);

       if (mSwitchEnabled) {
           if (mListPreference != null) {
               mListPreference.setEnabled(true);
           }
           try{
               mFeatureEnabled = mCNEManager.getNSRMEnabled(NSRMFeatureGetType.SYNC_CONNECT_DNS_WRITE);
               if (mFeatureEnabled) {
                   nsrmUpdateListPreference(3);
               } else  {
                   mFeatureEnabled = mCNEManager.getNSRMEnabled(NSRMFeatureGetType.SYNC_CONNECT_DNS);

                   if (mFeatureEnabled) {
                       nsrmUpdateListPreference(2);
                   } else {
                       nsrmUpdateListPreference(1);
                   }
               }
               polVersion = mCNEManager.getPolicyVersion(CNEManager.POLICY_TYPE_NSRM);
           } catch( CNEManagerException cne ){
               Log.e(TAG,""+cne);
           }
       }
       mPolVersionPref.setSummary(polVersion);
       mPolFilePathPref.setSummary(polFilePath);
    }

    protected String processUpdatePolicy() {
        if (!"".equals(polFilePath)) {
          try {
                mCNEManager.updatePolicy(CNEManager.POLICY_TYPE_NSRM,
                                polFilePath);
              } catch (CNEManagerException cne ){
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

    protected void processOnSharedPreferenceChanged(
            SharedPreferences sharedpreference, String key) {
        if (key.equals(CneCommon.NSRM_FILE_PATH_KEY)) {
            mPolFilePathPref = (EditTextPreference) mPreferenceScreen.findPreference(key);
            polFilePath = sharedpreference.getString(key,"");
            mPolFilePathPref.setSummary(polFilePath);
            Log.i(TAG,"mPolFilePathPref updated " + polFilePath);
        }
        if (key.equals(CneCommon.NSRM_LIST)) {
            int value = Integer.parseInt(mListPreference.getValue());
            Log.i(TAG,"mListPreference received " + value);
            switch (value)
            {
                case 1:
                    try{
                        mCNEManager.setNSRMEnabled(NSRMFeatureSetType.OFF);
                    } catch( CNEManagerException cne ){
                        Log.e(TAG,""+cne);
                    }
                    break;
                case 2:
                    try{
                        mCNEManager.setNSRMEnabled(NSRMFeatureSetType.SYNC_CONNECT_DNS);
                    } catch( CNEManagerException cne ){
                        Log.e(TAG,""+cne);
                    }
                    break;
                case 3:
                    try{
                        mCNEManager.setNSRMEnabled(NSRMFeatureSetType.SYNC_CONNECT_DNS_WRITE);
                    } catch( CNEManagerException cne ){
                        Log.e(TAG,""+cne);
                    }
                    break;
                default :
                    break;
            }
        }
    }


    protected boolean processOnPreferenceChange(Preference preference, Object newValueObj) {
        Log.i(TAG,"processOnPreferenceChange() " + newValueObj);

        return true;
    }


    protected void processOnPause(){
        mPreferenceScreen.getSharedPreferences()
                .unregisterOnSharedPreferenceChangeListener(this);
        if (mListPreference != null){
            mListPreference.setOnPreferenceChangeListener(null);
        }
        if (mReceiver != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }

    protected void processOnResume(){
        mPolVersionPref.setSummary(polVersion);
        polFilePath = mPolFilePathPref.getSummary().toString();
        mPolFilePathPref.setSummary(polFilePath);

        mPreferenceScreen.getSharedPreferences()
                .registerOnSharedPreferenceChangeListener(this);
        if (mListPreference != null){
            mListPreference.setOnPreferenceChangeListener(this);
        }
        mContext.registerReceiver(mReceiver, mIntentFilter);
    }

    private void nsrmUpdateListPreference(int value) {

       switch (value) {
           case 1:
               mListPreference.setValue("1");
               break;
           case 2:
               mListPreference.setValue("2");
               break;
           case 3:
               mListPreference.setValue("3");
               break;
           default:
               break;
       }
       mListPreference.setSummary("%s");
   }
}
