/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       GeoFence service module

GENERAL DESCRIPTION
  GeoFence service module

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/

package com.qualcomm.location;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.ArrayList;
import java.util.Random;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.os.Message;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.Log;
import android.location.GeoFenceParams;

public class GeoFenceKeeper {
    private static final String TAG = "GeoFenceKeeper";
    private static final boolean VERBOSE_DBG = true;
    private static final String[] MSGS = {
        "MSG_ADD",
        "MSG_REMOVE",
        "MSG_REMOVE_APP",
        "MSG_EXPIRE",
        "MSG_BREACH",
        "MSG_SSR"
    };
    private static final int TIME_UNIT_IN_MS = 1 << 16; // little over a minute
    private static final int MSG_ADD = 0;
    private static final int MSG_REMOVE = 1;
    private static final int MSG_REMOVE_APP = 2;
    private static final int MSG_EXPIRE = 3;
    private static final int MSG_BREACH = 4;
    private static final int MSG_SSR = 5;
    private static int mGeoFenceEnumerater = 1;

    private final MyHandler mHandler;
    private final Context mContext;
    private HashMap<PendingIntent, GeoFenceData> mAllGeoFences;
    private HashMap<Integer, GeoFenceData> mRunningGeoFences;
    private HashMap<Long, HashSet<GeoFenceData>> mTickingGeoFences;

    public GeoFenceKeeper(Looper looper, Context context) {
        mHandler = new MyHandler(looper);
        mContext = context;
        mAllGeoFences = new HashMap<PendingIntent, GeoFenceData>();
        mRunningGeoFences = new HashMap<Integer, GeoFenceData>();
        mTickingGeoFences = new HashMap<Long, HashSet<GeoFenceData>>();

        setTestEnv();
    }

    public boolean addGeoFence(GeoFenceServlet client, GeoFenceParams fenceParams) {
        boolean isSet = false;
        if (fenceParams.mRadius >= 0 &&
            // no point to add expired fences; -1 is forever though
            (fenceParams.mExpiration > 0 || fenceParams.mExpiration == -1)) {
            GeoFenceData fenceData = new GeoFenceData(client, fenceParams);
            Message message = Message.obtain(mHandler, MSG_ADD, fenceData);
            mHandler.sendMessage(message);
            isSet = true;
        } else {
            Log.w(TAG, "bad values: radius - "+fenceParams.mRadius+
                  " fenceParams.mExpiration - "+fenceParams.mExpiration);
        }
        return isSet;
    }

    public void removeGeoFence(PendingIntent intent) {
        mHandler.sendMessage(Message.obtain(mHandler, MSG_REMOVE, intent));
    }

    public void removeGeoFenceApp(int uid) {
        mHandler.sendMessage(Message.obtain(mHandler, MSG_REMOVE, uid, 0));
    }

    private void breachEvent(int engID, int whichWay, double lat, double lon) {
        //mHandler.sendMessage(new BreachEvent(engID, whichWay, lat, lon));
        mHandler.sendMessage(Message.obtain(mHandler, MSG_BREACH, engID, whichWay));
    }

    private final class MyHandler extends Handler {
        MyHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            int message = msg.what;
            Log.d(TAG, "handleMessage what - " +
                  ((VERBOSE_DBG && message < MSGS.length) ?
                   MSGS[message] : message));

