/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the latin-characters to digits map.
 * </p>
 */
public class DigitMapTableLatin {

    private static DigitMapTableLatin sInstance = null;

    protected static final int KEY_NUM = 10;

    private static final char[][] LATIN_TABLE = {
            // number 0
            {
                    0x0030
            },
            // number 1
            {
                    0x0031
            },
            // number 2
            {
                    0x0032, 0x0061, 0x0062, 0x0063, 0x0041, 0x0042,
                    0x0043, 0x00C0, 0x00C1, 0x00C4, 0x00C6, 0x00C2,
                    0x00C3, 0x0104, 0x00C5, 0x0100, 0x0102, 0x0181,
                    0x00C7, 0x010C, 0x0106, 0x00E0, 0x00E1, 0x00E4,
                    0x00E6, 0x00E2, 0x00E3, 0x0105, 0x00E5, 0x0101,
                    0x0103, 0x0253, 0x00E7, 0x010D, 0x0107
            },
            // number 3
            {
                    0x0033, 0x0064, 0x0065, 0x0066, 0x0044, 0x0045,
                    0x0046, 0x0110, 0x010E, 0x018A, 0x00D0, 0x00C9,
                    0x00C8, 0x00CA, 0x011A, 0x00CB, 0x0118, 0x0112,
                    0x0116, 0x018F, 0x1EB8, 0x0111, 0x010F, 0x0257,
                    0x00F0, 0x00E9, 0x00E8, 0x00EA, 0x011B, 0x00EB,
                    0x0119, 0x0113, 0x0117, 0x0259, 0x1EB9
            },
            // number 4
            {
                    0x0034, 0x0067, 0x0068, 0x0069, 0x0047, 0x0048,
                    0x0049, 0x011E, 0x0122, 0x0130, 0x00CD, 0x00CE,
                    0x00CF, 0x00CC, 0x012A, 0x012E, 0x1ECA, 0x011F,
                    0x0123, 0x0131, 0x00ED, 0x00EE, 0x00EF, 0x00EC,
                    0x012B, 0x012F, 0x1ECB
            },
            // number 5
            {
                    0x0035, 0x006A, 0x006B, 0x006C, 0x004A, 0x004B,
                    0x004C, 0x0136, 0x0198, 0x0139, 0x013D, 0x0141,
                    0x013B, 0x0137, 0x0199, 0x013A, 0x013E, 0x0142,
                    0x013C
            },
            // number 6
            {
                    0x0036, 0x006D, 0x006E, 0x006F, 0x004D, 0x004E,
                    0x004F, 0x00D1, 0x0147, 0x0143, 0x0145, 0x00D6,
                    0x00D3, 0x00D2, 0x00D4, 0x00D8, 0x00D5, 0x0150,
                    0x1ECC, 0x01A0, 0x0152, 0x00F1, 0x0148, 0x0144,
                    0x0146, 0x00F6, 0x00F3, 0x00F2, 0x00F4, 0x00F8,
                    0x00F5, 0x0151, 0x1ECD, 0x01A1
            },
            // number 7
            {
                    0x0037, 0x0070, 0x0071, 0x0072, 0x0073, 0x0050,
                    0x0051, 0x0052, 0x0053, 0x0158, 0x0154, 0x0156,
                    0x015E, 0x0160, 0x015A, 0x00DF, 0x1E62, 0x0159,
                    0x0155, 0x0157, 0x015F, 0x0161, 0x015B, 0x00DF,
                    0x1E63
            },
            // number 8
            {
                    0x0038, 0x0074, 0x0075, 0x0076, 0x0054, 0x0055,
                    0x0056, 0x00DE, 0x0164, 0x0162, 0x00DC, 0x00DA,
                    0x00D9, 0x00DB, 0x016E, 0x016A, 0x0172, 0x0170,
                    0x1EE4, 0x01AF, 0x00FE, 0x0165, 0x0163, 0x00FC,
                    0x00FA, 0x00F9, 0x00FB, 0x016B, 0x0173, 0x0171,
                    0x1EE5, 0x01B0
            },
            // number 9
            {
                    0x0039, 0x0077, 0x0078, 0x0079, 0x007A, 0x0057,
                    0x0058, 0x0059, 0x005A, 0x00DD, 0x01B3, 0x017D,
                    0x0179, 0x017B, 0x00FD, 0x01B4, 0x017E, 0x017A,
                    0x017C
            }
    };

    public static DigitMapTableLatin getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableLatin();
        }
        return sInstance;
    }

    protected boolean isNumber(char c) {
        return (c >= '0' && c <= '9');
    }

    /**
     * Check if the target is in the source.
     *
     * @param source
     * @param target
     * @return false if target was not existing in source
     */
    protected boolean contains(char[] source, char target) {
        if (source == null || target == 0) {
            return false;
        }

        boolean result = false;
        for (char c : source) {
            if (c == target) {
                result = true;
                break;
            }
        }
        return result;
    }

    /**
     * Get the mapping digit string from a char. If the char is not contained in
     * Latin alphabets, "0" is returned.
     *
     * @param c
     * @return
     */
    public String toDigits(char c) {
        String digits;
        if (isNumber(c)) {
            digits = "" + c;
        } else {
            int idx = -1;
            for (int i = 0; i < KEY_NUM; i++) {
                if (contains(LATIN_TABLE[i], c)) {
                    idx = i;
                    break;
                }
            }

            if (idx == -1) {
                digits = "" + Constants.SYMBOL_UNKNOWN_CHAR;
            } else {
                digits = Integer.toString(idx);
            }
        }
        return digits;
    }
}
