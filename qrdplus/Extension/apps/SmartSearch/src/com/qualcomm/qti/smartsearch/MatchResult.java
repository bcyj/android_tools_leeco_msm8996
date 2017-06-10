/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

/**
 * Matched result, used in smart search matching process
 */
public class MatchResult {

    // Used to show whether the char in string matched or not (0 or 1)
    private int[] mBits;

    // The length of mBits
    private int mLen = 0;

    // The contact name converted to digits
    private String mDigitName = null;

    public void setDigitName(String digName) {
        mDigitName = digName;
    }

    public String getDigitName() {
        return mDigitName;
    }

    public int[] getResultBits() {
        return mBits;
    }

    public MatchResult copy() {
        MatchResult copyResult = new MatchResult(mLen);
        for (int i = 0; i < mLen; i++) {
            if (isSetBit(i)) {
                copyResult.setBit(i);
            }
        }
        copyResult.setDigitName(mDigitName);
        return copyResult;
    }

    public MatchResult(int len) {
        mBits = new int[len];
        mLen = len;
        unsetAllBits();
    }

    public MatchResult() {
        this(128);
    }

    public void setBit(int pos) {
        if (pos >= mLen || pos < 0) {
            return;
        }
        mBits[pos] = 1;
    }

    public void unsetBit(int pos) {
        if (pos >= mLen || pos < 0) {
            return;
        }
        mBits[pos] = 0;
    }

    public void setAllBits() {
        for (int i = 0; i < mLen; i++) {
            mBits[i] = 1;
        }
    }

    public void unsetAllBits() {
        for (int i = 0; i < mLen; i++) {
            mBits[i] = 0;
        }
    }

    public boolean isSetBit(int pos) {
        if (pos >= mLen || pos < 0) {
            return false;
        }
        return (mBits[pos] == 1 ? true : false);
    }

    public int getBitsLen() {
        return mLen;
    }

    public int getSetBitsCount() {
        int cnt = 0;
        for (int i = 0; i < mLen; i++) {
            cnt += mBits[i];
        }
        return cnt;
    }

    public int getLastSetBitPos() {
        for (int i = mLen - 1; i >= 0; i--) {
            if (mBits[i] == 1) {
                return i;
            }
        }
        return -1;
    }

    public int getFirstSetBitPos() {
        for (int i = 0; i < mLen; i++) {
            if (mBits[i] == 1) {
                return i;
            }
        }
        return -1;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < mLen; i++) {
            sb.append((mBits[i] == 1) ? '1' : '0');
        }
        return sb.toString();
    }

}
