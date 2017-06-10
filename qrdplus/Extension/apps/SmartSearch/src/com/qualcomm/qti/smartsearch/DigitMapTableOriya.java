/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Oriya-characters to digits map.
 * </p>
 */
public class DigitMapTableOriya extends DigitMapTableLatin{

    private static DigitMapTableOriya sInstance = null;

    private static final char[][] EXTEND_ORIYA_TABLE = {
        // number 0
        {
            0x0B66
        },
        // number 1
        {
            0x0B67,  0x0B01,  0x0B02,  0x0B03,  0x0B3E,  0x0B3F,
            0x0B40,  0x0B41,  0x0B42,  0x0B43,  0x0B44,  0x0B47,
            0x0B48,  0x0B4B,  0x0B4C,  0x0B4D,  0x0B56,  0x0B57,
            0x0B62,  0x0B63
        },
        // number 2
        {
            0x0B68,  0x0B05,  0x0B06,  0x0B07,  0x0B08,  0x0B09,
            0x0B0A
        },
        // number 3
        {
            0x0B69,  0x0B0B,  0x0B0C,  0x0B0F,  0x0B10,  0x0B13,
            0x0B14
        },
        // number 4
        {
            0x0B6A,  0x0B15,  0x0B16,  0x0B17,  0x0B18,  0x0B19,
            0x0B1A,  0x0B1B
        },
        // number 5
        {
            0x0B6B,  0x0B1C,  0x0B1D,  0x0B1E,  0x0B1F,  0x0B20,
            0x0B21,  0x0B22
        },
        // number 6
        {
            0x0B6C,  0x0B23,  0x0B24,  0x0B25,  0x0B26,  0x0B27
        },
        // number 7
        {
            0x0B6D,  0x0B28,  0x0B2A,  0x0B2B,  0x0B2C,  0x0B2D,
            0x0B2E
        },
        // number 8
        {
            0x0B6E,  0x0B2F,  0x0B30,  0x0B32,  0x0B33,  0x0B35,
            0x0B5C,  0x0B5D
        },
        // number 9
        {
            0x0B6F,  0x0B36,  0x0B37,  0x0B38,  0x0B39,  0x0B5F,
            0x0B60,  0x0B61,  0x0B71
        },
    };

    public static DigitMapTableOriya getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableOriya();
        }
        return sInstance;
    }

    private DigitMapTableOriya() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_ORIYA_TABLE[i], c)) {
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
