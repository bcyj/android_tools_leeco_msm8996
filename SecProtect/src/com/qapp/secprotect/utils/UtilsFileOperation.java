/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class UtilsFileOperation {

    public static boolean writeFile(String filePath, String content) {
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

    public static boolean writeFile(String filePath, byte[] content) {
        boolean res = true;
        File file = new File(filePath);
        File dir = new File(file.getParent());
        if (!dir.exists())
            dir.mkdirs();
        try {
            FileOutputStream mFileWriter = new FileOutputStream(file, false);
            mFileWriter.write(content);
            mFileWriter.close();
        } catch (IOException e) {
            res = false;
        }
        return res;
    }

    public static String readFile(String filePath) {
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

    public static byte[] readFileBytes(String filePath) {
        byte[] res = null;
        File file = new File(filePath);
        if (!file.canRead())
            return res;

        try {
            byte[] buf = new byte[1024];
            int count = 0;
            FileInputStream fileInputStream = new FileInputStream(file);
            while ((count = fileInputStream.read(buf)) > 0) {

                byte[] tmp = UtilsConvert.subBytes(buf, 0, count);
                if (res == null)
                    res = tmp;
                else
                    res = UtilsConvert.combineBytes(res, tmp);
            }
            fileInputStream.close();

        } catch (IOException e) {
            res = null;
        }
        return res;
    }

    public static String findLineFromFile(String filePath, String lineHead) {

        String objectLine = null;
        if (lineHead == null)
            return null;
        File file = new File(filePath);
        if (!file.exists())
            return objectLine;
        try {
            String tmp;
            FileReader fileReader = new FileReader(file);
            BufferedReader bufferedReader = new BufferedReader(fileReader);
            while ((tmp = bufferedReader.readLine()) != null) {
                if (tmp.startsWith(lineHead)) {
                    objectLine = tmp.substring(lineHead.length());
                    break;
                }
            }

            bufferedReader.close();
            fileReader.close();

        } catch (IOException e) {
            objectLine = null;
        }
        return objectLine;
    }

    public static void removeFile(File file) {
        if (file.exists()) {
            if (file.isDirectory()) {
                File[] childFiles = file.listFiles();
                for (int i = 0; i < childFiles.length; i++) {
                    removeFile(childFiles[i].getPath());
                }
            }
            // if (!file.isDirectory())
            file.delete();
        }
    }

    public static void removeFile(String filePath) {
        removeFile(new File(filePath));
    }

    public static void removeFile(String[] filePaths) {
        for (int i = 0; i < filePaths.length; i++)
            removeFile(filePaths[i]);
    }

    public static boolean renameFile(String srcFile, String dstFile) {
        File file = new File(srcFile);
        if (!file.exists()) {
            return false;
        }
        File dst = new File(dstFile);
        return file.renameTo(dst);
    }

    public static boolean renameFile(File srcFile, File dstFile) {
        if (!srcFile.exists()) {
            return false;
        }
        return srcFile.renameTo(dstFile);
    }

    public static boolean makeDirs(String path) {
        boolean ret = false;
        File dir = new File(path);
        if (!dir.exists())
            ret = dir.mkdirs();
        return ret;
    }

    public static boolean copyFile(File srcFile, File dstFile) {
        if (!srcFile.exists())
            return false;
        try {
            InputStream inputStream = new FileInputStream(srcFile);
            if (dstFile.exists()) {
                dstFile.delete();
            }

            OutputStream outputStream = new FileOutputStream(dstFile);
            try {
                int cnt;
                byte[] buf = new byte[4096];
                while ((cnt = inputStream.read(buf)) >= 0) {
                    outputStream.write(buf, 0, cnt);
                }
            } finally {
                outputStream.close();
                inputStream.close();
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public static boolean copyFile(String srcFile, String dstFile) {
        return copyFile(new File(srcFile), new File(dstFile));
    }

    public static boolean copyFile(File srcFile, String dstFile) {
        return copyFile(srcFile, new File(dstFile));
    }
}
