/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch.provider;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.database.Cursor;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.timerswitch.R;
import com.android.timerswitch.TimerSwitchConstants;
import com.android.timerswitch.TimerSwitchUtils;

import java.util.Calendar;
import java.util.LinkedList;
import java.util.List;

public final class TimerSwitch implements Parcelable, TimerSwitchContract.SwitchesColumns {
    public static final long INVALID_ID = -1;
    public static final String DEFAULT_SORT_ORDER =
            _ID + " ASC";

    public static final String[] QUERY_COLUMNS = {
            _ID,
            HOUR,
            MINUTES,
            DAYS_OF_WEEK,
            ENABLED,
            SWITCH_TIME,
    };

    private static final int ID_INDEX = 0;
    private static final int HOUR_INDEX = 1;
    private static final int MINUTES_INDEX = 2;
    private static final int DAYS_OF_WEEK_INDEX = 3;
    private static final int ENABLED_INDEX = 4;
    private static final int SWITCH_TIME_INDEX = 5;

    public static final int COLUMN_COUNT = ENABLED_INDEX + 1;

    public static final Parcelable.Creator<TimerSwitch> CREATOR =
            new Parcelable.Creator<TimerSwitch>() {
        public TimerSwitch createFromParcel(Parcel p) {
            return new TimerSwitch(p);
        }

        public TimerSwitch[] newArray(int size) {
            return new TimerSwitch[size];
        }
    };

    public long id;
    public boolean enabled;
    public int hour;
    public int minutes;
    public DaysOfWeek daysOfWeek;
    public long switchtime;

    public TimerSwitch() {
        this(0, 0);
    }

    public TimerSwitch(int hour, int minutes) {
        this(hour, minutes, 0);
    }
    public TimerSwitch(int hour, int minutes, int mBitSet) {
        this(hour, minutes, mBitSet, INVALID_ID);
    }

    public TimerSwitch(int hour, int minutes, int mBitSet, long id) {
        this.id = id;
        this.hour = hour;
        this.minutes = minutes;
        this.enabled = false;
        this.daysOfWeek = new DaysOfWeek(mBitSet);
        this.switchtime = 0;
    }

    public TimerSwitch(Cursor c) {
        id = c.getLong(ID_INDEX);
        enabled = c.getInt(ENABLED_INDEX) == 1;
        hour = c.getInt(HOUR_INDEX);
        minutes = c.getInt(MINUTES_INDEX);
        daysOfWeek = new DaysOfWeek(c.getInt(DAYS_OF_WEEK_INDEX));
        switchtime = c.getLong(SWITCH_TIME_INDEX);
    }

    public TimerSwitch(Parcel p) {
        id = p.readLong();
        enabled = p.readInt() == 1;
        hour = p.readInt();
        minutes = p.readInt();
        daysOfWeek = new DaysOfWeek(p.readInt());
        switchtime = p.readLong();
    }

    public void writeToParcel(Parcel p, int flags) {
        p.writeLong(id);
        p.writeInt(enabled ? 1 : 0);
        p.writeInt(hour);
        p.writeInt(minutes);
        p.writeInt(daysOfWeek.getBitSet());
        p.writeLong(switchtime);
    }

    public int describeContents() {
        return 0;
    }

    public Calendar getTimerSwitchTime() {
        Calendar calendar = Calendar.getInstance();
        calendar.set(Calendar.HOUR_OF_DAY, hour);
        calendar.set(Calendar.MINUTE, minutes);
        return calendar;
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof TimerSwitch)) return false;
        final TimerSwitch other = (TimerSwitch) o;
        return id == other.id;
    }

    @Override
    public int hashCode() {
        return Long.valueOf(id).hashCode();
    }

    @Override
    public String toString() {
        return "Switch{" +
                ", id=" + id +
                ", enabled=" + enabled +
                ", hour=" + hour +
                ", minutes=" + minutes +
                ", daysOfWeek=" + daysOfWeek +
                ", switchTime=" + switchtime + "}";
    }
}
