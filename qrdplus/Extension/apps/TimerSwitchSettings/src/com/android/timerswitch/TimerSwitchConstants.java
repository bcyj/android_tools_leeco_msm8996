/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.android.timerswitch;

public class TimerSwitchConstants {

    public final static int SWITCH_ON = 1;
    public final static int SWITCH_OFF = 2;

    public final static String TIMER_SWITCH_KEY = "timer_switch";
    public static final String ACTION_POWER_OFF = "com.qualcomm.action.REQUEST_POWER_OFF";
    public static final String ACTION_POWER_ON = "com.qualcomm.action.REQUEST_POWER_ON";

    public static final long SCHEDULED_ADJUST_TIME = 2* 60 *1000;
    public static final long MILLIS_IN_MINUTE = 60 * 1000;

    public static final int DEFAULT_ON_HOUR = 7;
    public static final int DEFAULT_ON_MINUTES = 0;
    public static final int DEFAULT_ON_DAYOFWEEK = 127;

    public static final int DEFAULT_OFF_HOUR = 23;
    public static final int DEFAULT_OFF_MINUTES = 0;
    public static final int DEFAULT_OFF_DAYOFWEEK = 127;

    public static final String LOG_FILE_NAME = "timer_switch_log";

}
