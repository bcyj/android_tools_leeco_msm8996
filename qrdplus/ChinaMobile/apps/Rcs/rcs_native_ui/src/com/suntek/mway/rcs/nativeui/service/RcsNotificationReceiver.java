/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class RcsNotificationReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        // Make sure the RcsNotificationsService is running.
        context.startService(new Intent(context, RcsNotificationsService.class));
    }
}
