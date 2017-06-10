/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.util;

import java.security.MessageDigest;

import android.text.TextUtils;

public class MD5Util {

    private final static String[] hexDigits = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", //
            "a", "b", "c", "d", "e", "f" };

    public static String encode_32(String origin) {
        if (TextUtils.isEmpty(origin))
            return "";
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            String result = bytesToHexString(md.digest(origin.getBytes()));
            return result;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    public static String encode_16(String origin) {
        try {
            return encode_32(origin).substring(8, 24);
        } catch (Exception e1) {
            return "";
        }
    }

    public static String bytesToHexString(byte[] b) {
        StringBuffer resultSb = new StringBuffer();
        for (int i = 0; i < b.length; i++) {
            resultSb.append(byteToHexString(b[i]));
        }
        return resultSb.toString();
    }

    private static String byteToHexString(byte b) {
        int n = b;
        if (n < 0)
            n = 256 + n;
        int d1 = n / 16;
        int d2 = n % 16;
        return hexDigits[d1] + hexDigits[d2];
    }
}
