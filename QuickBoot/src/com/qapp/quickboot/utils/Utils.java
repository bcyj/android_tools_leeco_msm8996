/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.utils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import android.app.ActivityManager;
import android.app.ActivityManager.RecentTaskInfo;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.Vibrator;
import android.provider.Settings;
import android.util.Log;
import android.view.IWindowManager;

import com.android.internal.statusbar.IStatusBarService;

public class Utils {

    public static void saveStatus(Context context) {

        logd("");
        boolean airplaneMode = isAirplaneModeOn(context);
        UtilsSharedPreferences.saveBooleanValue(context, Values.KEY_SP_AIRPLANE_MODE, airplaneMode);

    }

    public static void restoreStatus(Context context) {

        logd("");
        boolean airplaneMode = UtilsSharedPreferences.getBooleanValueSaved(context,
                Values.KEY_SP_AIRPLANE_MODE, isAirplaneModeOn(context));
        setAirplaneMode(context, airplaneMode);
    }

    public static void setOngoingFlag(Context context) {

        logd("");
        SystemProperties.set(Values.KEY_QUICKBOOT_ONGOING, "true");
    }

    public static void clearOngoingFlag(Context context) {

        logd("");
        SystemProperties.set(Values.KEY_QUICKBOOT_ONGOING, null);
    }

    public static void syncStorage(Context context) {

        try {
            java.lang.Process process = Runtime.getRuntime().exec("sync");
            process.waitFor();
            logd("ExitValue=" + process.exitValue());

        } catch (IOException e) {
        } catch (InterruptedException e) {
        }

    }

    public static boolean isOngoing(Context context) {

        return SystemProperties.getBoolean(Values.KEY_QUICKBOOT_ONGOING, false);
    }

    public static boolean isUnderQuickBoot(Context context) {

        return "1".equals(SystemProperties.get(Values.KEY_QUICKBOOT_ENABLE, "0"));
    }

    public static void enableButtonLight(boolean enable) {

        // TODO:KK has different implementation
    }

    public static void exit(Context context) {

        logd("");
        UtilsProcess.killApplication(context.getPackageName(), Process.myUid());
    }

    public static void setAirplaneMode(Context context, boolean enable) {

        if (isAirplaneModeOn(context) == enable)
            return;
        Settings.Global.putInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON,
                enable ? 1 : 0);
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", enable);
        context.sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    public static boolean isAirplaneModeOn(Context context) {

        return (Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) == 1);
    }

    public static void setUSB(String usbConfig) {
        SystemProperties.set(Values.KEY_USB_CONFIG, usbConfig);
    }

    public static void startBootAnimation() {
        logd("");
        SystemProperties.set("service.bootanim.exit", "0");
        SystemProperties.set("ctl.start", "bootanim");
    }

    public static void stopBootAnimation() {
        logd("");
        SystemProperties.set("ctl.stop", "bootanim");
    }

    public static void startQbCharger() {
        logd("");
        SystemProperties.set("sys.qbcharger.enable", "true");
    }

    public static void stopQbCharger() {
        logd("");
        SystemProperties.set("sys.qbcharger.enable", "false");
    }

    public static void sleep(Context context) {
        logd("");
        ((PowerManager) context.getSystemService(Context.POWER_SERVICE)).goToSleep(SystemClock
                .uptimeMillis());
    }

    public static void wakeup(Context context) {
        logd("");
        ((PowerManager) context.getSystemService(Context.POWER_SERVICE)).wakeUp(SystemClock
                .uptimeMillis());
    }

    public static void vibrate(Context context) {
        vibrate(context, Values.POWEROFF_VIBRATE_TIME);
    }

    public static void vibrate(Context context, long vibrateTime) {
        logd("");
        // Vibrator vibrator = new SystemVibrator();
        Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        vibrator.vibrate(vibrateTime);

    }

    public static void enableWindowInput(boolean enable) {
        logd("");
        IWindowManager mWindowManager = (IWindowManager) IWindowManager.Stub
                .asInterface(ServiceManager.getService("window"));
        try {
            mWindowManager.setEventDispatching(enable);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static void clearNotification() {
        logd("");
        IStatusBarService statusBarService = IStatusBarService.Stub.asInterface(ServiceManager
                .getService("statusbar"));
        try {
            statusBarService.onClearAllNotifications(Process.SYSTEM_UID);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static void clearRecentApps(Context context) {
        logd("");
        ActivityManager activityManager = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);

        ArrayList<String> protectedAppList = UtilsProcess.getLauncherList(context);
        protectedAppList.add(context.getPackageName());

        final List<ActivityManager.RecentTaskInfo> recentTasks = activityManager.getRecentTasks(50,
                ActivityManager.RECENT_IGNORE_UNAVAILABLE);

        for (RecentTaskInfo recentTaskInfo : recentTasks) {
            Intent baseIntent = recentTaskInfo.baseIntent;
            if (baseIntent == null)
                continue;
            String packageName = baseIntent.getComponent().getPackageName();
            if (protectedAppList.contains(packageName))
                continue;

            logd("Remove " + packageName);
            activityManager.removeTask(recentTaskInfo.persistentId);
        }
    }

    private static void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(Values.TAG, s + "");
    }
}
