/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.backup.vcalendar;

import com.android.backup.EventRecurrence;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;
import android.provider.CalendarContract.Reminders;
import android.provider.CalendarContract.Events;
//import android.provider.Calendar.EventCount;
import android.provider.BaseColumns;
import com.android.backup.pim.PropertyNode;
import com.android.backup.pim.VDataBuilder;
import com.android.backup.pim.VNode;
import com.android.backup.vcalendar.CalendarStruct;
import com.android.backup.vcalendar.VCalComposer;
import com.android.backup.vcalendar.VCalException;
import com.android.backup.vcalendar.VCalParser;
import android.text.format.Time;
import android.util.Log;

import java.util.HashMap;
import java.util.Set;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import android.content.ContentUris;
import java.util.TimeZone;
import android.net.Uri;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.GregorianCalendar;
import android.text.TextUtils;

///zhl add to provide the vcal interface to other apps.
public class VCalendarManager
{

    private static final String TAG = "VCalendarManager";
    private static final boolean LOCAL_DEBUG = true;
    private static final String TIME_FORMAT_STRING = "%Y-%m-%d %H:%M:%S";

    private static final int DEFAULT_ALARM_MINUTES = 10;

    private List<ContentValues> mVCalValuesList;

    private HashMap<Integer, ArrayList<ContentValues>> mRemindersMap;

    private final Context mContext;
    private final ContentResolver mResolver;
    private Uri mUri;
    private HashMap<String, Long> mCalendars = null;

    private int vCalVersion = VCalComposer.VERSION_VCAL10_INT;
    private boolean mUseUTC = true;

    // when the received vcal data is 1.0 version,i'll drop the rrule data.
    // because the current android version doesn't support v1.0 rrule analysis'
    // and when the vcal1.0 rrule data comes into the calendarprovider ,
    // it'll only analyze the vcal2.0 rrule data,,,dropping the v1.0 rrule will
    // avoid throwing a exception

    private String mVcalVersion = VCalParser.VERSION_VCALENDAR20;

    public String mTimeZone = "+08:00"; // ;TimeZone.getDefault().toString() ;
                                        // //null

    public static final class EventCount implements BaseColumns
    {

        public static final String EVENT_TYPE = "event_type";
        public static final String EVENT_COUNT = "count";

        public static final Uri CONTENT_URI =
                Uri.parse("content://" + "com.android.calendar" + "/eventcount");
    }

    private static final String[] EVENT_PROJECTION = new String[] {
            Events._ID,
            Events.TITLE,
            Events.RRULE,
            Events.ALL_DAY,
            Events.CALENDAR_ID,
            Events.DTSTART,
            Events.DTEND,
            Events.DURATION,
            Events._SYNC_ID,
            Events.EVENT_TIMEZONE,
            Events.DESCRIPTION,
            Events.STATUS,
            Events.EVENT_LOCATION,
            Events.HAS_ALARM,
            Events.ACCESS_LEVEL,
            Events.LAST_DATE,
    };

    static final String[] CALENDARS_PROJECTION = new String[] {
            Calendars._ID, // 0
            Calendars.NAME, // 1
    };

    static final int CALENDARS_INDEX_ID = 0;
    static final int CALENDARS_INDEX_NAME = 1;
    static final String LOCAL_CALENDAR = "Local Calendar";

    public int mCalendarId = 1;
    public static final int FLAG_ALL_DATA = 0;
    public static final int FLAG_FIRST_DATA = 1;
    public static final int FLAG_VCAL_DATA = 2;
    public static final int FLAG_TAIL_DATA = 3;

    public boolean isDue = false;
    public boolean isTodoDue = false;

    /**
     * Constructor.
     *
     * @param context the context of the activity.
     * @param data Calendar data.
     * @throws IllegalArgumentException
     */
    public VCalendarManager(Context context, String data) throws IllegalArgumentException {
        if (context == null) {
            throw new IllegalArgumentException();
        }

        if (mVCalValuesList == null)
            mVCalValuesList = new ArrayList<ContentValues>();

        if (mRemindersMap == null)
            mRemindersMap = new HashMap<Integer, ArrayList<ContentValues>>();

        mContext = context;
        mResolver = mContext.getContentResolver();

        parseVCalendar(data);
    }

    /**
     * Constructor.
     *
     * @param context the context of the activity.
     * @param uri the stored event URI.
     */
    public VCalendarManager(Context context, Uri uri) {
        mContext = context;
        mResolver = mContext.getContentResolver();
        mUri = uri;
    }

    public void setVcalVersion(int version)
    {
        if ((version == VCalComposer.VERSION_VCAL10_INT)
                || (version == VCalComposer.VERSION_VCAL20_INT))
            vCalVersion = version;
        else
            vCalVersion = VCalComposer.VERSION_VCAL10_INT;

    }

    public void setUseUtc(boolean useUtc)
    {
        mUseUTC = useUtc;
    }

    /**
     * Load the formatted data of the stored calendar event.
     *
     * @return the formatted data string.
     */
    public String getVeventData(int dataMode) {

        if (mUri == null) {
            Log.e(TAG, "Bad content URI.");
            return null;
        }
        Cursor c = mResolver.query(mUri, EVENT_PROJECTION, null, null, null);

        String uid = mUri.toString();
        if (c == null) {
            return null;
        }
        try {

            c.moveToFirst();
            CalendarStruct calStruct = new CalendarStruct();
            mTimeZone = c
                    .getString(c.getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));
            if (!mUseUTC)
            {
                if ((mTimeZone == null) || (mTimeZone.length() == 0))
                    mUseUTC = true;
            }

            // because the dtstart and dtend will convert to utc standard
            // time,the vcal not need add the timezone info.
            if (!mUseUTC)
            {
                if (vCalVersion == VCalComposer.VERSION_VCAL10_INT) {
                    String timezone = c.getString(c
                            .getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));
                    TimeZone tz = TimeZone.getTimeZone(mTimeZone);
                    calStruct.timezone = generateTimezoneOffset(tz, vCalVersion);
                } else {
                    calStruct.timezone = mTimeZone;
                }

            }

            // if(isNull(calStruct.timezone))
            // useUtc=true;

