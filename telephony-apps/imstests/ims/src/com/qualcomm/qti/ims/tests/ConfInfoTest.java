/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.ims.tests;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import android.test.InstrumentationTestCase;
import android.content.res.Resources;
import android.content.res.AssetManager;

import org.codeaurora.ims.conference.ConfInfo;
import org.codeaurora.ims.conference.Element;

public class ConfInfoTest extends InstrumentationTestCase {

    private static final String ASSETS_XML_FILE_PATH = "xml";
    private static final String FILE_SEPARATOR = "/";
    private static final int NUMBER_OF_XML_FILES = 2;
    private AssetManager assetManager;
    private String[] xmlFileNames;

    protected void setUp() throws Exception {
        super.setUp();
        Resources resources = getInstrumentation().getContext().getResources();
        assetManager = resources.getAssets();
        xmlFileNames = assetManager.list(ASSETS_XML_FILE_PATH);
        assertEquals(NUMBER_OF_XML_FILES, xmlFileNames.length);
    }

    public void testValidXmlFiles() throws IOException {
        ConfInfo confInfo = new ConfInfo();

        InputStream inputStream = assetManager.open(ASSETS_XML_FILE_PATH + FILE_SEPARATOR
                + xmlFileNames[0]);
        confInfo.updateConfXmlBytes(getBytesFromInputStream(inputStream));
        assertEquals(Element.CONF_INFO, confInfo.mCachedElement.getTagName());
        String docVersion = confInfo.mCachedElement.getDocVersion();
        assertEquals("1", docVersion);

        inputStream = assetManager.open(ASSETS_XML_FILE_PATH + FILE_SEPARATOR + xmlFileNames[1]);
        confInfo.updateConfXmlBytes(getBytesFromInputStream(inputStream));
        assertEquals(Element.CONF_INFO, confInfo.mCachedElement.getTagName());
        docVersion = confInfo.mCachedElement.getDocVersion();
        assertEquals("2", docVersion);
    }

    private byte[] getBytesFromInputStream(InputStream inputStream) throws IOException {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        byte[] buffer = new byte[0xFFFF];
        for (int len; (len = inputStream.read(buffer)) != -1;) {
                byteArrayOutputStream.write(buffer, 0, len);
        }
        byteArrayOutputStream.flush();
        return byteArrayOutputStream.toByteArray();
    }
}
