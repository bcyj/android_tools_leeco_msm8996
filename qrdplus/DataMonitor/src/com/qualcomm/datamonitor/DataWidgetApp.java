/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.datamonitor;

import android.app.Application;
import android.appwidget.AppWidgetManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

public class DataWidgetApp extends Application {

    private static final String TAG = "DataWidgetApp";
    private static final String ACTION_NETWORK_STATS_UPDATED =
            "com.android.server.action.NETWORK_STATS_UPDATED";

    /*
     * The broadcast "com.android.server.action.NETWORK_STATS_UPDATED" is sent
     * by NetworkStatsService when it perform poll once, the method
     * performPollLocked will be executed every time system persist the pending
     * network status actively (except shutdown) such as persist status every 30
     * minutes through AlarmManager, Settings request to get the data usage,
     * update data usage history, etc. So we catch the broadcast here to update
     * the UI of our Data Widget to make sure the Widget keep the same status
     * with the status which NetworkStatsService saved.
     */
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Date network off do not need to update widget.
            if (DataUtils.getMobileDataEnabled(context) == false) {
                return;
            }
            if (DataWidget.getRemoteViews() != null &&
                    DataWidget.getWidget() != null) {
                DataWidget.refreshWidgetView(context,
                        DataWidget.getRemoteViews());
                try {
                    AppWidgetManager.getInstance(context).updateAppWidget(
                            DataWidget.getWidget(), DataWidget.getRemoteViews());
                } catch (Exception e) {
                    Log.e(TAG, "Can't update AppWidget: " + e);
                }
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_NETWORK_STATS_UPDATED);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
        unregisterReceiver(mReceiver);
    }
}
