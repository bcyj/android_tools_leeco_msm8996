/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Gujarati-characters to digits map.
 * </p>
 */
public class DigitMapTableGujarati extends DigitMapTableLatin{

    private static  DigitMapTableGujarati sInstance = null;

    private static final char[][] EXTEND_GUJARATI_TABLE = {
        // number 0
        {
            0x0AE6
        },
        // number 1
        {
            0x0AE7,  0x0A81,  0x0A82,  0x0A83,  0x0ABC,  0x0ABD,
            0x0ABE,  0x0ABF,  0x0AC0,  0x0AC1,  0x0AC2,  0x0AC3,
            0x0AC4,  0x0AC5,  0x0AC7,  0x0AC8,  0x0AC9,  0x0ACB,
            0x0ACC,  0x0ACD
        },
        // number 2
        {
            0x0AE8,  0x0A85,  0x0A86,  0x0A87,  0x0A88,  0x0A89,
            0x0A8A,  0x0A8B,  0x0AE0,  0x0A8C,  0x0AE1
        },
        // number 3
        {
            0x0AE9,  0x0A8D,  0x0A8F,  0x0A90,  0x0A91,  0x0A93,
            0x0A94
        },
        // number 4
        {
            0x0AEA,  0x0A95,  0x0A96,  0x0A97,  0x0A98,  0x0A99,
            0x0A9A,  0x0A9B
        },
        // number 5
        {
            0x0AEB,  0x0A9C,  0x0A9D,  0x0A9E,  0x0A9F,  0x0AA0
        },
        // number 6
        {
            0x0AEC,  0x0AA1,  0x0AA2,  0x0AA3,  0x0AA4,  0x0AA5,
            0x0AA6,  0x0AA7,  0x0AA8
        },
        // number 7
        {
            0x0AED,  0x0AAA,  0x0AAB,  0x0AAC,  0x0AAD,  0x0AAE,
            0x0AAF
        },
        // number 8
        {
            0x0AEE,  0x0AB0,  0x0AB2,  0x0AB3
        },
        // number 9
        {
            0x0AEF,  0x0AB5,  0x0AB6,  0x0AB7,  0x0AB8,  0x0AB9
        },

    };

    public static  DigitMapTableGujarati getInstance() {
        if (sInstance == null) {
            sInstance = new  DigitMapTableGujarati();
        }
        return sInstance;
    }

    private  DigitMapTableGujarati() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains( EXTEND_GUJARATI_TABLE[i], c)) {
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
