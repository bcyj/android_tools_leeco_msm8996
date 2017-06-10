/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.database;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;

import com.suntek.mway.rcs.client.aidl.plugin.entity.mcloudfile.FileNode;
import com.suntek.mway.rcs.nativeui.database.table.MCloudFileTable;

import java.util.ArrayList;

public class DB {
    private static DB instance;

    public static void init(Context context) {
        instance = new DB(context);
    }

    public static DB get() {
        return instance;
    }

    private final ContentResolver resolver;

    private DB(Context context) {
        resolver = context.getContentResolver();
    }

    public CursorLoader getMCloudFileLoader(Context context) {
        return new CursorLoader(context,
                MCloudFileTable.URI,
                MCloudFileTable.PROJECTION,
                null, null, null);
    }

    public int addMCloudFiles(ArrayList<FileNode> files) {
        deleteAllMCloudFiles();

        ContentValues[] valueses = new ContentValues[files.size()];
        for (int i = 0, size = files.size(); i < size; i++) {
            FileNode fileNode = files.get(i);
            ContentValues values = new ContentValues();
            values.put(MCloudFileTable.IS_FILE, getIntByBoolean(fileNode.isFile()));
            values.put(MCloudFileTable.IS_SHARED, getIntByBoolean(fileNode.isShared()));
            values.put(MCloudFileTable.SHARE_TYPE, fileNode.getShareType().ordinal());
            values.put(MCloudFileTable.IS_FIXED, getIntByBoolean(fileNode.isFixed()));
            values.put(MCloudFileTable.IS_NEED_UPDATE, getIntByBoolean(fileNode.isNeedUpdate()));
            values.put(MCloudFileTable.IS_NEED_UPLOAD, getIntByBoolean(fileNode.isNeedUpload()));
            values.put(MCloudFileTable.IS_SUCCESS, getIntByBoolean(fileNode.isSuccess()));
            values.put(MCloudFileTable.UPLOAD_SIZE, fileNode.getUploadSize());
            values.put(MCloudFileTable.TYPE, fileNode.getType().ordinal());
            values.put(MCloudFileTable.DIR_LEVEL, fileNode.getDirLevel());
            values.put(MCloudFileTable.ETAG, fileNode.geteTag());
            values.put(MCloudFileTable.NAME, fileNode.getName());
            values.put(MCloudFileTable.SUFFIX, fileNode.getSuffix());
            values.put(MCloudFileTable.CREATE_TIME, fileNode.getCreateTime());
            values.put(MCloudFileTable.UPDATE_TIME, fileNode.getUpdateTime());
            values.put(MCloudFileTable.SIZE, fileNode.getSize());
            values.put(MCloudFileTable.DIGEST, fileNode.getDigest());
            values.put(MCloudFileTable.REMOTE_PATH, fileNode.getRemotePath());
            values.put(MCloudFileTable.OLD_NAME, fileNode.getOldName());
            values.put(MCloudFileTable.OLD_REMOTE_PATH, fileNode.getOldRemotePath());
            values.put(MCloudFileTable.LOCAL_PATH, fileNode.getLocalPath());
            values.put(MCloudFileTable.PARENT_PATH, fileNode.getParentPath());
            values.put(MCloudFileTable.THUMBNAIL_URL, fileNode.getThumbnailURL());
            values.put(MCloudFileTable.LOCAL_THUMB_PATH, fileNode.getLocalThumbPath());
            values.put(MCloudFileTable.BIG_THUMB_URL, fileNode.getBigThumbURL());
            values.put(MCloudFileTable.LOCAL_BIG_THUMB_PATH, fileNode.getLocalBigThumbPath());
            values.put(MCloudFileTable.VERSION, fileNode.getVersion());
            values.put(MCloudFileTable.FILE_ID, fileNode.getId());
            values.put(MCloudFileTable.PARENT_ID, fileNode.getParentID());
            values.put(MCloudFileTable.FULL_PATH_ID, fileNode.getFullPathInID());
            values.put(MCloudFileTable.SHARE_PARENT_ID, fileNode.getShareParentID());
            // values.put(MCloudFileTable.FIELDS, fileNode.getFields());
            valueses[i] = values;
        }

        return resolver.bulkInsert(MCloudFileTable.URI, valueses);
    }

    private int getIntByBoolean(boolean value) {
        return value ? 1 : 0;
    }

    private boolean getBooleanByInt(int value) {
        return value == 1;
    }

    public void deleteAllMCloudFiles() {
        resolver.delete(MCloudFileTable.URI, null, null);
    }

