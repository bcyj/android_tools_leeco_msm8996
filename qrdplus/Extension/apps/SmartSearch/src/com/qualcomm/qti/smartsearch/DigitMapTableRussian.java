/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Russian-characters to digits map.
 * </p>
 */
public class DigitMapTableRussian extends DigitMapTableLatin {

    private static DigitMapTableRussian sInstance = null;

    private static final char[][] RUSSIAN_TABLE = {
            // number 0
            {
            },
            // number 1
            {
            },
            // number 2
            {
                    0x0410, 0x0411, 0x0412, 0x0413, 0x0430, 0x0431,
                    0x0432, 0x0433
            },
            // number 3
            {
                    0x0414, 0x0415, 0x0401, 0x0416, 0x0417, 0x0434,
                    0x0435, 0x0451, 0x0436, 0x0437

            },
            // number 4
            {
                    0x0418, 0x0419, 0x041A, 0x041B, 0x0438, 0x0439,
                    0x043A, 0x043B
            },
            // number 5
            {
                    0x041C, 0x041D, 0x041E, 0x041F, 0x043C, 0x043D,
                    0x043E, 0x043F
            },
            // number 6
            {
                    0x0420, 0x0421, 0x0422, 0x0423, 0x0440, 0x0441,
                    0x0442, 0x0443
            },
            // number 7
            {
                    0x0424, 0x0425, 0x0426, 0x0427, 0x0444, 0x0445,
                    0x0446, 0x0447
            },
            // number 8
            {
                    0x0428, 0x0429, 0x042A, 0x042B, 0x0448, 0x0449,
                    0x044A, 0x044B
            },
            // number 9
            {
                    0x042C, 0x042D, 0x042E, 0x042F, 0x044C, 0x044D,
                    0x044E, 0x044F
            }
    };

    public static DigitMapTableRussian getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableRussian();
        }
        return sInstance;
    }

    private DigitMapTableRussian() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(RUSSIAN_TABLE[i], c)) {
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
