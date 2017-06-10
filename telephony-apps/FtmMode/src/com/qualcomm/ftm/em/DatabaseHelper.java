/******************************************************************************
 * @file    DatabaseHelper.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.util.Log;

public class DatabaseHelper extends SQLiteOpenHelper {

    private static final int VERSION = 1;

    public DatabaseHelper(Context context, String name, CursorFactory factory, int version) {
        super(context, name, factory, version);
    }

    public DatabaseHelper(Context context, String name){
        this(context, name, VERSION);
    }

    public DatabaseHelper(Context context, String name, int version){
        this(context, name, null, version);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        Log.d("SQLiteOpenHelper", "create databases");
        db.execSQL("create table eminfo(_id INTEGER PRIMARY KEY autoincrement, " +
                "mName varchar(20), mValue long, mSub int, mIsNotImportant int)");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        Log.d("SQLiteOpenHelper", "update databases...");
    }
}
