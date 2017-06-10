/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Kanada-characters to digits map.
 * </p>
 */

public class DigitMapTableKannada extends DigitMapTableLatin{

    private static DigitMapTableKannada sInstance = null;

    private static final char[][] EXTEND_KANNADA_TABLE = {
        // number 0
        {
            0x0CE6
        },
        // number 1
        {
            0x0CE7,  0x0C81,  0x0C82,  0x0C83,  0x0CBC,  0x0CBD,
            0x0CBE,  0x0CBF,  0x0CC0,  0x0CC1,  0x0CC2,  0x0CC3,
            0x0CC4,  0x0CC6,  0x0CC7,  0x0CC8,  0x0CCA,  0x0CCB,
            0x0CCC,  0x0CCD,  0x0CD5,  0x0CD6
        },
        // number 2
        {
            0x0CE8,  0x0C85,  0x0C86,  0x0C87,  0x0C88,  0x0C89,
            0x0C8A
        },
        // number 3
        {
            0x0CE9,  0x0C8B,  0x0C8C,  0x0C8E,  0x0C8F,  0x0C90,
            0x0C92,  0x0C93,  0x0C94
        },
        // number 4
        {
            0x0CEA,  0x0C95,  0x0C96,  0x0C97,  0x0C98,  0x0C99,
            0x0C9A,  0x0C9B
        },
        // number 5
        {
            0x0CEB,  0x0C9C,  0x0C9D,  0x0C9E,  0x0C9F,  0x0CA0
        },
        // number 6
        {
            0x0CEC,  0x0CA1,  0x0CA2,  0x0CA3,  0x0CA4,  0x0CA5,
            0x0CA6,  0x0CA7
        },
        // number 7
        {
            0x0CED,  0x0CA8,  0x0CAA,  0x0CAB,  0x0CAC,  0x0CAD,
            0x0CAE
        },
        // number 8
        {
            0x0CEE,  0x0CAF,  0x0CB0,  0x0CB1,  0x0CB2,  0x0CB3
        },
        // number 9
        {
            0x0CEF,  0x0CB5,  0x0CB6,  0x0CB7,  0x0CB8,  0x0CB9
        },
    };

    public static DigitMapTableKannada getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableKannada();
        }
        return sInstance;
    }

    private DigitMapTableKannada() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_KANNADA_TABLE[i], c)) {
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
