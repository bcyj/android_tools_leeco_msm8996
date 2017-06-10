/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import static com.qapp.secprotect.Configs.INTERNAL_STORAGE;
import static com.qapp.secprotect.Configs.INTERNAL_STORAGE_REAL;
import static com.qapp.secprotect.Configs.SDCARD_REAL;
import static com.qapp.secprotect.Configs.SDCARD_ROOT;
import static com.qapp.secprotect.utils.UtilsLog.logd;

public class UtilsStrings {

    public static String getFileType(String fileName) {
        if (fileName == null)
            return "";
        int index = fileName.lastIndexOf(".");
        if (index < 0)
            return "";
        String ext = fileName.substring(index, fileName.length()).toLowerCase();
        return ext;
    }

    public static String getFileNameFromPath(String filePath) {
        String ret = "";
        int index = filePath.lastIndexOf("/");
        if (index < 0 || index >= filePath.length() - 1)
            return ret;
        ret = filePath.substring(index + 1);
        return ret;
    }

    public static String getFileNameWithoutPostfix(String filePath) {

        if (filePath == null)
            return "";
        int index = filePath.lastIndexOf(".");
        logd(index);
        if (index < 0 || index >= filePath.length() - 1)
            return filePath;
        return filePath.substring(0, index);
    }

    public static String getProtectedFilePath(String srcFilePath) {

        if (srcFilePath == null)
            return null;
        String dstFilePath = null;
        if (srcFilePath.startsWith(INTERNAL_STORAGE_REAL)) {
            dstFilePath = INTERNAL_STORAGE
                    + srcFilePath.substring(INTERNAL_STORAGE_REAL.length());
        } else if (srcFilePath.startsWith(SDCARD_REAL)) {
            dstFilePath = SDCARD_ROOT
                    + srcFilePath.substring(SDCARD_REAL.length());
        } else {
            return srcFilePath;
        }
        return dstFilePath;
    }
}
