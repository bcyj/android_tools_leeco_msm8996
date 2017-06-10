/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.quickboot.utils;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import android.util.Log;

public class UtilsFileOperation {

    public static boolean writeFile(String filePath, String content) {
        logd(filePath);
        boolean res = true;
        File file = new File(filePath);
        File dir = new File(file.getParent());
        if (!dir.exists())
            dir.mkdirs();
        try {
            FileWriter mFileWriter = new FileWriter(file, false);
            mFileWriter.write(content);
            mFileWriter.close();
        } catch (IOException e) {
            res = false;
        }
        return res;
    }

    public static String readFile(String filePath) {
        logd(filePath);
        String res = "";
        File file = new File(filePath);
        if (!file.exists())
            return res;

        try {
            char[] buf = new char[1024];
            int count = 0;
            FileReader fileReader = new FileReader(file);
            while ((count = fileReader.read(buf)) > 0) {
                res += new String(buf, 0, count);
            }
            fileReader.close();

        } catch (IOException e) {
            res = "";
        }
        return res;
    }

    private static void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(Values.TAG, s + "");
    }
}
