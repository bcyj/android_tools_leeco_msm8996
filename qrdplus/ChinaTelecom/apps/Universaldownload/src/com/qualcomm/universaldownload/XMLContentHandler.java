/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.util.Log;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import java.util.ArrayList;

public class XMLContentHandler extends DefaultHandler {

    public static final String TAG_CAPA = "capability";
    public static final String TAG_VERSION = "version";
    public static final String TAG_ADDR = "addr";
    public static final String TAG_MD5 = "md5";
    public static final String TAG_SERVER_IP = "server-ip";
    public static final String TAG_RESPONSE_SERVER = "response-addr";
    private static final String TAG = "XMLContentHandler";
    private ArrayList<UpdateInfo> mUpdateInfoList;
    private UpdateInfo mUpdateInfo = null;
    private StringBuilder content;
    private String mServerIP;
    private String mResponseServer;

    @Override
    public void startDocument() throws SAXException {
        super.startDocument();
        mUpdateInfoList = new ArrayList<UpdateInfo>();
        Log.i(TAG, "startDocument");
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        super.characters(ch, start, length);
        String s = new String(ch, start, length);
        if(content != null) {
            content.append(s);
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes)
            throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        content = new StringBuilder();
        if(TAG_CAPA.equals(localName)) {
            if(mUpdateInfo != null) {
                mUpdateInfoList.add(mUpdateInfo);
            }
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        Log.i(TAG, "endElement localName is " + localName + " content is " + content);
        if(TAG_CAPA.equals(localName)) {
            mUpdateInfo = new UpdateInfo();
            mUpdateInfo.setCapibility(content.toString());
        } else if(TAG_VERSION.equals(localName)) {
            mUpdateInfo.setVersion(content.toString());
        } else if(TAG_ADDR.equals(localName)) {
            mUpdateInfo.setAddr(content.toString());
        } else if(TAG_MD5.equals(localName)) {
            mUpdateInfo.setMd5(content.toString());
        } else if(TAG_SERVER_IP.equals(localName)) {
            mServerIP = content.toString();
        } else if(TAG_RESPONSE_SERVER.equals(localName)) {
            mResponseServer = content.toString();
        }
        content = null;
    }

    @Override
    public void endDocument() throws SAXException {
        super.endDocument();
        if(mUpdateInfo != null) {
            mUpdateInfoList.add(mUpdateInfo);
        }
    }

    public ArrayList<UpdateInfo> getUpdateInfos() {
        return mUpdateInfoList;
    }

    public String getServerIP() {
        return mServerIP;
    }

    public String getResponseServer() {
        return mResponseServer;
    }
}
