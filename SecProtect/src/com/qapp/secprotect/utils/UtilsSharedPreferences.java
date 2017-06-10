/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class UtilsSharedPreferences {

    public static String getStringValueSaved(Context mContext, String key, String def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getString(key, def);
    }

    public static long getLongValueSaved(Context mContext, String key, long def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getLong(key, def);
    }

    public static int getIntValueSaved(Context mContext, String key, int def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getInt(key, def);

    }

    public static boolean getBooleanValueSaved(Context mContext, String key, boolean def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getBoolean(key, def);

    }

    public static void saveBooleanValue(Context mContext, String key, boolean value) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putBoolean(key, value);
        editor.commit();
    }

    public static void saveIntValue(Context mContext, String key, int value) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putInt(key, value);
        editor.commit();
    }

    public static void saveStringValue(Context mContext, String key, String value) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putString(key, value);
        editor.commit();
    }

    public static void saveStringValue(Context mContext, String preferenceName, String key,
            String value) {

        SharedPreferences mSharedPreferences = mContext.getSharedPreferences(preferenceName,
                Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putString(key, value);
        editor.commit();
    }

    public static String getStringValueSaved(Context mContext, String preferenceName, String key,
            String def) {

        SharedPreferences mSharedPreferences = mContext.getSharedPreferences(preferenceName,
                Context.MODE_PRIVATE);
        return mSharedPreferences.getString(key, def);
    }

}
