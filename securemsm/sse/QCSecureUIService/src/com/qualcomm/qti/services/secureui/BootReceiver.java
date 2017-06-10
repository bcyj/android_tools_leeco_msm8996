/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Intent receiver for the following:
 * ACTION_BOOT_COMPLETED
 * */
public class BootReceiver extends BroadcastReceiver {

    /*
     * @see android.content.BroadcastReceiver#onReceive(android.content.Context, android.content.Intent)
     * At completion of boot, starts the Secure UI Service.
     */
    @Override
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      if (action == null) {
        return;
      }
      if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
        if (SecureUIService.context == null) {
          context.startService(new Intent(context, SecureUIService.class));
        }
      }
    }
}
