/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.database.table;

import android.net.Uri;

public class MCloudFileTable extends Table {

    public static final String TABLE_MCLOUD_FILE = "mcloud_file";
    public static final Uri URI = Uri.parse("content://" + AUTHORITY + "/" + TABLE_MCLOUD_FILE);



    public static final String IS_FILE = "is_file";
    public static final String IS_SHARED = "is_shared";
    public static final String IS_FIXED = "is_fixed";
    public static final String IS_NEED_UPDATE = "is_need_update";
    public static final String IS_NEED_UPLOAD = "is_need_upload";
    public static final String IS_SUCCESS = "is_success";
    public static final String UPLOAD_SIZE = "upload_size";
    public static final String TYPE = "type";
    public static final String DIR_LEVEL = "dir_level";
    public static final String ETAG = "etag";
    public static final String NAME = "name";
    public static final String SUFFIX = "suffix";
    public static final String SHARE_TYPE = "share_type";
    public static final String CREATE_TIME = "create_time";
    public static final String UPDATE_TIME = "update_time";
    public static final String SIZE = "size";
    public static final String DIGEST = "digest";
    public static final String REMOTE_PATH = "remote_path";
    public static final String OLD_NAME = "old_name";
    public static final String OLD_REMOTE_PATH = "old_remote_path";
    public static final String LOCAL_PATH = "local_path";
    public static final String PARENT_PATH = "parent_path";
    public static final String THUMBNAIL_URL = "thumbnail_url";
    public static final String LOCAL_THUMB_PATH = "local_thumb_path";
    public static final String BIG_THUMB_URL = "big_thumb_url";
    public static final String LOCAL_BIG_THUMB_PATH = "local_big_thumb_path";
    public static final String VERSION = "version";
    public static final String FILE_ID = "file_id";
    public static final String PARENT_ID = "parent_id";
    public static final String FULL_PATH_ID = "full_path_id";
    public static final String SHARE_PARENT_ID = "share_parent_id";
    public static final String FIELDS = "fields";

    @Override
    public String getTableName() {
        return TABLE_MCLOUD_FILE;
    }

    public static final String[] PROJECTION = new String[] {
            ID, IS_FILE, IS_SHARED, SHARE_TYPE, IS_FIXED,
            IS_NEED_UPDATE, IS_NEED_UPLOAD, IS_SUCCESS, UPLOAD_SIZE,
            TYPE, DIR_LEVEL, ETAG, NAME, SUFFIX, CREATE_TIME, UPDATE_TIME, SIZE, DIGEST,
            REMOTE_PATH, OLD_NAME, OLD_REMOTE_PATH, LOCAL_PATH, PARENT_PATH,
            THUMBNAIL_URL, LOCAL_THUMB_PATH, BIG_THUMB_URL, LOCAL_BIG_THUMB_PATH,
            VERSION, FILE_ID, PARENT_ID, FULL_PATH_ID, SHARE_PARENT_ID, FIELDS

    };

    @Override
    public String getCreateTableSql() {
        return "CREATE TABLE IF NOT EXISTS " + TABLE_MCLOUD_FILE + " (" +
                ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                IS_FILE + " INTEGER NOT NULL," +
                IS_SHARED + " INTEGER NOT NULL," +
                SHARE_TYPE + " INTEGER NOT NULL," +
                IS_FIXED + " INTEGER NOT NULL," +
                IS_NEED_UPDATE + " INTEGER NOT NULL," +
                IS_NEED_UPLOAD + " INTEGER NOT NULL," +
                IS_SUCCESS + " INTEGER NOT NULL," +
                UPLOAD_SIZE + " INTEGER NOT NULL," +
                TYPE + " INTEGER NOT NULL," +
                DIR_LEVEL + " INTEGER NOT NULL," +
                ETAG + " INTEGER NOT NULL," +
                NAME + " TEXT," +
                SUFFIX + " TEXT," +
                CREATE_TIME + " TEXT," +
                UPDATE_TIME + " TEXT," +
                SIZE + " INTEGER NOT NULL," +
                DIGEST + " TEXT," +
                REMOTE_PATH + " TEXT," +
                OLD_NAME + " TEXT," +
                OLD_REMOTE_PATH + " TEXT," +
                LOCAL_PATH + " TEXT," +
                PARENT_PATH + " TEXT," +
                THUMBNAIL_URL + " TEXT," +
                LOCAL_THUMB_PATH + " TEXT," +
                BIG_THUMB_URL + " TEXT," +
                LOCAL_BIG_THUMB_PATH + " TEXT," +
                VERSION + " INTEGER NOT NULL," +
                FILE_ID + " TEXT NOT NULL," +
                PARENT_ID + " TEXT," +
                FULL_PATH_ID + " TEXT," +
                SHARE_PARENT_ID + " TEXT," +
                FIELDS + " Integer)";
    }

}
