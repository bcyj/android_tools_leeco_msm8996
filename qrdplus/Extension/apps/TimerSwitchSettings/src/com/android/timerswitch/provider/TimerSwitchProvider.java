/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.timerswitch.provider;

import com.android.timerswitch.utils.Log;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

public class TimerSwitchProvider extends ContentProvider {
    private TimerSwitchDatabaseHelper mOpenHelper;

    private static final int SWITCHES = 1;
    private static final int SWITCHES_ID = 2;

    private static final UriMatcher sURLMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        sURLMatcher.addURI(TimerSwitchContract.AUTHORITY, "switches", SWITCHES);
        sURLMatcher.addURI(TimerSwitchContract.AUTHORITY, "switches/#", SWITCHES_ID);
    }

    public TimerSwitchProvider() {
        super();
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new TimerSwitchDatabaseHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs,
            String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        int match = sURLMatcher.match(uri);
        switch (match) {
            case SWITCHES:
                qb.setTables(TimerSwitchDatabaseHelper.SWITCHES_TABLE_NAME);
                break;
            case SWITCHES_ID:
                qb.setTables(TimerSwitchDatabaseHelper.SWITCHES_TABLE_NAME);
                qb.appendWhere(TimerSwitchContract.SwitchesColumns._ID + "=");
                qb.appendWhere(uri.getLastPathSegment());
                break;
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor ret = qb.query(db, projectionIn, selection, selectionArgs,
                null, null, sort);

        if (ret == null) {
            Log.e("Query: failed");
        } else {
            ret.setNotificationUri(getContext().getContentResolver(), uri);
        }

        return ret;
    }

    @Override
    public String getType(Uri uri) {
        int match = sURLMatcher.match(uri);
        switch (match) {
            case SWITCHES:
                return "vnd.android.cursor.dir/switches";
            case SWITCHES_ID:
                return "vnd.android.cursor.item/switches";
            default:
                throw new IllegalArgumentException("Unknown URL");
        }
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count;
        String switchId;
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        switch (sURLMatcher.match(uri)) {
            case SWITCHES_ID:
                switchId = uri.getLastPathSegment();
                count = db.update(TimerSwitchDatabaseHelper.SWITCHES_TABLE_NAME, values,
                        TimerSwitchContract.SwitchesColumns._ID + "=" + switchId,
                        null);
                break;
            default: {
                throw new UnsupportedOperationException(
                        "Cannot update URL: " + uri);
            }
        }
        if (Log.LOGV)
            Log.v("Notify change id: " + switchId + " url " + uri);
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        long rowId;
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        switch (sURLMatcher.match(uri)) {
            case SWITCHES:
                rowId = db.insert(TimerSwitchDatabaseHelper.SWITCHES_TABLE_NAME, null,
                        initialValues);
                break;
            default:
                throw new IllegalArgumentException("Cannot insert from URL: " + uri);
        }

        Uri uriResult = ContentUris.withAppendedId(TimerSwitchContract.SwitchesColumns.CONTENT_URI,
                rowId);
        getContext().getContentResolver().notifyChange(uriResult, null);
        return uriResult;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

}
