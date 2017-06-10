/**
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.util.ArrayList;
import java.util.List;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.qualcomm.update.UpdateInfo.Delta;

public class UpdateInfoHandler extends DefaultHandler {

    private List<UpdateInfo> updates;

    private UpdateInfo info;

    public void endElement(String uri, String localName, String qName) throws SAXException {
        if (UpdateInfo.QNAME_UPDATE.equals(qName)) {
            updates.add(info);
        }
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes)
            throws SAXException {
        if (UpdateInfo.QNAME_UPDATE.equals(qName)) {
            info = new UpdateInfo();
        }

        if (UpdateInfo.QNAME_VERSION.equals(qName))
            info.setVersion(attributes.getValue("data"));
        else if (UpdateInfo.QNAME_FILE.equals(qName))
            info.setFileName(attributes.getValue("data"));
        else if (UpdateInfo.QNAME_DES.equals(qName))
            info.setDescription(attributes.getValue("data"));
        else if (UpdateInfo.QNAME_DELTA.equals(qName)) {
            Delta delta = new Delta();
            try {
                delta.from = Integer.parseInt(attributes.getValue("from"));
                delta.to = Integer.parseInt(attributes.getValue("to"));
            } catch (Exception e) {
                return;
            }
            info.setDelta(delta);
        }
    }

    public void startDocument() throws SAXException {
        updates = new ArrayList<UpdateInfo>();
    }

    public List<UpdateInfo> getUpdates() {
        return updates;
    }
}
