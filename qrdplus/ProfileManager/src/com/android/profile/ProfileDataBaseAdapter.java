/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.LOG;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class ProfileDataBaseAdapter {

    private static final String TAG = "ProfileDataBaseAdapter";
    private static final String DB_NAME = "user_profiles";
    private static final String TABLE_NAME = "profiles";
    public static final String ID = "_id";

    public static final String PROFILE_NAME = "profile_name";
    public static final String SILENT = "silent";
    public static final String VIBRATE = "vibrate";
    public static final String RINGTONE1 = "slot1_ringtone";
    public static final String RINGTONE2 = "slot2_ringtone";
    public static final String RING_VOLUME = "ring_volume";

    public static final String DATA = "data";
    public static final String WIFI = "wifi";
    public static final String BLUETOOTH = "bluetooth";
    public static final String GPS_LOCATION = "gps_location";
    public static final String NETWORK_LOCATION = "network_location";
    public static final String BRIGHTNESS = "brightness";

    private static final int DB_VERSION = 1;
    private Context mContext = null;

    private static final String DB_CREATE = "CREATE TABLE " + TABLE_NAME + " (" + ID
            + " integer primary key," + PROFILE_NAME + " varchar," + SILENT + " boolean," + VIBRATE
            + " boolean," + RINGTONE1 + " varchar," + RINGTONE2 + " varchar," + RING_VOLUME
            + " integer," + DATA + " boolean," + WIFI + " boolean," + BLUETOOTH + " boolean,"
            + GPS_LOCATION + " boolean," + NETWORK_LOCATION + " boolean," + BRIGHTNESS
            + " integer)";

    private SQLiteDatabase mSQLiteDatabase = null;
    private DataBaseManagementHelper mDatabaseHelper = null;

    private static class DataBaseManagementHelper extends SQLiteOpenHelper {

        DataBaseManagementHelper(Context context) {

            super(context, DB_NAME, null, DB_VERSION);

        }

        @Override
        public void onCreate(SQLiteDatabase db) {

            db.execSQL(DB_CREATE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

            db.execSQL("DROP TABLE IF EXISTS notes");
            onCreate(db);
        }
    }

    public ProfileDataBaseAdapter(Context context) {

        mContext = context;
    }

    public void openDataBase() throws SQLException {

        logd("");
        mDatabaseHelper = new DataBaseManagementHelper(mContext);
        mSQLiteDatabase = mDatabaseHelper.getWritableDatabase();
    }

    public void closeDataBase() throws SQLException {

        logd("");
        mDatabaseHelper.close();
    }

    public long insertProfile(Profile mProfile) {

        String profile_name = mProfile.getProfileName();
        boolean silent = mProfile.isSilent();
        boolean vibrate = mProfile.isVibrate();
        String ringtone1 = mProfile.getRingtone1();
        String ringtone2 = mProfile.getRingtone2();
        int ringVolume = mProfile.getRingVolume();
        boolean data = mProfile.isData();
        boolean wifi = mProfile.isWifi();
        boolean bluetooth = mProfile.isBluetooth();
        boolean gpsLocation = mProfile.isGpsLocation();
        boolean networkLocation = mProfile.isNetworkLocation();
        int brightness = mProfile.getBrightness();

        ContentValues values = new ContentValues();
        values.put(PROFILE_NAME, profile_name);
        values.put(SILENT, silent);
        values.put(VIBRATE, vibrate);
        values.put(RINGTONE1, ringtone1);
        values.put(RINGTONE2, ringtone2);
        values.put(RING_VOLUME, ringVolume);
        values.put(DATA, data);
        values.put(WIFI, wifi);
        values.put(BLUETOOTH, bluetooth);
        values.put(GPS_LOCATION, gpsLocation);
        values.put(NETWORK_LOCATION, networkLocation);
        values.put(BRIGHTNESS, brightness);
        return mSQLiteDatabase.insert(TABLE_NAME, ID, values);
    }

    public boolean updateProfile(Profile mProfile) {

        int id = mProfile.getId();
        String profile_name = ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName());
        boolean silent = mProfile.isSilent();
        boolean vibrate = mProfile.isVibrate();
        String ringtone1 = mProfile.getRingtone1();
        String ringtone2 = mProfile.getRingtone2();
        int ring_volume = mProfile.getRingVolume();
        boolean data = mProfile.isData();
        boolean wifi = mProfile.isWifi();
        boolean bluetooth = mProfile.isBluetooth();
        boolean gpsLocation = mProfile.isGpsLocation();
        boolean networkLocation = mProfile.isNetworkLocation();
        int brightness = mProfile.getBrightness();

        ContentValues values = new ContentValues();
        values.put(PROFILE_NAME, profile_name);
        values.put(SILENT, silent);
        values.put(VIBRATE, vibrate);
        values.put(RINGTONE1, ringtone1);
        values.put(RINGTONE2, ringtone2);
        values.put(RING_VOLUME, ring_volume);
        values.put(DATA, data);
        values.put(WIFI, wifi);
        values.put(BLUETOOTH, bluetooth);
        values.put(GPS_LOCATION, gpsLocation);
        values.put(NETWORK_LOCATION, networkLocation);
        values.put(BRIGHTNESS, brightness);
        return mSQLiteDatabase.update(TABLE_NAME, values, ID + "=" + id, null) > 0;
    }

    public Cursor fetchProfile(int id) throws SQLException {

        logd("");
        // Cursor mCursor = mSQLiteDatabase.query(false, TABLE_NAME, null, ID +
        // "=" + id, null, null, null, null, null);
        Cursor mCursor = mSQLiteDatabase.query(false, TABLE_NAME, new String[] {
                ID, PROFILE_NAME, SILENT, VIBRATE, RINGTONE1, RINGTONE2, RING_VOLUME, DATA, WIFI,
                BLUETOOTH, GPS_LOCATION, NETWORK_LOCATION, BRIGHTNESS
        }, ID + "=" + id, null, null, null, null, null);

        if (mCursor != null) {
            mCursor.moveToFirst();
        }
        return mCursor;
    }

    public Cursor fetchAllProfiles() {

        logd("");
        return mSQLiteDatabase.query(TABLE_NAME, null, null, null, null, null, null);
    }

    public boolean deleteProfile(int id) {

        return mSQLiteDatabase.delete(TABLE_NAME, ID + "=" + id, null) > 0;
    }

    public boolean deleteAllProfiles() {

        return mSQLiteDatabase.delete(TABLE_NAME, null, null) > 0;
    }

    private static void logd(Object s) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

}
