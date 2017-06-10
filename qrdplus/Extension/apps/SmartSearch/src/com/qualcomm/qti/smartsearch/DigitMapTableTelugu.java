/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Telugu-characters to digits map.
 * </p>
 */
public class DigitMapTableTelugu extends DigitMapTableLatin {

    private static DigitMapTableTelugu sInstance = null;

    private static final char[][] EXTEND_TELUGU_TABLE = {
            // number 0
            {
                    0x0C66
            },
            // number 1
            {
                    0x0C01, 0x0C02, 0x0C03, 0x0C3E, 0x0C3F, 0x0C40,
                    0x0C41, 0x0C42, 0x0C43, 0x0C44, 0x0C46, 0x0C47,
                    0x0C48, 0x0C4A, 0x0C4B, 0x0C4C, 0x0C4D, 0x0C55,
                    0x0C56, 0x0C62, 0x0C63, 0x0C67
            },
            // number 2
            {
                    0x0C05, 0x0C06, 0x0C07, 0x0C08, 0x0C09, 0x0C0A,
                    0x0C68
            },
            // number 3
            {
                    0x0C0B, 0x0C0C, 0x0C0E, 0x0C0F, 0x0C10, 0x0C12,
                    0x0C13, 0x0C14, 0x0C69
            },
            // number 4
            {
                    0x0C15, 0x0C16, 0x0C17, 0x0C18, 0x0C19, 0x0C6A
            },
            // number 5
            {
                    0x0C1A, 0x0C1B, 0x0C1C, 0x0C1D, 0x0C1E, 0x0C6B
            },
            // number 6
            {
                    0x0C1F, 0x0C20, 0x0C21, 0x0C22, 0x0C23, 0x0C6C
            },
            // number 7
            {
                    0x0C24, 0x0C25, 0x0C26, 0x0C27, 0x0C28, 0x0C6D
            },
            // number 8
            {
                    0x0C2A, 0x0C2B, 0x0C2C, 0x0C2D, 0x0C2E, 0x0C6E
            },
            // number 9
            {
                    0x0C2F, 0x0C30, 0x0C31, 0x0C32, 0x0C33, 0x0C35,
                    0x0C36, 0x0C37, 0x0C38, 0x0C39, 0x0C3D, 0x0C58,
                    0x0C59, 0x0C60, 0x0C61, 0x0C6F
            }
    };

    public static DigitMapTableTelugu getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableTelugu();
        }
        return sInstance;
    }

    private DigitMapTableTelugu() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_TELUGU_TABLE[i], c)) {
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
