/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch.provider;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.android.timerswitch.utils.Log;

class TimerSwitchDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 2;

    // This creates a default timerswich at 7:00 for every day
    private static final String DEFAULT_SWITCH_1 = "(7, 0, 127, 0, 0);";

    // This creates a default timerswich at 23:00 for every day
    private static final String DEFAULT_SWITCH_2 = "(23, 0, 127, 0, 0);";

    // Database and table names
    static final String DATABASE_NAME = "timerswitch.db";
    static final String SWITCHES_TABLE_NAME = "switch_templates";

    private static void createSwitchesTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + SWITCHES_TABLE_NAME + " (" +
                TimerSwitchContract.SwitchesColumns._ID + " INTEGER PRIMARY KEY," +
                TimerSwitchContract.SwitchesColumns.HOUR + " INTEGER NOT NULL, " +
                TimerSwitchContract.SwitchesColumns.MINUTES + " INTEGER NOT NULL, " +
                TimerSwitchContract.SwitchesColumns.DAYS_OF_WEEK + " INTEGER NOT NULL, " +
                TimerSwitchContract.SwitchesColumns.ENABLED + " INTEGER NOT NULL, " +
                TimerSwitchContract.SwitchesColumns.SWITCH_TIME + " INTEGER NOT NULL );");
        Log.i("Switches Table created");
    }

    private Context mContext;

    public TimerSwitchDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        mContext = context;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        createSwitchesTable(db);

        Log.i("Inserting default switches");
        String cs = ", ";
        String insertMe = "INSERT INTO " + SWITCHES_TABLE_NAME + " (" +
                TimerSwitchContract.SwitchesColumns.HOUR + cs +
                TimerSwitchContract.SwitchesColumns.MINUTES + cs +
                TimerSwitchContract.SwitchesColumns.DAYS_OF_WEEK + cs +
                TimerSwitchContract.SwitchesColumns.ENABLED + cs +
                TimerSwitchContract.SwitchesColumns.SWITCH_TIME + ") VALUES ";
        db.execSQL(insertMe + DEFAULT_SWITCH_1);
        db.execSQL(insertMe + DEFAULT_SWITCH_2);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
        if (Log.LOGV) {
            Log.v("Upgrading database from version " + oldVersion + " to " + currentVersion);
        }
        Log.i("Dropping old table");
        db.execSQL("DROP TABLE IF EXISTS " + SWITCHES_TABLE_NAME + ";");
        onCreate(db);
    }
}
