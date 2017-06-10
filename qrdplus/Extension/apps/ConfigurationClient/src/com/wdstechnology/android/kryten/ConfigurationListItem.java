/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.wdstechnology.android.kryten;

import java.util.ArrayList;

import android.content.Context;
import android.database.Cursor;

import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.RelativeLayout;
import android.widget.TextView;

/**
 * This class manages the view for given conversation.
 */
public class ConfigurationListItem extends RelativeLayout {
    private static final String TAG = "ConfigurationListItem";
    public static final String BROWSERTAG = "w2";
    public static final String MMSTAG = "w4";
    public static final String POP3TAG = "110";
    public static final String IMAPTAG = "143";
    public static final String SMTPTAG = "25";

    private TextView mSubjectView;
    private TextView mTitleView;
    private TextView mDateView;
    private TextView mSimView;
    private Context mContext;

    public ConfigurationListItem(Context context) {
        super(context);
        mContext = context;
    }

    public ConfigurationListItem(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;

    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mTitleView = (TextView) findViewById(R.id.title);
        mSubjectView = (TextView) findViewById(R.id.subject);
        mDateView = (TextView) findViewById(R.id.date);
        mSimView = (TextView) findViewById(R.id.sim);
    }

    public final void bind(Context context, Cursor cursor) {
        long id = cursor.getLong(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns._ID));
        if (ConfigurationMessageActivity.mSelectedThreadIds != null
                && ConfigurationMessageActivity.mSelectedThreadIds.contains(id)) {
            setBackgroundColor(context.getResources()
                    .getColor(android.R.color.secondary_text_light));
        } else {
            setBackgroundColor(context.getResources().getColor(android.R.color.white));

        }

        ArrayList<String> subject = null;
        String sim = null;
        subject = getApplicationName(cursor);
        String mSubject = "";

        if(subject.contains(BROWSERTAG)){
            mSubject = getResources().getString(R.string.browser_appl);
        }
        if(subject.contains(MMSTAG)){
            mSubject = mSubject+", "+getResources().getString(R.string.mms_appl);
        }
        if(subject.contains(POP3TAG) || subject.contains(IMAPTAG) || subject.contains(SMTPTAG)){
            mSubject = mSubject+", "+getResources().getString(R.string.email_appl);
        }

        boolean install = (cursor.getInt(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.INSTALL)) == 1);
        String title;
        if (!install)
            title = getResources().getString(R.string.app_label);
        else
            title = getResources().getString(R.string.app_label)
                    + getResources().getString(R.string.instaled);
        mTitleView.setText(title);

        mSubjectView.setText(mSubject);
        sim = cursor.getString(cursor.getColumnIndex(ConfigurationDatabaseProvider.Columns.SIM));
        if (sim.equals("GSM2"))
            sim = getResources().getString(R.string.sim2);
        else
            sim = getResources().getString(R.string.sim1);
        mSimView.setText(sim);
        Long lDate = cursor.getLong(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.DATE));

        mDateView.setText(formatTimeStampString(lDate, false));
    }

    public String formatTimeStampString(long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
                DateUtils.FORMAT_ABBREV_ALL |
                DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make
        // showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(mContext, when, format_flags);
    }

    public static ArrayList<String> getApplicationName(Cursor cursor){
        ArrayList<String> appName = new ArrayList<String>();
        String appidBrowser = cursor.getString(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.APPID_BROWSER));
        String appidMms = cursor.getString(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.APPID_MMS));
        String appidPop3 = cursor.getString(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.APPID_POP3));
        String appidImap4 = cursor.getString(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.APPID_IMAP4));
        String appidSmtp = cursor.getString(cursor
                .getColumnIndex(ConfigurationDatabaseProvider.Columns.APPID_SMTP));
        if(appidBrowser != null)
            appName.add(BROWSERTAG);
        if(appidMms != null)
            appName.add(MMSTAG);
        if(appidPop3 != null || appidImap4 != null)
            appName.add(POP3TAG);
        return appName;
    }

}
