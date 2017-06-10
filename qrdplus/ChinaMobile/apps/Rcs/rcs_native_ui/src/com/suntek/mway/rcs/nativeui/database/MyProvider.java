/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.database;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;

import com.suntek.mway.rcs.nativeui.database.table.MCloudFileTable;
import com.suntek.mway.rcs.nativeui.database.table.Table;

import java.util.ArrayList;

public class MyProvider extends ContentProvider {
    private static final ArrayList<Table> tableList = new ArrayList<Table>();

    private static final UriMatcher URI_MATCHER;
    private SQLiteDatabase db;

    static {
        // 在这里加入新加的数据表
        tableList.add(new MCloudFileTable());

        URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
        for (int i = 0, size = tableList.size(); i < size; i++) {
            URI_MATCHER.addURI(Table.AUTHORITY, tableList.get(i).getTableName(), i);
        }
    }

    private String getTableByUri(Uri uri) {
        int match = URI_MATCHER.match(uri);
        if (match >= 0 && match < tableList.size()) {
            return tableList.get(match).getTableName();
        } else {
            throw new IllegalArgumentException("unknow URI" + uri);
        }
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        int count = db.delete(getTableByUri(uri), selection, selectionArgs);
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        long rowId = db.insert(getTableByUri(uri), null, values);
        if (rowId > 0) {
            Uri result = ContentUris.withAppendedId(uri, rowId);
            getContext().getContentResolver().notifyChange(result, null);
            return result;
        } else
            throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public int bulkInsert(Uri uri, ContentValues[] values) {
        int count = 0;
        try {
            db.beginTransaction();
            for (ContentValues value : values) {
                Uri resultUri = insert(uri, value);
                if (resultUri != null) {
                    count++;
                } else {
                    count = 0;
                    throw new SQLException("error in bulk insert");
                }
            }
            db.setTransactionSuccessful();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            db.endTransaction();
        }
        return count;
    }

    @Override
    public boolean onCreate() {
        DB dbOpenHelper = new DB(this.getContext());
        db = dbOpenHelper.getWritableDatabase();
        return false;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        Cursor c = db.query(getTableByUri(uri), projection, selection, selectionArgs, null, null,
                sortOrder);
        c.setNotificationUri(getContext().getContentResolver(), uri);
        return c;
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        int count = db.update(getTableByUri(uri), values, selection, selectionArgs);
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    static class DB extends SQLiteOpenHelper {
        private static final String DATABASE_NAME = "NativeUI";
        private static final int DATABASE_VERSION = 1;

        public DB(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            // 创建表
            String sql;
            for (Table table : tableList) {
                sql = table.getCreateTableSql();
                try {
                    db.execSQL(sql);// 需要异常捕获
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            onCreate(db);
        }
    }
}
