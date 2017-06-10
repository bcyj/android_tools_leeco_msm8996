/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarwidget;

import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.database.CursorWrapper;
import android.net.Uri;
import android.text.TextUtils;
import android.text.format.Time;
import android.util.Log;

/**
 * We use this to load the lunar cursor.
 */
public class LunarLoader extends CursorLoader {
    private static final String TAG = "LunarLoader";

    // The uri and params will be used to get the lunar info for one month.
    private static final Uri CONTENT_URI_GET_ONE_MONTH =
            Uri.parse("content://com.qualcomm.qti.lunarinfo/one_month");
    private static final String PARAM_YEAR = "year";
    private static final String PARAM_MONTH = "month";

    private int mYear = -1;
    private int mMonth = -1;

    private Context mContext;

    public LunarLoader(Context context) {
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
     * We will get the lunar info from the LunarService. And then build the
     * {@link LunarCursor}.
     */
    @Override
    public Cursor loadInBackground() {
        // Build the query uri.
        Uri queryUri = CONTENT_URI_GET_ONE_MONTH.buildUpon()
                .appendQueryParameter(PARAM_YEAR, String.valueOf(mYear))
                .appendQueryParameter(PARAM_MONTH, String.valueOf(mMonth))
                .build();

        Cursor cursor = mContext.getContentResolver().query(queryUri, null, null, null, null);
        if (cursor == null) return null;

        return new LunarCursor(cursor, this);
    }

    /**
     * Cursor data specifically for used by the {@link DateViews}. For this, we
     * could easily get the lunar info and if special.
     */
    static class LunarCursor extends CursorWrapper {
        private Cursor mCursor;
        private LunarLoader mLoader;

        private static final String COL_DAY = "day";
        private static final String COL_LUNAR_LABEL_SHORT = "lunar_label_short";
        private static final String COL_FESTIVAL_1 = "festival_1";
        private static final String COL_FESTIVAL_2 = "festival_2";

        private static int sColIndexDay = -1;
        private static int sColIndexLunarLabel = -1;
        private static int sColIndexFestival1 = -1;
        private static int sColIndexFestival2 = -1;

        public LunarCursor(Cursor cursor, LunarLoader loader) {
            super(cursor);
            mCursor = cursor;
            mLoader = loader;

            // Get the columns index.
            if (sColIndexDay < 0 && mCursor != null) {
                sColIndexDay = mCursor.getColumnIndexOrThrow(COL_DAY);
                sColIndexLunarLabel = mCursor.getColumnIndexOrThrow(COL_LUNAR_LABEL_SHORT);
                sColIndexFestival1 = mCursor.getColumnIndexOrThrow(COL_FESTIVAL_1);
                sColIndexFestival2 = mCursor.getColumnIndexOrThrow(COL_FESTIVAL_2);
            }
        }

        /**
         * Get the year for this cursor.
         */
        public int getYear() {
            return mLoader.mYear;
        }

        /**
         * Get the month for this cursor.
         */
        public int getMonth() {
            return mLoader.mMonth;
        }

        /**
         * Get the day of month for the special position.
         */
        public int getDayOfMonth() {
            return mCursor.getInt(sColIndexDay);
        }

        /**
         * Get the lunar info for the special position. If the day is festival, it will display
         * the festival info. If there are more than one festival info, the festival will display
         * end with "*".
         */
        public String getLunar() {
            String festival1 = mCursor.getString(sColIndexFestival1);
            if (TextUtils.isEmpty(festival1)) {
                String lunar = mCursor.getString(sColIndexLunarLabel);
                return lunar;
            } else {
                String festival2 = mCursor.getString(sColIndexFestival2);
                if (TextUtils.isEmpty(festival2)) {
                    return festival1;
                } else {
                    return festival1 + "*";
                }
            }
        }

        /**
         * If special for the special position.
         * @return true if it is festival.
         */
        public boolean isSpecial() {
            String festival1 = mCursor.getString(sColIndexFestival1);
            return !TextUtils.isEmpty(festival1);
        }
    }

}
