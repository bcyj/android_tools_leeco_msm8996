/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.utils;

public class Values {
    public static final String TAG = "QuickBoot";

    public static final String KEY_INTENT_LAUNCH_MODE = "mode";

    public static final String KEY_QUICKBOOT_ENABLE = "sys.quickboot.enable";
    public static final String KEY_QUICKBOOT_POWERON = "sys.quickboot.poweron";
    public static final String KEY_QUICKBOOT_POWEROFF = "sys.quickboot.poweroff";
    public static final String KEY_QUICKBOOT_ONGOING = "persist.sys.quickboot_ongoing";

    public static final String KEY_USB_CONFIG = "persist.sys.usb.config";

    public static final String KEY_SP_AIRPLANE_MODE = "airplane_mode";
    public static final String KEY_SP_USB_CONFIG = "usb_config";

    public static final String INTENT_QUICKBOOT_START = "org.codeaurora.quickboot.poweron_start";
    public static final String INTENT_QUICKBOOT_APPKILLED = "org.codeaurora.quickboot.appkilled";

    public static final int SDK_VERSION = android.os.Build.VERSION.SDK_INT;
    public static final int JELLYBEAN = 18;
    public static final int KITKAT = 19;

    public static final int MODE_POWER_OFF = 0;
    public static final int MODE_POWER_ON = 1;
    public static final int MODE_RESTORE = 2;

    public static final String SYS_POWERON_ALARM = "/sys/module/qpnp_rtc/parameters/poweron_alarm";
    public static final String ALARM_ALERT_ACTION = "com.android.deskclock.ALARM_ALERT";

    /** Configs */
    public static final boolean DEBUG = false;
    public static final int POWERON_VIBRATE_TIME = 300;
    public static final int POWEROFF_VIBRATE_TIME = 300;


}
