/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.utils;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsNativeUIApp;
import com.suntek.mway.rcs.nativeui.ui.RcsNotificationListActivity;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

public class RcsNotifyUtil {

    private static final int NOTIFY_ID = 20121237;

    public static final String ACTION_NAME = 
            "com.suntek.mway.rcs.nativeui.ACTION_LUNCHER_RCS_NOTIFICATION_LIST";

    private static NotificationManager notifManager;

    private static int notifyUnreadCount = 0;

    private RcsNotifyUtil() {
    }

    @SuppressWarnings("deprecation")
    public static void showNotification() {
        Context context = RcsNativeUIApp.getContext();
        notifyUnreadCount++;
        notifManager = (NotificationManager) context.getSystemService(
                Context.NOTIFICATION_SERVICE);
        if (notifManager == null)
            return;
        Intent intent = new Intent(ACTION_NAME);
        PendingIntent contentIntent = PendingIntent.getActivity(context,
                NOTIFY_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        final Notification notification = new Notification(R.drawable.calllog_tool_sms,
                context.getString(R.string.notifications),
                System.currentTimeMillis());
        notification.setLatestEventInfo(context, context.getString(R.string.notifications),
                context.getString(R.string.notifications_unread_count, notifyUnreadCount),
                contentIntent);
        notification.defaults = Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE;
        notification.flags = Notification.FLAG_AUTO_CANCEL;
        notifManager.notify(NOTIFY_ID, notification);
    }

    public static void cancelNotif() {
        notifManager = (NotificationManager) RcsNativeUIApp.getContext().getSystemService(
                Context.NOTIFICATION_SERVICE);
        if (notifManager != null) {
            notifyUnreadCount = 0;
            notifManager.cancel(NOTIFY_ID);
        }
    }
}
