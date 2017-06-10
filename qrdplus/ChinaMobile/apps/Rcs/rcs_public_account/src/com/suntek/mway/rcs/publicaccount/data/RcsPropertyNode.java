/*
 * Copyright (c) 2015 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.data;

import android.content.ContentValues;
import com.android.vcard.VCardEntry;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class RcsPropertyNode implements Serializable {
    public String value;

    public String name;

    public ContentValues paramMap;

    public Set<String> propGroupSet;

    public List<String> list;

    public Set<String> paramMapTYPE;

    public byte[] valueBytes;

    public RcsPropertyNode() {
        value = "";
        name = "";
        paramMap = new ContentValues();
        paramMapTYPE = new HashSet<String>();
        propGroupSet = new HashSet<String>();
        list = new ArrayList<String>();
    }

    public RcsPropertyNode(String propName, String propValue, List<String> propValue_vector,
            byte[] propValue_bytes, ContentValues paramMap, Set<String> paramMap_TYPE,
            Set<String> propGroupSet) {
        if (propName != null) {
            this.name = propName;
        } else {
            this.name = "";
        }
        if (propValue != null) {
            this.value = propValue;
        } else {
            this.value = "";
        }
        if (propValue_vector != null) {
            this.list = propValue_vector;
        } else {
            this.list = new ArrayList<String>();
        }
        this.valueBytes = propValue_bytes;
        if (paramMap != null) {
            this.paramMap = paramMap;
        } else {
            this.paramMap = new ContentValues();
        }
        if (paramMap_TYPE != null) {
            this.paramMapTYPE = paramMap_TYPE;
        } else {
            this.paramMapTYPE = new HashSet<String>();
        }
        if (propGroupSet != null) {
            this.propGroupSet = propGroupSet;
        } else {
            this.propGroupSet = new HashSet<String>();
        }
    }

    @Override
    public int hashCode() {
        throw new UnsupportedOperationException(
                "PropertyNode does not provide hashCode() implementation intentionally.");
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof RcsPropertyNode)) {
            return false;
        }

        RcsPropertyNode propNode = (RcsPropertyNode)obj;

        if (name == null || !name.equals(propNode.name)) {
            return false;
        } else if (!paramMapTYPE.equals(propNode.paramMapTYPE)) {
            return false;
        } else if (!paramMapTYPE.equals(propNode.paramMapTYPE)) {
            return false;
        } else if (!propGroupSet.equals(propNode.propGroupSet)) {
            return false;
        }

        if (valueBytes != null && Arrays.equals(valueBytes, propNode.valueBytes)) {
            return true;
        } else {
            if (!value.equals(propNode.value)) {
                return false;
            }
            return (list.equals(propNode.list) || list.size() == 1 || propNode.list.size() == 1);
        }
    }

    @Override
    public String toString() {
        StringBuilder sBuilder = new StringBuilder();
        sBuilder.append("propName: ");
        sBuilder.append(name);
        sBuilder.append(", paramMap: ");
        sBuilder.append(paramMap.toString());
        sBuilder.append(", paramMap_TYPE: [");
        boolean first = true;
        for (String elem : paramMapTYPE) {
            if (first) {
                first = false;
            } else {
                sBuilder.append(", ");
            }
            sBuilder.append('"');
            sBuilder.append(elem);
            sBuilder.append('"');
        }
        sBuilder.append("]");
        if (!propGroupSet.isEmpty()) {
            sBuilder.append(", propGroupSet: [");
            first = true;
            for (String elem : propGroupSet) {
                if (first) {
                    first = false;
                } else {
                    sBuilder.append(", ");
                }
                sBuilder.append('"');
                sBuilder.append(elem);
                sBuilder.append('"');
            }
            sBuilder.append("]");
        }
        if (list != null && list.size() > 1) {
            sBuilder.append(", propValue_vector size: ");
            sBuilder.append(list.size());
        }
        if (valueBytes != null) {
            sBuilder.append(", propValue_bytes size: ");
            sBuilder.append(valueBytes.length);
        }
        sBuilder.append(", propValue: \"");
        sBuilder.append(value);
        sBuilder.append("\"");
        return sBuilder.toString();
    }
}
