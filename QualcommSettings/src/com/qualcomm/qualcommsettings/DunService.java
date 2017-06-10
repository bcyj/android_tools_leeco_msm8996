/******************************************************************************
  @file    DunService.java
  @brief   Background Service for DUN.

  DESCRIPTION

  Background Service for DUN - Internal Data Call Enable/Disable

  ---------------------------------------------------------------------------
  Copyright (C) 2009,2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qualcommsettings;

import android.app.Service;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.os.RemoteException;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.content.ContentResolver;

import android.content.Context;
import android.os.ServiceManager;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.ReadableByteChannel;
import java.nio.channels.Channels;
import java.nio.charset.Charset;
import java.lang.Exception;

import android.util.Log;
import android.util.Config;

import android.telephony.TelephonyManager;
import com.android.internal.telephony.ITelephony;
import android.os.SystemProperties;
import android.os.SystemClock;

/**
 * Background Dun Service
 */
public class DunService extends Service {

   /**
    * Variables
    */
   private static final String TAG = "Dun Service";
   private static final int RETRY_COUNT = 3;
   private static final int DUN_SLEEP = 5000;

   private ITelephony telephony_service;
   private boolean mDunArbitrationStarted = false;
   /**
    * Binder Function
    */
   @Override
      public IBinder onBind(Intent intent) {

         return m_binder;

      }


   private final com.qualcomm.qualcommsettings.IDun.Stub m_binder = new com.qualcomm.qualcommsettings.IDun.Stub() {


      /**
       * Enable/Disable Socket Data Call
       */
      public void enableSocketDataCall(boolean value) throws RemoteException {
         int enableSdc = value? 1:0;
         Log.i(TAG," caller's uid " + m_binder.getCallingUid()
                    + ", pid " + m_binder.getCallingPid());

         /* Enforce permissions here. Only privileged Users
          * can avail this service
          */
         if (m_binder.getCallingUid() == Process.SYSTEM_UID ||
                 m_binder.getCallingUid() == Process.BLUETOOTH_UID) {
             messageHandler.sendMessage(Message.obtain(messageHandler, enableSdc));
         }
      }

   };

   @Override
      public void onCreate() {

         telephony_service = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
         if (telephony_service == null) {
            Log.e(TAG, "Dun Service Failed");
         } else {
            Log.i(TAG, "Dun Background Service Created Successfully");
         }

      }

   @Override
      public void onStart(Intent intent, int startId) {

         super.onStart(intent, startId);

         // publish the service
         if (ServiceManager.getService("dun") == null) {
             ServiceManager.addService("dun", m_binder);
         }

      }

   @Override
      public void onDestroy() {

         Log.w(TAG, "Background Service Destroyed Successfully ...");
         super.onDestroy();

      }

   private boolean enable_DataConnectivity(boolean value) {

       boolean result = false;
       if (value == true) {
           try {
               int retry_count = RETRY_COUNT;
               while (retry_count > 0) {
                   result = telephony_service.enableDataConnectivity();
                   if (result == true) {
                       Log.i(TAG, "Success: telephony_service.enableDataConnectivity");
                       break;
                   } else {
                       Log.i(TAG, "Failure: telephony_service.enableDataConnectivity");
                       retry_count--;
                   }
               }
           } catch (RemoteException e) {
               Log.e(TAG, "Exception - Data Call Not Enabled");
           }
       } else if (value == false) {
           try {
               int retry_count = RETRY_COUNT;
               while (retry_count > 0) {
                   result = telephony_service.disableDataConnectivity();
                   if (result == true) {
                       Log.i(TAG, "Success: telephony_service.disableDataConnectivity");
                       break;
                   } else {
                       Log.i(TAG, "Failure: telephony_service.disableDataConnectivity");
                       retry_count--;
                   }
               }
           } catch (RemoteException e) {
               Log.e(TAG, "Exception - Data Call Not Disabled");
           }
      }
      return result;
   }

   private Handler messageHandler = new Handler() {

      @Override
         public void handleMessage(Message msg) {

            switch (msg.what) {
               case 0:
                  ConnectivityManager cm = (ConnectivityManager)
                          getSystemService(Context.CONNECTIVITY_SERVICE);
                  // Disable socket data call
                  if (cm != null) {
                      if (cm.getMobileDataEnabled()) {
                          mDunArbitrationStarted = enable_DataConnectivity(false);
                      } else {
                          mDunArbitrationStarted = false;
                          Log.i(TAG, "Embedded data call was already disabled");
                      }
                  }
                  break;
               case 1:
                  // Enable socket data call
                  if (mDunArbitrationStarted) {
                      enable_DataConnectivity(true);
                      mDunArbitrationStarted = false;
                  } else {
                      Log.i(TAG, "Embedded data call is not disabled from here");
                  }
                  break;
               default:
                  break;
            }
         }
   };
}

