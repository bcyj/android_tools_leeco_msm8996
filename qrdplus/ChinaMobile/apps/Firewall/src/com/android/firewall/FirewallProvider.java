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
/*
 * oasis_zp@hmct add blacklist app
 */

package com.android.firewall;

import android.content.*;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

import android.provider.BaseColumns;
import android.util.Config;
import android.util.Log;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.os.StatFs;
import android.provider.ContactsContract.PhoneLookup;
import android.content.ContentResolver;
import android.content.ContentUris;

import com.android.internal.telephony.PhoneConstants;

import java.io.File;
import java.util.ArrayList;

public class FirewallProvider extends ContentProvider
{
    private static final String DATABASE_NAME = "firewall.db";

    private static final int BLACKLIST_ALL = 1;
    private static final int BLACKLIST_CURRENT = 2;
    private static final int WHITELIST_ALL = 3;
    private static final int WHITELIST_CURRENT = 4;
    private static final int FIREWALL_SWITCHER = 5;
    private static final int FIREWALL_MODE = 6;
    private static final int FIREWALL_BLOCK_C = 7;
    private static final int FIREWALL_BLOCK_G = 8;

    private static final int SMS_FIREWALL_SWITCHER = 9;
    private static final int SMS_FIREWALL_MODE = 10;

    private static final int BLOCKRECORD_ALL = 11;

    private static final String TAG = "FirewallProvider";
    private static final String BLACKLIST_TABLE = "blacklistitems";
    private static final String WHITELIST_TABLE = "whitelistitems";
    private static final String BLOCKRECORD_TABLE = "blockrecorditems";
    private static final String FIREWALL_SETTING_TABLE = "firewallsettings";
    private static final String EXTRA_NUMBER = "phonenumber";
    private static final int DEFAULT_SUBSCRIPTION = 0;

    private static final UriMatcher s_urlMatcher = new UriMatcher(UriMatcher.NO_MATCH);

    static {
        s_urlMatcher.addURI("com.android.firewall", "blacklistitems", BLACKLIST_ALL);
        s_urlMatcher.addURI("com.android.firewall", "blacklistitems/default", BLACKLIST_CURRENT);
        s_urlMatcher.addURI("com.android.firewall", "whitelistitems", WHITELIST_ALL);
        s_urlMatcher.addURI("com.android.firewall", "whitelistitems/default", WHITELIST_CURRENT);
        s_urlMatcher.addURI("com.android.firewall", "setting/1", FIREWALL_SWITCHER);
        s_urlMatcher.addURI("com.android.firewall", "setting/2", FIREWALL_MODE);
        s_urlMatcher.addURI("com.android.firewall", "setting/3", FIREWALL_BLOCK_C);
        s_urlMatcher.addURI("com.android.firewall", "setting/4", FIREWALL_BLOCK_G);

        s_urlMatcher.addURI("com.android.firewall", "setting/5", SMS_FIREWALL_SWITCHER);
        s_urlMatcher.addURI("com.android.firewall", "setting/6", SMS_FIREWALL_MODE);

        s_urlMatcher.addURI("com.android.firewall", "blockrecorditems", BLOCKRECORD_ALL);
    }

    private static final Uri SWITCHER_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/1");
    private static final Uri MODE_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/2");
    private static final Uri BLOCK_C_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/3");
    private static final Uri BLOCK_G_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/4");

    private static final Uri SMS_SWITCHER_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/5");
    private static final Uri SMS_MODE_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/6");

    private ContentResolver mContentResolver;

    // Firewall mode
    private static final int ONLY_BLOCK_BLACKLIST = 0;
    private static final int ONLY_ACCEPT_WHITELIST = 1;
    private static final int BLOCK_ALL = 2;
    private static final int ONLY_ACCEPT_CONTACTS = 3;
    private static final int BLOCK_CARD_ONE = 4;
    private static final int BLOCK_CARD_TWO = 5;

    private static final String IS_FORBIDDEN="isForbidden";

    public static final class Blacklist implements BaseColumns {

        public static final Uri CONTENT_URI = Uri
                .parse("content://com.android.firewall/blacklistitems");

        public static final Uri DEFAULT_BLACKLIST_URI =
                Uri.parse("content://com.android.firewall/blacklistitems/default");

