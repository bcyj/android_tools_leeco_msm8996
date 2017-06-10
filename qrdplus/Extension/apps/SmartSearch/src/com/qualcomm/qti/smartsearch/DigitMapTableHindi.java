/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Hindi-characters to digits map.
 * </p>
 */
public class DigitMapTableHindi extends DigitMapTableLatin {

    private static DigitMapTableHindi sInstance = null;

    private static final char[][] EXTEND_HINDI_TABLE = {
            // number 0
            {
                    0x0966
            },
            // number 1
            {
                    0x0900, 0x0901, 0x0902, 0x0903, 0x093C, 0x093E,
                    0x093F, 0x0940, 0x0941, 0x0942, 0x0943, 0x0944,
                    0x0945, 0x0946, 0x0947, 0x0948, 0x0949, 0x094A,
                    0x094B, 0x094C, 0x094D, 0x094E, 0x094F, 0x0951,
                    0x0952, 0x0953, 0x0954, 0x0955, 0x0962, 0x0963,
                    0x0967
            },
            // number 2
            {
                    0x0968, 0x0904, 0x0905, 0x0906, 0x0907, 0x0908,
                    0x0909, 0x090A
            },
            // number 3
            {
                    0x0969, 0x090D, 0x090E, 0x090F, 0x0910, 0x0913,
                    0x0914
            },
            // number 4
            {
                    0x096A, 0x0915, 0x0916, 0x0917, 0x0918, 0x0919,
                    0x0958, 0x0959, 0x095A
            },
            // number 5
            {
                    0x096B, 0x091A, 0x091B, 0x091C, 0x091D, 0x091E,
                    0x095B, 0x0979
            },
            // number 6
            {
                    0x096C, 0x091F, 0x0920, 0x0921, 0x0922, 0x0923,
                    0x095C, 0x095D
            },
            // number 7
            {
                    0x096D, 0x0924, 0x0925, 0x0926, 0x0927, 0x0928
            },
            // number 8
            {
                    0x096E, 0x092A, 0x092B, 0x092C, 0x092D, 0x092E,
                    0x095E
            },
            // number 9
            {
                    0x096F, 0x092F, 0x0930, 0x0931, 0x0932, 0x0933,
                    0x0934, 0x0935, 0x0936, 0x0937, 0x0938, 0x0939,
                    0x095F, 0x097A
            }
    };

    public static DigitMapTableHindi getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableHindi();
        }
        return sInstance;
    }

    private DigitMapTableHindi() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_HINDI_TABLE[i], c)) {
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
