/* Copyright (c) 2013 Qualcomm Technologies, Inc
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.ims.conference;

import java.util.ArrayList;
import java.util.List;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import org.codeaurora.ims.conference.InvalidConfVersionException;
import org.codeaurora.ims.conference.Element;
import android.provider.BaseColumns;
import android.util.Log;

public class SAXXMLHandler extends DefaultHandler {
    private Element mMessageElement;
    private Element mUsersElement;
    private Element mConfDescriptionElement;
    private Element mConfuri;
    private List<Element> mMessageSubList;
    private List<Element> mConfUriSubElementList;
    private List<Element> muserSubElementlist;
    private List<Element> mConfDescSubElementList;
    private List<Element> usersubElementList;
    private List<Element> mUsersList;
    private List<Element> muserEndPointList;
    private List<Element> mConfUriEntryList; // TODO check this reference
    private StringBuilder builder;

    private boolean mInConfDescription = false;
    private boolean mInUser = false;
    private boolean mIsEndPoint = false;
    private boolean mIsEnpointInfo = false;
    private boolean mIsreferredInfo = false;
    private boolean mIsJoiningInfo = false;
    private boolean mIsDisconnectInfo = false;
    private boolean mIsUserCallInfo = false;
    private boolean mIsconuri = false;
    private boolean mIsassociated = false;
    private boolean mIsserviceuri = false;
    private boolean mIsmedia = false;
    private boolean mIsentry = false;
    private boolean mIscallinfo = false;
    private boolean mIsroles = false;


    private final String LOGTAG = "SAXXMLHandler";

    public SAXXMLHandler() {
        Log.d(LOGTAG, "New List obj created");
        mMessageElement = new Element();
    }

    public Element getConferenceInfoLatestMessage() {
        return mMessageElement;
    }

    public Element getConferenceDescElement() {
        return mConfDescriptionElement;
    }

    // public Element getConfUri(){}
    @Override
    public void startDocument() throws SAXException {
        super.startDocument();
        mConfUriEntryList = new ArrayList<Element>();
        muserEndPointList = new ArrayList<Element>();
        builder = new StringBuilder();
    }

    // Event Handlers
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if (qName.equalsIgnoreCase(Element.CONF_INFO)) {
            mMessageElement = new Element();
            mMessageElement.setTagName(Element.CONF_INFO);
            mMessageElement.setMapAttribute();
            mMessageElement.setParentTag(null);
            mMessageElement.setAttributValue(Element.CONF_STATE,
                    attributes.getValue(Element.CONF_STATE));
            mMessageElement.setAttributValue(Element.CONF_VERSION,
                    attributes.getValue(Element.CONF_VERSION));
            mMessageElement.setAttributValue(Element.CONF_ENTITY,
                    attributes.getValue(Element.CONF_ENTITY));
            mMessageSubList = new ArrayList<Element>();
            mMessageElement.setSubElementList(mMessageSubList);
            Log.d(LOGTAG, "New ConfreInf obj created");
        }
        if (qName.equalsIgnoreCase(Element.CONF_DESC)
                || qName.equalsIgnoreCase(Element.CONF_MAX_COUNT)) {
            // create a new instance of employee
            mConfDescriptionElement = new Element();
            mConfDescriptionElement.setTagName(Element.CONF_DESC);
            mConfDescriptionElement.setParentTag(Element.CONF_INFO);
            mConfDescriptionElement.setMapAttribute();
            mConfDescSubElementList = new ArrayList<Element>();
            mConfDescriptionElement.setSubElementList(mConfDescSubElementList);
            mMessageSubList.add(mConfDescriptionElement);
            mInConfDescription = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_USERS)) {
            mUsersElement = new Element();
            mUsersElement.setTagName(Element.CONF_USERS);
            mUsersElement.setParentTag(Element.CONF_INFO);
            muserSubElementlist = new ArrayList<Element>();
            mUsersElement.setSubElementList(muserSubElementlist);
            mMessageSubList.add(mUsersElement);
        } else if (qName.equalsIgnoreCase(Element.CONF_USER)) {
            Log.d(LOGTAG, "User need to added start");
            Element UserElement = new Element();
            UserElement.setTagName(Element.CONF_USER);
            UserElement.setParentTag(Element.CONF_USERS);
            UserElement.setMapAttribute();
            UserElement.setAttributValue(Element.CONF_STATE,
                    attributes.getValue(Element.CONF_STATE));
            UserElement.setAttributValue(Element.CONF_ENTITY,
                    attributes.getValue(Element.CONF_ENTITY));
            muserSubElementlist.add(UserElement);
            /*
             * UserInfoMessage = message.getNewUserInstance();
             * UserInfoMessage.setState
             * (attributes.getValue(Element.CONF_STATE));
             * UserInfoMessage.setEntity
             * (attributes.getValue(Element.CONF_ENTITY));
             */
            mInUser = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT) && mInUser
                && !mIsassociated) {
            mIscallinfo = false;
        } else if (qName.equalsIgnoreCase(Element.CONF_ASSOCIATED_AORS)
                && mInUser) {
            mIsassociated = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)
                && mIsassociated) {
            mIsentry = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_URI) && mIsassociated) {
            mIsentry = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_ROLES)) {
            mIsroles = true;
            mIsentry = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_ENDPOINT) && mInUser) {
            Element endpoint = new Element();
            endpoint.setTagName(Element.CONF_ENDPOINT);
            endpoint.setParentTag(Element.CONF_USER);
            endpoint.setMapAttribute();
            endpoint.setAttributValue(Element.CONF_ENTITY,
                    attributes.getValue(Element.CONF_ENTITY));
            muserEndPointList.add(endpoint);
            mIsEndPoint = true;
        } else if (qName.equalsIgnoreCase(Element.CONF_URIS)) {
            mIsconuri = true;
            mConfuri = new Element();
            mConfuri.setTagName(Element.CONF_URIS);
            mConfuri.setParentTag(Element.CONF_DESC);
            mConfUriSubElementList = new ArrayList<Element>();
            mConfuri.setSubElementList(mConfUriSubElementList);
            mConfDescSubElementList.add(mConfuri);
        } else if (qName.equalsIgnoreCase(Element.CONF_ENTRY) && mIsconuri) {
        }
        if (mIsEndPoint) {
            mIsEnpointInfo = true;
            if (qName.equalsIgnoreCase(Element.CONF_REFERRED)) {
                mIsreferredInfo = true;
            } else if (qName.equalsIgnoreCase(Element.CONF_JOINING_INFO)) {
                mIsJoiningInfo = true;
            } else if (qName.equalsIgnoreCase(Element.CONF_DISC_INFO)) {
                mIsDisconnectInfo = true;
            } else if (qName.equalsIgnoreCase(Element.CONF_CALL_INFO)) {
                // mUserCallInfo =
                // UserInfoMessage.getEndpoint().getUserCallInfo();
                mIsUserCallInfo = true;
            }
        }
        /*
         * else if(qName.equalsIgnoreCase("display-text") && isconuri) { isentry
         * = true; } else if(qName.equalsIgnoreCase("uri") && isconuri) {
         * isentry = true; } else if(qName.equalsIgnoreCase("purpose") &&
         * isconuri) { isentry = true; }
         */
    }

    public void characters(char[] ch, int start, int length)
            throws SAXException {
        super.characters(ch, start, length);
        builder.append(ch, start, length);
    }

    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        super.endElement(uri, localName, qName);
        if (mMessageElement != null) {
            Log.d(LOGTAG, "endelment:inside");
            if (mInConfDescription) {
                if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)) {
                    mConfDescriptionElement.setAttributValue(
                            Element.CONF_DISPLAY_TEXT, builder.toString()
                                    .trim());
                } else if (qName.equalsIgnoreCase(Element.CONF_MAX_COUNT)) {
                    mConfDescriptionElement.setAttributValue(
                            Element.CONF_MAX_COUNT, builder.toString().trim());
                }
                mInConfDescription = false;
            } else if (qName.equalsIgnoreCase(Element.CONF_SUBJECT)) {
                mConfDescriptionElement.setAttributValue(Element.CONF_SUBJECT,
                        builder.toString().trim());
            } else if (qName.equalsIgnoreCase(Element.CONF_FREE_TEXT)) {
                mConfDescriptionElement.setAttributValue(
                        Element.CONF_FREE_TEXT, builder.toString().trim());
            } else if (qName.equalsIgnoreCase(Element.CONF_KEYWORDS)) {
                mConfDescriptionElement.setAttributValue(Element.CONF_KEYWORDS,
                        builder.toString().trim());
            }
            if (mIsconuri) {
                Element mEntry = new Element();
                mEntry.setTagName(Element.CONF_ENTRY);
                mEntry.setParentTag(Element.CONF_URIS);
                if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)) {
                    mEntry.setAttributValue(Element.CONF_DISPLAY_TEXT, builder
                            .toString().trim());
                } else if (qName.equalsIgnoreCase(Element.CONF_URI)) {
                    mEntry.setAttributValue(Element.CONF_URI, builder.toString()
                            .trim());
                } else if (qName.equalsIgnoreCase(Element.CONF_PURPOSE)) {
                    mEntry.setAttributValue(Element.CONF_PURPOSE, builder
                            .toString().trim());
                } else if (qName.equalsIgnoreCase(Element.CONF_ENTRY)) {
                    mConfUriSubElementList.add(mEntry);
                } else if (qName.equalsIgnoreCase(Element.CONF_URIS)) {
                    mConfuri.setSubElementList(mConfUriSubElementList);
                    mIsconuri = false;
                }
            }
            if (mInUser) {
                Element UserElement = muserSubElementlist
                        .get(muserSubElementlist.size() - 1);
                UserElement.setTagName(Element.CONF_USER);
                UserElement.setParentTag(Element.CONF_USERS);
                ArrayList<Element> usersubElementList = new ArrayList<Element>();
                UserElement.setSubElementList(usersubElementList);
                if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)
                        && !mIscallinfo && !mIsassociated) {
                    // UserInfoMessage.setDisplayText(builder.toString().trim());
                    UserElement.setAttributValue(Element.CONF_DISPLAY_TEXT,
                            builder.toString().trim());
                }
                if (mIsentry) {
                    Element Associated = new Element();
                    Associated.setMapAttribute();
                    Element Role;
                    if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)
                            && mIsassociated) {
                        Associated.setTagName(Element.CONF_ASSOCIATED_AORS);
                        Associated.setParentTag(Element.CONF_USER);
                        Associated.setAttributValue(Element.CONF_DISPLAY_TEXT,
                                builder.toString().trim());
                        /*
                         * UserInfoMessage.appendToAssociatedaors(Element.
                         * CONF_DISPLAY_TEXT, builder.toString().trim());
                         */
                    } else if (qName.equalsIgnoreCase(Element.CONF_URI)
                            && mIsassociated) {
                        Associated.setAttributValue(Element.CONF_URI, builder
                                .toString().trim());
                        /*
                         * UserInfoMessage.appendToAssociatedaors(Element.URI,
                         */
                    } else if (qName.equalsIgnoreCase(Element.CONF_ENTRY)
                            && mIsassociated) {
                        usersubElementList.add(Associated);
                        mIsentry = false;
                    } else if (qName.equalsIgnoreCase(Element.CONF_ENTRY)
                            && mIsroles) {
                        Role = new Element();
                        Role.setMapAttribute();
                        Role.setTagName(Element.CONF_ROLES);
                        Role.setParentTag(Element.CONF_ASSOCIATED_AORS);
                        Role.setAttributValue(Element.CONF_ENTRY, builder
                                .toString().trim());
                        usersubElementList.add(Role);
                        // UserInfoMessage.setRoles(builder.toString().trim());
                    } else if (qName.equalsIgnoreCase(Element.CONF_LANGUAGE)) {
                        UserElement.setAttributValue(Element.CONF_LANGUAGE,
                                builder.toString().trim());
                    }
                    /*
                     * else if (qName.equalsIgnoreCase("uri")) {
                     * UserInfoMessage.
                     * appendToAssociatedaors("uri",builder.toString ()); }
                     */
                }
                if (mIsEndPoint) {
                    Element endpoint = muserEndPointList.get(muserEndPointList
                            .size() - 1);
                    endpoint.setTagName(Element.CONF_ENDPOINT);
                    endpoint.setParentTag(Element.CONF_USER);
                    ArrayList<Element> EndpointInfoList = new ArrayList<Element>();
                    if (qName.equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)) {
                        endpoint.setAttributValue(Element.CONF_DISPLAY_TEXT,
                                builder.toString().trim());
                    } else if (qName.equalsIgnoreCase(Element.CONF_STATUS)) {
                        endpoint.setAttributValue(Element.CONF_STATUS, builder
                                .toString().trim());
                    } else if (qName.equalsIgnoreCase(Element.CONF_JOIN_METHOD)) {
                        endpoint.setAttributValue(Element.CONF_JOIN_METHOD,
                                builder.toString().trim());
                    } else if (qName.equalsIgnoreCase(Element.CONF_DISC_METHOD)) {
                        endpoint.setAttributValue(Element.CONF_DISC_METHOD,
                                builder.toString().trim());
                    }
                    if (mIsEnpointInfo) {
                        if (mIsreferredInfo) {
                            Element refferedinfo = new Element();
                            refferedinfo.setTagName(Element.CONF_REFERRED);
                            refferedinfo.setParentTag(Element.CONF_ENDPOINT);
                            refferedinfo.setMapAttribute();
                            EndpointInfoList.add(refferedinfo);
                            if (qName.equalsIgnoreCase(Element.CONF_WHEN)) {
                                refferedinfo.setAttributValue(
                                        Element.CONF_WHEN, builder.toString()
                                                .trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_REASON)) {
                                refferedinfo.setAttributValue(
                                        Element.CONF_REASON, builder.toString()
                                                .trim());
                            } else if (qName.equalsIgnoreCase(Element.CONF_BY)) {
                                refferedinfo.setAttributValue(Element.CONF_BY,
                                        builder.toString().trim());
                            }
                        } else if (mIsJoiningInfo) {
                            Element JoiningInfo = new Element();
                            JoiningInfo.setTagName(Element.CONF_JOINING_INFO);
                            JoiningInfo.setParentTag(Element.CONF_ENDPOINT);
                            JoiningInfo.setMapAttribute();
                            EndpointInfoList.add(JoiningInfo);
                            if (qName.equalsIgnoreCase(Element.CONF_WHEN)) {
                                JoiningInfo.setAttributValue(Element.CONF_WHEN,
                                        builder.toString().trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_REASON)) {
                                JoiningInfo.setAttributValue(
                                        Element.CONF_REASON, builder.toString()
                                                .trim());
                            } else if (qName.equalsIgnoreCase(Element.CONF_BY)) {
                                JoiningInfo.setAttributValue(Element.CONF_BY,
                                        builder.toString().trim());
                            }
                        } else if (mIsDisconnectInfo) {
                            Element DisconnectInfo = new Element();
                            DisconnectInfo.setMapAttribute();
                            DisconnectInfo.setTagName(Element.CONF_DISC_INFO);
                            DisconnectInfo.setParentTag(Element.CONF_ENDPOINT);
                            EndpointInfoList.add(DisconnectInfo);
                            if (qName.equalsIgnoreCase(Element.CONF_WHEN)) {
                                DisconnectInfo.setAttributValue(
                                        Element.CONF_WHEN, builder.toString()
                                                .trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_REASON)) {
                                DisconnectInfo.setAttributValue(
                                        Element.CONF_REASON, builder.toString()
                                                .trim());
                            } else if (qName.equalsIgnoreCase(Element.CONF_BY)) {
                                DisconnectInfo.setAttributValue(
                                        Element.CONF_BY, builder.toString()
                                                .trim());
                            }
                        } else if (mIsUserCallInfo) {
                            Element UserCallInfo = new Element();
                            UserCallInfo.setMapAttribute();
                            UserCallInfo.setTagName(Element.CONF_CALL_INFO);
                            UserCallInfo.setParentTag(Element.CONF_ENDPOINT);
                            EndpointInfoList.add(UserCallInfo);
                            if (qName
                                    .equalsIgnoreCase(Element.CONF_DISPLAY_TEXT)) {
                                UserCallInfo.setAttributValue(
                                        Element.CONF_DISPLAY_TEXT, builder
                                                .toString().trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_CALL_ID)) {
                                UserCallInfo.setAttributValue(
                                        Element.CONF_CALL_ID, builder
                                                .toString().trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_FROM_TAG)) {
                                UserCallInfo.setAttributValue(
                                        Element.CONF_FROM_TAG, builder
                                                .toString().trim());
                            } else if (qName
                                    .equalsIgnoreCase(Element.CONF_TO_TAG)) {
                                UserCallInfo.setAttributValue(
                                        Element.CONF_TO_TAG, builder.toString()
                                                .trim());
                            }
                        }
                    }
                    endpoint.setSubElementList(EndpointInfoList);
                    usersubElementList.add(endpoint);
                }
                if (qName.equalsIgnoreCase(Element.CONF_ASSOCIATED_AORS)) {
                    mIsassociated = false;
                }
                if (qName.equalsIgnoreCase(Element.CONF_USER)) {
                    Log.d(LOGTAG, "User need to added end");
                    mInUser = false;
                    mIsEndPoint = false;
                    mIsEnpointInfo = false;
                    mIsUserCallInfo = false;
                }
                if (qName.equalsIgnoreCase(Element.CONF_USERS)) {
                    // add it to the list
                    mUsersList.add(UserElement);
                }
            }
            if (qName.equalsIgnoreCase(Element.CONF_INFO)) {
                // add it to the list
            }
            builder.setLength(0);
        }
    }

    public void endDocument() {
        try {
            super.endDocument();
            Log.d(LOGTAG, "Root Tag Name:" + mMessageElement.getTagName());
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }
}
