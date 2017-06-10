/* Copyright (c) 2013 Qualcomm Technologies, Inc
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.ims.conference;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.android.ims.ImsConferenceState;

import android.os.Bundle;
import android.os.Parcel;
import android.util.Log;

/*
 * confInfo will maintain context of previous conference info element and present element
 * to do further validation part before updating conference data to UI.
 */
public class ConfInfo {
    // Member variable
    public Element mCachedElement;
    public Element mNewElement;

    // Local variables
    private static SAXXMLHandler sHandler;
    private final int VERSION_INVALID = 0;
    private final int VERSION_EQUAL = 1;
    private final int VERSION_GREATER = 2;
    private final int VERSION_lESSER = 3;
    private final int STATE_FULL = 4;
    private final int STATE_PARTIAL = 5;
    private final int STATE_DELETE = 6;
    public String LOGTAG = "ConfInfo";
    public ArrayList<String> allowedPartial = new ArrayList<String>();
    private boolean debug = false;

    // user (String) : Tel or SIP URI
    public static final int INDEX_USER = 0;
    // user > display text (String)
    public static final int INDEX_DISPLAY_TEXT = 1;
    // user > endpoint (String) : URI or GRUU or Phone number
    public static final int INDEX_ENDPOINT = 2;
    // user > endpoint > status
    public static final int INDEX_STATUS = 3;

    public static final int MAX_CONF_PARTICIPANT_INFO = 4;

    public ConfInfo() {
        mCachedElement = new Element();
        setMapAttributeWithDefaultValue();
        mNewElement = new Element();
        preparePartialList();
    }

    private void setMapAttributeWithDefaultValue() {
        mCachedElement.setMapAttribute();
        mCachedElement.getMapAttribute().put(Element.CONF_VERSION, Element.CONF_NA);
    }

    public static void setSAXHandler(SAXXMLHandler handler) {
        sHandler = handler;
    }

    public void updateConfXmlBytes(byte[] confxml) {
        InputStream is = new ByteArrayInputStream(confxml);
        SAXXMLParser confparser = SAXXMLParser.getSAXXMLParser();
        mNewElement = confparser.parse(is);
        debugLog("*************New Notification*****************");
        updateNotification(null, mCachedElement, mNewElement, -1);
    }

    private void debugLog(String str) {
        if (debug)
            Log.d(LOGTAG, str);
    }

    private void log(String str) {
        Log.d(LOGTAG, str);
    }

    /*
     * adding this method for debugging purpose. not enabled by default
     */
    private void dumpstate() {
        List<Element> list = mCachedElement.getSubElementList();
        if (list != null) {
            int length = list.size();
            debugLog(mCachedElement.getTagName());
            debugLog("sublist length: " + length);
            for (int index = 0; index < length; index++) {
                debugLog("SubElement list Element at Index" + index + "::"
                        + list.get(index).getTagName());
                if (list.get(index).getSubElementList() != null) {
                    int length2 = list.get(index).getSubElementList().size();
                    debugLog("SubElement List Length:" + length2);
                    for (int index2 = 0; index2 < length2; index2++) {
                        debugLog("SubElement List Length:"
                                + list.get(index).getSubElementList().get(index2).
                                        getTagName());
                    }
                } else {
                    debugLog("List two is null");
                }
            }
        } else {
            debugLog("List one is null");
        }
        getUserUriList();
    }

    /*
     * Get user uri list. key is Entity for User tag Users is parent tag
     */
    public String[] getUserUriList() {
        String[] userUri = null;
        if (mCachedElement != null) {
            log("mCachedElement reference " + mCachedElement);
            String version = mCachedElement.getAttributeValue(Element.CONF_VERSION);
            debugLog("version string is " + version);
            if (version != null && !version.equalsIgnoreCase(
                    Element.CONF_NA))
            {
                Element.clearMatchedElementsList();
                ArrayList<Element> usersList = Element.getMatchedElements(
                        Element.CONF_USER, Element.CONF_USERS, mCachedElement);
                int length = usersList.size();
                log("userlist length in getuserUri = " + length);
                userUri = new String[length];
                for (int index = 0; index < length; index++) {
                    userUri[index] = usersList.get(index).getAttributeValue(
                            Element.CONF_ENTITY);
                    log("Inside getUser URI list" + userUri[index].toString());
                }
            }
        } else {
            log("conf_version not valid");
        }
        return userUri;
    }

