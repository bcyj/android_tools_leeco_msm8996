/* Copyright (c) 2013 Qualcomm Technologies, Inc
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.ims.conference;

import java.io.InputStream;
import java.util.List;
import javax.xml.parsers.SAXParserFactory;
import java.text.ParseException;
import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import android.util.Log;

public class SAXXMLParser {
    private static SAXXMLParser sXmlparser = null;
    private static SAXXMLHandler sSaxHandler;

    synchronized public static Element parse(InputStream is) {
        try {
            // create a XMLReader from SAXParser
            XMLReader xmlReader = SAXParserFactory.newInstance().newSAXParser()
                    .getXMLReader();
            // create a SAXXMLHandler
            sSaxHandler = new SAXXMLHandler();
            // store handler in XMLReader
            xmlReader.setContentHandler(sSaxHandler);
            InputSource inputSource = new InputSource(is);
            inputSource.setEncoding("utf-8");
            // the process starts
            xmlReader.parse(inputSource);
        } catch (Exception ex) {
            if (ex instanceof InvalidConfVersionException) {
                Log.d("SAXXMLHandler", "Exception caught at SAXParser");
            } else if (ex instanceof ParseException) {
                Log.d("SAXXMLHandler", "InValid Format , Exception in Parser");
                ex.printStackTrace();
            }
        }
        return sSaxHandler.getConferenceInfoLatestMessage();
    }

    public static SAXXMLParser getSAXXMLParser() {
        if (sXmlparser == null) {
            sXmlparser = new SAXXMLParser();
        }
        return sXmlparser;
    }

    public Element getConfInfoHandle() {
        return sSaxHandler.getConferenceInfoLatestMessage();
    }
}
