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
import java.util.NoSuchElementException;

import android.app.PendingIntent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.location.IGeoFenceListener;
import android.location.IGeoFencer;
import android.location.GeoFenceParams;

public class GeoFenceServlet implements IBinder.DeathRecipient {
    private static final String TAG = "GeoClientRemoteHandle";

    static private final HashMap<IGeoFenceListener, GeoFenceServlet> mAllClients = new HashMap<IGeoFenceListener, GeoFenceServlet>();

    static synchronized GeoFenceServlet getGeoFenceServlet(
            IBinder client, GeoFenceKeeper keeper) {
        GeoFenceServlet clientHandle = mAllClients.get(client);
        if (clientHandle == null) {
            clientHandle = new GeoFenceServlet(client, keeper);
            mAllClients.put(clientHandle.mClient, clientHandle);
        }
        return clientHandle;
    }

    private final IGeoFenceListener mClient;
    private final HashSet<PendingIntent> mGeoFenceIDs;
    private final GeoFenceKeeper mGeoFenceKeeper;

    private GeoFenceServlet(IBinder client, GeoFenceKeeper keeper) {
        mClient = IGeoFenceListener.Stub.asInterface(client);
        mGeoFenceIDs = new HashSet<PendingIntent>();
        mGeoFenceKeeper = keeper;
        try {
            mClient.asBinder().linkToDeath(this, 0);
        } catch (RemoteException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    @Override
    public void binderDied() {
        Log.d(TAG, "binderDied");
        for (PendingIntent fence : mGeoFenceIDs) {
            mGeoFenceKeeper.removeGeoFence(fence);
        }
        mGeoFenceIDs.clear();
        synchronized (mAllClients) {
            mAllClients.remove(this);
        }
    }

    boolean addGeoFence(GeoFenceParams fence) {
        mGeoFenceIDs.add(fence.mIntent);
        return mGeoFenceKeeper.addGeoFence(this, fence);
    }

    void removeGeoFence(PendingIntent fenceID) {
        mGeoFenceKeeper.removeGeoFence(fenceID);
    }

    void onGeoFenceDone(PendingIntent fenceID, boolean expired) {
        mGeoFenceIDs.remove(fenceID);
        if (expired) {
            try {
                mClient.geoFenceExpired(fenceID);
            } catch (RemoteException re) {
                // do nothing if remote process died
                // binderDied() will take care of the rest
            }
        }
        if (mGeoFenceIDs.isEmpty()) {
            try {
                mClient.asBinder().unlinkToDeath(this, 0);
                synchronized (mAllClients) {
                    mAllClients.remove(this);
                }
            } catch (NoSuchElementException nsee) {
            }
        }
    }
}
