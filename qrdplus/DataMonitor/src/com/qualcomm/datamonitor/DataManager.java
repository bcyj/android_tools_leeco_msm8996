/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.datamonitor;

import static android.net.NetworkTemplate.buildTemplateMobileAll;
import static com.qualcomm.datamonitor.DataUtils.LOG_ON;
import android.content.Context;
import android.net.INetworkStatsService;
import android.net.NetworkStatsHistory;
import android.net.NetworkTemplate;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.text.format.Time;
import android.util.Log;
import android.os.INetworkManagementService;

public class DataManager {

    private static Context context;
    private static final String TAG = "DataManager";

    public static Context getContext() {
        return context;
    }

    public static void setContext(Context context) {
        DataManager.context = context;
    }

    public static void init(Context context) {
        DataManager.context = context;
    }
    public static boolean isDataAvailable(){
        final INetworkManagementService netManager = INetworkManagementService.Stub
                .asInterface(ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE));
        try {
            if (!netManager.isBandwidthControlEnabled()) {
                 return false;
            }
        } catch (RemoteException e) {
                 return false;
        }
        return true;
    }

    public static long getMonthData() {
        if (!isDataAvailable()) {
            return -1;
        }
        final TelephonyManager telephony = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        final INetworkStatsService statsService = INetworkStatsService.Stub
                .asInterface(ServiceManager
                        .getService(Context.NETWORK_STATS_SERVICE));

        NetworkTemplate mTemplate = buildTemplateMobileAll(telephony
                .getSubscriberId());
        logd(mTemplate);

        Time mTimeNow = new Time();
        mTimeNow.setToNow();

        Time mTimeStart = new Time();
        // monthDay:1-31 month:0-11
        mTimeStart.set(0, 0, 0, 1, mTimeNow.month, mTimeNow.year);

        final long start = mTimeStart.toMillis(true);
        final long now = mTimeNow.toMillis(true);
        final long end = now;
        logd(start + "/" + end + "/" + now);

        try {
            NetworkStatsHistory nh = statsService.openSession().getHistoryForNetwork(
                    mTemplate, 10);
            if (nh == null)
                return -1;
            android.net.NetworkStatsHistory.Entry entry = nh.getValues(start,
                    end, now, null);
            final long totalBytes = entry != null ? entry.rxBytes
                    + entry.txBytes : -1;
            return totalBytes;
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return -1;
    }

    public static String getMonthDataString() {
        return DataUtils.formatDataToString(getMonthData());
    }

    private static void logd(Object s) {

        if (!LOG_ON)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

}
