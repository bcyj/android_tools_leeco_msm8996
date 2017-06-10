/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.timerswitch;

import android.app.AlarmManager;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Parcel;
import android.text.format.DateFormat;
import android.widget.Toast;

import com.android.datetimepicker.time.TimePickerDialog;
import com.android.timerswitch.provider.DaysOfWeek;
import com.android.timerswitch.provider.TimerSwitch;
import com.android.timerswitch.provider.TimerSwitchContract;
import com.android.timerswitch.utils.Log;
import com.android.timerswitch.R;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

/**
 * Static utility methods for TimerSwitch.
 */
public class TimerSwitchUtils {
    public static final String FRAG_TAG_TIME_PICKER = "time_dialog";
    public static final String TIMER_SWTICH_RAW_DATA = "intent.extra.timer_switch_raw";
    public static final String TIMER_SWITCH_MANAGER_TAG = "TIMER_SWITCH_MANAGER";
    public static final String TIMER_SWITCH_STATE_EXTRA = "intent.extra.timerswitch.state";

    public static String getFormattedTime(Context context, Calendar time) {
        String skeleton = DateFormat.is24HourFormat(context) ? "EHm" : "Ehma";
        String pattern = DateFormat.getBestDateTimePattern(Locale.getDefault(), skeleton);
        return (String) DateFormat.format(pattern, time);
    }

    public static String getTimerSwitchTimeText(Context context, TimerSwitch instance) {
        String switchTimeStr = getFormattedTime(context, instance.getTimerSwitchTime());
        return switchTimeStr;
    }

    public static void showTimeEditDialog(FragmentManager manager, final TimerSwitch timerSwitch,
            TimePickerDialog.OnTimeSetListener listener, boolean is24HourMode) {

        int hour = (timerSwitch == null) ? 0: timerSwitch.hour;
        int minutes = (timerSwitch == null) ? 0: timerSwitch.minutes;

        TimePickerDialog dialog = TimePickerDialog.newInstance(listener,
                hour, minutes, is24HourMode);
        //dialog.setThemeDark(true);

        // Make sure the dialog isn't already added.
        manager.executePendingTransactions();
        final FragmentTransaction ft = manager.beginTransaction();
        final Fragment prev = manager.findFragmentByTag(FRAG_TAG_TIME_PICKER);
        if (prev != null) {
            ft.remove(prev);
        }
        ft.commit();

        if (dialog != null && !dialog.isAdded()) {
            dialog.show(manager, FRAG_TAG_TIME_PICKER);
        }
    }

    /**
     * format "switch set for 2 days 7 hours and 53 minutes from now"
     */
    private static String formatToast(Context context, long timeInMillis, long switchId) {
        long delta = timeInMillis - System.currentTimeMillis();
        long hours = delta / (1000 * 60 * 60);
        long minutes = delta / (1000 * 60) % 60;
        long days = hours / 24;
        hours = hours % 24;

        String daySeq = (days == 0) ? "" :
                (days == 1) ? context.getString(R.string.day) :
                        context.getString(R.string.days, Long.toString(days));

        String minSeq = (minutes == 0) ? "" :
                (minutes == 1) ? context.getString(R.string.minute) :
                        context.getString(R.string.minutes, Long.toString(minutes));

        String hourSeq = (hours == 0) ? "" :
                (hours == 1) ? context.getString(R.string.hour) :
                        context.getString(R.string.hours, Long.toString(hours));

        String switchSeq = (switchId == 1) ? context.getString(R.string.switch_on) :
                context.getString(R.string.switch_off);

        boolean dispDays = days > 0;
        boolean dispHour = hours > 0;
        boolean dispMinute = minutes > 0;

        int index = (dispDays ? 1 : 0) |
                (dispHour ? 2 : 0) |
                (dispMinute ? 4 : 0);

        String[] formats = context.getResources().getStringArray(R.array.switch_set);
        return String.format(formats[index], switchSeq, daySeq, hourSeq, minSeq);
    }

    public static void popAlarmSetToast(Context context, long timeInMillis, long switchId) {
        String toastText = formatToast(context, timeInMillis, switchId);
        Toast toast = Toast.makeText(context, toastText, Toast.LENGTH_LONG);
        ToastMaster.setToast(toast);
        toast.show();
    }

