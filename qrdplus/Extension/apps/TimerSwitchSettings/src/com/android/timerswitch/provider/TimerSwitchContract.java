/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.android.timerswitch.provider;

import android.net.Uri;
import android.provider.BaseColumns;

public final class TimerSwitchContract {
    public static final String AUTHORITY = "com.android.timerswitch";

    private TimerSwitchContract() {}

    protected interface SwitchesColumns extends BaseColumns {

        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/switches");

        public static final String HOUR = "hour";

        public static final String MINUTES = "minutes";

        public static final String DAYS_OF_WEEK = "daysofweek";

        public static final String SWITCH_TIME = "switchtime";

        public static final String ENABLED = "enabled";

    }
}
