/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.data;

import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.io.File;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class AuthDBManager {

    private static final String DB_NAME = "secprotect";

    private static final String DB_FILE = "/data/media/.secprotect/secprotect.db";
    private static final String DB_JOURNAL = "/data/media/.secprotect/secprotect.db-journal";
    private static final String DB_PATH = "/data/media/.secprotect";
    private final boolean NON_STANDARD_PATH = true;
    private static final String TABLE_AUTHACCESS = "authaccess";
    private static final int DB_VERSION = 1;

    public static final String UID = "uid";
    public static final String GID = "gid";
    public static final String PACKAGE_NAME = "package_name";
    public static final String LAST_PATH = "last_path";
    public static final String MODE = "mode";
    public static final String REMEMBER = "remember";
    public static final String TIME = "time";

    static final String AUTHACCESS_TABLE_ITEMS[] = { UID, GID, PACKAGE_NAME,
            LAST_PATH, MODE, REMEMBER, TIME };
    static final char AUTHACCESS_TABLE_ITEMS_TYPE[] = { 'i', 'i', 's', 's',
            'i', 'i', 'l' };

    static final int MODE_DENY = 0;
    static final int MODE_ALLOW = 1;
    static final int MODE_ASK = 2;

    private Context mContext = null;
    // DB's id: 1 2 3..

    private SQLiteDatabase mSQLiteDatabase = null;
    private DatabaseOpenHelper mDatabaseOpenHelper = null;

    private static class DatabaseOpenHelper extends SQLiteOpenHelper {

        DatabaseOpenHelper(Context context) {
            super(context, DB_NAME, null, DB_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            logd("");
            String settingsTable = makeSQLCommand(TABLE_AUTHACCESS,
                    AUTHACCESS_TABLE_ITEMS);
            db.execSQL(settingsTable);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            logd("onUpgrade: " + oldVersion + " ->" + newVersion);
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_AUTHACCESS);
            onCreate(db);
        }

        @Override
        public void onDowngrade(SQLiteDatabase db, int oldVersion,
                int newVersion) {

            logd("onDowngrade: " + oldVersion + " ->" + newVersion);
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_AUTHACCESS);
            onCreate(db);
        }
    }

    public void removeTable(String tableName) {

    }

    public AuthDBManager(Context context) {

        mContext = context;
    }

    private static String makeSQLCommand(String tableName, String[] items) {
        String sqlCommand;
        sqlCommand = "CREATE TABLE " + tableName;
        sqlCommand += " (" + UID + " integer primary key,";

        for (int i = 1; i < items.length; i++) {
            String item = items[i];

            switch (AUTHACCESS_TABLE_ITEMS_TYPE[i]) {
            case 'i':
                sqlCommand += item + " interger";
                break;
            case 'l':
                sqlCommand += item + " long";
                break;
            case 's':
                sqlCommand += item + " varchar";
                break;
            default:
                logd("Unknown type");
                break;
            }
            if (i < items.length - 1) {
                sqlCommand += ",";
            }
        }
        sqlCommand += ")";
        logd(sqlCommand);
        // CREATE TABLE authaccess (_id integer primary key,uid interger,gid
        // interger,package varchar,mode interger,remember interger,time
        // interger)
        return sqlCommand;
    }

    public void openDataBase() throws SQLException {

        logd("");
        mDatabaseOpenHelper = new DatabaseOpenHelper(mContext);
        // create database if not exists
        if (NON_STANDARD_PATH) {
            File dbDir = new File(DB_PATH);
            if (!dbDir.exists()) {
                dbDir.mkdir();
                dbDir.setExecutable(true, false);
                dbDir.setReadable(true, false);
                dbDir.setWritable(true, false);
            }
            mSQLiteDatabase = SQLiteDatabase
                    .openOrCreateDatabase(DB_FILE, null);
            logd(mSQLiteDatabase.getVersion());
            if (mSQLiteDatabase.getVersion() == 0) {
                String settingsTable = makeSQLCommand(TABLE_AUTHACCESS,
                        AUTHACCESS_TABLE_ITEMS);
                mSQLiteDatabase.execSQL(settingsTable);
                mSQLiteDatabase.setVersion(DB_VERSION);
                File dbFile = new File(DB_FILE);
                dbFile.setReadable(true, false);
                dbFile.setWritable(true, false);
                File dbJournal = new File(DB_JOURNAL);
                dbJournal.setReadable(true, false);
                dbJournal.setWritable(true, false);
            }

        } else {
            mSQLiteDatabase = mDatabaseOpenHelper.getWritableDatabase();
        }
    }

    public void closeDataBase() throws SQLException {

        logd("");
        if (NON_STANDARD_PATH) {
            if (mSQLiteDatabase != null && mSQLiteDatabase.isOpen()) {
                mSQLiteDatabase.close();
                mSQLiteDatabase = null;
            }
        } else {
            mDatabaseOpenHelper.close();
        }
    }

    public long insert(AuthInfo authInfo) {
        logd("");
        return mSQLiteDatabase.insert(TABLE_AUTHACCESS, null,
                getContentValues(authInfo));
    }

    public boolean update(AuthInfo authInfo, int uid) {
        return mSQLiteDatabase.update(TABLE_AUTHACCESS,
                getContentValues(authInfo), "uid" + "=" + uid, null) > 0;
    }

    public boolean update(AuthInfo authInfo) {
        return update(authInfo, authInfo.uid);
    }

    public Cursor query(int uid) throws SQLException {

        Cursor mCursor = mSQLiteDatabase.query(false, TABLE_AUTHACCESS, null,
                UID + "=" + uid, null, null, null, null, null);

        if (mCursor != null) {
            mCursor.moveToFirst();
        }
        return mCursor;
    }

    public Cursor query(String selection) throws SQLException {
        // select * from authaccess where id=2 AND package_name='unknown'
        Cursor mCursor = mSQLiteDatabase.query(false, TABLE_AUTHACCESS, null,
                selection, null, null, null, null, null);

        if (mCursor != null) {
            mCursor.moveToFirst();
        }
        return mCursor;
    }

    public Cursor queryAll() {

        logd("");
        return mSQLiteDatabase.query(TABLE_AUTHACCESS, null, null, null, null,
                null, null);
    }

    public boolean delete(int uid) {

        return mSQLiteDatabase.delete(TABLE_AUTHACCESS, UID + "=" + uid, null) > 0;
    }

    public boolean deleteAll() {

        return mSQLiteDatabase.delete(TABLE_AUTHACCESS, null, null) > 0;
    }

    ContentValues getContentValues(AuthInfo authInfo) {

        ContentValues contentValues = new ContentValues();
        contentValues.put(UID, authInfo.uid);
        contentValues.put(GID, authInfo.gid);
        contentValues.put(PACKAGE_NAME, authInfo.packageName);
        contentValues.put(LAST_PATH, authInfo.lastPath);
        contentValues.put(MODE, authInfo.mode);
        contentValues.put(REMEMBER, authInfo.remember);
        contentValues.put(TIME, authInfo.time);
        return contentValues;
    }

    public static AuthInfo getAuthInfoByCursor(Cursor cursor) {
        AuthInfo authInfo = new AuthInfo();
        authInfo.uid = cursor.getInt(cursor.getColumnIndex(UID));
        authInfo.gid = cursor.getInt(cursor.getColumnIndex(GID));
        authInfo.packageName = cursor.getString(cursor
                .getColumnIndex(PACKAGE_NAME));
        authInfo.lastPath = cursor.getString(cursor.getColumnIndex(LAST_PATH));
        authInfo.mode = cursor.getInt(cursor.getColumnIndex(MODE));
        authInfo.remember = cursor.getInt(cursor.getColumnIndex(REMEMBER));
        authInfo.time = cursor.getInt(cursor.getColumnIndex(TIME));
        return authInfo;
    }
}