    /*
     * Get user uri list. key is Entity for User tag Users is parent tag
     */
    public ImsConferenceState getConfUriList() {
        if (mCachedElement != null) {
            log("mCachedElement reference " + mCachedElement);
            String version = mCachedElement.getAttributeValue(Element.CONF_VERSION);
            debugLog("version string is " + version);
            if (version != null && !version.equalsIgnoreCase(Element.CONF_NA))
            {
                Element.clearMatchedElementsList();
                ArrayList<Element> usersList = Element.getMatchedElements(
                        Element.CONF_USER, Element.CONF_USERS, mCachedElement);
                int length = usersList.size();
                log("userlist length in getuserUri = " + length);
                Parcel p = Parcel.obtain();
                p.writeInt(length);
                for (int index = 0; index < length; index++) {
                    String[] participantInfo = getParticipantInfoFromElement(usersList.get(index));

                    log("getConfUriList[" + index + "] -> userEntity="
                            + participantInfo[INDEX_USER]
                            + ", Display Text=" + participantInfo[INDEX_DISPLAY_TEXT]
                            + ", endPoint=" + participantInfo[INDEX_ENDPOINT]
                            + ", status=" + participantInfo[INDEX_STATUS]);

                    p.writeString(participantInfo[INDEX_USER]);

                    Bundle b = new Bundle();
                    b.putString(ImsConferenceState.USER, participantInfo[INDEX_USER]);
                    b.putString(ImsConferenceState.ENDPOINT, participantInfo[INDEX_ENDPOINT]);
                    b.putString(ImsConferenceState.STATUS, participantInfo[INDEX_STATUS]);

                    p.writeParcelable(b, 0);
                }
                p.setDataPosition(0);
                return ImsConferenceState.CREATOR.createFromParcel(p);
            }
        } else {
            log("conf_version not valid");
        }
        return null;
    }

    private String[] getParticipantInfoFromElement(Element e) {
        String[] participantInfo = new String[MAX_CONF_PARTICIPANT_INFO];

        // Extract Entity name from the User element
        participantInfo[INDEX_USER] = e.getAttributeValue(Element.CONF_ENTITY);

        // Extract display text from the User element
        participantInfo[INDEX_DISPLAY_TEXT] = e.getAttributeValue(Element.CONF_DISPLAY_TEXT);

        List<Element> userSubElements = e.getSubElementList();
        for (int i = 0; i < userSubElements.size(); i++) {
            log("subElement[" + i + "]:: " + userSubElements.get(i).getTagName());
            if (userSubElements.get(i).getTagName().equals(Element.CONF_ENDPOINT)) {
                // Extract endpoint->entity from the user->endpoint element
                participantInfo[INDEX_ENDPOINT] = userSubElements.get(i).getAttributeValue(
                        Element.CONF_ENTITY);
                // Extract endpoint->status from the user->endpoint element
                participantInfo[INDEX_STATUS] = userSubElements.get(i).getAttributeValue(
                        Element.CONF_STATUS);
            }
        }

        return participantInfo;
    }

    /*
     * To Find element in UserElementList which is match with the same source element key value If
     * there element match , return element index If no element found , return -1
     */
    private int getElementIndexOnKeyMatch(List<Element> mList,
            Element source, String key) {
        int length = mList.size();
        int ret = -1;
        String value = source.getAttributeValue(key);
        if (value != null) {
            for (int i = 0; i < length; i++) {
                if (value.equalsIgnoreCase(mList.get(i).getAttributeValue(key))) {
                    ret = i;
                    break;
                }
            }
        }
        return ret;
    }

    /*
     * Update Attribute list
     * newElementTags are taken into mapped keys array , and then cached element
     * attributes will be updated picking them from new element attribute list
     */
    private void updateAttributeList(Element cachedElement, Element newElement) {
        Set<String> newElementTags = new HashSet<String>(newElement
                .getMapAttribute().keySet());
        Object mappedKeys[] = newElementTags.toArray();
        for (int i = 0; i < mappedKeys.length; i++) {
            String key = mappedKeys[i].toString();
            cachedElement.setAttributValue(key,
                    newElement.getAttributeValue(key));
        }
    }

