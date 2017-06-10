/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qrd.wappush;

import java.io.IOException;
import java.io.InputStream;
import org.xml.sax.SAXException;
import android.content.Context;
import android.util.Log;
import java.util.*;
import com.android.internal.util.HexDump;


public class WapPushParser
{

    public class CONST_VALUE_TOKEN
    {
        public static final int SWITCH_PAGE          = 0x00;
        public static final int END                  = 0x01;
        public static final int ENTITY           = 0x02;
        public static final int STRING_INLINE        = 0x03;
        public static final int OPAQUE           = 0xC3;
        public static final int STR_TBL_REF          = 0x83;
        public static final int SI_PUB_ID        = 0x05;  //SI 1.0 Public Identifier
        public static final int SL_PUB_ID        = 0x06;  //SL 1.0 Public Identifier
        public static final int SI_TAG_CONTENT_ONLY  = 0x45;
        public static final int SL_TAG           = 0x05;
        public static final int INDICATION_TAG       = 0x06;
    }

    public static final int SI_TYPE          = 0x01;
    public static final int SL_TYPE          = 0x02;
    private static final int ATTR_START_TAG_BASE         = 0x05;
    private static final int ATTR_VALUE_TOKEN_BASE       = 0x55;  //85
    private static final int OPERATION_PARSE_HEAD = 0x01;
    private static final int OPERATION_PARSE_END = 0x02;
    private static final int OPERATION_PARSE_ATTR = 0x03;
    private static final int OPERATION_PARSE_CONTENT = 0x04;

    private static String TAG = "WAP_PUSH_Parser";
    private static HashMap<Integer, String> charsetMapping;
    private static final String ATTR_VALUE_TOKEN[] = new String[]{
            ".com/",
            ".edu/",
            ".net/",
            ".org/",
    };

    private static final String SI_ATTR_START_MAPPING[] = new String[]{
            "action=signal-none",
            "action=signal-low",
            "action=signal-medium",
            "action=signal-high",
            "action=delete",
            "created=",
            "href=",
            "href=http://",
            "href=http://www.",
            "href=https://",
            "href=https://www.",
            "si_expires=",
            "si_id=",
            "class=",
    };

    private static final String SL_ATTR_START_MAPPING[] = new String[]{
            "action=execute-low",
            "action=execute-high",
            "action=cache",
            "href=",
            "href=http://",
            "href=http://www.",
            "href=https://",
            "href=https://www.",
    };

    private int mPushType;
    private byte[] mEmbeddedStrTbl;
    private InputStream mInputStream;

    private int mCharSet;
    private int mVersion;
    private String mAction = null;
    private String mContent = null;
    private String mHyperLink = null;
    private int mCurrentAttrToken;
    private boolean bHaveAttr = false;  //whether the tag have attr
    private boolean bHaveContent = false; //whether the tag have content

    static
    {
        charsetMapping = new HashMap<Integer, String>();
        charsetMapping.put(0, "*");
        charsetMapping.put(3, "us-ascii");
        charsetMapping.put(4, "iso-8859-1");
        charsetMapping.put(5, "iso-8859-2");
        charsetMapping.put(6, "iso-8859-3");
        charsetMapping.put(7, "iso-8859-4");
        charsetMapping.put(8, "iso-8859-5");
        charsetMapping.put(9, "iso-8859-6");
        charsetMapping.put(10, "iso-8859-7");
        charsetMapping.put(11, "iso-8859-8");
        charsetMapping.put(12, "iso-8859-9");
        charsetMapping.put(17, "shift_JIS");
        charsetMapping.put(106, "utf-8");
        charsetMapping.put(2026, "big5");
        charsetMapping.put(1000, "iso-10646-ucs-2");
        charsetMapping.put(1015, "utf-16");
    }


//Help Function start

    private int Helper_readByte() throws IOException, SAXException {
        int nRet = mInputStream.read();
        if(-1 == nRet)
            throw new SAXException("Error: Unexpected EOF");
        else
            return nRet;
    }

    private int Helper_readInt32() throws SAXException, IOException {
       int nTemp;
       int nRet = 0;

        do {
            nTemp = Helper_readByte();
            nRet = (nTemp & 0x7f) | (nRet << 7);
        } while((nTemp & 0x80) != 0);

        return nRet;
    }


    private String Helper_readInlineStr() throws IOException, SAXException {

        int length = 0;
        int nRet = mInputStream.read();
        ArrayList<Byte> inlineArraylist = new ArrayList<Byte>();

        while(nRet != 0) {
            if(nRet == -1)
                throw new SAXException("Error: Unexpected EOF");
            Byte curValue = (byte)(nRet & 0xff);
            inlineArraylist.add(curValue);
            length++;
            nRet = mInputStream.read();
        }

        byte[] destData = new byte[length];
        for(int j = 0; j < length; j++) {
            destData[j] = ((Byte)inlineArraylist.get(j)).byteValue();
        }

        return new String(destData, (String)charsetMapping.get(mCharSet));
    }


