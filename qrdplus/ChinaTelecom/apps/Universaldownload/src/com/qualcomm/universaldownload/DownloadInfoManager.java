/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/
package com.qualcomm.universaldownload;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class DownloadInfoManager {
    private static final String TAG = "DownloadInfoManager";
    private static DownloadInfoManager instance;
    private DownloadDBHelper downloadHelper;

    private DownloadInfoManager(Context context) {
        downloadHelper = new DownloadDBHelper(context);
    }

    public List<DownloadInfo> getDownloadInfos(int state) {
        SQLiteDatabase database = downloadHelper.getReadableDatabase();
        String sql = "SELECT capability, url, md5, state, downloadsize, version, file FROM" +
                " download_info WHERE state=?";
        Cursor cursor = database.rawQuery(sql, new String[] {String.valueOf(state)});
        ArrayList<DownloadInfo> infos = new ArrayList<DownloadInfo>();
        Log.d(TAG, "getDownloadInfos state is " + state +
                "cursor.getCount() is " + cursor.getCount());
        if(cursor.getCount() > 0) {
            while(cursor.moveToNext()) {
                DownloadInfo info = new DownloadInfo(cursor.getString(0),cursor.getString(1),
                        cursor.getInt(4));
                info.setDownloadState(cursor.getInt(3));
                info.setMd5(cursor.getString(2));
                info.setVersion(cursor.getString(5));
                info.setDownloadFile(cursor.getString(6));
                infos.add(info);
            }
        }
        return infos;
    }

    public void saveDownloadInfo(DownloadInfo info) {
        SQLiteDatabase db = downloadHelper.getWritableDatabase();
        String sql = "INSERT INTO download_info(capability, url, md5, state, file, downloadsize," +
                " version) values (?, ?, ?, ?, ?, ?, ?)";
        Object[] bindArgs = {info.getCapability(), info.getAddr(), info.getMd5(),
                info.getDownloadState(), info.getDownloadFile(), info.getDownloadSize(),
                info.getVersion()};
        db.execSQL(sql, bindArgs);
    }

    public void updateDownloadInfo(DownloadInfo info, int state, int size) {
        info.setDownloadState(state);
        info.setDownloadSize(size);
        SQLiteDatabase db = downloadHelper.getWritableDatabase();
        String sql = "UPDATE download_info set state=?, downloadsize=? WHERE capability=?";
        Object[] bindArgs = {state, size, info.getCapability()};
        db.execSQL(sql,bindArgs);
    }

    public void updateDownloadInfo(DownloadInfo downloadInfo, String filePath) {
        downloadInfo.setDownloadFile(filePath);
        SQLiteDatabase db = downloadHelper.getWritableDatabase();
        String sql = "UPDATE download_info set file=? WHERE capability=?";
        Object[] bindArgs = {filePath, downloadInfo.getCapability()};
        db.execSQL(sql,bindArgs);
    }

    public static DownloadInfoManager instance(Context context) {
        if(instance == null) {
            instance = new DownloadInfoManager(context);
        }
        return instance;
    }

    public DownloadInfo getDownloadInfo(String capability) {
        SQLiteDatabase database = downloadHelper.getReadableDatabase();
        String sql = "SELECT capability, url, md5, state, downloadsize, version, file FROM" +
                " download_info WHERE capability=?";
        Cursor cursor = database.rawQuery(sql, new String[] {capability});
        ArrayList<DownloadInfo> infos = new ArrayList<DownloadInfo>();
        if(cursor.getCount() > 0) {
            while(cursor.moveToNext()) {
                DownloadInfo info = new DownloadInfo(cursor.getString(0),cursor.getString(1),
                        cursor.getInt(4));
                info.setDownloadState(cursor.getInt(3));
                info.setMd5(cursor.getString(2));
                info.setVersion(cursor.getString(5));
                info.setDownloadFile(cursor.getString(6));
                infos.add(info);
            }
        }
        if(infos.size() > 0) {
            return infos.get(0);
        }
        return null;
    }

    public void deleteDownloadInfo(String capability) {
        SQLiteDatabase database = downloadHelper.getWritableDatabase();
        database.delete("download_info", "capability=?", new String[] {capability});
        database.close();
    }

    public void closeDb() {
        downloadHelper.close();
    }
}
