/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.quicinc.cne.settings.controllers;

import com.quicinc.cne.settings.common.CneCommon;

import android.content.Context;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.SharedPreferences;
import android.preference.PreferenceScreen;
import android.os.AsyncTask;
import dalvik.system.PathClassLoader;

import android.os.IBinder;
import android.os.IInterface;

import java.lang.Class;
import java.lang.reflect.Method;
import android.os.ServiceManager;

import java.lang.Exception;

import android.util.Log;
import android.widget.Toast;
import android.view.Gravity;
import android.content.IntentFilter;
import com.quicinc.cneapiclient.CNEManager;
import com.quicinc.cne.settings.R;

public abstract class BaseController
       implements OnPreferenceChangeListener, OnSharedPreferenceChangeListener {

   protected static String TAG = "BaseController";
   protected Context mContext;
   protected PreferenceScreen mPreferencesScreen;
   protected CNEManager mCNEManager;

   public BaseController(Context context, PreferenceScreen preferencesScreen) {
       Log.i(TAG,"Constructor");
       if (context == null){
       Log.e(TAG,"Context is missing.");
           throw new NullPointerException();
       }

       if (preferencesScreen == null){
           Log.e(TAG,"PreferenceScreen is missing.");
           throw new NullPointerException();
       }

       mContext = context;
       mPreferencesScreen = preferencesScreen;

       try
       {
           mCNEManager = new CNEManager(context);
       }
       catch (Exception e)
       {
           Log.e(TAG,"CNEManager initialization fails: " + e.toString());
           throw new NullPointerException();
       }
   }

   public void onResume() {
       Log.i(TAG,"onResume()");
       processOnResume();
   }

   public void onPause() {
       Log.i(TAG,"onPause()");
       processOnPause();
   }

   @Override
   public boolean onPreferenceChange(Preference preference, Object newValueObj) {
       Log.i(TAG,"onPreferenceChange()");
       return processOnPreferenceChange(preference, newValueObj);
   }

   @Override
   public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key)
   {
       Log.i(TAG,"onSharedPreferenceChanged()");
       processOnSharedPreferenceChanged(sharedPreferences, key);
   }

   protected abstract void processOnPause();
   protected abstract void processOnResume();

   protected abstract boolean processOnPreferenceChange(Preference preference, Object newValueObj);
   protected abstract void processOnSharedPreferenceChanged(SharedPreferences sharedpreference, String key);
   protected abstract String processUpdatePolicy();

   protected class UpdatePolicyTask extends AsyncTask<Object, Void, String> {
       protected String doInBackground(Object... params) {
           return processUpdatePolicy();
       }

       protected void onPostExecute(String resp) {
           makeToast(resp);
       }
   }

   protected void makeToast(String text) {
       Toast t = Toast.makeText( mContext, text, Toast.LENGTH_LONG);
       t.setGravity(Gravity.CENTER | Gravity.BOTTOM, 0, 0);
       t.show();
   }
}
