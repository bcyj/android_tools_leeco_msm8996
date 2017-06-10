/*
 * Copyright (c) 2011, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

import java.io.File;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class DatabaseOperator {
    private static DatabaseOperator sInstance = null;

    private final static String DATABASE_PATH = "/data/data/com.android.qworldclock/databases";
    private final static String DATABASE_NAME = DATABASE_PATH + "/timezone.db";
    private final static String TABLE_NAME = "timezonelist";
    private final static String COLUMN_ID = "_id";
    final static String COLUMN_TIMEZONE_ID = "ID";
    final static String COLUMN_NAME_OR_SEQUENCE = "name_sequence";
    final static String COLUMN_GMT = "GMT";
    final static String COLUMN_CUSTOM = "custom";
    final static int INDEX_ID = 0;
    final static int INDEX_SEQUENCE = 1;
    final static int INDEX_CITY = 1;
    final static int INDEX_TIMEZONE_ID = 2;
    final static int INDEX_GMT = 3;
    final static int INDEX_CUSTOM = 4;

    private final static String SQL_CREATE_TABLE = "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (" + COLUMN_ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                  + COLUMN_TIMEZONE_ID + " TEXT,"
                                                  + COLUMN_NAME_OR_SEQUENCE + " TEXT NOT NULL,"
                                                  + COLUMN_GMT + " TEXT NOT NULL,"
                                                  + COLUMN_CUSTOM + " TEXT NOT NULL)";
    private SQLiteDatabase mDb = null;

    private DatabaseOperator(Context context) {
    }

    public static DatabaseOperator getInstance(Context context) {
        if(null == sInstance) {
            sInstance = new DatabaseOperator(context);
        }
        return sInstance;
    }

    public boolean openDatabase() {
        File file = new File(DATABASE_PATH);
        if(!file.exists() || !file.isDirectory()) {
            file.mkdirs();
        }
        mDb = SQLiteDatabase.openDatabase(DATABASE_NAME, null, SQLiteDatabase.OPEN_READWRITE | SQLiteDatabase.CREATE_IF_NECESSARY);
        if(null == mDb) return false;
        mDb.execSQL(SQL_CREATE_TABLE);  // create table if NOT exists
        return true;
    }

    public void closeDatabase() {
        mDb.close();
    }

    public int insert(String id, String displayNameOrSequence, String gmt, boolean custom) {
        if(null != mDb) {
            ContentValues content = new ContentValues();
            content.put(COLUMN_TIMEZONE_ID, id);
            content.put(COLUMN_NAME_OR_SEQUENCE, displayNameOrSequence);
            content.put(COLUMN_GMT, gmt);
            content.put(COLUMN_CUSTOM, custom ? "TRUE" : "FALSE");
            return (int) (mDb.insert(TABLE_NAME, null, content));
        } else {
            return -1;
        }
    }

    public boolean delete(String _id) {
        String[] arg = new String[1];
        arg[0] = _id;
        return 1 == mDb.delete(TABLE_NAME, "_id=?", arg);
    }

    public boolean update(String _id, String id, String displayName, String gmt, boolean custom) {
        ContentValues value = new ContentValues();
        value.put(COLUMN_TIMEZONE_ID, id);
        value.put(COLUMN_NAME_OR_SEQUENCE, displayName);
        value.put(COLUMN_GMT, gmt);
        value.put(COLUMN_CUSTOM, custom ? "TRUE" : "FALSE");
        String[] arg = new String[1];
        arg[0] = _id;
        return 1 == mDb.update(TABLE_NAME, value, "_id=?", arg);
    }

    public Cursor query(String whereClause) {
        if(null == mDb) {
            return null;
        } else {
            return mDb.query(TABLE_NAME, null, whereClause, null, null, null, null);
        }
    }
}
