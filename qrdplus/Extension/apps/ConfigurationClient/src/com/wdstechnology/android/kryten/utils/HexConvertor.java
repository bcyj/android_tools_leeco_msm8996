/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.wdstechnology.android.kryten.utils;

import java.util.Vector;

public class HexConvertor {

    public static byte[] convert(String input) {
        StringBuffer convertMe = new StringBuffer(input);
        Vector<Integer> bytes = new Vector<Integer>();

        while (convertMe.length() > 0) {
            String octet;
            int endPos = convertMe.length();
            if (convertMe.length() == 1) {
                octet = convertMe.toString();
                convertMe.delete(0, endPos);
            } else {
                int startPos = convertMe.length() - 2;
                octet = convertMe.substring(startPos);
                convertMe.delete(startPos, endPos);
            }

            int value = Integer.parseInt(octet, 16);
            bytes.add(new Integer(value));
        }

        int byteCount = bytes.size();
        byte[] result = new byte[byteCount];

        for (int i = 0; i < byteCount; i++) {
            Integer element = (Integer) bytes.get(i);
            result[byteCount - i - 1] = element.byteValue();
        }

        return result;
    }

    public static String convert(byte[] input) {
        StringBuffer buffer = new StringBuffer();

        for (int i = 0; i < input.length; i++) {
            int value = input[i];
            value = value & 0xFF;

            String hexDigits = Integer.toHexString(value);
            if (hexDigits.length() == 1) {
                hexDigits = "0" + hexDigits;
            }
            buffer.append(hexDigits);
        }

        return buffer.toString().toUpperCase();
    }

    public static byte convertJavaSyntaxByte(String value) {
        if (!value.substring(0, 2).equals("0x")) {
            throw new RuntimeException("Java syntax bytes must start with 0x");
        } else {
            String byteAsString = value.replaceAll("0x", "");
            return convert(byteAsString)[0];
        }
    }

}
