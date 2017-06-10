/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.borqs.videocall;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.ComponentName;
import com.qualcomm.qti.vtremoteservice.IRemoteService;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;

public class RemoteServiceConnector {

    private static Context ctx = null;
    private static IRemoteService iRemoteService;
    private ServiceConnection sConn;
    private static final String TAG = "RemoteServiceConnector";

    public RemoteServiceConnector(Context ctx) {
        this.ctx = ctx;
        try {
            System.loadLibrary("omx_amrdec_sharedlibrary");
            sConn = new ServiceConnection() {
                @Override
                public void onServiceDisconnected(ComponentName name) {
                    iRemoteService = null;
                }

                @Override
                public void onServiceConnected(ComponentName name,
                        IBinder service) {
                    Log.d(TAG, "VT RemoteService connected");
                    iRemoteService = IRemoteService.Stub
                            .asInterface((IBinder) service);
                }
            };
            Intent intent = new Intent();
            intent.setComponent(new ComponentName(
                    "com.qualcomm.qti.vtremoteservice",
                    "com.qualcomm.qti.vtremoteservice.VTRemoteService"));
            ctx.bindService(intent, sConn, Context.BIND_AUTO_CREATE);
        }catch (SecurityException secEx){
            Log.e(TAG, "unable to load omx_amrdec_sharedlibrary"+ secEx);
        }catch (NullPointerException nullEx) {
            Log.e(TAG, "omx_amrdec_sharedlibrary not exist"+ nullEx);
        }
    }

    public static void setProperty(String key, String val) {
        try {
            iRemoteService.setProperty(key, val);
        } catch (RemoteException ex) {
            Log.i(TAG, "RemoteServiceConnector setProperty ex=" + ex);
        }
    }
}