            switch (message) {
            case MSG_ADD:
                handleAddGeoFence(msg);
                break;
            case MSG_REMOVE:
                handleRemoveGeoFence(msg);
                break;
            case MSG_REMOVE_APP:
                handleRemoveGeoFenceApp(msg);
                break;
            case MSG_EXPIRE:
                handleExpiredGeoFences(msg);
                break;
            case MSG_BREACH:
                handleBreachEvent(msg);
                break;
            default:
                Log.w(TAG, "known message "+message);
            }
        }
    };

    private class GeoFenceData {
        private final GeoFenceServlet mClient;
        private final GeoFenceParams mParams;
        private final Long mLifeInTimeUnit;
        private final Integer mActTransID;
        private Integer mEngID;

        private GeoFenceData(GeoFenceServlet client, GeoFenceParams params) {
            mClient = client;
            mParams = params;
            if (-1 == mParams.mExpiration) {
                mLifeInTimeUnit = -1L;
            } else {
                mLifeInTimeUnit = upTimeInTimeUnit(mParams.mExpiration);
            }
            synchronized (getClass()) {
                mActTransID = mGeoFenceEnumerater++;
            }
            mEngID = null;
        }
    }

    private void handleAddGeoFence(Message m) {
        GeoFenceData fenceData = (GeoFenceData)m.obj;
        // as the part of the first step to add a new geofence, we
        // remove the old fence, if there is one
        removeGeoFence(mAllGeoFences.put(fenceData.mParams.mIntent, fenceData));
        handleCleanAddGeoFence(fenceData);
    }
    private void handleCleanAddGeoFence(GeoFenceData fenceData) {
        long lifeSpan = fenceData.mLifeInTimeUnit;
        Log.v(TAG, "handleAddGeoFence lifeSpan - "+lifeSpan);
        if (lifeSpan != -1) {
            HashSet<GeoFenceData> tickingGroup = mTickingGeoFences.get(lifeSpan);
            if (tickingGroup == null) {
                // no other GeoFences expire at this time slot
                tickingGroup = new HashSet<GeoFenceData>();
                mTickingGeoFences.put(lifeSpan, tickingGroup);

                Message message = Message.obtain(mHandler, MSG_EXPIRE, tickingGroup);
                // start ticking
                mHandler.sendMessageDelayed(message, timeFromNowInMs(lifeSpan));
            }

            tickingGroup.add(fenceData);
        }

        long engID = startGeoFence(fenceData.mActTransID, fenceData.mParams.mLatitude,
                                   fenceData.mParams.mLongitude, fenceData.mParams.mRadius);

        if (engID == -1) {
            Log.w(TAG, "startGeoFence failed");
        } else {
            fenceData.mEngID = fenceData.mActTransID;
            mRunningGeoFences.put(fenceData.mEngID, fenceData);
        }

        logMe("AddGeoFenceHandler");
    }

    private void handleRemoveGeoFence(Message m) {
        PendingIntent intent = (PendingIntent)m.obj;
        removeGeoFence(mAllGeoFences.remove(intent));
        logMe("RemoveGeoFenceHandler");
    }

    private void handleRemoveGeoFenceApp(Message m) {
        int uid = m.arg1;
        ArrayList<PendingIntent> removedFences = null;
        for (Entry<PendingIntent, GeoFenceData> entry : mAllGeoFences.entrySet()) {
            if (entry.getValue().mParams.mUid == uid) {
                if (removedFences == null) {
                    removedFences = new ArrayList<PendingIntent>();
                }
                removedFences.add(entry.getKey());
            }
        }
        if (removedFences != null) {
            for (int i = removedFences.size()-1; i>=0; i--) {
                removeGeoFence(mAllGeoFences.remove(removedFences.get(i)));
            }
            logMe("RemoveGeoFenceUserHandler uid -"+uid);
        }
    }

    private void handleExpiredGeoFences(Message m) {
        HashSet<GeoFenceData> tg = (HashSet<GeoFenceData>)m.obj;

        if (!tg.isEmpty()) {
            long groupID = tg.iterator().next().mLifeInTimeUnit;
            HashSet<GeoFenceData> tickingGroup = mTickingGeoFences.remove(groupID);

            if (tickingGroup != null) {
                HashSet<GeoFenceData> members = tickingGroup;
                for (GeoFenceData member : members) {
                    removeGeoFenceCommon(
                        mAllGeoFences.remove(member.mParams.mIntent), true);
                }
                members.clear();
            }
            logMe("TickingGroup groupID -"+groupID);
        }
    }

    private void handleBreachEvent(Message m) {
        handleBreachEvent(m.arg1, m.arg2);
    }
    private void handleBreachEvent(int id, int direction){
        GeoFenceData geofence = mRunningGeoFences.get(id);
        if (null != geofence) {
            PendingIntent intent = geofence.mParams.mIntent;
            Intent enteredIntent = new Intent();
            enteredIntent.putExtra(LocationManager.KEY_PROXIMITY_ENTERING,
                                   (direction+1) == GeoFenceParams.ENTERING);
            try {
                intent.send(mContext, 0, enteredIntent);
            } catch (CanceledException e) {
                // remove geofence associated with this pending intent
                removeGeoFence(intent);
            }
        }
    }

    private GeoFenceData removeGeoFenceCommon(GeoFenceData fenceData, boolean expired) {
        if (fenceData != null) {
            if (fenceData.mEngID == null) {
                Log.e(TAG, "mEngID null, invalid data?");
            } else {
                mRunningGeoFences.remove(fenceData.mEngID);

                // stop and remove the geoFence
                stopGenFence(fenceData.mEngID);
            }

            fenceData.mClient.onGeoFenceDone(fenceData.mParams.mIntent, expired);
        }
        return fenceData;
    }

    private void removeGeoFence(GeoFenceData gfd) {
        GeoFenceData fenceData = removeGeoFenceCommon(gfd, false);
        if (fenceData != null &&
            fenceData.mLifeInTimeUnit != -1) {
            HashSet<GeoFenceData> tickingGroup = mTickingGeoFences
                    .get(fenceData.mLifeInTimeUnit);
            if (tickingGroup != null) {
                tickingGroup.remove(fenceData);
                if (tickingGroup.isEmpty()) {
                    mHandler.removeMessages(MSG_EXPIRE, tickingGroup);
                    mTickingGeoFences.remove(fenceData.mLifeInTimeUnit);
                }
            }
        }
    }

    private final class BreachEvent implements Runnable {
        private final int mID;
        private final int mDirection;
        private final double mLat;
        private final double mLon;
        BreachEvent(int id, int whichWay, double lat, double lon) {
            mID = id;
            mDirection = whichWay;
            mLat = lat;
            mLon = lon;
        }

        @Override
        public void run() {
            handleBreachEvent(mID, mDirection);
        }
    }

    private void logMe(String where) {
        if (VERBOSE_DBG) {
            Log.v(TAG, "logMe "+where);
            Log.v(TAG, "mAllGeoFences -"+mAllGeoFences);
            Log.v(TAG, "mRunningGeoFences -"+mRunningGeoFences);
            Log.v(TAG, "mTickingGeoFences -"+mTickingGeoFences);
        }
    }

    // converts a time relative to new in milliseconds
    // to a time relative to the device powerup in "time units"
    // "time units" is roughly a minute, or (1 << TIME_UNIT_IN_MS) ms
    static private long upTimeInTimeUnit(long timeFromNowInMs) {
        return (SystemClock.uptimeMillis() + timeFromNowInMs) >> TIME_UNIT_IN_MS;
    }

    // converts a time relative to the device powerup in "time units"
    // to a time relative to now in milliseconds.
    // "time units" is roughly a minute, or (1 << TIME_UNIT_IN_MS) ms
    static private long timeFromNowInMs(long upTimeInTimeUnit) {
        return (upTimeInTimeUnit << TIME_UNIT_IN_MS)
                - SystemClock.uptimeMillis();
    }

    private native long startGeoFence(int txID, double lat, double lon, float radius);
    private native void stopGenFence(int engID);
    private native void testBreach(int breacherID);
    private static native void classInit();

    static {
        System.loadLibrary("locationservice");
        classInit();
    }

    // here below is for on target test only
    private static final boolean ON_TARGET_DEBUG = true;
    private static final String TEXT_ACTION_POS_INJ =
        "com.qualcomm.location.geofence.breach";
    private static final String TEXT_ACTION_CHECK_GEOFENCES =
        "com.qualcomm.location.geofence.checkgeofences";
    private Random mRandom;
    private final BroadcastReceiver mTestPosInjRcvr = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int size = mAllGeoFences.size();
            if (size == 0) {
                Log.v(TAG, "no active GeoFences");
            } else if (action.equals(TEXT_ACTION_POS_INJ)) {
                int rand = mRandom.nextInt(size);
                Log.v(TAG, "action="+TEXT_ACTION_POS_INJ+", "+size+" fences, breach "+rand);
                Iterator<GeoFenceData> values = mAllGeoFences.values().iterator();
                GeoFenceData geoFenceData = values.next();

                for (int i = 0; i < rand; i++) {
                    geoFenceData = values.next();
                }
                testBreach(geoFenceData.mEngID);
            } else if (action.equals(TEXT_ACTION_CHECK_GEOFENCES)) {
                debugCheckData();
            }
        }
    };
    private void setTestEnv() {
        if (ON_TARGET_DEBUG) {
            mRandom = new Random();
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(TEXT_ACTION_POS_INJ);
            intentFilter.addAction(TEXT_ACTION_CHECK_GEOFENCES);
            mContext.registerReceiver(mTestPosInjRcvr, intentFilter);
        }
    }
    private void unsetTestEnv() {
        if (ON_TARGET_DEBUG) {
            mContext.unregisterReceiver(mTestPosInjRcvr);
        }
    }

    public void debugCheckGeoFences() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                debugCheckData();
            }
        });
    }

    private void debugCheckData() {
        if (ON_TARGET_DEBUG) {
            Log.v(TAG, "debugCheckData: number checking...");

            int allSize = mAllGeoFences.size();
            int runningSize = mRunningGeoFences.size();
            // cheap check first.
            if (allSize != runningSize) {
                String msg = "debugCheckData size check failed.. allSize - "+allSize
                    +" runningSize - "+runningSize;
                throw new Error(msg);
            }

            Log.v(TAG, "debugCheckData: mRunningGeoFences KEY checking...");
            for (Entry<Integer, GeoFenceData> entry : mRunningGeoFences.entrySet()) {
                if (entry.getKey() != entry.getValue().mEngID) {
                    String msg = "debugCheckData mRunningGeoFences KEY check failed.. "
                        +" mismatched mEngID - "+entry.getKey()+" mRunningGeoFences "
                        +mRunningGeoFences;
                    throw new Error(msg);
                }
            }
            Log.v(TAG, "debugCheckData: mAllGeoFences KEY checking...");
            for (Entry<PendingIntent,GeoFenceData> entry : mAllGeoFences.entrySet()) {
                if (!entry.getKey().equals(entry.getValue().mParams.mIntent)) {
                    String msg = "debugCheckData mAllGeoFences KEY check failed.. "
                        +" mismatched PendingIntent - "+entry.getKey()+" mAllGeoFences - "
                        +mAllGeoFences;
                    throw new Error(msg);
                }
            }

            Log.v(TAG, "debugCheckData: cross reference checking...");
            int tickingSize = 0;
            Collection allFences = mAllGeoFences.values();
            Collection runningFences = mRunningGeoFences.values();
            long curStamp = upTimeInTimeUnit(0);

            for (Entry<Long, HashSet<GeoFenceData>> tge : mTickingGeoFences.entrySet()) {
                // timestamp check first, since it is cheap
                if (curStamp > tge.getKey()) {
                    String msg = "debugCheckData timestamp check failed.. curStamp - "
                        +curStamp+" tge.ts "+tge.getKey()+" mTickingGeoFences "
                        +mTickingGeoFences;
                    throw new Error(msg);
                }
                Collection fences = tge.getValue();
                if (!allFences.containsAll(fences)) {
                    String msg = "debugCheckData cross check failed.. "
                        +"not in mAllGeoFences, tickgroup - "+tge.getKey()
                        +" mTickingGeoFences - "+mTickingGeoFences
                        +"mAllGeoFences - "+mAllGeoFences;
                    throw new Error(msg);
                }
                if (!runningFences.containsAll(fences)) {
                    String msg = "debugCheckData cross check failed.. "
                        +"not in mRunningGeoFences, tickgroup - "+tge.getKey()
                        +" mTickingGeoFences - "+mTickingGeoFences
                        +" mRunningGeoFences - "+mRunningGeoFences;
                    throw new Error(msg);
                }
                tickingSize += fences.size();
            }

            Log.v(TAG, "debugCheckData: tickingSize checking...");
            if (allSize < tickingSize) {
                String msg = "debugCheckData size check failed.. allSize - "+allSize
                    +" tickingSize - "+tickingSize;
                throw new Error(msg);
            }

            Log.v(TAG, "debugCheckData check passed!!!");
        }
    }
}
