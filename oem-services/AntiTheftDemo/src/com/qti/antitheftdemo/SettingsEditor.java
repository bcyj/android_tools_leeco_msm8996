/******************************************************************************
 * @file    SettingsEditor.java
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qti.antitheftdemo;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

public class SettingsEditor {

    static public final String PREFS_NAME = "com.qti.antitheftdemo";
    static public final String CONFIRMATION_CODE = "CONFIRMATION_CODE";
    static public final String FRIEND_NUMBER = "FRIEND_NUMBER";
    static public final String PERMISSION_ACCEPTED = "PERMISSION_ACCEPTED";
    static public final String SUBSCRIBER_ID = "SUBSCRIBER_ID";

    private static String getString(Context context, String name) {
        SharedPreferences settings = context.getApplicationContext()
            .getSharedPreferences(PREFS_NAME, 0);
        return settings.getString(name, null);
    }

    private static boolean getBoolean(Context context, String name) {
        SharedPreferences settings = context.getApplicationContext()
            .getSharedPreferences(PREFS_NAME, 0);
        return settings.getBoolean(name, false);
    }

    public static String getConfirmationCode(Context context) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return null;
        }
        return getString(context, CONFIRMATION_CODE);
    }

    public static String getFriendNumber(Context context) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return null;
        }
        return getString(context, FRIEND_NUMBER);
    }

    public static String getSubscriberId(Context context) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return null;
        }
        return getString(context, SUBSCRIBER_ID);
    }

    public static boolean getPermission(Context context) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return false;
        }
        return getBoolean(context, PERMISSION_ACCEPTED);
    }

    private static void setString(Context context, String name, String value) {
        SharedPreferences settings = context.getApplicationContext()
            .getSharedPreferences(PREFS_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.putString(name, value);
        editor.commit();
    }

    private static void setBoolean(Context context, String name, boolean value) {
        SharedPreferences settings = context.getApplicationContext()
            .getSharedPreferences(PREFS_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.putBoolean(name, value);
        editor.commit();
    }

    public static void setConfirmationCode(Context context, String value) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return;
        }
        setString(context, CONFIRMATION_CODE, value);
    }

    public static void setFriendNumber(Context context, String value) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return;
        }
        setString(context, FRIEND_NUMBER, value);
    }

    public static void setSubscriberId(Context context, String value) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return;
        }
        setString(context, SUBSCRIBER_ID, value);
    }

    public static void setPermission(Context context, boolean value) {
        if (context == null) {
            Log.e(Debug.TAG, "Context is null.");
            return;
        }
        setBoolean(context, PERMISSION_ACCEPTED, value);
    }

}
