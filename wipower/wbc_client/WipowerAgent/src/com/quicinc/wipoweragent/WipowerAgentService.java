/*=========================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wipoweragent;

import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;

import com.quicinc.wbc.WbcManager;

public class WipowerAgentService extends Service implements WbcManager.WbcEventListener {
    private static final boolean DBG = true;
    private static final String TAG = "WiPwrAg-Svc";
    private WbcManager mWbcManager;

    @Override
    public void onCreate() {
        if (DBG) Log.v(TAG, "onCreate");
        super.onCreate();

        if (SystemProperties.getBoolean("ro.bluetooth.wipower", false) == true) {
            mWbcManager = WbcManager.getInstance();
            mWbcManager.register(this);
        }
    }

    @Override
    public void onDestroy() {
        if (DBG) Log.v(TAG, "onDestroy");
        super.onDestroy();

        if (mWbcManager != null) {
            mWbcManager.unregister(this);
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (DBG) Log.v(TAG, "onBind");
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DBG) Log.v(TAG, "onStartCommand: " + intent );

        if (intent != null && intent.getAction() != null) {
            if (intent.getAction().equals("com.quicinc.wbc.action.WIPOWER_ICON_ENABLE")) {
                this.showNotification();
            } else if (intent.getAction().equals("com.quicinc.wbc.action.WIPOWER_ICON_DISABLE")) {
                this.cancelNotification();
            }
        }

        if (mWbcManager != null) {
            if (DBG) Log.v(TAG, "getWipowerCapable:" + mWbcManager.getWipowerCapable());
            if (DBG) Log.v(TAG, "getPtuPresence" + mWbcManager.getPtuPresence());
            if (DBG) Log.v(TAG, "getWipowerCharging:" + mWbcManager.getWipowerCharging());
            if (DBG) Log.v(TAG, "getChargingReqd:" + mWbcManager.getChargingRequired());
        }

        // restart if got killed
        return START_STICKY;
    }

    @Override
    public void onWbcEventUpdate(int what, int arg1, int arg2) {
        if (DBG) Log.d(TAG, "onWbcEventUpdate rcvd: " + what + ", " + arg1 + ", " + arg2);
    }

    void showNotification() {
        Notification.Builder builder = new Notification.Builder(this);

        builder.setSmallIcon(R.drawable.ic_stat_rezence)
        .setContentTitle(getResources().getText(R.string.wp_charging))
        .setTicker(getResources().getText(R.string.wp_charging))
        .setWhen(0)
        .setDefaults(Notification.DEFAULT_ALL);

        Notification notification = builder.build();

        //NotificationManager notifier = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        //notifier.notify(1, notification);
        this.startForeground(1, notification);
    }

    void cancelNotification() {
        //NotificationManager notifier = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        //notifier.cancel(1);
        this.stopForeground(true);
    }
}
