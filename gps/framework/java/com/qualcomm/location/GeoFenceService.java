/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       GeoFence service module

GENERAL DESCRIPTION
  GeoFence service module

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/

package com.qualcomm.location;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.location.IGeoFencer;
import android.location.GeoFenceParams;

public class GeoFenceService extends Service {
    private static final String TAG = "GeoFenceService";
    private static final boolean LOCAL_LOG = true;

    private HandlerThread mKeeperThread;
    private GeoFenceKeeper mGeoFenceKeeper;

    private final IGeoFencer.Stub mGeoFencer = new IGeoFencer.Stub() {
        @Override
        public boolean setGeoFence(IBinder who, GeoFenceParams params)
                throws RemoteException {
            boolean isSet = false;
            synchronized (mKeeperThread) {
                if (mKeeperThread != null) {
                    GeoFenceServlet servlet = GeoFenceServlet
                            .getGeoFenceServlet(who, mGeoFenceKeeper);
                    Log.v(TAG, "setGeoFence "+params.toString());
                    isSet = servlet.addGeoFence(params);
                }
            }
            return isSet;
        }

        @Override
        public void clearGeoFence(IBinder who, PendingIntent fence)
                throws RemoteException {
            synchronized (mKeeperThread) {
                if (mKeeperThread != null) {
                    GeoFenceServlet servlet = GeoFenceServlet
                            .getGeoFenceServlet(who, mGeoFenceKeeper);
                    Log.v(TAG, "clearGeoFence "+fence.toString());
                    servlet.removeGeoFence(fence);
                }
            }
        }

        @Override
        public void clearGeoFenceUser(int uid)
                throws RemoteException {
            synchronized (mKeeperThread) {
                if (mKeeperThread != null) {
                    Log.v(TAG, "clearGeoFence uid - "+uid);
                    mGeoFenceKeeper.removeGeoFenceApp(uid);
                }
            }
        }

        // @Override
        public void debugCheckGeoFences() throws RemoteException {
           synchronized (mKeeperThread) {
               if (mKeeperThread != null) {
                   mGeoFenceKeeper.debugCheckGeoFences();
               }
           }
        }
    };

    @Override
    public void onCreate() {
        log("onCreate");
        mKeeperThread = new HandlerThread(TAG);
        mKeeperThread.start();
        mGeoFenceKeeper = new GeoFenceKeeper(mKeeperThread.getLooper(), this);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        log("onBind");
        return mGeoFencer;
    }

    @Override
    public void onDestroy() {
        Log.v(TAG, "onDestroy");
        synchronized (mKeeperThread) {
            if (mKeeperThread != null) {
                mKeeperThread.quit();
                mKeeperThread = null;
                mGeoFenceKeeper = null;
            }
        }
        // the process is going away. We shouldn't need to do anything else.
        // it is expected that QMI notifies modem. And the rest will be handled
        // by locMW.
    }

    private void log(String s) {
        if (LOCAL_LOG) {
            Log.d(TAG, s);
        }
    }
}
