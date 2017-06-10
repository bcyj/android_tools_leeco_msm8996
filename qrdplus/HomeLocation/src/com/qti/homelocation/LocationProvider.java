/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import java.util.HashMap;
import java.util.Map;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Bundle;

import com.qti.homelocation.GeocodedLocation.Area;
import com.qti.homelocation.GeocodedLocation.AreaCode;
import com.qti.homelocation.GeocodedLocation.Section;
import com.qti.homelocation.LocationDatabaseHelper.Views;

public class LocationProvider extends ContentProvider {

    private static final Map<String, String> SECTION_COLUMNS = new HashMap<String, String>();
    private static final Map<String, String> AREA_CODE_COLUMNS = new HashMap<String, String>();
    private static final Map<String, String> AREA_COLUMNS = new HashMap<String, String>();
    private static final Map<String, String> AREA_CODE_PROJECT_COLUMNS =
            new HashMap<String, String>();
    private static final Map<String, String> LOCATION_PROJECT_COLUMNS =
            new HashMap<String, String>();

    static {
        SECTION_COLUMNS.put(Section._ID, Section._ID);
        SECTION_COLUMNS.put(Section.NUMBER_START, Section.NUMBER_START);
        SECTION_COLUMNS.put(Section.NUMBER_END, Section.NUMBER_END);

        AREA_CODE_COLUMNS.put(AreaCode._ID, AreaCode._ID);
        AREA_CODE_COLUMNS.put(AreaCode.CODE_ID, AreaCode.CODE_ID);
        AREA_CODE_COLUMNS.put(AreaCode.CITY_NAME, AreaCode.CITY_NAME);
        AREA_CODE_COLUMNS.put(AreaCode.CODE, AreaCode.CODE);

        AREA_COLUMNS.put(Area._ID, Area._ID);
        AREA_COLUMNS.put(Area.AREA_ID, Area.AREA_ID);
        AREA_COLUMNS.put(Area.AREA_NAME, Area.AREA_NAME);

        AREA_CODE_PROJECT_COLUMNS.putAll(AREA_CODE_COLUMNS);
        AREA_CODE_PROJECT_COLUMNS.putAll(AREA_COLUMNS);

        LOCATION_PROJECT_COLUMNS.putAll(SECTION_COLUMNS);
        LOCATION_PROJECT_COLUMNS.putAll(AREA_CODE_PROJECT_COLUMNS);
    }

    private static final int LOCATION = 10;
    private static final int SECTION = 11;
    private static final int AREA_CODE = 12;
    private static final int AREA = 13;

    private static final UriMatcher sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        sUriMatcher.addURI(GeocodedLocation.AUTHORITY, GeocodedLocation.PATH, LOCATION);
        sUriMatcher.addURI(GeocodedLocation.AUTHORITY, Section.PATH, SECTION);
        sUriMatcher.addURI(GeocodedLocation.AUTHORITY, AreaCode.PATH, AREA_CODE);
        sUriMatcher.addURI(GeocodedLocation.AUTHORITY, Area.PATH, AREA);
    }

    private LocationDatabaseHelper databaseOpenHelper;

    @Override
    public boolean onCreate() {
        databaseOpenHelper = new LocationDatabaseHelper(getContext());
        return false;
    }

    @Override
    public Bundle call(String method, String arg, Bundle extras) {
        Bundle result = new Bundle();
        if ("getLocation".equals(method)) {
            GeocodedLocation location = GeocodedLocation.getLocation(getContext(), arg);
            if (location != null) {
                result.putString("location", location.getAreaCode().getAddress());
            }
        }
        return result;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        final SQLiteDatabase db = databaseOpenHelper.getReadableDatabase();
        final int match = sUriMatcher.match(uri);
        switch (match) {
            case LOCATION:
                qb.setTables(Views.SECTION);
                qb.setProjectionMap(LOCATION_PROJECT_COLUMNS);
                break;
            case SECTION:
                qb.setTables(Section.PATH);
                qb.setProjectionMap(SECTION_COLUMNS);
                break;
            case AREA_CODE:
                qb.setTables(Views.AREA);
                qb.setProjectionMap(AREA_CODE_PROJECT_COLUMNS);
                break;
            case AREA:
                qb.setTables(Area.PATH);
                qb.setProjectionMap(AREA_COLUMNS);
                break;

        }
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, sortOrder);
        return c;
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        final int match = sUriMatcher.match(uri);
        final SQLiteDatabase db = databaseOpenHelper.getWritableDatabase();
        long rowId = -1;
        switch (match) {
            case SECTION:
                rowId = db.insert(Section.PATH, null, values);
                break;
            case AREA_CODE:
                rowId = db.insert(AreaCode.PATH, null, values);
                break;
            case AREA:
                rowId = db.insert(Area.PATH, null, values);
                break;

        }
        Uri result = null;
        if (rowId != -1) {
            result = ContentUris.withAppendedId(uri, rowId);
            getContext().getContentResolver().notifyChange(uri, null);
        }
        return result;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        return 0;
    }
}