    public FileNode getFileNodeByCursor(Cursor cursor) {
        FileNode fileNode = new FileNode();
        boolean isFile = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_FILE)));
        fileNode.setFile(isFile);
        boolean isShared = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_SHARED)));
        fileNode.setShared(isShared);
        FileNode.ShareType shareType = FileNode.ShareType.valueOf(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_SHARED)));
        fileNode.setShareType(shareType);
        boolean isFixed = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_FIXED)));
        fileNode.setFixed(isFixed);
        boolean isNeedUpdate = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_NEED_UPDATE)));
        fileNode.setNeedUpdate(isNeedUpdate);
        boolean isNeedUpload = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_NEED_UPLOAD)));
        fileNode.setNeedUpload(isNeedUpload);
        boolean isSuccess = getBooleanByInt(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.IS_SUCCESS)));
        fileNode.setSuccess(isSuccess);
        long uploadSize = cursor.getLong(cursor.getColumnIndex(MCloudFileTable.UPLOAD_SIZE));
        fileNode.setUploadSize(uploadSize);
        FileNode.FileType fileType = FileNode.FileType.valueOf(cursor.getInt(cursor
                .getColumnIndex(MCloudFileTable.TYPE)));
        fileNode.setType(fileType);
        int dirLevel = cursor.getInt(cursor.getColumnIndex(MCloudFileTable.DIR_LEVEL));
        fileNode.setDirLevel(dirLevel);
        long eTag = cursor.getLong(cursor.getColumnIndex(MCloudFileTable.ETAG));
        fileNode.seteTag(eTag);
        String name = cursor.getString(cursor.getColumnIndex(MCloudFileTable.NAME));
        fileNode.setName(name);
        String suffix = cursor.getString(cursor.getColumnIndex(MCloudFileTable.SUFFIX));
        fileNode.setSuffix(suffix);
        String createTime = cursor.getString(cursor.getColumnIndex(MCloudFileTable.CREATE_TIME));
        fileNode.setCreateTime(createTime);
        String updateTime = cursor.getString(cursor.getColumnIndex(MCloudFileTable.UPDATE_TIME));
        fileNode.setUpdateTime(updateTime);
        long size = cursor.getLong(cursor.getColumnIndex(MCloudFileTable.SIZE));
        fileNode.setSize(size);
        String digest = cursor.getString(cursor.getColumnIndex(MCloudFileTable.DIGEST));
        fileNode.setDigest(digest);
        String remotePath = cursor.getString(cursor.getColumnIndex(MCloudFileTable.REMOTE_PATH));
        fileNode.setRemotePath(remotePath);
        String oldName = cursor.getString(cursor.getColumnIndex(MCloudFileTable.OLD_NAME));
        fileNode.setOldName(oldName);
        String oldRemotePath = cursor.getString(cursor
                .getColumnIndex(MCloudFileTable.OLD_REMOTE_PATH));
        fileNode.setOldRemotePath(oldRemotePath);
        String localPath = cursor.getString(cursor.getColumnIndex(MCloudFileTable.LOCAL_PATH));
        fileNode.setLocalPath(localPath);
        String parentPath = cursor.getString(cursor.getColumnIndex(MCloudFileTable.PARENT_PATH));
        fileNode.setParentPath(parentPath);
        String thumbnailURL = cursor
                .getString(cursor.getColumnIndex(MCloudFileTable.THUMBNAIL_URL));
        fileNode.setThumbnailURL(thumbnailURL);
        String localThumbPath = cursor
                .getString(cursor.getColumnIndex(MCloudFileTable.LOCAL_THUMB_PATH));
        fileNode.setLocalThumbPath(localThumbPath);
        String bigThumbURL = cursor.getString(cursor.getColumnIndex(MCloudFileTable.BIG_THUMB_URL));
        fileNode.setBigThumbURL(bigThumbURL);
        String localBigThumbPath = cursor
                .getString(cursor.getColumnIndex(MCloudFileTable.LOCAL_BIG_THUMB_PATH));
        fileNode.setLocalBigThumbPath(localBigThumbPath);
        long version = cursor.getLong(cursor.getColumnIndex(MCloudFileTable.VERSION));
        fileNode.setVersion(version);
        String id = cursor.getString(cursor.getColumnIndex(MCloudFileTable.FILE_ID));
        fileNode.setId(id);
        String parentID = cursor.getString(cursor.getColumnIndex(MCloudFileTable.PARENT_ID));
        fileNode.setParentID(parentID);
        String fullPathID = cursor.getString(cursor.getColumnIndex(MCloudFileTable.FULL_PATH_ID));
        fileNode.setFullPathInID(fullPathID);
        String shareParentID = cursor.getString(cursor
                .getColumnIndex(MCloudFileTable.SHARE_PARENT_ID));
        fileNode.setShareParentID(shareParentID);
        return fileNode;
    }
}