    /*
     * Get the key value and for given element tag. key value is unique for non
     * atomic element
     */
    private String getKey(Element element){
        String ret = null;
        String tag = element.getTagName();
        if (tag.equalsIgnoreCase(Element.CONF_USER) ||
                tag.equalsIgnoreCase(Element.CONF_ENDPOINT)
                || tag.equalsIgnoreCase(Element.CONF_ENTRY)) {
            ret = Element.CONF_ENTITY;
        } else if (tag.equalsIgnoreCase(Element.CONF_MEDIA)) {
            ret = Element.CONF_ID;
        } else if (tag.equalsIgnoreCase(Element.CONF_SIDEBAR_REF)) {
            ret = Element.CONF_URI;
        } else {
            log(" :Is not supported");
        }
        return ret;
    }

    /*
     * Look up table to maintain table partial state supporting tags
     */
    private void preparePartialList() {
        allowedPartial.add(Element.CONF_INFO);
        allowedPartial.add(Element.CONF_USERS);
        allowedPartial.add(Element.CONF_USER);
        allowedPartial.add(Element.CONF_ENDPOINT);
        allowedPartial.add(Element.CONF_SIDEBAR_VALUE);
        allowedPartial.add(Element.CONF_SIDEBAR_REF);
    }

    private boolean isPartialAllowed(String tagname) {
        return allowedPartial.contains(tagname);
    }

    /*
     * Partial update of old element with newelementInput :
     * oldList :- Parent of List of elements which are at the same level as the
     * old element & new element null for root element or elements with no
     * sub elements
     * OldElement :- Existing element that needs to be updated null if
     * no matching old element
     * newElement :- New element that has to be added to
     * the existing list
     * index :- non zero if old element is present in the old list
     * -1 if old element is null
     */
    private void updateNotification(List<Element> oldList, Element oldElement,
            Element newElement, int index) {
        log("UpdateNotification args OldList: " + oldList + ", OldElement: " +
                oldElement + ", NewElement: " + newElement + ", index = " + index);
        debugLog("mCachedElement element: " + mCachedElement);
        try {
            String latestState = checkElementState(newElement);
            String newTagName = newElement.getTagName();
            String oldTagName = null;
            if (oldElement != null)
                oldTagName = oldElement.getTagName();
            debugLog("Old Element Tag name: " + oldTagName);
            debugLog("New Element Tag name: " + newTagName);
            /*
             * if state attribute is not present default state should be full
             */
            if (latestState.equalsIgnoreCase(Element.STATE_FULL)) {
                if (oldList == null) {
                    log("Root Element is replced with Full state");
                    mCachedElement = newElement;
                } else {
                    if (oldList != null) {

                        if (index < 0) {
                            debugLog("adding new Element with Full state");
                            oldList.add(newElement);
                        } else {
                            debugLog("replacing Element with Full state with parentTag"
                                    + newElement.getParentTag() + " Index :" + index);
                            oldList.set(index, newElement);
                        }
                    }
                }
            }
            else if (latestState.equalsIgnoreCase(Element.STATE_PARTIAL)) {
                log("updateNotification: partial state");

                try {
                    if (isPartialAllowed(newTagName)) {
                        debugLog("Partial Notification state is supported to this element"
                                + newTagName);
                        if (oldElement != null || index >= 0) {
                            List<Element> mNewSubElementList = newElement.getSubElementList();
                            if (oldList != null) {
                                /*
                                 * old element will be mCached element if it is root
                                 * element otherwise we will pick element at the
                                 * same level in oldlist
                                 */
                                oldElement = oldList.get(index);
                            }
                            List<Element> oldSubElementList =
                                    oldElement.getSubElementList();
                            debugLog("Old Element Tag name: " + oldTagName);
                            updateAttributeList(oldElement, newElement);
                            if (oldSubElementList == null) {
                                oldSubElementList =
                                        reIndexOldElementList(oldSubElementList, newElement);
                            }
                            handleNotificationOnSubElements(mNewSubElementList,
                                    oldSubElementList);
                        } else {
                            debugLog("Partial Notification state, but no old element.");
                            oldList.add(newElement);
                        }
                    }
                } catch (Exception ex) {
                    log("Exception in handlePatialNotification ");
                    ex.printStackTrace();
                }
            } else if (latestState.equalsIgnoreCase(Element.STATE_DELETE)) {
                log("updateNotification Deleting Element");
                if (!newTagName.equals(Element.CONF_INFO)) {
                    oldList.remove(index);
                } else {
                    mCachedElement.ClearAll();
                }
            }
        } catch (NullPointerException ex) {
            log("Null Pointer Exception in UpdateNotification");
            ex.printStackTrace();
        } catch (IndexOutOfBoundsException ex) {
            log("Indexout of bound exception in UpdateNotification");
            ex.printStackTrace();
        }
        log("updateNotification : comming out");
    }