    private String Helper_readStringTbl()  throws IOException, SAXException {
        int offset = Helper_readInt32();
        int end = offset;
        for(; mEmbeddedStrTbl[end] != 0; end++);
        int length = end - offset;
        return new String(mEmbeddedStrTbl, offset, length, (String)charsetMapping.get(mCharSet));
    }



    private String Helper_ReadOpaque() throws SAXException, IOException {
        int length = Helper_readInt32();
        byte[] opaqueData = new byte[length];
        for(int j = 0; j < length; j++) {
            opaqueData[j] = (byte)Helper_readByte();
        }

        return HexDump.toHexString(opaqueData);
    }


    private void Helper_IgnoreEntity() throws SAXException, IOException {
        int ignore = Helper_readInt32();
    }

    private void Helper_IgnoreSwitchPage() throws SAXException, IOException {
        int ignore = Helper_readByte();
    }


    private int parseHead() throws SAXException, IOException {
        mVersion = Helper_readByte();
        int nCheckPID = (mPushType == WapPushParser.SI_TYPE) ?
            WapPushParser.CONST_VALUE_TOKEN.SI_PUB_ID : WapPushParser.CONST_VALUE_TOKEN.SL_PUB_ID;
        int nCheckToken = (mPushType == WapPushParser.SI_TYPE) ?
            WapPushParser.CONST_VALUE_TOKEN.INDICATION_TAG: WapPushParser.CONST_VALUE_TOKEN.SL_TAG;

        if(Helper_readInt32() != nCheckPID)
            throw new SAXException("Error: incorrect Public ID in SI Head");
        mCharSet = Helper_readInt32();

        int stringTblLen = Helper_readInt32();
        mEmbeddedStrTbl = new byte[stringTblLen];
        for(int j = 0; j < stringTblLen; j++) {
             mEmbeddedStrTbl[j] = (byte)Helper_readByte();
        }

        if (mPushType == WapPushParser.SI_TYPE) {
            if (Helper_readByte() != WapPushParser.CONST_VALUE_TOKEN.SI_TAG_CONTENT_ONLY)
                throw new SAXException("Error: incorrect SI TAG in SI Head");
        }

        int indication_Tag = Helper_readByte();
        if ((((byte)indication_Tag) & 0x3f) != nCheckToken)
            throw new SAXException("Error: incorrect TAG in Head");

        bHaveAttr = (indication_Tag & 0x80) != 0;
        bHaveContent = (indication_Tag & 0x40) != 0;

        if(bHaveAttr)
            return OPERATION_PARSE_ATTR;
        if(bHaveContent)
            return OPERATION_PARSE_CONTENT;
        throw new SAXException("Error: TAG have none attr and content");
    }


    private void storeAttr(int attr, String value) throws SAXException, IOException {
        int attrValidMax = ((mPushType == WapPushParser.SI_TYPE) ? SI_ATTR_START_MAPPING.length
            : SL_ATTR_START_MAPPING.length) - 1 + ATTR_START_TAG_BASE;
        if(attr > attrValidMax)
            throw new SAXException("Error: unSupported TAG, discard");

        String[] attrStartTemp = (mPushType == WapPushParser.SI_TYPE) ? SI_ATTR_START_MAPPING:
            SL_ATTR_START_MAPPING;

        String attrFullVal = attrStartTemp[attr-ATTR_START_TAG_BASE] + value;

        if(attrFullVal.startsWith("action="))
            mAction = attrFullVal.substring(new String("action=").length());
        else if (attrFullVal.startsWith("href="))
            mHyperLink= attrFullVal.substring(new String("href=").length());
        else
            ; //current not support.
    }

    private void storeContent(String value) throws SAXException, IOException {
        mContent = value;
    }


