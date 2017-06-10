/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.utils;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import android.app.ActivityManagerNative;
import android.app.IActivityManager;
import android.app.WallpaperInfo;
import android.app.WallpaperManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;

public class UtilsProcess {

    public static boolean isProcessRunning(String processName) {

        logd(processName);
        boolean ret = false;
        try {
            java.lang.Process process = Runtime.getRuntime().exec("ps");
            process.waitFor();

            InputStream inStream = process.getInputStream();
            InputStreamReader inReader = new InputStreamReader(inStream);
            BufferedReader inBuffer = new BufferedReader(inReader);
            String s;
            while ((s = inBuffer.readLine()) != null) {

                if (s.contains(processName)) {
                    logd(processName + " is running");
                    ret = true;
                    break;
                }
            }

        } catch (Exception e) {
            logd(e);
            ret = false;
        }

        return ret;
    }

    private static boolean isPowerOffAlarmSupported(Context context) {
        // TODO:
        return true;
    }

    private static ArrayList<String> getProcessWhiteList(Context context) {
        final ArrayList<String> whiteList = new ArrayList<String>();
        // Add process which need to be kept
        if (isPowerOffAlarmSupported(context))
            whiteList.add("com.android.deskclock");
        // whiteList.add("android.process.acore");
        // whiteList.add("android.process.media");
        return whiteList;
    }

    private static boolean isKillablePackage(Context context, PackageInfo packageInfo) {

        // nonsystem apps considered as killable
        if ((packageInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0)
            return true;

        if ((packageInfo.applicationInfo.flags & ApplicationInfo.FLAG_PERSISTENT) != 0)
            return false;

        return true;
    }

    public static void killApplications(Context context) {

        PackageManager packageManager = context.getPackageManager();
        IActivityManager activityManager = ActivityManagerNative.getDefault();

        List<PackageInfo> installedPackages = packageManager.getInstalledPackages(0);

        String wallPaper = getLiveWallPaper(context);
        ArrayList<String> launcherList = getLauncherList(context);
        ArrayList<String> processWhiteList = getProcessWhiteList(context);

        ArrayList<String> killedList = new ArrayList<String>();
        for (PackageInfo packageInfo : installedPackages) {

            String packageName = packageInfo.packageName;

            // don't kill wallPaper, Launcher
            if (launcherList.contains(packageName) || packageName.equals(wallPaper)
                    || packageName.equals(context.getPackageName()))
                continue;

            if (processWhiteList.contains(packageInfo.applicationInfo.processName))
                continue;

            if (isKillablePackage(context, packageInfo)) {
                // logd(packageInfo.packageName);
                killedList.add(packageName);
                try {
                    activityManager.killApplicationWithAppId(packageName,
                            packageInfo.applicationInfo.uid, "quickboot");

                } catch (RemoteException e) {
                    logd(e);
                }
            }

        }
        clearAppAlarms(context, killedList);
    }

    private static void clearAppAlarms(Context context, ArrayList<String> list) {

        if (list == null || list.size() <= 0)
            return;

        String[] packageList = new String[list.size()];
        for (int i = 0; i < packageList.length; i++) {
            packageList[i] = list.get(i);
        }
        Intent intent = new Intent(Values.INTENT_QUICKBOOT_APPKILLED);
        intent.putExtra(Intent.EXTRA_PACKAGES, packageList);
        context.sendBroadcast(intent, "android.permission.DEVICE_POWER");
    }

    /*
     * If user has selected a default Launcher, will only return the default
     * one.
     */
    static ArrayList<String> getLauncherList(Context context) {
        final Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);

        final List<ResolveInfo> resolveInfos = context.getPackageManager().queryIntentActivities(
                intent, 0);
        if (resolveInfos == null) {
            logd("No Launcher");
            return null;
        }
        final ArrayList<String> launcherList = new ArrayList<String>();
        ResolveInfo resolveInfo = context.getPackageManager().resolveActivity(intent, 0);
        if (resolveInfo != null && !"android".equals(resolveInfo.activityInfo.packageName))
            // has default launcher
            launcherList.add(resolveInfo.activityInfo.packageName);
        else {
            for (ResolveInfo info : resolveInfos)
                launcherList.add(info.activityInfo.packageName);
            if (launcherList.size() == 0)
                return null;
        }
        return launcherList;
    }

    private static String getLiveWallPaper(Context context) {
        WallpaperInfo wallpaperInfo = WallpaperManager.getInstance(context).getWallpaperInfo();
        if (wallpaperInfo != null)
            return wallpaperInfo.getComponent().getPackageName();

        return null;
    }

    private static String getCurrentInputMethod(Context context) {

        String currentInputMethod = Settings.Secure.getString(context.getContentResolver(),
                Settings.Secure.DEFAULT_INPUT_METHOD);
        currentInputMethod = UtilsStringOperation.getPackageNameFromClass(currentInputMethod);
        return currentInputMethod;

    }

    public static void killApplication(String packageName, int uid) {
        IActivityManager activityManager = ActivityManagerNative.getDefault();
        try {
            activityManager.killApplicationWithAppId(packageName, uid, "quickboot");
        } catch (RemoteException e) {
            logd(e);
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
