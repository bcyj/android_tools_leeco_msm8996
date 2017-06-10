/******************************************************************************
 * @file    PrimitiveParser.java
 * @brief   Contains parsing and explicit type-checking functions for the
 *          promoted datatypes needed to work with unsigned types in Java.
 *
 *
 * Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/

package com.qualcomm.qcrilhook;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class PrimitiveParser {

    // QMI Type Handling
    public static void checkByte(short val) throws NumberFormatException {
        if ((val & 0xFF00) != 0) {
            throw new NumberFormatException();
        }
        return;
    }

    public static void checkByte(int val) throws NumberFormatException {
        if ((val & 0xFFFFFF00) != 0) {
            throw new NumberFormatException();
        }
        return;
    }

    public static byte parseByte(short val) throws NumberFormatException {
        checkByte(val);
        return (byte) (val & 0x00FF);
    }

    public static byte parseByte(int val) throws NumberFormatException {
        checkByte(val);
        return (byte) (val & 0x00FF);
    }

    public static byte parseByte(char val) throws NumberFormatException {
        checkByte(val);
        return (byte) (val & 0x00FF);
    }

    public static void checkShort(int val) throws NumberFormatException {
        if ((val & 0xFFFF0000) != 0) {
            throw new NumberFormatException();
        }
        return;
    }

    public static short parseShort(int val) throws NumberFormatException {
        checkShort(val);
        return (short) (val & 0x0000FFFF);
    }

    public static void checkInt(long val) throws NumberFormatException {
        if ((val & 0xFFFFFFFF00000000L) != 0) {
            throw new NumberFormatException();
        }
        return;
    }

    public static int parseInt(long val) throws NumberFormatException {
        checkInt(val);
        return (int) (val & 0x00000000FFFFFFFFL);
    }

    public static long parseLong(String val) throws NumberFormatException {
        /*
         * BigInteger(String value) throws NumberFormat Exception if value is
         * not a valid representation of a BigInteger
         */
        ByteBuffer buf = ByteBuffer.wrap(new BigInteger(val).toByteArray());
        buf.order(ByteOrder.BIG_ENDIAN); // BigInteger returns big-endian
        // byte array
        return buf.getLong();
    }

    public static short toUnsigned(byte val) {
        return (short) (val & 0xFF);
    }

    public static int toUnsigned(short val) {
        return (int) (val & 0xFFFF);
    }

    public static long toUnsigned(int val) {
        return (long) (val & 0xFFFFFFFF);
    }

    // NV Type Parsing
    public static byte parseUnsignedByte(String in) throws NumberFormatException {
        short t = Short.parseShort(in);
        if ((t & 0xFF00) != 0) {
            throw new NumberFormatException();
        }
        return (byte) (t & 0x00FF);
    }

    public static short parseUnsignedShort(String in) throws NumberFormatException {
        int t = Integer.parseInt(in);
        if ((t & 0xFFFF0000) != 0) {
            throw new NumberFormatException();
        }
        return (short) (t & 0x0000FFFF);
    }

    public static int parseUnsignedInt(String in) throws NumberFormatException {
        long t = Long.parseLong(in);
        if ((t & 0xFFFFFFFF00000000L) != 0) {
            throw new NumberFormatException();
        }
        return (int) (t & 0x00000000FFFFFFFFL);
    }

    public static String toUnsignedString(byte in) {
        return String.valueOf(toUnsigned(in));
    }

    public static String toUnsignedString(short in) {
        return String.valueOf(toUnsigned(in));
    }

    public static String toUnsignedString(int in) {
        return String.valueOf(toUnsigned(in));
    }

}
