/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.stk;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;

/**
 * This receiver enables and disables StkLauncherActivity component
 * of STKProxy app based on STK app's status. If STK app is enabled,
 * this component gets disabled and vice versa. When STK app is
 * not present in system, this app acts like a proxy STK app.
 */
public class StkEnableDisableReceiver extends BroadcastReceiver {
    private final String mStkPackageName = "com.android.stk";

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        if (Intent.ACTION_PACKAGE_CHANGED.equals(action)) {
            Uri dataUri = intent.getData();
            if (dataUri != null
                    && dataUri.getSchemeSpecificPart().equalsIgnoreCase(
                            mStkPackageName)) {
                // STK app state is changed. STK proxy app is
                // modified accordingly.
                enableDisableApp(context);
            }
        } else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            enableDisableApp(context);
        }

    }
    private void enableDisableApp(Context context) {
        PackageManager pm = context.getPackageManager();
        Intent stkResolverIntent = pm
                .getLaunchIntentForPackage(mStkPackageName);
        Intent stkProxyResolverIntent = pm
                .getLaunchIntentForPackage(context.getPackageName());
        // If SIM slots are active and STK Proxy app is enabled, STK proxy
        // app needs to be disabled.
        if ((stkResolverIntent != null)
                && (stkProxyResolverIntent != null)) {
            pm.setComponentEnabledSetting(
                    new ComponentName(
                            context.getPackageName(),
                            StkLauncherActivity.class.getName()),
                    PackageManager.COMPONENT_ENABLED_STATE_DISABLED, 1);
        } else if ((stkResolverIntent == null)
                && (stkProxyResolverIntent == null)) {
            // If SIM slots are not active and STK Proxy app is disabled,
            // STK proxy app needs to be enabled.
            pm.setComponentEnabledSetting(
                    new ComponentName(
                            context.getPackageName(),
                            StkLauncherActivity.class.getName()),
                    PackageManager.COMPONENT_ENABLED_STATE_ENABLED, 1);
        }
    }
}
