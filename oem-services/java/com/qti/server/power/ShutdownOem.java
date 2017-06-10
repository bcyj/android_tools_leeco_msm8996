/******************************************************************************
  @file    ShutdownOem.java
  @brief   Qualcomm Shutdown specific code.

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.server.power;

import android.util.Log;
import java.lang.reflect.Method;
import dalvik.system.PathClassLoader;

public final class ShutdownOem {
    private static final String TAG = "QualcommShutdown";

    public void rebootOrShutdown(boolean reboot, String reason) {

        Log.i(TAG, "Qualcomm reboot/shutdown.");
        //Sub-system shutdown.
        if(SubSystemShutdown.shutdown() != 0) {
            Log.e(TAG, "Failed to shutdown modem.");
        } else
            Log.i(TAG, "Modem shutdown successful.");
    }
}
