/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.appsettings;

import java.io.File;

import com.qapp.secprotect.data.AuthInfo;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.graphics.drawable.Drawable;

public class AppRecord {

    private final AppListLoader mLoader;
    private final ApplicationInfo mInfo;
    private final File mApkFile;

    private Drawable mIcon;
    private boolean mMounted;
    private String mLabel;
    private AuthInfo mAuthInfo;

    public AppRecord(AppListLoader loader, ApplicationInfo info) {
        mLoader = loader;
        mInfo = info;
        mApkFile = new File(info.sourceDir);
    }

    public AuthInfo getAuthInfo() {
        return mAuthInfo;
    }

    public void setAuthInfo(AuthInfo mAuthInfo) {
        this.mAuthInfo = mAuthInfo;
    }

    public ApplicationInfo getApplicationInfo() {
        return mInfo;
    }

    public String getLabel() {
        return mLabel;
    }

    public Drawable getIcon() {
        return mIcon;
    }

    @Override
    public String toString() {
        return mLabel;
    }

    void loadLabel(Context context) {
        if (mLabel == null || !mMounted) {
            if (!mApkFile.exists()) {
                mMounted = false;
                mLabel = mInfo.packageName;
            } else {
                mMounted = true;
                CharSequence label = mInfo.loadLabel(context
                        .getPackageManager());
                mLabel = label != null ? label.toString() : mInfo.packageName;
            }
        }
    }

    void loadIcon(Context context) {

        if (mIcon == null) {
            if (mApkFile.exists()) {
                mIcon = mInfo.loadIcon(mLoader.mPackageManager);
                return;
            } else {
                mMounted = false;
            }
        } else if (!mMounted) {

            if (mApkFile.exists()) {
                mMounted = true;
                mIcon = mInfo.loadIcon(mLoader.mPackageManager);
                return;
            }
        } else {
            return;
        }

        mIcon = context.getResources().getDrawable(
                android.R.drawable.sym_def_app_icon);
    }

}
