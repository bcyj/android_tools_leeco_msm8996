/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.content.ContentUris;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.database.CursorWrapper;
import android.database.MatrixCursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Instances;
import android.text.TextUtils;
import android.text.format.Time;
import android.util.Log;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;

/**
 * We use this to load the event cursor.
 */
public class EventLoader extends CursorLoader {
    private static final String TAG = "EventLoader";

    private static final String[] COLS = new String[] { "monthDay",
            "eventCount", "firstEventTitle" };

    private Context mContext;

    private int mYear = -1;
    private int mMonth = -1;

    public EventLoader(Context context) {
        super(context);
        mContext = context;
    }

    public void load(Time time) {
        if (Utility.DEBUG) {
            Log.i(TAG, "Start to load cursor, and the time is:" + time.year
                    + "-" + time.month);
        }
        reset();
        mYear = time.year;
        mMonth = time.month;
        startLoading();
    }

    /**
     * We will use the {@link EventModel} to load the events info, and build the
     * {@link EventCursor} with the {@link #COLS}.
     */
    @Override
    public Cursor loadInBackground() {
        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(Calendar.YEAR, mYear);
        calendar.set(Calendar.MONTH, mMonth);

        int startDay = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
        int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);

        // use the EventModel to load the events info.
        EventModel model = new EventModel(mContext, this);
        HashMap<String, ArrayList<String>> eventsMap = model.getEventsMap();
        if (eventsMap == null || eventsMap.size() < 1) {
            return null;
        }

        // build the EventCursor.
        MatrixCursor matrixCursor = new MatrixCursor(COLS);
        for (int i = startDay; i <= endDay; i++) {
            ArrayList<Object> row = new ArrayList<Object>(COLS.length);
            row.add(i); // add the month day

            ArrayList<String> events = eventsMap.get(String.valueOf(i));
            if (events == null || events.size() < 1) {
                // no event
                row.add(0); // add the event count
                row.add(null); // add the first event title
            } else {
                row.add(events.size()); // add the event count
                row.add(events.get(0)); // add the event title
            }
            matrixCursor.addRow(row);
        }

