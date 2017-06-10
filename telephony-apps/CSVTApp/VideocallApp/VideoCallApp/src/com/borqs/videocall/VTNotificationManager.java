/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 *
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.borqs.videocall;

import com.android.internal.telephony.CallerInfo;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.StatusBarManager;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.SystemClock;
import android.provider.CallLog.Calls;
import android.provider.Contacts.Phones;
import android.text.TextUtils;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;

public class VTNotificationManager {

	private static String TAG = "VTNotificationMg";

	// notification types
	static final int VT_MISSED_CALL_NOTIFICATION = 6111;
	static final int VT_IN_CALL_NOTIFICATION = 6112;

	private QueryHandler mQueryHandler = null;
	private static final int CALL_LOG_TOKEN = -1;
	private static final int CONTACT_TOKEN = -2;

	private static VTNotificationManager sMe = null;

	private Context mContext;

	private NotificationManager mNotificationMgr;
	private StatusBarManager mStatusBar;
	private VideoCallApp mApp;
	// used to track the missed VT call counter, default to 0.
	private int mNumberMissedCalls = 0;

	private static final String[] CALL_LOG_PROJECTION = new String[] {
			Calls._ID, Calls.NUMBER, Calls.DATE, Calls.DURATION, Calls.TYPE,
			/* SAM: Calls.CALLTYPE*/ };

	/** The projection to use when querying the phones table */
	static final String[] PHONES_PROJECTION = new String[] { Phones.NUMBER,
			Phones.NAME };

	public VTNotificationManager(Context context) {
		// TODO Auto-generated constructor stub
		mContext = context;
		mApp = VideoCallApp.getInstance();
		mNotificationMgr = (NotificationManager) context
				.getSystemService(Context.NOTIFICATION_SERVICE);
		mStatusBar = (StatusBarManager) context.getSystemService(Context.STATUS_BAR_SERVICE);
	}

	static void init(Context context) {
		if (sMe == null) {
			sMe = new VTNotificationManager(context);
		}

		// update the notifications that need to be touched at startup.
		//sMe.updateNotifications();
	}

	void notifyInVTCall(VTService service) {
		if (MyLog.DEBUG) MyLog.d(TAG, "nofifyInVTCall");
		if(service == null)
			return;

		/*Notification notification = new Notification(
				R.drawable.stat_notify_calling,null, System
						.currentTimeMillis());
		*/
		Notification notification = new Notification();
		// The PendingIntent to launch our activity if the user selects this
		// notification
		Intent intent = mApp.constuctRestoreIntent();

		PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0,
				intent, 0);
		notification.icon = R.drawable.stat_notify_calling;
		notification.contentIntent =contentIntent;
        notification.flags |= Notification.FLAG_ONGOING_EVENT;

         int notificationResId = R.layout.ongoing_call_notification;
         if(/*!mContext.isAndroidTheme()*/false){
                 notificationResId = R.layout.art_ongoing_call_notification;
         }

        RemoteViews contentView = new RemoteViews(mContext.getPackageName(),
                notificationResId);
        contentView.setImageViewResource(R.id.icon, R.drawable.stat_notify_calling);

        long chronometerBaseTime = SystemClock.elapsedRealtime();
        boolean bDisplayTime = false;
        if (mApp.mIsVTConnected ) {
            long callDurationMsec = mApp.mCallTime.getCallDuration();
            if (MyLog.DEBUG)Log.d(TAG,"callDurationMsec:"+callDurationMsec);
            if(callDurationMsec > 0) {
		bDisplayTime = true;
                chronometerBaseTime = SystemClock.elapsedRealtime() - callDurationMsec*1000;
            }
            else
            {
		//for multimedia ring,if multimedia ring start,but remote do not answer,return.
		return;
            }
        }

        if (MyLog.DEBUG)Log.d(TAG,"chronometerBaseTime:"+chronometerBaseTime);
        String expandedViewLine1 = mContext.getString(R.string.notification_current_call);
        if (bDisplayTime){
            expandedViewLine1 = mContext.getString(R.string.notification_ongoing_call_format);
        }
        contentView.setChronometer(R.id.txtchrmter,
                chronometerBaseTime,
                expandedViewLine1,
                bDisplayTime);

