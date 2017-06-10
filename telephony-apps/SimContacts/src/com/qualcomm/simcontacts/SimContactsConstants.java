/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.simcontacts;

import android.telephony.TelephonyManager;

public class SimContactsConstants {

    public static final String ACCOUNT_NAME_SIM = "SIM";

    public static String getSimAccountName(int slotId) {
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            return ACCOUNT_NAME_SIM + (slotId + 1);
        } else {
            return ACCOUNT_NAME_SIM;
        }
    }

    public static final String ACCOUNT_NAME_PHONE = "PHONE";
    public static final String ACCOUNT_TYPE_SIM = "com.android.sim";
    public static final String ACCOUNT_TYPE_PHONE = "com.android.localphone";
    public static final String ACCOUNT_PASSWORD = "";
    public static final String EXTRA_ACCOUNT_NAME = "account_name";
    public static final String AUTHORITY = "com.android.contacts";
    public static final String STR_TAG = "tag";
    public static final String STR_NUMBER = "number";
    public static final String STR_EMAILS = "emails";
    public static final String STR_ANRS = "anrs";
    public static final String STR_NEW_TAG = "newTag";
    public static final String STR_NEW_NUMBER = "newNumber";
    public static final String STR_NEW_EMAILS = "newEmails";
    public static final String STR_NEW_ANRS = "newAnrs";
    public static final String STR_PIN2 = "pin2";
    public static final String USIM = "USIM";
    public static final int SIM_STATE_LOAD = 0;
    public static final int SIM_STATE_READY = 1;
    public static final int SIM_STATE_NOT_READY = 2;
    public static final int SIM_STATE_ERROR = 3;

}
