/*=========================================================================
  WipowerBatteryControl.java
  DESCRIPTION
  Wipower Battery Control HAL abstraction

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wbcservice;

import android.util.Log;
import android.os.SystemProperties;

public class WipowerBatteryControl {

    private WbcEventListener mListener;
    private static final String TAG = "Wbc-J";
    private static boolean sDbg = false;

    public static final int WBC_STATUS_OK = 0;
    public static final int WBC_STATUS_ERROR = 1;

    /* Begin - Should be in-sync with HAL */
    public static final int WBC_EVENT_TYPE_WIPOWER_CAPABLE_STATUS = 1;
    public static final int WBC_EVENT_TYPE_PTU_PRESENCE_STATUS  = 2;
    public static final int WBC_EVENT_TYPE_WIPOWER_CHARGING_ACTIVE_STATUS = 3;
    public static final int WBC_EVENT_TYPE_CHARGING_REQUIRED_STATUS = 4;

    public static final int WBC_WIPOWER_INCAPABLE = 0;
    public static final int WBC_WIPOWER_CAPABLE = 1;

    public static final int WBC_PTU_STATUS_NOT_PRESENT = 0;
    public static final int WBC_PTU_STATUS_PRESENT = 1;

    public static final int WBC_WIPOWER_STATUS_NOT_CHARGING = 0;
    public static final int WBC_WIPOWER_STATUS_CHARGING_ACTIVE = 1;

    public static final int WBC_BATTERY_STATUS_CHARGING_NOT_REQUIRED = 0;
    public static final int WBC_BATTERY_STATUS_CHARGING_REQUIRED = 1;
    /* End - Should be in-sync with HAL */

    public interface WbcEventListener {
        void onWbcEventUpdate(int what, int arg1, int arg2);
    }

    public WipowerBatteryControl(WbcEventListener listener) {
        this.init();
        mListener = listener;
    }

    private void sendEventFromNative(int what, int arg1, int arg2) {
        if (sDbg) Log.i(TAG, "Rcvd event from native: " + what + ", " + arg1 + ", " + arg2);

        if (mListener != null) {
            mListener.onWbcEventUpdate(what, arg1, arg2);
        } else {
            Log.w(TAG, "No listener registered!");
        }
    }

    private native void init();
    private native void finish();

    public native void echo(int value);
    public native int getWipowerCapable();
    public native int getPtuPresence();
    public native int getWipowerCharging();
    public native int getChargingRequired();

    static {
        String debug = SystemProperties.get("persist.wbc.log_level", "0");
        sDbg = debug.equals("0") ? false : true;
        System.loadLibrary("wbc_jni");
    }
}
