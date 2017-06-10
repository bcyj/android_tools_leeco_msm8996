/*
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dm;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.os.SystemProperties;

public class DmNetwork {
    private static final String TAG = DmReceiver.DM_TAG + "DmNetwork: ";

    public static final int DMNetworkType = ConnectivityManager.TYPE_MOBILE_FOTA;
    public static final String DMFeatureEnable = Phone.FEATURE_ENABLE_FOTA;
    private ConnectivityManager mConnMgr = null;
    private static BroadcastReceiver mNetworkStateIntentReceiver = null;
    private IntentFilter mNetworkStateChangedFilter = null;
    private final Object mConnectLock = new Object();
    private int mConnectStatus = -1;

    Context mContext;
    public int connectiontype = -1;
    private static DmNetwork mInstance;
    String mType = DmService.getInstance().getAPN();

    public static DmNetwork getInstance()
    {
        return mInstance;
    }

    DmNetwork(Context context) {
        mContext = context;
        mInstance = this;
    }

    public void init() {
        mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        create_netstate_receiver();
        registe_netstate_receiver();
    }

    public void end()
    {
        unregiste_netstate_receiver();
    }

    private boolean isNetworkAvailable() {
        // NetworkInfo networkInfo;

        // if(mConnMgr!=null)
        // {
        // networkInfo =
        // mConnMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        // }

        return true;
    }

    private boolean isNetworkAvailable(String modem) {
        // return mConnMgr.isAvailable(ConnectivityManager.TYPE_MOBILE, modem);
        return true;
    }

    public int beginConnectivity(String type) {

        if (mConnMgr == null) {
            Log.v(TAG, "[ppp link] begin connectivity fail, mConnMgr is null");
            return PhoneConstants.APN_REQUEST_FAILED;
        }
        Log.v(TAG, "[ppp link] begin connectivity, type: " + type);

        mType = type;

        connectiontype=DMNetworkType;
        Log.d(TAG,"connectiontype="+connectiontype);
        if(isNetworkAvailable(null)) {
            Log.d(TAG,"startUsingNetworkFeature for DM");
            if (DmService.getInstance().showDataConnectDialog()) {
                mConnectStatus =
                    mConnMgr.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
                DMFeatureEnable);
            } else {
                mConnectStatus = PhoneConstants.APN_TYPE_NOT_AVAILABLE;
            }
        } else {
            mConnectStatus = PhoneConstants.APN_TYPE_NOT_AVAILABLE;
        }

        switch (mConnectStatus) {
            case PhoneConstants.APN_REQUEST_STARTED: {
                synchronized (mConnectLock)
                {
                    try {
                        mConnectLock.wait(30 * 1000);
                        Log.d(TAG, "///////// mConnectLock got the notify mConnectStatus "
                                + mConnectStatus);
                    } catch (InterruptedException e) {
                        Log.d(TAG, "#### InterruptedException ###");
                    }

                    return mConnectStatus;
                }
            }
            case PhoneConstants.APN_ALREADY_ACTIVE:
            case PhoneConstants.APN_TYPE_NOT_AVAILABLE:
            case PhoneConstants.APN_REQUEST_FAILED:
                return mConnectStatus;
            default:
                break;
        }

        return mConnectStatus;
    }

    public void endConnectivity() {
        try {
            if (mConnMgr != null) {
                Log.v(TAG, "[ppp link] end connectivity");
                mConnMgr.stopUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE, DMFeatureEnable);
            }
        } finally {
            connectiontype = -1;
            // do nothing
        }
    }

    private void create_netstate_receiver() {
        if (mNetworkStateIntentReceiver != null) {
            return;
        }
        Log.v(TAG, "[ppp link] create a net state receiver");

        mNetworkStateChangedFilter = new IntentFilter();
        mNetworkStateChangedFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

        mNetworkStateIntentReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "[ppp link] network state receiver on-receive");
                Log.d(TAG, "[ppp link] intent.getAction() = " + intent.getAction());
                if (intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                    Log.v(TAG, "[ppp link] network state receiver got CONNECTIVITY_ACTION");
                    boolean Connectivity = false;
                    NetworkInfo ni = (NetworkInfo) intent
                            .getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
                    Log.d(TAG, "[ppp link] network state receiver got CONNECTIVITY_ACTION ni: "
                            + ni);
                    Log.d(TAG, "[ppp link] network state receiver got ni.getType() ni.getType(): "
                            + ni.getType());
                    if (ni != null && ni.getType() == connectiontype)
                    {
                        if (ni.getState() == NetworkInfo.State.CONNECTED)
                        {
                            Connectivity = true;
                        }
                        else
                        {
                            Connectivity = false;
                        }

                        Log.d(TAG, "<<<<<<<<<<<<<<<<<connected: " + Connectivity);

                        if (Connectivity)
                        {
                            //
                            mConnectStatus = PhoneConstants.APN_ALREADY_ACTIVE;
                            DmJniInterface.setNetworkState(mConnectStatus);
                            synchronized (mConnectLock) {
                                try {
                                    Log.d(TAG, "///////// notify mConnectLock ");
                                    mConnectLock.notifyAll();
                                } catch (Exception e) {
                                    Log.d(TAG, e.toString());
                                }
                            }

                        }
                        else
                        {
                            mConnectStatus = PhoneConstants.APN_REQUEST_FAILED;
                            DmJniInterface.setNetworkState(mConnectStatus);
                            synchronized (mConnectLock) {
                                try {
                                    Log.d(TAG, "///////// notify mConnectLock ");
                                    mConnectLock.notifyAll();
                                } catch (Exception e) {
                                    Log.d(TAG, e.toString());
                                }
                            }
                        }
                    }
                }
            }
        };
    }

    private void registe_netstate_receiver() {
        if (mNetworkStateChangedFilter != null &&
                mNetworkStateIntentReceiver != null) {
            Log.v(TAG, "[ppp link] registe receiver");
            mContext.registerReceiver(mNetworkStateIntentReceiver,
                    mNetworkStateChangedFilter);
        }
    }

    private void unregiste_netstate_receiver() {
        if (mNetworkStateIntentReceiver != null) {
            Log.v(TAG, "[ppp link] unregiste receiver");
            mContext.unregisterReceiver(mNetworkStateIntentReceiver);
            mNetworkStateIntentReceiver = null;
            mNetworkStateChangedFilter = null;
        }
    }
}
