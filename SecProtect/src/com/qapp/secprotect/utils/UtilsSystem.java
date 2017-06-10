/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.os.Process;

public class UtilsSystem {

    public static boolean isMonkeyRunning() {
        return ActivityManager.isUserAMonkey();
    }

    public static String getPackageNameByPid(Context context, int uid) {

        String packageName = "unknown";

        if (uid < Process.FIRST_APPLICATION_UID) {
            packageName = "uid:" + uid;
            return packageName;
        }
        PackageManager packageManager = context.getPackageManager();
        String packages[] = packageManager.getPackagesForUid(uid);
        if (packages != null) {
            if (packages.length == 1) {
                packageName = packages[0];
            } else if (packages.length > 1) {
                packageName = "uid:" + uid;
            }
        }

        // ActivityManager activityManager = (ActivityManager) context
        // .getSystemService(Context.ACTIVITY_SERVICE);
        // List<ActivityManager.RunningAppProcessInfo> runningList =
        // activityManager
        // .getRunningAppProcesses();
        // for (ActivityManager.RunningAppProcessInfo processInfo : runningList)
        // {
        // logd(processInfo.processName + " processPid="+processInfo.pid
        // +" pid="+pid);
        // if (processInfo.pid == pid) {
        // String processName = processInfo.processName;
        // for (String p : packages) {
        // if (processName.startsWith(p)) {
        // return p;
        // }
        // }
        // }
        // }
        return packageName;
    }

    public static String getAppName(Context context, int uid, String packageName) {
        String displayName = "unknown";
        if (uid < Process.FIRST_APPLICATION_UID) {
            displayName = "uid:" + uid;
            return displayName;
        }
        PackageManager packageManager = context.getPackageManager();
        ApplicationInfo applicationInfo = null;
        try {
            applicationInfo = packageManager.getApplicationInfo(packageName, 0);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        if (applicationInfo != null) {
            displayName = (String) packageManager.getApplicationLabel(applicationInfo);
        }
        return displayName;
    }

    public static Drawable getAppIcon(Context context, int uid, String packageName) {
        
        if (uid >= Process.FIRST_APPLICATION_UID) {
            try {
                Drawable appIcon = context.getPackageManager().getApplicationIcon(packageName);
                return appIcon;
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }
        }
        return context.getResources().getDrawable(android.R.drawable.sym_def_app_icon);
    }

    public static boolean isImageFile(String path) {
        MediaFile.MediaFileType mediaFileType = MediaFile.getFileType(path);
        int fileType = (mediaFileType == null ? 0 : mediaFileType.fileType);
        return MediaFile.isImageFileType(fileType);
    }
}
