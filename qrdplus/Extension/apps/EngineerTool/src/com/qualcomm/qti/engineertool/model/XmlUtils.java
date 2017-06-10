/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.model;

import android.content.res.XmlResourceParser;
import android.text.TextUtils;

import org.xmlpull.v1.XmlPullParser;

public class XmlUtils {
    // The attrs as we support.
    public static final String ATTR_ACTION         = "action";
    public static final String ATTR_CARRIER        = "carrier";
    public static final String ATTR_CHECKED        = "checked";
    public static final String ATTR_CLASS          = "class";
    public static final String ATTR_DELAY_MILLIS   = "delay_millis";
    public static final String ATTR_EXPECT         = "expect";
    public static final String ATTR_EXPECT_TYPE    = "expect_type";
    public static final String ATTR_FUNCTION       = "function";
    public static final String ATTR_HINT           = "hint";
    public static final String ATTR_ICON           = "icon";
    public static final String ATTR_ID             = "id";
    public static final String ATTR_INFO           = "info";
    public static final String ATTR_INFO_GRAVITY   = "info_gravity";
    public static final String ATTR_KEY            = "key";
    public static final String ATTR_LABEL          = "label";
    public static final String ATTR_MENU           = "menu";
    public static final String ATTR_MESSAGE        = "message";
    public static final String ATTR_PACKAGE        = "package";
    public static final String ATTR_PARAMS         = "params";
    public static final String ATTR_SUMMARY        = "summary";
    public static final String ATTR_SUMMARY_LABELS = "summary_labels";
    public static final String ATTR_SUMMARY_VALUES = "summary_values";
    public static final String ATTR_TITLE          = "title";
    public static final String ATTR_TYPE           = "type";
    public static final String ATTR_VALUE          = "value";
    public static final String ATTR_XML            = "xml";

    // The tags as we support.
    public static final String TAG_CHECKITEM       = "checkitem";
    public static final String TAG_CHILD           = "child";
    public static final String TAG_DIALOG          = "dialog";
    public static final String TAG_DO              = "do";
    public static final String TAG_EDIT            = "edit";
    public static final String TAG_EXTRA           = "extra";
    public static final String TAG_GROUP           = "group";
    public static final String TAG_INCLUDE         = "include";
    public static final String TAG_INTENT          = "intent";
    public static final String TAG_LIST            = "list";
    public static final String TAG_NEGATIVE        = "negative";
    public static final String TAG_NEUTRAL         = "neutral";
    public static final String TAG_OP              = "op";
    public static final String TAG_POSITIVE        = "positive";
    public static final String TAG_SET             = "set";
    public static final String TAG_SPINNER         = "spinner";
    public static final String TAG_SWITCH          = "switch";
    public static final String TAG_VIEW            = "view";

    /**
     * Try to get the attribute's value as integer, if do not exist this attr, return -1.
     */
    public static int getAttributeValueAsInt(XmlPullParser xml, String attrName) {
        String result = xml.getAttributeValue(null, attrName);
        if (TextUtils.isEmpty(result)) return -1;

        try {
            return Integer.parseInt(result);
        } catch (NumberFormatException ex) {
            return -1;
        }
    }

    /**
     * Try to get the attribute's value as the resource id. If not, return -1.
     */
    public static int getAttributeResourceValue(XmlPullParser xml, String attrName) {
        if (xml instanceof XmlResourceParser) {
            int resId = ((XmlResourceParser) xml).getAttributeResourceValue(null, attrName, -1);
            if (resId > 0) {
                return resId;
            }
        }

        return -1;
    }

    /**
     * Try to get the attribute's value as {@link #StringRes}.
     */
    public static StringRes getAttributeAsStringRes(XmlPullParser xml, String attrName) {
        if (xml instanceof XmlResourceParser) {
            int resId = ((XmlResourceParser) xml).getAttributeResourceValue(null, attrName, -1);
            if (resId > 0) {
                return new StringRes(resId, null);
            }
        }

        return new StringRes(-1, xml.getAttributeValue(null, attrName));
    }
}
