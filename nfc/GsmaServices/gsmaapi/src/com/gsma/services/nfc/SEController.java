/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.gsma.services.nfc;

import java.util.HashMap;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;

import android.os.Handler;
import android.os.Message;
import android.os.IBinder;

import android.util.Log;

public final class SEController {

    private static final String TAG = "GsmaSEController";
    final static boolean DBG = true;

    public static final String SE_NAME_SIM1 = "SIM1";
    public static final String SE_NAME_SIM2 = "SIM2";
    public static final String SE_NAME_ESE1 = "eSE1";
    public static final String SE_NAME_ESE2 = "eSE2";
    public static final String SE_NAME_SD1  = "SD1";

    static HashMap<Context, SEController> sSEControllerMap = new HashMap();

    private Context mContext;

    public interface Callbacks {
        public abstract void onGetDefaultController (SEController seController);
    }

    private volatile IGsmaService mGsmaService;
    boolean mIsBound;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mGsmaService = IGsmaService.Stub.asInterface(service);
        }
        public void onServiceDisconnected(ComponentName className) {
            mGsmaService = null;
        }
    };

    void bindService(Context context) {
        if (DBG) Log.d(TAG, "bindService()");
        mIsBound = ServiceUtils.bindService(context, mConnection);
    }

    void unbindService(Context context) {
        if (DBG) Log.d(TAG, "unbindService()");
        if (mIsBound) {
            context.unbindService(mConnection);
            mIsBound = false;
        }
    }

    private SEController (Context context) {
        mContext = context;
        bindService (mContext);
    }

    // Helper to return the instance of the SE Controller in a callback function
    // context : the callling application's context
    // cb : callback to use for the default controller
    public static synchronized void getDefaultController (Context context, SEController.Callbacks cb) {

        if (DBG) Log.d(TAG, "getDefaultController()");

        SEController seController = sSEControllerMap.get(context);
        if (seController == null) {
            seController = new SEController(context);
            sSEControllerMap.put(context, seController);
        }
        cb.onGetDefaultController (seController);
    }

    // return the name of the active Secure Element
    public String getActiveSecureElement() {
        if (DBG) Log.d(TAG, "getActiveSecureElement()");
        try {
            return mGsmaService.getActiveSecureElement();
        } catch (Exception e) {
            Log.e(TAG, "mGsmaService.getActiveSecureElement() : " + e.getMessage());
            return null;
        }
    }

    // sets a specified Secure Element as the active one
    public void setActiveSecureElement(String SEName) {
        if (DBG) Log.d(TAG, "setActiveSecureElement() " + SEName);

        try {
            if (mGsmaService.isNfccEnabled() == false) {
                throw new IllegalStateException("NFC adapter is not enabled");
            }
        } catch (Exception e) {
            Log.e(TAG, "mGsmaService.isNfccEnabled() : " + e.getMessage());
        }

        if ((SEName == null)||
            ((SEName.equals(SE_NAME_SIM1)==false)&&
             (SEName.equals(SE_NAME_SIM2)==false)&&
             (SEName.equals(SE_NAME_ESE1)==false)&&
             (SEName.equals(SE_NAME_ESE2)==false)&&
             (SEName.equals(SE_NAME_SD1)==false))) {
            throw new IllegalArgumentException("Invalid SE Name");
        }

        try {
            mGsmaService.setActiveSecureElement(SEName);
        } catch (Exception e) {
            Log.e(TAG, "mGsmaService.setActiveSecureElement() : " + e.getMessage());
            // SecurityException - if the application is not allowed to use this API
            throw new SecurityException("application is not allowed");
        }
    }

    // after a call to this method, any authorized/registered components can receive a transaction
    // event. This call shall not imply a power cycle of the mobile device, the modem or the UICC.
    public void enableMultiEvt_transactionReception (String SEName) {
        if (DBG) Log.d(TAG, "enableMultiEvt_transactionReception() " + SEName);

        if ((SEName == null)||
            ((SEName.equals(SE_NAME_SIM1)==false)&&
             (SEName.equals(SE_NAME_SIM2)==false)&&
             (SEName.equals(SE_NAME_ESE1)==false)&&
             (SEName.equals(SE_NAME_ESE2)==false)&&
             (SEName.equals(SE_NAME_SD1)==false))) {
            throw new IllegalArgumentException("Invalid SE Name");
        }

        try {
            mGsmaService.enableMultiReception(SEName);
        } catch (Exception e) {
            Log.e(TAG, "mGsmaService.enableMultiReception() : " + e.getMessage());
            // SecurityException - if the application is not allowed to use this API
            throw new SecurityException("application is not allowed");
        }
    }
}


