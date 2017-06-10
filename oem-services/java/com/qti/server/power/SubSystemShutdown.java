/******************************************************************************
  @file    SubSystemShutdown.java
  @brief   Qualcomm Shutdown specific code.

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.server.power;

public final class SubSystemShutdown {
    private static final String TAG = "SubSystemShutdown";

    public static native int shutdown ();

    static {
        System.loadLibrary("SubSystemShutdown");
    }
}
