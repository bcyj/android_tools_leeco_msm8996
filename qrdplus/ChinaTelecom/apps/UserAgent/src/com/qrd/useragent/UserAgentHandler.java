/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qrd.useragent;

import android.content.Context;
import android.os.Build;
import android.util.DisplayMetrics;

import java.util.Locale;

/* China Telcom's User Agent String format for android phone:
 *******************************************************************************************************************************
 * Mozilla/5.0 (Linux;U;Android_Version;zh-cn; Model/SW Version; Screen Wide*Screen Height;CTC/2.0) Browser Name/Browser Version
 * Mozilla/5.0: can not be changed
 * Version: android version
 * zh-cn: language
 * Model: compose of manufacturer name and phone model, use '-' to link them
 * SW Version: software version
 * U: strong safety requirements, this value can't be changed
 * Screen Wide*Screen Height: the screen resolution of width and height, use '*' to delimite
 * CTC/2.0: compliance with requirements of CT full function browser(the browser that only support WAP is CTC/1.0, if unsupport
 *          WAP, can't include this field
 * Browser Name/Version: browser name and version
 * '-': indicate whitespace
 *******************************************************************************************************************************
 */

public class UserAgentHandler {

    private final String BASE_USERAGENT = "Mozilla/5.0 (Linux; U; Android %s) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30";

    /* to improve performance, use a static string to record the user agent string
     * so there is no need to read file and property every time for UA. */
    private static String mUAString = null;

    public String getUAString(Context context) {

        //if the mUAString is not null, return it
        if (mUAString != null)
            return mUAString;

        StringBuffer usBuffer = new StringBuffer();

        //Add android version
        usBuffer.append(Build.VERSION.RELEASE + "; ");

        //Add lanauage
        final String language = Locale.getDefault().getLanguage();
        if (language != null) {
            usBuffer.append(language);
            final String country = Locale.getDefault().getCountry();
            if (country != null) {
                usBuffer.append("-");
                usBuffer.append(country.toLowerCase());
            }
        } else {
            //default to "en"
            usBuffer.append("en");
        }
        usBuffer.append("; ");

        //Add the model
        usBuffer.append(Build.MODEL + " Build/");

        //Add software version
        usBuffer.append(Build.ID);

        //Add screen sizei
        /* if (context != null) {
            DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
            if (displayMetrics != null)
                usBuffer.append(displayMetrics.widthPixels + "*" + displayMetrics.heightPixels);
        }*/

        //Add CTC version, if the browser do not support wap, do not add this field

        mUAString = String.format(BASE_USERAGENT, usBuffer);
        return mUAString;
    }
}
