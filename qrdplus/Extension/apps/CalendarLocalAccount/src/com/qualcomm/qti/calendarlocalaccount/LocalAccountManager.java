/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarlocalaccount;

import android.accounts.AccountManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class LocalAccountManager extends BroadcastReceiver {
    private static final String TAG = "LocalAccountManager";

    private static final String CALENDAR_PROVIDER = "com.android.providers.calendar";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (context.getResources().getBoolean(R.bool.enabled)) {
            String action = intent.getAction();
            Log.i(TAG, "Receive the action: " + action);

            AccountManager am = AccountManager.get(context);
            if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
                am.addAccount(Authenticator.ACCOUNT_TYPE, null, null, null, null, null, null);
            } else if (Intent.ACTION_PACKAGE_DATA_CLEARED.equals(action)) {
                String packageName = intent.getData().getSchemeSpecificPart();
                if (CALENDAR_PROVIDER.equals(packageName)) {
                    Log.d(TAG, "The calendar provider data cleared, need add the account again.");
                    am.addAccount(Authenticator.ACCOUNT_TYPE, null, null, null, null, null, null);
                }
            } else if (Intent.ACTION_LOCALE_CHANGED.equals(action)) {
                am.editProperties(Authenticator.ACCOUNT_TYPE, null, null, null);
            }
        }
    }

}