        public static final String DEFAULT_SORT_ORDER = "_id ASC";

        public static final String NUMBER = "number";

        public static final String NAME = "name";

        public static final String PERSON_ID = "person_id";

        // block flag 1 is block (PCI ADD)
        public static final String CALL_BLOCKED_FLAG = "call_block";

        public static final String SMS_BLOCKED_FLAG = "sms_block";

    }

    public static final class Whitelist implements BaseColumns {

        public static final Uri CONTENT_URI = Uri
                .parse("content://com.android.firewall/whitelistitems");

        public static final Uri DEFAULT_WHITELIST_URI =
                Uri.parse("content://com.android.firewall/whitelistitems/default");

        public static final String DEFAULT_SORT_ORDER = "_id ASC";

        public static final String NUMBER = "number";

        public static final String NAME = "name";

        public static final String PERSON_ID = "person_id";
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {
        // Context to access resources with
        // private Context mContext;

        static final int DATABASE_VERSION = 3;

        public DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
            // mContext = context;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {

            // Set up the database schema
            db.execSQL("CREATE TABLE " + FIREWALL_SETTING_TABLE +
                    " (_id INTEGER PRIMARY KEY," +
                    "name TEXT UNIQUE," +
                    "value TEXT);");

            db.execSQL("CREATE TABLE " + BLACKLIST_TABLE +
                    " (_id INTEGER PRIMARY KEY," +
                    "number TEXT UNIQUE," +
                    "person_id INTEGER, " +
                    "call_block INTEGER DEFAULT 1, " +
                    "sms_block INTEGER DEFAULT 1, " +
                    "name TEXT);");

            db.execSQL("CREATE TABLE " + WHITELIST_TABLE +
                    " (_id INTEGER PRIMARY KEY," +
                    "number TEXT UNIQUE," +
                    "person_id INTEGER, " +
                    "name TEXT);");

            // BlockRecord
            db.execSQL(BlockRecord.CREATE_TABLE_BLOCK_RECORD);

            initDatabase(db);
        }

