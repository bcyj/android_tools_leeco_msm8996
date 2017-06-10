/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All rights reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.cmcc.custom;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.System;
import android.util.Log;

public class CmccBootCompletedReceiver extends BroadcastReceiver {

    private static final String KEY_DEFAULT_IME = "persist.env.settings.ime";
    private static final String PACKAGE_PINYIN_IME = "com.android.inputmethod.pinyin";
    private static final String ID_PINYIN_IME = PACKAGE_PINYIN_IME + "/.PinyinIME";

    @Override
    public void onReceive(Context ctx, Intent intent) {
        String intentAction = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(intentAction)) {
            boolean hasFeature = SystemProperties.getBoolean(KEY_DEFAULT_IME, false);
            if (hasFeature) {
                PackageInfo packageInfo = null;
                try {
                    packageInfo = ctx.getPackageManager().getPackageInfo(
                            PACKAGE_PINYIN_IME, 0);
                    if (packageInfo != null) {
                        Settings.Secure.putString(ctx.getContentResolver(),
                                Settings.Secure.DEFAULT_INPUT_METHOD, ID_PINYIN_IME
                                );
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    SystemProperties.set(KEY_DEFAULT_IME, "false");
                }
            }
        }
    }
}
