/******************************************************************************
 * @file    PrivilegedCBWidgetProvider.java
 * @brief   Implementation of privileged Cell Broadcast receiver for Brazil
 *
 *
 * ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qti.cbwidget;

import android.content.Context;
import android.content.Intent;

public class PrivilegedCBWidgetProvider extends CBWidgetProvider {
    @Override
    public void onReceive(Context context, Intent intent) {
        // Pass the message to the base class implementation, noting that it
        // was permission-checked on the way in.
        onReceiveWithPrivilege(context, intent, true);
    }
}