    static Calendar calculateTimerSwitch(int hour, int minute, DaysOfWeek daysOfWeek) {

        // start with now
        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int nowHour = c.get(Calendar.HOUR_OF_DAY);
        int nowMinute = c.get(Calendar.MINUTE);

        // if timerswitch is behind current time, advance one day
        if (hour < nowHour || hour == nowHour && minute <= nowMinute) {
            c.add(Calendar.DAY_OF_YEAR, 1);
        }
        c.set(Calendar.HOUR_OF_DAY, hour);
        c.set(Calendar.MINUTE, minute);
        c.set(Calendar.SECOND, 0);
        c.set(Calendar.MILLISECOND, 0);

        int addDays = daysOfWeek.calculateDaysToNextAlarm(c);
        if (addDays > 0) {
            c.add(Calendar.DAY_OF_WEEK, addDays);
        }
        return c;
    }

    public static Calendar createInstanceAfter(TimerSwitch timerSwitch, Calendar time) {
        Calendar nextInstanceTime = Calendar.getInstance();
        nextInstanceTime.set(Calendar.YEAR, time.get(Calendar.YEAR));
        nextInstanceTime.set(Calendar.MONTH, time.get(Calendar.MONTH));
        nextInstanceTime.set(Calendar.DAY_OF_MONTH, time.get(Calendar.DAY_OF_MONTH));
        nextInstanceTime.set(Calendar.HOUR_OF_DAY, timerSwitch.hour);
        nextInstanceTime.set(Calendar.MINUTE, timerSwitch.minutes);
        nextInstanceTime.set(Calendar.SECOND, 0);
        nextInstanceTime.set(Calendar.MILLISECOND, 0);

        // If we are still behind the passed in time, then add a day
        if (nextInstanceTime.getTimeInMillis() <= time.getTimeInMillis()) {
            nextInstanceTime.add(Calendar.DAY_OF_YEAR, 1);
        }

        // The day of the week might be invalid, so find next valid one
        int addDays = timerSwitch.daysOfWeek.calculateDaysToNextAlarm(nextInstanceTime);
        if (addDays > 0) {
            nextInstanceTime.add(Calendar.DAY_OF_WEEK, addDays);
        }

        return nextInstanceTime;
    }

    public static ContentValues createContentValues(TimerSwitch timerSwitch) {
        ContentValues values = new ContentValues(TimerSwitch.COLUMN_COUNT);
        if (timerSwitch.id != TimerSwitch.INVALID_ID) {
            values.put(TimerSwitch._ID, timerSwitch.id);
        }

        values.put(TimerSwitch.ENABLED, timerSwitch.enabled ? 1 : 0);
        values.put(TimerSwitch.HOUR, timerSwitch.hour);
        values.put(TimerSwitch.MINUTES, timerSwitch.minutes);
        values.put(TimerSwitch.DAYS_OF_WEEK, timerSwitch.daysOfWeek.getBitSet());
        values.put(TimerSwitch.SWITCH_TIME, timerSwitch.switchtime);

        return values;
    }

    public static Uri getUri(long switchId) {
        return ContentUris.withAppendedId(TimerSwitch.CONTENT_URI, switchId);
    }

    public static long getId(Uri contentUri) {
        return ContentUris.parseId(contentUri);
    }

    /**
     * Get switch cursor loader for all timerswitches.
     */
    public static CursorLoader getAlarmsCursorLoader(Context context) {
        return new CursorLoader(context, TimerSwitch.CONTENT_URI,
                TimerSwitch.QUERY_COLUMNS, null, null, TimerSwitch.DEFAULT_SORT_ORDER);
    }

    /**
     * Get switch by id.
     */
    public static TimerSwitch getTimerSwitchById(ContentResolver contentResolver, long switchId) {
        Cursor cursor = contentResolver.query(getUri(switchId), TimerSwitch.QUERY_COLUMNS, null,
                null, null);
        TimerSwitch result = null;
        if (cursor == null) {
            return result;
        }

        try {
            if (cursor.moveToFirst()) {
                result = new TimerSwitch(cursor);
            }
        } finally {
            cursor.close();
        }

        return result;
    }

