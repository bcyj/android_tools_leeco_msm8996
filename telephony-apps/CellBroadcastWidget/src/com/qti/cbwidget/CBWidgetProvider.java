/******************************************************************************
 * @file    CBWidgetProvider.java
 * @brief   Implementation of Cell Broadcast Widget for Brazil
 *
 *
 * ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qti.cbwidget;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.UserHandle;
import android.telephony.CellBroadcastMessage;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.RemoteViews;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

public class CBWidgetProvider extends AppWidgetProvider {
    private static String sCBMessage = "";
    private static final String LOG_TAG = "CBWidgetProvider";
    /* Cell Broadcast for channel 50 */
    private static boolean isMSim = false;
    private static final int CB_CHANNEL_50 = 50;

    static final String CB_AREA_INFO_RECEIVED_ACTION =
            "android.cellbroadcastreceiver.CB_AREA_INFO_RECEIVED";

    static final String GET_LATEST_CB_AREA_INFO_ACTION =
            "android.cellbroadcastreceiver.GET_LATEST_CB_AREA_INFO";

    // Require this permission to prevent third-party spoofing.
    static final String CB_AREA_INFO_SENDER_PERMISSION =
            "android.permission.RECEIVE_EMERGENCY_BROADCAST";

    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        for (int appWidgetId : appWidgetIds) {
            Intent cellBroadcastIntent = new Intent(Intent.ACTION_MAIN);
            if (isMSim) {
                cellBroadcastIntent.setComponent(new ComponentName("com.android.settings",
                        "com.android.settings.deviceinfo.MSimStatus"));
            } else {
                cellBroadcastIntent.setComponent(new ComponentName("com.android.settings",
                        "com.android.settings.deviceinfo.Status"));
            }
            cellBroadcastIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            PendingIntent pendingIntent = PendingIntent.getActivity(context, 0,
                    cellBroadcastIntent, 0);
            RemoteViews remoteView = new RemoteViews(context.getPackageName(), R.layout.widget);
            if (TextUtils.isEmpty(sCBMessage)) {
                sCBMessage = context.getString(R.string.cb_default_value);
                // Ask CellBroadcastReceiver to broadcast the latest area info
                // received
                Intent getLatestIntent = new Intent(GET_LATEST_CB_AREA_INFO_ACTION);
                getLatestIntent.putExtra(PhoneConstants.PHONE_KEY, SubscriptionManager.
                        getPhoneId(SubscriptionManager.getDefaultSubId()));
                context.sendBroadcast(getLatestIntent, CB_AREA_INFO_SENDER_PERMISSION);
            }
            remoteView.setTextViewText(R.id.textCB, sCBMessage);
            remoteView.setOnClickPendingIntent(R.id.LinearLayoutCB, pendingIntent);
            appWidgetManager.updateAppWidget(appWidgetId, remoteView);
        }
    }

    @Override
    public void onDisabled(Context context) {
        sCBMessage = context.getString(R.string.cb_default_value);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        onReceiveWithPrivilege(context, intent, false);
    }

    protected void onReceiveWithPrivilege(Context context, Intent intent, boolean privileged) {
        String action = intent.getAction();
        isMSim = TelephonyManager.getDefault().isMultiSimEnabled();

        if (action.equals("android.appwidget.action.APPWIDGET_DISABLED")) {
            this.onDisabled(context);
        } else if (action.equals("android.intent.action.AIRPLANE_MODE")) {
            boolean airplaneModeOn = intent.getBooleanExtra("state", false);
            if (airplaneModeOn) {
                sCBMessage = context.getString(R.string.cb_no_service);
            } else {
                sCBMessage = context.getString(R.string.cb_default_value);
            }
        } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
            int state = intent.getExtras().getInt("state");
            if (state != ServiceState.STATE_IN_SERVICE) {
                sCBMessage = context.getString(R.string.cb_no_service);
            } else if (TextUtils.isEmpty(sCBMessage)) {
                sCBMessage = context.getString(R.string.cb_default_value);
            }
        } else if (action.equals(CB_AREA_INFO_RECEIVED_ACTION)) {
            if (privileged) {
                Bundle extras = intent.getExtras();
                if (extras == null) {
                    return;
                }
                CellBroadcastMessage message = (CellBroadcastMessage) extras.get("message");
                if (message == null) {
                    return;
                }
                String country = "";
                if (isMSim) {
                    country = TelephonyManager.getDefault().getSimCountryIso(message.getSubId());
                } else {
                    country = TelephonyManager.getDefault().getSimCountryIso();
                }
                int serviceCategory = message.getServiceCategory();
                if ("br".equals(country) && serviceCategory == CB_CHANNEL_50) {
                    sCBMessage = message.getMessageBody();
                } else
                    return; // ignoring CB messages from countries other than
                            // Brazil and channels other than 50
            } else {
                Log.e(LOG_TAG, "ignoring unprivileged action " + action);
                return;
            }
        }
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        ComponentName thisAppWidget = new ComponentName(context.getPackageName(),
                CBWidgetProvider.class.getName());
        int[] appWidgetIds = appWidgetManager.getAppWidgetIds(thisAppWidget);
        this.onUpdate(context, appWidgetManager, appWidgetIds);
    }
}
