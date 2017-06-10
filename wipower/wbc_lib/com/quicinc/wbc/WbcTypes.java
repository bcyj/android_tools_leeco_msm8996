/*=========================================================================
  WbcTypes.java
  DESCRIPTION
  WBC Types

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wbc;

public class WbcTypes {

    /* Begin - Should be in-sync with HAL */
    private static final int WBC_EVENT_TYPE_WIPOWER_CAPABLE_STATUS = 1;
    private static final int WBC_EVENT_TYPE_PTU_PRESENCE_STATUS  = 2;
    private static final int WBC_EVENT_TYPE_WIPOWER_CHARGING_ACTIVE_STATUS = 3;
    /**
     * Charging required status
     */
    public static final int WBC_EVENT_TYPE_CHARGING_REQUIRED_STATUS = 4;

    private static final int WBC_WIPOWER_INCAPABLE = 0;
    private static final int WBC_WIPOWER_CAPABLE = 1;

    private static final int WBC_PTU_STATUS_NOT_PRESENT = 0;
    private static final int WBC_PTU_STATUS_PRESENT = 1;

    private static final int WBC_WIPOWER_STATUS_NOT_CHARGING = 0;
    private static final int WBC_WIPOWER_STATUS_CHARGING_ACTIVE = 1;

    public static final int WBC_BATTERY_STATUS_CHARGING_NOT_REQUIRED = 0;
    public static final int WBC_BATTERY_STATUS_CHARGING_REQUIRED = 1;
    /* End - Should be in-sync with HAL */

    public final static String ACTION_WIPOWER_ICON_ENABLE =
            "com.quicinc.wbc.action.WIPOWER_ICON_ENABLE";

    public final static String ACTION_WIPOWER_ICON_DISABLE =
            "com.quicinc.wbc.action.WIPOWER_ICON_DISABLE";

    public final static String ACTION_SHOW_BLUETOOTH_NEEDED_UI_DIALOG =
            "com.quicinc.wbc.action.SHOW_BLUETOOTH_NEEDED_UI_DIALOG";
}
