/******************************************************************************
 * @file    EmbmsSntpClient.java
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.embms;

import java.nio.ByteBuffer;
import java.util.Calendar;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.net.SntpClient;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;


public class EmbmsSntpClient {
    private static final String LOG_TAG = "EmbmsSntpClient";
    private static final boolean LOGD = true;
    private static final int SUCCESS = 0;
    private static final int FAILURE = -1;
    private static final int EVENT_GET_SNTP_TIME = 1;

    private static final String ACTION_SET_ALARM
            = "com.qualcomm.embms.EmbmsSntpClient.action.RING_ALARM";
    BroadcastReceiver mBroadcastReceiver;
    private static Handler mTarget;
    private static int mWhat;

    // Holds the singleton instance
    private static EmbmsSntpClient sSingleton;

    private Handler mHandler;
    private SntpClient mClient;
    private final AlarmManager mAlarmManager;
    private final Context mContext;
    private final PendingIntent mPendingIntent;

    // Time computed from the SNTP server response.
    private long mSntpTime;

    // Time at which the response from SNTP server was received
    private long mTimeStamp;

    // SNTP success
    private boolean mSuccess;

    private final String mServer;
    private final long mPeriodicity;
    private final long mTimeout;

    // How often to request SNTP time, in milliseconds
    // default time 24 hours
    public long SNTP_TIME_RETRIEVAL_PERIODICITY_MS = 24 * 60 * 60 * 1000;

    // How often to request SNTP time incase of sntp time retrieval failure
    // default time 1 hour
    private static final long SNTP_RETRY_INTERVAL_MS = 1 * 60 * 60 * 1000;

    private EmbmsSntpClient(String server, long timeout, Context context) {
        mServer = server;
        mTimeout = timeout;
        mContext = context;
        mClient = new SntpClient();
        mPeriodicity = SystemProperties.getLong("persist.embms.sntp.periodicity",
                SNTP_TIME_RETRIEVAL_PERIODICITY_MS);
        mAlarmManager = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        mPendingIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(ACTION_SET_ALARM), 0);
        initializeHandler();
        registerForAlarms();
        if (LOGD) {
            Log.i(LOG_TAG, "creating EmbmsSntpClient using server = " + mServer + "periodicity = "
                    + mPeriodicity);
        }
    }

    public static synchronized EmbmsSntpClient getInstance(Context context) {
        if (sSingleton == null) {
            String server;
            String sntpServer = SystemProperties.get("persist.embms.sntp.server");
            final Resources res = context.getResources();
            final ContentResolver resolver = context.getContentResolver();

            final String defaultServer = res
                    .getString(com.android.internal.R.string.config_ntpServer);
            final long defaultTimeout = res
                    .getInteger(com.android.internal.R.integer.config_ntpTimeout);

            final String secureServer = Settings.Global.getString(resolver,
                    Settings.Global.NTP_SERVER);
            final long timeout = Settings.Global.getLong(resolver, Settings.Global.NTP_TIMEOUT,
                    defaultTimeout);

            if (sntpServer == null || sntpServer.length() == 0) {
                Log.d(LOG_TAG, "Invalid Sntp server ; Using the default Sntp Server");
                server = secureServer != null ? secureServer : defaultServer;
            } else {
                server = sntpServer;
            }
            sSingleton = new EmbmsSntpClient(server, timeout, context);
        }

        return sSingleton;
    }

    public void dispose () {
        sSingleton = null;
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    public void initializeHandler() {
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                if (LOGD) Log.i(LOG_TAG, "handleMessage received message : " + msg.what);
                switch (msg.what) {
                    case EVENT_GET_SNTP_TIME:
                        requestSntpTime();
                        break;
                    default:
                        Log.e(LOG_TAG, "Unexpected message received what = " + msg.what);
                        break;
                }
            }
        };
    }

    public class SntpResponse {
        public int resp_sntpSuccess;
        public long resp_sntpTime;
        public long resp_timeStamp;

        public SntpResponse(int success, long time, long timeStamp) {
            this.resp_sntpSuccess = success;
            this.resp_sntpTime = time;
            this.resp_timeStamp = timeStamp;
        }
    }

    /**
     * Returns the server name that is used for retrieving SNTP time.
     */
    public String getServer() {
        return mServer;
    }

    /**
     * Returns the periodicity in milli-seconds that is used to control SNTP
     * sync up with the server.
     */
    public long getPeriodicity() {
        return mPeriodicity;
    }

    /**
     * Returns the time computed from the SNTP transaction.
     *
     * @return time value computed from the SNTP server response
     */
    private long getSntpTime() {
        return mSntpTime;
    }

    private long getTimeStamp() {
        return mTimeStamp;
    }

    private boolean getSntpSuccess() {
        return mSuccess;
    }

    private void registerForAlarms() {
        if(LOGD) Log.i(LOG_TAG, "registerforAlarms");
        mBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mHandler.obtainMessage(EVENT_GET_SNTP_TIME).sendToTarget();
            }
        };
        mContext.registerReceiver(mBroadcastReceiver, new IntentFilter(ACTION_SET_ALARM));
    }

    /**
     * Invokes the SntpClient.java to retrieve the SNTP time from mServer.
     */
    private void requestSntpTime() {
        if (LOGD) Log.i(LOG_TAG, "requestSNTPTime called ");
        final String server = this.mServer;
        final long timeout = this.mTimeout;
        mTimeStamp = SystemClock.elapsedRealtime();

        final AsyncTask<Void,Void,Void> getTimeTask = new AsyncTask<Void, Void, Void>(){

            protected Void doInBackground(Void... aVoid) {
                    if (mClient.requestTime(server, (int) timeout)) {
                        mSntpTime = mClient.getNtpTime();
                        if (LOGD) {
                        Log.i(LOG_TAG, "SntpClient: requestTime() = true"
                                 + "getNtpTime() = " + mSntpTime
                                 + "time stamp =" + mTimeStamp);
                        }
                        mSuccess = true;
                    } else {
                        if (LOGD) Log.i(LOG_TAG, "SntpClient: requestTime() = false");
                        mSuccess = false;
                    }
            return null;
            }

            protected void onPostExecute(Void aVoid){
                sntpTimeResponse();
            }
        };
        getTimeTask.execute();
    }

    /**
     * sends response to the EmbmsService
     */
    private void sntpTimeResponse() {
        if(LOGD) Log.i(LOG_TAG, "sntpTimeResponse called");
        boolean success = getSntpSuccess();
        long time = getSntpTime();
        long timeStamp = getTimeStamp();
        // Create the response msg
        Message msg = Message.obtain(mTarget, mWhat);
        if (LOGD) {
            Log.i(LOG_TAG, "Sntp Client ret: success = " + success + " sntp time in ms = " + time
                    + " sntpTimeStamp = " + timeStamp);
        }
        if (success) {
            msg.obj = new SntpResponse(SUCCESS, time, timeStamp);
            resetAlarm(mPeriodicity);
        } else {
            msg.obj = new SntpResponse(FAILURE, time, timeStamp);
            resetAlarm(SNTP_RETRY_INTERVAL_MS);
        }

        msg.sendToTarget();
    }

    /**
     * Cancel old alarm and start new one for the specified interval
     *
     * @param interval when to trigger the alarm starting from now
     */
    private void resetAlarm(long interval) {
        Log.i(LOG_TAG, "resetAlarm with interval = " + interval);
        mAlarmManager.cancel(mPendingIntent);
        long now = SystemClock.elapsedRealtime();
        long next = now + interval;
        mAlarmManager.set(AlarmManager.ELAPSED_REALTIME, next, mPendingIntent);
    }

    public void stopTimeReporting() {
        if(LOGD) Log.i(LOG_TAG, "stopTimeReporting called");
        mAlarmManager.cancel(mPendingIntent);
    }

    public void startTimeReporting(Handler handler, int what) {
        if(LOGD) Log.i(LOG_TAG, "startTimeReporting called");
        mTarget = handler;
        mWhat = what;
        if(LOGD) Log.i(LOG_TAG, "target = " + mTarget + "what = " + mWhat);
        requestSntpTime();
    }
}
