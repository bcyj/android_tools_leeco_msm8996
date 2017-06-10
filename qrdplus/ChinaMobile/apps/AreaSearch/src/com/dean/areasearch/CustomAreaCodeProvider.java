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
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

import com.dean.areasearch.AreaCodeTables.AreaCodeTable;

public class CustomAreaCodeProvider extends ContentProvider
{

    private static final String TABLE_NAME = "areacodes";

    private static final int AREA_CODES = 1;
    private static final int AREA_CODE_ID = 2;

    private static final UriMatcher sUriMatcher;

    static
    {
        sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        sUriMatcher.addURI(AreaCodeTables.AUTHORITY, "areacodes", AREA_CODES);
        sUriMatcher.addURI(AreaCodeTables.AUTHORITY, "areacodes/#", AREA_CODE_ID);
    }


    private DBHelper mOpenHelper;

    @Override
    public boolean onCreate()
    {
        mOpenHelper = new DBHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
    {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        switch (sUriMatcher.match(uri))
        {
            case AREA_CODES:
                qb.setTables(TABLE_NAME);
                break;

            case AREA_CODE_ID:
                qb.setTables(TABLE_NAME);
                qb.appendWhere(AreaCodeTable._ID + "=" + uri.getPathSegments().get(1));
                break;

            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        // If no sort order is specified use the default
        String orderBy;
        if (TextUtils.isEmpty(sortOrder))
        {
            orderBy = AreaCodeTables.AreaCodeTable.DEFAULT_SORT_ORDER;
        }
        else
        {
            orderBy = sortOrder;
        }

        // Get the database and run the query

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);

        // Tell the cursor which uri to watch, so it knows when its source data changes
        c.setNotificationUri(getContext().getContentResolver(), uri);

        return c;
    }

    @Override
    public String getType(Uri uri)
    {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues)
    {
        if(sUriMatcher.match(uri) == AREA_CODES)
        {
            ContentValues values;
            if(initialValues != null)
            {
                values = new ContentValues(initialValues);
            }
            else
            {
                values = new ContentValues();
            }

            Long now = Long.valueOf(System.currentTimeMillis());

            // Make sure that the fields are all set
            if(values.containsKey(AreaCodeTables.AreaCodeTable.MODIFIED_DATE) == false)
            {
                values.put(AreaCodeTables.AreaCodeTable.MODIFIED_DATE, now);
            }

            if(values.containsKey(AreaCodeTables.AreaCodeTable.CODE) == false)
            {
                values.put(AreaCodeTables.AreaCodeTable.CODE, "area_code");
            }

            if(values.containsKey(AreaCodeTables.AreaCodeTable.CITY_NAME) == false)
            {
                values.put(AreaCodeTables.AreaCodeTable.CODE, "city_name");
            }

            SQLiteDatabase db = mOpenHelper.getWritableDatabase();

            long rowId = db.insert(TABLE_NAME, AreaCodeTable.CODE, values);
            if(rowId > 0)
            {
                Uri noteUri = ContentUris.withAppendedId(AreaCodeTables.AreaCodeTable.CONTENT_URI, rowId);
                getContext().getContentResolver().notifyChange(noteUri, null);
                return noteUri;
            }
        }
        else
        {
            throw new IllegalArgumentException("Unknown URI " + uri);
        }

        throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs)
    {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count;
        switch (sUriMatcher.match(uri))
        {
            case AREA_CODES:
                count = db.delete(TABLE_NAME, where, whereArgs);
                break;

            case AREA_CODE_ID:
                String noteId = uri.getPathSegments().get(1);
                count = db.delete(TABLE_NAME, AreaCodeTable._ID + "=" + noteId
                        + (!TextUtils.isEmpty(where) ? " AND (" + where + ')' : ""), whereArgs);
                break;

            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }


    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs)
    {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count;
        switch (sUriMatcher.match(uri))
        {
            case AREA_CODES:
                count = db.update(TABLE_NAME, values, where, whereArgs);
                break;

            case AREA_CODE_ID:
                String noteId = uri.getPathSegments().get(1);
                count = db.update(TABLE_NAME, values, AreaCodeTable._ID + "=" + noteId
                        + (!TextUtils.isEmpty(where) ? " AND (" + where + ')' : ""), whereArgs);
                break;

            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }
}