    /*
     * This method helps to iterate through new element and to find
     * corresponding element in old element and calls updateNotification method
     * to update state
     */
    private void handleNotificationOnSubElements(List<Element> mNewSubElementList,
            List<Element> mOldSubElementList) {
        Element newSubElement = null;
        int iIndex = -1;
        Element tempelement;
        int newListLength = mNewSubElementList.size();
        log("updateNotification : HandlePartialNotification");
        for (int elementIndex = 0; elementIndex < newListLength; elementIndex++) {
            newSubElement = mNewSubElementList.get(elementIndex);
            debugLog("New List :" + newListLength + "at index : " + elementIndex
                    + "tag name:"
                    + newSubElement.getTagName());
            log("Old Element List :" + mOldSubElementList);
            if (isPartialAllowed(newSubElement.getTagName())) {
                String key = getKey(newSubElement);
                if (key != null) {
                    log("Key is not null");
                    iIndex = getElementIndexOnKeyMatch(mOldSubElementList,
                            newSubElement, key);
                    debugLog("updateNotification : HandlePartialNotification " +
                            "elementIndex : "
                            + elementIndex);
                    debugLog("Old element index Index value: " + iIndex);
                    updateNotification(mOldSubElementList, null, newSubElement, iIndex);
                }
                else {
                    log("key is null");
                    iIndex = getElementIndexOnTagMatch(mOldSubElementList,
                            newSubElement.getTagName().toString());
                    if (iIndex < 0) {
                        log("element is not found after doing Index on key match");
                        // New element was not present in old list. Update the list.
                        updateNotification(mOldSubElementList, null, newSubElement, iIndex);
                    } else {
                        tempelement = mOldSubElementList.get(iIndex);
                        List<Element> mOldList = tempelement.getSubElementList();
                        List<Element> mNewList = newSubElement.getSubElementList();
                        if (mNewList != null && mOldList != null)
                            handleNotificationOnSubElements(mNewList, mOldList);
                    }
                }
            } else {
                log("Tag not valid for Partial Notification");
                iIndex = getElementIndexOnTagMatch(mOldSubElementList,
                        newSubElement.getTagName().toString());
                debugLog("newSubElement Tag Name: " + newSubElement.getTagName().toString());
                if (iIndex >= 0) {
                    tempelement = mOldSubElementList.get(iIndex);
                    updateNotification(mOldSubElementList, tempelement,
                            newSubElement, iIndex);
                } else {
                    log("May be tags are not at same level checking it by reindexing one level up");
                    if(mOldSubElementList != null){
                    reIndexOldElementList(mOldSubElementList, newSubElement);
                    tempelement = mOldSubElementList.get(iIndex);
                    updateNotification(mOldSubElementList, tempelement,
                            newSubElement, iIndex);
                    }
                }
            }
        }
    }

    /*
     * Helps in rearranging old element list to a new reference once it reaches
     * a null value during its traversal
     */
    private List<Element> reIndexOldElementList(List<Element> mOldSubElementList,
            Element newSubElement) {
        if (mOldSubElementList != null) {
            for (int index = 0; index < mOldSubElementList.size(); index++) {
                Element temp = mOldSubElementList.get(index);
                if (temp.getTagName().equalsIgnoreCase(newSubElement.getTagName())) {
                    debugLog("Old Element List item at Index" + "[" + index + "]"
                            + temp.getTagName());
                    mOldSubElementList = temp.getSubElementList();
                }
            }
        } else {
            debugLog("old element list is null");
        }

        return mOldSubElementList;
    }

