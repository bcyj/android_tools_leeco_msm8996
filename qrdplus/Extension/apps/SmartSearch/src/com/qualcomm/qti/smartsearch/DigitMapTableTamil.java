/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Tamil-characters to digits map.
 * </p>
 */
public class DigitMapTableTamil extends DigitMapTableLatin {

    private static DigitMapTableTamil sInstance = null;

    private static final char[][] EXTEND_TAMIL_TABLE = {
            // number 0
            {
                    0x0BE6
            },
            // number 1
            {
                    0x0B82, 0x0B83, 0x0BBE, 0x0BBF, 0x0BC0, 0x0BC1,
                    0x0BC2, 0x0BC6, 0x0BC7, 0x0BC8, 0x0BCA, 0x0BCB,
                    0x0BCC, 0x0BCD, 0x0BD7, 0x0BE7
            },
            // number 2
            {
                    0x0B85, 0x0B86, 0x0B87, 0x0B88, 0x0B89, 0x0B8A,
                    0x0BE8
            },
            // number 3
            {
                    0x0B8E, 0x0B8F, 0x0B90, 0x0B92, 0x0B93, 0x0B94,
                    0x0BE9
            },
            // number 4
            {
                    0x0B95, 0x0B99, 0x0B9A, 0x0B9E, 0x0BEA
            },
            // number 5
            {
                    0x0B9F, 0x0BA3, 0x0BA4, 0x0BA8, 0x0BEB
            },
            // number 6
            {
                    0x0BAA, 0x0BAE, 0x0BAF, 0x0BEC
            },
            // number 7
            {
                    0x0BB0, 0x0BB2, 0x0BB5, 0x0BED
            },
            // number 8
            {
                    0x0BA9, 0x0BB1, 0x0BB3, 0x0BB4, 0x0BEE
            },
            // number 9
            {
                    0x0B9C, 0x0BB6, 0x0BB7, 0x0BB8, 0x0BB9, 0x0BD0,
                    0x0BEF
            }
    };

    public static DigitMapTableTamil getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableTamil();
        }
        return sInstance;
    }

    private DigitMapTableTamil() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_TAMIL_TABLE[i], c)) {
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
