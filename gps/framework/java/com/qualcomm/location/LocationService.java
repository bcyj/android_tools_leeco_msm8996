/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/

/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2011,2012, The Linux Foundation. All rights reserved.
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

package com.qualcomm.location;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Date;
import java.util.List;
import java.util.Properties;
import java.util.Map.Entry;
import java.util.Random;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;
import android.provider.Telephony.Carriers;
import android.database.sqlite.SQLiteException;

import com.qualcomm.location.GpsXtraDownloader;
import com.qualcomm.location.GpsNetInitiatedHandler;
import com.qualcomm.location.GpsNetInitiatedHandler.GpsNiNotification;

import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.ContentObserver;
import android.location.Location;
import android.location.LocationManager;
import android.location.INetInitiatedListener;
import android.telephony.TelephonyManager;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.NetworkInfo;
import android.net.Network;
import android.net.NetworkRequest;
import android.net.NetworkCapabilities;
import android.net.Uri;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.util.NtpTrustedTime;

public class LocationService extends Service {
    private static final String TAG = "LocSvc_java";
    private static boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG);

    //Timeout for ConnectivityManager.requestNetwork
    //The modem timeout is 30 seconds. We'll try to send back a response
    //before it times out
    private static final int NETWORK_REQUEST_TIMEOUT_MS = 1000 * 29;
    //Network states passed from network callback
    private static final int NETWORK_UNAVAILABLE = 0;
    private static final int NETWORK_AVAILABLE   = 1;

    // these need to match GpsApgsStatusValue defines in gps.h
    /** AGPS status event values. */
    private static final int GPS_REQUEST_AGPS_DATA_CONN = 1;
    private static final int GPS_RELEASE_AGPS_DATA_CONN = 2;
    private static final int GPS_AGPS_DATA_CONNECTED = 3;
    private static final int GPS_AGPS_DATA_CONN_DONE = 4;
    private static final int GPS_AGPS_DATA_CONN_FAILED = 5;

    // The GPS_CAPABILITY_* flags must match the values in gps.h
    private static final int GPS_CAPABILITY_SCHEDULING = 0x0000001;
    private static final int GPS_CAPABILITY_MSB = 0x0000002;
    private static final int GPS_CAPABILITY_MSA = 0x0000004;
    private static final int GPS_CAPABILITY_SINGLE_SHOT = 0x0000008;
    private static final int GPS_CAPABILITY_ON_DEMAND_TIME = 0x0000010;

    // Handler messages
    private static final int ENABLE = 2;
    private static final int HANDLE_NETWORK_CALLBACK = 3;
    private static final int UPDATE_NETWORK_STATE = 4;
    private static final int INJECT_NTP_TIME = 5;
    private static final int DOWNLOAD_XTRA_DATA = 6;
    private static final int INJECT_NTP_TIME_FINISHED = 10;
    private static final int DOWNLOAD_XTRA_DATA_FINISHED = 11;
    private static final int REPORT_AGPS_STATUS = 12;

    // how often to request NTP time, in milliseconds
    // current setting 24 hours
    private static final long NTP_INTERVAL = 24*60*60*1000;

    // the long interval to retry xtra data download after hitting the cap, in milliseconds
    // current setting 24 hours
    private static final long XTRA_DATA_INTERVAL = 24*60*60*1000;

    // retry interval base for both NTP and xtra data download, in milliseconds
    // current setting 4 minutes
    private static final long RETRY_INTERVAL_BASE = 4*60*1000;

    // retry caps for NTP and xtra data download
    private static final int NTP_RETRY_TIMES_CAP = 12;
    private static final int XTRA_DATA_RETRY_TIMES_CAP = 3;

    // states for injecting ntp and downloading xtra data
    private static final int STATE_PENDING_NETWORK = 0;
    private static final int STATE_DOWNLOADING = 1;
    private static final int STATE_IDLE = 2;

    // flags to trigger NTP or XTRA data download when network becomes available
    // initialized to pending to do NTP and XTRA automatically after booting
    private int mInjectNtpTimePending = STATE_PENDING_NETWORK;
    private int mDownloadXtraDataPending = STATE_IDLE;

    private static final String PROPERTIES_FILE = "/etc/gps.conf";

    // capabilities of the GPS engine
    private int mEngineCapabilities;

    // true if XTRA is supported
    private boolean mSupportsXtra;

    // set to true if the GPS engine does not do on-demand NTP time requests
    private boolean mPeriodicTimeInjection;

    // true if we are enabled, protected by this
    private boolean mEnabled;

    // true if we have wifi network connectivity
    private boolean mWifiNetworkAvailable;

    // true if we have mobile network connectivity
    private boolean mMobileNetworkAvailable;

    // properties loaded from PROPERTIES_FILE
    private Properties mProperties;

    private Context mContext;
    private NtpTrustedTime mNtpTime;

    private ConnectivityManager mConnMgr;
    private GpsNetInitiatedHandler mNIHandler;

    private String mDefaultApn;

    // NTP and xtra data download retry times
    private int mNTPRetryTimes = 0;
    private int mXtraDataRetryTimes = 0;

    private boolean mWifiConnectivity = false;
    private boolean mWwanConnectivity = false;


    // Wakelocks
    private final static String WAKELOCK_KEY = "LocationService";
    private PowerManager.WakeLock mWakeLock;

    private WifiState mWifiState = null;

    private Object mLock = new Object();

    //Request to send to the Connectivity Manager for starting a data call
    private NetworkRequest mWwanNetworkRequest = new NetworkRequest.Builder().addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR).build();
    private NetworkRequest mSuplNetworkRequest = new NetworkRequest.Builder().addCapability(NetworkCapabilities.NET_CAPABILITY_SUPL).build();

    //Callback to send to ConnectivityManager
    private ConnectivityManager.NetworkCallback mWwanNetworkCallback=null;
    private ConnectivityManager.NetworkCallback mSuplNetworkCallback=null;

    //This class is used in sendMessage() to indicate whether or not a wakelock
    //is held when sending the message.
    private static class LocSvcMsgObj {
        Object obj;
        boolean wakeLockHeld;
        public LocSvcMsgObj (Object obj, Boolean wakeLock) {
            this.obj = obj;
            this.wakeLockHeld = wakeLock;
        }
    };

    @Override
    public void onCreate() {
        if (DEBUG)
            Log.d(TAG, "onCreate ");

        mContext = this;
        mNtpTime = NtpTrustedTime.getInstance(mContext);
        mNIHandler = new GpsNetInitiatedHandler(mContext, mNetInitiatedListener);

        // Create a wake lock
        PowerManager powerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, WAKELOCK_KEY);
        mWakeLock.setReferenceCounted(true);

        mWifiState = new WifiState();

        mConnMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

        mProperties = new Properties();
        try {
            File file = new File(PROPERTIES_FILE);
            FileInputStream stream = new FileInputStream(file);
            mProperties.load(stream);
            stream.close();

            String debugLevelString = mProperties.getProperty("DEBUG_LEVEL");
            if (debugLevelString != null) {
                try {
                    int debugLevel = Integer.parseInt(debugLevelString.trim());
                    if (debugLevel > 3)
                        DEBUG = true;
                } catch (NumberFormatException e) {
                    Log.e(TAG, "unable to parse DEBUG_LEVEL: " + debugLevelString);
                }
            }
        } catch (IOException e) {
            Log.w(TAG, "Could not open GPS configuration file " + PROPERTIES_FILE);
        }

        listenForBroadcasts();

        enable();

    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void listenForBroadcasts() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        mContext.registerReceiver(mBroadcastReciever, intentFilter, null, mHandler);
        Uri uri = Uri.parse("content://telephony/carriers/preferapn");
        mContext.getContentResolver().registerContentObserver(
                uri, false, mDefaultApnObserver);
    }

    private final BroadcastReceiver mBroadcastReciever = new BroadcastReceiver() {
        @Override public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DEBUG) Log.d(TAG, "onReceive");
            if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                if (DEBUG) Log.d(TAG, "got connectivity action");
                // retrieve NetworkInfo result for this UID
                NetworkInfo info =
                        intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);

                int networkState;
                if (intent.getBooleanExtra(ConnectivityManager.EXTRA_NO_CONNECTIVITY, false) ||
                    !info.isConnected()) {
                    if (info.getType() == ConnectivityManager.TYPE_WIFI) {
                        mWifiConnectivity = false;
                    } else {
                        mWwanConnectivity = false;
                    }

                    if (!mWifiConnectivity && !mWwanConnectivity) {
                        if (DEBUG)
                            Log.d(TAG, "broadcast - no network connections");
                        networkState = NETWORK_UNAVAILABLE;
                    } else {
                        // reset the network info to the current connection
                        info = mConnMgr.getActiveNetworkInfo();
                        if ((info != null) && info.isConnected()) {
                            networkState = NETWORK_AVAILABLE;
                        } else {
                            if (DEBUG)
                                Log.d(TAG, "connectivity mgr - no network connections");
                            mWifiConnectivity = false;
                            mWwanConnectivity = false;
                            networkState = NETWORK_UNAVAILABLE;
                        }
                    }
                } else {
                    if (info.getType() == ConnectivityManager.TYPE_WIFI) {
                        mWifiConnectivity = true;
                    } else {
                        mWwanConnectivity = true;
                    }
                    networkState = NETWORK_AVAILABLE;
                }

                if (DEBUG) Log.d(TAG, "Connectivity status :" +
                                 " mWifiConnectivity - "
                                 + mWifiConnectivity +
                                 "; mWwanConnectivity - "
                                 + mWwanConnectivity);

                sendMessage(UPDATE_NETWORK_STATE, networkState, info);
            }
            else if (action.equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)) {
                /**
                 * When a client wants to connect to a specific SSID, we don't want
                 * them to block forever in the case that this SSID disappears.
                 * If we notice that's gone, we will return failure right away to
                 * the client.
                 */
                String ssid = null;

                if (mWifiState.state == mWifiState.WIFI_STATE_OPENING) {
                  ssid = mWifiState.currentSSID;
                } else if (mWifiState.state == mWifiState.WIFI_STATE_CLOSING) {
                  ssid = mWifiState.originalSSID;
                } else {
                  //do nothing
                  return;
                }

                List<ScanResult> results = mWifiState.mWifiManager.getScanResults();
                for (ScanResult result : results) {
                    String ssid_result = convertToQuotedString (result.SSID);
                    if (ssid_result.equals(ssid)) {
                      return;
                    }
                }

                //What we were waiting on is no longer here. Fail.
                mWifiState.handleFailure();
            }
        }
    };

    private void sendMessage(int message, int arg1, int arg2, Object obj) {
        // hold a wake lock until this message is delivered
        // note that this assumes the message will not be removed from the queue before
        // it is handled (otherwise the wake lock would be leaked).
        if(DEBUG) {
            Log.d(TAG, "Sending msg: "+ message + " arg1:"+ arg1 +" arg2:"+ arg2);
        }
        mWakeLock.acquire();
        //wakeLockHeld in LocSvcMsgObj is set to true to indicate that
        //a wakelock is held before sending this message
        mHandler.obtainMessage(message, arg1, arg2,
                               new LocSvcMsgObj(obj, true)).sendToTarget();
    }

    private void sendMessage(int message, int arg, Object obj) {
        sendMessage(message, arg, 1, obj);
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int message = msg.what;
            LocSvcMsgObj msgObj=null;
            if (msg.obj != null) {
                msgObj = (LocSvcMsgObj)msg.obj;
            }
            switch (message) {
                case ENABLE:
                    if (msg.arg1 == 1) {
                        handleEnable();
                    } else {
                        handleDisable();
                    }
                    break;
                case UPDATE_NETWORK_STATE:
                    if(msgObj != null) {
                        handleUpdateNetworkState(msg.arg1, (NetworkInfo)msgObj.obj);
                    }
                    break;
                case INJECT_NTP_TIME:
                    handleInjectNtpTime();
                    break;
                case DOWNLOAD_XTRA_DATA:
                    if (mSupportsXtra) {
                        handleDownloadXtraData();
                    }
                    break;
                case INJECT_NTP_TIME_FINISHED:
                    mInjectNtpTimePending = STATE_IDLE;
                    break;
                case DOWNLOAD_XTRA_DATA_FINISHED:
                    mDownloadXtraDataPending = STATE_IDLE;
                    break;
                case REPORT_AGPS_STATUS:
                    if(msgObj != null) {
                        handleReportAgpsStatus((ReportAgpsStatusMessage)msgObj.obj);
                    }
                    break;
                case HANDLE_NETWORK_CALLBACK:
                    if(msgObj != null) {
                        handleNetworkCallback(msg.arg1, msg.arg2, (Network)msgObj.obj);
                    }
                    break;
            }
            // if wakelock was taken for this message, release it
            if((msgObj != null) && (msgObj.wakeLockHeld)) {
                mWakeLock.release();
            }
        }
    };

    ContentObserver mDefaultApnObserver = new ContentObserver(mHandler) {
        @Override
        public void onChange(boolean selfChange) {
            mDefaultApn = getDefaultApn();
            if (DEBUG) Log.d(TAG, "Observer mDefaultApn=" + mDefaultApn);
        }
    };

    private void handleNetworkCallback(int state, int connType, Network network) {
        NetworkInfo info = mConnMgr.getNetworkInfo(network);

        if (DEBUG) {
            Log.d(TAG, "handleNetworkCallback connTyp: "+ connType + " state: " +state+
                  " network:" +network+ " info:" +info);
        }

        AGpsConnectionInfo agpsConnInfo=getAGpsConnectionInfo(connType);
        if(agpsConnInfo.mState == AGpsConnectionInfo.STATE_OPENING) {
            if(state == NETWORK_UNAVAILABLE) {
                if (DEBUG) Log.d(TAG, "call native_agps_data_conn_failed");
                agpsConnInfo.mAPN = null;
                agpsConnInfo.mState = AGpsConnectionInfo.STATE_CLOSED;
                native_agps_data_conn_failed(agpsConnInfo.mAgpsType);
            }
            else {
                if (DEBUG) Log.d(TAG, "call native_agps_data_conn_open");
                native_agps_data_conn_open(agpsConnInfo.getAgpsType(),
                                           agpsConnInfo.getApn(info),
                                           agpsConnInfo.getBearerType(info));
                agpsConnInfo.mState = AGpsConnectionInfo.STATE_OPEN;
            }
        }
        else if(DEBUG) {
            Log.d(TAG, "agpsConnInfo.mState:"+agpsConnInfo.mState);
        }
    }

    private void handleUpdateNetworkState(int state, NetworkInfo info) {
        //Default to ANY
        int connType = AGpsConnectionInfo.CONNECTION_TYPE_ANY;
        AGpsConnectionInfo agpsConnInfo=null;

        if (info != null) {
            switch (info.getType()) {
              case ConnectivityManager.TYPE_WIFI: {
                  connType = AGpsConnectionInfo.CONNECTION_TYPE_WIFI;
                  mWifiNetworkAvailable = (state == NETWORK_AVAILABLE);
                  break;
              }
              case ConnectivityManager.TYPE_MOBILE: {
                  connType = AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY;
                  mMobileNetworkAvailable = (state == NETWORK_AVAILABLE);
                  break;
              }
            }
            agpsConnInfo = getAGpsConnectionInfo(connType);

            if (DEBUG) {
                Log.d(TAG, "updateNetworkState connTyp: " + connType +
                      (mWifiNetworkAvailable || mMobileNetworkAvailable ? " available" : " unavailable")
                      + " info: " + info + " info.getType():"+info.getType());
            }
        }

        if (null != agpsConnInfo &&
            connType == AGpsConnectionInfo.CONNECTION_TYPE_WIFI) {
            if (mWifiState.state == mWifiState.WIFI_STATE_OPENING ||
                mWifiState.state == mWifiState.WIFI_STATE_CLOSING) {
                NetworkInfo.State networkState = info.getState();
                NetworkInfo.DetailedState detailedState = info.getDetailedState();
                if (DEBUG) {
                    Log.d(TAG, "handleUpdateNetworkState for TYPE_WIFI detailedstate = " + detailedState+
                          ", and state = "+networkState);
                }
                if (!info.isAvailable()) {
                    if (DEBUG) Log.e(TAG, "ERROR: handleUpdateNetworkState connect to wifi failed!!");
                    mWifiState.handleFailure();
                    return;
                }
                if (detailedState != NetworkInfo.DetailedState.CONNECTED) {
                    //note: disconnected means: IP traffic not available.
                    //kind of misleading. idle means disconnected but possible available
                    if (DEBUG) {
                        Log.d(TAG,
                              "handleUpdateNetworkState neither connected nor disconnected. return until it is ready");
                    }
                    return;
                }
                String ssid = (mWifiState.state == mWifiState.WIFI_STATE_OPENING) ? mWifiState.currentSSID : mWifiState.originalSSID;
                if (isWifiConnectedToSSID(info, ssid)) {
                    if (DEBUG) {
                        Log.d(TAG,
                              "handleUpdateNetworkState succeeded! wifi connected, and ssid matches expected!");
                    }
                    mWifiState.handleSuccess();
                } else {
                    Log.e(TAG, "isWifiConnectedToSSID returned false!");
                    mWifiState.handleFailure();
                }
            } else {
                if (DEBUG) Log.d(TAG, "ignore wifi update if we are not in OPENING or CLOSING");
            }
        }
        else {
            Log.e(TAG, "Either agpsConnInfo is NULL or connType is wrong. connType:" + connType + " ");
        }

        if (mWifiNetworkAvailable || mMobileNetworkAvailable) {
            if (mInjectNtpTimePending == STATE_PENDING_NETWORK) {
                sendMessage(INJECT_NTP_TIME, 0, null);
            }
            if (mDownloadXtraDataPending == STATE_PENDING_NETWORK) {
                sendMessage(DOWNLOAD_XTRA_DATA, 0, null);
            }
        }
    }

    private void handleInjectNtpTime() {
        if (mInjectNtpTimePending == STATE_DOWNLOADING) {
            // already downloading data
            return;
        }
        if (!mWifiNetworkAvailable && !mMobileNetworkAvailable) {
            // try again when network is up
            mInjectNtpTimePending = STATE_PENDING_NETWORK;
            return;
        }
        mInjectNtpTimePending = STATE_DOWNLOADING;

        // hold wake lock while task runs
        mWakeLock.acquire();
        AsyncTask.THREAD_POOL_EXECUTOR.execute(new Runnable() {
            @Override
            public void run() {
                long delay;

                // force refresh NTP cache when outdated
                if (mNtpTime.getCacheAge() >= NTP_INTERVAL) {
                    mNtpTime.forceRefresh();
                }

                // only update when NTP time is fresh
                if (mNtpTime.getCacheAge() < NTP_INTERVAL) {
                    long time = mNtpTime.getCachedNtpTime();
                    long timeReference = mNtpTime.getCachedNtpTimeReference();
                    long certainty = mNtpTime.getCacheCertainty();
                    long now = System.currentTimeMillis();

                    Log.d(TAG, "NTP server returned: "
                            + time + " (" + new Date(time)
                            + ") reference: " + timeReference
                            + " certainty: " + certainty
                            + " system time offset: " + (time - now));

                    native_inject_time(time, timeReference, (int) certainty);
                    delay = NTP_INTERVAL;
                } else {
                    if (DEBUG) Log.d(TAG, "requestTime failed");
                    if (mNTPRetryTimes == NTP_RETRY_TIMES_CAP) {
                        if (DEBUG)
                            Log.d(TAG, "NTP retries has hit the cap,"+
                                  " so no more retry in the next "+(NTP_INTERVAL/3600000)+
                                  " hours.");
                        mNTPRetryTimes = 0;
                        delay = NTP_INTERVAL;
                    } else {
                        mNTPRetryTimes++;
                        Random randomNum = new Random();
                        delay = (RETRY_INTERVAL_BASE + randomNum.nextInt(60)*1000)*mNTPRetryTimes;
                        if (DEBUG)
                            Log.d(TAG, "This is " + mNTPRetryTimes + "th NTP retry," +
                                  " which interval is " + delay);
                    }
                }

                sendMessage(INJECT_NTP_TIME_FINISHED, 0, null);

                if (mPeriodicTimeInjection) {
                    // send delayed message for next NTP injection
                    // since this is delayed and not urgent we do not hold a wake lock here
                    mHandler.sendEmptyMessageDelayed(INJECT_NTP_TIME, delay);
                }

                // release wake lock held by task
                mWakeLock.release();
            }
        });
    }

    private void handleDownloadXtraData() {
        if (mDownloadXtraDataPending == STATE_DOWNLOADING) {
            // already downloading data
            return;
        }
        if (!mWifiNetworkAvailable && !mMobileNetworkAvailable) {
            // try again when network is up
            mDownloadXtraDataPending = STATE_PENDING_NETWORK;
            return;
        }
        mDownloadXtraDataPending = STATE_DOWNLOADING;

        // hold wake lock while task runs
        mWakeLock.acquire();
        AsyncTask.THREAD_POOL_EXECUTOR.execute(new Runnable() {
            @Override
            public void run() {
                GpsXtraDownloader xtraDownloader = new GpsXtraDownloader(mContext, mProperties);
                byte[] data = xtraDownloader.downloadXtraData();
                if (data != null) {
                    if (DEBUG) {
                        Log.d(TAG, "calling native_inject_xtra_data");
                    }
                    native_inject_xtra_data(data, data.length);
                }

                sendMessage(DOWNLOAD_XTRA_DATA_FINISHED, 0, null);

                if (data == null) {
                    // try again later
                    // since this is delayed and not urgent we do not hold a wake lock here
                    long delay = 0;
                    if (mXtraDataRetryTimes == XTRA_DATA_RETRY_TIMES_CAP) {
                        if (DEBUG)
                            Log.d(TAG, "xtra data download retries has hit the cap,"+
                                  " so no more retry in the next "+(XTRA_DATA_INTERVAL/3600000)+
                                  " hours.");
                        mXtraDataRetryTimes = 0;
                        delay = XTRA_DATA_INTERVAL;
                    } else {
                        mXtraDataRetryTimes ++;
                        Random randomNum = new Random();
                        delay = (RETRY_INTERVAL_BASE +
                                 randomNum.nextInt(60)*1000)*mXtraDataRetryTimes;
                        if (DEBUG)
                            Log.d(TAG, "This is " + mXtraDataRetryTimes +
                                  "th xtra data download retry," +
                                  " which interval is " + delay);
                    }
                    mHandler.sendEmptyMessageDelayed(DOWNLOAD_XTRA_DATA, delay);
                }

                // release wake lock held by task
                mWakeLock.release();
            }
        });
    }

    /**
     * called from native code to request XTRA data
     */
    private void xtraDownloadRequest() {
        if (DEBUG) Log.d(TAG, "xtraDownloadRequest");
        sendMessage(DOWNLOAD_XTRA_DATA, 0, null);
    }

    public void enable() {
        sendMessage(ENABLE, 1, null);
    }

    private void handleEnable() {
        if (DEBUG) Log.d(TAG, "handleEnable");

        synchronized (mLock) {
            if (mEnabled) return;
            mEnabled = true;
        }

        boolean enabled = native_init();

        if (enabled) {
            mSupportsXtra = true;
        } else {
            synchronized (mLock) {
                mEnabled = false;
            }
            Log.w(TAG, "Failed to enable location provider");
        }
    }

    public void disable() {
        sendMessage(ENABLE, 0, null);
    }

    private void handleDisable() {
        if (DEBUG) Log.d(TAG, "handleDisable");

        synchronized (mLock) {
            if (!mEnabled) return;
            mEnabled = false;
        }

        // do this before releasing wakelock
        native_cleanup();
    }

    public boolean isEnabled() {
        synchronized (mLock) {
            return mEnabled;
        }
    }

    private boolean hasCapability(int capability) {
        return ((mEngineCapabilities & capability) != 0);
    }

    /**
     * called from native code to inform us what the GPS engine capabilities are
     */
    private void setEngineCapabilities(int capabilities) {
        mEngineCapabilities = capabilities;

        if (!hasCapability(GPS_CAPABILITY_ON_DEMAND_TIME) && !mPeriodicTimeInjection) {
            mPeriodicTimeInjection = true;
            requestUtcTime();
        }
    }

    private void associateToNetwork(String ssid, String password) {
        boolean b;
        Log.d(TAG, "associateToNetwork begin ssid ="+ssid+", password="+password);

        mWifiState.currentSSID = ssid;

        //0) save current state info so we can restore it on if release
        WifiInfo originalNetworkInfo = mWifiState.mWifiManager.getConnectionInfo();
        mWifiState.originalNetId = originalNetworkInfo.getNetworkId();
        mWifiState.originalSSID = originalNetworkInfo.getSSID();
        mWifiState.originalNetworkPreference = mConnMgr.getNetworkPreference();

        if (DEBUG) Log.d(TAG, "saved original wifi info. originalNetId  = " + mWifiState.originalNetId + ", originalNetworkPreference = "+ mWifiState.originalNetworkPreference );

        //2) set network pref to wifi
        mConnMgr.setNetworkPreference(ConnectivityManager.TYPE_WIFI);
        if (DEBUG) Log.d(TAG, "network prefence changed to wifi");
        if (DEBUG) Log.d(TAG, "get WPA wifi config for ssid="+ssid+", password="+password);
        WifiConfiguration wc;
        if (password == null) {
          wc = getWifiConfigurationForOpen(ssid);
        } else {
          wc = getWifiConfigurationForWPA(ssid, password);
        }
        if (DEBUG) Log.d(TAG, "wifi configuration is : "+wc);
        mWifiState.currentNetId = mWifiState.mWifiManager.addNetwork(wc);

        if (mWifiState.currentNetId < 0) {
          if (DEBUG) Log.e(TAG, "ERROR: "+ssid+" add Network returned " + mWifiState.currentNetId);
          mWifiState.handleFailure();
        } else {
          if (DEBUG) Log.d(TAG, ssid+" add Network returned " + mWifiState.currentNetId);

          //4) assoc to network
          b = mWifiState.mWifiManager.enableNetwork(mWifiState.currentNetId, true);
          //mWifiState.mWifiManager.reassociate();
          if (b) {
            if (DEBUG) Log.d(TAG, "enableNetwork returned " + b);
          } else {
            if (DEBUG) Log.e(TAG, "ERROR: enableNetwork returned " + b);
            mWifiState.handleFailure();
          }
        }
    }

    private void restoreOriginalNetworkPreference() {
      mConnMgr.setNetworkPreference(mWifiState.originalNetworkPreference);
      mWifiState.originalNetworkPreference = -1;
    }

    private int getCurrentNetId() {
        WifiInfo info = mWifiState.mWifiManager.getConnectionInfo();
        return info.getNetworkId();
    }

    /**
     * If the string does not have "", convert it to quoted string. For an
     * example, if the string is <abcdef>, the return string will be <"abcdef">.
     * If the string has "", return it.
     *
     * @param string
     *            To be converted to quoted string
     * @return string with quote
     */
    private String convertToQuotedString(String string) {
        if (TextUtils.isEmpty(string)) {
            return "";
        }

        final int lastPos = string.length() - 1;
        if (lastPos < 0 || (string.charAt(0) == '"' && string.charAt(lastPos) == '"')) {
            return string;
        }

        return "\"" + string + "\"";
    }

    private WifiConfiguration getWifiConfigurationForNoAuth(String SSID) {
        WifiConfiguration conf = new WifiConfiguration();
        conf.allowedAuthAlgorithms.clear();
        conf.allowedGroupCiphers.clear();
        conf.allowedKeyManagement.clear();
        conf.allowedPairwiseCiphers.clear();
        conf.allowedProtocols.clear();
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        conf.hiddenSSID = false;
        conf.SSID = convertToQuotedString(SSID);

        conf.status = WifiConfiguration.Status.ENABLED;

        return conf;
    }

    private WifiConfiguration getWifiConfigurationForOpen(String SSID)
    {
        WifiConfiguration conf = new WifiConfiguration();
        conf.allowedAuthAlgorithms.clear();
        conf.allowedGroupCiphers.clear();
        conf.allowedKeyManagement.clear();
        conf.allowedPairwiseCiphers.clear();
        conf.allowedProtocols.clear();
        conf.hiddenSSID = false;
        conf.SSID = convertToQuotedString(SSID);
        conf.priority = 1;

        conf.status = WifiConfiguration.Status.DISABLED;
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);

        return conf;
    }

    private WifiConfiguration getWifiConfigurationForWPA(String SSID, String password) {
        WifiConfiguration conf = new WifiConfiguration();
        conf.allowedAuthAlgorithms.clear();
        conf.allowedGroupCiphers.clear();
        conf.allowedKeyManagement.clear();
        conf.allowedPairwiseCiphers.clear();
        conf.allowedProtocols.clear();
        conf.hiddenSSID = false;
        conf.SSID = convertToQuotedString(SSID);
        conf.priority = 1;

      if (password.matches("[0-9A-Fa-f]{64}")) {
        Log.d(TAG, "A 64 bit hex password entered.");
        conf.preSharedKey = password;
      } else {
        Log.d(TAG, "A normal password entered: I am quoting it.");
        conf.preSharedKey = convertToQuotedString(password);
      }

      conf.status = WifiConfiguration.Status.DISABLED;
      //this mighth not be necessary:
        //conf.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.RSN);

        return conf;
    }

    private WifiConfiguration getWifiConfigurationForWEP(String SSID, String password) {
        WifiConfiguration conf = new WifiConfiguration();
        conf.allowedAuthAlgorithms.clear();
        conf.allowedGroupCiphers.clear();
        conf.allowedKeyManagement.clear();
        conf.allowedPairwiseCiphers.clear();
        conf.allowedProtocols.clear();
        conf.hiddenSSID = false;
        conf.SSID = convertToQuotedString(SSID);
        conf.priority = 40;

      conf.status = WifiConfiguration.Status.DISABLED;
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        conf.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
        conf.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
        conf.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.SHARED);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
        conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);

        conf.wepKeys[0] = convertToQuotedString(password);
        conf.wepTxKeyIndex = 0;

        return conf;
    }

    public boolean isWifiConnectedToSSID(NetworkInfo info, String ssid) {
        NetworkInfo.DetailedState networkState = info.getDetailedState();
        if (networkState == NetworkInfo.DetailedState.CONNECTED) {
              WifiInfo wifiInfo = mWifiState.mWifiManager.getConnectionInfo();
              if (DEBUG) Log.d(TAG, "wifiInfo  = " + wifiInfo);
              if ((ssid != null) && ssid.equals(wifiInfo.getSSID())) {
                if (DEBUG) Log.d(TAG, "wifi connected, and ssid matches expected!");
                return true;
              } else {
                if (DEBUG) Log.e(TAG, "ssid="+ssid+" doesn't match wifiInfo.getSSID()="+wifiInfo.getSSID());
                return false;
              }
          } else {
            if (DEBUG) Log.e(TAG, "not connected");
            return false;
          }
    }

    public boolean isWifiConnected (NetworkInfo info) {
        return  info.getDetailedState() == NetworkInfo.DetailedState.CONNECTED;
    }

    /**
     * called from native code to update AGPS status
     */
    private void reportAGpsStatus(int type,
                                  int status,
                                  byte[] ipAddr,
                                  String ssid,
                                  String password) {
        if (DEBUG) Log.d(TAG, "reportAGpsStatus with type = " + type +
                              " status = " + status +
                              " ipAddr = " + ipAddr +
                              " ssid = " + ssid +
                              " password = " + password);
        ReportAgpsStatusMessage rasm = new ReportAgpsStatusMessage(type,
                                                                   status,
                                                                   ipAddr,
                                                                   ssid,
                                                                   password);

        LocSvcMsgObj obj = new LocSvcMsgObj((Object)rasm, false);
        Message msg = new Message();
        msg.what = REPORT_AGPS_STATUS;
        msg.obj = obj;

        mHandler.sendMessage(msg);
    }

    private void handleReportAgpsStatus(ReportAgpsStatusMessage rasm) {
        int type = rasm.type;
        int status = rasm.status;
        byte[] ipAddr = rasm.ipAddr;
        String ssid = rasm.ssid;
        String password = rasm.password;

        if (DEBUG) Log.d(TAG, "handleReportAgpsStatus with type = " + type +
                              " status = " + status +
                              " ipAddr = " + ipAddr +
                              " ssid = " + ssid +
                              " password = " + password);

        // Convert the ssid to quoted string if it is not already.
        ssid = convertToQuotedString(ssid);

        AGpsConnectionInfo agpsConnInfo = getAGpsConnectionInfo(type);
        if (agpsConnInfo == null) {
            if (DEBUG) Log.d(TAG, "reportAGpsStatus agpsConnInfo is null for type "+type);
            // we do not handle this type of connection
            return;
        }

        switch (status) {
            case GPS_REQUEST_AGPS_DATA_CONN:
                if (DEBUG) Log.d(TAG, "GPS_REQUEST_AGPS_DATA_CONN");

                switch (type) {
                case AGpsConnectionInfo.CONNECTION_TYPE_SUPL:
                case AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY: {
                    NetworkInfo info = mConnMgr.getNetworkInfo(agpsConnInfo.getCMConnType());
                    if (DEBUG) Log.d(TAG, "type: "+type);
                    agpsConnInfo.connect(type);
                    break;
                }

                case AGpsConnectionInfo.CONNECTION_TYPE_WIFI: {
                    if (DEBUG) Log.d(TAG, "type == AGpsConnectionInfo.CONNECTION_TYPE_WIFI");
                    if (mWifiState.state != WifiState.WIFI_STATE_CLOSED) {
                        if (DEBUG) Log.e(TAG, "Error: request Wifi but WifiState is not WIFI_STATE_CLOSED");
                        native_agps_data_conn_failed(agpsConnInfo.getAgpsType());
                    }
                    if (mWifiState.mWifiManager.isWifiEnabled()) {
                        if (DEBUG) Log.d(TAG, "wifi enabled");
                        NetworkInfo info = mConnMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
                        if (isWifiConnectedToSSID(info, ssid)) {
                            if (DEBUG) Log.d(TAG, "already connected to this SSID. not associating to it...");
                            mWifiState.originalNetworkPreference = mConnMgr.getNetworkPreference();
                            if (mWifiState.originalNetworkPreference == ConnectivityManager.TYPE_WIFI) {
                                if (DEBUG) Log.d(TAG, "network Preference already TYPE_mWifiState.mWifiManager. do nothing");
                            } else {
                                if (DEBUG) Log.d(TAG, "network Preference not already TYPE_mWifiState.mWifiManager. change it.");
                                mConnMgr.setNetworkPreference(ConnectivityManager.TYPE_WIFI);
                            }
                            if (DEBUG) Log.d(TAG, "wifi connected, and ssid matches expected!");
                            mWifiState.originalNetId = getCurrentNetId();
                            mWifiState.currentNetId = mWifiState.originalNetId;
                            mWifiState.originalSSID = mWifiState.mWifiManager.getConnectionInfo().getSSID();

                            mWifiState.state = WifiState.WIFI_STATE_OPEN;
                            agpsConnInfo.mState = AGpsConnectionInfo.STATE_OPEN;

                            native_agps_data_conn_open(AGpsConnectionInfo.CONNECTION_TYPE_WIFI, "dummy-apn", AGpsConnectionInfo.BEARER_IPV4);
                        } else if (false == isWifiConnected(info)){
                            if (DEBUG) Log.d(TAG, "not connected to any SSID. associating to it...");
                            mWifiState.state = WifiState.WIFI_STATE_OPENING;
                            agpsConnInfo.mState = AGpsConnectionInfo.STATE_OPENING;
                            associateToNetwork(ssid, password);
                        } else {
                            if (DEBUG) Log.e (TAG, "Associated to a different ssid. Not associating...");
                            native_agps_data_conn_failed(agpsConnInfo.getAgpsType());
                        }
                    } else {
                        if (DEBUG) Log.e(TAG, "ERROR: wifi not enabled.. (we assume it is enabled)");
                        native_agps_data_conn_failed(agpsConnInfo.getAgpsType());
                    }
                    break;
                }

                default:
                    if (DEBUG) Log.e(TAG, "type == unknown");
                    break;
                }
                break;
            case GPS_RELEASE_AGPS_DATA_CONN: {
                if (DEBUG) Log.d(TAG, "GPS_RELEASE_AGPS_DATA_CONN");

                switch (type) {
                case AGpsConnectionInfo.CONNECTION_TYPE_SUPL:
                case AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY:
                {
                    agpsConnInfo.release(type);
                    agpsConnInfo.mState = AGpsConnectionInfo.STATE_CLOSED;
                    break;
                }
                case AGpsConnectionInfo.CONNECTION_TYPE_WIFI: {
                    if (DEBUG) Log.v(TAG, "case AGpsConnectionInfo.CONNECTION_TYPE_WIFI");
                    mWifiState.restoreOriginalWifiSettings(false);
                    return;
                }
                default:
                    if (DEBUG) Log.e(TAG, "GPS_RELEASE_AGPS_DATA_CONN but current network state is unknown!");
                    return;
                }
                break;
            }
            case GPS_AGPS_DATA_CONNECTED:
                if (DEBUG) Log.d(TAG, "GPS_AGPS_DATA_CONNECTED");
                break;
            case GPS_AGPS_DATA_CONN_DONE:
                if (DEBUG) Log.d(TAG, "GPS_AGPS_DATA_CONN_DONE");
                break;
            case GPS_AGPS_DATA_CONN_FAILED:
                if (DEBUG) Log.d(TAG, "GPS_AGPS_DATA_CONN_FAILED");
                break;
        }
    }

    //=============================================================
    // NI Client support
    //=============================================================
    private final INetInitiatedListener mNetInitiatedListener = new INetInitiatedListener.Stub() {
        // Sends a response for an NI reqeust to HAL.
        @Override
        public boolean sendNiResponse(int notificationId, int userResponse)
        {
            // TODO Add Permission check

            if (DEBUG) Log.d(TAG, "sendNiResponse, notifId: " + notificationId +
                    ", response: " + userResponse);
            native_send_ni_response(notificationId, userResponse);
            return true;
        }
    };

    // Called by JNI function to report an NI request.
    public void reportNiNotification(
            int notificationId,
            int niType,
            int notifyFlags,
            int timeout,
            int defaultResponse,
            String requestorId,
            String text,
            int requestorIdEncoding,
            int textEncoding,
            String extras  // Encoded extra data
        )
    {
        Log.i(TAG, "reportNiNotification: entered");
        Log.i(TAG, "notificationId: " + notificationId +
                ", niType: " + niType +
                ", notifyFlags: " + notifyFlags +
                ", timeout: " + timeout +
                ", defaultResponse: " + defaultResponse);

        Log.i(TAG, "requestorId: " + requestorId +
                ", text: " + text +
                ", requestorIdEncoding: " + requestorIdEncoding +
                ", textEncoding: " + textEncoding);

        GpsNiNotification notification = new GpsNiNotification();

        notification.notificationId = notificationId;
        notification.niType = niType;
        notification.needNotify = (notifyFlags & GpsNetInitiatedHandler.GPS_NI_NEED_NOTIFY) != 0;
        notification.needVerify = (notifyFlags & GpsNetInitiatedHandler.GPS_NI_NEED_VERIFY) != 0;
        notification.privacyOverride = (notifyFlags & GpsNetInitiatedHandler.GPS_NI_PRIVACY_OVERRIDE) != 0;
        notification.timeout = timeout;
        notification.defaultResponse = defaultResponse;
        notification.requestorId = requestorId;
        notification.text = text;
        notification.requestorIdEncoding = requestorIdEncoding;
        notification.textEncoding = textEncoding;

        // Process extras, assuming the format is
        // one of more lines of "key = value"
        Bundle bundle = new Bundle();

        if (extras == null) extras = "";
        Properties extraProp = new Properties();

        try {
            extraProp.load(new StringReader(extras));
        }
        catch (IOException e)
        {
            Log.e(TAG, "reportNiNotification cannot parse extras data: " + extras);
        }

        for (Entry<Object, Object> ent : extraProp.entrySet())
        {
            bundle.putString((String) ent.getKey(), (String) ent.getValue());
        }

        notification.extras = bundle;

        mNIHandler.handleNiNotification(notification);
    }

    /**
     * Called from native code to request utc time info
     */
    private void requestUtcTime() {
        sendMessage(INJECT_NTP_TIME, 0, null);
    }

    private void reportXtraServer(String server1, String server2, String server3) {

        Log.d(TAG, "reportXtraServer \n server1="+ server1 + "\n server2=" + server2 + "\n server3=" + server3);
        if(server1 == null) {
            mProperties.setProperty("XTRA_SERVER_1","");
        } else {
            mProperties.setProperty("XTRA_SERVER_1", server1);
        }
        if(server2 == null) {
            mProperties.setProperty("XTRA_SERVER_2","");
        } else {
            mProperties.setProperty("XTRA_SERVER_2", server2);
        }
        if(server3 == null) {
            mProperties.setProperty("XTRA_SERVER_3","");
        } else {
            mProperties.setProperty("XTRA_SERVER_3", server3);
        }
    }

    private String getDefaultApn() {
        Uri uri = Uri.parse("content://telephony/carriers/preferapn");
        String apn = null;

        try {
            Cursor cursor =
                mContext.getContentResolver().query(uri, new String[] {"apn"},
                                                    null, null, Carriers.DEFAULT_SORT_ORDER);
            if (null != cursor) {
                try {
                    if (cursor.moveToFirst()) {
                        apn = cursor.getString(0);
                    }
                } finally {
                    cursor.close();
                }
            }
        } catch (SQLiteException e) {
            Log.e(TAG, "SQLiteException on mContext.getContentResolver().query");
        } catch (Exception e) {
            Log.e(TAG, "Unknown exception"+e+" on mContext.getContentResolver().query");
        }

        if (apn == null) {
            apn = "dummy-apn";
        }

        return apn;
    }

    static { class_init_native(); }
    private static native void class_init_native();

    private native boolean native_init();
    private native void native_cleanup();

    // XTRA Support
    private native void native_inject_time(long time, long timeReference, int uncertainty);
    private native void native_inject_xtra_data(byte[] data, int length);
    private native int native_request_xtra_server();

    // AGPS Support
    private native void native_agps_data_conn_open(int agpsType, String apn, int bearerType);
    private native void native_agps_data_conn_closed(int agpsType);
    private native void native_agps_data_conn_failed(int agpsType);

    // Network-initiated (NI) Support
    private native void native_send_ni_response(int notificationId, int userResponse);

    // AGPS support
    private native void native_update_network_state(boolean connected, int type,
            boolean roaming, boolean available, String extraInfo, String defaultAPN);

    private static AGpsConnectionInfo[] mAGpsConnections = new AGpsConnectionInfo[3];
    private AGpsConnectionInfo getAGpsConnectionInfo(int connType) {
        if (DEBUG) Log.d(TAG, "getAGpsConnectionInfo connType - "+connType);
        switch (connType)
        {
        case AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY:
        case AGpsConnectionInfo.CONNECTION_TYPE_C2K:
            if (null == mAGpsConnections[0])
                mAGpsConnections[0] = new AGpsConnectionInfo(ConnectivityManager.TYPE_MOBILE, connType);
            return mAGpsConnections[0];
        case AGpsConnectionInfo.CONNECTION_TYPE_SUPL:
            if (null == mAGpsConnections[1])
                mAGpsConnections[1] = new AGpsConnectionInfo(ConnectivityManager.TYPE_MOBILE_SUPL, connType);
            return mAGpsConnections[1];
        case AGpsConnectionInfo.CONNECTION_TYPE_WIFI:
            if (null == mAGpsConnections[2])
                mAGpsConnections[2] = new AGpsConnectionInfo(ConnectivityManager.TYPE_WIFI, connType);
            return mAGpsConnections[2];
        default:
            return null;
        }
    }

    private final class LocNetworkCallback extends NetworkCallback {
        private int connType;

        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            if (DEBUG) Log.d(TAG, "OnAvailable for: "+ connType);
            sendMessage(HANDLE_NETWORK_CALLBACK, NETWORK_AVAILABLE, connType, network);
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            if (DEBUG) Log.d(TAG, "OnUnavailable for: "+ connType);
            sendMessage(HANDLE_NETWORK_CALLBACK, NETWORK_UNAVAILABLE, connType, null);
        }

        public int getConnType() {
            if (DEBUG) Log.d(TAG, "getConnType for: "+ connType);
            return connType;
        }

        public LocNetworkCallback(int type) {
            super();
            connType = type;
            if (DEBUG) Log.d(TAG, "New LocNetworkCallback for: "+ connType);
        }
    };

    private class AGpsConnectionInfo {
        // these need to match AGpsType enum in gps.h
        private static final int CONNECTION_TYPE_ANY = 0;
        private static final int CONNECTION_TYPE_SUPL = 1;
        private static final int CONNECTION_TYPE_C2K = 2;
        private static final int CONNECTION_TYPE_WWAN_ANY = 3;
        private static final int CONNECTION_TYPE_WIFI = 4;

        // this must match the definition of gps.h
        private static final int BEARER_INVALID = -1;
        private static final int BEARER_IPV4 = 0;
        private static final int BEARER_IPV6 = 1;
        private static final int BEARER_IPV4V6 = 2;

        // for mState
        private static final int STATE_CLOSED = 0;
        private static final int STATE_OPENING = 1;
        private static final int STATE_OPEN = 2;
        private static final int STATE_KEEP_OPEN = 3;

        // SUPL vs ANY (which really is non-SUPL)
        private final int mCMConnType;
        private final int mAgpsType;
        private final String mPHConnFeatureStr;
        private String mAPN;
        private int mIPvVerType;
        private int mState;
        private InetAddress mIpAddr;
        private int mBearerType;

        private AGpsConnectionInfo(int connMgrConnType, int agpsType) {
            mCMConnType = connMgrConnType;
            mAgpsType = agpsType;
            if (ConnectivityManager.TYPE_MOBILE_SUPL == connMgrConnType) {
                mPHConnFeatureStr = Phone.FEATURE_ENABLE_SUPL;
            } else {
                mPHConnFeatureStr = Phone.FEATURE_ENABLE_HIPRI;
            }
            mAPN = null;
            mState = STATE_CLOSED;
            mIpAddr = null;
            mBearerType = BEARER_INVALID;
        }
        private int getAgpsType() {
            return mAgpsType;
        }
        private int getCMConnType() {
            return mCMConnType;
        }
        private InetAddress getIpAddr() {
            return mIpAddr;
        }
        private String getApn(NetworkInfo info) {

            if (info != null) {
                mAPN = info.getExtraInfo();
            }
            if (mAPN == null) {
                /* We use the value we read out from the database. That value itself
                   is default to "dummy-apn" if no value from database. */
                mDefaultApn = getDefaultApn();
                mAPN = mDefaultApn;
            }

            return mAPN;
        }
        private int getBearerType(NetworkInfo info) {
            if (mAPN == null) {
                mAPN = getApn(info);
            }
            String ipProtocol = null;
            TelephonyManager phone = (TelephonyManager)
                    mContext.getSystemService(Context.TELEPHONY_SERVICE);

            // get IP protocol here
            int networkType = phone.getNetworkType();
            String selection = null;
            if (TelephonyManager.NETWORK_TYPE_EHRPD == networkType) {
                selection = "apn = '" + mAPN + "'";
                selection += " and type LIKE '%supl%'";
            } else {
                selection = "current = 1";
                selection += " and apn = '" + mAPN + "'";
                selection += " and carrier_enabled = 1";
            }

            try {
                Cursor cursor =
                    mContext.getContentResolver().query(Carriers.CONTENT_URI,
                                                        new String[] {Carriers.PROTOCOL},
                                                        selection, null,
                                                        Carriers.DEFAULT_SORT_ORDER);
                if (null != cursor) {
                    try {
                        if (cursor.moveToFirst()) {
                            ipProtocol = cursor.getString(0);
                        }
                    } finally {
                        cursor.close();
                    }
                }
            } catch (SQLiteException e) {
                Log.e(TAG, "SQLiteException on mContext.getContentResolver().query");
            } catch (Exception e) {
                Log.e(TAG, "Unknown exception"+e+" on mContext.getContentResolver().query");
            }
            Log.d(TAG, "ipProtocol: " + ipProtocol + " apn: " + mAPN +
                  " networkType: " + phone.getNetworkTypeName() + " state: " + mState);

            if (null == ipProtocol) {
                mBearerType = BEARER_IPV4;
            } else if (ipProtocol.equals("IPV6")) {
                mBearerType = BEARER_IPV6;
            } else if (ipProtocol.equals("IPV4V6")) {
                mBearerType = BEARER_IPV4V6;
            } else {
                mBearerType = BEARER_IPV4;
            }

            return mBearerType;
        }

        private void connect(int connType) {
            final ConnectivityManager connMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (DEBUG) Log.d(TAG, "connect() type: "+connType);

            if(connType == AGpsConnectionInfo.CONNECTION_TYPE_SUPL) {
                if(mSuplNetworkCallback == null) {
                    mSuplNetworkCallback = new LocNetworkCallback(connType);
                }
                connMgr.requestNetwork(mSuplNetworkRequest, mSuplNetworkCallback,
                                       NETWORK_REQUEST_TIMEOUT_MS);
                mState = AGpsConnectionInfo.STATE_OPENING;
            }
            else if (connType == AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY) {
                if(mWwanNetworkCallback == null) {
                    mWwanNetworkCallback = new LocNetworkCallback(connType);
                }
                connMgr.requestNetwork(mWwanNetworkRequest, mWwanNetworkCallback,
                                       NETWORK_REQUEST_TIMEOUT_MS);
                mState = AGpsConnectionInfo.STATE_OPENING;
            }
            else {
                Log.e(TAG, "connect() unknown type: "+connType);
            }
        }

        private void release(int connType) {
            final ConnectivityManager connMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (DEBUG) Log.d(TAG, "release() type: "+connType);
            if(connType == AGpsConnectionInfo.CONNECTION_TYPE_SUPL) {
                if(mSuplNetworkCallback != null) {
                    connMgr.unregisterNetworkCallback(mSuplNetworkCallback);
                    native_agps_data_conn_closed(AGpsConnectionInfo.CONNECTION_TYPE_SUPL);
                }
            }
            else if (connType == AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY) {
                if(mWwanNetworkCallback != null) {
                    connMgr.unregisterNetworkCallback(mWwanNetworkCallback);
                    native_agps_data_conn_closed(AGpsConnectionInfo.CONNECTION_TYPE_WWAN_ANY);
                }
            }
            else {
                Log.e(TAG, "release() unknown type: "+connType);
            }
        }
    }

    private class WifiState {
        private static final int WIFI_STATE_CLOSED = 0;
        private static final int WIFI_STATE_OPENING = 1;
        private static final int WIFI_STATE_OPEN = 2;
        private static final int WIFI_STATE_CLOSING = 3;

        private int state = WIFI_STATE_CLOSED;
        private String currentSSID = null;
        private int currentNetId = -1;
        private int originalNetId = -1;
        private String originalSSID = null;
        private int originalNetworkPreference = -1;

        private boolean reportFailOnClosed = false;

        private WifiManager mWifiManager = null;

        public WifiState() {
            mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        }

        private void restoreOriginalNetworkPreference() {
          mConnMgr.setNetworkPreference(originalNetworkPreference);
          originalNetworkPreference = -1;
        }

        /*
         * Restore Wifi and network settings to original
         * The restore is started here, and could completed here,
         * or by an asynchronous broadcast event saying network restore complete.
         */
        private void restoreOriginalWifiSettings(boolean ReportFailOnClosed) {
          if (DEBUG) Log.v(TAG, "restoreOriginalWifiSettings");

          if (DEBUG) Log.v(TAG, "originalNetId = "+ originalNetId );
          if (DEBUG) Log.v(TAG, "currentNetId = "+ currentNetId );
          if (DEBUG) Log.v(TAG, "reportFailOnClosed = "+ reportFailOnClosed);


          this.reportFailOnClosed = ReportFailOnClosed;
          boolean b;
          if (mConnMgr.getNetworkPreference() == originalNetworkPreference) {
              if (DEBUG) Log.v(TAG, "current network preference same as original. do nothing.");
          } else {
              if (DEBUG) Log.v(TAG, "restoring original Network Preference...");
              restoreOriginalNetworkPreference();
          }

          /*
          * Do not restore original network settings if original network connection was "none"
          * Side effect: you will be connected to something after it's oven even though originally
          * you were not connected to anything.
          */
          if ((currentNetId == originalNetId) ||
              (originalNetId == -1)) {
              if (DEBUG) Log.v(TAG, "currentNetId == originalNetId or original was not connected. don't touch netId.");

              state = WIFI_STATE_CLOSED;
              if (reportFailOnClosed) {
                  /*
                  * We added a network but it failed, let's remove it
                  */
                  if (currentNetId >= 0) {
                    if (DEBUG) Log.v(TAG, "removing currentNetId = " + currentNetId);
                    b = mWifiManager.removeNetwork(currentNetId);
                    if (b) {
                      if (DEBUG) Log.e(TAG, "successfully removed current AP");
                    } else {
                      if (DEBUG) Log.e(TAG, "ERROR: removeNetwork returned " + b);
                    }
                  }
                  native_agps_data_conn_failed(AGpsConnectionInfo.CONNECTION_TYPE_WIFI);
              } else {
                  native_agps_data_conn_closed(AGpsConnectionInfo.CONNECTION_TYPE_WIFI);
              }
          } else {
              state = WIFI_STATE_CLOSING;
              if (currentNetId >= 0) {
                /*
                if (DEBUG) Log.v(TAG, "There is a current NetId. disconnecting...");
                b = mWifiManager.disconnect();
                if (b) {
                  if (DEBUG) Log.e(TAG, "successfully disconnected from current AP");
                } else {
                  if (DEBUG) Log.e(TAG, "ERROR: disconnect returned " + b);
                }
                */

                b = mWifiManager.removeNetwork(currentNetId);
                if (b) {
                  if (DEBUG) Log.e(TAG, "successfully removed current AP");
                } else {
                  if (DEBUG) Log.e(TAG, "ERROR: removeNetwork returned " + b);
                }
              }

              if (DEBUG) Log.v(TAG, "restoring original network...");
              b = mWifiManager.enableNetwork(originalNetId, true);
              if (b) {
                if (DEBUG) Log.d(TAG, "enableNetwork returned " + b);
              } else {
                if (DEBUG) Log.e(TAG, "ERROR: enableNetwork returned " + b);
              }
          }
          if (DEBUG) Log.v(TAG, "restoreOriginalWifiSettings end");
        }


        /*
        * Waiting is now done, state has finished changing
        * to be what we were waiting for
        */
        private void handleSuccess() {
          if (state == WIFI_STATE_OPENING) {
            if (DEBUG) Log.v(TAG, "handleSuccess for WIFI_STATE_OPENING");
            native_agps_data_conn_open(AGpsConnectionInfo.CONNECTION_TYPE_WIFI, "dummy-apn", AGpsConnectionInfo.BEARER_IPV4);

            state = WIFI_STATE_OPEN;
            getAGpsConnectionInfo(AGpsConnectionInfo.CONNECTION_TYPE_WIFI).mState = AGpsConnectionInfo.STATE_OPEN;
          } else if (state == WIFI_STATE_CLOSING) {
            if (DEBUG) Log.v(TAG, "handleSuccess for WIFI_STATE_CLOSING");
            if (reportFailOnClosed) {
              reportFailOnClosed = false;
              native_agps_data_conn_failed(AGpsConnectionInfo.CONNECTION_TYPE_WIFI);
            } else {
              native_agps_data_conn_closed(AGpsConnectionInfo.CONNECTION_TYPE_WIFI);
            }
            state = WIFI_STATE_CLOSED;
            getAGpsConnectionInfo(AGpsConnectionInfo.CONNECTION_TYPE_WIFI).mState = AGpsConnectionInfo.STATE_CLOSED;
            currentNetId = -1;
            currentSSID = null;
            originalNetId = -1;
            originalSSID = null;
            originalNetworkPreference = -1;
          } else {
            if (DEBUG) Log.e(TAG, "handleSuccess invalid case");
          }
        }

        /*
        * We have noticed at this point that we can no longer succeed
        * in whatever we were waiting to do.
        */
        private void handleFailure() {
          if (state == WIFI_STATE_OPENING) {
            if (DEBUG) Log.v(TAG, "handleFailure for WIFI_STATE_OPENING");
            restoreOriginalWifiSettings(true);
          } else if (state == WIFI_STATE_CLOSING) {
            if (DEBUG) Log.v(TAG, "handleFailure for WIFI_STATE_CLOSING");
            state = WIFI_STATE_CLOSED;
            native_agps_data_conn_failed(AGpsConnectionInfo.CONNECTION_TYPE_WIFI);
          } else {
            if (DEBUG) Log.e(TAG, "handleFailure invalid case");
          }
        }
    }

    private class ReportAgpsStatusMessage {
      int type;
      int status;
      byte[] ipAddr;
      String ssid;
      String password;

      public ReportAgpsStatusMessage(int type,
                                     int status,
                                     byte[] ipAddr,
                                     String ssid,
                                     String password) {
        this.type = type;
        this.status = status;
        this.ipAddr = ipAddr;
        this.ssid = ssid;
        this.password = password;
      }
    }
}
