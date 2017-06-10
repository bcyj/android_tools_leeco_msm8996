/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.lunarinfo;

import java.util.ArrayList;
import java.util.Calendar;

import android.content.Context;
import android.database.MatrixCursor;
import android.util.Log;

public class LunarInfo {
    private static final String TAG = "LunarInfo";

    // The columns for result.
    private static final String COL_ID = "_id";
    private static final String COL_YEAR = "year";
    private static final String COL_MONTH = "month";
    private static final String COL_DAY = "day";
    private static final String COL_LUNAR_YEAR = "lunar_year";
    private static final String COL_LUNAR_MONTH = "lunar_month";
    private static final String COL_LUNAR_DAY = "lunar_day";
    private static final String COL_LUNAR_LABEL_LONG = "lunar_label_long";
    private static final String COL_LUNAR_LABEL_SHORT = "lunar_label_short";
    private static final String COL_ANIMAL = "animal";
    private static final String COL_FESTIVAL_1 = "festival_1";
    private static final String COL_FESTIVAL_2 = "festival_2";
    private static final String COL_FESTIVAL_3 = "festival_3";
    private static final String COL_FESTIVAL_4 = "festival_4";

    private static final String[] PROJECTION = {
            COL_ID,
            COL_YEAR,
            COL_MONTH,
            COL_DAY,
            COL_LUNAR_YEAR,
            COL_LUNAR_MONTH,
            COL_LUNAR_DAY,
            COL_LUNAR_LABEL_LONG,
            COL_LUNAR_LABEL_SHORT,
            COL_ANIMAL,
            COL_FESTIVAL_1,
            COL_FESTIVAL_2,
            COL_FESTIVAL_3,
            COL_FESTIVAL_4
    };

    private static final int FESTIVAL_MAX_COUNT = 4;

    private static LunarInfo sInstance = null;

    private Context mContext = null;
    private DateInfo mDateInfo = null;

    public static LunarInfo getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new LunarInfo(context);
        }
        return sInstance;
    }

    private LunarInfo(Context context) {
        mContext = context;
    }

    /**
     * Used to get the cursor with columns {@link #PROJECTION} for one day.
     */
    public MatrixCursor buildCursor(int year, int month, int day) {
        MatrixCursor cursor = new MatrixCursor(PROJECTION);
        mDateInfo = LunarParser.parseLunarCalendar(mDateInfo, year, month, day);
        buildCursor(cursor);
        return cursor;
    }

    /**
     * Used to get the cursor with columns {@link #PROJECTION} for one month.
     */
    public MatrixCursor buildCursor(int year, int month) {
        Calendar calendar = Calendar.getInstance();
        calendar.set(Calendar.YEAR, year);
        calendar.set(Calendar.MONTH, month);
        int startDay = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
        int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);
        return buildCursor(year, month, startDay, year, month, endDay);
    }

    /**
     * Used to get the cursor with columns {@link #PROJECTION} for days as from-to.
     */
    public MatrixCursor buildCursor(int from_year, int from_month, int from_day,
            int to_year, int to_month, int to_day) {
        MatrixCursor cursor = new MatrixCursor(PROJECTION);

        Calendar start = Calendar.getInstance();
        start.set(Calendar.YEAR, from_year);
        start.set(Calendar.MONTH, from_month);
        start.set(Calendar.DAY_OF_MONTH, from_day);

        Calendar end = Calendar.getInstance();
        end.set(Calendar.YEAR, to_year);
        end.set(Calendar.MONTH, to_month);
        end.set(Calendar.DAY_OF_MONTH, to_day);

        while (start.getTimeInMillis() <= end.getTimeInMillis()) {
            int year = start.get(Calendar.YEAR);
            int month = start.get(Calendar.MONTH);
            int day = start.get(Calendar.DAY_OF_MONTH);
            mDateInfo = LunarParser.parseLunarCalendar(mDateInfo, year, month, day);
            buildCursor(cursor);

            // Plus one day for start.
            start.set(Calendar.DAY_OF_MONTH, day + 1);
        }

        return cursor;
    }

    private void buildCursor(MatrixCursor cursor) {
        if (cursor == null || mDateInfo == null) return;

        // Build the cursor.
        ArrayList<Object> row = new ArrayList<Object>(PROJECTION.length);
        row.add(cursor.getCount());         // _id
        row.add(mDateInfo._solar_year);     // year
        row.add(mDateInfo._solar_month);    // month
        row.add(mDateInfo._solar_day);      // day
        row.add(mDateInfo._lunar_year);     // lunar_year
        row.add(mDateInfo._lunar_month);    // lunar_month
        row.add(mDateInfo._lunar_day);      // lunar_day

        // Get the lunar label.
        String suggestionLongLabel = LunarParser.getLunarLongLabel(mContext, mDateInfo);
        row.add(suggestionLongLabel);       // lunar_label_long
        String suggestionShortLabel = LunarParser.getLunarShortLabel(mContext, mDateInfo);
        row.add(suggestionShortLabel);      // lunar_label_short

        // Get the animal label.
        String animal = LunarParser.getAnimalsYear(mContext, mDateInfo);
        row.add(animal);                    // animal

        // Get the festival and solar terms info.
        ArrayList<String> festivalList = new ArrayList<String>();
        FestivalParser.getFestivals(mContext, mDateInfo, festivalList);
        SolarTermsParser.getSolarTerms(mContext, mDateInfo, festivalList);
        if (festivalList.size() > FESTIVAL_MAX_COUNT) {
            Log.w(TAG, "We found more than max festivals, and will ignore some of them.");
        }

        // Add the festival labels to row.
        for (int i = 0; i < FESTIVAL_MAX_COUNT; i++) {
            String label = "";
            if (i < festivalList.size()) label = festivalList.get(i);
            row.add(label);                 // festivals
        }

        cursor.addRow(row);
    }

    public static class DateInfo {
        public int _solar_year;
        public int _solar_month;
        public int _solar_day;

        public boolean _isLeapMonth;
        public int _lunar_year;
        public int _lunar_month;
        public int _lunar_day;

        public DateInfo(){
        }

        public DateInfo(int solarYear, int solarMonth, int solarDay, boolean isLeapMonth,
                int lunarYear, int lunarMonth, int lunarDay) {
            _solar_year = solarYear;
            _solar_month = solarMonth;
            _solar_day = solarDay;

            _isLeapMonth = isLeapMonth;
            _lunar_year = lunarYear;
            _lunar_month = lunarMonth;
            _lunar_day = lunarDay;
        }
    }
}
