/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.data;

import java.io.InputStream;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.DefaultHandler;

import com.suntek.mway.rcs.client.api.util.log.LogHelper;

public class RcsGeoLocationParser extends DefaultHandler {

    /** The accumulator. */
    private StringBuffer accumulator;

    /** The geo location. */
    private RcsGeoLocation geoLocation = new RcsGeoLocation();

    /**
     * Instantiates a new geo location parser.
     *
     * @param input
     *            the input
     * @throws Exception
     *             the exception
     */
    public RcsGeoLocationParser(InputStream input) throws Exception {
        SAXParserFactory factory = SAXParserFactory.newInstance();
        SAXParser parser = factory.newSAXParser();
        parser.parse(input, this);
    }

    /*
     * (non-Javadoc)
     *
     * @see org.xml.sax.helpers.DefaultHandler#characters(char[], int, int)
     */
    @Override
    public void characters(char buffer[], int start, int length) {
        accumulator.append(buffer, start, length);
    }

    /*
     * (non-Javadoc)
     *
     * @see org.xml.sax.helpers.DefaultHandler#endDocument()
     */
    @Override
    public void endDocument() throws SAXException {
        super.endDocument();
    }

    /*
     * (non-Javadoc)
     *
     * @see org.xml.sax.helpers.DefaultHandler#endElement(java.lang.String,
     * java.lang.String, java.lang.String)
     */
    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        super.endElement(uri, localName, qName);

        String value = accumulator.toString();

        if (localName.equals("pos")) {
            String geoLocs[] = value.split(" ");
            double lat = 0;
            double lng = 0;
            if (geoLocs.length >= 2) {
                try {
                    lat = Double.parseDouble(geoLocs[0]);
                    lng = Double.parseDouble(geoLocs[1]);
                } catch (Exception e) {
                    LogHelper.e("throw exception", e);
                }
            }
            geoLocation.setLng(lng);
            geoLocation.setLat(lat);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see org.xml.sax.helpers.DefaultHandler#startDocument()
     */
    @Override
    public void startDocument() throws SAXException {
        super.startDocument();
        accumulator = new StringBuffer();
    }

    /*
     * (non-Javadoc)
     *
     * @see org.xml.sax.helpers.DefaultHandler#startElement(java.lang.String,
     * java.lang.String, java.lang.String, org.xml.sax.Attributes)
     */
    @Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        accumulator.setLength(0);
        if (localName.equals("rcspushlocation")) {
            String label = attributes.getValue("label");
            geoLocation.setLabel(label);
        }

    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.xml.sax.helpers.DefaultHandler#warning(org.xml.sax.SAXParseException)
     */
    public void warning(SAXParseException exception) {
        // do nothing.
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.xml.sax.helpers.DefaultHandler#error(org.xml.sax.SAXParseException)
     */
    public void error(SAXParseException exception) {
        // do nothing.
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.xml.sax.helpers.DefaultHandler#fatalError(org.xml.sax.SAXParseException
     * )
     */
    public void fatalError(SAXParseException exception) throws SAXException {
        throw exception;
    }

    /**
     * Gets the geo location.
     *
     * @return the geo location
     */
    public RcsGeoLocation getGeoLocation() {
        return geoLocation;
    }

}