    private int parseAttr() throws SAXException, IOException {
        StringBuffer attrVal = new StringBuffer();
        int token;
        if (mCurrentAttrToken == 0)
            mCurrentAttrToken = Helper_readByte();

        token = Helper_readByte();

        while(token>0x80 || token==WapPushParser.CONST_VALUE_TOKEN.ENTITY
                 || token==WapPushParser.CONST_VALUE_TOKEN.END ||
                 token==WapPushParser.CONST_VALUE_TOKEN.STRING_INLINE
                 || token==WapPushParser.CONST_VALUE_TOKEN.STR_TBL_REF ||
                 token==WapPushParser.CONST_VALUE_TOKEN.OPAQUE) {

            switch(token) {
                case WapPushParser.CONST_VALUE_TOKEN.END:
                    storeAttr(mCurrentAttrToken, attrVal.toString());
                    return OPERATION_PARSE_CONTENT;

                case WapPushParser.CONST_VALUE_TOKEN.STRING_INLINE:
                    attrVal.append(Helper_readInlineStr());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.STR_TBL_REF:
                    attrVal.append(Helper_readStringTbl());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.OPAQUE:
                    attrVal.append(Helper_ReadOpaque());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.ENTITY:
                    char c = (char)Helper_readInt32();
                    attrVal.append(c);
                    break;

                default:
                    attrVal.append(ATTR_VALUE_TOKEN[token - ATTR_VALUE_TOKEN_BASE]);
                    break;
            }

            token = Helper_readByte();
        }

        storeAttr(mCurrentAttrToken, attrVal.toString());
        mCurrentAttrToken = token;
        return OPERATION_PARSE_ATTR;

    }

    private int parseContent() throws SAXException, IOException {
        if (!bHaveContent)
            return OPERATION_PARSE_END;

        StringBuffer attrVal = new StringBuffer();
        int token;

        token = Helper_readByte();

        while(token>0x80 || token==WapPushParser.CONST_VALUE_TOKEN.ENTITY
                 || token==WapPushParser.CONST_VALUE_TOKEN.SWITCH_PAGE
                 || token==WapPushParser.CONST_VALUE_TOKEN.END
                 || token==WapPushParser.CONST_VALUE_TOKEN.STRING_INLINE
                 || token==WapPushParser.CONST_VALUE_TOKEN.STR_TBL_REF
                 || token==WapPushParser.CONST_VALUE_TOKEN.OPAQUE) {

            switch(token) {
                case WapPushParser.CONST_VALUE_TOKEN.END:
                    storeContent(attrVal.toString());
                    return OPERATION_PARSE_END;

                case WapPushParser.CONST_VALUE_TOKEN.STRING_INLINE:
                    attrVal.append(Helper_readInlineStr());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.STR_TBL_REF:
                    attrVal.append(Helper_readStringTbl());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.OPAQUE:
                    attrVal.append(Helper_ReadOpaque());
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.ENTITY:
                    Helper_IgnoreEntity();
                    break;

                case WapPushParser.CONST_VALUE_TOKEN.SWITCH_PAGE:
                    Helper_IgnoreSwitchPage();
                    break;

                default:
                    attrVal.append(ATTR_VALUE_TOKEN[token - 0x85]);
                    break;
            }

            token = Helper_readByte();
        }

        throw new SAXException("Error: Exception when Parse Content");
    }

    public void parse(InputStream inputstream, int pushType)
            throws SAXException, IOException {
        mInputStream = inputstream;
        mPushType = pushType;
        int nextOperation = OPERATION_PARSE_HEAD;
        mCurrentAttrToken = 0;
        if ((mPushType != WapPushParser.SI_TYPE) && (mPushType != WapPushParser.SL_TYPE))
            throw new SAXException("Error: unsupport Push Type");

        while (nextOperation != OPERATION_PARSE_END) {
            switch(nextOperation) {
                case OPERATION_PARSE_HEAD:
                    nextOperation = parseHead();
                    break;

                case OPERATION_PARSE_ATTR:
                    nextOperation = parseAttr();
                    break;

                case OPERATION_PARSE_CONTENT:
                    nextOperation = parseContent();
                    break;

                default:
                    break;
            }
        }

    }

    public String getAction() {
        if (mAction != null && (!mAction.isEmpty()))
            return mAction;
        return null;
    }

    public String getContent() {
        if (mContent != null && (!mContent.isEmpty()))
            return mContent;
        return null;
    }


    public String getHyperLink() {
        if (mHyperLink!= null && (!mHyperLink.isEmpty()))
            return mHyperLink;
        return null;
    }

/*----
    public static void test(Context ctx) {

        FileInputStream fis = null;

        try {
            fis = ctx.openFileInput("test_si.dat");
        } catch (Exception e) {
            Log.e(TAG, "open si data failed");
        }
        byte[] buff = new byte[256];
        int length = 0;

        try {
            length = fis.read(buff);
            Log.e(TAG, "test data length is " + length);
        } catch (Exception e) {
        Log.e(TAG, "read si data failed");
        }
        ByteArrayInputStream bais = new ByteArrayInputStream(buff, 0, length);
        WapPushParser testParser = new WapPushParser();

        try {
            testParser.parse(bais, WapPushParser.SI_TYPE);
            Log.e(TAG, "action is " + testParser.getAction() + " HyperLink is "
                   + testParser.getHyperLink() + " Content is " + testParser.getContent());
        } catch (Exception e) {
            Log.e(TAG, "parse si data failed");
        }
    }

    --*/


}
