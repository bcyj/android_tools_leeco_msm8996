/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.pvd;

import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.util.Log;
import android.os.Looper;
import android.os.AsyncTask;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.NoSuchElementException;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.os.SystemProperties;
import android.provider.Settings;

import com.qualcomm.location.LBSSystemMonitorService;
import com.qualcomm.location.LocationService;

import com.qualcomm.location.pvd.IPVDService;
import com.qualcomm.location.pvd.IPVDResponseListener;
import com.qualcomm.location.pvd.PVDStatus;

import com.qualcomm.location.ulp.UlpEngine;
import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;
import android.content.ContentResolver;
import android.provider.Settings;

/**
 * @hide
 */
public class PVDService extends Service {
    private static final String TAG = "PVDService";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);
    private static final String PVD_SERVICE_NAME = "com.qualcomm.location.pvd.PVDService";
    private static final String IZAT_CONF = "/etc/izat.conf";
    private static final int DEFAULT_PVD_QUERY_TIMEOUT_BEFORE_DETECTION_MS = 120000;
    private static final int DEFAULT_PVD_QUERY_TIMEOUT_AFTER_DETECTION_MS = 60000;
    private static final int DEFAULT_CACHE_STATE_VALIDITY_TIME_MS = 60000;
    private static final int DEFAULT_LAST_BREACH_POS_VALIDITY_TIME_MS = 15000;
    private static final String PVD_QUERY_TIMER_BEFORE_DETECTION = "PVD_QUERY_TIMER_BEFORE_DETECTION";
    private static final String PVD_QUERY_TIMER_AFTER_DETECTION = "PVD_QUERY_TIMER_AFTER_DETECTION";
    private static final String PVD_CACHE_STATE_TIMEOUT = "PVD_CACHE_STATE_TIMEOUT";
    private static final String PVD_BREACH_POS_TIMEOUT = "PVD_BREACH_POS_TIMEOUT";
    private static final String ENH_LOCATION_SERVICES_ENABLED = "enhLocationServices_on";
    private static final String ULP_PVD_QUERY = "pvd_query";

    // Handler Messages
    private static final int REQUEST_PVD_REGISTER = 1;
    private static final int REQUEST_PVD_DEREGISTER = 2;
    private static final int REQUEST_PVD_QUERY = 3;
    private static final int HANDLE_PVD_STATUS = 4;
    private static final int PVD_REQUEST_NETWORK_LOCATION = 5;
    private static final int PVD_UPDATE_NETWORK_LOCATION = 6;

    // Network Provider Location Errors
    private static final int NLP_NO_ERROR = 0;
    private static final int NLP_ERROR = 1;

    // PVD Status - Should match the ones defined in ulp_service.h
    public static final int ULP_PVD_STATUS_UNKNOWN = 0;
    public static final int ULP_PVD_STATUS_DETECTED = 1;
    public static final int ULP_PVD_STATUS_UNDETECTED = 2;
    public static final int ULP_PVD_STATUS_BUSY = 3;
    public static final int ULP_PVD_STATUS_GENERAL_FAILURE = 4;

    private static int mPVDQueryTimer = DEFAULT_PVD_QUERY_TIMEOUT_BEFORE_DETECTION_MS;
    private static int mPVDQueryTimerBeforeDetection = DEFAULT_PVD_QUERY_TIMEOUT_BEFORE_DETECTION_MS;
    private static int mPVDQueryTimerAfterDetection = DEFAULT_PVD_QUERY_TIMEOUT_AFTER_DETECTION_MS;
    private static int mPVDCacheStateTimeout = DEFAULT_CACHE_STATE_VALIDITY_TIME_MS;
    private static int mPVDBreachPosTimeout = DEFAULT_LAST_BREACH_POS_VALIDITY_TIME_MS;

    private boolean mOngoingQuery = false;
    // Timer creates a separate thread so we want to create it only if there is a listener
    private Timer mTimer = null;
    // Handler
    private static PVDHandler mHandler = null;
    // data structure for keeping <IPVDResponseListener, ListenerDeathRecipient> tuples
    private HashMap<IPVDResponseListener, ListenerDeathRecipient> mListeners
        = new HashMap<IPVDResponseListener, ListenerDeathRecipient>();

    private PVDStatus mCurrentPVDStatus = PVDStatus.PVD_STATUS_UNKNOWN;
    private String mCurrentPVDVenueInfo = null;
    private long mCachedTimeMs = 0;
    private static LocationManager mLocMgr = null;
    private BreachPosition mLastBreachPosition = null;
    private long mLastBreachPositionTimeMs = 0;


    // Class to keep the BreachPosition
    public static class BreachPosition {
        private double latitude = 0.0;
        private double longitude = 0.0;
        private float  accuracy = 0.0f;
        public BreachPosition (double lat, double lon, float radius) {
            latitude = lat;
            longitude = lon;
            accuracy = radius;
        }
    };

    // Class to send the registration info to handler
    private class PVDRegistrationInfo {
        public IPVDResponseListener listener;
        public BreachPosition position;
    };

    private static String getConfigVal(String confFile, String name) {
        String val = null;

        try {
            File file = new File(confFile);
            FileInputStream stream = new FileInputStream(file);
            Properties properties = new Properties();
            properties.load(stream);
            stream.close();

            val = properties.getProperty(name);
            if (VERBOSE_DBG) Log.v(TAG, name+" - "+val);
        } catch (Exception e) {
            Log.w(TAG, "Could not open the configuration file ");
        }

        return val;
    }

    // Get the PVD Query timer value from conf file
    static {
        String val = getConfigVal(IZAT_CONF, PVD_QUERY_TIMER_BEFORE_DETECTION);
        if (null != val) {
            try {
                mPVDQueryTimerBeforeDetection =
                    Integer.valueOf(val.trim()).intValue();
            Log.v (TAG, "PVD_QUERY_TIMER_BEFORE_DETECTION = "
                    + mPVDQueryTimerBeforeDetection);
            } catch (NullPointerException e) {
                e.printStackTrace();
            }
        } else {
            if (VERBOSE_DBG) Log.v (TAG, "PVD timer before detection not defined in conf use default");
            mPVDQueryTimerBeforeDetection = DEFAULT_PVD_QUERY_TIMEOUT_BEFORE_DETECTION_MS;
        }
        // Init the timer with the PVD undetected timer value
        mPVDQueryTimer = mPVDQueryTimerBeforeDetection;

        val = getConfigVal(IZAT_CONF, PVD_QUERY_TIMER_AFTER_DETECTION);
        if (null != val) {
            try {
                mPVDQueryTimerAfterDetection =
                    Integer.valueOf(val.trim()).intValue();
            Log.v (TAG, "PVD_QUERY_TIMER_AFTER_DETECTION = "
                    + mPVDQueryTimerAfterDetection);
            } catch (NullPointerException e) {
                e.printStackTrace();
            }
        } else {
            if (VERBOSE_DBG) Log.v (TAG, "PVD timer after detection not defined in conf use default");
            mPVDQueryTimerAfterDetection = DEFAULT_PVD_QUERY_TIMEOUT_AFTER_DETECTION_MS;
        }

        val = getConfigVal(IZAT_CONF, PVD_CACHE_STATE_TIMEOUT);
        if (null != val) {
            try {
                mPVDCacheStateTimeout =
                    Integer.valueOf(val.trim()).intValue();
            Log.v (TAG, "PVD_CACHE_STATE_TIMEOUT = "
                    + mPVDCacheStateTimeout);
            } catch (NullPointerException e) {
                e.printStackTrace();
            }
        } else {
            if (VERBOSE_DBG) Log.v (TAG, "PVD cache state validity not defined in conf use default");
            mPVDCacheStateTimeout = DEFAULT_CACHE_STATE_VALIDITY_TIME_MS;
        }

        val = getConfigVal(IZAT_CONF, PVD_BREACH_POS_TIMEOUT);
        if (null != val) {
            try {
                mPVDBreachPosTimeout =
                    Integer.valueOf(val.trim()).intValue();
            Log.v (TAG, "PVD_BREACH_POS_TIMEOUT = "
                    + mPVDBreachPosTimeout);
            } catch (NullPointerException e) {
                e.printStackTrace();
            }
        } else {
            if (VERBOSE_DBG) Log.v (TAG, "PVD breach pos timeout not defined in conf use default");
            mPVDBreachPosTimeout = DEFAULT_LAST_BREACH_POS_VALIDITY_TIME_MS;
        }
    }

    // Handler to handle messages from the clients and UlpEngine
    private final class PVDHandler extends Handler {
        public PVDHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int message = msg.what;
            if (VERBOSE_DBG) Log.v(TAG, "handleMessage what - " + message);
            switch (message) {
                case REQUEST_PVD_REGISTER:
                    handlePVDRegister ((PVDRegistrationInfo) msg.obj);
                    break;
                case REQUEST_PVD_DEREGISTER:
                    handlePVDDeregister ((IPVDResponseListener) msg.obj);
                    break;
                case REQUEST_PVD_QUERY:
                    handlePVDQuery ((BreachPosition)msg.obj);
                    break;
                case HANDLE_PVD_STATUS:
                    handleReportPVDStatus(msg.arg1, (String)msg.obj);
                    break;
                case PVD_REQUEST_NETWORK_LOCATION:
                    handleNetworkLocationRequest ();
                    break;
                case PVD_UPDATE_NETWORK_LOCATION:
                    handleNetworkLocationUpdate((Location) msg.obj);
                    break;
                default:
                    break;
            }
        }
    };


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (VERBOSE_DBG) Log.v(TAG, "onStartCommand Event");

        // register service on entry
        if (ServiceManager.getService(PVD_SERVICE_NAME) == null) {
            Log.d(TAG, "start PVD service");

            ServiceManager.addService(PVD_SERVICE_NAME, mBinder.asBinder());
            mHandler = new PVDHandler (Looper.getMainLooper());
        }

        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        if (VERBOSE_DBG) Log.v(TAG, "onDestroy Event");

        // Stop the timer if still running
        if (mTimer != null) {
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // ignore
        return false;
    }

    private final IPVDService.Stub mBinder = new IPVDService.Stub() {
        @Override
        public boolean registerPVDListener(IPVDResponseListener listener, boolean valid_pos,
                double lat, double lon, float radius) {

            boolean retVal = false;
            if (VERBOSE_DBG) Log.v(TAG, "registerPVDListener");

            if (listener == null || listener.asBinder() == null) {
                if (VERBOSE_DBG) Log.v(TAG, "registerPVDListener - Invalid listener");
                return retVal;
            }

            PVDRegistrationInfo info = new PVDRegistrationInfo ();
            info.listener = listener;
            info.position = null;
            if (true == valid_pos) {
                info.position = new BreachPosition (lat, lon, radius);
            }

            sendMessage (REQUEST_PVD_REGISTER, 0, 0, info);
            retVal = true;
            return retVal;
        }

        @Override
        public boolean unregisterPVDListener(IPVDResponseListener listener) {

            boolean retVal = false;
            if (VERBOSE_DBG) Log.v(TAG, "unregisterPVDListener");

            if (listener == null || listener.asBinder() == null) {
                if (VERBOSE_DBG) Log.v(TAG, "unregisterPVDListener - inlalid listener");
                return retVal;
            }

            sendMessage (REQUEST_PVD_DEREGISTER, 0, 0, listener);
            retVal = true;
            return retVal;
        }

    };

    private IPVDResponseListener findOrigListener(IPVDResponseListener listener) {
        IBinder binder = listener.asBinder();

        for (IPVDResponseListener l : mListeners.keySet()) {
            if (l.asBinder() == binder) {
                return l;
            }
        }

        return null;
    }

    /**
     * Handles the registration request in Handler context
     * @param info Registration Info
     */
    void handlePVDRegister (PVDRegistrationInfo info) {
        // Put the listener data in the Map
        if (null != info.position) {
            mLastBreachPosition = info.position;
            mLastBreachPositionTimeMs = System.currentTimeMillis();
        }

        // Check if the listener is already registered
        IPVDResponseListener origListener = findOrigListener(info.listener);
        if (null != origListener) {
            if (VERBOSE_DBG) Log.v(TAG, "handlePVDRegister - already registered listener");
            return;
        }

        ListenerDeathRecipient death = new ListenerDeathRecipient(info.listener);
        try {
            info.listener.asBinder().linkToDeath(death, 0);
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        mListeners.put(info.listener, death);

        // Check the number of listeners
        if (mListeners.size() == 1) {
            if (VERBOSE_DBG) Log.v(TAG, "handlePVDRegister - first listener");
            // This is the first listener.
            // Create the timer
            mTimer = new Timer ();
            // Schedule the timer now.
            mTimer.schedule(new TimeoutTask(), mPVDQueryTimer, mPVDQueryTimer);

            // Send PVDQuery
            sendPVDQuery (mLastBreachPosition);
        } else {
            if (VERBOSE_DBG) Log.v(TAG, "handlePVDRegister - not first listener");
            // More than one listener. if there is ongoing query do nothing
            if (mOngoingQuery == false) {
                if (VERBOSE_DBG) Log.v(TAG, "handlePVDRegister - not first listener, no ongoing query");
                // No ongoing query. Check if the cached status can be used
                if (System.currentTimeMillis() - mPVDCacheStateTimeout > mCachedTimeMs) {
                    if (VERBOSE_DBG) Log.v(TAG, "registerPVDListener - not first listener, cache invalid");
                    // Cache is not valid any more. Trigger the query
                    sendPVDQuery (mLastBreachPosition);
                    // reset the timer so it kicks in mPVDQueryTimer later
                    // from this current request.
                    mTimer.cancel();
                    mTimer = new Timer ();
                    mTimer.schedule(new TimeoutTask(), mPVDQueryTimer, mPVDQueryTimer);
                } else {
                    // Cache is valid. Report the cached status
                    // Report to all listeners
                    ReportPVDStatusTask task = new ReportPVDStatusTask(
                            mCurrentPVDStatus, mCurrentPVDVenueInfo, info.listener, false);
                    task.execute();
                }
            }
        }
    }

    /**
     * Deregisters the listener in Handler context
     * @param listener
     */
    private void handlePVDDeregister (IPVDResponseListener listener) {
        IPVDResponseListener origListener = findOrigListener(listener);
        if (origListener != null) {
            ListenerDeathRecipient death = mListeners.get(origListener);
            if (death != null) {
                try {
                    origListener.asBinder().unlinkToDeath(death, 0);
                } catch (NoSuchElementException e) {
                    e.printStackTrace();
                }
            }
            if (VERBOSE_DBG) Log.v(TAG, "remove listener " + origListener);
            mListeners.remove(origListener);
        }

        if (mListeners.size() == 0) {
            onAllListenersGone();
        }
    }

    /**
     * Receipient to the death for any of the registered listener.
     * If a listener has died then this class helps to remove the listener
     * from our listener list.
     */
    private class ListenerDeathRecipient implements IBinder.DeathRecipient {
        IPVDResponseListener mListener;

        ListenerDeathRecipient(IPVDResponseListener listener) {
            mListener = listener;
        }

        @Override
        public void binderDied() {
            if (VERBOSE_DBG) Log.v(TAG, "binderDided(" + mListener + ")");

            if (mListeners.containsKey(mListener)) {
                if (VERBOSE_DBG) Log.v(TAG, "binderDided(" + mListener + "). removed it");
                mListeners.remove(mListener);
            }

            if (mListeners.size() == 0) {
                onAllListenersGone();
            }
        }
    }

    /**
     * Cleans up after all listeners are gone.
     */
    private void onAllListenersGone() {
        synchronized (this) {
            if (VERBOSE_DBG) Log.v(TAG, "all listeners are gone");

            // Stop the timer
            mTimer.cancel();
            mTimer.purge();
            // Stop timer thread
            mTimer = null;

            mPVDQueryTimer = mPVDQueryTimerBeforeDetection;

            // No more ongoing query
            mOngoingQuery = false;
        }
    }

    /**
     * Class that extends the AsyncTask to report the PVD Status to
     * listeners.
     */
    private class ReportPVDStatusTask extends AsyncTask<Void, Void, Void> {
        private PVDStatus mStatus;
        private String mJson;
        private IPVDResponseListener mListener;
        private boolean mRemoveListener;

        /**
         * Constructor
         * @param status of PVD
         * @param json JSON string
         * @param listener send data to given listener; if null send to all listeners
         * @param removeListener remove listener / listeners after sending status
         */
        public ReportPVDStatusTask(PVDStatus status, String json,
                IPVDResponseListener listener, boolean removeListener) {
            if (VERBOSE_DBG) {
                Log.v(TAG, "create ReportPVDStatusTask status=" + status + " json="
                        + json + " listener=" + listener + " removeListener=" +
                        removeListener);
            }
            mStatus = status;
            mJson = json;
            mListener = listener;
            mRemoveListener = removeListener;
        }

        @Override
        protected Void doInBackground(Void... params) {
            if (mListener != null) {
                doReportPVDStatusToSingleListener();
            } else {
                doReportPVDStatusToAllListeners();
            }

            return null;
        }

        /**
         * Reports the PVD Status to single listener
         */
        private void doReportPVDStatusToSingleListener() {
            if (VERBOSE_DBG) Log.v(TAG, "doReportPVDStatusToSingleListener");
            try {
                mListener.onPipVenueDetectionStatus(mStatus, mJson);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            if (mRemoveListener) {
                if (VERBOSE_DBG) Log.v(TAG, "doReportPVDStatusToSingleListener - remove listener");
                mListeners.remove(mListener);
            }
        }

        /**
         * Reports the PVD status to all listeners
         */
        private void doReportPVDStatusToAllListeners() {
           if (VERBOSE_DBG) Log.v(TAG, "doReportPVDStatusToAllListeners");

           for (IPVDResponseListener listener : mListeners.keySet()) {
               if (VERBOSE_DBG) Log.v(TAG, "doReportPVDStatusToAllListeners: notify " + listener);
               try {
                    listener.onPipVenueDetectionStatus(mStatus, mJson);
               } catch (RemoteException e) {
                    if (VERBOSE_DBG) {
                        Log.v(TAG, "doReportPVDStatusToAllListeners: notify "
                                + listener + " failed. remove this listener.");
                    }
                    mListeners.remove(listener);
               }
               if (mRemoveListener) {
                   if (VERBOSE_DBG) Log.v(TAG, "doReportPVDStatusToAllListeners - remove listener");
                   mListeners.remove(listener);
               }

            }

            if (mListeners.size() == 0) {
                onAllListenersGone();
            }
        }
    };

    /**
     * Returns Breach position from last registered client
     * It checks if the last known position is valid or not and returns the
     * position / null accordingly.
     * @return BreachPosition - null / known position
     */
    private BreachPosition getLastRegisteredBreachPosition () {
        BreachPosition position = null;
        if (System.currentTimeMillis() - mPVDBreachPosTimeout >
            mLastBreachPositionTimeMs) {
            // Last Breach position not valid any more
            if (VERBOSE_DBG) Log.v(TAG, "getLastRegisteredBreachPosition - Not valid");
        } else {
            // Still valid
            position = mLastBreachPosition;
        }
        return position;
    }

    /**
     * TimeoutTask that extends TimerTask
     * run () gets called upon timer expiration.
     * Sends the PVD Query request to Ulp native upon timer
     * expiration
     */
    private class TimeoutTask extends TimerTask {
        @Override
        public void run () {
            // As long as there is a listener, send the query
            if (mListeners.size() >= 1) {
                // Query with the Breach position from the last registered listener
                sendPVDQuery(getLastRegisteredBreachPosition());
            } else {
                if (VERBOSE_DBG) Log.v(TAG, "TimeTask run - No listeners");
            }

        }
    }


    /**
     * Sends Message to Handler
     * @param what Message ID
     * @param arg1 int argument
     * @param arg2 int argument
     * @param obj Object argument
     */
    private static final void sendMessage(int what, int arg1, int arg2, Object obj) {
        // remove any previous messages in the handler queue except
        // register / deregister requests
        if (!(what == REQUEST_PVD_REGISTER || what == REQUEST_PVD_DEREGISTER)) {
            mHandler.removeMessages(what);
        }
        Message m = Message.obtain(mHandler, what, arg1, arg2, obj);
        mHandler.sendMessage(m);
    }

    /**
     * Handles the PVD status report in Handler.
     *
     * @param status Status of PVD
     * @param json String containing JSON for the PVD
     */
    private void handleReportPVDStatus(int status, String json) {
        if (VERBOSE_DBG) {
            Log.d(TAG, "handleReportPVDStatus status=" + status +
                " json=" + json);
        }

        synchronized (this) {
            // If there is no on going query, ignore any response
            if (true == mOngoingQuery) {
                // No more query ongoing
                mOngoingQuery = false;

                PVDStatus stat = PVDStatus.PVD_STATUS_UNKNOWN;
                int timer = mPVDQueryTimerBeforeDetection;
                switch (status) {
                case ULP_PVD_STATUS_DETECTED: {
                    timer = mPVDQueryTimerAfterDetection;
                    stat = PVDStatus.PVD_STATUS_DETECTED;
                    break;
                }
                case ULP_PVD_STATUS_UNDETECTED: stat = PVDStatus.PVD_STATUS_UNDETECTED; break;
                case ULP_PVD_STATUS_BUSY: stat = PVDStatus.PVD_STATUS_BUSY; break;
                case ULP_PVD_STATUS_GENERAL_FAILURE: stat = PVDStatus.PVD_STATUS_GENERAL_FAILURE; break;
                default: stat = PVDStatus.PVD_STATUS_UNKNOWN; break;
                }

                // check if the timer is to be changed
                if (mPVDQueryTimer != timer) {
                    if (VERBOSE_DBG) Log.v(TAG, "onPVDStatusReceived timer reset");
                    // reset the timer so it kicks in mPVDQueryTimer later
                    // from this current request.
                    mPVDQueryTimer = timer;
                    mTimer.cancel();
                    mTimer = new Timer ();
                    mTimer.schedule(new TimeoutTask(), mPVDQueryTimer, mPVDQueryTimer);
                }

                // Cache the current status with the current time stamp
                mCurrentPVDStatus = stat;
                mCurrentPVDVenueInfo = json;
                mCachedTimeMs = System.currentTimeMillis();

                // Report to all listeners
                ReportPVDStatusTask task = new ReportPVDStatusTask(stat, json, null, false);
                task.execute();
            } else {
                if (VERBOSE_DBG) Log.v(TAG, "onPVDStatusReceived no ongoing query");
            }
        }
    }

    /**
     * Sends the PVD Query message to the handler
     * @param position Breach Position
     * @return true for success, false otherwise
     */
    private boolean sendPVDQuery(BreachPosition position) {
        if (VERBOSE_DBG) Log.d(TAG, "sendPVDQuery position=" + position);

        sendMessage(REQUEST_PVD_QUERY, 0, 0, position);
        return true;
    }

    /**
     * Handles the PVD Query in Handler.
     * Checks for the EULA acceptance and Wifi Enablement before
     * sending the request to the ulp native.
     * @param position Breach position
     * @return true - if request sent to ulp native
     *         false - if request could not be sent
     */
    private boolean handlePVDQuery(BreachPosition position) {

        boolean retVal = false;
        if (VERBOSE_DBG) Log.d(TAG, "handlePVDQuery position=" + position);
        // Check the Wifi settings and Enh settings
        // If any of the settings disabled, send an error to all listeners
        // and remove them from listener list
        PVDStatus stat = PVDStatus.PVD_STATUS_UNKNOWN;
        if (isWiFiEnabled() == false) {
            stat = PVDStatus.PVD_STATUS_WIFI_DISABLED;
        } else if (isEulaAccepted () == false) {
            stat = PVDStatus.PVD_STATUS_NO_EULA;
        }

        if (stat != PVDStatus.PVD_STATUS_UNKNOWN) {
            if (VERBOSE_DBG) {
                Log.v(TAG, "handlePVDQuery report error to all listeners");
            }

            // Report to all listeners and remove
            ReportPVDStatusTask task = new ReportPVDStatusTask(stat, null, null, true);
            task.execute();
        } else if (null == position) {
            if (VERBOSE_DBG) Log.v(TAG, "handlePVDQuery - invalid breach pos, request NLP");
            // Request the position from NLP. The request for PVDQuery will
            // be sent after the position from NLP is available.
            requestNetworkLocation ();
        } else {
            if (VERBOSE_DBG) {
                Log.v(TAG, "handlePVDQuery send the request to ULP");
            }

            if (position == null) {
                UlpEngine.native_ue_send_pvd_query (false, 0.0, 0.0, 0.0f);
            } else {
                UlpEngine.native_ue_send_pvd_query (true, position.latitude,
                        position.longitude, position.accuracy);
            }
            mOngoingQuery = true;
            retVal = true;
        }
        return retVal;
    }

    /**
     * Checks if IZat EULA is accepted by user
     *
     * @return ture, if this is QTI powered device and EULA is
     *               accepted by user;<br/>
     *         false, if this is not QTI powered device or EULA
     *               is not accepted by user. In the second case, Settings
     *               app can be launched to user, which would promp to
     *               user to review and possibly accept EULA.<br/>
     */
    private boolean isEulaAccepted() {
        String s = null;
        try {
            s = Settings.Secure.getString(getApplicationContext().getContentResolver(),
                ENH_LOCATION_SERVICES_ENABLED);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return s != null && s.equals("1");
    }

    /**
     * Checks if WiFi is enabled
     *
     * @return ture, if WiFi is ON.<br/>
     *         false, if WiFi is OFF.<br/>
     */
    private boolean isWiFiEnabled() {
        ContentResolver resolver = null;
        boolean retVal = false;
        try {
            resolver = getApplicationContext().getContentResolver();
            retVal = ((Settings.Global.getInt(resolver,Settings.Global.WIFI_ON) == 1) ||
                (Settings.Global.getInt(resolver,Settings.Global.WIFI_ON) == 2));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return retVal;
    }

// HACK ALERT - Temp code that's to be removed in Phase2
    private LocationManager getLocationManagerInstance () {
        if (null == mLocMgr) {
            mLocMgr = (LocationManager) getApplicationContext().getSystemService(Context.LOCATION_SERVICE);
        }
        return mLocMgr;
    }

    /**
     * Sends the request to get the network location
     */
    private void requestNetworkLocation() {
         if (VERBOSE_DBG) Log.v(TAG, "requestNetworkLocation");
         sendMessage(PVD_REQUEST_NETWORK_LOCATION, 0, 0, null );
    }

    private void handleNetworkLocationUpdate(Location location) {
        if (null != location) {
            Log.v(TAG, "handleNetworkLocationUpdate. lat" + location.getLatitude()+ "lon" +
                    location.getLongitude() + "accurancy " + location.getAccuracy());
            // Create a breach position with this location
            BreachPosition pos = new BreachPosition (location.getLatitude(),
                    location.getLongitude(), location.getAccuracy());
            mLastBreachPosition = pos;
            mLastBreachPositionTimeMs = System.currentTimeMillis();
            // As long as there is a listener, send the query
            if (mListeners.size() >= 1) {
                sendPVDQuery (pos);
            } else {
                Log.v(TAG, "handleNetworkLocationUpdate - no listeners - Ignore");
            }
        } else {
            Log.v(TAG, "handleNetworkLocationUpdate - No location");
            // TODO - Something
        }
    }

    /*
     * NetworkLocationListener
     *
     */
     private LocationListener mNetworkLocationListener = new LocationListener() {
         public void onLocationChanged(Location location) {
             if (VERBOSE_DBG) Log.v(TAG, "onLocationChanged for NLP lat" + location.getLatitude()+ "lon" +
             location.getLongitude() + "accurancy " + location.getAccuracy());
             sendMessage(PVD_UPDATE_NETWORK_LOCATION, NLP_NO_ERROR, 0, location);
         }
         public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
             if (VERBOSE_DBG) Log.v(TAG, "Status update for NLP" + arg0 + " status = " + arg1);
             // We only care about Provider being unavailable for long time.
             // No need to worry about TEMPORARILY_UNAVAILABLE / AVAILABLE
             if (arg1 == LocationProvider.OUT_OF_SERVICE) {
                 sendMessage(PVD_UPDATE_NETWORK_LOCATION, NLP_ERROR, 0, null);
             }
         }
         public void onProviderEnabled(String arg0) {
             if (VERBOSE_DBG) Log.v(TAG, "onProviderEnabled for NLP.state " + arg0);
         }
         public void onProviderDisabled(String arg0) {
             if (VERBOSE_DBG) Log.v(TAG, "onProviderDisabled for NLP.state " + arg0);
             sendMessage(PVD_UPDATE_NETWORK_LOCATION, NLP_ERROR, 0, null);
         }
     };

     /**
      * Handles Network location request in Handler
      */
     private void handleNetworkLocationRequest() {
         if (VERBOSE_DBG) Log.v(TAG, "handleNetworkLocationRequest - single update");
         try {
             LocationRequest request =
                      LocationRequest.createFromDeprecatedProvider(
                               LocationManager.NETWORK_PROVIDER, 0, 0, false);
             request.setHideFromAppOps(true);
             mLocMgr.requestLocationUpdates(request, mNetworkLocationListener,
                                            mHandler.getLooper());
        }
        catch(RuntimeException e) {
            if (VERBOSE_DBG)
                Log.e(TAG, "Cannot request for passive location updates");
        }
     }


// HACK ALERT ENDS

    /**
     * Called from the native code to update the PVD status
     * @param pvdStatus Status of PIP Venue detection - Can be as defined with PVD_STATUS..
     * @param venueInfo Venue information in json format
     */
    public static void reportPVDStatus(int pvdStatus, byte[] venueInfo)
    {
        if (VERBOSE_DBG) {
            Log.v(TAG, "reportPVDStatus pvdStatus: " + pvdStatus + " venueInfo: " + venueInfo);
        }
        String json = null;
        if (null != venueInfo) {
            json = new String (venueInfo);
        }
        sendMessage(HANDLE_PVD_STATUS, pvdStatus, 0, json);
    }

}
