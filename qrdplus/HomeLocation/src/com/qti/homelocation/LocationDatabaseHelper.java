/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import com.qti.homelocation.GeocodedLocation.Area;
import com.qti.homelocation.GeocodedLocation.AreaCode;
import com.qti.homelocation.GeocodedLocation.Section;

public class LocationDatabaseHelper extends SQLiteOpenHelper {

    private static final String TAG = "Location";

    private static final String DATABASE_NAME = "location.db";
    private static final int DATABASE_VERSION = 1;

    private Context mContext;

    public LocationDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        mContext = context;
        initDBFromAssetsIfNeed();
    }

    public static interface SectionColumns {
        String _ID = Section.PATH + "." + Section._ID;
        String NUMBER_START = Section.PATH + "." + Section.NUMBER_START;
        String NUMBER_END = Section.PATH + "." + Section.NUMBER_END;
        String CODE_ID = Section.PATH + "." + Section.CODE_ID;
    }

    public static interface AreaCodeColumns {
        String _ID = AreaCode.PATH + "." + AreaCode._ID;
        String CODE_ID = AreaCode.PATH + "." + AreaCode.CODE_ID;
        String CODE = AreaCode.PATH + "." + AreaCode.CODE;
        String CITY_NAME = AreaCode.PATH + "." + AreaCode.CITY_NAME;
        String AREA_ID = AreaCode.PATH + "." + AreaCode.AREA_ID;
    }

    public static interface AreaColumns {
        String AREA_ID = Area.PATH + "." + Area.AREA_ID;
        String AREA_NAME = Area.PATH + "." + Area.AREA_NAME;
    }

    public interface Views {
        public static final String SECTION = "view_section";
        public static final String AREA = "view_area";
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        createTables(db);
        createViews(db);
    }

    private void createTables(SQLiteDatabase db) {

        db.execSQL("CREATE TABLE " + Section.PATH + " (" + Section._ID
                + " INTEGER PRIMARY KEY AUTOINCREMENT," + Section.NUMBER_START + " INTEGER,"
                + Section.NUMBER_END + " INTEGER," + Section.CODE_ID + " INTEGER);");

        db.execSQL("CREATE TABLE " + AreaCode.PATH + " (" + AreaCode._ID
                + " INTEGER PRIMARY KEY AUTOINCREMENT," + AreaCode.CODE_ID + " INTEGER,"
                + AreaCode.CODE + " TEXT," + AreaCode.CITY_NAME + " TEXT," + AreaCode.AREA_ID
                + " INTEGER);");

        db.execSQL("CREATE TABLE " + Area.PATH + " (" + Area._ID
                + " INTEGER PRIMARY KEY AUTOINCREMENT," + Area.AREA_ID + " INTEGER,"
                + Area.AREA_NAME + " TEXT);");
    }

    private void createViews(SQLiteDatabase db) {
        db.execSQL("DROP VIEW IF EXISTS " + Views.SECTION + ";");
        db.execSQL("DROP VIEW IF EXISTS " + Views.AREA + ";");

        String areaColumns = AreaCodeColumns.CODE_ID + " as " + AreaCode.CODE_ID + ","
                + AreaCodeColumns.CODE + " as " + AreaCode.CODE + ","
                + AreaCodeColumns.CITY_NAME + " as " + AreaCode.CITY_NAME + ","
                + AreaColumns.AREA_ID + " as " + Area.AREA_ID + ","
                + AreaColumns.AREA_NAME + " as " + Area.AREA_NAME;

        String areaSelect = "SELECT " + AreaCodeColumns._ID + " as " + AreaCode._ID + ","
                + areaColumns + " FROM " + AreaCode.PATH + " JOIN " + Area.PATH + " ON("
                + AreaCodeColumns.AREA_ID + "=" + AreaColumns.AREA_ID + ")";

        db.execSQL("CREATE VIEW " + Views.AREA + " AS "
                + areaSelect);

        String sectionColumns = SectionColumns.NUMBER_START + " as " + Section.NUMBER_START + ","
                + SectionColumns.NUMBER_END + " as " + Section.NUMBER_END + "," + areaColumns;

        String sectionSelect = "SELECT " + SectionColumns._ID + " as " + Section._ID + ","
                + sectionColumns + " FROM " + Section.PATH + " JOIN " + AreaCode.PATH + " ON("
                + SectionColumns.CODE_ID + "=" + AreaCodeColumns.CODE_ID + ") JOIN " + Area.PATH
                + " ON(" + AreaCodeColumns.AREA_ID + "=" + AreaColumns.AREA_ID + ")";

        db.execSQL("CREATE VIEW " + Views.SECTION + " AS "
                + sectionSelect);

    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }

    private boolean initDBFromAssetsIfNeed() {
        String targetPath = "/data/data/" + mContext.getPackageName() + "/databases/"
                + LocationDatabaseHelper.DATABASE_NAME;
        File target = new File(targetPath);
        if (target.exists()) {
            return true;
        }
        if (!target.getParentFile().exists()) {
            target.getParentFile().mkdirs();
        }
        InputStream is = null;
        FileOutputStream fos = null;
        try {
            Log.d(TAG, "initialize database " + targetPath);
            is = mContext.getAssets().open(LocationDatabaseHelper.DATABASE_NAME);
            fos = new FileOutputStream(target);
            byte[] buffer = new byte[1024];
            int count = 0;
            while ((count = is.read(buffer)) > 0) {
                fos.write(buffer, 0, count);
            }
            return true;
        } catch (Exception e) {
            Log.w(TAG, "copy " + targetPath + " failed!", e);
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                }
            }
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                }
            }
        }
        return false;
    }
}
