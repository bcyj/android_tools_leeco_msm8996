/*
 * Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.android.qworldclock;

import java.util.Calendar;
import java.util.TimeZone;

import android.content.Context;
import android.util.AttributeSet;
// import android.util.Log;
import android.widget.TextView;
import android.content.ContentResolver;

public class QDigitalClock extends TextView implements TickListener{
    private final static String TAG = "QDigitalClock";

    private TimeZone mTimeZone = null;

    private Context mContext;

    public QDigitalClock(Context context, AttributeSet attrs) {
        this(context, attrs, TimeZone.getDefault().getID());
    }

    public QDigitalClock(Context context, String timezone) {
        this(context, null, timezone);
    }

    public QDigitalClock(Context context, AttributeSet attrs, String timezone) {
        super(context, attrs);
        mTimeZone = TimeZone.getTimeZone(timezone);
        setLines(2);
        mContext = context;
    }

    public void setTimeZone(String timezone) {
        mTimeZone = TimeZone.getTimeZone(timezone);
    }

    @Override
    protected void onAttachedToWindow() {
        // Log.d(TAG, "onAttachedToWindow");
        register();
        updateTime();
    }

    @Override
    protected void onDetachedFromWindow() {
        // Log.d(TAG, "onDetachedFromWindow");
        unregister();
    }

    public void updateTime() {
        Calendar calendar = Calendar.getInstance(mTimeZone);
        setTextSize(16);
        ContentResolver c = mContext.getContentResolver();
        String strTimeFormat = android.provider.Settings.System.getString(c, android.provider.Settings.System.TIME_12_24);
        String strDateFormat = android.provider.Settings.System.getString(mContext.getContentResolver(),
                android.provider.Settings.System.DATE_FORMAT);

        String time = null;
        String date = null;
        // Set time format.
        if ("24".equals(strTimeFormat)) {
            time = String.format("  %1$tH:%1$tM:%1tS\n", calendar);
        } else {
            time = String.format("  %1$tI:%1$tM:%1tS %1$tp\n", calendar);
        }
        // Set date format.
        if ("MM-dd-yyyy".equals(strDateFormat)) {
            date = String.format("  %1$tm-%1$td-%1$tY", calendar);
        } else if ("dd-MM-yyyy".equals(strDateFormat)) {
            date = String.format("  %1$td-%1$tm-%1$tY", calendar);
        } else if ("yyyy-MM-dd".equals(strDateFormat)) {
            date = String.format("  %1$tY-%1$tm-%1$td", calendar);
        } else {
            date = String.format("  %1$tm-%1$td-%1$tY", calendar);
        }
        setText(time + date);
    }

    public boolean register() {
        return TickBroadCastor.getInstance().addListener(this);
    }

    public boolean unregister() {
        return TickBroadCastor.getInstance().removeListener(this);
    }

    public void setContext(Context context) {
        mContext = context;
    }
}
