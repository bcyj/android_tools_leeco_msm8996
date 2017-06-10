/*
 * Copyright (c) 2015 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
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
package com.suntek.mway.rcs.publicaccount.data;

import com.android.vcard.VCardConfig;
import com.android.vcard.VCardInterpreter;
import com.android.vcard.VCardProperty;
import com.android.vcard.VCardUtils;

import android.content.ContentValues;
import android.util.Base64;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

public class RcsVcardNodeBuilder implements VCardInterpreter {
    private static String LOG_TAG = "VNodeBuilder";

    private RcsVcardNode mCurrentVcardNode;

    private List<RcsVcardNode> mVNodeList = new ArrayList<RcsVcardNode>();

    private boolean mStrictLineBreakParsing;

    private String mSourceCharset;

    private String mTargetCharset;

    public RcsVcardNodeBuilder(String targetCharset, boolean strictLineBreakParsing) {
        mSourceCharset = VCardConfig.DEFAULT_INTERMEDIATE_CHARSET;
        if (targetCharset == null) {
            mTargetCharset = VCardConfig.DEFAULT_IMPORT_CHARSET;
        } else {
            mTargetCharset = targetCharset;
        }
        mStrictLineBreakParsing = strictLineBreakParsing;
    }

    public RcsVcardNodeBuilder() {
        this(VCardConfig.DEFAULT_IMPORT_CHARSET, false);
    }

    @Override
    public void onVCardStarted() {
    }

    @Override
    public void onVCardEnded() {
    }

    @Override
    public void onEntryStarted() {
        mCurrentVcardNode = new RcsVcardNode();
        mVNodeList.add(mCurrentVcardNode);
    }

    @Override
    public void onEntryEnded() {
        int lastIndex = mVNodeList.size() - 1;
        // mVNodeList.remove(lastIndex--);
        mCurrentVcardNode = lastIndex >= 0 ? mVNodeList.get(lastIndex) : null;
    }

    @Override
    public void onPropertyCreated(VCardProperty property) {
        RcsPropertyNode rcsPropNode = new RcsPropertyNode();
        rcsPropNode.name = property.getName();
        List<String> groupList = property.getGroupList();
        if (groupList != null) {
            rcsPropNode.propGroupSet.addAll(groupList);
        }
        Map<String, Collection<String>> propertyParameterMap = property.getParameterMap();
        for (String paramType : propertyParameterMap.keySet()) {
            Collection<String> paramValueList = propertyParameterMap.get(paramType);
            if (paramType.equalsIgnoreCase("TYPE")) {
                rcsPropNode.paramMapTYPE.addAll(paramValueList);
            } else {
                for (String paramValue : paramValueList) {
                    rcsPropNode.paramMap.put(paramType, paramValue);
                }
            }
        }

        if (property.getRawValue() == null) {
            rcsPropNode.valueBytes = null;
            rcsPropNode.list.clear();
            rcsPropNode.list.add("");
            rcsPropNode.value = "";
            return;
        }

        final List<String> values = property.getValueList();
        if (values == null || values.size() == 0) {
            rcsPropNode.list.clear();
            rcsPropNode.list.add("");
            rcsPropNode.value = "";
        } else {
            rcsPropNode.list.addAll(values);
            rcsPropNode.value = list2String(rcsPropNode.list);
        }
        rcsPropNode.valueBytes = property.getByteValue();

        mCurrentVcardNode.propNodeList.add(rcsPropNode);
        System.out.println("........" + rcsPropNode.toString());
    }

    private String list2String(List<String> nodeList) {
        int size = nodeList.size();
        if (size == 1) {
            return nodeList.get(0);
        } else if (size > 1) {
            StringBuilder typeListB = new StringBuilder();
            for (String type : nodeList) {
                typeListB.append(type).append(";");
            }
            int len = typeListB.length();
            if (len > 0 && typeListB.charAt(len - 1) == ';') {
                return typeListB.substring(0, len - 1);
            }
            return typeListB.toString();
        } else {
            return "";
        }
    }

    public List<RcsVcardNode> getVcardNodeList() {
        return mVNodeList;
    }

    public RcsVcardNode getCurrentVcardNode() {
        return mCurrentVcardNode;
    }

    public String getResult() {
        throw new RuntimeException("Not supported");
    }
}
