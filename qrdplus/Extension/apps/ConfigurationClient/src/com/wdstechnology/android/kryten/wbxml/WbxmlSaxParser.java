/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.wdstechnology.android.kryten.wbxml;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import com.wdstechnology.android.kryten.parser.WbxmlParser;
import com.wdstechnology.android.kryten.sax.SimpleContentHandler;

public class WbxmlSaxParser {

    private WbxmlParser mPullParser;

    public WbxmlSaxParser() {
        mPullParser = new WbxmlParser();
        mPullParser.setTagTable(0, WbxmlTokenTable.TAG_TABLE_CODEPAGE_0);
        mPullParser.setTagTable(1, WbxmlTokenTable.TAG_TABLE_CODEPAGE_1);
        mPullParser.setAttrStartTable(0, WbxmlTokenTable.ATTRIBUTE_START_TABLE_CODEPAGE_0);
        mPullParser.setAttrStartTable(1, WbxmlTokenTable.ATTRIBUTE_START_TABLE_CODEPAGE_1);
        mPullParser.setAttrValueTable(0, WbxmlTokenTable.ATTRIBUTE_VALUE_TABLE_CODEPAGE_0);
        mPullParser.setAttrValueTable(1, WbxmlTokenTable.ATTRIBUTE_VALUE_TABLE_CODEPAGE_1);
    }

    public void parse(InputStream bytesIn, SimpleContentHandler contentHandler)
            throws XmlPullParserException, IOException {
        mPullParser.setInput(bytesIn, null);

        contentHandler.startDocument();

        boolean endOfDocument = false;
        while (!endOfDocument) {
            int state = mPullParser.next();

            switch (state) {
                case XmlPullParser.START_TAG: {
                    contentHandler.startElement(mPullParser.getName(), getAttributes());
                    break;
                }

                case XmlPullParser.END_TAG: {
                    contentHandler.endElement(mPullParser.getName());
                    break;
                }

                case XmlPullParser.END_DOCUMENT: {
                    contentHandler.endDocument();
                    endOfDocument = true;
                    break;
                }
            }
        }
    }

    private Map<String, String> getAttributes() {
        Map<String, String> attributes = new HashMap<String, String>();

        int attributeCount = 0;
        if ((attributeCount = mPullParser.getAttributeCount()) > 0) {
            for (int i = 0; i < attributeCount; i++) {
                attributes.put(mPullParser.getAttributeName(i), mPullParser.getAttributeValue(i));
            }
        }

        return attributes;
    }

}