        return new EventCursor(matrixCursor, this);
    }

    /**
     * Cursor data specifically for used by the {@link DateViews}. Contains a
     * cursor of event in addition to the event count and first event title. For
     * this, we could easily get the values we want.
     */
    static class EventCursor extends CursorWrapper {
        private Cursor mCursor;
        private EventLoader mLoader;

        public EventCursor(Cursor cursor, EventLoader loader) {
            super(cursor);
            mCursor = cursor;
            mLoader = loader;
        }

        /**
         * Get the year for this cursor.
         *
         * @return the year number
         */
        public int getYear() {
            return mLoader.mYear;
        }

        /**
         * Get the month for this cursor.
         *
         * @return the month number
         */
        public int getMonth() {
            return mLoader.mMonth;
        }

        /**
         * Get the day of month for the special position.
         *
         * @return the day number
         */
        public int getDayOfMonth() {
            return mCursor.getInt(0);
        }

        /**
         * Get the event count for the special position.
         *
         * @return the event count. If there is not event, return 0;
         */
        public int getEventCount() {
            return mCursor.getInt(1);
        }

        /**
         * Get the first event title fot the special position.
         *
         * @return the first event title. If there is not event, return null.
         */
        public String getFirstEventTitle() {
            return mCursor.getString(2);
        }
    }

    /**
     * We used this to load the events info. We will get the info from the
     * {@link CalendarContract.Instances}, and sort the result by
     * {@link CalendarContract.Instances#BEGIN} ascending.
     */
    static class EventModel {
        private Context mContext;
        private EventLoader mLoader;

        private static final String SORT_BY_BEGIN_ASC = Instances.BEGIN
                + " ASC";

        private static final String[] PROJECTION = new String[] {
                Instances.TITLE, Instances.BEGIN, Instances.END,
                Instances.EVENT_TIMEZONE, Instances.ALL_DAY };
        private static final int INDEX_TITLE = 0;
        private static final int INDEX_BEGIN = 1;
        private static final int INDEX_END = 2;
        private static final int INDEX_TIMEZONE = 3;
        private static final int INDEX_ALLDAY = 4;

        private final Uri mUri;

        public EventModel(Context context, EventLoader loader) {
            mContext = context;
            mLoader = loader;
            mUri = buildUri();
        }

        /**
         * Return the events map, the key is the date, and the value is the
         * events' list.
         *
         * @return the events map
         */
        public HashMap<String, ArrayList<String>> getEventsMap() {
            HashMap<String, ArrayList<String>> eventsMap = new HashMap<String, ArrayList<String>>();

            Calendar calendar = Calendar.getInstance();
            calendar.clear();
            calendar.set(Calendar.YEAR, mLoader.mYear);
            calendar.set(Calendar.MONTH, mLoader.mMonth);
            int startDay = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
            int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);

            // get the cursor from the database.
            Cursor cursor = mContext.getContentResolver().query(mUri,
                    PROJECTION, null, null, SORT_BY_BEGIN_ASC);
            try {
                if (cursor == null || cursor.getCount() < 1) {
                    Log.w(TAG,
                            "get the event cursor is null or it's count is 0");
                    return eventsMap;
                }

                while (cursor.moveToNext()) {
                    String title = cursor.getString(INDEX_TITLE);
                    long start = cursor.getLong(INDEX_BEGIN) + 1;
                    long end = cursor.getLong(INDEX_END) - 1;
                    String timezone = cursor.getString(INDEX_TIMEZONE);
                    boolean allDay = cursor.getInt(INDEX_ALLDAY) == 1;

                    // Get the start millisecond and end millisecond for the
                    // event's TimeZone.
                    Time startTime = new Time();
                    startTime.clear(timezone);
                    startTime.set(startDay, mLoader.mMonth, mLoader.mYear);
                    long startMillis = startTime.toMillis(true);

                    Time endTime = new Time();
                    endTime.clear(timezone);
                    endTime.set(endDay + 1, mLoader.mMonth, mLoader.mYear);
                    long endMillis = endTime.toMillis(true) - 1;

                    // If end time of this event instance is less than start
                    // time of current month, meaning this event does not
                    // happened in current month, so don't count this event to
                    // current month
                    if (end < startMillis || start > endMillis) {
                        continue;
                    }

                    int eventStartDay = startDay;
                    // get the correct eventStartDay.
                    if (start >= startMillis) {
                        Time time = new Time();
                        // use the current TimeZone to get the month day
                        time.clear(calendar.getTimeZone().getID());
                        // Allday event will always use UTC time zone, so we need format
                        // time use its timezone
                        if (allDay && !TextUtils.isEmpty(timezone)) {
                            time.clear(timezone);
                        }
                        time.set(start);
                        eventStartDay = time.monthDay;
                    }

                    int eventEndDay = endDay;
                    // get the correct eventEventDay.
                    if (end > 0 && end <= endMillis) {
                        Time time = new Time();
                        // use the current TimeZone to get the month day
                        time.clear(calendar.getTimeZone().getID());
                        // Allday event will always use UTC time zone, so we need format
                        // time use its timezone
                        if (allDay && !TextUtils.isEmpty(timezone)) {
                            time.clear(timezone);
                        }
                        time.set(end);
                        eventEndDay = time.monthDay;
                    }

                    // set the event info to the map.
                    for (int i = eventStartDay; i <= eventEndDay; i++) {
                        ArrayList<String> events = eventsMap.get(String
                                .valueOf(i));
                        if (events == null) {
                            events = new ArrayList<String>();
                            eventsMap.put(String.valueOf(i), events);
                        }
                        events.add(title);
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
            return eventsMap;
        }

        /**
         * Build the uri for query.
         */
        private Uri buildUri() {
            Calendar calendar = Calendar.getInstance();
            calendar.clear();
            calendar.set(Calendar.YEAR, mLoader.mYear);
            calendar.set(Calendar.MONTH, mLoader.mMonth);

            int startDay = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
            int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);

            Time startTime = new Time();
            startTime.set(startDay, mLoader.mMonth, mLoader.mYear);
            long startMillis = startTime.toMillis(true);

            Time endTime = new Time();
            endTime.set(endDay + 1, mLoader.mMonth, mLoader.mYear);
            long endMillis = endTime.toMillis(true) - 1;

            Uri.Builder builder = Instances.CONTENT_URI.buildUpon();
            ContentUris.appendId(builder, startMillis);
            ContentUris.appendId(builder, endMillis);
            return builder.build();
        }
    }

}
