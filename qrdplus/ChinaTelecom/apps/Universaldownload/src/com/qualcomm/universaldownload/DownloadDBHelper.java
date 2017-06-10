/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/
package com.qualcomm.universaldownload;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DownloadDBHelper extends SQLiteOpenHelper {
    public static final String DB_NAME = "download.db";
    public DownloadDBHelper(Context context) {
        super(context, DB_NAME, null, 1);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE IF NOT EXISTS download_info(" +
                "_id integer PRIMARY KEY AUTOINCREMENT, capability char UNIQUE," +
                " url char, md5 char, state integer, file char," +
                " downloadsize integer, version char ) ");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS download_info");
        onCreate(db);
    }
}
