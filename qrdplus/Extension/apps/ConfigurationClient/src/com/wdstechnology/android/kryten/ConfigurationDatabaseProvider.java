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

import java.util.HashMap;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.util.Log;

public class ConfigurationDatabaseProvider extends ContentProvider {

    private static final String TAG = "CPDatabaseProvider";
    private static final String DATABASE_NAME = "CPMessage.db";
    private static final int DATABASE_VERSION = 1;
    private static final String TABLE_NAME = "CP";
    public static final String AUTHORITY = "com.wdstechnology.android.kryten.CPDatabaseProvider";
    private static final UriMatcher sUriMatcher;
    private static final int CP = 1;
    private static HashMap<String, String> messageProjectionMap;
    public static final Uri CONTENT_URI = Uri.parse("content://"
            + AUTHORITY + "/" + TABLE_NAME);
    public static final String CONTENT_TYPE = "vnd.android.cursor.dir/message";

    public static class Columns {
        public final static String _ID = "_id";
        public final static String NAME = "name";
        public final static String NUMERIC = "numeric";
        public final static String MCC = "mcc";
        public final static String MNC = "mnc";
        public final static String APN = "apn";
        public final static String USER = "user";
        public final static String SERVER = "server";
        public final static String PASSWORD = "password";
        public final static String PROXY = "proxy";
        public final static String PORT = "port";
        public final static String MMSPROXY = "mmsproxy";
        public final static String MMSPORT = "mmsport";
        public final static String MMSC = "mmsc";
        public final static String AUTHTYPE = "authtype";
        public final static String TYPE = "type";
        public final static String CURRENT = "current";
        public final static String PROTOCOL = "protocol";
        public final static String ROAMING_PROTOCOL = "roaming_protocol";
        public final static String CARRIER_ENABLED = "carrier_enabled";
        public final static String BEARER = "bearer";
        public final static String DATE = "date";
        public final static String SIM = "sim";
        public final static String MAC = "mac";
        public final static String SEC = "sec";
        public final static String DOC = "doc";
        public final static String INSTALL = "install";
        //Email
        public final static String INBOUND_SERVER_URI = "inbound_server_uri";
        public final static String OUTBOUND_SERVER_URI = "outbound_server_uri";
        public final static String INBOUND_SERVICE = "inbound_service";
        public final static String OUTBOUND_SERVICE = "outbound_service";
        public final static String INBOUND_PORTNBR = "inbound_portnbr";
        public final static String OUTBOUND_PORTNBR = "outbound_portnbr";
        public final static String EMAIL_DISPLAYNAME = "email_displayname";
        public final static String EMAIL_ID = "email_id";
        public final static String EMAIL_PASSWORD = "email_password";

        public final static String APPID_BROWSER = "appidBrowser";
        public final static String APPID_MMS = "appidMms";
        public final static String APPID_POP3 = "appidPop3";
        public final static String APPID_IMAP4 = "appidImap4";
        public final static String APPID_SMTP = "appidSmtp";

    }

