/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import android.content.Context;

import com.qapp.secprotect.data.AuthDBManager;
import com.qapp.secprotect.data.AuthInfo;

public class UtilsDatabase {
    private static ScheduledExecutorService mScheduledExecutorService = Executors
            .newSingleThreadScheduledExecutor();

    private static class GrantRunnable implements Runnable {
        Context mContext;
        AuthInfo mAuthInfo;

        public GrantRunnable(final Context mContext, final AuthInfo mAuthInfo) {
            this.mContext = mContext;
            this.mAuthInfo = mAuthInfo;
        }

        @Override
        public void run() {
            AuthDBManager mDbManager = new AuthDBManager(mContext);
            mDbManager.openDataBase();
            mDbManager.update(mAuthInfo);
            mDbManager.closeDataBase();
        }
    };

    public static void setGrantMode(final Context mContext,
            final AuthInfo mAuthInfo) {

        mScheduledExecutorService
                .execute(new GrantRunnable(mContext, mAuthInfo));
    }
}
