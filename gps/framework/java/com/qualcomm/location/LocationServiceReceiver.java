/*
 *                     Location Service Reciever
 *
 * GENERAL DESCRIPTION
 *   This file is the receiver for the ACTION SHUTDOWN
 *
 * Copyright (c) 2013-2014 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.location;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.UserHandle;

import com.qualcomm.lib.location.osagent.*;

public class LocationServiceReceiver extends BroadcastReceiver {
    private static final String TAG = "LocationServiceReceiver";

    static {
        System.loadLibrary("locationservice");
    }

    private native void nativeShutdown();

    @Override
    public void onReceive(Context context, Intent intent) {
        try {
          String intentAction = intent.getAction();
          if (intentAction != null){
             if (intentAction.equals(Intent.ACTION_BOOT_COMPLETED)) {
                  Intent i = new Intent(context, LBSSystemMonitorService.class);
                  context.startServiceAsUser(i, UserHandle.OWNER);

                  Intent intentLocationService = new Intent(context, LocationService.class);
                  intentLocationService.setAction("com.qualcomm.location.LocationService");
                  context.startServiceAsUser(intentLocationService, UserHandle.OWNER);

                  context.startServiceAsUser(new Intent(context, OsAgent.class),
                                             UserHandle.OWNER);
            }
          }
        }
        catch (Exception e) {
          e.printStackTrace();
        }
    }
}