    private int getElementIndexOnTagMatch(List<Element> aList, String tagName) {
        int iIndex = -1;
        if (aList != null) {
            int length = aList.size();
            for (int i = 0; i < length; i++) {
                if (aList.get(i).getTagName().equalsIgnoreCase(tagName)) {
                    iIndex = i;
                    break;
                }
            }
        }
        return iIndex;
    }

    /*
     * If state not present to a tag , default state will be full otherwise
     * resultant state will be value of new element of corresponding new
     * notification
     */
    private String checkElementState(Element element) {
        String resultantState = Element.STATE_FULL;
        String elementState;
        elementState = element.getAttributeValue(Element.CONF_STATE);
        if (elementState != null) {
            resultantState = elementState;
        }
        debugLog("Notification state is" + resultantState + " element state " + elementState);
        return resultantState;
    }

    public int validateConfInfoVersion(Element newmessage) {
        int oldversion = Integer.parseInt(mCachedElement
                .getAttributeValue(Element.CONF_VERSION));
        if (newmessage.getMapAttribute().containsKey(Element.CONF_VERSION)) {
            int newversion = Integer.parseInt(newmessage
                    .getAttributeValue(Element.CONF_VERSION));
            if (oldversion == newversion) {
                return this.VERSION_EQUAL;
            } else if (oldversion > newversion) {
                return this.VERSION_GREATER;
            } else if (oldversion < newversion) {
                return this.VERSION_lESSER;
            }
        }
        return this.VERSION_INVALID;
    }

    public void UpdateLocalVersionNumber(Map<String, String> parent,
            String value) {
        if (parent.containsKey(Element.CONF_VERSION)) {
            if (!parent.get(Element.CONF_VERSION).equalsIgnoreCase(value)) {
                parent.remove(Element.CONF_VERSION);
            }
        }
        parent.put(Element.CONF_VERSION, value);
    }

    public String getState(Map<String, String> parent) {
        if (parent.containsKey(Element.CONF_STATE)) {
            return parent.get(Element.CONF_STATE);
        } else {
            return null;
        }
    }

    public void UpdateConfState(Map<String, String> parent, String value) {
        parent.put(Element.CONF_STATE, value);
    }

    public void UpdateConfState(String key, String value) {
        mCachedElement.setAttributValue(key, value);
    }

    public void UpdateConfInfoAllElement(Element element) {
        mCachedElement = element;
    }

    private void UpdateConfInfoElement(String key, String value) {
        sHandler.getConferenceInfoLatestMessage().setAttributValue(key, value);
    }

    private void UpdateConfDesElement(String key, String value) {
        sHandler.getConferenceDescElement().setAttributValue(key, value);
    }

    private boolean compareElements(Element lhs) {
        int index = 0;
        int lindex = 0;
        if (lhs.getTagName().equalsIgnoreCase(mCachedElement.getTagName())
                && mCachedElement.getParentTag().equalsIgnoreCase(
                        mCachedElement.getParentTag())) {
            /*
             * if(this.mSubElement != null && lhs.mSubElement != null){ index =
             * this.mSubElement.size(); lindex = lhs.mSubElement.size();
             * if(index == lindex){ while(index!=0 ){
             * compareElements(lhs.mSubElement.get(index)); index --; }} return
             * false; }
             */
            if (lhs.getMapAttribute() != null
                    && mCachedElement.getMapAttribute() != null) {
                if (lhs.getMapAttribute().equals(
                        mCachedElement.getMapAttribute()))
                    return true;
            }
        }
        return false;
    }

    public void dispose() {
        if (mCachedElement != null)
            mCachedElement.ClearAll();
        Element.clearMatchedElementsList();
        if (mNewElement != null)
            mNewElement.ClearAll();
    }

    public void clearAndSetDefault() {
        dispose();
        setMapAttributeWithDefaultValue();
    }

    private int getConfStateValue() {
        return 1;
    }
}
