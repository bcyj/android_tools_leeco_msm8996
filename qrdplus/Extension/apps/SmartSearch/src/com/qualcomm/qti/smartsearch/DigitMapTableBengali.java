/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Bengali-characters to digits map.
 * </p>
 */
public class DigitMapTableBengali extends DigitMapTableLatin {

    private static DigitMapTableBengali sInstance = null;

    private static final char[][] EXTEND_BENGALI_TABLE = {
            // number 0
            {
                    0x09B9, 0x09BD, 0x09CE, 0x09DC, 0x09DD, 0x09DF,
                    0x09E0, 0x09E1, 0x09E6
            },
            // number 1
            {
                    0x0981, 0x0982, 0x0983, 0x09BC, 0x09BE, 0x09BF,
                    0x09C0, 0x09C1, 0x09C2, 0x09C3, 0x09C4, 0x09C7,
                    0x09C8, 0x09CB, 0x09CC, 0x09CD, 0x09D7, 0x09E2,
                    0x09E3, 0x09E7, 0x09F7
            },
            // number 2
            {
                    0x0985, 0x0986, 0x0987, 0x0988, 0x0989, 0x098A,
                    0x098B, 0x098C, 0x09E8
            },
            // number 3
            {
                    0x098F, 0x0990, 0x0993, 0x0994, 0x09E9
            },
            // number 4
            {
                    0x0995, 0x0996, 0x0997, 0x0998, 0x0999, 0x09EA
            },
            // number 5
            {
                    0x099A, 0x099B, 0x099C, 0x099D, 0x099E, 0x09EB
            },
            // number 6
            {
                    0x099F, 0x09A0, 0x09A1, 0x09A2, 0x09A3, 0x09EC
            },
            // number 7
            {
                    0x09A4, 0x09A5, 0x09A6, 0x09A7, 0x09A8, 0x09ED
            },
            // number 8
            {
                    0x09AA, 0x09AB, 0x09AC, 0x09AD, 0x09AE, 0x09EE
            },
            // number 9
            {
                    0x09AF, 0x09B0, 0x09B2, 0x09B6, 0x09B7, 0x09B8,
                    0x09EF
            }
    };

    public static DigitMapTableBengali getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableBengali();
        }
        return sInstance;
    }

    private DigitMapTableBengali() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_BENGALI_TABLE[i], c)) {
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
