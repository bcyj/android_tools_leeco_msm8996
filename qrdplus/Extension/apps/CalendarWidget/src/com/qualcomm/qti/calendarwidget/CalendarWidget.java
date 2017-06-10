/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


package com.qualcomm.qti.calendarwidget;

import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.provider.CalendarContract.Calendars;
import android.util.Log;
import android.widget.RemoteViewsService;

public class CalendarWidget extends AppWidgetProvider {
    private static final String TAG = "CalendarWidget";

    @Override
    public void onEnabled(Context context) {
        if (Utility.DEBUG) {
            Log.i(TAG, "Enable this widget.");
        }

        // If we enable this widget, we will set the date to today.
        WidgetManager.gotoToday();
        super.onEnabled(context);
    }


    @Override
    public void onDisabled(Context context) {
        if (Utility.DEBUG) {
            Log.i(TAG, "Disable this widget.");
        }

        // If we disable this widget, we need stop the services.
        context.stopService(new Intent(context, DateService.class));
        context.stopService(new Intent(context, CommonService.class));
        context.stopService(new Intent(context, WeekNumberService.class));
        super.onDisabled(context);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (Utility.DEBUG) {
            Log.i(TAG, "receive the action:" + action);
        }

        AppWidgetManager awm = AppWidgetManager.getInstance(context);
        int[] ids = awm.getAppWidgetIds(new ComponentName(context,
                CalendarWidget.class));
        if (Intent.ACTION_TIME_CHANGED.equals(action)
                || Intent.ACTION_DATE_CHANGED.equals(action)
                || Intent.ACTION_TIMEZONE_CHANGED.equals(action)) {
            // we need to go to today if we changed date manually.
            if (!WidgetManager.isToday()) {
                WidgetManager.gotoToday();
            }
            WidgetManager.updateWidgets(context, awm, ids);
        } else if (Utility.COMMAND_DATE_CHANGED.equals(action)) {
            WidgetManager.updateWidgets(context, awm, ids);
        } else if (Utility.COMMAND_GOTO_TODAY.equals(action)) {
            if (!WidgetManager.isToday()) {
                WidgetManager.gotoToday();
                WidgetManager.updateWidgets(context, awm, ids);
            }
        } else if (Utility.COMMAND_REFRESH.equals(action)) {
            // we need to request sync.
            if (Utility.getAccountCount(context) > 0) {
                Bundle extras = new Bundle();
                extras.putBoolean(ContentResolver.SYNC_EXTRAS_MANUAL, true);
                extras.putBoolean("metafeedonly", true);
                ContentResolver.requestSync(null /* all accounts */,
                        Calendars.CONTENT_URI.getAuthority(), extras);
                // update the refresh image state
                WidgetManager.updateWidgets(context, awm, ids);
            } else {
                Intent noAccount = new Intent();
                noAccount.setClass(context, NoAccountAlert.class);
                noAccount.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(noAccount);
            }
        } else if (Intent.ACTION_PACKAGE_CHANGED.equals(action)) {
            String packageName = intent.getData().getSchemeSpecificPart();
            if (Utility.CALENDAR_PACKAGE_NAME.equals(packageName)) {
                Log.d(TAG,
                        "Receive the calendar application update action. Update the widget state.");
                // we need update the add event button's state.
                WidgetManager.updateWidgets(context, awm, ids);
            }
        } else {
            super.onReceive(context, intent);
        }
    }

    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager,
            int[] appWidgetIds) {
        WidgetManager.updateWidgets(context, appWidgetManager, appWidgetIds);
        super.onUpdate(context, appWidgetManager, appWidgetIds);
    }

    /**
     * We use the DateService for: 1) To provide a widget factory for
     * RemoteViews. 2) Catch our command, and make the DateViews to process the
     * intent.
     */
    public static class DateService extends RemoteViewsService {

        @Override
        public RemoteViewsFactory onGetViewFactory(Intent intent) {
            int[] widgetIds = intent == null ? null : intent
                    .getIntArrayExtra(AppWidgetManager.EXTRA_APPWIDGET_ID);
            if (widgetIds == null) {
                return null;
            }

            // Find the existing widget or create it.
            return DateViews.getOrCreateViews(this.getApplicationContext(), widgetIds);
        }

        @Override
        public int onStartCommand(Intent intent, int flags, int startId) {
            if (intent != null && intent.getData() != null) {
                // DateViews creates intents, so it knows how to handle them.
                DateViews.processIntent(this, intent);
            }
            return Service.START_NOT_STICKY;
        }

    }

    public static class CommonService extends RemoteViewsService {

        @Override
        public RemoteViewsFactory onGetViewFactory(Intent intent) {
            int[] widgetIds = intent == null ? null : intent
                    .getIntArrayExtra(AppWidgetManager.EXTRA_APPWIDGET_ID);
            if (widgetIds == null) {
                return null;
            }

            // Find the existing widget or create it.
            return CommonViews.getOrCreateViews(this.getApplicationContext(), widgetIds);
        }

        @Override
        public int onStartCommand(Intent intent, int flags, int startId) {
            if (intent != null && intent.getData() != null) {
                // CommonViews creates intents, so it knows how to handle them.
                CommonViews.processIntent(this, intent);
            }
            return Service.START_NOT_STICKY;
        }

    }

    public static class WeekNumberService extends RemoteViewsService {

        @Override
        public RemoteViewsFactory onGetViewFactory(Intent intent) {
            int[] widgetIds = intent == null ? null : intent
                    .getIntArrayExtra(AppWidgetManager.EXTRA_APPWIDGET_ID);
            if (widgetIds == null) {
                return null;
            }

            // Find the existing widget or create it.
            return WeekNumberViews.getOrCreateViews(this.getApplicationContext(), widgetIds);
        }

    }

}
