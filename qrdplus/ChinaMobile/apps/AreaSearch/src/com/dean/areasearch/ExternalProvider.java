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
package com.dean.areasearch;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

public class ExternalProvider extends ContentProvider {

    private static final String DATABASE_NAME = "acnumber.db";
    private static final int DATABASE_VERSION = 5;
    static SQLiteDatabase db = null;
    static SQLiteDatabase db1 = null;

    private static final UriMatcher sUriMatcher;

    static {
        sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        sUriMatcher.addURI("externalareasearch", "area_info", 1);
        sUriMatcher.addURI("externalareasearch", "mobile", 2);
        sUriMatcher.addURI("externalareasearch", "international", 3);
        sUriMatcher.addURI("externalareasearch", "cvi", 4);
    }

    static SharedPreferences pref;
    Context ctx;

    /**
     * This class helps open, create, and upgrade the database file.
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);

        }

        @Override
        public void onCreate(SQLiteDatabase db) {

        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

        }
    }

    /* end of class DatabaseHelper */

    @Override
    public boolean onCreate() {
        ctx = getContext();
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {

        try {

            pref = this.getContext().getSharedPreferences("pref",
                    getContext().MODE_PRIVATE);

            if (!pref.contains("FIRST_RUN")) {

                return null;
            }

            SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

            //add for cvi
            if (sUriMatcher.match(uri)==4)
            {
                qb.setTables("cvi");
                if (db == null) {
                    db = SQLiteDatabase
                            .openOrCreateDatabase(
                                    "data/data/com.dean.areasearch/files/acnumber.db",
                                    null, null);
                }

                String mcc = "mcc='" + selection + "'";
                Cursor c = qb.query(db, new String[] { "code" }, mcc, selectionArgs,
                        null, null, null);

                c.setNotificationUri(getContext().getContentResolver(), uri);

                return c;

            }

            if (selection == null) {
                switch (sUriMatcher.match(uri)) {
                case 1:
                    qb.setTables("area_info");
                    break;

                case 2:
                    qb.setTables("mobile");
                    break;

                case 3:
                    qb.setTables("international");
                    break;

                default:
                    throw new IllegalArgumentException("Unknown URI " + uri);
                }
                if (db == null) {
                    db = SQLiteDatabase
                            .openOrCreateDatabase(
                                    "data/data/com..areasearch/files/acnumber.db",
                                    null, null);
                }
                Cursor c = qb.query(db, projection, selection, selectionArgs,
                        null, null, null);

                c.setNotificationUri(getContext().getContentResolver(), uri);

                return c;
            }

            else if ((selection != null) && ((selection.length() >= 2))) {
                // custom number
                String temp = selection;

                if ((selection.length() >= 10)
                        && selection.substring(0, 2).equals("86")
                        && (!selection.equals("86"))) {

                    temp = selection.substring(2);

                    if (temp.substring(0, 2).equals("10")) {
                        temp = "0" + temp;
                    } else if (temp.charAt(0) != '1') {
                        temp = "0" + temp;
                    }

                }

                if ((selection.length() >= 5)
                        && selection.substring(0, 3).equals("+86")
                        && (!selection.equals("+86"))) {

                    temp = selection.substring(3);

                    if (temp.substring(0, 2).equals("10")) {
                        temp = "0" + temp;
                    } else if (temp.charAt(0) != '1') {
                        temp = "0" + temp;
                    }

                }

                if ((selection.length() >= 6)
                        && selection.substring(0, 4).equals("0086")
                        && (!selection.equals("0086"))) {
                    temp = selection.substring(4);

                    if (temp.substring(0, 2).equals("10")) {
                        temp = "0" + temp;
                    } else if (temp.charAt(0) != '1') {
                        temp = "0" + temp;
                    }
                }

                selection = temp;

                // search custom number start
                if (new File(
                        "data/data/com.dean.areasearch/databases/custom_area_codes.db")
                        .exists()) {


                    String queryStr1 = "code='" + selection + "'";
                    qb.setTables("areacodes");



                    if ((selection.charAt(0) == '0')
                            && (selection.charAt(1) != '0')
                            && (selection.length() > 2)
                            && (selection.length() < 13)) {

                        if ((selection.charAt(1) == '1')
                                || (selection.charAt(1) == '2')) {
                            selection = selection.substring(0, 3);
                        }

                        else if (selection.length() > 3) {
                            selection = selection.substring(0, 4);

                        }

                        String queryStr = "code='" + selection + "'";
                        if(db1==null){
                            DBHelper custommOpenHelper = new DBHelper(getContext());
                            db1 = custommOpenHelper.getReadableDatabase();
                            }
                        Cursor c2 = qb.query(db1, new String[] { "cityname" },
                                queryStr, selectionArgs, null, null, null);
                        c2.setNotificationUri(
                                getContext().getContentResolver(), uri);

                        if (c2.getCount()==0) {

                            c2.close();

                        } else {
                            return c2;

                        }
                    } else if ((selection.charAt(0) == '1')
                            && (selection.length() < 12)
                            && (selection.length() > 7)) {

                         if (selection.equals("13800138000"))
                         {
                             return null;
                         }

                        selection = selection.substring(0, 8);
                        String queryStr = "code='" + selection + "'";
                        if(db1==null){
                            DBHelper custommOpenHelper = new DBHelper(getContext());
                            db1 = custommOpenHelper.getReadableDatabase();
                            }
                        Cursor c3 = qb.query(db1, new String[] { "cityname" },
                                queryStr, null, null, null, null);

                        c3.setNotificationUri(
                                getContext().getContentResolver(), uri);
                        if (c3.getCount()==0) {

                            c3.close();

                        } else {
                            return c3;

                        }
                    }
                    if(db1==null){
                    DBHelper custommOpenHelper = new DBHelper(getContext());
                    db1 = custommOpenHelper.getReadableDatabase();
                    }
                    Cursor c1 = qb.query(db1, new String[] { "cityname" }, queryStr1,
                            null, null, null, null);
                    c1.setNotificationUri(getContext().getContentResolver(),
                            uri);
                    if (c1.getCount()==0) {

                        c1.close();

                    } else {
                        return c1;

                    }

                }

                // search custom number end

                String lang = null;

                if (getContext().getResources().getConfiguration().locale
                        .getLanguage().equals("zh")) {
                    lang = "'and language='zhcn'";
                } else {
                    lang = "'and language='en'";
                }
                // international digi No.
                if ((selection.charAt(0) == '0')
                        && (selection.charAt(1) == '0')) {
                    String queryStr = "number='" + selection + lang;

                    qb.setTables("international");

                    if (db == null) {

                        db = SQLiteDatabase
                                .openOrCreateDatabase(
                                        "data/data/com.dean.areasearch/files/acnumber.db",
                                        null, null);
                    }
                    Cursor c = qb.query(db, new String[] { "country",
                            "language" }, queryStr, selectionArgs, null, null,
                            null);
                    c.setNotificationUri(getContext().getContentResolver(), uri);
                    return c;
                }
                // domestic digi No.
                else if ((selection.charAt(0) == '0')
                        && (selection.length() > 2)
                        && (selection.length() < 13)) {

                    if ((selection.charAt(1) == '1')
                            || (selection.charAt(1) == '2')) {
                        selection = selection.substring(0, 3);
                    }

                    else if (selection.length() > 3) {
                        selection = selection.substring(0, 4);

                    }

                    String queryStr = "tel_prex='" + selection
                            + "' and shared!='1'";

                    qb.setTables("area_info");

                    if (db == null) {

                        db = SQLiteDatabase
                                .openOrCreateDatabase(
                                        "data/data/com.dean.areasearch/files/acnumber.db",
                                        null, null);

                    }
                    Cursor c = qb.query(db, new String[] { "area_name" },
                            queryStr, selectionArgs, null, null, null);
                    c.setNotificationUri(getContext().getContentResolver(), uri);
                    return c;
                }
                // mobile number
                else if ((selection.charAt(0) == '1')
                        && (selection.length() < 12)
                        && (selection.length() > 7)) {

                    selection = selection.substring(0, 8);
                    int number = 0;
                    for (int i = 0; i < 8; i++) {
                        if (selection.charAt(i) < '0'
                                || selection.charAt(i) > '9') {
                            number = 9999999;
                            break;
                        }
                    }
                    if (number == 0) {

                        number = Integer.parseInt(selection);
                    }
                    String quali = "num_from<=" + number + " and num_to>="
                            + number;


                    qb.setTables("mobile");

                    if (db == null) {

                        db = SQLiteDatabase
                                .openOrCreateDatabase(
                                        "data/data/com.dean.areasearch/files/acnumber.db",
                                        null, null);

                    }

                    Cursor cursor = qb.query(db, new String[] { "area_name" },
                            quali, null, null, null, null);

                    cursor.setNotificationUri(
                            getContext().getContentResolver(), uri);
                    return cursor;

                }
                return null;

            }
            return null;

        } catch (SQLException e) {

            return null;
        }
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        return uri;

    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        return 0;

    }

    @Override
    public int update(Uri uri, ContentValues values, String where,
            String[] whereArgs) {
        return 0;
    }

    public static String toHexString(byte[] b) {
        char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F' };

        StringBuilder sb = new StringBuilder(b.length * 2);
        for (int i = 0; i < b.length; i++) {

            sb.append(HEX_DIGITS[(b[i] & 0xf0) >>> 4]);
            sb.append(HEX_DIGITS[b[i] & 0x0f]);

        }

        return sb.toString();

    }

    public class CopyDatabase implements Runnable {

        @Override
        public void run() {
            // TODO Auto-generated method stub
            AreaCodeTables.InitDB(ctx);
            // SharedPreferences pref = ctx.getSharedPreferences("pref",
            // ctx.MODE_PRIVATE);
            if (!pref.contains("FIRST_RUN")) {

                SharedPreferences.Editor editor = pref.edit();
                editor.putBoolean("FIRST_RUN", false);

                editor.remove("copying");
                editor.apply();
            }
        }

    }
}
