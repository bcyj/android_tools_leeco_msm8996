/******************************************************************************
 * @file    DeviceManagerAdminReceiver.java
 * @brief   Receiver to receive device manager admin events.
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 ******************************************************************************/
package com.qti.antitheftdemo;

import android.app.admin.DeviceAdminReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class DeviceManagerAdminReceiver extends DeviceAdminReceiver {

    @Override
        public void onEnabled(Context context, Intent intent) {
            super.onEnabled(context, intent);
        }

    @Override
        public void onDisabled(Context context, Intent intent) {
            super.onDisabled(context, intent);
        }
}
