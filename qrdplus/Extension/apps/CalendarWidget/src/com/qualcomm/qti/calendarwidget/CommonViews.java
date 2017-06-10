/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.OnAccountsUpdateListener;
import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SyncInfo;
import android.content.SyncStatusObserver;
import android.content.res.Resources;
import android.net.Uri;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Events;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.Log;
import android.view.View;
import android.widget.RemoteViews;
import android.widget.RemoteViewsService;
import android.widget.Toast;

import com.qualcomm.qti.calendarwidget.CalendarWidget.CommonService;

import java.util.Calendar;
import java.util.List;

/**
 * This class is implements {@link RemoteViewsService.RemoteViewsFactory}, and
 * it provide the views for {@link CalendarWidget.CommonService}. It will
 * provide the weeks' view according to the position {@link #getViewAt(int)},
 * and bind the common views as added event button and so on.
 */
public class CommonViews implements RemoteViewsService.RemoteViewsFactory,
        SyncStatusObserver, OnAccountsUpdateListener {
    private static final String TAG = "CommonViews";

    private static CommonViews sInstance;

    private Context mContext;
    private AppWidgetManager mAppWidgetManager;
    private int[] mAppWidgetIds;
    private Resources mRes;

    private int mYear;
    private int mMonth;
    private RemoteViews mViews;

    private Object mObserverHandle = null;

    /**
     * We use the following convention for our add event commands:
     * widget://command/add_event
     */
    private static final String MIME_TYPE = "com.qualcomm.qti.calendarwidget/widget_data/add_event";
    private static final String ADD_EVENT = "add_event";
    private static final Uri COMMAND_URI = Uri
            .parse("widget://command/add_event");

    public CommonViews(Context context, int[] appWidgetIds) {
        sInstance = this;
        mContext = context;
        mAppWidgetManager = AppWidgetManager.getInstance(mContext);
        mAppWidgetIds = appWidgetIds;
        mRes = context.getResources();
    }

    public static synchronized CommonViews getOrCreateViews(Context context,
            int[] appWidgetIds) {
        if (sInstance != null) {
            return sInstance;
        } else {
            return new CommonViews(context, appWidgetIds);
        }
    }

    public static void update(Context context, int[] appWidgetIds,
            RemoteViews views, int year, int month) {
        CommonViews cv = getOrCreateViews(context, appWidgetIds);
        cv.start(views, appWidgetIds, year, month);
    }

    /**
     * To handle the intent which is created by this. And this will called by
     * {@link CalendarWidget.CommonService}.
     */
    public static boolean processIntent(Context context, Intent intent) {
        final Uri data = intent.getData();
        if (data == null)
            return false;

        List<String> pathSegments = data.getPathSegments();
        String command = pathSegments.get(0);
        if (ADD_EVENT.equals(command)) {
            try {
                Intent addEvent = new Intent(Intent.ACTION_EDIT);
                addEvent.setClassName(Utility.CALENDAR_PACKAGE_NAME,
                        Utility.CALENDAR_EDITEVENTACTIVITY);
                addEvent.setData(Events.CONTENT_URI);
                addEvent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(addEvent);
                return true;
            } catch (ActivityNotFoundException e) {
                Log.w(TAG, "Catch the ActivityNotFoundException, return true.");
                if (!Utility.isCalendarEnabled(context)) {
                    Toast.makeText(context, R.string.no_target_activity,
                            Toast.LENGTH_LONG).show();
                }
                return true;
            }
        }
        return false;
    }

    /**
     * Start to update the views.
     */
    private void start(RemoteViews views, int[] appWidgetIds, int year,
            int month) {
        mViews = views;
        mAppWidgetIds = appWidgetIds;
        mYear = year;
        mMonth = month;

        // update the views.
        updateHeader(false);
        updateWeekTitle();
    }

    /**
     * Update the common views, such as goto_today, choose_date, add_event and
     * request_sync. And update the intent for these views.
     *
     * @param syncActive
     *            the sync status, true if the sync for calendar authority is
     *            active.
     */
    private void updateHeader(boolean syncActive) {
        if (mViews == null)
            return;

        // Set the goto today intent. And we will deal with this command in
        // CalendarWidget.
        Intent gotoToday = new Intent(Utility.COMMAND_GOTO_TODAY);
        mViews.setOnClickPendingIntent(R.id.go_to_today, PendingIntent
                .getBroadcast(mContext, 0, gotoToday,
                        PendingIntent.FLAG_CANCEL_CURRENT));

        // Set the date title intent. And we want to start the activity.
        Intent chooseDate = new Intent();
        chooseDate.setClass(mContext, ChooseDateActivity.class);
        chooseDate.putExtra(ChooseDateActivity.EXTRA_YEAR, mYear);
        chooseDate.putExtra(ChooseDateActivity.EXTRA_MONTH, mMonth);
        mViews.setOnClickPendingIntent(R.id.choose_date, PendingIntent
                .getActivity(mContext, 0, chooseDate,
                        PendingIntent.FLAG_UPDATE_CURRENT));

        // Update the date title text.
        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(Calendar.YEAR, mYear);
        calendar.set(Calendar.MONTH, mMonth);
        calendar.set(Calendar.DAY_OF_MONTH,
                calendar.getActualMinimum(Calendar.DAY_OF_MONTH));
        String dateTitle = DateUtils.formatDateTime(
                mContext,
                calendar.getTimeInMillis(),
                DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_NO_MONTH_DAY
                        | DateUtils.FORMAT_SHOW_YEAR
                        | DateUtils.FORMAT_ABBREV_MONTH).toString();
        mViews.setTextViewText(R.id.choose_date, dateTitle);

        // set today
        Time today = new Time();
        today.setToNow();
        if (today.monthDay < 10) {
            mViews.setTextViewText(R.id.today, " " + today.monthDay);
        } else {
            mViews.setTextViewText(R.id.today, String.valueOf(today.monthDay));
        }

        // Set the add event intent. And we want to go to calendar.
        Intent addEvent = new Intent();
        addEvent.setClass(mContext, CommonService.class);
        addEvent.setDataAndType(COMMAND_URI, MIME_TYPE);
        mViews.setOnClickPendingIntent(R.id.add_event, PendingIntent
                .getService(mContext, 0, addEvent,
                        PendingIntent.FLAG_UPDATE_CURRENT));

        // Set the refresh intent. And we will try to start sync.
        if (syncActive) {
            mViews.setViewVisibility(R.id.refreshing, View.VISIBLE);
            mViews.setViewVisibility(R.id.refresh, View.INVISIBLE);
        } else {
            mViews.setViewVisibility(R.id.refresh, View.VISIBLE);
            mViews.setViewVisibility(R.id.refreshing, View.INVISIBLE);
        }
        Intent refresh = new Intent(Utility.COMMAND_REFRESH);
        // Because the flag PendingIntent.FLAG_CANCEL_CURRENT will cancel the
        // PendingIntent that created by another widget, so when add more than
        // one widget, only the PendingIntent that created by the last added
        // widget is active, so when click the refresh button on another widget
        // will start a canceled PendingIntent. So use the flag
        // FLAG_UPDATE_CURRENT
        // instead of FLAG_CANCEL_CURRENT
        mViews.setOnClickPendingIntent(R.id.refresh, PendingIntent
                .getBroadcast(mContext, 0, refresh,
                        PendingIntent.FLAG_UPDATE_CURRENT));
    }

    private void updateWeekTitle() {
        if (mViews == null)
            return;

        mAppWidgetManager.notifyAppWidgetViewDataChanged(mAppWidgetIds,
                R.id.week_name);

        Intent intent = new Intent(mContext, CommonService.class);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetIds);
        intent.setData(Uri.parse(intent.toUri(Intent.URI_INTENT_SCHEME)));
        mViews.setRemoteAdapter(R.id.week_name, intent);
    }

    @Override
    public RemoteViews getViewAt(int position) {
        RemoteViews views = new RemoteViews(mContext.getPackageName(),
                R.layout.week_name);

        int index = (position + Utility.FIRST_DAY_OF_WEEK) % 7;
        String[] weeks = mRes.getStringArray(R.array.weeks);
        views.setTextViewText(R.id.week, weeks[index]);

        if (Utility.DEBUG) {
            Log.d(TAG, "get the view at: " + position + ", the week is: "
                    + weeks[index]);
        }
        return views;
    }

    @Override
    public void onCreate() {
        /**
         * We want to monitor all the type of status changed, so we must set the
         * mask as {@link ContentResolver#SYNC_OBSERVER_TYPE_ALL}. But it is
         * hide, so we set the mask value same as the
         * {@link ContentResolver#SYNC_OBSERVER_TYPE_ALL}. Or we could set this
         * application as platform.
         */
        mObserverHandle = ContentResolver.addStatusChangeListener(
                0x7fffffff/* SYNC_OBSERVER_TYPE_ALL */, this);
        AccountManager.get(mContext).addOnAccountsUpdatedListener(this, null,
                true);
    }

    @Override
    public void onDataSetChanged() {
        // We are doing nothing in onDataSetChanged()
    }

    @Override
    public void onDestroy() {
        if (mObserverHandle != null) {
            ContentResolver.removeStatusChangeListener(mObserverHandle);
        }
        AccountManager.get(mContext).removeOnAccountsUpdatedListener(this);
    }

    @Override
    public int getCount() {
        // Caused by there are 7 days in one week. So we will return 7.
        return 7;
    }

    @Override
    public RemoteViews getLoadingView() {
        RemoteViews views = new RemoteViews(mContext.getPackageName(),
                R.layout.week_name);
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

    @Override
    public void onStatusChanged(int which) {
        if (Utility.DEBUG) {
            Log.i(TAG, "Handle the status changed, which:" + which);
        }

        if (mAppWidgetIds == null || mViews == null)
            return;

        if (which == ContentResolver.SYNC_OBSERVER_TYPE_ACTIVE) {
            List<SyncInfo> syncs = ContentResolver.getCurrentSyncs();
            for (SyncInfo sync : syncs) {
                if (Utility.DEBUG) {
                    Log.d(TAG, "sync.authority: " + sync.authority);
                    Log.d(TAG, "sync.account: " + sync.account);
                }
                if (CalendarContract.CONTENT_URI.getAuthority().equals(
                        sync.authority)) {
                    updateHeader(true);
                    mAppWidgetManager.updateAppWidget(mAppWidgetIds, mViews);
                }
            }
        } else {
            updateHeader(false);
            mAppWidgetManager.updateAppWidget(mAppWidgetIds, mViews);
        }
    }

    @Override
    public void onAccountsUpdated(Account[] accounts) {
        if (Utility.DEBUG) {
            Log.i(TAG, "Catch the account update message.");
        }
        if (mAppWidgetIds == null || mViews == null)
            return;

        if (Utility.getAccountCount(mContext) < 1) {
            updateHeader(false);
            mAppWidgetManager.updateAppWidget(mAppWidgetIds, mViews);
        }
    }

}
