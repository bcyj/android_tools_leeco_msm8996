/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.lci;

import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.util.Log;
import android.os.AsyncTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.NoSuchElementException;
import java.util.Iterator;
import java.util.List;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.os.SystemProperties;

import com.qualcomm.location.LBSSystemMonitorService;
import com.qualcomm.location.LocationService;

import com.qualcomm.location.lci.ILciService;
import com.qualcomm.location.lci.ILciResponseListener;

import com.qualcomm.location.ulp.UlpEngine;
import com.qualcomm.location.ulp.UlpEngine.UlpLciListener;

/**
 * @hide
 */
public class LciService extends Service {
    private static final String TAG = "LciService";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);
    private static final String LCI_SERVICE_NAME = "com.qualcomm.location.lci.LciService";

    // data structure for keeping <ILciResponseListener, ListenerDeathRecipient> tuples
    // allow parallel access of multiple threads
    private ConcurrentHashMap<ILciResponseListener, ListenerDeathRecipient> mListeners
        = new ConcurrentHashMap<ILciResponseListener, ListenerDeathRecipient>(8, 0.9f, 1);

    private UlpEngine mUlpEngine = null;
    private String mCurrentLciCandidates = null;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (VERBOSE_DBG) Log.v(TAG, "onStartCommand Event");

        // register service on entry
        if (ServiceManager.getService(LCI_SERVICE_NAME) == null) {
            Log.d(TAG, "start LCI service");

            ServiceManager.addService(LCI_SERVICE_NAME, mBinder.asBinder());
            mUlpEngine = UlpEngine.getInstance(getApplicationContext());
            mUlpEngine.startListenLciCandidatesNonBlock(mUlpLciListener);
        }

        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        if (VERBOSE_DBG) Log.v(TAG, "onDestroy Event");

         // unregister service on exit
        if (ServiceManager.getService(LCI_SERVICE_NAME) != null) {
            mUlpEngine.stopListenLciCandidatesNonBlock();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        // ignore
        return null;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // ignore
        return false;
    }

    private final ILciService.Stub mBinder = new ILciService.Stub() {
        private int mRefCount = 0;
        private ActivityManager mActivityManager = null;

        @Override
        public synchronized int addRef() {
           if (++mRefCount == 1) {
               onFirstClientRegistered();
           }

           return mRefCount;
        }

        @Override
        public synchronized int release() {
            if (--mRefCount <= 0) {
                 onAllClientsGone();
            }
            return mRefCount;
        }

        @Override
        public int requestLciCandidates(ILciResponseListener listener) {
            LciStatusCode rv = LciStatusCode.SUCCESS;

            if (VERBOSE_DBG) Log.v(TAG, "requestLciCandidates");

            if (listener == null || listener.asBinder() == null) {
                rv = LciStatusCode.FAIL;
                return rv.getInt();
            }

            ListenerDeathRecipient death = new ListenerDeathRecipient(listener);
            try {
                listener.asBinder().linkToDeath(death, 0);
            } catch (RemoteException e) {
                rv = LciStatusCode.FAIL;
                return rv.getInt();
            }
            mListeners.put(listener, death);

            // return current LCI candidates for first time
            ReportLciCandidatesTask task = new ReportLciCandidatesTask(mCurrentLciCandidates, listener);
            task.execute();

            return rv.getInt();
        }

        @Override
        public int sendUserLciSelection(String lci) {
            LciStatusCode rv = LciStatusCode.SUCCESS;

            if (VERBOSE_DBG) Log.v(TAG, "sendUserLciSelection(" + lci + ")");

            if (!mUlpEngine.isScreenOn()) {
                if (VERBOSE_DBG) Log.w(TAG, "sendUserLciSelection() must be called while screen on.");
                rv = LciStatusCode.FAIL_CALL_NOT_SCREEN_ON;
                return rv.getInt();
            }

            if (!checkIfCallerInForeground()) {
                if (VERBOSE_DBG) Log.w(TAG, "sendUserLciSelection() must be called by foreground app.");
                rv = LciStatusCode.FAIL_APP_NOT_FOREGROUND;
                return rv.getInt();
            }

            mUlpEngine.sendUserLciSelectionNonBlock(lci);

            return rv.getInt();
        }

        @Override
        public int unregisterListener(ILciResponseListener listener) {
            LciStatusCode rv = LciStatusCode.SUCCESS;

            if (VERBOSE_DBG) Log.v(TAG, "unregisterListener");

            if (listener == null || listener.asBinder() == null) {
                rv = LciStatusCode.FAIL;
                return rv.getInt();
            }

            ILciResponseListener origListener = findOrigListener(listener);
            if (origListener != null) {
                ListenerDeathRecipient death = mListeners.get(origListener);
                if (death != null) {
                    try {
                        origListener.asBinder().unlinkToDeath(death, 0);
                    } catch (NoSuchElementException e) {
                        rv = LciStatusCode.FAIL;
                    }
                }

                if (VERBOSE_DBG) Log.v(TAG, "remove listener " + origListener);
                mListeners.remove(origListener);
            }

            if (mListeners.size() == 0) {
                onAllListenersGone();
            }

            return rv.getInt();
        }

        private ILciResponseListener findOrigListener(ILciResponseListener listener) {
            IBinder binder = listener.asBinder();

            for (ILciResponseListener l : mListeners.keySet()) {
                if (l.asBinder() == binder) {
                    return l;
                }
            }

            return null;
        }

        private boolean checkIfCallerInForeground() {
            RunningAppProcessInfo fgApp = null;
            RunningAppProcessInfo app;
            int callingPid;

            callingPid = getCallingPid();

            if(mActivityManager == null) {
               mActivityManager = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
               if(mActivityManager == null) {
                  return false;
               }
            }

            // get fg app
            List <RunningAppProcessInfo> list = mActivityManager.getRunningAppProcesses();
            Iterator <RunningAppProcessInfo> iter = list.iterator();
            while(iter.hasNext()){
               app = iter.next();
               if(app.pid == callingPid){
                   fgApp = app;
                   break;
               }
            }

            // compare with fg app
            boolean isAppInForeground = (fgApp!= null && fgApp.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND);
            if (VERBOSE_DBG) {
                Log.v(TAG, "foreground check:" + ((fgApp == null)? "null" : Integer.toString(fgApp.importance)));
            }

            return isAppInForeground;
        }

    };

    private class ListenerDeathRecipient implements IBinder.DeathRecipient {
        ILciResponseListener mListener;

        ListenerDeathRecipient(ILciResponseListener listener) {
            mListener = listener;
        }

        @Override
        public void binderDied() {
            if (VERBOSE_DBG) Log.v(TAG, "binderDided(" + mListener + ")");

            if (mListeners.contains(mListener)) {
                if (VERBOSE_DBG) Log.v(TAG, "binderDided(" + mListener + "). removed it");
                mListeners.remove(mListener);
            }

            if (mListeners.size() == 0) {
                onAllListenersGone();
            }
        }
    }

    private void onAllListenersGone() {
        if (VERBOSE_DBG) Log.v(TAG, "all listeners are gone");

        // TODO: time to release resource?
    }

    private void onFirstClientRegistered() {
        if (VERBOSE_DBG) Log.v(TAG, "first client connected");

        // enable PIP
        mUlpEngine.updatePipUserSetting(true);
    }

    private void onAllClientsGone() {
        if (VERBOSE_DBG) Log.v(TAG, "all clients are gone");

        // TODO: time to release resource?
        // stopSelf();

        // disable PIP
        mUlpEngine.updatePipUserSetting(false);
    }

    private class ReportLciCandidatesTask extends AsyncTask<Void, Void, Void> {
        private String mJson;
        private ILciResponseListener mListener;

        /**
         * @param json LCI candidates in JSON string
         * @param listener send data to given listener; if null send to all listeners
         */
        public ReportLciCandidatesTask(String json, ILciResponseListener listener) {
            if (VERBOSE_DBG) {
                Log.v(TAG, "create ReportLciCandidatesTask(json=" + json + ",listener=" + listener + ")");
            }
            mJson = json;
            mListener = listener;
        }

        @Override
        protected Void doInBackground(Void... params) {
            if (mListener != null) {
                doReportLciCandidatesToSingleListener();
            } else {
                doReportLciCandidatesToAllListeners();
            }

            return null;
        }

        private void doReportLciCandidatesToSingleListener() {
            if (VERBOSE_DBG) Log.v(TAG, "doReportLciCandidatesToSingleListener");
            try {
                mListener.onLciCandidatesReceived(mJson);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        private void doReportLciCandidatesToAllListeners() {
           if (VERBOSE_DBG) Log.v(TAG, "doReportLciCandidatesToAllListeners");

           for (ILciResponseListener listener : mListeners.keySet()) {
               if (VERBOSE_DBG) Log.v(TAG, "doReportLciCandidatesToAllListeners: notify " + listener);
               try {
                    listener.onLciCandidatesReceived(mJson);
               } catch (RemoteException e) {
                    if (VERBOSE_DBG) {
                        Log.v(TAG, "doReportLciCandidatesToAllListeners: notify "
                                + listener + " failed. remove this listener.");
                    }
                    mListeners.remove(listener);
               }
            }

            if (mListeners.size() == 0) {
                onAllListenersGone();
            }
        }
    };

    private UlpLciListener mUlpLciListener = new UlpLciListener() {
        @Override
        public void onLciCandidatesReceivedNonBlock(String json){
            if (VERBOSE_DBG) Log.v(TAG, "onLciCandidatesReceivedNonBlock(json=" + json + ")");

            //XXX: remove this block when no duplicate data will be received
            if ((json == null && mCurrentLciCandidates == null)
                    || (json != null && json.equalsIgnoreCase(mCurrentLciCandidates))) {
                if (VERBOSE_DBG) Log.v(TAG, "onLciCandidatesReceivedNonBlock() repeated data. ignored");
                return;
            }

            mCurrentLciCandidates = json;
            ReportLciCandidatesTask task = new ReportLciCandidatesTask(json, null);
            task.execute();
        }
    };

}
