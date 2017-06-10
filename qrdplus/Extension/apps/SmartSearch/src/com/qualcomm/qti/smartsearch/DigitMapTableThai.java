/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Thai-characters to digits map.
 * </p>
 */
public class DigitMapTableThai extends DigitMapTableLatin {

    private static DigitMapTableThai sInstance = null;

    private static final char[][] EXTEND_THAI_TABLE = {
            // number 0
            {
                    0x0E50, 0x0E2F, 0x0E30, 0x0E32, 0x0E33, 0x0E40,
                    0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46,
                    0x0E4F, 0x0E5A, 0x0E5B, 0x0E31, 0x0E34, 0x0E35,
                    0x0E36, 0x0E37, 0x0E38, 0x0E39, 0x0E3A, 0x0E47,
                    0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D,
                    0x0E4E
            },
            // number 1
            {
                    0x0E51, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05
            },
            // number 2
            {
                    0x0E52, 0x0E06, 0x0E07, 0x0E08, 0x0E09
            },
            // number 3
            {
                    0x0E53, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D
            },
            // number 4
            {
                    0x0E54, 0x0E0E, 0x0E0F, 0x0E10, 0x0E11, 0x0E12,
                    0x0E13
            },
            // number 5
            {
                    0x0E55, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18
            },
            // number 6
            {
                    0x0E56, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D
            },
            // number 7
            {
                    0x0E57, 0x0E1E, 0x0E1F, 0x0E20, 0x0E21, 0x0E22
            },
            // number 8
            {
                    0x0E58, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
                    0x0E28, 0x0E29, 0x0E2A
            },
            // number 9
            {
                    0x0E59, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E
            }
    };

    public static DigitMapTableThai getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableThai();
        }
        return sInstance;
    }

    private DigitMapTableThai() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_THAI_TABLE[i], c)) {
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
