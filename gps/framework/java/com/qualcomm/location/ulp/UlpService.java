/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

package com.qualcomm.location.ulp;

import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.os.IBinder;
import android.os.UserHandle;
import android.util.Log;
import com.qualcomm.location.LBSSystemMonitorService;
import com.qualcomm.location.LocationService;
import com.qualcomm.lib.location.osagent.*;

public class UlpService extends Service {
    private static final String TAG = "UlpService";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);

    private UnifiedLocationProvider mProvider;

    @Override
    public IBinder onBind(Intent intent) {
        if (VERBOSE_DBG)
                Log.v(TAG, "onBind Event");
        if (mProvider == null) {
            startAdditionalServices(getApplicationContext());
            mProvider = new UnifiedLocationProvider(getApplicationContext());
        }
        return mProvider.getBinder();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        if (VERBOSE_DBG)
                Log.v(TAG, "onUnbind Event");
        // make sure to stop performing work
        if (mProvider != null) {
            mProvider.onDisable();
        }
      return false;
    }

    @Override
    public void onDestroy() {
        if (VERBOSE_DBG)
                Log.v(TAG, "onDestroy Event");
        mProvider = null;
    }

    private void startAdditionalServices(Context context) {
        Log.v(TAG, "+startAdditionalServices");
        Intent i = new Intent(context, LBSSystemMonitorService.class);
        context.startServiceAsUser(i, UserHandle.OWNER);

        Intent intentLocationService = new Intent(context, LocationService.class);
        intentLocationService.setAction("com.qualcomm.location.LocationService");
        context.startServiceAsUser(intentLocationService, UserHandle.OWNER);

        Intent j = new Intent();
        j.setPackage("com.qualcomm.location.XT");
        j.setAction("com.android.location.XT.IXTSrv");
        context.startServiceAsUser(j, UserHandle.OWNER);

        Intent osAgentServiceIntent = new Intent(context, OsAgent.class);
        osAgentServiceIntent.setAction("com.qualcomm.lib.location.osagent.OsAgent");
        context.startServiceAsUser(osAgentServiceIntent, UserHandle.OWNER);

        Intent alarmServicesIntent = new Intent();
        alarmServicesIntent.setAction("com.qualcomm.services.location.LocationAlarmService");
        alarmServicesIntent.setPackage("com.qualcomm.services.location");
        context.startServiceAsUser(alarmServicesIntent, UserHandle.OWNER);

        Log.v(TAG, "-startAdditionalServices");

    }
}
