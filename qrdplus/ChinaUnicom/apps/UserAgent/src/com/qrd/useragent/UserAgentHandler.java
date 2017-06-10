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

/* China Unicom's user agent format(compose of below items and separate by whitespace):
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

    /* the BASE_USERAGENT is formated according to carrier's requirement. Some need to get from system
     * the first %s:  phone model
     * the second %s: linux version
     * the third %s:  android version
     * the fourth %s: software version
     * some other may be needed to changed by customer,eg: profile version, configuration verison, etc
     */
    private final String BASE_USERAGENT = "%s/1.0 Linux/%s Android/%s Release/%s Browser/AppWebKit534.30 Profile/MIDP-1.0 Configuration/CLDC-1.0 Mobile";
    private static final String DEFAULT_LINUX_VERSION = "3.0.21";

    /* to improve performance, use a static string to record the user agent string
     * so there is no need to read file and property for UA. */
    private static String mUAString = null;

    public String getUAString(Context context) {

        //for CU format UA, there are some issue. So CU allow us user android default UA
        //here I return null for this function, in browser, it will use android UA.
        return null;

        //if the mUAString is not null, return it
        /*if (mUAString != null)
            return mUAString;

        //get the linux kernel version
        String kernelVersion = null;
        String linuxVersion = null;
        try {
            //read content from version file
            BufferedReader reader = new BufferedReader(new FileReader("/proc/version"), 256);
            try {
                kernelVersion = reader.readLine();
            } finally {
                reader.close();
            }

            String keyWord = "version ";
            int index = kernelVersion.indexOf(keyWord);
            linuxVersion = kernelVersion.substring(index + keyWord.length());
            if (!TextUtils.isEmpty(linuxVersion)) {
                //generally, the subStr's format is 3.0.21-perf+, but we only need number parts
                index = linuxVersion.indexOf("-");
                if (index > 0) {
                    linuxVersion = linuxVersion.substring(0, index);
                }
                Log.d(TAG, "the linuxVersion is "+linuxVersion);
            }
        } catch (IOException e) {
            Log.e(TAG, "IO Exception when getting kernel version for user agent string");
            //give it a default value
            linuxVersion = DEFAULT_LINUX_VERSION;
        }

        if (TextUtils.isEmpty(linuxVersion))
            linuxVersion = DEFAULT_LINUX_VERSION;

        //Build.MODEL means phone model
        //Build.VERSION_RELEASE means android version
        // Build.ID means sw version
        mUAString = String.format(BASE_USERAGENT, Build.MODEL, linuxVersion, Build.VERSION.RELEASE, Build.ID);
        return mUAString;*/
    }
}
