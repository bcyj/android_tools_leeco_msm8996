/** 
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2008 The Android Open Source Project
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

package com.wdstechnology.android.kryten.parser;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.util.Hashtable;
import java.util.Vector;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

public class WbxmlParser implements XmlPullParser {

    private InputStream mIn;
    private int TAG_TABLE = 0;
    private int ATTR_START_TABLE = 1;
    private int ATTR_VALUE_TABLE = 2;
    private String[] mAttrStartTable;
    private String[] mAttrValueTable;
    private String[] mTagTable;
    private byte[] mStringTable;
    private Hashtable mCacheTable = null;
    private boolean mProcessNsp;
    private int mDepth;
    private String[] mElementStack = new String[16];
    private String[] mNspStack = new String[8];
    private int[] mNspCounts = new int[4];
    private int mAttributeCount;
    private String[] mAttributes = new String[16];
    private int mNextId = -2;
    private Vector mTables = new Vector();
    private int mVersion;
    private int mPublicIdentifierId;
    private String mPrefix;
    private String mNamespace;
    private String mName;
    private String mText;
    private Object mWapExtensionData;
    private int mWapCode;
    private int mType;
    private boolean mDegenerated;
    private boolean isWhitespace;
    private String mEncoding;

    public boolean getFeature(String paramString) {
        if ("http://xmlpull.org/v1/doc/features.html#process-namespaces"
                .equals(paramString))
            return this.mProcessNsp;
        return false;
    }

    public String getInputEncoding() {
        return this.mEncoding;
    }

    public void defineEntityReplacementText(String paramString1,
            String paramString2) throws XmlPullParserException {
    }

    public Object getProperty(String paramString) {
        return null;
    }

    public int getNamespaceCount(int paramInt) {
        if (paramInt > this.mDepth)
            throw new IndexOutOfBoundsException();
        return this.mNspCounts[paramInt];
    }

    public String getNamespacePrefix(int paramInt) {
        return this.mNspStack[(paramInt << 1)];
    }

    public String getNamespaceUri(int paramInt) {
        return this.mNspStack[((paramInt << 1) + 1)];
    }

    public String getNamespace(String paramString) {
        if ("xml".equals(paramString))
            return "http://www.w3.org/XML/1998/mNamespace";
        if ("xmlns".equals(paramString))
            return "http://www.w3.org/2000/xmlns/";
        for (int i = (getNamespaceCount(this.mDepth) << 1) - 2; i >= 0; i -= 2)
            if (paramString == null) {
                if (this.mNspStack[i] == null)
                    return this.mNspStack[(i + 1)];
            } else if (paramString.equals(this.mNspStack[i]))
                return this.mNspStack[(i + 1)];
        return null;
    }

    public int getDepth() {
        return this.mDepth;
    }

    public String getPositionDescription() {
        StringBuffer localStringBuffer = new StringBuffer(
                this.mType < XmlPullParser.TYPES.length ? XmlPullParser.TYPES[this.mType]
                        : "unknown");
        localStringBuffer.append(' ');
        if ((this.mType == 2) || (this.mType == 3)) {
            if (this.mDegenerated)
                localStringBuffer.append("(empty) ");
            localStringBuffer.append('<');
            if (this.mType == 3)
                localStringBuffer.append('/');
            if (this.mPrefix != null)
                localStringBuffer.append("{" + this.mNamespace + "}"
                        + this.mPrefix + ":");
            localStringBuffer.append(this.mName);
            int i = this.mAttributeCount << 2;
            for (int j = 0; j < i; j += 4) {
                localStringBuffer.append(' ');
                if (this.mAttributes[(j + 1)] != null)
                    localStringBuffer.append("{" + this.mAttributes[j] + "}"
                            + this.mAttributes[(j + 1)] + ":");
                localStringBuffer.append(this.mAttributes[(j + 2)] + "='"
                        + this.mAttributes[(j + 3)] + "'");
            }
            localStringBuffer.append('>');
        } else if (this.mType != 7) {
            if (this.mType != 4) {
                localStringBuffer.append(getText());
            } else if (this.isWhitespace) {
                localStringBuffer.append("(whitespace)");
            } else {
                String str = getText();
                if (str.length() > 16)
                    str = str.substring(0, 16) + "...";
                localStringBuffer.append(str);
            }
        }
        return localStringBuffer.toString();
    }

    public int getLineNumber() {
        return -1;
    }

    public int getColumnNumber() {
        return -1;
    }

    public boolean isWhitespace() throws XmlPullParserException {
        if ((this.mType != 4) && (this.mType != 7) && (this.mType != 5))
            exception("Wrong event mType");
        return this.isWhitespace;
    }

    public String getText() {
        return this.mText;
    }

    public char[] getTextCharacters(int[] paramArrayOfInt) {
        if (this.mType >= 4) {
            paramArrayOfInt[0] = 0;
            paramArrayOfInt[1] = this.mText.length();
            char[] arrayOfChar = new char[this.mText.length()];
            this.mText.getChars(0, this.mText.length(), arrayOfChar, 0);
            return arrayOfChar;
        }
        paramArrayOfInt[0] = -1;
        paramArrayOfInt[1] = -1;
        return null;
    }

    public String getNamespace() {
        return this.mNamespace;
    }

    public String getName() {
        return this.mName;
    }

    public String getPrefix() {
        return this.mPrefix;
    }

    public boolean isEmptyElementTag() throws XmlPullParserException {
        if (this.mType != 2)
            exception("Wrong event mType");
        return this.mDegenerated;
    }

    public int getAttributeCount() {
        return this.mAttributeCount;
    }

    public String getAttributeType(int paramInt) {
        return "CDATA";
    }

    public boolean isAttributeDefault(int paramInt) {
        return false;
    }

    public String getAttributeNamespace(int paramInt) {
        if (paramInt >= this.mAttributeCount)
            throw new IndexOutOfBoundsException();
        return this.mAttributes[(paramInt << 2)];
    }

    public String getAttributeName(int paramInt) {
        if (paramInt >= this.mAttributeCount)
            throw new IndexOutOfBoundsException();
        return this.mAttributes[((paramInt << 2) + 2)];
    }

    public String getAttributePrefix(int paramInt) {
        if (paramInt >= this.mAttributeCount)
            throw new IndexOutOfBoundsException();
        return this.mAttributes[((paramInt << 2) + 1)];
    }

    public String getAttributeValue(int paramInt) {
        if (paramInt >= this.mAttributeCount)
            throw new IndexOutOfBoundsException();
        return this.mAttributes[((paramInt << 2) + 3)];
    }

    public String getAttributeValue(String paramString1, String paramString2) {
        for (int i = (this.mAttributeCount << 2) - 4; i >= 0; i -= 4)
            if ((this.mAttributes[(i + 2)].equals(paramString2))
                    && ((paramString1 == null) || (this.mAttributes[i]
                            .equals(paramString1))))
                return this.mAttributes[(i + 3)];
        return null;
    }

    public int getEventType() throws XmlPullParserException {
        return this.mType;
    }

    public int next() throws XmlPullParserException, IOException {
        this.isWhitespace = true;
        int i = 9999;
        while (true) {
            String str = this.mText;
            nextImpl();
            if (this.mType < i)
                i = this.mType;
            if (i <= 5) {
                if (i < 4)
                    break;
                if (str != null)
                    this.mText = (str + this.mText);
                switch (peekId()) {
                    case 2:
                    case 3:
                    case 4:
                    case 68:
                    case 131:
                    case 132:
                    case 196:
                }
            }
        }
        this.mType = i;
        if (this.mType > 4)
            this.mType = 4;
        return this.mType;
    }

    public int nextToken() throws XmlPullParserException, IOException {
        this.isWhitespace = true;
        nextImpl();
        return this.mType;
    }

    public int nextTag() throws XmlPullParserException, IOException {
        next();
        if ((this.mType == 4) && (this.isWhitespace))
            next();
        if ((this.mType != 3) && (this.mType != 2))
            exception("unexpected mType");
        return this.mType;
    }

    public String nextText() throws XmlPullParserException, IOException {
        if (this.mType != 2)
            exception("precondition: START_TAG");
        next();
        String str;
        if (this.mType == 4) {
            str = getText();
            next();
        } else {
            str = "";
        }
        if (this.mType != 3)
            exception("END_TAG expected");
        return str;
    }

    public void require(int paramInt, String paramString1, String paramString2)
            throws XmlPullParserException, IOException {
        if ((paramInt != this.mType)
                || ((paramString1 != null) && (!paramString1
                        .equals(getNamespace())))
                || ((paramString2 != null) && (!paramString2.equals(getName()))))
            exception("expected: "
                    + (paramInt == 64 ? "WAP Ext." : new StringBuffer()
                            .append(XmlPullParser.TYPES[paramInt]).append(" {")
                            .append(paramString1).append("}")
                            .append(paramString2).toString()));
    }

    public void setInput(Reader paramReader) throws XmlPullParserException {
        exception("InputStream required");
    }

    public void setInput(InputStream paramInputStream, String paramString)
            throws XmlPullParserException {
        this.mIn = paramInputStream;
        try {
            this.mVersion = readByte();
            this.mPublicIdentifierId = readInt();
            if (this.mPublicIdentifierId == 0)
                readInt();
            int i = readInt();
            if (null == paramString)
                switch (i) {
                    case 4:
                        this.mEncoding = "ISO-8859-1";
                        break;
                    case 106:
                        this.mEncoding = "UTF-8";
                        break;
                    default:
                        throw new UnsupportedEncodingException("" + i);
                }
            else
                this.mEncoding = paramString;
            int j = readInt();
            this.mStringTable = new byte[j];
            int k = 0;
            while (k < j) {
                int m = paramInputStream.read(this.mStringTable, k, j - k);
                if (m <= 0)
                    break;
                k += m;
            }
            selectPage(0, true);
            selectPage(0, false);
        } catch (IOException localIOException) {
            exception("Illegal input format");
        }
    }

    public void setFeature(String paramString, boolean paramBoolean)
            throws XmlPullParserException {
        if ("http://xmlpull.org/v1/doc/features.html#process-namespaces"
                .equals(paramString))
            this.mProcessNsp = paramBoolean;
        else
            exception("unsupported feature: " + paramString);
    }

    public void setProperty(String paramString, Object paramObject)
            throws XmlPullParserException {
        throw new XmlPullParserException("unsupported property: " + paramString);
    }

    private final boolean adjustNsp() throws XmlPullParserException {
        boolean bool = false;
        String str1;
        int j;
        String str2;
        for (int i = 0; i < this.mAttributeCount << 2; i += 4) {
            str1 = this.mAttributes[(i + 2)];
            j = str1.indexOf(':');
            if (j != -1) {
                str2 = str1.substring(0, j);
                str1 = str1.substring(j + 1);
            } else {
                if (!str1.equals("xmlns"))
                    continue;
                str2 = str1;
                str1 = null;
            }
            if (!str2.equals("xmlns")) {
                bool = true;
            } else {
                int tmp95_92 = this.mDepth;
                int[] tmp95_88 = this.mNspCounts;
                int tmp97_96 = tmp95_88[tmp95_92];
                tmp95_88[tmp95_92] = (tmp97_96 + 1);
                int k = tmp97_96 << 1;
                this.mNspStack = ensureCapacity(this.mNspStack, k + 2);
                this.mNspStack[k] = str1;
                this.mNspStack[(k + 1)] = this.mAttributes[(i + 3)];
                if ((str1 != null) && (this.mAttributes[(i + 3)].equals("")))
                    exception("illegal empty mNamespace");
                System.arraycopy(this.mAttributes, i + 4, this.mAttributes, i,
                        (--this.mAttributeCount << 2) - i);
                i -= 4;
            }
        }
        if (bool)
            for (int i = (this.mAttributeCount << 2) - 4; i >= 0; i -= 4) {
                str1 = this.mAttributes[(i + 2)];
                j = str1.indexOf(':');
                if (j == 0)
                    throw new RuntimeException("illegal attribute mName: "
                            + str1 + " at " + this);
                if (j != -1) {
                    str2 = str1.substring(0, j);
                    str1 = str1.substring(j + 1);
                    String str3 = getNamespace(str2);
                    if (str3 == null)
                        throw new RuntimeException("Undefined Prefix: " + str2
                                + " mIn " + this);
                    this.mAttributes[i] = str3;
                    this.mAttributes[(i + 1)] = str2;
                    this.mAttributes[(i + 2)] = str1;
                    for (int m = (this.mAttributeCount << 2) - 4; m > i; m -= 4)
                        if ((str1.equals(this.mAttributes[(m + 2)]))
                                && (str3.equals(this.mAttributes[m])))
                            exception("Duplicate Attribute: {" + str3 + "}"
                                    + str1);
                }
            }
        int i = this.mName.indexOf(':');
        if (i == 0) {
            exception("illegal tag mName: " + this.mName);
        } else if (i != -1) {
            this.mPrefix = this.mName.substring(0, i);
            this.mName = this.mName.substring(i + 1);
        }
        this.mNamespace = getNamespace(this.mPrefix);
        if (this.mNamespace == null) {
            if (this.mPrefix != null)
                exception("undefined mPrefix: " + this.mPrefix);
            this.mNamespace = "";
        }
        return bool;
    }

    private final void setTable(int paramInt1, int paramInt2,
            String[] paramArrayOfString) {
        if (this.mStringTable != null)
            throw new RuntimeException(
                    "setXxxTable must be called before setInput!");
        while (this.mTables.size() < 3 * paramInt1 + 3)
            this.mTables.addElement(null);
        this.mTables.setElementAt(paramArrayOfString, paramInt1 * 3 + paramInt2);
    }

    private final void exception(String paramString)
            throws XmlPullParserException {
        throw new XmlPullParserException(paramString, this, null);
    }

    private void selectPage(int paramInt, boolean paramBoolean)
            throws XmlPullParserException {
        if ((this.mTables.size() == 0) && (paramInt == 0))
            return;
        if (paramInt * 3 > this.mTables.size())
            exception("Code Page " + paramInt + " undefined!");
        if (paramBoolean) {
            this.mTagTable = ((String[]) this.mTables.elementAt(paramInt * 3
                    + this.TAG_TABLE));
        } else {
            this.mAttrStartTable = ((String[]) this.mTables.elementAt(paramInt
                    * 3 + this.ATTR_START_TABLE));
            this.mAttrValueTable = ((String[]) this.mTables.elementAt(paramInt
                    * 3 + this.ATTR_VALUE_TABLE));
        }
    }

    private final void nextImpl() throws IOException, XmlPullParserException {
        if (this.mType == 3)
            this.mDepth -= 1;
        if (this.mDegenerated) {
            this.mType = 3;
            this.mDegenerated = false;
            return;
        }
        this.mText = null;
        this.mPrefix = null;
        this.mName = null;
        int i;
        for (i = peekId(); i == 0; i = peekId()) {
            this.mNextId = -2;
            selectPage(readByte(), true);
        }
        this.mNextId = -2;
        int j;
        switch (i) {
            case -1:
                this.mType = 1;
                break;
            case 1:
                j = this.mDepth - 1 << 2;
                this.mType = 3;
                this.mNamespace = this.mElementStack[j];
                this.mPrefix = this.mElementStack[(j + 1)];
                this.mName = this.mElementStack[(j + 2)];
                break;
            case 2:
                this.mType = 6;
                j = (char) readInt();
                this.mText = ("" + j);
                this.mName = ("#" + j);
                break;
            case 3:
                this.mType = 4;
                this.mText = readStrI();
                break;
            case 64:
            case 65:
            case 66:
            case 128:
            case 129:
            case 130:
            case 192:
            case 193:
            case 194:
            case 195:
                this.mType = 64;
                this.mWapCode = i;
                this.mWapExtensionData = parseWapExtension(i);
                break;
            case 67:
                throw new RuntimeException("PI curr. not supp.");
            case 131:
                this.mType = 4;
                this.mText = readStrT();
                break;
            default:
                parseElement(i);
        }
    }

    public Object parseWapExtension(int paramInt) throws IOException,
            XmlPullParserException {
        switch (paramInt) {
            case 64:
            case 65:
            case 66:
                return readStrI();
            case 128:
            case 129:
            case 130:
                return new Integer(readInt());
            case 192:
            case 193:
            case 194:
                return null;
            case 195:
                int i = readInt();
                byte[] arrayOfByte = new byte[i];
                while (i > 0)
                    i -= this.mIn.read(arrayOfByte, arrayOfByte.length - i, i);
                return arrayOfByte;
        }
        exception("illegal id: " + paramInt);
        return null;
    }

    public void readAttr() throws IOException, XmlPullParserException {
        int i = readByte();
        int j = 0;
        while (i != 1) {
            while (i == 0) {
                selectPage(readByte(), false);
                i = readByte();
            }
            String str = resolveId(this.mAttrStartTable, i);
            int k = str.indexOf('=');
            StringBuffer localStringBuffer;
            if (k == -1) {
                localStringBuffer = new StringBuffer();
            } else {
                localStringBuffer = new StringBuffer(str.substring(k + 1));
                str = str.substring(0, k);
            }
            for (i = readByte(); (i > 128) || (i == 0) || (i == 2) || (i == 3)
                    || (i == 131) || ((i >= 64) && (i <= 66))
                    || ((i >= 128) && (i <= 130)); i = readByte())
                switch (i) {
                    case 0:
                        selectPage(readByte(), false);
                        break;
                    case 2:
                        localStringBuffer.append((char) readInt());
                        break;
                    case 3:
                        localStringBuffer.append(readStrI());
                        break;
                    case 64:
                    case 65:
                    case 66:
                    case 128:
                    case 129:
                    case 130:
                    case 192:
                    case 193:
                    case 194:
                    case 195:
                        localStringBuffer.append(resolveWapExtension(i,
                                parseWapExtension(i)));
                        break;
                    case 131:
                        localStringBuffer.append(readStrT());
                        break;
                    default:
                        localStringBuffer.append(resolveId(this.mAttrValueTable, i));
                }
            this.mAttributes = ensureCapacity(this.mAttributes, j + 4);
            this.mAttributes[(j++)] = "";
            this.mAttributes[(j++)] = null;
            this.mAttributes[(j++)] = str;
            this.mAttributes[(j++)] = localStringBuffer.toString();
            this.mAttributeCount += 1;
        }
    }

    private int peekId() throws IOException {
        if (this.mNextId == -2)
            this.mNextId = this.mIn.read();
        return this.mNextId;
    }

    protected String resolveWapExtension(int paramInt, Object paramObject) {
        if ((paramObject instanceof byte[])) {
            StringBuffer localStringBuffer = new StringBuffer();
            byte[] arrayOfByte = (byte[]) paramObject;
            for (int i = 0; i < arrayOfByte.length; i++) {
                localStringBuffer.append("0123456789abcdef"
                        .charAt(arrayOfByte[i] >> 4 & 0xF));
                localStringBuffer.append("0123456789abcdef"
                        .charAt(arrayOfByte[i] & 0xF));
            }
            return localStringBuffer.toString();
        }
        return "$(" + paramObject + ")";
    }

    String resolveId(String[] paramArrayOfString, int paramInt)
            throws IOException {
        int i = (paramInt & 0x7F) - 5;
        if (i == -1) {
            this.mWapCode = -1;
            return readStrT();
        }
        if ((i < 0) || (paramArrayOfString == null)
                || (i >= paramArrayOfString.length)
                || (paramArrayOfString[i] == null))
            throw new IOException("id " + paramInt + " undef.");
        this.mWapCode = (i + 5);
        return paramArrayOfString[i];
    }

    void parseElement(int paramInt) throws IOException, XmlPullParserException {
        this.mType = 2;
        this.mName = resolveId(this.mTagTable, paramInt & 0x3F);
        this.mAttributeCount = 0;
        if ((paramInt & 0x80) != 0)
            readAttr();
        this.mDegenerated = ((paramInt & 0x40) == 0);
        int i = this.mDepth++ << 2;
        this.mElementStack = ensureCapacity(this.mElementStack, i + 4);
        this.mElementStack[(i + 3)] = this.mName;
        if (this.mDepth >= this.mNspCounts.length) {
            int[] arrayOfInt = new int[this.mDepth + 4];
            System.arraycopy(this.mNspCounts, 0, arrayOfInt, 0,
                    this.mNspCounts.length);
            this.mNspCounts = arrayOfInt;
        }
        this.mNspCounts[this.mDepth] = this.mNspCounts[(this.mDepth - 1)];
        for (int j = this.mAttributeCount - 1; j > 0; j--)
            for (int k = 0; k < j; k++)
                if (getAttributeName(j).equals(getAttributeName(k)))
                    exception("Duplicate Attribute: " + getAttributeName(j));
        if (this.mProcessNsp)
            adjustNsp();
        else
            this.mNamespace = "";
        this.mElementStack[i] = this.mNamespace;
        this.mElementStack[(i + 1)] = this.mPrefix;
        this.mElementStack[(i + 2)] = this.mName;
    }

    private final String[] ensureCapacity(String[] paramArrayOfString,
            int paramInt) {
        if (paramArrayOfString.length >= paramInt)
            return paramArrayOfString;
        String[] arrayOfString = new String[paramInt + 16];
        System.arraycopy(paramArrayOfString, 0, arrayOfString, 0,
                paramArrayOfString.length);
        return arrayOfString;
    }

    int readByte() throws IOException {
        int i = this.mIn.read();
        if (i == -1)
            throw new IOException("Unexpected EOF");
        return i;
    }

    int readInt() throws IOException {
        int i = 0;
        int j;
        do {
            j = readByte();
            i = i << 7 | j & 0x7F;
        } while ((j & 0x80) != 0);
        return i;
    }

    String readStrI() throws IOException {
        ByteArrayOutputStream localByteArrayOutputStream = new ByteArrayOutputStream();
        boolean bool = true;
        while (true) {
            int i = this.mIn.read();
            if (i == 0)
                break;
            if (i == -1)
                throw new IOException("Unexpected EOF");
            if (i > 32)
                bool = false;
            localByteArrayOutputStream.write(i);
        }
        this.isWhitespace = bool;
        String str = new String(localByteArrayOutputStream.toByteArray(),
                this.mEncoding);
        localByteArrayOutputStream.close();
        return str;
    }

    String readStrT() throws IOException {
        int i = readInt();
        if (this.mCacheTable == null)
            this.mCacheTable = new Hashtable();
        String str = (String) this.mCacheTable.get(new Integer(i));
        if (str == null) {
            int j;
            for (j = i; (j < this.mStringTable.length)
                    && (this.mStringTable[j] != 0); j++)
                ;
            str = new String(this.mStringTable, i, j - i, this.mEncoding);
            this.mCacheTable.put(new Integer(i), str);
        }
        return str;
    }

    public void setTagTable(int paramInt, String[] paramArrayOfString) {
        setTable(paramInt, this.TAG_TABLE, paramArrayOfString);
    }

    public void setAttrStartTable(int paramInt, String[] paramArrayOfString) {
        setTable(paramInt, this.ATTR_START_TABLE, paramArrayOfString);
    }

    public void setAttrValueTable(int paramInt, String[] paramArrayOfString) {
        setTable(paramInt, this.ATTR_VALUE_TABLE, paramArrayOfString);
    }

    public int getWapCode() {
        return this.mWapCode;
    }

    public Object getWapExtensionData() {
        return this.mWapExtensionData;
    }
}
