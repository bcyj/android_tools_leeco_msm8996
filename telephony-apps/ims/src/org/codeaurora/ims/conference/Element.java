/* Copyright (c) 2013 Qualcomm Technologies, Inc
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.ims.conference;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import android.util.Log;

/*
 * This class will act container to conference info xml data
 * Every Parsed parent tag is structured in Element and their attributes will be
 * maintained in mAttribute of particular element object. child element with no further
 * child with also be inserted into mAttribute.
 * */
public class Element {
    public static final String CONF_INFO = "conference-info";
    public static final String CONF_STATE = "state";
    public static final String CONF_VERSION = "version";
    public static final String CONF_ENTITY = "entity";
    public static final String CONF_DESC = "conference-description";
    public static final String CONF_MAX_COUNT = "maximum-user-count";
    public static final String CONF_USER = "user";
    public static final String CONF_USERS = "users";
    public static final String CONF_DISPLAY_TEXT = "display-text";
    public static final String CONF_ASSOCIATED_AORS = "associated-aors";
    public static final String CONF_URI = "uri";
    public static final String CONF_ROLES = "roles";
    public static final String CONF_ENDPOINT = "endpoint";
    public static final String CONF_URIS = "conf_uris";
    public static final String CONF_ENTRY = "entry";
    public static final String CONF_REFERRED = "refrred";
    public static final String CONF_JOINING_INFO = "joining-info";
    public static final String CONF_DISC_INFO = "disconnection-info";
    public static final String CONF_CALL_INFO = "call-info";
    public static final String CONF_SUBJECT = "subject";
    public static final String CONF_FREE_TEXT = "free-text";
    public static final String CONF_KEYWORDS = "keywords";
    public static final String CONF_PURPOSE = "purpose";
    public static final String CONF_LANGUAGE = "languages";
    public static final String CONF_STATUS = "status";
    public static final String CONF_JOIN_METHOD = "joining-method";
    public static final String CONF_DISC_METHOD = "disconnection-method";
    public static final String CONF_WHEN = "when";
    public static final String CONF_REASON = "reason";
    public static final String CONF_BY = "by";
    public static final String CONF_CALL_ID = "call-id";
    public static final String CONF_FROM_TAG = "from-tag";
    public static final String CONF_TO_TAG = "to-tag";
    public static final String CONF_AVIL_MEDIA = "available-media";
    public static final String HOST_INFO = "host-info";
    public static final String STATE_PARTIAL = "partial";
    public static final String STATE_DELETE = "deleted";
    public static final String STATE_FULL = "full";
    public static final String STATE_INVALID = "invalid";
    public static final String CONF_SIDEBAR_VALUE = "sidebars-by-val";
    public static final String CONF_SIDEBAR_REF = "sidebars-by-ref";
    public static final String CONF_MEDIA = "media";
    public static final String CONF_ID = "id";
    public static final String CONF_NA = "NotApplicable";
    private String mTagName;// current tag name
    private String mParentTag; // parent tag name
    private Map<String, String> mAttribute; // attributes as key value
    private List<Element> mSubElement;// list of chaild tag as Element
    private static ArrayList<Element> mMatchedElementList = new ArrayList<Element>();
    private static String LOGTAG = "Element";
    public boolean mIsUpdateRequire = false;

    public void Element() {
        this.mTagName = null;
        this.mParentTag = null;
        this.mAttribute = null;
        this.mSubElement = null;
    }

    public void setTagName(String tag) {
        this.mTagName = tag;
    }

    public void setMapAttribute() {
        this.mAttribute = new HashMap<String, String>();
    }

    public Map<String, String> getMapAttribute() {
        return mAttribute;
    }

    public void setMapAttribute(Map<String, String> marg) {
        this.mAttribute = marg;
    }

    public String getTagName() {
        return this.mTagName;
    }

    public String getParentTag() {
        return this.mParentTag;
    }

    public void setParentTag(String parenttag) {
        this.mParentTag = parenttag;
    }

    public String getAttributeValue(String attname) {
        String ret = null;
        if ((attname != null) && mAttribute != null && (mAttribute.containsKey(attname))) {
            ret = mAttribute.get(attname);
        }
        return ret;
    }

    public void setAttributValue(String key, String value) {
        mAttribute.put(key, value);
    }

    public List<Element> getSubElementList() {
        return this.mSubElement;
    }

    public void addSubElement(Element child) {
        this.mSubElement.add(child);
    }

    public void updateAttributeValue(String attname, String value) {
        this.mAttribute.put(attname, value);
    }

    public void setSubElementList(List<Element> list) {
        this.mSubElement = list;
    }

