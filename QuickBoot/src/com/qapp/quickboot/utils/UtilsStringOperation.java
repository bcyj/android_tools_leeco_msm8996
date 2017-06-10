/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.utils;

public class UtilsStringOperation {

    public static String getPackageNameFromClass(String className) {

        String packageName = className;
        int splitIndex = className.indexOf("/");
        if (splitIndex >= 0) {
            packageName = className.substring(0, splitIndex);
        }
        return packageName;
    }
}
