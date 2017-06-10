/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;

public class GesturesStore {

    public static final String TABLE_GESTURES = "gestures";
    public static final String COLUMN_GESTURE_PACKAGE_NAME = "package_name";
    public static final String COLUMN_GESTURE_CLASS_NAME = "class_name";
    public static final String COLUMN_GESTURE_ENABLE = "enable";

    private class SettingsSQLiteOpenHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 1;
        private static final String DATABASE_NAME = "settings.db";

        public SettingsSQLiteOpenHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + TABLE_GESTURES + "(" + BaseColumns._ID
                    + " INTEGER PRIMARY KEY," + COLUMN_GESTURE_PACKAGE_NAME + " text,"
                    + COLUMN_GESTURE_CLASS_NAME + " text," + COLUMN_GESTURE_ENABLE
                    + " COLUMN_ENABLE);");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        }
    }

    private static GesturesStore sInstance;
    private final Context mContext;
    private final SettingsSQLiteOpenHelper mSettingsSQLiteOpenHelper;

    private GesturesStore(Context context) {
        mContext = context;
        mSettingsSQLiteOpenHelper = new SettingsSQLiteOpenHelper(mContext);
    }

    public static final void create(Context context) {
        synchronized (GesturesStore.class) {
            if (sInstance == null) {
                sInstance = new GesturesStore(context);
            }
        }
    }

    public static final GesturesStore getInstance() {
        synchronized (GesturesStore.class) {
            if (sInstance == null) {
                throw new RuntimeException("Settings was not initialized");
            }
            return sInstance;
        }
    }

    public Cursor query(String table, String[] projection, String selection,
            String[] selectionArgs,
            String sortOrder) {
        return mSettingsSQLiteOpenHelper.getReadableDatabase().query(table, projection, selection,
                selectionArgs, null, null, sortOrder);
    }

    public long insert(String table, ContentValues values) {
        return mSettingsSQLiteOpenHelper.getWritableDatabase().insert(table, null, values);
    }

    public int delete(String table, String selection, String[] selectionArgs) {
        return mSettingsSQLiteOpenHelper.getWritableDatabase().delete(table, selection,
                selectionArgs);
    }

    public int update(String table, ContentValues values, String selection, String[] selectionArgs) {
        return mSettingsSQLiteOpenHelper.getWritableDatabase().update(table, values, selection,
                selectionArgs);
    }

}
