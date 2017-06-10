/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * <p>
 * Implements the Khmer-characters to digits map.
 * </p>
 */
public class DigitMapTableKhmer extends DigitMapTableLatin {

    private static DigitMapTableKhmer sInstance = null;

    private static final char[][] EXTEND_KHMER_TABLE ={
      // number 0
        {
            0x17E0
        },
       // number 1
        {
            0x17E1, 0x17B6, 0x17B7, 0x17B8, 0x17B9, 0x17BA,
            0x17BB, 0x17BC, 0x17BD, 0x17BE, 0x17BF, 0x17C0,
            0x17C1, 0x17C2, 0x17C3, 0x17C4, 0x17C5, 0x17C6,
            0x17C7, 0x17C8
        },
        // number 2
        {
            0x17E2, 0x1780, 0x1781, 0x1782, 0x1783, 0x1784,
            0x1785
        },
        // number 3
        {
            0x17E3, 0x1786, 0x1787, 0x1788, 0x1789, 0x178A,
            0x178B
        },
        // number 4
        {
            0x17E4, 0x178C, 0x178D, 0x178E, 0x178F, 0x1790,
            0x1791
        },
        // number 5
        {
            0x17E5, 0x1792, 0x1793, 0x1794, 0x1795, 0x1796,
            0x1797
        },
        // number 6
        {
            0x17E6, 0x1798, 0x1799, 0x179A, 0x179B, 0x179C,
            0x179D
        },
        // number 7
        {
            0x17E7, 0x179E, 0x179F, 0x17A0, 0x17A1, 0x17A2,
        },
        // number 8
        {
            0x17E8, 0x17A5, 0x17A6, 0x17A7, 0x17A8, 0x17A9,
            0x17A9, 0x17AA, 0x17AB
        },
        // number 9
        {
            0x17E9, 0x17AC, 0x17AD, 0x17AE, 0x17AF, 0X17B0,
            0X17B1, 0X17B2, 0X17B3
        },
    };
    public static DigitMapTableKhmer getInstance() {
        if (sInstance == null) {
            sInstance = new DigitMapTableKhmer();
        }
        return sInstance;
    }

    private  DigitMapTableKhmer() {
    }

    @Override
    public String toDigits(char c) {
        int idx = -1;
        for (int i = 0; i < KEY_NUM; i++) {
            if (contains(EXTEND_KHMER_TABLE[i], c)) {
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