    private boolean CheckForParentTag(Element element, String Parenttag) {
        if (element.mParentTag.equals(Parenttag)) {
            Log.d(LOGTAG, "Parent Tag" + element.mParentTag);
            Log.d(LOGTAG, "Search Parent Tag" + Parenttag);
            return true;
        } else
            return false;
    }

    public static void clearMatchedElementsList() {
        mMatchedElementList.clear();
    }

    public static ArrayList<Element> getMatchedElements(String tagname,
            String parentTag, Element root) {
        if (root != null) {
            if (root.getSubElementList() == null ) {
                if (root.mParentTag != null && root.mParentTag.equals(parentTag)
                        && root.mTagName.equals(tagname)) {
                    mMatchedElementList.add(root);
                    Log.d(LOGTAG, "Single node element added to mMatchedElementList");
                }
            }else {
                Log.d(LOGTAG, "Sublist not null");
                for (int index = 0; index < root.mSubElement.size(); index++) {

                    Element currentElement = root.getSubElementList().get(index);
                    if (currentElement.mTagName.equals(tagname)) {
                        mMatchedElementList.add(currentElement);
                        Log.d(LOGTAG, "Sub Element " + index + "added to mMatchedElementList");
                    } else if (currentElement.getMapAttribute() != null
                            && currentElement.getMapAttribute().containsKey(tagname)) {
                        if (currentElement.mParentTag.equals(parentTag)) {
                            mMatchedElementList.add(currentElement);
                        } else {
                            Log.d(LOGTAG, "Ignoring sub Element " + index
                                    + "with no match as Parent tag ");
                        }
                    } else {
                        Log.d(LOGTAG, "Recurssive call on index " + index);
                        getMatchedElements(tagname, parentTag, currentElement);
                    }
                }
            }
        } else {
            Log.d(LOGTAG, "Root element is null");
        }
        return mMatchedElementList;
    }

    public ArrayList<Element> getElementWithKey(String tagname,
            String parenttag, String key) {
        int index = 0;
        List<Element> SubElement = getMatchedElements(tagname, parenttag, this);
        ArrayList<Element> KeyMatchElementList = new ArrayList<Element>();
        for (index = 0; index < SubElement.size(); index++) {
            if (SubElement.get(index).mAttribute.containsKey(key)) {
                KeyMatchElementList.add(SubElement.get(index));
            }
        }
        return KeyMatchElementList;
    }

    public String getDocVersion() {
        return this.mAttribute.get(CONF_VERSION);
    }

    public String getElementState(Element element) {
        return element.mAttribute.get(CONF_STATE);
    }

    public void DeleteSubElementsFromTree(Element element) {
        if (element.mAttribute.get(CONF_STATE).equals(STATE_DELETE)) {
            this.ClearAll();
        }
    }

    /*
     * public List<Element> getSubElementList(String tagname,String parenttag){
     * int index = 0; List<Element> SubElement = new ArrayList<Element>();
     * if(this.mParentTag.equals(parenttag)&&this.mTagName.equals(tagname)){
     * return this.getSubElementList(); }else{ getSubElementList( tagname,
     * parenttag) } }
     */
    public void ClearAll() {
        this.mTagName = null;
        if (this.mAttribute != null)
            this.mAttribute.clear();
        this.mAttribute = null;
        if (this.mSubElement != null)
            this.mSubElement.clear();
        this.mSubElement = null;
    }

    /*
     * @Override public int compareTo(Element rhs) { // TODO Auto-generated
     * method stub this.mTagName.equals(rhs.mTagName);
     * this.mParentTag.equals(rhs.mParentTag);
     * this.mAttribute.equals(rhs.mAttribute); compareTo(rhs.mSubElement.get());
     * return 0; }
     */
    public boolean compareElements(Element lhs) {
        int index = 0;
        int lindex = 0;
        if (lhs.mTagName.equals(this.mTagName)
                && lhs.mParentTag.equals(this.mParentTag)) {
            if (this.mSubElement != null && lhs.mSubElement != null) {
                index = this.mSubElement.size();
                lindex = lhs.mSubElement.size();
                if (index == lindex) {
                    while (index != 0) {
                        compareElements(lhs.mSubElement.get(index));
                        index--;
                    }
                }
                return false;
            } else if (this.mAttribute != null && lhs.mAttribute != null) {
                if (lhs.mAttribute.equals(this.mAttribute))
                    return true;
            }
            return true;
        } else {
            return false;
        }
    }

    public boolean IsUpdateRequire() {
        return mIsUpdateRequire;
    }
}
