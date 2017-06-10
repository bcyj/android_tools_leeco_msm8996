/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qrd.useragent;

import android.content.Context;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/* China Mobile's user agent format(compose of below items and separate by whitespace):
 **************************************************************************************
 * Phone mode/model version     (model version need configurated by customer)
 * OS/OS version
 * Platform/Platform version
 * Release/SW version
 * Browser/Browser version
 * Profile/Profile version      (need configurated by customer)
 * Configuration/Configuration  (need configurated by customer)
 * Other items/Other items property    (these items are optional)
 *******************************************************************************************************************************
 * eg: MSM8x25/1.0 Linux/2.6.35.7 Android/4.0 Rlease/06.30.2012 Browser/AppleWebKit533.1 Profile/MIDP-1.0 Configuration/CLDC-1.0
 *******************************************************************************************************************************
 */

public class UserAgentHandler {

    private static final String TAG = "UserAgentHandler";

    private final String BASE_USERAGENT = "%s/1.0 Android/%s Release/%s Browser/AppleWebKit534.30 Profile/MIDP-2.0 Configuration/CLDC-1.1";
    private static final String DEFAULT_LINUX_VERSION = "3.0.21";
    /* to improve performance, use a static string to record the user agent string
     * so there is no need to read file and property for UA. */
    private static String mUAString = null;

    public String getUAString(Context context) {


        //Build.MODEL means phone model
        //Build.VERSION_RELEASE means android version
        //Build.ID means sw version
        mUAString = String.format(BASE_USERAGENT, Build.MODEL, Build.VERSION.RELEASE, Build.ID);
        return mUAString;
    }
}
