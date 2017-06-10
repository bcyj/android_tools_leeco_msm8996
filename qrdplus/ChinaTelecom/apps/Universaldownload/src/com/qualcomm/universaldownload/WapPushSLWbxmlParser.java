/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import java.io.StringReader;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.Stack;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class WapPushSLWbxmlParser {

    private final static String versionStr = "<?xml version=\"1.0\" encoding=\"\"?>";
    private final static String SLPublicIdentifier =
            "<!DOCTYPE sl PUBLIC \"-//WAPFORUM//DTD SL 1.0//EN\"" +
                    " \"http://www.wapforum.org/DTD/sl.dtd\">";
    private final static String charsetUTF8 = "encoding=\"UTF-8\"";

    private byte[] mWbxmlData;
    private String mContent;
    private Stack<String> mCurrentTag = new Stack<String>();

    public WapPushSLWbxmlParser(byte[] d) throws Exception {
        mWbxmlData = d;
        mContent = "";
        mCurrentTag.clear();
    }

    public String getContent() {
        return mContent;
    }

    public void Parser() throws Exception {
        byte[] txt;
        String buf;
        String slInfo = versionStr;
        int p = 0;
        boolean isReadStringMode = false;
        boolean isUrl = false;
        boolean needQuotationmark = false;

        if (mWbxmlData.length == 0) {
            throw new Exception("data zero length");
        }

        for (int i = 1; i < mWbxmlData.length; ++i) {
            if (isReadStringMode) {
                switch(mWbxmlData[i]) {
                    case (byte)0x00:
                        isReadStringMode = false;
                        txt = new byte[i - p];
                        System.arraycopy(mWbxmlData, p, txt, 0, i - p);
                        if (isUrl) {
                            slInfo += URLEncoder.encode(new String(txt, "UTF-8"), "UTF-8");
                            isUrl = false;
                        } else {
                            slInfo += new String(txt, "UTF-8");
                        }

                        if (needQuotationmark) {
                            slInfo += "\"";
                            needQuotationmark = false;
                        }
                        break;

                    default:
                        break;
                }
            } else {
                switch(mWbxmlData[i]) {
                    case (byte)0x06:
                        slInfo += SLPublicIdentifier;
                        break;

                    case (byte)0x6A:
                        slInfo = slInfo.replaceAll("encoding=\"\"", charsetUTF8);
                        break;

                    case (byte)0x00:
                        break;

                    case (byte)0x03:
                        isReadStringMode = true;
                        txt = null;
                        p = i + 1;
                        break;

                    case (byte)0x05:
                        slInfo += "<sl ";
                        mCurrentTag.add("sl");
                        mCurrentTag.add(">");
                        break;

                    case (byte)0x0A:
                        slInfo += "href=\"http://";
                        isUrl = true;
                        needQuotationmark = true;
                        break;

                    case (byte)0x01:
                        buf = mCurrentTag.pop();
                        if (buf.equals(">")) {
                            slInfo += ">";
                        } else {
                            slInfo += "</" + buf +">";
                        }
                        break;

                    default:
                        buf = Integer.toHexString(0xFF & mWbxmlData[i]);
                        if (buf.length() == 1) {
                            buf = "0" + buf;
                        }

                        throw new Exception("Unkonwn byte: 0x" + buf.toUpperCase() + " pos: " + i);
                }
            }
        }

        StringReader sr = new StringReader(slInfo);
        InputSource is = new InputSource(sr);
        DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder docBuilder = docBuilderFactory.newDocumentBuilder();
        Document doc = docBuilder.parse(is);

        Element root = doc.getDocumentElement();
        NodeList nodeList = root.getElementsByTagName("sl");
        Element nd = (Element)nodeList.item(0);
        mContent = URLDecoder.decode(nd.getAttribute("href"), "UTF-8");
    }

}
