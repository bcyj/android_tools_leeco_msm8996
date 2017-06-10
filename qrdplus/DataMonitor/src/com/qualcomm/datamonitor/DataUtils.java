/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.datamonitor;

import java.text.DecimalFormat;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

public class DataUtils {

    public static final int KbFractionDigits = 1;
    public static int widgetTextColor = Color.LTGRAY;

    public static final String KEY_MONTH_THIS = "monthThis";
    public static final String KEY_MONTH_LAST = "monthLast";
    public static final String KEY_DATA_MONTH = "dataMonth";
    public static final String KEY_DATA_HISTORY = "dataHistory";
    public static final String KEY_WIFI_MONTH = "wifiMonth";
    public static final String KEY_WIFI_HISTORY = "wifiHistory";
    public static final String KEY_DATA_LIMIT = "mobileMonthLimit";

    public static final String KEY_DATA_MONTH_COUNTER = "monthMobileCounter0";
    public static final String KEY_DATA_HISTORY_COUNTER = "historyMobileCounter0";
    public static final String KEY_WIFI_MONTH_COUNTER = "monthWifiCounter0";
    public static final String KEY_WIFI_HISTORY_COUNTER = "historyWifiCounter0";

    static final boolean DEBUG_MODE = false;
    static boolean LOG_ON = true;
    static final int DEFAULT_MONTH_LIMIT = 50;
    static final double DEFAULT_WARN_RATIO = 0.95;
    static final long SECOND = 1000;
    static final long MINUTE = SECOND * 60;
    static final long HOUR = MINUTE * 60;
    static final long KB = 1024;
    static final long MB = 1024 * KB;
    static final long GB = 1024 * MB;
    static final long TB = 1024 * GB;
    static final long DEFAULT_PERIOD = 1 * MINUTE;
    // mode of clear operation
    static final int CLEAR_ALL = 0;
    static final int CLEAR_MOBILE_MONTH = 1;
    static final int CLEAR_WIFI_MONTH = 2;
    static final int CLEAR_MOBILE_ALL = 3;
    static final int CLEAR_WIFI_ALL = 4;
    static final boolean ENABLE_CONNECTIVITY = true;
    static final boolean DISABLE_CONNECTIVITY = false;

    // choose update period
    static final Long[] periodValues = { 30 * SECOND, 1 * MINUTE, 5 * MINUTE,
            10 * MINUTE, 30 * MINUTE, 1 * HOUR };
    static final int NOTIFICATION_ID = 11426;

    static final String ACTION_NETWORK_TOOGLE = "traffic.action.ACTION_NETWORK_TOOGLE";
    static final String ACTION_CONNECTIVITY_CHANGE = ConnectivityManager.CONNECTIVITY_ACTION;
    // The broadcast we want to receive when setMobileDataEnabled is called
    static final String ACTION_MOBILE_CONNECTIVITY_CHANGE =
        "android.net.conn.MOBILE_CONNECTIVITY_CHANGE";
    static final String ACTION_TIME_CHANGE = "android.intent.action.TIME_SET";

    public static String formatDataToString(long data) {

        DecimalFormat formater = new DecimalFormat();
        formater.setGroupingSize(0);
        String s = "";
        if (Math.abs(data) >= TB) {
            formater.setMaximumFractionDigits(KbFractionDigits + 3);
            s += formater.format((double) data / TB) + "TB";
        } else if (Math.abs(data) >= GB) {
            formater.setMaximumFractionDigits(KbFractionDigits + 2);
            s += formater.format((double) data / GB) + "GB";
        } else if (Math.abs(data) >= MB) {
            formater.setMaximumFractionDigits(KbFractionDigits + 1);
            s += formater.format((double) data / MB) + "MB";
        } else {
            formater.setMaximumFractionDigits(KbFractionDigits);
            s += formater.format((double) data / KB) + "KB";
        }
        return s;
    }

    public static long getLongValueSaved(Context mContext, String key, long def) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return stateSaved.getLong(key, def);
    }

    public static int getIntValueSaved(Context mContext, String key, int def) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return stateSaved.getInt(key, def);

    }

    public static boolean getBooleanValueSaved(Context mContext, String key,
            boolean def) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return stateSaved.getBoolean(key, def);

    }

    public static void saveIntValue(Context mContext, String key, int value) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = stateSaved.edit();
        editor.putInt(key, value);
        editor.commit();

    }

    public static void saveStringValue(Context mContext, String key,
            String value) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = stateSaved.edit();
        editor.putString(key, value);
        editor.commit();

    }

    public static String getStringValueSaved(Context mContext, String key,
            String def) {

        SharedPreferences stateSaved = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return stateSaved.getString(key, def);
    }

    public static boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    public static TelephonyManager getTelephonyManager(Context context) {
        return ((TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE));
    }

    private static boolean hasIccCard(Context context) {
        TelephonyManager telephonyManager = getTelephonyManager(context);
        if (telephonyManager.isMultiSimEnabled()) {
            int prfDataSlotId = SubscriptionManager.getSlotId(
                    SubscriptionManager.getDefaultDataSubId());
            int simState = telephonyManager.getSimState(prfDataSlotId);
            boolean active = (simState != TelephonyManager.SIM_STATE_ABSENT)
                    && (simState != TelephonyManager.SIM_STATE_UNKNOWN);
            return active && telephonyManager.hasIccCard(prfDataSlotId);
        } else {
            return telephonyManager.hasIccCard();
        }
    }

    public static void setMobileDataEnabled(Context context, boolean enabled) {
        TelephonyManager telephonyManager = getTelephonyManager(context);
        if (isAirplaneModeOn(context) || !hasIccCard(context)) {
            return;
        }

        int phoneCount = telephonyManager.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            Settings.Global.putInt(context.getContentResolver(),
                    Settings.Global.MOBILE_DATA + i, (enabled) ? 1 : 0);
            int[] subId = SubscriptionManager.getSubId(i);
            telephonyManager.setDataEnabled(subId[0], enabled);
        }
    }

    public static boolean getMobileDataEnabled(Context context) {
        TelephonyManager telephonyManager = getTelephonyManager(context);
        return !isAirplaneModeOn(context) && hasIccCard(context) && telephonyManager.getDataEnabled();
    }
}
