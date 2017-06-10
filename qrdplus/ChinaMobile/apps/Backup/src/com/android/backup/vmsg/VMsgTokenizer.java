/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.backup.vmsg;

import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import org.apache.commons.codec.net.QuotedPrintableCodec;
import java.nio.ByteBuffer;
import java.lang.String;
import org.apache.commons.codec.DecoderException;
import android.util.Log;

class VMsgTokenizer {

    private StringTokenizer tokenizer;
    private Token currentToken;
    private static final String SUBJECT_TAG = "Subject";
    private static final String TAG = "VMsgTokenizer";

    public VMsgTokenizer(String msg) {
        tokenizer = new StringTokenizer(msg, "\n\r\f");
        currentToken = prepNextToken();
    }

    public Token next() throws NoSuchElementException {
        if (!tokenizer.hasMoreTokens()) {
            throw new NoSuchElementException();
        }
        return currentToken = prepNextToken();
    }

    public Token current() {
        return currentToken;
    }

    public boolean hasNext() {
        return tokenizer.hasMoreTokens();
    }

    private Token prepNextToken() {
        if (!tokenizer.hasMoreTokens()) {
            return null;
        }
        return new Token(tokenizer.nextToken());
    }

    class Token {

        private String key;
        private String value;
        private String token;

        private Token(String token) {
            this.token = token;
            int i = token.indexOf(":");
            if (i < 0) {
                key = "";
                value = token;
            } else {
                String[] mKey = token.split(";");
                String tempStr = "";
                int ret = 0;
                int record = 0;
                if (mKey.length >= 3 && mKey[0].equalsIgnoreCase(SUBJECT_TAG)) {
                    //
                    key = "Subject";
                    record = ret;
                    for (int j = 0; j < token.length(); j++) {
                        if (token.charAt(j) == ';') {
                            ret = j;
                            break;
                        }
                    }
                    if (ret >= token.length()) {
                        for (int j = 0; j < token.length(); j++) {
                            if (token.charAt(j) == ':') {
                                ret = j;
                                break;
                            }
                        }
                        value = token.substring(ret + 1);
                        return;
                    }
                    String temp1 = token.substring(0, ret);
                    Log.d(TAG, "Temp1=" + temp1);
                    record = ret;
                    for (int j = ret + 1; j < token.length(); j++) {
                        if (token.charAt(j) == ';') {
                            ret = j;
                            break;
                        }
                    }
                    String temp2 = token.substring(record + 1, ret);
                    Log.d(TAG, "Temp2=" + temp2);
                    record = ret;
                    for (int j = ret + 1; j < token.length(); j++) {
                        if (token.charAt(j) == ':') {
                            ret = j;
                            break;
                        }
                    }
                    String temp3 = token.substring(record + 1, ret);
                    Log.d(TAG, "Temp3=" + temp3);

                    byte[] bytes;

                    if (ret < token.length()) {
                        value = token.substring(ret + 1);
                        bytes = value.getBytes();
                        try {
                            bytes = QuotedPrintableCodec.decodeQuotedPrintable(bytes);
                        } catch (DecoderException e) {
                            Log.d(TAG, "QuotedPrintableCodec : DecoderException");
                        }
                        value = new String(bytes);
                    } else {
                        value = "";
                    }

                } else {
                    if (i == token.length()) {
                        key = token.substring(0, i);
                        value = "";
                    } else {
                        key = token.substring(0, i);
                        value = token.substring(i + 1);
                    }
                }
                // sunxiaoming add 2012.8.21 End
            }
        }

        String getKey() {
            return key;
        }

        String getToken() {
            return token;
        }

        String getValue() {
            return value;
        }
    }
}