    /**
     * Get all switch given conditions.
     */
    public static List<TimerSwitch> getTimerSwitches(ContentResolver contentResolver,
            String selection, String... selectionArgs) {
        Cursor cursor = contentResolver.query(TimerSwitch.CONTENT_URI, TimerSwitch.QUERY_COLUMNS,
                selection, selectionArgs, null);
        List<TimerSwitch> result = new LinkedList<TimerSwitch>();
        if (cursor == null) {
            return result;
        }

        try {
            if (cursor.moveToFirst()) {
                do {
                    result.add(new TimerSwitch(cursor));
                } while (cursor.moveToNext());
            }
        } finally {
            cursor.close();
        }

        return result;
    }

    public static boolean updateTimerSwitch(ContentResolver contentResolver, TimerSwitch timerSwitch) {
        Log.d("updateTimerSwitch:" + timerSwitch.id);
        if (timerSwitch.id == TimerSwitch.INVALID_ID) {
            return false;
        }
        ContentValues values = createContentValues(timerSwitch);
        long rowsUpdated = contentResolver.update(getUri(timerSwitch.id), values, null, null);
        return rowsUpdated == 1;
    }

    public static void resetTimerSwitch(Context mContext, TimerSwitch timerSwitch) {
        if (timerSwitch.id == TimerSwitch.INVALID_ID)
            return;

        TimerSwitch defTimerSwitch = null;
        if (timerSwitch.id == TimerSwitchConstants.SWITCH_ON) {
            defTimerSwitch = new TimerSwitch(TimerSwitchConstants.DEFAULT_ON_HOUR,
                    TimerSwitchConstants.DEFAULT_ON_MINUTES,
                    TimerSwitchConstants.DEFAULT_ON_DAYOFWEEK, TimerSwitchConstants.SWITCH_ON);
        } else if (timerSwitch.id == TimerSwitchConstants.SWITCH_OFF){
            defTimerSwitch = new TimerSwitch(TimerSwitchConstants.DEFAULT_OFF_HOUR,
                    TimerSwitchConstants.DEFAULT_OFF_MINUTES,
                    TimerSwitchConstants.DEFAULT_OFF_DAYOFWEEK, TimerSwitchConstants.SWITCH_OFF);
        }
        if (updateTimerSwitch(mContext.getContentResolver(), defTimerSwitch)) {
            return;
        }
        disableTimerSwitch(mContext, timerSwitch);
    }

    public static void deleteAllExpiredTimerSwitch(Context context, long switchId) {
        Log.d("deleteAllExpiredTimerSwitch:" + switchId);
        ContentResolver cr = context.getContentResolver();
        TimerSwitch timerSwitch = getTimerSwitchById(cr, switchId);
        disableTimerSwitch(context, timerSwitch);
    }

    public static void disableExpiredTimerSwitch(final Context context) {
        final ContentResolver contentResolver = context.getContentResolver();
        List<TimerSwitch> timerSwitches = getTimerSwitches(contentResolver, null);
        long now = System.currentTimeMillis();
        for (TimerSwitch timerSwitch : timerSwitches) {
            if (timerSwitch.enabled) {
                if (!timerSwitch.daysOfWeek.isRepeating() && timerSwitch.switchtime < now) {
                    disableTimerSwitch(context, timerSwitch);
                    timerSwitch.enabled = false;
                    updateTimerSwitch(contentResolver, timerSwitch);
                }
            }
        }
    }

    public static void setNextTimerSwitch(final Context context) {
        final ContentResolver contentResolver = context.getContentResolver();
        long now = System.currentTimeMillis();
        List<TimerSwitch> timerSwitches = getTimerSwitches(contentResolver, null);
        for (TimerSwitch timerSwitch : timerSwitches) {
            if (timerSwitch.enabled) {
                // dismiss expired
                disableTimerSwitch(context, timerSwitch);
                enableTimerSwitch(context, timerSwitch);
            }
        }
    }

    public static void enableTimerSwitch(final Context mContext, final TimerSwitch timerSwitch) {
        // update in-day timerswitch
        enableTimerSwitchInternal(mContext.getContentResolver(), timerSwitch);
        switch ((int) timerSwitch.id) {
            case TimerSwitchConstants.SWITCH_ON:
                registerNextTimerSwitchOn(mContext, timerSwitch);
                break;

            case TimerSwitchConstants.SWITCH_OFF:
                registerNextTimerSwitchOff(mContext, timerSwitch);
                break;
        }
    }

