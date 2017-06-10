/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.vzw.nfc;

import java.lang.reflect.Method;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.content.ComponentName;
import android.os.AsyncTask;
import android.os.RemoteException;
import android.util.Log;

import qcom.nfc.IQNfcSecureElementManager;

public final class AidFilter {
    private static final String TAG = "AidFilter";
    final static boolean DBG = true;

    private IQNfcSecureElementManager mQNfcSecureElementManager = null;
    boolean mIsBound = false;
    MyConnection mConn = null;
    Context mContext = null;
    Object mServiceConnected;

    byte[] mFilterList;
    boolean mEnableFilterConditionPending;
    boolean mDisableFilterConditionPending;
    byte mFilterConditionTag;

    protected static Context getContext() {
        Context context = null;
        try {
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            Class[] params = new Class[0];
            Method currentActivityThread = activityThreadClass.getDeclaredMethod("currentActivityThread", params);
            Boolean accessible = currentActivityThread.isAccessible();
            currentActivityThread.setAccessible(true);
            Object obj = currentActivityThread.invoke(activityThreadClass);
            if (obj == null) {
                Log.d(TAG, "The current activity thread is null!");
                return null;
            }
            currentActivityThread.setAccessible(accessible);
            Method getSystemContext = activityThreadClass.getDeclaredMethod("getSystemContext", params);
            accessible = getSystemContext.isAccessible();
            getSystemContext.setAccessible(true);
            context = (Context) getSystemContext.invoke(obj);
            getSystemContext.setAccessible(accessible);
        } catch (Exception e) {
            Log.d(TAG, Log.getStackTraceString(e));
        }
        return context;
    }

    public class MyConnection implements ServiceConnection {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            if (DBG) Log.d(TAG, "onServiceConnected");
            mQNfcSecureElementManager = IQNfcSecureElementManager.Stub.asInterface(service);
            if (mServiceConnected != null) {
                synchronized (mServiceConnected) {
                    mServiceConnected.notify();
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            if (DBG) Log.d(TAG, "onServiceDiscnnected");
            mQNfcSecureElementManager = null;
        }
    };

    void bindService(Context context) {
        if (DBG) Log.d(TAG, "bindService()");
        Intent intent = new Intent(IQNfcSecureElementManager.class.getName());
        intent.setClassName("com.android.nfc",
                            "com.android.nfc.SecureElementService");
        mIsBound = context.bindService(intent, mConn, Context.BIND_AUTO_CREATE);
    }

    void unbindService(Context context) {
        if (DBG) Log.d(TAG, "unbindService()");
        if (mIsBound) {
            context.unbindService(mConn);
            mIsBound = false;
        }
    }

    /* Async task */
    GetNfcAdaptersTask mGetNfcAdaptersTask;

    private class GetNfcAdaptersTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
        }

        @Override
        protected Void doInBackground(Void... arg0) {
            try {
                mServiceConnected = new Object();
                mContext = getContext();
                if (mConn == null ) {
                    mConn = new MyConnection();
                    bindService(mContext);
                }
                synchronized (mServiceConnected) {
                    mServiceConnected.wait(1000);
                }
                mServiceConnected = null;

                if (mQNfcSecureElementManager == null) {
                    Log.e(TAG, "GetNfcAdaptersTask: cannot get QNfcSecureElementManager ");
                } else {
                    if (mFilterList != null) {
                        mQNfcSecureElementManager.setClfAidFilterList(mFilterList);
                        mFilterList = null;
                    } else if (mEnableFilterConditionPending) {
                        mQNfcSecureElementManager.enableClfAidFilterCondition(mFilterConditionTag);
                        mEnableFilterConditionPending = false;
                    } else if (mDisableFilterConditionPending) {
                        mQNfcSecureElementManager.disableClfAidFilterCondition(mFilterConditionTag);
                        mDisableFilterConditionPending = false;
                    }
                }
            } catch( Exception e ){
                Log.e(TAG, "doInBackground(): got exception");
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            mGetNfcAdaptersTask = null;
        }
    }

    public boolean setFilterList(byte[] filterList) {
        if (DBG) Log.d(TAG, "setFilterList()");
        try {
            if (mQNfcSecureElementManager != null) {
                return mQNfcSecureElementManager.setClfAidFilterList(filterList);
            } else {
                mFilterList = filterList;
                mGetNfcAdaptersTask = new GetNfcAdaptersTask();
                mGetNfcAdaptersTask.execute();
                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, "setFilterList(): Exception - " + e.getMessage());
            return false;
        }
    }

    public boolean enableFilterCondition(byte filterConditionTag) {
        if (DBG) Log.d(TAG, "enableFilterCondition()");
        try {
            if (mQNfcSecureElementManager != null) {
                return mQNfcSecureElementManager.enableClfAidFilterCondition(filterConditionTag);
            } else {
                mEnableFilterConditionPending = true;
                mFilterConditionTag = filterConditionTag;
                mGetNfcAdaptersTask = new GetNfcAdaptersTask();
                mGetNfcAdaptersTask.execute();
                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, "enableFilterCondition(): Exception - " + e.getMessage());
            return false;
        }
    }

    public boolean disableFilterCondition(byte filterConditionTag) {
        if (DBG) Log.d(TAG, "disableFilterCondition()");
        try {
            if (mQNfcSecureElementManager != null) {
                return mQNfcSecureElementManager.disableClfAidFilterCondition(filterConditionTag);
            } else {
                mDisableFilterConditionPending = true;
                mFilterConditionTag = filterConditionTag;
                mGetNfcAdaptersTask = new GetNfcAdaptersTask();
                mGetNfcAdaptersTask.execute();
                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, "disableFilterCondition(): Exception - " + e.getMessage());
            return false;
        }
    }
}