    static {
        sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        sUriMatcher.addURI(AUTHORITY, TABLE_NAME, CP);

        messageProjectionMap = new HashMap<String, String>();
        messageProjectionMap.put(Columns._ID, Columns._ID);
        messageProjectionMap.put(Columns.NAME, Columns.NAME);
        messageProjectionMap.put(Columns.NUMERIC, Columns.NUMERIC);
        messageProjectionMap.put(Columns.MCC, Columns.MCC);
        messageProjectionMap.put(Columns.MNC, Columns.MNC);
        messageProjectionMap.put(Columns.APN, Columns.APN);
        messageProjectionMap.put(Columns.USER, Columns.USER);
        messageProjectionMap.put(Columns.SERVER, Columns.SERVER);
        messageProjectionMap.put(Columns.PASSWORD, Columns.PASSWORD);
        messageProjectionMap.put(Columns.PROXY, Columns.PROXY);
        messageProjectionMap.put(Columns.PORT, Columns.PORT);
        messageProjectionMap.put(Columns.MMSPROXY, Columns.MMSPROXY);
        messageProjectionMap.put(Columns.MMSPORT, Columns.MMSPORT);
        messageProjectionMap.put(Columns.MMSC, Columns.MMSC);
        messageProjectionMap.put(Columns.AUTHTYPE, Columns.AUTHTYPE);
        messageProjectionMap.put(Columns.TYPE, Columns.TYPE);
        messageProjectionMap.put(Columns.CURRENT, Columns.CURRENT);
        messageProjectionMap.put(Columns.PROTOCOL, Columns.PROTOCOL);
        messageProjectionMap.put(Columns.ROAMING_PROTOCOL, Columns.ROAMING_PROTOCOL);
        messageProjectionMap.put(Columns.CARRIER_ENABLED, Columns.CARRIER_ENABLED);
        messageProjectionMap.put(Columns.BEARER, Columns.BEARER);
        messageProjectionMap.put(Columns.DATE, Columns.DATE);
        messageProjectionMap.put(Columns.SIM, Columns.SIM);
        messageProjectionMap.put(Columns.MAC, Columns.MAC);
        messageProjectionMap.put(Columns.SEC, Columns.SEC);
        messageProjectionMap.put(Columns.DOC, Columns.DOC);
        messageProjectionMap.put(Columns.INSTALL, Columns.INSTALL);
        //Email
        messageProjectionMap.put(Columns.INBOUND_SERVER_URI, Columns.INBOUND_SERVER_URI);
        messageProjectionMap.put(Columns.OUTBOUND_SERVER_URI, Columns.OUTBOUND_SERVER_URI);
        messageProjectionMap.put(Columns.INBOUND_SERVICE, Columns.INBOUND_SERVICE);
        messageProjectionMap.put(Columns.OUTBOUND_SERVICE, Columns.OUTBOUND_SERVICE);
        messageProjectionMap.put(Columns.INBOUND_PORTNBR, Columns.INBOUND_PORTNBR);
        messageProjectionMap.put(Columns.OUTBOUND_PORTNBR, Columns.OUTBOUND_PORTNBR);
        messageProjectionMap.put(Columns.EMAIL_DISPLAYNAME, Columns.EMAIL_DISPLAYNAME);
        messageProjectionMap.put(Columns.EMAIL_ID, Columns.EMAIL_ID);
        messageProjectionMap.put(Columns.EMAIL_PASSWORD, Columns.EMAIL_PASSWORD);

        messageProjectionMap.put(Columns.APPID_BROWSER, Columns.APPID_BROWSER);
        messageProjectionMap.put(Columns.APPID_MMS, Columns.APPID_MMS);
        messageProjectionMap.put(Columns.APPID_POP3, Columns.APPID_POP3);
        messageProjectionMap.put(Columns.APPID_IMAP4, Columns.APPID_IMAP4);
        messageProjectionMap.put(Columns.APPID_SMTP, Columns.APPID_SMTP);
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {
        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + TABLE_NAME + " ("
                    + Columns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                    + Columns.NAME + " TEXT, "
                    + Columns.NUMERIC + " TEXT,"
                    + Columns.MCC + " TEXT,"
                    + Columns.MNC + " TEXT, "
                    + Columns.APN + " TEXT, "
                    + Columns.USER + " TEXT,"
                    + Columns.SERVER + " TEXT,"
                    + Columns.PASSWORD + " TEXT, "
                    + Columns.PROXY + " TEXT, "
                    + Columns.PORT + " TEXT,"
                    + Columns.MMSPROXY + " TEXT,"
                    + Columns.MMSPORT + " TEXT,"
                    + Columns.MMSC + " TEXT,"
                    + Columns.AUTHTYPE + " TEXT,"
                    + Columns.TYPE + " TEXT,"
                    + Columns.CURRENT + " INTEGER,"
                    + Columns.PROTOCOL + " TEXT, "
                    + Columns.ROAMING_PROTOCOL + " TEXT,"
                    + Columns.CARRIER_ENABLED + " BOOLEAN,"
                    + Columns.BEARER + " INTEGER,"
                    + Columns.DATE + " INTEGER,"
                    + Columns.SIM + " TEXT,"
                    + Columns.MAC + " TEXT,"
                    + Columns.SEC + " TEXT,"
                    + Columns.DOC + " BLOB,"
                    + Columns.INSTALL + " BOOLEAN, "
                    + Columns.INBOUND_SERVER_URI + " TEXT, "
                    + Columns.OUTBOUND_SERVER_URI + " TEXT, "
                    + Columns.INBOUND_SERVICE + " TEXT, "
                    + Columns.OUTBOUND_SERVICE + " TEXT, "
                    + Columns.INBOUND_PORTNBR + " INTEGER, "
                    + Columns.OUTBOUND_PORTNBR + " INTEGER, "
                    + Columns.EMAIL_DISPLAYNAME + " TEXT, "
                    + Columns.EMAIL_ID + " TEXT, "
                    + Columns.EMAIL_PASSWORD + " TEXT, "
                    + Columns.APPID_BROWSER + " TEXT,"
                    + Columns.APPID_MMS + " TEXT,"
                    + Columns.APPID_POP3 + " TEXT,"
                    + Columns.APPID_IMAP4 + " TEXT,"
                    + Columns.APPID_SMTP + " TEXT "
                    + ");");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion
                    + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAME);
            onCreate(db);
        }
    }

    private DatabaseHelper dbHelper;

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        // TODO Auto-generated method stub
        SQLiteDatabase db = dbHelper.getWritableDatabase();
        int count;
        switch (sUriMatcher.match(uri)) {
            case CP:
                count = db.delete(TABLE_NAME, where, whereArgs);
                break;

            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public String getType(Uri uri) {
        // TODO Auto-generated method stub
        switch (sUriMatcher.match(uri)) {
            case CP:
                return CONTENT_TYPE;
            default:
                return null;
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        // TODO Auto-generated method stub
        if (sUriMatcher.match(uri) != CP) {
            throw new IllegalArgumentException("Unknown URI " + uri);
        }
        ContentValues values;
        if (initialValues != null) {
            values = new ContentValues(initialValues);
        } else {
            return null;
        }

        SQLiteDatabase db = dbHelper.getWritableDatabase();
        long rowId = db.insert(TABLE_NAME, null, values);
        if (rowId > 0) {
            Uri messageUri = ContentUris.withAppendedId(CONTENT_URI, rowId);
            getContext().getContentResolver().notifyChange(messageUri, null);
            return messageUri;
        }

        throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public boolean onCreate() {
        // TODO Auto-generated method stub
        dbHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        // TODO Auto-generated method stub
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        switch (sUriMatcher.match(uri)) {
            case CP:
                qb.setTables(TABLE_NAME);
                qb.setProjectionMap(messageProjectionMap);
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
        SQLiteDatabase db = dbHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, sortOrder);
        c.setNotificationUri(getContext().getContentResolver(), uri);
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where,
            String[] whereArgs) {
        // TODO Auto-generated method stub
        SQLiteDatabase db = dbHelper.getWritableDatabase();
        int count;
        switch (sUriMatcher.match(uri)) {
            case CP:
                count = db.update(TABLE_NAME, values, where, whereArgs);
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

}
