/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
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

package com.wdstechnology.android.kryten.sax;

import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import com.wdstechnology.android.kryten.ConfigurationListItem;

import android.os.SystemProperties;

import android.util.Log;

public class WapProvisioningDocContentHandler implements SimpleContentHandler {

    private static final String ROOT_DOCUMENT_ELEMENT = "ROOT";
    private static final String CHARACTERISTIC_DOCUMENT_ELEMENT = "characteristic";
    private static final String PARM_DOCUMENT_ELEMENT = "parm";
    public static final String APPLICATION_ID_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[name=APPID]]";
    private static final String MNC_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], " +
            "characteristic[type=VALIDITY], parm[name=NETWORK]]";
    private static final String MCC_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], " +
            "characteristic[type=VALIDITY], parm[name=COUNTRY]]";
    private static final String VENDORCONFIG_ID_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=VENDORCONFIG], parm[name=NAME]]";
    private static final String BOOTSTRAP_NAME_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=BOOTSTRAP], parm[name=NAME]]";
    private static final String NAPDEF_BEARER_TYPE_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], parm[name=BEARER]]";
    private static final String NAPDEF_ADDRTYPE_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], parm[name=NAP-ADDRTYPE]]";
    private static final String NAPDEF_ADDRESS_PATH =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], parm[name=NAP-ADDRESS]]";
    private static final String NAPDEF_AUTHTYPE =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], " +
            "characteristic[type=NAPAUTHINFO], parm[name=AUTHTYPE]]";
    private static final String NAPDEF_AUTHNAME =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], " +
            "characteristic[type=NAPAUTHINFO], parm[name=AUTHNAME]]";
    private static final String NAPDEF_AUTHSECRET =
            "[ROOT, wap-provisioningdoc, characteristic[type=NAPDEF], " +
            "characteristic[type=NAPAUTHINFO], parm[name=AUTHSECRET]]";
    public static final String MMSC =
            "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[name=ADDR]]";
    public static final String MMSPROXY =
            "[ROOT, wap-provisioningdoc, characteristic[type=PXLOGICAL], " +
            "characteristic[type=PXPHYSICAL], parm[name=PXADDR]]";
    public static final String MMSPORT =
            "[ROOT, wap-provisioningdoc, characteristic[type=PXLOGICAL], " +
            "characteristic[type=PXPHYSICAL], characteristic[type=PORT], parm[name=PORTNBR]]";

    public static final String APPLICATION_ID_PATH_BROWSER = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[[name=APPID] [value=w2]]]";
    public static final String APPLICATION_ID_PATH_MMS = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[[name=APPID] [value=w4]]]";
    public static final String APPLICATION_ID_PATH_POP3 = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[[name=APPID] [value=110]]]";
    public static final String APPLICATION_ID_PATH_IMAP4 = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[[name=APPID] [value=143]]]";
    public static final String APPLICATION_ID_PATH_SMTP = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[[name=APPID] [value=25]]]";

    public static final String EMAIL_DISPLAYNAME = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[name=DISPLAY-NAME]]";
    public static final String EMAIL_ID = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], parm[name=FROM]]";
    public static final String EMAIL_PASSWORD = "[ROOT, wap-provisioningdoc, characteristic[type=APPLICATION], characteristic[type=APPAUTH], parm[name=AAUTHSECRET]]";

    private String mInboundServerString = null ;
    private String mOutboundServerString = null ;
    private String mInboundServerSecurity = null ;
    private String mOutboundServerSecurity = null ;
    private String appIdPrev = null;
    private String appIdTemp = null;
    private String isCharAppAddr = null;
    private String isCharPort = null;

    public Map<String, String> mParameters = new HashMap<String, String>();
    private Stack<String> mContext = new Stack<String>();
    private boolean mHasParsedCompleteDocument = false;

    @Override
    public void startDocument() {
        mContext.push(ROOT_DOCUMENT_ELEMENT);
    }

    @Override
    public void startElement(String elementName, Map<String, String> attributes) {
        mContext.push(createContextPath(elementName, attributes));

        if (PARM_DOCUMENT_ELEMENT.equals(elementName)) {
            addParameter(attributes);
        }
    }

    @Override
    public void endElement(String elementName) {
        mContext.pop();
    }

    @Override
    public void endDocument() {
        mHasParsedCompleteDocument = ROOT_DOCUMENT_ELEMENT.equals(mContext.pop())
                && mContext.empty();
    }

    public boolean isDocumentWellFormed() {
        return mHasParsedCompleteDocument;
    }

    public Map<String, String> getParameters() {

        HashMap<String, String> p = new HashMap<String, String>();
        p.put("name", mParameters.get(BOOTSTRAP_NAME_PATH));
        p.put("apn", mParameters.get(NAPDEF_ADDRESS_PATH));
        p.put("authtype", mParameters.get(NAPDEF_AUTHTYPE));
        p.put("user", mParameters.get(NAPDEF_AUTHNAME));
        p.put("password", mParameters.get(NAPDEF_AUTHSECRET));
        p.put("mcc", mParameters.get(MCC_PATH));
        p.put("mnc", mParameters.get(MNC_PATH));
        p.put("mmsc", mParameters.get(MMSC));
        p.put("mmsproxy", mParameters.get(MMSPROXY));
        p.put("mmsport", mParameters.get(MMSPORT));

        p.put("appIdBrowser", mParameters.get(APPLICATION_ID_PATH_BROWSER));
        p.put("appIdMms", mParameters.get(APPLICATION_ID_PATH_MMS));
        p.put("appIdPop3", mParameters.get(APPLICATION_ID_PATH_POP3));
        p.put("appIdImap4", mParameters.get(APPLICATION_ID_PATH_IMAP4));
        p.put("appIdSmtp", mParameters.get(APPLICATION_ID_PATH_SMTP));

        //Email
        p.put("inbound_server_uri", mInboundServerString);
        p.put("outbound_server_uri", mOutboundServerString);
        if (mParameters.get(APPLICATION_ID_PATH_POP3) != null)
            p.put("inbound_portnbr", ConfigurationListItem.POP3TAG);
        else if (mParameters.get(APPLICATION_ID_PATH_IMAP4) != null)
            p.put("inbound_portnbr", ConfigurationListItem.IMAPTAG);
        if(mParameters.get(APPLICATION_ID_PATH_SMTP) != null)
            p.put("outbound_portnbr", ConfigurationListItem.SMTPTAG);
        p.put("inbound_service", mInboundServerSecurity);
        p.put("outbound_service", mOutboundServerSecurity);
        p.put("email_displayname",  mParameters.get(EMAIL_DISPLAYNAME));
        p.put("email_id",  mParameters.get(EMAIL_ID));
        p.put("email_password",  mParameters.get(EMAIL_PASSWORD));

        return p;
    }

    private String createContextPath(String elementName,
            Map<String, String> attributes) {
        String attType = attributes.get("type");
        String attName = attributes.get("name");
        String attValue = attributes.get("value");

            if (CHARACTERISTIC_DOCUMENT_ELEMENT.equals(elementName)) {
                String printElem = elementName + "[type="
                        + attType + "]";
                if (attType != null) {
                    isCharAppAddr = attType;
                    isCharPort = attType;
                }
                return elementName + "[type=" + attType + "]";
            } else if (PARM_DOCUMENT_ELEMENT.equals(elementName)) {
                if ((attValue != null)
                        && (attName != null)
                        && (attName.equals("APPID"))
                        && (attValue.equals(ConfigurationListItem.BROWSERTAG)
                                || attValue.equals(ConfigurationListItem.MMSTAG)
                                || attValue.equals(ConfigurationListItem.POP3TAG)
                                || attValue.equals(ConfigurationListItem.IMAPTAG) || attributes
                                .get("value").equals(ConfigurationListItem.SMTPTAG))) {
                    appIdPrev = attValue;
                    appIdTemp = attValue;
                    String print = elementName + "[[name="
                            + attName + "]" + " " + "[value="
                            + attValue + "]]";
                    return elementName + "[[name=" + attName
                            + "]" + " " + "[value=" + attValue
                            + "]]";
                } else if (isCharAppAddr != null && isCharAppAddr.equals("APPADDR")
                        && attName.equals("ADDR")) {

                    if (appIdPrev != null
                            && ((appIdPrev.equals(ConfigurationListItem.POP3TAG)) || (appIdPrev
                                    .equals(ConfigurationListItem.IMAPTAG)))) {

                        mInboundServerString = attValue;

                    } else if (appIdPrev != null && appIdPrev.equals(ConfigurationListItem.SMTPTAG)) {

                            mOutboundServerString = attValue;

                    }
                    appIdPrev = null;
                    isCharAppAddr = null;
                    return elementName;
                } else if (isCharPort != null && isCharPort.equals("PORT")
                        && attName.equals("SERVICE")) {
                    if (appIdTemp.equals(ConfigurationListItem.POP3TAG) || appIdTemp.equals(ConfigurationListItem.IMAPTAG)) {
                        mInboundServerSecurity = attValue;
                    } else if (appIdTemp.equals(ConfigurationListItem.SMTPTAG)) {
                        mOutboundServerSecurity = attValue;
                    }
                    appIdTemp = null;
                    isCharPort = null;
                    return elementName;
                } else {

                    return elementName + "[name=" + attName
                            + "]";
                }
            } else {
                return elementName;
            }
    }

    private void addParameter(Map<String, String> attributes) {
        mParameters.put(mContext.toString(), attributes.get("value"));
    }

    public boolean isDocumentValid() {
        String name = mParameters.get(BOOTSTRAP_NAME_PATH);
        String bearer = mParameters.get(NAPDEF_BEARER_TYPE_PATH);
        String napType = mParameters.get(NAPDEF_ADDRTYPE_PATH);
        String vendorId = mParameters.get(VENDORCONFIG_ID_PATH);
        String applicationIdBrowser = mParameters.get(APPLICATION_ID_PATH_BROWSER);
        String applicationIdMms = mParameters.get(APPLICATION_ID_PATH_MMS);
        String applicationIdPop3 = mParameters.get(APPLICATION_ID_PATH_POP3);
        String applicationIdImap4 = mParameters.get(APPLICATION_ID_PATH_IMAP4);
        String applicationIdSmtp = mParameters.get(APPLICATION_ID_PATH_SMTP);
        String device;
        if ((vendorId == null) || vendorId.equals(""))
            vendorId = "NETWORKIDENT";
        String mcc = mParameters.get(MCC_PATH);
        String mnc = mParameters.get(MNC_PATH);
        return name != null && !name.equals("") && "GSM-GPRS".equals(bearer)
                && "APN".equals(napType) && "NETWORKIDENT".equals(vendorId) &&
                (ConfigurationListItem.BROWSERTAG.equals(applicationIdBrowser)
                 || ConfigurationListItem.MMSTAG.equals(applicationIdMms)
                 || ConfigurationListItem.POP3TAG.equals(applicationIdPop3)
                 || ConfigurationListItem.IMAPTAG.equals(applicationIdImap4)
                 || ConfigurationListItem.SMTPTAG.equals(applicationIdSmtp));
    }

    public String getName() {
        return mParameters.get(BOOTSTRAP_NAME_PATH);
    }

}