    public static void disableTimerSwitch(final Context mContext, final TimerSwitch timerSwitch) {
        String mAction = null;
        switch ((int) timerSwitch.id) {
            case TimerSwitchConstants.SWITCH_ON:
                Log.d("DisableTimerSwitchON");
                mAction = TimerSwitchConstants.ACTION_POWER_ON;
                break;

            case TimerSwitchConstants.SWITCH_OFF:
                Log.d("DisableTimerSwitchOFF");
                mAction = TimerSwitchConstants.ACTION_POWER_OFF;
                break;
        }
        unregisterNextTimerSwitch(mContext, timerSwitch, mAction);
    }

    public static void enableTimerSwitchInternal(final ContentResolver contentResolver,
            final TimerSwitch timerSwitch) {

        if (timerSwitch.enabled) {
            timerSwitch.switchtime = calculateTimerSwitch(timerSwitch.hour, timerSwitch.minutes,
                        timerSwitch.daysOfWeek).getTimeInMillis();
        }
        updateTimerSwitch(contentResolver, timerSwitch);
    }

    public static void unregisterNextTimerSwitch(final Context mContext,
            final TimerSwitch timerSwitch, String action) {

        AlarmManager am = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        Intent intent = createTimerSwitchIntent(mContext, timerSwitch, action);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0,
                intent, PendingIntent.FLAG_UPDATE_CURRENT);
        if (pendingIntent != null) {
            am.cancel(pendingIntent);
        }
    }

    public static Intent createTimerSwitchIntent(Context mContext, TimerSwitch timerSwitch,
            String action) {
        Intent intent = new Intent(mContext, TimerSwitchReceiver.class);
        intent.setAction(action);
        Parcel out = unparseTimerSwitch(timerSwitch);
        if (out != null) {
            intent.putExtra(TIMER_SWTICH_RAW_DATA, out.marshall());
        }
        return intent;
    }

    public static void registerNextTimerSwitchOn(final Context mContext,
            final TimerSwitch timerSwitch) {
        AlarmManager am = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);

        Intent onIntent = createTimerSwitchIntent(mContext, timerSwitch,
                TimerSwitchConstants.ACTION_POWER_ON);
        PendingIntent onPendingIntent = PendingIntent.getBroadcast(mContext, 0,
                onIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        LogFormatTime(timerSwitch.switchtime, "ON");
        am.setExact(AlarmManager.RTC_POWEROFF_WAKEUP, timerSwitch.switchtime,
                onPendingIntent);
    }

    public static void registerNextTimerSwitchOff(final Context mContext, TimerSwitch timerSwitch) {
        AlarmManager am = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        Intent intent = createTimerSwitchIntent(mContext, timerSwitch,
                TimerSwitchConstants.ACTION_POWER_OFF);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0,
                intent, PendingIntent.FLAG_UPDATE_CURRENT);

        LogFormatTime(timerSwitch.switchtime, "OFF");
        am.setExact(AlarmManager.RTC_WAKEUP, timerSwitch.switchtime, pendingIntent);
    }

    public static void LogFormatTime(long time, String state) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Log.d("enableNextTimerSwitch" + state + ":" + sdf.format(time));
    }

    public static String getFormatTime(long time) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return sdf.format(time);
    }

    public static Parcel unparseTimerSwitch(TimerSwitch timerSwitch) {
        Parcel out = Parcel.obtain();
        timerSwitch.writeToParcel(out, 0);
        out.setDataPosition(0);
        return out;
    }

    public static TimerSwitch parseTimerSwitchFromIntent(final Intent intent) {
        TimerSwitch timerSwitch = null;
        final byte[] data = intent.getByteArrayExtra(TIMER_SWTICH_RAW_DATA);
        if (data != null) {
            Parcel in = Parcel.obtain();
            in.unmarshall(data, 0, data.length);
            in.setDataPosition(0);
            timerSwitch = TimerSwitch.CREATOR.createFromParcel(in);
        }

        return timerSwitch;
    }

    public static void writeFile(Context context, String filename, String content) {
        try {
            FileOutputStream fos = context.openFileOutput(filename, context.MODE_APPEND);
            PrintStream ps = new PrintStream(fos);
            ps.println(content);
            ps.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