        String expandedViewLine2 = "";
		CallerInfo mCallerInfo = mApp.getCallerInfo();
        if(mCallerInfo != null)
        {
		expandedViewLine2 =mCallerInfo.name;
		if (MyLog.DEBUG) Log.d(TAG,"chronometerBaseTime-callInfo Name:"+expandedViewLine2);
		if(expandedViewLine2== null || expandedViewLine2.equals(""))
            {expandedViewLine2 = mCallerInfo.phoneNumber;}
        }
        // Display the caller info in the correct view.
        contentView.setTextViewText(R.id.txtname, expandedViewLine2);
        notification.contentView = contentView;
        mNotificationMgr.notify(VT_IN_CALL_NOTIFICATION, notification);
        service.startForeground(VT_IN_CALL_NOTIFICATION,notification);

	}

	void closeInCallNotification() {
		if( mNotificationMgr != null){
			if (MyLog.DEBUG) MyLog.d(TAG, "close Notification.");
			try {
				mNotificationMgr.cancel(VT_IN_CALL_NOTIFICATION);
				/*Intent intent = new Intent("CallNotifier.CallEnd");
				if(mApp!=null)
				{
					mApp.sendBroadcast(intent);
					if (MyLog.DEBUG) MyLog.d(TAG, "Notification home end call.");
				}*/
			} catch (Exception e) {
				// TODO: handle exception
				Log.e(TAG, "cancel in call notification error: " + e);
			}
		}
	}

	void closeMissedCallNotification(boolean updateCallLog) {
		if (updateCallLog) {
			clearMissedVTCallLog();
		}

		if( mNotificationMgr != null){
			if (MyLog.DEBUG) MyLog.d(TAG, "close Notification.");
			try {
			    mNumberMissedCalls = 0;
				mNotificationMgr.cancel(VT_MISSED_CALL_NOTIFICATION);
			} catch (Exception e) {
				// TODO: handle exception
				Log.e(TAG, "cancel missed call notification error: " + e);
			}

		}
	}

	/**
	 * Displays a notification about a missed call.
	 *
	 * @param nameOrNumber
	 *            either the contact name, or the phone number if no contact
	 * @param label
	 *            the label of the number if nameOrNumber is a name, null if it
	 *            is a number
	 */
	void notifyVTMissedCall(String name, String number, String label, long date) {
		// title resource id
		if (MyLog.DEBUG) MyLog.d(TAG, "notifyVTMissedCall...name: " + name + " number: " + number
				+ " label: " + label + " date: " + date);
		int titleResId;
		// the text in the notification's line 1 and 2.
		String expandedText, callName;

		// increment number of missed calls.
		mNumberMissedCalls++;
		if (MyLog.DEBUG) MyLog.d(TAG, "mNumberMissedCalls: " + mNumberMissedCalls);

		// get the name for the ticker text
		// i.e. "Missed call from <caller name or number>"
		if (name != null && TextUtils.isGraphic(name)) {
			callName = name;
		} else if (!TextUtils.isEmpty(number)) {
			callName = number;
		} else {
			// use "unknown" if the caller is unidentifiable.
			callName = mContext.getString(R.string.NullName);
		}

		// display the first line of the notification:
		// 1 missed call: call name
		// more than 1 missed call: <number of calls> + "missed calls"
		if (mNumberMissedCalls == 1) {
			titleResId = R.string.notification_missedCallTitle;
			expandedText = callName;
		} else {
			titleResId = R.string.notification_missedCallsTitle;
			expandedText = "" + mNumberMissedCalls
					+ mContext.getString(R.string.notification_missedCallsMsg);
		}

	    // create intent entry for notification
        Intent intent = new Intent(Intent.ACTION_VIEW, null);
        intent.setType("vnd.android.cursor.dir/calls");

		Notification notification = new Notification(
				android.R.drawable.stat_notify_missed_call,// icon
				mContext.getString(R.string.notification_missedCallTicker)
						+ callName, date);
		notification.setLatestEventInfo(mContext, // context
				mContext.getText(titleResId), // expandedTitle
				expandedText, // expandedText
				PendingIntent.getActivity(mContext, 0, intent, 0)// content
				// intent
				);
		notification.flags |= Notification.FLAG_AUTO_CANCEL;
		notification.deleteIntent = createClearMissedVTCallsIntent();

		if (MyLog.DEBUG) MyLog.d(TAG, "set notification");
		mNotificationMgr.notify(VT_MISSED_CALL_NOTIFICATION, notification);
	}

	void prepareToNotifyVTMissedCall(String name, String number, String label, long date) {
        Intent intent = new Intent ("com.borqs.videocall.action.NotifyMissedCall");
        intent.putExtra("name", name);
        intent.putExtra("number", number);
        intent.putExtra("label", label);
        intent.putExtra("date", date);
        mContext.sendBroadcast(intent);
		notifyVTMissedCall(name, number, label, date);
	}

	private PendingIntent createClearMissedVTCallsIntent() {
		Intent intent = new Intent(mContext, VTCallReceiver.class);
		intent.setAction(VideoCallApp.INTENT_ACTION_CLEAR_MISSED_VTCALL);
		return PendingIntent.getBroadcast(mContext, 0, intent, 0);
	}

	private void clearMissedVTCallLog() {
		ContentValues values = new ContentValues();
		values.put(Calls.NEW, 0);
		values.put(Calls.IS_READ, 1);
		StringBuilder where = new StringBuilder();
		where.append(Calls.NEW);
		where.append(" = 1 AND ");
		where.append(Calls.TYPE);
		where.append(" = ?");
		mContext.getContentResolver().update(Calls.CONTENT_URI, values, where.toString(),
		new String[]{ Integer.toString(Calls.MISSED_TYPE) });
	}

	static VTNotificationManager getDefault() {
		return sMe;
	}

	/**
	 * Makes sure notifications are up to date.
	 */
    /*
	void updateNotifications() {
		if (MyLog.DEBUG) MyLog.d(TAG, "updateNotifications...");

		mNumberMissedCalls = 0;

		// instantiate query handler
		mQueryHandler = new QueryHandler(mContext.getContentResolver());

		// setup query spec, look for all Missed calls that are new.
		StringBuilder where = new StringBuilder("type=");
		where.append(Calls.MISSED_TYPE);
		where.append(" AND call_type=");
		where.append(VideoCallApp.VT_CALLLOG_CALLTYPE);
		where.append(" AND new=1");

		// start the query
		mQueryHandler.startQuery(CALL_LOG_TOKEN, null, Calls.CONTENT_URI,
				CALL_LOG_PROJECTION, where.toString(), null,
				Calls.DEFAULT_SORT_ORDER);

		// Depend on android.app.StatusBarManager to be set to
		// disable(DISABLE_NONE) upon startup. This will be the
		// case even if the phone app crashes.
	}
    */
	/**
	 * Class used to run asynchronous queries to re-populate the notifications
	 * we care about.
	 */
	private class QueryHandler extends AsyncQueryHandler {

		/**
		 * Used to store relevant fields for the Missed Call notifications.
		 */
		private class NotificationInfo {
			public String name;
			public String number;
			public String label;
			public long date;
		}

		public QueryHandler(ContentResolver cr) {
			super(cr);
		}

		/**
		 * Handles the query results. There are really 2 steps to this, similar
		 * to what happens in RecentCallsListActivity. 1. Find the list of
		 * missed calls 2. For each call, run a query to retrieve the caller's
		 * name.
		 */
		@Override
		protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
			// TODO: it would be faster to use a join here, but for the purposes
			// of this small record set, it should be ok.

			// Note that CursorJoiner is not useable here because the number
			// comparisons are not strictly equals; the comparisons happen in
			// the SQL function PHONE_NUMBERS_EQUAL, which is not available for
			// the CursorJoiner.

			// Executing our own query is also feasible (with a join), but that
			// will require some work (possibly destabilizing) in Contacts
			// Provider.

			// At this point, we will execute subqueries on each row just as
			// RecentCallsListActivity.java does.
			switch (token) {
			case CALL_LOG_TOKEN:
				if (MyLog.DEBUG) MyLog.d(TAG, "call log query complete.");

				// initial call to retrieve the call list.
				if (cursor != null) {
					while (cursor.moveToNext()) {
						// for each call in the call log list, create
						// the notification object and query contacts
						NotificationInfo n = getNotificationInfo(cursor);

						if (MyLog.DEBUG) MyLog.d(TAG, "query contacts with number: " + n.number);

						mQueryHandler.startQuery(CONTACT_TOKEN, n, Uri
								.withAppendedPath(Phones.CONTENT_FILTER_URL,
										n.number), PHONES_PROJECTION, null,
								null, Phones.DEFAULT_SORT_ORDER);
					}

					if (MyLog.DEBUG) MyLog.d(TAG, "close call log cursor");
					cursor.close();
				}
				break;
			case CONTACT_TOKEN:
				if (MyLog.DEBUG) MyLog.d(TAG, "contact query complete.");

				// subqueries to get the caller name.
				if ((cursor != null) && (cookie != null)) {
					NotificationInfo n = (NotificationInfo) cookie;

					if (cursor.moveToFirst()) {
						// we have contacts data, get the name.
						if (MyLog.DEBUG) MyLog.d(TAG, "contact :" + n.name + " found for phone: "
								+ n.number);
						n.name = cursor.getString(cursor
								.getColumnIndexOrThrow(Phones.NAME));
					}

					// send the notification
					if (MyLog.DEBUG) MyLog.d(TAG, "sending notification.");
					notifyVTMissedCall(n.name, n.number, n.label, n.date);

					if (MyLog.DEBUG) MyLog.d(TAG, "closing contact cursor.");
					cursor.close();
				}
				if (cookie == null)
				{
					if (cursor != null)
					cursor.close();
				}
				break;
			default:
				if (cursor != null)
				cursor.close();
				break;
			}
		}


		/**
		 * Factory method to generate a NotificationInfo object given a cursor
		 * from the call log table.
		 */
		private final NotificationInfo getNotificationInfo(Cursor cursor) {
			NotificationInfo n = new NotificationInfo();
			n.name = null;
			n.number = cursor.getString(cursor
					.getColumnIndexOrThrow(Calls.NUMBER));
			n.label = cursor
					.getString(cursor.getColumnIndexOrThrow(Calls.TYPE));
			n.date = cursor.getLong(cursor.getColumnIndexOrThrow(Calls.DATE));

			// make sure we update the number depending upon saved values in
			// CallLog.addCall(). If either special values for unknown or
			// private number are detected, we need to hand off the message
			// to the missed call notification.
			if (MyLog.DEBUG) MyLog.d(TAG, "NotificationInfo constructed for number: " + n.number);
			return n;
		}
	}

	/**
     * StatusBarMgr implementation
     */
    class StatusBarMgr {
        // current settings
        private boolean mIsNotificationEnabled = true;
        private boolean mIsExpandedViewEnabled = true;

        private StatusBarMgr () {
        }

        /**
         * Sets the notification state (enable / disable
         * vibrating notifications) for the status bar,
         * updates the status bar service if there is a change.
         * Independent of the remaining Status Bar
         * functionality, including icons and expanded view.
         */
        void enableNotificationAlerts(boolean enable) {
			if (MyLog.DEBUG) MyLog.d(TAG, "enableNotificationAlerts: " + mIsNotificationEnabled+" , enable="+enable);
            if (mIsNotificationEnabled != enable) {
                mIsNotificationEnabled = enable;
                updateStatusBar();
            }
        }

        /**
         * Sets the ability to expand the notifications for the
         * status bar, updates the status bar service if there
         * is a change. Independent of the remaining Status Bar
         * functionality, including icons and notification
         * alerts.
         */
        void enableExpandedView(boolean enable) {
            if (mIsExpandedViewEnabled != enable) {
                mIsExpandedViewEnabled = enable;
                updateStatusBar();
            }
        }

        /**
         * Method to synchronize status bar state with our current
         * state.
         */
        void updateStatusBar() {
            int state = StatusBarManager.DISABLE_NONE;

            if (!mIsExpandedViewEnabled) {
                state |= StatusBarManager.DISABLE_EXPAND;
            }

            if (!mIsNotificationEnabled) {
                state |= StatusBarManager.DISABLE_NOTIFICATION_ALERTS;
                //state |= StatusBarManager.NOTIFICATION_ALERTS_MINI;
            }

			if (MyLog.DEBUG) MyLog.d(TAG, "update status bar state="+ state );
            // send the message to the status bar manager.
            mStatusBar.disable(state);
        }
    }

    /**
     * Factory method
     */
    private StatusBarMgr mStatusBarMgr;
    StatusBarMgr getStatusBarMgr() {
        if (mStatusBarMgr == null) {
            mStatusBarMgr = new StatusBarMgr();
        }
        return mStatusBarMgr;
    }

}