        private void initDatabase(SQLiteDatabase db) {
            // db.execSQL("INSERT INTO " + BLACKLIST_TABLE +
            // " (number, person_id, name) VALUES (\"17901\", 0, \"17901\");");
            // 0 --> 1 enable firewall as default
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"switcher\", \"0\");");
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"mode\", \"0\");");
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"block_c\", \"0\");");
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"block_g\", \"0\");");
            // 0 disable sms firewall as default
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"sms_switcher\", \"0\");");
            db.execSQL("INSERT INTO " + FIREWALL_SETTING_TABLE +
                    " (name, value) VALUES (\"sms_mode\", \"0\");");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + FIREWALL_SETTING_TABLE + ";");
            db.execSQL("DROP TABLE IF EXISTS " + BLACKLIST_TABLE + ";");
            db.execSQL("DROP TABLE IF EXISTS " + WHITELIST_TABLE + ";");
            db.execSQL("DROP TABLE IF EXISTS " + BLOCKRECORD_TABLE + ";");

            onCreate(db);
        }
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new DatabaseHelper(getContext());
        mContentResolver = getContext().getContentResolver();
        return true;
    }

    @Override
    public Cursor query(Uri url, String[] projectionIn, String selection,
            String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        int match = s_urlMatcher.match(url);
        switch (match) {
        // do nothing
            case BLACKLIST_ALL: {
                qb.setTables(BLACKLIST_TABLE);
                break;
            }

            case BLACKLIST_CURRENT: {
                qb.setTables(BLACKLIST_TABLE);
                qb.appendWhere("_id = 1");
                break;
            }

            case WHITELIST_ALL: {
                qb.setTables(WHITELIST_TABLE);
                break;
            }

            case WHITELIST_CURRENT: {
                qb.setTables(WHITELIST_TABLE);
                qb.appendWhere("_id = 1");
                break;
            }

            case FIREWALL_SWITCHER: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 1");
                break;
            }

            case FIREWALL_MODE: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 2");
                break;
            }

            case FIREWALL_BLOCK_C: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 3");
                break;
            }

            case FIREWALL_BLOCK_G: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 4");
                break;
            }

            case SMS_FIREWALL_SWITCHER: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 5");
                break;
            }

            case SMS_FIREWALL_MODE: {
                qb.setTables(FIREWALL_SETTING_TABLE);
                // qb.appendWhere("_id = 6");
                break;
            }
            case BLOCKRECORD_ALL: {
                qb.setTables(BLOCKRECORD_TABLE);
                break;
            }

            default: {
                return null;
            }
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor ret = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);
        ret.setNotificationUri(getContext().getContentResolver(), url);
        return ret;
    }

    @Override
    public String getType(Uri url)
    {
        switch (s_urlMatcher.match(url)) {
            case BLACKLIST_ALL:
                return "vnd.android.cursor.dir/blacklistitems";

            case BLACKLIST_CURRENT:
                return "vnd.android.cursor.item/blacklistitems";

            case WHITELIST_ALL:
                return "vnd.android.cursor.dir/whitelistitems";

            case WHITELIST_CURRENT:
                return "vnd.android.cursor.item/whitelistitems";

            case FIREWALL_SWITCHER:
                return "vnd.android.cursor.item/firewallsettings";

            case FIREWALL_MODE:
                return "vnd.android.cursor.item/firewallsettings";

            case FIREWALL_BLOCK_C:
                return "vnd.android.cursor.item/firewallsettings";

            case FIREWALL_BLOCK_G:
                return "vnd.android.cursor.item/firewallsettings";

            case SMS_FIREWALL_SWITCHER:
                return "vnd.android.cursor.item/firewallsettings";

            case SMS_FIREWALL_MODE:
                return "vnd.android.cursor.item/firewallsettings";

            default:
                throw new IllegalArgumentException("Unknown URL " + url);
        }
    }

    @Override
    public Uri insert(Uri url, ContentValues initialValues)
    {
        Uri result = null;
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int match = s_urlMatcher.match(url);
        boolean notify = false;
        switch (match)
        {
            case BLACKLIST_ALL: {
                ContentValues values;
                if (initialValues != null) {
                    values = new ContentValues(initialValues);
                } else {
                    values = new ContentValues();
                }

                if (values.containsKey(Blacklist.NUMBER) == false) {
                    values.put(Blacklist.NUMBER, "");
                }
                if (values.containsKey(Blacklist.NAME) == false) {
                    values.put(Blacklist.NAME, 0);
                }

                long rowID = db.insert(BLACKLIST_TABLE, null, values);
                if (rowID > 0)
                {
                    result = ContentUris.withAppendedId(Blacklist.CONTENT_URI, rowID);
                    notify = true;
                }

                if (Config.LOGD)
                    Log.d(TAG, "inserted " + values.toString() + " rowID = " + rowID);
                if (notify) {
                    if (Config.LOGD)
                        Log.d(TAG, "inserted send notification");
                    getContext().getContentResolver().notifyChange(Blacklist.CONTENT_URI, null);
                }
                break;
            }
            case WHITELIST_ALL: {
                ContentValues values;
                if (initialValues != null) {
                    values = new ContentValues(initialValues);
                } else {
                    values = new ContentValues();
                }

                if (values.containsKey(Whitelist.NUMBER) == false) {
                    values.put(Whitelist.NUMBER, "");
                }
                if (values.containsKey(Whitelist.NAME) == false) {
                    values.put(Whitelist.NAME, 0);
                }

                long rowID = db.insert(WHITELIST_TABLE, null, values);
                if (rowID > 0)
                {
                    result = ContentUris.withAppendedId(Whitelist.CONTENT_URI, rowID);
                    notify = true;
                }

                if (Config.LOGD)
                    Log.d(TAG, "inserted " + values.toString() + " rowID = " + rowID);
                if (notify) {
                    if (Config.LOGD)
                        Log.d(TAG, "inserted send notification");
                    getContext().getContentResolver().notifyChange(Whitelist.CONTENT_URI, null);
                }
                break;
            }
            case BLOCKRECORD_ALL: {
                ContentValues values;
                if (initialValues != null) {
                    values = new ContentValues(initialValues);
                } else {
                    values = new ContentValues();
                }
                long rowID = db.insert(BLOCKRECORD_TABLE, null, values);
                if (rowID > 0) {
                    result = ContentUris.withAppendedId(BlockRecord.CONTENT_URI, rowID);
                    notify = true;
                }

                if (Config.LOGD)
                    Log.d(TAG, "inserted " + values.toString() + " rowID = " + rowID);
                if (notify) {
                    if (Config.LOGD)
                        Log.d(TAG, "inserted send notification");
                    getContext().getContentResolver().notifyChange(Blacklist.CONTENT_URI, null);
                }
                break;
            }
            default:
                break;
        }

        return result;
    }

    @Override
    public int delete(Uri url, String where, String[] whereArgs)
    {
        int count;

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int match = s_urlMatcher.match(url);
        switch (match)
        {
            case BLACKLIST_ALL: {
                count = db.delete(BLACKLIST_TABLE, where, whereArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(Blacklist.CONTENT_URI, null);
                }
                break;
            }

            case WHITELIST_ALL: {
                count = db.delete(WHITELIST_TABLE, where, whereArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(Whitelist.CONTENT_URI, null);
                }
                break;
            }
            case BLOCKRECORD_ALL: {
                count = db.delete(BLOCKRECORD_TABLE, where, whereArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(BlockRecord.CONTENT_URI, null);
                }
                break;
            }

            default: {
                throw new UnsupportedOperationException("Cannot delete that URL: " + url);
            }
        }

        return count;
    }

    @Override
    public int update(Uri url, ContentValues values, String where, String[] whereArgs)
    {
        int count = 0;

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int match = s_urlMatcher.match(url);
        switch (match)
        {
            case BLACKLIST_ALL: {
                count = db.update(BLACKLIST_TABLE, values, where, whereArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(Blacklist.CONTENT_URI, null);
                }
                break;
            }

            case WHITELIST_ALL: {
                count = db.update(WHITELIST_TABLE, values, where, whereArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(Whitelist.CONTENT_URI, null);
                }
                break;
            }

            case FIREWALL_SWITCHER: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "1"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(SWITCHER_CONTENT_URI, null);
                }

                break;
            }

            case FIREWALL_MODE: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "2"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(MODE_CONTENT_URI, null);
                }
                break;
            }

            case FIREWALL_BLOCK_C: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "3"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(BLOCK_C_CONTENT_URI, null);
                }

                break;
            }

            case FIREWALL_BLOCK_G: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "4"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(BLOCK_G_CONTENT_URI, null);
                }
                break;
            }

            case SMS_FIREWALL_SWITCHER: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "5"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(SMS_SWITCHER_CONTENT_URI, null);
                }

                break;
            }

            case SMS_FIREWALL_MODE: {
                count = db.update(FIREWALL_SETTING_TABLE, values, "_id=?", new String[] {
                    "6"
                });
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(SMS_MODE_CONTENT_URI, null);
                }
                break;
            }

            default: {
                throw new UnsupportedOperationException("Cannot update that URL: " + url);
            }
        }

        return count;
    }

    @Override
    public Bundle call(String method, String arg, Bundle extras) {
        int subscription = extras.getInt(PhoneConstants.SUBSCRIPTION_KEY,
                DEFAULT_SUBSCRIPTION);
        String number = extras.getString(EXTRA_NUMBER);

        if (number == null) {
            return null;
        }
        if (IS_FORBIDDEN.equals(method)) {
            extras.putBoolean(IS_FORBIDDEN, isForbidden(subscription, number));
            return extras;
        }
        return null;
    }

    private SQLiteOpenHelper mOpenHelper;

    public static String bestNumMatch(String callingNum, Context mcontext) {
        Uri uri = callingNumMatch(callingNum, mcontext);

        if (uri != null)
        {
            if (uri.getPathSegments().size() >= 1)
                return uri.getPathSegments().get(1);
        }

        return null;
    }

    public static Uri callingNumMatch(String callingNum, Context mcontext) {
        Uri nameMatching = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI,
                Uri.encode(callingNum));
        ContentResolver cr = mcontext.getContentResolver();
        String[] projection = new String[] {
            PhoneLookup.NUMBER
        };
        Cursor mcursor = null;
        try {
            mcursor = cr.query(nameMatching, projection, null, null, null);
        } catch (IllegalArgumentException e) {
        }
        ArrayList<String> numArray = new ArrayList<String>();
        if (mcursor != null) {
            try {
                // mcursor.moveToPosition(-1);
                while (mcursor.moveToNext()) {
                    numArray.add(mcursor.getString(0));
                }
            } finally {
                mcursor.close();
            }
        }

        if (callingNum.length() >= 11) {
            ArrayList<String> matchingNums = new ArrayList<String>();

            for (int i = 0; i < numArray.size(); i++) {
                if (numArray.get(i).length() >= 11) {
                    String numComing = callingNum.substring(callingNum.length() - 11);
                    String numInPhone = numArray.get(i).substring(numArray.get(i).length() - 11);
                    if (numComing.equals(numInPhone)) {
                        nameMatching = matchingName(numArray.get(i), cr);
                        return nameMatching;
                    }
                }
                else if (numArray.get(i).length() >= 7 && numArray.get(i).length() < 11) {
                    String numComing = callingNum.substring(callingNum.length()
                            - numArray.get(i).length());
                    if (numComing.equals(numArray.get(i)))
                        matchingNums.add(numArray.get(i));
                }
            }
            String maxMatchNum = maxMatching(callingNum, matchingNums);
            if (maxMatchNum != null) {
                nameMatching = matchingName(maxMatchNum, cr);
            }
        }
        else if (callingNum.length() >= 7 && callingNum.length() < 11) {
            ArrayList<String> matchingNums = new ArrayList<String>();

            for (int i = 0; i < numArray.size(); i++) {
                if (numArray.get(i).length() >= 11) {
                    String subPhoneNumber = numArray.get(i).substring(
                            numArray.get(i).length() - callingNum.length());
                    if (callingNum.equals(subPhoneNumber)) {
                        // nameMatching = matchingName(numArray.get(i), cr);
                        // return nameMatching;
                        matchingNums.add(numArray.get(i));
                    }
                }
                else if (numArray.get(i).length() >= 7 && numArray.get(i).length() < 11) {

                    if (callingNum.length() < numArray.get(i).length()) {

                        String subPhoneNumber = numArray.get(i).substring(
                                numArray.get(i).length() - callingNum.length());
                        if (callingNum.equals(subPhoneNumber)) {
                            // nameMatching = matchingName(numArray.get(i), cr);
                            // return nameMatching;
                            matchingNums.add(numArray.get(i));
                        }
                    }
                    else if (callingNum.length() > numArray.get(i).length()) {

                        String subcallingNum = callingNum.substring(callingNum.length()
                                - numArray.get(i).length());
                        if (subcallingNum.equals(numArray.get(i))) {
                            matchingNums.add(numArray.get(i));
                        }
                    }
                    else if (callingNum.length() == numArray.get(i).length()) {

                        if (callingNum.equals(numArray.get(i))) {

                            nameMatching = matchingName(numArray.get(i), cr);
                            return nameMatching;
                        }
                    }
                }
            }
            String maxMatchNum = maxMatching(callingNum, matchingNums);
            if (maxMatchNum != null) {
                nameMatching = matchingName(maxMatchNum, cr);
            }
        }
        else if (callingNum.length() < 7) {
            for (int i = 0; i < numArray.size(); i++) {
                if (callingNum.equals(numArray.get(i))) {
                    nameMatching = matchingName(numArray.get(i), cr);
                    return nameMatching;
                }
            }
        }
        return nameMatching;
    }

    public static Uri matchingName(String matchNum, ContentResolver cr) {
        // String nameMatching = null;
        // long ID = -1;
        Uri resultUri = null;
        resultUri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri.encode(matchNum));
        return resultUri;
    }

    public static String maxMatching(String callingNum, ArrayList<String> mArray) {
        for (int i = 0; i < mArray.size(); i++) {
            if (callingNum.equals(mArray.get(i)))
                return mArray.get(i);
        }
        if (mArray != null && mArray.size() != 0) {
            String strmaxMatching = mArray.get(0);
            for (int i = 0; i < mArray.size(); i++) {
                if (mArray.get(i).length() > strmaxMatching.length()) {
                    strmaxMatching = mArray.get(i);
                }
            }
            return strmaxMatching;
        } else {
            return null;
        }
    }

    /**
     * Check the incoming phone number and return.
     * Firewall is not enable, return false;
     * Firewall is white mode, and the number is in the list, return false;
     * Firewall is black mode, and the number is not in the list, return false;
     * If the number lenth is longer than 11, only use the last 11st num.
     */
    public boolean isForbidden(int subscription, String number) {
        if (number == null || number.length() == 0) {
            return false;
        }
        if (checkdataFull()) {
            return false;
        }
        if (!isFirewallEnabled()) {
            return false;
        }

        //Check the last 11 numbers
        int len = number.length();
        if (len > 11) {
            number = number.substring(len - 11, len);
        }
        boolean result = isForbiddenNumber(subscription, number);
        return result;
    }

    //Query the incoming phone number is forbidden or not.
    private boolean isForbiddenNumber(int subscription, String number) {
        if (mContentResolver == null) {
            return false;
        }

        if (number != null && !number.equals("")) {
            switch (Integer.valueOf(getFirewallMode())) {
                case ONLY_BLOCK_BLACKLIST:
                {
                    Cursor c = mContentResolver.query(Blacklist.CONTENT_URI,
                            new String[] {"_id", "number", "person_id", "name"},
                            "number" + " LIKE '%" + number +"'", null, null);
                    if (c == null) {
                        return false;
                    }
                    try {
                        if (c.getCount() > 0) {
                            return true;
                        } else {
                            return false;
                        }
                    } finally {
                        c.close();
                        c = null;
                    }
                }
                    //break;
                case ONLY_ACCEPT_WHITELIST:
                {
                    Cursor c = mContentResolver.query(Whitelist.CONTENT_URI,
                            new String[] {"_id", "number", "person_id", "name"},
                            "number" + " LIKE '%" + number +"'", null, null);
                    if (c == null) {
                        return true;
                    }
                    try {
                        if (c.getCount() > 0) {
                            return false;
                        } else {
                            return true;
                        }
                    } finally {
                        c.close();
                        c = null;
                    }
                }
                    //break;
                case BLOCK_ALL:
                    return true;
                    //break;
                case ONLY_ACCEPT_CONTACTS:
                {
                    Cursor c = mContentResolver.query(Uri.withAppendedPath(
                            PhoneLookup.CONTENT_FILTER_URI,
                            Uri.encode(number)), null, null, null, null);
                    if (c == null) {
                        return true;
                    }
                    try {
                        if (c.getCount() > 0) {
                            return false;
                        } else {
                            return true;
                        }
                    } finally {
                        c.close();
                        c = null;
                    }
                }
                    //break;
                case BLOCK_CARD_ONE:
                    if (subscription == 0) {
                        return true;
                    } else {
                        return false;
                    }
                    //break;
                case BLOCK_CARD_TWO:
                    if (subscription == 1) {
                        return true;
                    } else {
                        return false;
                    }
                    //break;
                default:
                    break;
            }
        }
        return false;
    }

    // Check the firewall is enabled or not.
    private boolean isFirewallEnabled() {
        boolean result = false;
        if (mContentResolver == null) {
            return false;
        }
        Cursor c = mContentResolver.query(SWITCHER_CONTENT_URI,
                new String[] {"value"},"_id = ?", new String [] {"1"}, null);
        if (c != null) {
            c.moveToFirst();
            result = c.getString(0).equals("1");
            c.close();
            c = null;
        }
        return result;
    }

    private String getFirewallMode() {
        String result = null;
        if (mContentResolver == null) {
            return "";
        }
        Cursor c = mContentResolver.query(MODE_CONTENT_URI,
                new String[] {"value"}, "_id = ?", new String [] {"2"}, null);
        if (c != null) {
            c.moveToFirst();
            result = c.getString(0);
            c.close();
            c = null;
        }
        return result;
    }

    private static boolean checkdataFull() {
        File path = new File("/data/");
        StatFs stat = new StatFs(path.getPath());
        long blockSize = stat.getBlockSize();
        long availableBlocks = stat.getAvailableBlocks();
        long available = availableBlocks*blockSize;
        if (available < 10*1024*1024) {
            return true;
        }
        return false;
    }
}
