/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

public class IPerfConfig {
    private static int fileLogging; // Set the interval for logging to file

    private static int displayLogging; // Set the interval for logging to displa

    public static void setDisplayLogging(int displayLogging) {
        IPerfConfig.displayLogging = displayLogging;
    }

    public static int getDisplayLogging() {
        return displayLogging;
    }

    public static void setFileLogging(int fileLogging) {
        IPerfConfig.fileLogging = fileLogging;
    }

    public static int getFileLogging() {
        return fileLogging;
    }

}
