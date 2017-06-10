/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.content.Intent;
import android.text.format.Time;
import android.util.Log;
import android.widget.RemoteViews;

/**
 * We used this class to manager the CalendarWidget. It will control the widgets
 * date, and update the views of the widgets for the date.
 */
public class WidgetManager {
    private static final String TAG = "WidgetManager";

    private static int sYear = -1;
    private static int sMonth = -1;

    /**
     * To update the views (contains {@link CommonViews},
     * {@link WeekNumberViews} and {@link DateViews}) for all the widgets at the
     * saved date.
     *
     * @param context
     *            the application context
     * @param appWidgetManager
     *            the app widget manager
     * @param appWidgetIds
     *            the ids of this widget
     */
    public static void updateWidgets(Context context,
            AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        if (Utility.DEBUG) {
            Log.i(TAG, "WidgetManager will update the widgets.");
        }
        if (sYear == -1 || sMonth == -1) {
            gotoToday();
        }

        RemoteViews views = new RemoteViews(context.getPackageName(),
                R.layout.widget);
        // the header views will be update with in WeekNameViews.
        CommonViews.update(context, appWidgetIds, views, sYear, sMonth);
        WeekNumberViews.update(context, appWidgetIds, views, sYear, sMonth);
        DateViews.update(context, appWidgetIds, views, sYear, sMonth);

        if (appWidgetManager != null) {
            appWidgetManager.updateAppWidget(appWidgetIds, views);
        }
    }

    /**
     * Set the date the widgets want to show. This will be called by
     * {@link ChooseDateActivity#onClick(android.view.View)}
     *
     * @param context
     *            the application context
     * @param year
     *            the year number which we want to changed as
     * @param month
     *            the month number which we want to changed as
     */
    public static void setDate(Context context, int year, int month) {
        if (sYear != year || sMonth != month) {
            sYear = year;
            sMonth = month;
            // we need notify calendar widget update the views.
            Intent intent = new Intent(Utility.COMMAND_DATE_CHANGED);
            context.sendBroadcast(intent);
        }
    }

    /**
     * Set the date as today.
     */
    public static void gotoToday() {
        Time time = new Time();
        time.setToNow();
        sYear = time.year;
        sMonth = time.month;
    }

    /**
     * @return true if saved date as today, otherwise false
     */
    public static boolean isToday() {
        Time time = new Time();
        time.setToNow();
        if (sYear == time.year && sMonth == time.month) {
            return true;
        } else {
            return false;
        }
    }
}
