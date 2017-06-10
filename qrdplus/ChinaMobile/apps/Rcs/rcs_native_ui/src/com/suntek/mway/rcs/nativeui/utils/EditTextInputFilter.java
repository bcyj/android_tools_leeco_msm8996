/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.utils;

import android.text.InputFilter;
import android.text.Spanned;

public class EditTextInputFilter implements InputFilter {
    private int unicodeLength = 0;

    public EditTextInputFilter(int length) {
        this.unicodeLength = length;
    }

    @Override
    public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart,
            int dend) {
        CharSequence result = null;
        int source_count = getWordCount(source.toString());
        int dest_count = getWordCount(dest.toString());
        int keep = unicodeLength - dest_count;
        int count = dest_count + source_count;
        if (keep <= 0) {
            result= "";
        } else if (count <= unicodeLength) {
            result = source;
        } else {
            char[] ch = source.toString().toCharArray();
            int k = keep;
            keep = 0;
            for (int i = 0; i < ch.length; i++) {
                if (getWordCount(ch[i]) == 3) {
                    k = k - 3;
                } else {
                    k--;
                }
                if (k <= 0) {
                    break;
                }
                keep++;
            }
            result = source.subSequence(start, start + keep);
        }
        return result;

    }

    public int getWordCount(CharSequence s) {
        int length = 0;
        for (int i = 0; i < s.length(); i++) {
            int ascii = Character.codePointAt(s, i);
            if (ascii >= 0 && ascii <= 255)
                length++;
            else
                length += 3;
        }
        return length;

    }

    private int getWordCount(char ascii) {
        if (ascii >= 0 && ascii <= 255)
            return 1;
        else
            return 3;
    }

}
