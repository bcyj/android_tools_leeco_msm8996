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


import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

public class NativeProvider extends ContentProvider {

    private static final String DATABASE_NAME = "acnumber.db";
    private static final String CUSTOM_DATABASE_NAME = "custom_area_codes.db";
    private static final int DATABASE_VERSION = 5;
    SQLiteDatabase db = null;

    private static final UriMatcher sUriMatcher;

    static {
        sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        sUriMatcher.addURI("nativeareasearch", "area_info", 1);
        sUriMatcher.addURI("nativeareasearch", "mobile", 2);
        sUriMatcher.addURI("nativeareasearch", "international", 3);
    }

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
    private static class CustomDatabaseHelper extends SQLiteOpenHelper {

        CustomDatabaseHelper(Context context) {
            super(context, CUSTOM_DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {

        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

        }
    }

    /* end of class DatabaseHelper */
    private DatabaseHelper mOpenHelper;
    private CustomDatabaseHelper custommOpenHelper;

    @Override
    public boolean onCreate() {
        AreaCodeTables.InitDB(getContext());
        mOpenHelper = new DatabaseHelper(getContext());
        custommOpenHelper = new CustomDatabaseHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

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

                db = SQLiteDatabase.openOrCreateDatabase(
                        "data/data/com.dean.areasearch/files/acnumber.db",
                        null, null);

            }
            Cursor c = qb.query(db, projection, selection, selectionArgs, null,
                    null, null);

            c.setNotificationUri(getContext().getContentResolver(), uri);
            // db.close();
            return c;
        }

        else if (selection.charAt(0) == 'p') {

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
            // SQLiteDatabase db = mOpenHelper.getWritableDatabase();
            if (db == null) {
                db = SQLiteDatabase.openOrCreateDatabase(
                        "data/data/com.dean.areasearch/files/acnumber.db",
                        null, null);
            }
            Cursor c = qb.query(db, projection, selection, selectionArgs, null,
                    null, null);

            c.setNotificationUri(getContext().getContentResolver(), uri);

            return c;

        }

        else if ((selection != null)&&((selection.length() >= 2))) {
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

            String queryStr1 = "code='" + selection + "'";

            qb.setTables("areacodes");
            SQLiteDatabase db1 = custommOpenHelper.getWritableDatabase();

            Cursor c1 = qb.query(db1, new String[] { "cityname" }, queryStr1,
                    null, null, null, null);
            c1.setNotificationUri(getContext().getContentResolver(), uri);


            if (c1.moveToFirst()) {

                return c1;

            }

            //cut domestic No from 0532******* to 0532
            else if ((selection.charAt(0) == '0') && (selection.charAt(1) != '0')&&(selection.length() > 2)
                    && (selection.length() < 13)) {

                if ((selection.charAt(1) == '1')
                        || (selection.charAt(1) == '2')) {
                    selection = selection.substring(0, 3);

                }

                else if (selection.length() > 3) {

                    selection = selection.substring(0, 4);

                }

                String queryStr = "code='" + selection + "'";
                Cursor c2 = qb.query(db1, new String[] { "cityname" },
                        queryStr, selectionArgs, null, null, null);
                c2.setNotificationUri(getContext().getContentResolver(), uri);
                if (c2.moveToFirst()) {
                    return c2;
                }
            }


            // cut mobile No. from 1380532**** to 1380532
            else if ((selection.charAt(0) == '1')
                    && (selection.length() < 12) && (selection.length() > 7)) {
                 if (selection.equals("13800138000"))
                 {
                     return null;
                 }
                selection = selection.substring(0, 8);
                String queryStr = "code='" + selection + "'";

                Cursor c3 = qb.query(db1, new String[] { "cityname" },
                        queryStr, null, null, null, null);

                c3.setNotificationUri(getContext().getContentResolver(), uri);
                if (c3.moveToFirst()) {
                    return c3;
                }
            }

            String lang = null;

            if(getContext().getResources().getConfiguration().locale.getLanguage().equals("zh")){
                lang = "'and language='zhcn'";
            }
            else{
                lang = "'and language='en'";
            }
            // international digi No.

            if ((selection.charAt(0) == '0') && (selection.charAt(1) == '0')) {

                String queryStr = "number='" + selection + lang;

                qb.setTables("international");
                // SQLiteDatabase db = mOpenHelper.getWritableDatabase();
                if (db == null) {
                    db = SQLiteDatabase
                            .openOrCreateDatabase(
                                    "data/data/com.dean.areasearch/files/acnumber.db",
                                    null, null);
                }
                Cursor c = qb.query(db, new String[] { "country", "language" },
                        queryStr, selectionArgs, null, null, null);

                c.setNotificationUri(getContext().getContentResolver(), uri);
                return c;
            }
            // domestic digi No.
            else if ((selection.charAt(0) == '0') && (selection.length() > 2)
                    && (selection.length() < 13)) {

                if ((selection.charAt(1) == '1')
                        || (selection.charAt(1) == '2')) {
                    selection = selection.substring(0, 3);
                }

                else if (selection.length() > 3) {
                    selection = selection.substring(0, 4);

                }

                String queryStr = "tel_prex='" + selection
                        + "' and shared!='2'";

                qb.setTables("area_info");
                if (db == null) {
                    db = SQLiteDatabase
                            .openOrCreateDatabase(
                                    "data/data/com.dean.areasearch/files/acnumber.db",
                                    null, null);
                }
                Cursor c = qb.query(db, new String[] { "area_name" }, queryStr,
                        selectionArgs, null, null, null);
                c.setNotificationUri(getContext().getContentResolver(), uri);
                return c;
            }
            // mobile digi number
            else if ((selection.charAt(0) == '1') && (selection.length() < 12)
                    && (selection.length() > 7)) {
                selection = selection.substring(0, 8);
                int number =0;
                for(int i=0; i<8; i++){
                    if (selection.charAt(i)<'0'||selection.charAt(i)>'9'){
                        number= 9999999;
                        break;
                    }
                }
               if (number ==0){

                   number = Integer.parseInt(selection);
               }
                String quali = "num_from<=" + number + " and num_to>=" + number;
                qb.setTables("mobile");
                if (db == null) {
                    db = SQLiteDatabase
                            .openOrCreateDatabase(
                                    "data/data/com.dean.areasearch/files/acnumber.db",
                                    null, null);
                }
                Cursor cursor = qb.query(db, new String[] { "area_name" },
                        quali, null, null, null, null);

                cursor.setNotificationUri(getContext().getContentResolver(),
                        uri);
                return cursor;

            }
            return c1;

        }
        return null;

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

}
