/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.utils;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.List;

public class UtilsConvert {

    public static String[] getStringArrayFromList(List<String> inFileList) {
        if (inFileList.size() < 1)
            return null;
        String[] inFiles = new String[inFileList.size()];
        for (int i = 0; i < inFileList.size(); i++) {
            inFiles[i] = inFileList.get(i);
        }
        return inFiles;
    }

    public static String getSizeString(long size) {
        if (size < 1024) {
            return String.valueOf(size) + "B";
        } else {
            size = size / 1024;
        }
        if (size < 1024) {
            return String.valueOf(size) + "KB";
        } else {
            size = size * 100 / 1024;
        }
        return String.valueOf((size / 100)) + "."
                + ((size % 100) < 10 ? "0" : "") + String.valueOf((size % 100))
                + "MB";
    }

    public static String byteToString(byte[] bytes) {
        int length = 0;
        for (int i = 0; i < bytes.length; i++) {
            if (bytes[i] == '\0') {
                length = i;
                break;
            }
        }
        length = length < 0 ? 0 : length;
        return new String(bytes, 0, length);
    }

    public static byte[] ObjectToByte(java.lang.Object obj) {
        byte[] bytes = null;
        try {
            // object to bytearray
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            ObjectOutputStream objectOutputStream = new ObjectOutputStream(
                    byteArrayOutputStream);
            objectOutputStream.writeObject(obj);

            bytes = byteArrayOutputStream.toByteArray();

            byteArrayOutputStream.close();
            objectOutputStream.close();
        } catch (Exception e) {
            System.out.println("translation " + e.getMessage());
            e.printStackTrace();
        }
        return bytes;
    }

    public static Object ByteToObject(byte[] bytes) {

        if (bytes == null)
            return null;
        Object obj = null;
        try {
            // bytearray to object
            ByteArrayInputStream bi = new ByteArrayInputStream(bytes);
            ObjectInputStream oi = new ObjectInputStream(bi);

            obj = oi.readObject();
            bi.close();
            oi.close();
        } catch (Exception e) {
            System.out.println("translation " + e.getMessage());
            e.printStackTrace();
        }
        return obj;
    }

    public static byte[] subBytes(byte[] src, int begin, int count) {
        byte[] data = new byte[count];
        for (int i = begin; i < begin + count; i++)
            data[i - begin] = src[i];
        return data;
    }

    public static byte[] combineBytes(byte[]... inputs) {
        int size = 0;
        for (int i = 0; i < inputs.length; i++) {
            size += inputs[i].length;
        }
        byte[] output = new byte[size];
        int lastPos = 0;
        for (int i = 0; i < inputs.length; i++) {
            System.arraycopy(inputs[i], 0, output, lastPos, inputs[i].length);
            lastPos += inputs[i].length;
        }

        return output;
    }

    public static byte[] hexToByte(String hexString) {
        int len = hexString.length() / 2;
        byte[] result = new byte[len];
        for (int i = 0; i < len; i++)
            result[i] = Integer.valueOf(hexString.substring(2 * i, 2 * i + 2),
                    16).byteValue();
        return result;
    }

    public static String byteToHex(byte[] buf) {
        final String HEX = "0123456789ABCDEF";
        if (buf == null)
            return "";
        StringBuffer result = new StringBuffer(2 * buf.length);
        for (int i = 0; i < buf.length; i++) {
            result.append(HEX.charAt((buf[i] >> 4) & 0x0f)).append(
                    HEX.charAt(buf[i] & 0x0f));
        }
        return result.toString();
    }
}