            // event list:
            CalendarStruct.EventStruct evtStruct = new CalendarStruct.EventStruct();
            evtStruct.timezone = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.EVENT_TIMEZONE));
            evtStruct.uid = uid;
            evtStruct.description = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.DESCRIPTION));
            // evtStruct.dtend =
            // convertLongToRFC2445DateTime(c.getLong(c.getColumnIndexOrThrow(
            // Calendar.Events.DTEND)));//transitionMillisToVCalendarTime
            String strTimeZone = c.getString(c
                    .getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));
            TimeZone myTimeZone = TimeZone.getTimeZone(strTimeZone);
            evtStruct.dtend = transitionMillisToVCalendarTime(
                    c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.DTEND)),
                    myTimeZone,
                    false);

            evtStruct.allDay = c.getString(c.getColumnIndexOrThrow(// sxm add
                                                                   // for allDay
                                                                   // 20121114
                    CalendarContract.Events.ALL_DAY));
            long startMillis = -1;
            if (!c.isNull(c.getColumnIndexOrThrow(CalendarContract.Events.DTSTART)))
            {
                startMillis = c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.DTSTART));
                evtStruct.dtstart = transitionMillisToVCalendarTime(startMillis,
                        myTimeZone,
                        false);
            }

            evtStruct.duration = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.DURATION));
            evtStruct.event_location = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.EVENT_LOCATION));
            evtStruct.has_alarm = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.HAS_ALARM));
            // evtStruct.last_date =
            // convertLongToRFC2445DateTime(c.getLong(c.getColumnIndexOrThrow(
            // Calendar.Events.LAST_DATE)));
            evtStruct.last_date = transitionMillisToVCalendarTime(
                    c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.LAST_DATE)),
                    myTimeZone,
                    false);
            evtStruct.rrule = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.RRULE));
            evtStruct.status = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.STATUS));
            evtStruct.title = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.TITLE));
            /*
             * int category = c.getInt(c.getColumnIndexOrThrow(
             * CalendarContract.Events.EVENT_TYPE)); switch(category) { default:
             * case Events.EVENT_MEMO:
             * //evtStruct.category="MEMO";//category:default as null break;
             * case Events.EVENT_APPOINTMENT: evtStruct.category="APPOINTMENT";
             * break; case Events.EVENT_ANNIVERSARY:
             * evtStruct.category="ANNIVERSARY"; break; } int classfication=
             * c.getInt(c.getColumnIndexOrThrow(
             * CalendarContract.Events.CLASSFICATION)); switch(classfication) {
             * default: case 0: evtStruct.classfication="PUBLIC"; break; case 1:
             * evtStruct.classfication="PRIVATE"; break; case 2:
             * evtStruct.classfication="CONFIDENTIAL"; break; }
             */
            if (!isNull(evtStruct.has_alarm)) {
                getReminders(evtStruct, Uri.parse(uid).getLastPathSegment(), startMillis);
            }

            calStruct.addEventList(evtStruct);

            try { // build vcalendar:
                VCalComposer composer = null;
                switch (dataMode)
                {
                    default:
                    case FLAG_ALL_DATA:
                        composer = new VCalComposer();
                        break;

                    case FLAG_FIRST_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_HEAD
                                | VCalComposer.MODE_MASK_VCAL);
                        break;

                    case FLAG_VCAL_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_VCAL);
                        break;

                    case FLAG_TAIL_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_VCAL
                                | VCalComposer.MODE_MASK_TAIL);
                        break;
                }
                return composer.createVCal(calStruct, vCalVersion);
            } catch (Exception e) {
                Log.d(TAG, "CreateVcal catch exception:" + e.getMessage());
                e.printStackTrace();
                return null;
            }
        } finally {
            c.close();
        }
    }

    static String generateTimezoneOffset(TimeZone timezone, int vCalVersion) {
        int offset = timezone.getRawOffset();
        int p = Math.abs(offset);

        StringBuilder name = new StringBuilder();
        if (offset < 0) {
            name.append('-');
        } else {
            name.append('+');
        }
        int hour = p / (60 * 60000);

        if (hour < 10)
            name.append('0');
        name.append(hour);
        if (vCalVersion == VCalComposer.VERSION_VCAL10_INT)
            name.append(':');
        int min = p / 60000;
        min %= 60;
        if (min < 10) {
            name.append('0');
        }
        name.append(min);
        return name.toString();
    }

    public String getVtodoData(int dataMode) {

        if (mUri == null) {
            Log.e(TAG, "Bad content URI.");
            return null;
        }
        // Cursor c = mResolver.query(mUri,null, null, null, null);
        Cursor c = mResolver.query(mUri, EVENT_PROJECTION, null, null, null);

        String uid = mUri.toString();
        if (c == null) {
            return null;
        }
        try {

            // boolean useUtc=false;

            c.moveToFirst();
            CalendarStruct calStruct = new CalendarStruct();

            mTimeZone = c
                    .getString(c.getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));

            if (!mUseUTC)
            {
                if ((mTimeZone == null) || (mTimeZone.length() == 0))
                    mUseUTC = true;
            }

            // because the dtstart and dtend will convert to utc standard
            // time,the vcal not need add the timezone info.
            if (!mUseUTC)
            {
                if (vCalVersion == VCalComposer.VERSION_VCAL10_INT) {
                    String timezone = c.getString(c
                            .getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));
                    TimeZone tz = TimeZone.getTimeZone(mTimeZone);
                    calStruct.timezone = generateTimezoneOffset(tz, vCalVersion);
                } else {
                    calStruct.timezone = mTimeZone;
                }
            }
            // if(isNull(calStruct.timezone))
            // useUtc=true;

            // todo list:
            CalendarStruct.TodoStruct todoStruct = new CalendarStruct.TodoStruct();

            todoStruct.uid = uid;
            todoStruct.description = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.DESCRIPTION));
            todoStruct.timezone = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.EVENT_TIMEZONE));// sunxiaoming add
                                                             // 20120903

            String strTimeZone = c.getString(c
                    .getColumnIndexOrThrow(CalendarContract.Events.EVENT_TIMEZONE));
            TimeZone myTimeZone = TimeZone.getTimeZone(strTimeZone);

            long startMills = -1;
            if (!c.isNull(c.getColumnIndexOrThrow(CalendarContract.Events.DTSTART)))
            {
                startMills = c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.DTSTART));
                // todoStruct.dtstart =
                // convertLongToRFC2445DateTime(startMills);
                todoStruct.dtstart = transitionMillisToVCalendarTime(startMills,
                        myTimeZone,
                        false);
            }

            long endMills = c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.DTEND));
            // todoStruct.dtend = convertLongToRFC2445DateTime(endMills);
            todoStruct.dtend = transitionMillisToVCalendarTime(endMills,
                    myTimeZone,
                    false);

            todoStruct.duration = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.DURATION));

            if (todoStruct.duration == null)
            {
                todoStruct.duration = getDuration(startMills, endMills);
            }

            todoStruct.has_alarm = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.HAS_ALARM));

            // todoStruct.last_date =
            // convertLongToRFC2445DateTime(c.getLong(c.getColumnIndexOrThrow(
            // Calendar.Events.LAST_DATE)));
            todoStruct.last_date = transitionMillisToVCalendarTime(
                    c.getLong(c.getColumnIndexOrThrow(CalendarContract.Events.LAST_DATE)),
                    myTimeZone,
                    false);

            todoStruct.status = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.STATUS));

            todoStruct.title = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.TITLE));

            // todoStruct.priority=c.getString(c.getColumnIndexOrThrow(
            // CalendarContract.Events.PRIORITY));

            todoStruct.allDay = c.getString(c.getColumnIndexOrThrow(
                    CalendarContract.Events.ALL_DAY));
            if (!isNull(todoStruct.has_alarm)) {
                getTodoReminders(todoStruct, Uri.parse(uid).getLastPathSegment(), startMills);
            }

            calStruct.addTodoList(todoStruct);

            try { // build vcalendar:
                VCalComposer composer = null;
                switch (dataMode)
                {
                    default:
                    case FLAG_ALL_DATA:
                        composer = new VCalComposer();
                        break;

                    case FLAG_FIRST_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_HEAD
                                | VCalComposer.MODE_MASK_VCAL);
                        break;

                    case FLAG_VCAL_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_VCAL);
                        break;

                    case FLAG_TAIL_DATA:
                        composer = new VCalComposer(VCalComposer.MODE_MASK_VCAL
                                | VCalComposer.MODE_MASK_TAIL);
                        break;
                }
                return composer.createVCal(calStruct, vCalVersion);
            } catch (Exception e) {
                Log.d(TAG, "CreateVcal catch exception:" + e.toString());
                e.printStackTrace();
                return null;
            }
        } finally {
            c.close();
        }
    }

    private void getReminders(CalendarStruct.EventStruct evtStruct, String localid, long startMills) {

        Cursor c = mResolver.query(CalendarContract.Reminders.CONTENT_URI,
                null, "event_id=" + localid, null, null);
        String data = "";
        String alarmTime = "";
        int minute = 0;
        long convertMills = 0;

        while ((c != null) && c.moveToNext()) {
            data = c.getString(c.getColumnIndexOrThrow(CalendarContract.Reminders.METHOD));
            minute = c.getInt(c.getColumnIndexOrThrow(CalendarContract.Reminders.MINUTES));

            StringBuilder sb = new StringBuilder();
            // you can also get some other alarm infomation if you need to
            // constrct vcal
            switch (vCalVersion)
            {
                default:
                case VCalComposer.VERSION_VCAL10_INT: {
                    if (startMills != -1)
                    {
                        convertMills = startMills - minute * 60000;
                        alarmTime = convertLongToRFC2445DateTime(convertMills);
                        sb.append(alarmTime);
                        sb.append(";;");
                        evtStruct.addAlarmTimeList(sb.toString());
                    }

                }
                    break;

                case VCalComposer.VERSION_VCAL20_INT: {
                    if (minute >= 0)
                    {
                        sb.append("-PT");
                        sb.append(minute);
                        sb.append('M');
                        alarmTime = sb.toString();

                        evtStruct.addAlarmTimeList(alarmTime);
                    }
                }
                    break;

            }

            evtStruct.addReminderList(data);
        }
        if (c != null) {
            c.close();
        }
    }

    private void getTodoReminders(CalendarStruct.TodoStruct todoStruct, String localid,
            long startMills) {
        Cursor c = mResolver.query(CalendarContract.Reminders.CONTENT_URI,
                null, "event_id=" + localid, null, null);
        String data = "";
        String alarmTime = "";
        int minute = 0;
        long convertMills = 0;
        StringBuilder sb = new StringBuilder();
        while ((c != null) && c.moveToNext()) {
            data = c.getString(c.getColumnIndexOrThrow(CalendarContract.Reminders.METHOD));

            minute = c.getInt(c.getColumnIndexOrThrow(CalendarContract.Reminders.MINUTES));

            // you can also get some other alarm infomation if you need to
            // constrct vcal
            switch (vCalVersion)
            {
                default:
                case VCalComposer.VERSION_VCAL10_INT: {
                    if (startMills != -1)
                    {
                        convertMills = startMills - minute * 60000;
                        alarmTime = convertLongToRFC2445DateTime(convertMills);
                        todoStruct.addAlarmTimeList(alarmTime);
                    }

                }
                    break;

                case VCalComposer.VERSION_VCAL20_INT: {
                    if (minute > 0)
                    {
                        sb.append("-PT");
                        sb.append(minute);
                        sb.append('M');
                        alarmTime = sb.toString();

                        todoStruct.addAlarmTimeList(alarmTime);
                    }
                }
                    break;

            }

            todoStruct.addReminderList(data);
        }
        if (c != null) {
            c.close();
        }
    }

    private void addReminders(ArrayList<ContentValues> valuesList)
    {
        if ((valuesList == null) || (valuesList.size() == 0))
            return;

        int index = mVCalValuesList.size();

        mRemindersMap.put(index, valuesList);

    }

    private String getDuration(long startMills, long endMills)
    {

        long durationMills = endMills - startMills;
        long tem1;

        if (durationMills < 0)
            return null;

        StringBuilder durationSb = new StringBuilder();
        long weeks = 0;
        long days = 0;
        long hours = 0;
        long minutes = 0;
        long seconds = 0;

        tem1 = durationMills / 1000;

        durationSb.append("P");

        if ((weeks = tem1 / (7 * 24 * 60 * 60)) != 0)
            durationSb.append(weeks).append("W");

        tem1 = tem1 % (7 * 24 * 60 * 60);
        if ((days = tem1 / (24 * 60 * 60)) != 0)
            durationSb.append(days).append("D");

        durationSb.append("T");

        tem1 = tem1 % (24 * 60 * 60);
        if ((hours = tem1 / (60 * 60)) != 0)
            durationSb.append(hours).append("H");

        tem1 = tem1 % (60 * 60);
        if ((hours = tem1 / 60) != 0)
            durationSb.append(hours).append("M");

        tem1 = tem1 % 60;
        if ((hours = tem1) != 0)
            durationSb.append(hours).append("S");

        return durationSb.toString();
    }

    static String transitionMillisToVCalendarTime(long millis, TimeZone tz, boolean dst) {
        StringBuilder sb = new StringBuilder();

        /*
         * int rawOffsetMinutes = tz.getRawOffset() / MINUTES; String
         * standardOffsetString = utcOffsetString(rawOffsetMinutes); Log.i(TAG,
         * "transitionMillisToVCalendarTime"+standardOffsetString);
         */

        GregorianCalendar cal = new GregorianCalendar(tz);
        cal.setTimeInMillis(millis);
        int zoneOffset = cal.get(java.util.Calendar.ZONE_OFFSET);
        int dstOffset = cal.get(java.util.Calendar.DST_OFFSET);
        cal.add(java.util.Calendar.MILLISECOND, -(zoneOffset + dstOffset));

        sb.append(cal.get(java.util.Calendar.YEAR));
        sb.append(formatTwo(cal.get(java.util.Calendar.MONTH) + 1));
        sb.append(formatTwo(cal.get(java.util.Calendar.DAY_OF_MONTH)));
        sb.append('T');
        sb.append(formatTwo(cal.get(java.util.Calendar.HOUR_OF_DAY)));
        sb.append(formatTwo(cal.get(java.util.Calendar.MINUTE)));
        sb.append(formatTwo(cal.get(java.util.Calendar.SECOND)));
        sb.append('Z');

        return sb.toString();
    }

    static final String[] sTwoCharacterNumbers =
            new String[] {
                    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"
            };

    static String formatTwo(int num) {
        if (num <= 12) {
            return sTwoCharacterNumbers[num];
        } else
            return Integer.toString(num);
    }

    long transitionVCalendarTimeToMillis(String VCalendarTime, TimeZone timezone)
    {
        if (TextUtils.isEmpty(VCalendarTime)) {
            return 0;
        }

        String date = VCalendarTime;

        GregorianCalendar cal = new GregorianCalendar(Integer.parseInt(date.substring(0, 4)),
                Integer.parseInt(date.substring(4, 6)) - 1, Integer.parseInt(date.substring(6, 8)),
                Integer.parseInt(date.substring(9, 11)), Integer.parseInt(date.substring(11, 13)),
                Integer.parseInt(date.substring(13, 15)));

        cal.setTimeZone(timezone);

        return cal.getTimeInMillis();
    }

    private String convertLongToRFC2445DateTime(long mills) {

        Time time = new Time();

        if (!mUseUTC)
        {
            time = new Time(mTimeZone);
            time.set(mills);
            return time.format("%Y%m%dT%H%M%S");
        }

        time = new Time();
        TimeZone tz = TimeZone.getDefault();
        // int offset = tz.getRawOffset();
        int offset = tz.getOffset(mills);// pengshijie modify for daylight
                                         // savings time.
        int p = Math.abs(offset);
        long convertMills = 0;

        if (offset < 0) {
            convertMills = mills + p;
        } else {
            convertMills = mills - p;
        }

        // test
        int min = p / 60000;
        min %= 60;
        // test

        time.set(convertMills);

        return time.format("%Y%m%dT%H%M%SZ");

    }

    private boolean isNull(String str) {
        if ((str == null) || str.trim().equals("")) {
            return true;
        }
        return false;
    }

    private void parseVCalendar(String data) {
        VCalParser parser = new VCalParser();
        VDataBuilder builder = new VDataBuilder();
        ContentValues contentValues;

        if (null == data) {
            if (LOCAL_DEBUG)
                Log.d(TAG, "The parameter of parseVCalendar is null");
            return;
        }

        try {
            parser.parse(data, builder);
            mVcalVersion = parser.getVcalVersion();
        } catch (VCalException e) {
            Log.e(TAG, "VCalException: ", e);
            return;
        }

        if (LOCAL_DEBUG)
        {
            Log.d(TAG, "The parser version is :" + mVcalVersion);
        }

        String localCalendarId = "";
        for (VNode vnode : builder.vNodeList) {
            // VCALENDAR field MUST present before VENENT and VTODO
            if (vnode.VName.equalsIgnoreCase("VCALENDAR")) {
                // If no Calendar, just set -1 as CalendarId, because user
                // should view the content even if no calendar created.
                mCalendarId = getLocalidForCalendar();
                if (LOCAL_DEBUG)
                    Log.d(TAG, "The calendar id is:" + mCalendarId);
                localCalendarId = String.valueOf(mCalendarId);

                getVcalTimeZone(vnode);
            }
            else if (vnode.VName.equalsIgnoreCase("VEVENT"))
            {
                contentValues = setEventMap(vnode, localCalendarId);
                mVCalValuesList.add(contentValues);

            }
            else if (vnode.VName.equalsIgnoreCase("VTODO"))
            {
                contentValues = setTodoMap(vnode, localCalendarId);
                mVCalValuesList.add(contentValues);
            }
            else
            {
            }
        }

        return;
    }

    private void getVcalTimeZone(VNode vnode)
    {
        for (PropertyNode prop : vnode.propList) {
            if (prop.propValue != null) {
                if (prop.propName.equalsIgnoreCase("TZ")) {
                    mTimeZone = prop.propValue;
                    break;
                }
            }
        }

    }

    private int getLocalidForCalendar() {

        Cursor mCalendarsCursor = mResolver.query(CalendarContract.Calendars.CONTENT_URI,
                CALENDARS_PROJECTION,
                null, null, null);
        int id = 1;

        if (mCalendarsCursor == null)
            return id;

        if (mCalendarsCursor.moveToFirst())
        {
            id = mCalendarsCursor.getInt(CALENDARS_INDEX_ID);
            String name = mCalendarsCursor.getString(CALENDARS_INDEX_NAME);
            if (name != null && name.equalsIgnoreCase(LOCAL_CALENDAR))
            {
                mCalendarsCursor.close();
                return id;
            }
            if (LOCAL_DEBUG)
                Log.d(TAG, "The id is :" + id + ",name:" + name);
        }

        while (mCalendarsCursor.moveToNext()) {
            id = mCalendarsCursor.getInt(CALENDARS_INDEX_ID);
            String name = mCalendarsCursor.getString(CALENDARS_INDEX_NAME);

            if (LOCAL_DEBUG)
                Log.d(TAG, "The id is :" + id + ",name:" + name);

            if (name != null && name.equalsIgnoreCase(LOCAL_CALENDAR))
            {
                mCalendarsCursor.close();
                return id;
            }
        }
        mCalendarsCursor.close();

        return id;

    }

    private ContentValues setEventMap(VNode vnode, String calId) {
        ContentValues values = new ContentValues();

        ArrayList<ContentValues> remindersList = new ArrayList<ContentValues>();
        ContentValues reminderValues = new ContentValues();

        values.put(CalendarContract.Events.CALENDAR_ID, calId);
        // values.put(Calendar.Events.EVENT_TYPE,Calendar.Events.EVENT_MEMO);//set
        // the default category
        // values.put(Calendar.Events.HAS_ALARM,1);//set alarm

        if (mTimeZone != null)
        {
            if (LOCAL_DEBUG)
                Log.d(TAG, "The time zone is :" + mTimeZone);
            TimeZone tz = TimeZone.getTimeZone("GMT" + mTimeZone);
            // TimeZone tz = TimeZone.getTimeZone("America/Los_Angeles");
            // values.put(CalendarContract.Events.EVENT_TIMEZONE,tz.getID());
        }

        String tempString = "";
        long startMills = 0;
        long endMills = 0;
        String alarmTime = "";
        long alarmMills = 0;
        boolean hasAlarm = false;
        boolean hasAttendeeData = false;

        // sunxiaoming add 20120904
        for (PropertyNode prop : vnode.propList) {
            if (prop.propName.equalsIgnoreCase("TZID")) {
                mTimeZone = prop.propValue;
                TimeZone tz1 = TimeZone.getTimeZone(mTimeZone);
                values.put(CalendarContract.Events.EVENT_TIMEZONE, tz1.getID());
                int offset1 = tz1.getRawOffset();
                // mTimeZone = String.valueOf(offset1);
                int p = Math.abs(offset1);

                StringBuilder name = new StringBuilder();

                if (offset1 < 0) {
                    name.append('-');
                } else {
                    name.append('+');
                }

                int hour = p / (60 * 60000);

                if (hour < 10)
                    name.append('0');

                name.append(hour);

                if (vCalVersion == VCalComposer.VERSION_VCAL10_INT)
                    name.append(':');

                int min = p / 60000;
                min %= 60;

                if (min < 10) {
                    name.append('0');
                }
                name.append(min);
                mTimeZone = name.toString();
                break;
            }
            else {
                values.put(CalendarContract.Events.EVENT_TIMEZONE, mTimeZone);
            }
        }
        for (PropertyNode prop : vnode.propList) {
            if (prop.propValue != null) {
                Time time = new Time();

                values.put(CalendarContract.Events.HAS_ATTENDEE_DATA, 1);
                TimeZone myTimeZone = TimeZone.getTimeZone(mTimeZone);
                if (prop.propName.equalsIgnoreCase("DESCRIPTION")) {
                    values.put(CalendarContract.Events.DESCRIPTION, prop.propValue);
                } else if (prop.propName.equalsIgnoreCase("DTSTART")) {
                    tempString = prop.propValue;
                    if (tempString.contains("Z")) {
                        startMills = /* parseTimeStr */transitionVCalendarTimeToMillis(tempString,
                                myTimeZone);
                    } else {
                        startMills = parseTimeStr(tempString, mTimeZone);
                    }
                    values.put(CalendarContract.Events.DTSTART, startMills);
                    // time.parse(prop.propValue);
                    // values.put(Calendar.Events.DTSTART, time.toMillis(false
                    // /* use isDst */));
                } else if (prop.propName.equalsIgnoreCase("DTEND")) {
                    tempString = prop.propValue;
                    if (tempString.contains("Z")) {
                        endMills = /* parseTimeStr */transitionVCalendarTimeToMillis(tempString,
                                myTimeZone);
                    } else {
                        endMills = parseTimeStr(tempString, mTimeZone);
                    }
                    values.put(CalendarContract.Events.DTEND, endMills);
                    // time.parse(prop.propValue);
                    // values.put(Calendar.Events.DTEND, time.toMillis(false /*
                    // use isDst */));
                } else if (prop.propName.equalsIgnoreCase("SUMMARY")) {
                    values.put(CalendarContract.Events.TITLE, prop.propValue);
                } else if (prop.propName.equalsIgnoreCase("LOCATION")) {
                    values.put(CalendarContract.Events.EVENT_LOCATION, prop.propValue);
                } else if (prop.propName.equalsIgnoreCase("DUE")
                        || prop.propName.equalsIgnoreCase("DURATION")) {
                    isDue = true;
                    values.put(CalendarContract.Events.DURATION, prop.propValue);
                }
                else if (prop.propName.equalsIgnoreCase("X-ALLDAY")) {
                    Log.d(TAG, "ALL_DAY  = " + prop.propValue);
                    values.put(CalendarContract.Events.ALL_DAY, prop.propValue);
                } else if (prop.propName.equalsIgnoreCase("RRULE")) {
                    Log.d(TAG, "mVcalVersion:" + mVcalVersion);
                    try {
                        EventRecurrence excepRecurrence = new EventRecurrence();
                        excepRecurrence.parse(prop.propValue);
                        if (mVcalVersion.equals(VCalParser.VERSION_VCALENDAR20)) {
                            values.put(CalendarContract.Events.RRULE,
                                    prop.propValue);
                        } else if (mVcalVersion
                                .equals(VCalParser.VERSION_VCALENDAR10)) {
                            values.put(CalendarContract.Events.RRULE,
                                    prop.propValue);
                        }
                    } catch (Exception e) {
                        prop.propValue = convertRrule(prop.propValue);
                        values.put(CalendarContract.Events.RRULE, prop.propValue);
                    }
                }
                /*
                 * else if (prop.propName.equalsIgnoreCase("RRULE")) {
                 * //pengshijie modify begin 20100911.
                 * if(mVcalVersion.equals(VCalParser.VERSION_VCALENDAR20)) {
                 * values.put(Calendar.Events.RRULE, prop.propValue); } else
                 * if(mVcalVersion.equals(VCalParser.VERSION_VCALENDAR10)) {
                 * values.put(Calendar.Events.RRULE, prop.propValue); }
                 * //pengshijie modify end. }
                 */

                else if (prop.propName.equalsIgnoreCase("CLASS")) {
                    String defaultValue = "0";
                    if (prop.propValue.equalsIgnoreCase("PRIVATE")) {
                        defaultValue = "1";
                    } else if (prop.propValue.equalsIgnoreCase("CONFIDENTIAL")) {
                        defaultValue = "2";
                    }
                    // values.put(Calendar.Events.CLASSFICATION,
                    // prop.propValue);
                    // values.put(CalendarContract.Events.CLASSFICATION,
                    // defaultValue);
                }
                else if (prop.propName.equalsIgnoreCase("COMPLETED")
                        || prop.propName.equalsIgnoreCase("DTSTAMP")) {// sunxiaoming
                                                                       // add
                                                                       // for
                                                                       // the
                                                                       // chinaMobile
                                                                       // new
                                                                       // standard
                    if (prop.propValue.contains("Z")) {
                        long myMillis = transitionVCalendarTimeToMillis(prop.propValue, myTimeZone);
                        values.put(CalendarContract.Events.LAST_DATE, myMillis);
                    } else {
                        time.parse(prop.propValue);
                        values.put(CalendarContract.Events.LAST_DATE, time.toMillis(false /*
                                                                                           * use
                                                                                           * isDst
                                                                                           */));
                    }
                }
                else if (prop.propName.equalsIgnoreCase("AALARM"))
                {

                    if (prop.propValue.equalsIgnoreCase("DEFAULT"))
                    {
                        if (!hasAlarm)
                        {
                            hasAlarm = true;
                            values.put(CalendarContract.Events.HAS_ALARM, 1);// set
                                                                             // alarm
                        }
                        reminderValues.put(CalendarContract.Reminders.MINUTES,
                                DEFAULT_ALARM_MINUTES);
                        reminderValues.put(CalendarContract.Reminders.METHOD,
                                CalendarContract.Reminders.METHOD_ALERT);
                        remindersList.add(reminderValues);
                    }
                    else
                    {
                        if (mVcalVersion.equals(VCalParser.VERSION_VCALENDAR20)) {// sunxiaoming
                                                                                  // add
                                                                                  // to
                                                                                  // distinguish
                                                                                  // 2.0
                                                                                  // or
                                                                                  // 1.0
                            long mAlarmTime = 0;
                            alarmTime = prop.propValue_vector.get(0);
                            mAlarmTime = Long.parseLong(alarmTime);
                            if (!hasAlarm) {
                                hasAlarm = true;
                                values.put(CalendarContract.Events.HAS_ALARM, 1);
                            }
                            reminderValues.put(CalendarContract.Reminders.MINUTES,
                                    (mAlarmTime) / 60000);
                            reminderValues.put(CalendarContract.Reminders.METHOD,
                                    CalendarContract.Reminders.METHOD_ALERT);
                            remindersList.add(reminderValues);
                        } else {
                            alarmTime = prop.propValue_vector.get(0);
                            if ((alarmTime == null) || (alarmTime.length() == 0))
                            {
                            }
                            else
                            {
                                alarmMills = parseTimeStr(alarmTime, mTimeZone);
                                if (alarmMills >= startMills)
                                {
                                }
                                else
                                {
                                    if (!hasAlarm)
                                    {
                                        hasAlarm = true;
                                        values.put(CalendarContract.Events.HAS_ALARM, 1);// set
                                                                                         // alarm
                                    }

                                    reminderValues.put(CalendarContract.Reminders.MINUTES,
                                            (startMills - alarmMills) / 60000);
                                    reminderValues.put(CalendarContract.Reminders.METHOD,
                                            CalendarContract.Reminders.METHOD_ALERT);
                                    remindersList.add(reminderValues);

                                }
                            }
                        }
                    }
                }
            }
        }
        if (isDue) {
            // sunxiaoming add ,when Due exist,then delete the following keys
            values.remove(CalendarContract.Events.DTEND);
            values.remove(CalendarContract.Events.LAST_DATE);
        }

        if (remindersList.size() > 0)
            addReminders(remindersList);

        return values;
    }

    private ContentValues setTodoMap(VNode vnode, String calId) {
        ContentValues values = new ContentValues();
        ArrayList<ContentValues> remindersList = new ArrayList<ContentValues>();
        ContentValues reminderValues = new ContentValues();
        values.put(CalendarContract.Events.HAS_ATTENDEE_DATA, 1);// sunxiaoming
                                                                 // add to set a
                                                                 // default
                                                                 // value for
                                                                 // HAS_ATTENDEE_DATA
        values.put(CalendarContract.Events.CALENDAR_ID, calId);
        // values.put(CalendarContract.Events.EVENT_TYPE,CalendarContract.Events.EVENT_TASK);
        values.put(CalendarContract.Events.HAS_ALARM, 1);// set alarm
        if (mTimeZone != null)
        {
            if (LOCAL_DEBUG)
                Log.d(TAG, "The time zone is :" + mTimeZone);
            TimeZone tz = TimeZone.getTimeZone("GMT" + mTimeZone);
            // TimeZone tz = TimeZone.getTimeZone("America/Los_Angeles");
            // values.put(CalendarContract.Events.EVENT_TIMEZONE,tz.getID());
        }

        if (LOCAL_DEBUG)
            Log.d(TAG, "The event type is task");

        String tempString = "";
        long startMills = 0;
        long endMills = 0;
        String alarmTime = "";
        long alarmMills = 0;
        boolean hasAlarm = false;
        // sunxiaoming add 20120904
        for (PropertyNode prop : vnode.propList) {
            if (prop.propName.equalsIgnoreCase("TZID")) {
                mTimeZone = prop.propValue;
                // values.put(CalendarContract.Events.EVENT_TIMEZONE,mTimeZone);//sxm
                // 20121018
                TimeZone tz1 = TimeZone.getTimeZone(mTimeZone);
                values.put(CalendarContract.Events.EVENT_TIMEZONE, tz1.getID());// zhz
                                                                                // 20121026
                int offset1 = tz1.getRawOffset();
                // mTimeZone = String.valueOf(offset1);
                int p = Math.abs(offset1);

                StringBuilder name = new StringBuilder();

                if (offset1 < 0) {
                    name.append('-');
                } else {
                    name.append('+');
                }

                int hour = p / (60 * 60000);

                if (hour < 10)
                    name.append('0');

                name.append(hour);

                if (vCalVersion == VCalComposer.VERSION_VCAL10_INT)
                    name.append(':');

                int min = p / 60000;
                min %= 60;

                if (min < 10) {
                    name.append('0');
                }
                name.append(min);
                mTimeZone = name.toString();
                break; // zhanghongzhi add 20121026
            }
            else {
                values.put(CalendarContract.Events.EVENT_TIMEZONE, mTimeZone);
            }
        }
        // sunxiaoming add 20120904 End
        for (PropertyNode prop : vnode.propList) {
            if (prop.propValue != null) {
                Time time = new Time();
                TimeZone myTimeZone = TimeZone.getTimeZone(mTimeZone);
                if (prop.propName.equalsIgnoreCase("DESCRIPTION")) {
                    values.put(CalendarContract.Events.DESCRIPTION, prop.propValue);
                } else if (prop.propName.equalsIgnoreCase("DTSTART")) {
                    tempString = prop.propValue;
                    if (tempString.contains("Z")) {
                        startMills = /* parseTimeStr */transitionVCalendarTimeToMillis(tempString,
                                myTimeZone);
                    } else {
                        startMills = parseTimeStr(tempString, mTimeZone);
                    }
                    String temp2 = convertLongToRFC2445DateTime(startMills);
                    values.put(CalendarContract.Events.DTSTART, startMills);
                    // time.parse(prop.propValue);
                    // values.put(Calendar.Events.DTSTART, time.toMillis(false
                    // /* use isDst */));
                }
                // pengshijie add begin for end time 20100928.
                else if (prop.propName.equalsIgnoreCase("DTEND")) {
                    tempString = prop.propValue;
                    if (tempString.contains("Z")) {
                        endMills = /* parseTimeStr */transitionVCalendarTimeToMillis(tempString,
                                myTimeZone);
                    } else {
                        endMills = parseTimeStr(tempString, mTimeZone);
                    }
                    values.put(CalendarContract.Events.DTEND, endMills);
                    // pengshijie add begin for end time 20100928.

                } else if (prop.propName.equalsIgnoreCase("SUMMARY")) {
                    values.put(CalendarContract.Events.TITLE, prop.propValue);
                }
                else if (prop.propName.equalsIgnoreCase("X-ALLDAY")) {
                    values.put(CalendarContract.Events.ALL_DAY, prop.propValue);
                }

                else if (prop.propName.equalsIgnoreCase("DUE")) {
                    values.put(CalendarContract.Events.DURATION, prop.propValue);
                    isTodoDue = true;
                } else if (prop.propName.equalsIgnoreCase("COMPLETED")
                        || prop.propName.equalsIgnoreCase("DTSTAMP")) {// sunxiaoming
                                                                       // add
                                                                       // for
                                                                       // the
                                                                       // chinaMobile
                                                                       // new
                                                                       // standard
                    tempString = prop.propValue;
                    if (tempString.contains("Z")) {
                        endMills = /* parseTimeStr */transitionVCalendarTimeToMillis(tempString,
                                myTimeZone);
                    } else {
                        endMills = parseTimeStr(tempString, mTimeZone);
                    }
                    values.put(CalendarContract.Events.LAST_DATE, endMills);
                    // time.parse(prop.propValue);
                    // values.put(Calendar.Events.LAST_DATE, time.toMillis(false
                    // /* use isDst */));
                }

                else if (prop.propName.equalsIgnoreCase("AALARM"))
                {

                    if (prop.propValue.equalsIgnoreCase("DEFAULT"))
                    {
                        if (!hasAlarm)
                        {
                            hasAlarm = true;
                            values.put(CalendarContract.Events.HAS_ALARM, 1);// set
                                                                             // alarm
                        }
                        if (LOCAL_DEBUG)
                            Log.d(TAG, "Set the default alarm.");
                        reminderValues.put(CalendarContract.Reminders.MINUTES,
                                DEFAULT_ALARM_MINUTES);
                        reminderValues.put(CalendarContract.Reminders.METHOD,
                                CalendarContract.Reminders.METHOD_ALERT);
                        remindersList.add(reminderValues);
                    }
                    else
                    {
                        if (mVcalVersion.equals(VCalParser.VERSION_VCALENDAR20)) {
                            long mAlarmTime = 0;
                            alarmTime = prop.propValue_vector.get(0);
                            mAlarmTime = Long.parseLong(alarmTime);
                            if (!hasAlarm) {
                                hasAlarm = true;
                                values.put(CalendarContract.Events.HAS_ALARM, 1);
                            }
                            reminderValues.put(CalendarContract.Reminders.MINUTES,
                                    (mAlarmTime) / 60000);
                            reminderValues.put(CalendarContract.Reminders.METHOD,
                                    CalendarContract.Reminders.METHOD_ALERT);
                            remindersList.add(reminderValues);
                        } else {
                            alarmTime = prop.propValue_vector.get(0);
                            if ((alarmTime == null) || (alarmTime.length() == 0))
                            {
                            }
                            else
                            {
                                alarmMills = parseTimeStr(alarmTime, mTimeZone);
                                if (alarmMills >= startMills)
                                {
                                }
                                else
                                {
                                    if (!hasAlarm)
                                    {
                                        hasAlarm = true;
                                        values.put(CalendarContract.Events.HAS_ALARM, 1);// set
                                                                                         // alarm
                                    }

                                    if (LOCAL_DEBUG)
                                        Log.d(TAG, "Set alarm time:" + (startMills - alarmMills)
                                                / 60000);

                                    reminderValues.put(CalendarContract.Reminders.MINUTES,
                                            (startMills - alarmMills) / 60000);
                                    reminderValues.put(CalendarContract.Reminders.METHOD,
                                            CalendarContract.Reminders.METHOD_ALERT);
                                    remindersList.add(reminderValues);

                                }
                            }
                        }
                    }
                }
                if (isTodoDue) {
                    // sunxiaoming add ,when Due exist,then delete the following
                    // keys
                    values.remove(CalendarContract.Events.DTEND);
                    values.remove(CalendarContract.Events.LAST_DATE);
                }

            }
        }
        /*
         * if(isTodoDue){ //sunxiaoming add ,when Due exist,then delete the
         * following keys values.remove(Calendar.Events.DTEND);
         * values.remove(Calendar.Events.LAST_DATE); }
         */
        if (remindersList.size() > 0)
            addReminders(remindersList);

        return values;
    }

    public boolean saveVcalendar() {
        return false;
    }

    /****
     * return: full:-2 failure:-1 cancel:0 success:int,the count of insert
     ****/
    public int saveVcalendar_new(boolean fullResult[])
    {
        ContentValues eventValue = new ContentValues();
        Uri uri = null;

        int result = 0;

        int[] count = new int[4];
        // int eventType=Events.EVENT_MEMO;

        // getEventsCount(mResolver,count);

        for (int i = 0; i < mVCalValuesList.size(); i++)
        {

            eventValue = mVCalValuesList.get(i);
            uri = mResolver.insert(CalendarContract.Events.CONTENT_URI, eventValue);
            result++;

            ArrayList<ContentValues> remindersList;

            if (uri != null)
            {
                long eventId = ContentUris.parseId(uri);

                if (LOCAL_DEBUG)
                    Log.d(TAG, "Save event success.");

                remindersList = mRemindersMap.get(i);
                if ((remindersList == null) || (remindersList.size() == 0))
                {
                }
                else
                {
                    if (LOCAL_DEBUG)
                        Log.d(TAG, "Save reminder.");

                    for (ContentValues content : remindersList)
                    {
                        ContentValues alarmValue = new ContentValues();
                        alarmValue.putAll(content);
                        alarmValue.put(CalendarContract.Reminders.EVENT_ID, eventId);
                        mResolver.insert(CalendarContract.Reminders.CONTENT_URI, alarmValue);
                    }
                }
            }

            if (uri == null)
            {
                if (LOCAL_DEBUG)
                    Log.d(TAG, "Insert error!");
                return -1;
            }

        }
        return result;
    }

    private long parseTimeStr(String timeStr, String offsetStr)
    {
        if ((timeStr == null) || (timeStr.length() == 0))
            return -1;
        if (!timeStr.contains("T")) {
            timeStr = timeStr + "T000000";
        }
        Time time1 = new Time();
        boolean isUtc = time1.parse(timeStr);
        long mills = time1.normalize(false);

        if (isUtc)
            return mills;

        if ((offsetStr == null) || (offsetStr.length() == 0))
            return mills;

        if (LOCAL_DEBUG)
            Log.d(TAG, "The offset is :" + offsetStr + ",time is:" + timeStr);

        TimeZone tz = TimeZone.getDefault();
        int offsetMills = 0;
        String hourStr = "";
        int hour = 0;
        String minuteStr = "";
        int minute = 0;
        StringBuilder hSb = new StringBuilder();
        StringBuilder mSb = new StringBuilder();
        String offset = offsetStr.trim();
        String prefix;
        int start = -1;

        boolean convert = false;
        int charNum = 0;
        boolean beginMin = false;
        for (int i = 0; i < offset.length(); i++)
        {
            prefix = offset.substring(i, i + 1);

            if (LOCAL_DEBUG)
                Log.d(TAG, "The prefix is:" + prefix);

            if (i == 0)
            {
                if (prefix.equals("+"))
                {
                    convert = false;
                }
                else if (prefix.equals("-"))
                {
                    convert = true;
                }
                else
                {
                    hSb.append(prefix);
                    charNum++;
                    convert = false;
                }
                continue;
            }
            if (prefix.equals(":"))
            {
                beginMin = true;
                continue;
            }
            if (beginMin)
            {
                mSb.append(prefix);
                continue;
            }
            hSb.append(prefix);
            charNum++;
            if (charNum == 2)
            {
                beginMin = true;
            }
        }

        if (LOCAL_DEBUG)
            Log.d(TAG, "The hour:" + hourStr + ",minute:" + minuteStr);

        hourStr = hSb.toString();
        minuteStr = mSb.toString();

        if (hourStr.length() != 0)
        {
            hour = Integer.parseInt(hourStr);
        }

        if (minuteStr.length() != 0)
        {
            minute = Integer.parseInt(minuteStr);
        }

        if (convert)
            offsetMills -= (hour * 60 * 60 * 1000 + minute * 60 * 1000);
        else
            offsetMills += (hour * 60 * 60 * 1000 + minute * 60 * 1000);

        tz.setRawOffset(offsetMills);

        StringBuilder sb = new StringBuilder();
        sb.append(timeStr).append('Z');
        Time time2 = new Time();
        time2.parse(sb.toString());
        mills = time2.normalize(false);
        mills -= offsetMills;

        return mills;

    }

    private static String pad(long c) {
        if (c >= 10)
            return String.valueOf(c);
        else
            return "0" + String.valueOf(c);
    }

    public static String getVcalName()
    {
        String vcalName = "";
        StringBuilder sb = new StringBuilder();

        long now = System.currentTimeMillis();
        Time time = new Time();
        time.set(now);
        /* Start of pengshijie 2010.6.13 for */
        time.month++;
        /* End of pengshijie 2010.6.13 for */
        vcalName = sb.append(time.year)
                .append(pad(time.month))
                .append(pad(time.monthDay))
                .append(pad(time.hour))
                .append(pad(time.minute))
                .append(pad(time.second))
                .append("_vcal.vcs").toString();

        return vcalName;
    }

    public static boolean vCalIsSingle(Context context, Uri uri)
    {

        ContentResolver contentResolver = context.getContentResolver();
        boolean isSingle = true;

        try {

            FileInputStream ins = (FileInputStream) contentResolver.openInputStream(uri);
            BufferedReader br = new BufferedReader(new InputStreamReader(ins));

            String line = br.readLine();

            boolean inVcal = false;
            boolean inVevent = false;
            boolean inVtodo = false;

            int vCals = 0;
            String normalStr = "";

            while (line != null)
            {
                if (!inVcal)
                {
                    normalStr = normalizeText(line.substring(0));

                    char c = 0;
                    int len1 = normalStr.length();

                    if (len1 == 0)
                    {
                        line = br.readLine();
                        continue;
                    }

                    int index1 = 0;

                    for (index1 = 0; index1 < len1; index1++)
                    {
                        c = normalStr.charAt(index1);
                        if (c == ':') {
                            break;
                        }
                    }

                    String name = normalStr.substring(0, index1);

                    if (name.toUpperCase().trim().equals("BEGIN"))
                    {
                        String vcalName = normalStr.substring(index1 + 1);

                        if (vcalName.toUpperCase().trim().equals("VCALENDAR"))
                        {
                            inVcal = true;

                            if (vCals > 0)
                            {
                                Log.d(TAG, "The vcal count is not single.");
                                isSingle = false;
                                break;
                            }
                        }
                        else
                        {
                            line = br.readLine();
                            continue;
                        }
                    }
                }
                else
                {

                    if ((!inVtodo) && (!inVevent))
                    {
                        // grab the name
                        normalStr = normalizeText(line.substring(0));

                        char c = 0;
                        int len2 = normalStr.length();

                        if (len2 == 0)
                        {
                            line = br.readLine();
                            continue;
                        }

                        int index2 = 0;

                        for (index2 = 0; index2 < len2; index2++)
                        {
                            c = normalStr.charAt(index2);
                            if (c == ':') {
                                break;
                            }
                        }

                        String name = normalStr.substring(0, index2);

                        if (name.toUpperCase().trim().equals("BEGIN"))
                        {
                            String vcalName = normalStr.substring(index2 + 1);

                            if (vcalName.toUpperCase().trim().equals("VEVENT"))
                            {


                                vCals++;
                                if (vCals >= 2)
                                {
                                    Log.d(TAG, "The vcal count is not single");
                                    isSingle = false;
                                    break;
                                }
                                inVevent = true;

                            }
                            else if (vcalName.toUpperCase().trim().equals("VTODO"))
                            {

                                vCals++;
                                if (vCals >= 2)
                                {
                                    Log.d(TAG, "The vcal count is not single");
                                    isSingle = false;
                                    break;
                                }
                                inVtodo = true;
                            }
                            else
                            {
                                line = br.readLine();
                                continue;
                            }

                        }
                        else if (name.toUpperCase().trim().equals("END")) // end:vcalendar
                        {

                            String vcalName = normalStr.substring(index2 + 1);

                            if (vcalName.toUpperCase().trim().equals("VCALENDAR"))
                            {
                                inVcal = false;
                                continue;
                            }

                        }
                        else
                            ;

                    }
                    else
                    {

                        if (inVevent)
                        {
                            // grab the name
                            normalStr = normalizeText(line.substring(0));

                            char c = 0;
                            int len3 = normalStr.length();

                            if (len3 == 0)
                            {
                                line = br.readLine();
                                continue;
                            }

                            int index3 = 0;

                            for (index3 = 0; index3 < len3; index3++)
                            {
                                c = normalStr.charAt(index3);
                                if (c == ':') {
                                    break;
                                }
                            }

                            String name = normalStr.substring(0, index3);
                            if (name.toUpperCase().trim().equals("END"))
                            {
                                String vcalName = normalStr.substring(index3 + 1);

                                if (vcalName.toUpperCase().trim().equals("VEVENT"))
                                {
                                    inVevent = false;
                                    continue;
                                }
                            }
                            else
                            {
                                line = br.readLine();
                                continue;
                            }

                        }

                        if (inVtodo)
                        {
                            // grab the name
                            normalStr = normalizeText(line.substring(0));

                            char c = 0;
                            int len3 = normalStr.length();

                            if (len3 == 0)
                            {
                                line = br.readLine();
                                continue;
                            }

                            int index3 = 0;

                            for (index3 = 0; index3 < len3; index3++)
                            {
                                c = normalStr.charAt(index3);
                                if (c == ':') {
                                    break;
                                }
                            }

                            String name = normalStr.substring(0, index3);
                            if (name.toUpperCase().trim().equals("END"))
                            {
                                String vcalName = normalStr.substring(index3 + 1);

                                if (vcalName.toUpperCase().trim().equals("VTODO"))
                                {
                                    inVtodo = false;
                                }
                            }
                            else
                            {
                                line = br.readLine();
                                continue;
                            }
                        }
                    }
                }

                line = br.readLine();

            }
            br.close();

        } catch (Exception ex)
        {
            Log.d(TAG, "Read data error:" + ex.getMessage());
        }

        return isSingle;

    }

    private static String normalizeText(String text) {

        text = text.replaceAll("\r\n", "\n");

        text = text.replaceAll("\r", "\n");
        text = text.replaceAll("\n ", "");
        text = text.replaceAll("\n", "");
        return text;
    }

    // add by zhangjun from t96
    private String convertRrule(String rruleStr) {

        String convertToRFC2445Str = null;
        String[] ruleList = rruleStr.split(" ");
        int listSize = ruleList.length;
        if (listSize > 0) {
            String recur = ruleList[0];
            if (recur.equalsIgnoreCase("D1")) {
                if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                    convertToRFC2445Str = "FREQ=DAILY;WKST=SU";
                } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                    convertToRFC2445Str = "FREQ=DAILY;WKST=MO";
                } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                    convertToRFC2445Str = "FREQ=DAILY;WKST=SA";
                }
            } else if (recur.equalsIgnoreCase("W1")) {
                if (listSize > 3) {
                    if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=SU;BYDAY=MO,TU,WE,TH,FR";
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=MO;BYDAY=MO,TU,WE,TH,FR";
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=SA;BYDAY=MO,TU,WE,TH,FR";
                    }
                } else if (listSize == 3) {
                    if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=SU;BYDAY=" + ruleList[1];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=MO;BYDAY=" + ruleList[1];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                        convertToRFC2445Str = "FREQ=WEEKLY;WKST=SA;BYDAY=" + ruleList[1];
                    }
                }
            } else if (recur.equalsIgnoreCase("MP1")) {
                if (listSize == 4) {
                    String monthWeekNum = (String) ruleList[1].subSequence(0, 1);
                    if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=SU;BYDAY=" + monthWeekNum
                                + ruleList[2];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=MO;BYDAY=" + monthWeekNum
                                + ruleList[2];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=SA;BYDAY=" + monthWeekNum
                                + ruleList[2];
                    }
                }
            } else if (recur.equalsIgnoreCase("MD1")) {
                if (listSize == 3) {
                    if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=SU;BYMONTHDAY=" + ruleList[1];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=MO;BYMONTHDAY=" + ruleList[1];
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                        convertToRFC2445Str = "FREQ=MONTHLY;WKST=SA;BYMONTHDAY=" + ruleList[1];
                    }
                }
            } else if (recur.equalsIgnoreCase("YM1")) {
                if (listSize == 2) {
                    if (ruleList[listSize - 1].equalsIgnoreCase("#0")) {
                        convertToRFC2445Str = "RRULE:FREQ=YEARLY;WKST=SU";
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#1")) {
                        convertToRFC2445Str = "RRULE:FREQ=YEARLY;WKST=MO";
                    } else if (ruleList[listSize - 1].equalsIgnoreCase("#6")) {
                        convertToRFC2445Str = "RRULE:FREQ=YEARLY;WKST=SA";
                    }
                }
            }
        }
        return convertToRFC2445Str;
    }
}
