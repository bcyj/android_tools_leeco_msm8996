/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.appsettings;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;
import android.os.Process;
import android.support.v4.content.AsyncTaskLoader;

import com.qapp.secprotect.data.AuthDBManager;
import com.qapp.secprotect.utils.UtilsLog;

public class AppListLoader extends AsyncTaskLoader<List<AppRecord>> {
    final PackageManager mPackageManager;

    List<AppRecord> mApps;

    public AppListLoader(Context context) {
        super(context);
        mPackageManager = getContext().getPackageManager();
    }

    AuthDBManager mAuthDBManager;

    @Override
    public List<AppRecord> loadInBackground() {

        mAuthDBManager = new AuthDBManager(getContext());
        mAuthDBManager.openDataBase();
        Cursor cursor = mAuthDBManager.queryAll();
        final Context context = getContext();
        PackageManager packageManager = context.getPackageManager();
        int uidIndex = cursor.getColumnIndex(AuthDBManager.UID);
        int packageNameIndex = cursor
                .getColumnIndex(AuthDBManager.PACKAGE_NAME);

        UtilsLog.logd("app num=" + cursor.getCount());
        List<AppRecord> appList = new ArrayList<AppRecord>();
        for (cursor.moveToFirst(); !(cursor.isAfterLast()); cursor.moveToNext()) {
            int uid = cursor.getInt(uidIndex);
            if (uid < Process.FIRST_APPLICATION_UID) {
                continue;
            }
            String packageName = cursor.getString(packageNameIndex);

            ApplicationInfo applicationInfo = null;
            try {
                applicationInfo = packageManager.getApplicationInfo(
                        packageName, 0);
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }
            if (applicationInfo == null) {
                continue;
            }
            AppRecord entry = new AppRecord(this, applicationInfo);
            entry.loadLabel(context);
            entry.loadIcon(context);
            entry.setAuthInfo(AuthDBManager.getAuthInfoByCursor(cursor));
            appList.add(entry);
        }
        cursor.close();
        mAuthDBManager.closeDataBase();

        Collections.sort(appList, ALPHA_COMPARATOR);
        return appList;
    }

    public static final Comparator<AppRecord> ALPHA_COMPARATOR = new Comparator<AppRecord>() {
        private final Collator sCollator = Collator.getInstance();

        @Override
        public int compare(AppRecord object1, AppRecord object2) {
            return sCollator.compare(object1.getLabel(), object2.getLabel());
        }
    };

    @Override
    public void deliverResult(List<AppRecord> apps) {
        if (isReset()) {

            if (apps != null) {
                onReleaseResources(apps);
            }
        }
        List<AppRecord> oldApps = apps;
        mApps = apps;
        if (isStarted()) {
            super.deliverResult(apps);
        }

        if (oldApps != null) {
            onReleaseResources(oldApps);
        }
    }

    @Override
    protected void onStartLoading() {
        if (mApps != null) {
            deliverResult(mApps);
        }

        if (takeContentChanged() || mApps == null) {
            forceLoad();
        }
    }

    @Override
    protected void onStopLoading() {
        cancelLoad();
    }

    @Override
    public void onCanceled(List<AppRecord> apps) {
        super.onCanceled(apps);
        onReleaseResources(apps);
    }

    @Override
    protected void onReset() {
        super.onReset();

        onStopLoading();

        if (mApps != null) {
            onReleaseResources(mApps);
            mApps = null;
        }

    }

    protected void onReleaseResources(List<AppRecord> apps) {

    }
}
