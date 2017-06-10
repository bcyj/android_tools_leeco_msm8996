/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.RemoteViewsService;

import com.qualcomm.qti.calendarwidget.CalendarWidget.WeekNumberService;

import java.util.Calendar;

/**
 * This class is implements {@link RemoteViewsService.RemoteViewsFactory}, and
 * it provide the views for {@link CalendarWidget.WeekNumberService}. It will
 * provide the weeks' number of the year according to the position
 * {@link #getViewAt(int)}.
 */
public class WeekNumberViews implements RemoteViewsService.RemoteViewsFactory {
    private static final String TAG = "WeekNumberViews";

    private static WeekNumberViews sInstance;

    private Context mContext;
    private AppWidgetManager mAppWidgetManager;
    private int[] mAppWidgetIds;

    private int mYear;
    private int mMonth;
    private RemoteViews mViews;

    private int[] mWeekNumbers;

    public WeekNumberViews(Context context, int[] appWidgetIds) {
        sInstance = this;
        mContext = context;
        mAppWidgetManager = AppWidgetManager.getInstance(mContext);
        mAppWidgetIds = appWidgetIds;
    }

    public static synchronized WeekNumberViews getOrCreateViews(
            Context context, int[] appWidgetIds) {
        if (sInstance != null) {
            return sInstance;
        } else {
            return new WeekNumberViews(context, appWidgetIds);
        }
    }

    public static void update(Context context, int[] appWidgetIds,
            RemoteViews views, int year, int month) {
        WeekNumberViews wnv = getOrCreateViews(context, appWidgetIds);
        wnv.start(views, appWidgetIds, year, month);
    }

    /**
     * Start to update the weeks' number of the year.
     */
    private void start(RemoteViews views, int[] appWidgetIds, int year,
            int month) {
        mViews = views;
        mAppWidgetIds = appWidgetIds;
        mYear = year;
        mMonth = month;

        updateWeekNumber();
    }

    private void updateWeekNumber() {
        if (mViews == null)
            return;

        initWeekNumber();
        mAppWidgetManager.notifyAppWidgetViewDataChanged(mAppWidgetIds,
                R.id.week_number);

        Intent intent = new Intent(mContext, WeekNumberService.class);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetIds);
        intent.setData(Uri.parse(intent.toUri(Intent.URI_INTENT_SCHEME)));
        mViews.setRemoteAdapter(R.id.week_number, intent);
    }

    private void initWeekNumber() {
        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(Calendar.YEAR, mYear);
        calendar.set(Calendar.MONTH, mMonth);
        calendar.setFirstDayOfWeek(Utility.FIRST_DAY_OF_WEEK);

        int startDay = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
        int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);

        calendar.set(Calendar.DAY_OF_MONTH, startDay);
        int startWeekNumber = calendar.get(Calendar.WEEK_OF_YEAR);
        calendar.set(Calendar.DAY_OF_MONTH, endDay);
        int endWeekNumber = calendar.get(Calendar.WEEK_OF_YEAR);

        int count = calendar.get(Calendar.WEEK_OF_MONTH);
        mWeekNumbers = new int[count];
        if (startWeekNumber > endWeekNumber) {
            int thisYearCount = count - endWeekNumber;
            for (int i = 0; i < thisYearCount; i++) {
                mWeekNumbers[i] = startWeekNumber + i;
            }
            for (int j = count - 1; j > thisYearCount - 1; j--) {
                mWeekNumbers[j] = endWeekNumber--;
            }
        } else {
            for (int i = 0; i < count; i++) {
                mWeekNumbers[i] = startWeekNumber + i;
            }
        }
    }

    @Override
    public RemoteViews getViewAt(int position) {
        if (mWeekNumbers == null) {
            initWeekNumber();
        }

        RemoteViews views = new RemoteViews(mContext.getPackageName(),
                R.layout.week_number);
        if (position < mWeekNumbers.length) {
            views.setTextViewText(R.id.week,
                    String.valueOf(mWeekNumbers[position]));

            if (Utility.DEBUG) {
                Log.d(TAG, "get the view at: " + position
                        + ", the week number is: " + mWeekNumbers[position]);
            }
        } else {
            views.setTextViewText(R.id.week, "");
        }
        return views;
    }

    @Override
    public void onCreate() {
    }

    @Override
    public void onDataSetChanged() {
    }

    @Override
    public void onDestroy() {
    }

    @Override
    public int getCount() {
        // Caused by we will always show 6 rows in date view.
        return Utility.DATE_VIEW_ROWS;
    }

    @Override
    public RemoteViews getLoadingView() {
        RemoteViews views = new RemoteViews(mContext.getPackageName(),
                R.layout.week_number);
        views.setTextViewText(R.id.week, null);
        return views;
    }

    @Override
    public int getViewTypeCount() {
        return 1;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public boolean hasStableIds() {
        return false;
    }

}
