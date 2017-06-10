/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the French-characters to digits map.
 * </p>
 */
public class DigitMapTableFrench extends DigitMapTableLatin {

    private static DigitMapTableFrench sInstance = null;

    private static final char[][] EXTEND_FRENCH_TABLE = {
        // number 0
        {

        },
        // number 1
        {

        },
        // number 2
        {
            0x0041, 0x0042, 0x0043, 0x0061, 0x0062, 0x0063,
            0x00C0, 0x00E0, 0x00C2, 0x00E2, 0x00C7, 0x00E7
        },
        // number 3
        {
            0x0044, 0x0045, 0x0046, 0x0064, 0x0065, 0x0066,
            0x00C8, 0x00E8, 0x00C9, 0x00E9, 0x00CA, 0x00EA,
            0x00CB, 0x00EB
        },
        // number 4
        {
            0x0047, 0x0048, 0x0049, 0x0067, 0x0068, 0x0069,
            0x00CE, 0x00EE, 0x00CF, 0x00EF
        },
        // number 5
        {
            0x004A, 0x004B, 0x004C, 0x006A, 0x006B, 0x006C
        },
        // number 6
        {
            0x004D, 0x004E, 0x004F, 0x006D, 0x006E, 0x006F,
            0x00D4, 0x00F4
        },
        // number 7
        {
            0x0050, 0x0051, 0x0052, 0x0053, 0x0070, 0x0071,
            0x0072, 0x0073
        },
        // number 8
        {
            0x0054, 0x0055, 0x0056, 0x0074, 0x0075, 0x0076,
            0x00D9, 0x00F9, 0x00DB, 0x00FB, 0x00DC, 0x00FC
        },
        // number 9
        {
            0x0057, 0x0058, 0x0059, 0x005A, 0x0077, 0x0078,
            0x0079, 0x007A, 0x0178, 0x00FF
        }
    };

    public static DigitMapTableFrench getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableFrench();
        }
        return sInstance;
    }

    private DigitMapTableFrench() {

    }

    @Override
    public String toDigits(char c) {
        // TODO Auto-generated method stub
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_FRENCH_TABLE[i], c)) {
                idx = i;
                break;
            }
        }
        if (idx == -1) {
            return super.toDigits(c);
        } else {
            return Integer.toString(idx);
        }
    }
}
