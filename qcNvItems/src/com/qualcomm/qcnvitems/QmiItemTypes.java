/******************************************************************************
 * @file    QmiItemTypes.java
 * @brief   Contains Java QMI type definitions mirroring structures used in the modem.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcnvitems;

import com.qualcomm.qcnvitems.BaseQmiTypes.*;

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;

public class QmiItemTypes {

    public static final String LOG_TAG = "QmiItemTypes";

    // size of basic data types
    public static final int SIZE_OF_BYTE = 1;

    public static final int SIZE_OF_SHORT = 2;

    public static final int SIZE_OF_INT = 4;

    public static final int SIZE_OF_LONG = 8;

    // table of field types for single-parameter QMI messages
    public static class FieldTypeValues {
        public static final short AKEY_TYPE = 0x01;

        public static final short MOB_CAI_REV_TYPE = 0x01;

        public static final short RTRE_CONFIG_TYPE = 0x10;
    }

    // null byte stream
    public static class QmiNull extends BaseQmiItemType {
        QmiNull() {
        }

        @Override
        public int getSize() {
            return 0;
        }

        @Override
        public byte[] toByteArray() {
            return new byte[0];
        }

        @Override
        public String toString() {
            return "val=null";
        }

        @Override
        /*
         * Overrides toTlv() method in BaseQmiItemType which normally includes
         * both tlv size and length with the actual data. This version of
         * toTlv() corresponds to a completely null tlv (null bytestream)
         * instead of an empty tlv (type and length present but no payload).
         */
        public byte[] toTlv(short type) {
            return new byte[0];
        }
    }

    // some primitive types
    public static class QmiByte extends BaseQmiItemType {
        private byte mVal;

        QmiByte() {
            this.mVal = 0;
        }

        QmiByte(short val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseByte(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        QmiByte(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < SIZE_OF_BYTE) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.get();
        }

        public short toShort() {
            return PrimitiveParser.toUnsigned(mVal);
        }

        public int getSize() {
            return SIZE_OF_BYTE;
        }

        public String toString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mVal);
            return buf.array();
        }
    }

    public static class QmiShort extends BaseQmiItemType {
        private short mVal;

        QmiShort() {
            this.mVal = 0;
        }

        QmiShort(int val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseShort(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        QmiShort(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < SIZE_OF_SHORT) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.getShort();
        }

        public int toInt() {
            return PrimitiveParser.toUnsigned(mVal);
        }

        public int getSize() {
            return SIZE_OF_SHORT;
        }

        public String toString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putShort(mVal);
            return buf.array();
        }
    }

    public static class QmiInteger extends BaseQmiItemType {
        private int mVal;

        QmiInteger() {
            this.mVal = 0;
        }

        QmiInteger(long val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseInt(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        QmiInteger(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < SIZE_OF_INT) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.getInt();
        }

        public long toLong() {
            return PrimitiveParser.toUnsigned(mVal);
        }

        public int getSize() {
            return SIZE_OF_INT;
        }

        public String toString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putInt(mVal);
            return buf.array();
        }
    }

    public static class QmiLong extends BaseQmiItemType {
        private long mVal;

        public QmiLong() {
            this.mVal = 0;
        }

        public QmiLong(String mVal) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseLong(mVal);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiLong(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < SIZE_OF_LONG) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.getLong();
        }

        public String toStringValue() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.order(ByteOrder.BIG_ENDIAN); // BigInteger takes a big-endian
            // byte array
            buf.putLong(mVal);
            return new BigInteger(1, buf.array()).toString();
        }

        @Override
        public int getSize() {
            return SIZE_OF_LONG;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putLong(mVal);
            return buf.array();
        }

        @Override
        public String toString() {
            return "val=" + mVal;
        }
    }

    /*
     * String implementation for dynamic-length QMI strings. The outputted byte
     * stream contains both the String length and the actual string in ASCII.
     */

    public static class QmiString extends BaseQmiItemType {
        private String mVal;

        public static final int LENGTH_SIZE = 1;

        public QmiString() {
            this.mVal = new String();
        }

        public QmiString(String mVal) throws InvalidParameterException {
            if (mVal.length() > 255) {
                throw new InvalidParameterException();
            }
            this.mVal = mVal;
        }

        public QmiString(byte[] bArray) throws InvalidParameterException {
            ByteBuffer buf = createByteBuffer(bArray);
            byte length = buf.get();
            byte[] b = new byte[length];
            for (int i = 0; i < length; i++) {
                b[i] = buf.get();
            }
            try {
                mVal = new String(b, QMI_CHARSET);
            } catch (UnsupportedEncodingException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public int length() {
            return mVal.length();
        }

        public String toStringValue() {
            return mVal;
        }

        @Override
        public int getSize() {
            return LENGTH_SIZE + mVal.length();
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(PrimitiveParser.parseByte((short) length()));
            for (int i = 0; i < mVal.length(); i++) {
                buf.put((byte) mVal.charAt(i));
            }
            return buf.array();
        }

        @Override
        public String toString() {
            return "val=" + mVal;
        }
    }

    // structures
    public static class SidNid extends BaseQmiItemType {
        private short numInstances;

        private int[] sid, nid;

        public static final int NUM_INSTANCES_SIZE = 1;

        public static final int SID_SIZE = 2;

        public static final int NID_SIZE = 2;

        public SidNid() {
            this.numInstances = 0;
        }

        public SidNid(int sid, int nid) {
            this((short) 1, new int[] {
                sid
            }, new int[] {
                nid
            });
        }

        public SidNid(int[] sid, int[] nid) throws InvalidParameterException {
            this((short) sid.length, sid, nid);
        }

        public SidNid(short numInstances, int[] sid, int[] nid) throws InvalidParameterException {
            setSidNid(numInstances, sid, nid);
        }

        public SidNid(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < NUM_INSTANCES_SIZE) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.numInstances = PrimitiveParser.toUnsigned(buf.get());
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            for (int i = 0; i < numInstances; i++) {
                this.sid[i] = PrimitiveParser.toUnsigned(buf.getShort());
                this.nid[i] = PrimitiveParser.toUnsigned(buf.getShort());
            }
        }

        public void setNumInstances(short numInstances) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(numInstances);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.numInstances = numInstances;
        }

        public int getNumInstances() {
            return numInstances;
        }

        public int getSid() {
            return getSid(0);
        }

        public int getSid(int index) {
            return sid[index];
        }

        public void setSid(int sid) throws InvalidParameterException {
            try{
                setSid(sid, 0);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public void setSid(int sid, int index) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(sid);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.sid[index] = sid;
        }

        public int getNid() {
            return getNid(0);
        }

        public int getNid(int index) {
            return nid[index];
        }

        public void setNid(int nid) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(nid);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            setNid(nid, 0);
        }

        public void setNid(int nid, int index) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(nid);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.nid[index] = nid;
        }

        public void setSidNid(short numInstances, int[] sid, int[] nid)
                throws InvalidParameterException {
            if (sid.length != nid.length || sid.length != numInstances) {
                throw new InvalidParameterException();
            }
            try {
                PrimitiveParser.checkByte(numInstances);
                for (int i = 0; i < numInstances; i++) {
                    PrimitiveParser.checkShort(sid[i]);
                    PrimitiveParser.checkShort(nid[i]);
                }
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.numInstances = numInstances;
            this.sid = sid;
            this.nid = nid;
        }

        public void setSidNid(int[] sid, int[] nid) throws InvalidParameterException {
            setSidNid((short) sid.length, sid, nid);
        }

        @Override
        public int getSize() {
            return NUM_INSTANCES_SIZE + numInstances * (SID_SIZE + NID_SIZE);
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(PrimitiveParser.parseByte(numInstances));
            for (int i = 0; i < numInstances; i++) {
                buf.putShort(PrimitiveParser.parseShort(sid[i]));
                buf.putShort(PrimitiveParser.parseShort(nid[i]));
            }
            return buf.array();
        }

        @Override
        public String toString() {
            String temp = String.format("num_instances=%d", numInstances);
            for (int i = 0; i < numInstances; i++) {
                temp += String.format(", sid[%d]=%d, nid[%d]=%d", i, sid[i], i, nid[i]);
            }
            return temp;
        }
    }

    public static class MinImsi extends BaseQmiItemType {
        protected String mcc, imsi11_12, imsiS1, imsiS2;

        public static final int MCC_SIZE = 3;

        public static final int IMSI11_12_SIZE = 2;

        public static final int IMSI_S1_SIZE = 7;

        public static final int IMSI_S2_SIZE = 3;

        public MinImsi() throws InvalidParameterException {
            setMcc("000");
            setImsi11_12("00");
            setImsiS1("0000000");
            setImsiS2("000");
        }

        public MinImsi(String mcc, String imsi11_12, String imsiS1, String imsiS2)
                throws InvalidParameterException {
            setMcc(mcc);
            setImsi11_12(imsi11_12);
            setImsiS1(imsiS1);
            setImsiS2(imsiS2);
        }

        public MinImsi(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            mcc = "";
            for (int i = 0; i < MCC_SIZE; i++) {
                mcc += Character.toString((char) buf.get());
            }
            imsi11_12 = "";
            for (int i = 0; i < IMSI11_12_SIZE; i++) {
                imsi11_12 += Character.toString((char) buf.get());
            }
            imsiS1 = "";
            for (int i = 0; i < IMSI_S1_SIZE; i++) {
                imsiS1 += Character.toString((char) buf.get());
            }
            imsiS2 = "";
            for (int i = 0; i < IMSI_S2_SIZE; i++) {
                imsiS2 += Character.toString((char) buf.get());
            }
        }

        public String getMcc() {
            return mcc;
        }

        public void setMcc(String mcc) throws InvalidParameterException {
            if (mcc.length() != MCC_SIZE) {
                throw new InvalidParameterException();
            }
            this.mcc = mcc;
        }

        public String getImsi11_12() {
            return imsi11_12;
        }

        public void setImsi11_12(String imsi11_12) throws InvalidParameterException {
            if (imsi11_12.length() != IMSI11_12_SIZE) {
                throw new InvalidParameterException();
            }
            this.imsi11_12 = imsi11_12;
        }

        public String getImsiS1() {
            return imsiS1;
        }

        public void setImsiS1(String imsiS1) throws InvalidParameterException {
            if (imsiS1.length() != IMSI_S1_SIZE) {
                throw new InvalidParameterException();
            }
            this.imsiS1 = imsiS1;
        }

        public String getImsiS2() {
            return imsiS2;
        }

        public void setImsiS2(String imsiS2) throws InvalidParameterException {
            if (imsiS2.length() != IMSI_S2_SIZE) {
                throw new InvalidParameterException();
            }
            this.imsiS2 = imsiS2;
        }

        public String getImsiNumber() {
            return imsiS1 + imsiS2;
        }

        public void setImsiNumber(String phNumber) throws InvalidParameterException {
            if (phNumber.length() != IMSI_S1_SIZE + IMSI_S2_SIZE) {
                throw new InvalidParameterException();
            }
            try {
                setImsiS1(phNumber.substring(0, IMSI_S1_SIZE));
                setImsiS2(phNumber.substring(phNumber.length() - IMSI_S2_SIZE, phNumber.length()));
            } catch (IndexOutOfBoundsException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        @Override
        public int getSize() {
            return MCC_SIZE + IMSI11_12_SIZE + IMSI_S1_SIZE + IMSI_S2_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            for (int i = 0; i < MCC_SIZE; i++) {
                buf.put((byte) mcc.charAt(i));
            }
            for (int i = 0; i < IMSI11_12_SIZE; i++) {
                buf.put((byte) imsi11_12.charAt(i));
            }
            for (int i = 0; i < IMSI_S1_SIZE; i++) {
                buf.put((byte) imsiS1.charAt(i));
            }
            for (int i = 0; i < IMSI_S2_SIZE; i++) {
                buf.put((byte) imsiS2.charAt(i));
            }
            return buf.array();
        }

        @Override
        public String toString() {
            return "mcc=" + mcc + ", imsi11_12=" + imsi11_12 + ", imsiS1=" + imsiS1 + ", imsiS2="
                    + imsiS2;
        }

        // Parsing/Transformation Methods (static)
        public static String minToPhString(int min1Value, short min2Value) {

            try {
                String table[] = {
                        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"
                };

                String phString = "";

                // top 3 digits
                phString += table[min2Value / 100 % 10];
                phString += table[min2Value / 10 % 10];
                phString += table[min2Value % 10];

                // next 3 digits
                int tempValue = min1Value >> 14;
                phString += table[tempValue / 100 % 10];
                phString += table[tempValue / 10 % 10];
                phString += table[tempValue % 10];

                // next 1 digit
                tempValue = min1Value & 0x3FFF;
                tempValue = tempValue >> 10 & 0xF;
                phString += String.valueOf(tempValue == 10 ? 0 : tempValue);

                // last 3 digits
                tempValue = min1Value & 0x3FF;
                phString += table[tempValue / 100 % 10];
                phString += table[tempValue / 10 % 10];
                phString += table[tempValue % 10];

                return phString;
            } catch (Exception e) {
                return "";
            }
        }

        public static int phStringToMin1(String phString) throws InvalidParameterException {
            int table[] = {
                    9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
            };

            if (phString.length() != 10) {
                throw new InvalidParameterException("Invalid phone number");
            }

            char ph[] = phString.toCharArray();

            int min1 = 0;

            int tempValue = 0;
            tempValue = (short) (tempValue * 10 + (table[ph[3] - '0']));
            tempValue = (short) (tempValue * 10 + (table[ph[4] - '0']));
            tempValue = (short) (tempValue * 10 + (table[ph[5] - '0']));
            tempValue = tempValue << 14;
            min1 = tempValue;

            tempValue = ph[6] - '0';
            tempValue = (tempValue == 0 ? 10 : tempValue);
            tempValue = tempValue << 10;
            min1 += tempValue;

            tempValue = 0;
            tempValue = (short) (tempValue * 10 + (table[ph[7] - '0']));
            tempValue = (short) (tempValue * 10 + (table[ph[8] - '0']));
            tempValue = (short) (tempValue * 10 + (table[ph[9] - '0']));

            min1 += tempValue;

            return min1;
        }

        public static short phStringToMin2(String phString) throws InvalidParameterException {

            short table[] = {
                    9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
            };

            if (phString.length() != 10) {
                throw new InvalidParameterException("Invalid phone number");
            }

            char ph[] = phString.toCharArray();

            short min2 = 0;

            min2 = (short) (min2 * 10 + (table[ph[0] - '0']));
            min2 = (short) (min2 * 10 + (table[ph[1] - '0']));
            min2 = (short) (min2 * 10 + (table[ph[2] - '0']));

            return min2;
        }
    }

    public static class TrueImsi extends MinImsi {
        private short imsiAddrNum;

        public static final int IMSI_ADDR_NUM_SIZE = 1;

        public TrueImsi() {
            super();
            imsiAddrNum = 0;
        }

        public TrueImsi(String mcc, String imsi11_12, String imsiS1, String imsiS2,
                short imsiAddrNum) throws InvalidParameterException, InvalidParameterException {
            super(mcc, imsi11_12, imsiS1, imsiS2);
            setImsiAddrNum(imsiAddrNum);
        }

        public TrueImsi(byte[] bArray) throws InvalidParameterException {
            super(bArray);
            int sSize = super.getSize();
            if (bArray.length < sSize + IMSI_ADDR_NUM_SIZE) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            for (int i = 0; i < sSize; i++) {
                buf.get(); // advance to proper position
            }
            this.imsiAddrNum = PrimitiveParser.toUnsigned(buf.get());
        }

        public short getImsiAddrNum() {
            return imsiAddrNum;
        }

        public void setImsiAddrNum(short imsiAddrNum) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(imsiAddrNum);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.imsiAddrNum = imsiAddrNum;
        }

        @Override
        public int getSize() {
            return super.getSize() + IMSI_ADDR_NUM_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            for (int i = 0; i < MCC_SIZE; i++) {
                buf.put((byte) mcc.charAt(i));
            }
            for (int i = 0; i < IMSI11_12_SIZE; i++) {
                buf.put((byte) imsi11_12.charAt(i));
            }
            for (int i = 0; i < IMSI_S1_SIZE; i++) {
                buf.put((byte) imsiS1.charAt(i));
            }
            for (int i = 0; i < IMSI_S2_SIZE; i++) {
                buf.put((byte) imsiS2.charAt(i));
            }
            buf.put(PrimitiveParser.parseByte(imsiAddrNum));
            return buf.array();
        }

        @Override
        public String toString() {
            return super.toString() + String.format(", imsiAddrNum=%d", imsiAddrNum);
        }
    }

    public static class CdmaChannels extends BaseQmiItemType {
        private int primaryA, primaryB, secondaryA, secondaryB;

        public static final int PRIMARY_A_SIZE = 2;

        public static final int PRIMARY_B_SIZE = 2;

        public static final int SECONDARY_A_SIZE = 2;

        public static final int SECONDARY_B_SIZE = 2;

        public CdmaChannels() {

        }

        public CdmaChannels(int primaryA, int primaryB, int secondaryA, int secondaryB)
                throws InvalidParameterException {
            setPrimaryChannelA(primaryA);
            setPrimaryChannelB(primaryB);
            setSecondaryChannelA(secondaryA);
            setSecondaryChannelB(secondaryB);
        }

        public CdmaChannels(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.primaryA = PrimitiveParser.toUnsigned(buf.getShort());
            this.primaryB = PrimitiveParser.toUnsigned(buf.getShort());
            this.secondaryA = PrimitiveParser.toUnsigned(buf.getShort());
            this.secondaryB = PrimitiveParser.toUnsigned(buf.getShort());
        }

        public int getPrimaryChannelA() {
            return primaryA;
        }

        public void setPrimaryChannelA(int primaryA) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(primaryA);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.primaryA = primaryA;
        }

        public int getPrimaryChannelB() {
            return primaryB;
        }

        public void setPrimaryChannelB(int primaryB) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(primaryB);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.primaryB = primaryB;
        }

        public int getSecondaryChannelA() {
            return secondaryA;
        }

        public void setSecondaryChannelA(int secondaryA) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(secondaryA);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.secondaryA = secondaryA;
        }

        public int getSecondaryChannelB() {
            return secondaryB;
        }

        public void setSecondaryChannelB(int secondaryB) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(secondaryB);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.secondaryB = secondaryB;
        }

        @Override
        public int getSize() {
            return PRIMARY_A_SIZE + PRIMARY_B_SIZE + SECONDARY_A_SIZE + SECONDARY_B_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putShort(PrimitiveParser.parseShort(primaryA));
            buf.putShort(PrimitiveParser.parseShort(primaryB));
            buf.putShort(PrimitiveParser.parseShort(secondaryA));
            buf.putShort(PrimitiveParser.parseShort(secondaryB));
            return buf.array();
        }

        @Override
        public String toString() {
            return String.format("primaryA=%d, primaryB=%d, secondaryA=%d, secondaryB=%d",
                    primaryA, primaryB, secondaryA, secondaryB);
        }
    }

    public static class Threegpp2Info extends BaseQmiStructType {
        private boolean inSetMode;

        private QmiString namName, dirNum;

        private SidNid sidNid;

        private MinImsi minImsi;

        private TrueImsi trueImsi;

        private CdmaChannels cdmaChannels;

        public static final short NAM_ID_TYPE = 0x01; // same for get/set

        public static final short GET_NAM_NAME_TYPE = 0x10;

        public static final short GET_DIR_NUM_TYPE = 0x11;

        public static final short GET_SID_NID_TYPE = 0x12;

        public static final short GET_MIN_IMSI_TYPE = 0x13;

        public static final short GET_TRUE_IMSI_TYPE = 0x14;

        public static final short GET_CDMA_CHANNELS_TYPE = 0x15;

        public static final short SET_DIR_NUM_TYPE = 0x10;

        public static final short SET_SID_NID_TYPE = 0x11;

        public static final short SET_MIN_IMSI_TYPE = 0x12;

        public static final short SET_TRUE_IMSI_TYPE = 0x13;

        public static final short SET_CDMA_CHANNELS_TYPE = 0x14;

        public Threegpp2Info(String dirNum, SidNid sidNid, MinImsi minImsi, TrueImsi trueImsi,
                CdmaChannels cdmaChannels) {
            this.dirNum = new QmiString(dirNum);
            this.sidNid = sidNid;
            this.minImsi = minImsi;
            this.trueImsi = trueImsi;
            this.cdmaChannels = cdmaChannels;
            this.inSetMode = true;
        }

        public Threegpp2Info(String namName, String dirNum, SidNid sidNid, MinImsi minImsi,
                TrueImsi trueImsi, CdmaChannels cdmaChannels) {
            this(dirNum, sidNid, minImsi, trueImsi, cdmaChannels);
            this.namName = new QmiString(namName);
            this.inSetMode = false;
        }

        /*
         * Constructor assumes GetMode by default, since most important returned
         * Threegpp2Info structures will be from GET_3GPP2_SUBSCRIPTION_INFO.
         * However, the SET_3GPP2 call also returns the updated info with
         * different Type Values.
         */
        public Threegpp2Info(byte[] qmiMsg) {
            this(qmiMsg, false);
        }

        public Threegpp2Info(byte[] qmiMsg, boolean inSetMode) {
            this.inSetMode = inSetMode;
            ByteBuffer buf = ByteBuffer.wrap(qmiMsg);
            buf.order(BaseQmiItemType.QMI_BYTE_ORDER);
            int size = qmiMsg.length;
            while (size > 0) {
                short type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());
                byte[] data = parseData(buf, length);
                size -= BaseQmiItemType.TLV_TYPE_SIZE + BaseQmiItemType.TLV_LENGTH_SIZE + length;
                if (inSetMode) {
                    switch (type) {
                        case SET_DIR_NUM_TYPE:
                            this.dirNum = new QmiString(data);
                            break;

                        case SET_SID_NID_TYPE:
                            this.sidNid = new SidNid(data);
                            break;

                        case SET_MIN_IMSI_TYPE:
                            this.minImsi = new MinImsi(data);
                            break;

                        case SET_TRUE_IMSI_TYPE:
                            this.trueImsi = new TrueImsi(data);
                            break;

                        case SET_CDMA_CHANNELS_TYPE:
                            this.cdmaChannels = new CdmaChannels(data);
                            break;
                    }

                } else {
                    switch (type) {
                        case GET_NAM_NAME_TYPE:
                            this.namName = new QmiString(data);
                            break;

                        case GET_DIR_NUM_TYPE:
                            this.dirNum = new QmiString(data);
                            break;

                        case GET_SID_NID_TYPE:
                            this.sidNid = new SidNid(data);
                            break;

                        case GET_MIN_IMSI_TYPE:
                            this.minImsi = new MinImsi(data);
                            break;

                        case GET_TRUE_IMSI_TYPE:
                            this.trueImsi = new TrueImsi(data);
                            break;

                        case GET_CDMA_CHANNELS_TYPE:
                            this.cdmaChannels = new CdmaChannels(data);
                            break;
                    }
                }
            }
        }

        public String getNamName() {
            if (inSetMode) {
                return "";
            } else {
                return namName.toStringValue();
            }
        }

        public String getDirNum() {
            return dirNum.toStringValue();
        }

        public void setDirNum(String dirNum) {
            this.inSetMode = true;
            this.dirNum = new QmiString(dirNum);
        }

        public SidNid getSidNid() {
            return sidNid;
        }

        public void setSidNid(SidNid sidNid) {
            this.inSetMode = true;
            this.sidNid = sidNid;
        }

        public MinImsi getMinImsi() {
            return minImsi;
        }

        public void setMinImsi(MinImsi minImsi) {
            this.inSetMode = true;
            this.minImsi = minImsi;
        }

        public TrueImsi getTrueImsi() {
            return trueImsi;
        }

        public void setTrueImsi(TrueImsi trueImsi) {
            this.inSetMode = true;
            this.trueImsi = trueImsi;
        }

        public CdmaChannels getCdmaChannels() {
            return cdmaChannels;
        }

        public void setCdmaChannels(CdmaChannels cdmaChannels) {
            this.inSetMode = true;
            this.cdmaChannels = cdmaChannels;
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return getItems(this.inSetMode);
        }

        public BaseQmiItemType[] getItems(boolean inSetMode) {
            if (inSetMode) {
                return new BaseQmiItemType[] {
                        dirNum, sidNid, minImsi, trueImsi, cdmaChannels
                };
            } else {
                return new BaseQmiItemType[] {
                        namName, dirNum, sidNid, minImsi, trueImsi, cdmaChannels
                };
            }
        }

        @Override
        public short[] getTypes() {
            return getTypes(this.inSetMode);
        }

        public static short[] getTypes(boolean inSetMode) {
            if (inSetMode) {
                return new short[] {
                        SET_DIR_NUM_TYPE, SET_SID_NID_TYPE, SET_MIN_IMSI_TYPE, SET_TRUE_IMSI_TYPE,
                        SET_CDMA_CHANNELS_TYPE
                };
            } else {
                return new short[] {
                        GET_NAM_NAME_TYPE, GET_DIR_NUM_TYPE, GET_SID_NID_TYPE, GET_MIN_IMSI_TYPE,
                        GET_TRUE_IMSI_TYPE, GET_CDMA_CHANNELS_TYPE
                };
            }
        }
    }

    /*
     * Currently only the boolean 'enable' is considered part of this structure
     * as QMI does not support the short 'rings' parameter, which is only
     * included for NV compatibility. This is reflected in the getSize() and
     * toByteArray() functions as well as in the constructor.
     */
    public static class AutoAnswer extends BaseQmiItemType {
        boolean enable;

        short rings;

        public static final short DEFAULT_RINGS = 5;

        public static final int ENABLE_SIZE = 1;

        public static final int RINGS_SIZE = 1;

        public AutoAnswer(boolean enable, short rings) throws InvalidParameterException {
            this.enable = enable;
            if (enable) {
                setRings(rings);
            } else {
                setRings((short)0);
            }
        }

        public AutoAnswer(short rings) {
            this(true, rings);
        }

        public AutoAnswer(boolean enable) {
            this(enable, DEFAULT_RINGS);
        }

        public AutoAnswer(byte[] bArray) throws InvalidParameterException {
            ByteBuffer buf = createByteBuffer(bArray);
            byte enable = buf.get();
            if (enable == 0) {
                this.enable = false;
            } else if (enable == 1) {
                this.enable = true;
            } else {
                throw new InvalidParameterException();
            }
        }

        public boolean isEnabled() {
            return enable;
        }

        public void setEnabled(boolean enable) {
            this.enable = enable;
        }

        public short getRings() {
            return rings;
        }

        public void setRings(short rings) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(rings);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.rings = rings;
        }

        @Override
        public int getSize() {
            return ENABLE_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            if (enable) {
                buf.put((byte) 1);
            } else {
                buf.put((byte) 0);
            }
            return buf.array();
        }

        @Override
        public String toString() {
            return String.format("enable=" + enable + ", rings=%d", rings);
        }
    }

    public static class TimerCount extends BaseQmiItemType {
        private short namId;

        private long timerCount;

        public static final int NAM_ID_SIZE = 1;

        public static final int TIMER_SIZE = 4;

        public TimerCount() {

        }

        public TimerCount(long timerCount, short namId) throws InvalidParameterException {
            setNamId(namId);
            setTimerCount(timerCount);
        }

        public TimerCount(long timerCount) throws InvalidParameterException {
            this(timerCount, BaseQmiTypes.DEFAULT_NAM);
        }

        public TimerCount(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.namId = PrimitiveParser.toUnsigned(buf.get());
            this.timerCount = PrimitiveParser.toUnsigned(buf.getInt());
        }

        public short getNamId() {
            return namId;
        }

        public void setNamId(short namId) {
            PrimitiveParser.checkByte(namId);
            this.namId = namId;
        }

        public long getTimerCount() {
            return timerCount;
        }

        public void setTimerCount(long timerCount) {
            PrimitiveParser.checkInt(timerCount);
            this.timerCount = timerCount;
        }

        @Override
        public int getSize() {
            return NAM_ID_SIZE + TIMER_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(SIZE_OF_LONG);
            buf.put(PrimitiveParser.parseByte(namId));
            buf.putInt(PrimitiveParser.parseInt(timerCount));
            return buf.array();
        }

        @Override
        public String toString() {
            String temp = String.format("namId=%d", namId);
            return temp + ", timerCount=" + timerCount;
        }
    }

    public static class PreferredVoiceSo extends BaseQmiItemType {
        private short namId, evrcCapability;

        private int homePageVoiceSo, homeOrigVoiceSo, roamOrigVoiceSo;

        // ServiceProgramming constants

        public static final int NAM_ID_SIZE = 1;

        public static final int EVRC_CAPABILITY_SIZE = 1;

        public static final int HOME_PAGE_VOICE_SO_SIZE = 2;

        public static final int HOME_ORIG_VOICE_SO_SIZE = 2;

        public static final int ROAM_ORIG_VOICE_SO_SIZE = 2;

        // NV constants

        public final static int VOICE_IS_96_A = 1;

        public final static int VOICE_EVRC = 3;

        public final static int VOICE_13K_IS733 = 17;

        public final static int VOICE_4GV_NARROW_BAND = 68;

        public final static int VOICE_4GV_WIDE_BAND = 70;

        public final static int VOICE_13K = 32768;

        public final static int VOICE_IS_96 = 32769;

        public final static int VOICE_WVRC = 32803;

        public PreferredVoiceSo() {

        }

        public PreferredVoiceSo(short evrcCapability, int homePageVoiceSo, int homeOrigVoiceSo,
                int roamOrigVoiceSo, short namId) throws InvalidParameterException {
            setNamId(namId);
            setEvrcCapability(evrcCapability);
            setHomePageVoiceSo(homePageVoiceSo);
            setHomeOrigVoiceSo(homeOrigVoiceSo);
            setRoamOrigVoiceSo(roamOrigVoiceSo);
        }

        public PreferredVoiceSo(short evrcCapability, int homePageVoiceSo, int homeOrigVoiceSo,
                int roamOrigVoiceSo) throws InvalidParameterException {
            this(evrcCapability, homePageVoiceSo, homeOrigVoiceSo, roamOrigVoiceSo,
                    BaseQmiTypes.DEFAULT_NAM);
        }

        public PreferredVoiceSo(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.namId = PrimitiveParser.toUnsigned(buf.get());
            this.evrcCapability = PrimitiveParser.toUnsigned(buf.get());
            this.homePageVoiceSo = PrimitiveParser.toUnsigned(buf.getShort());
            this.homeOrigVoiceSo = PrimitiveParser.toUnsigned(buf.getShort());
            this.roamOrigVoiceSo = PrimitiveParser.toUnsigned(buf.getShort());
        }

        public short getNamId() {
            return namId;
        }

        public void setNamId(short namId) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(namId);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.namId = namId;
        }

        public short getEvrcCapability() {
            return evrcCapability;
        }

        public void setEvrcCapability(short evrcCapability) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(evrcCapability);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.evrcCapability = evrcCapability;
        }

        public int getHomePageVoiceSo() {
            return homePageVoiceSo;
        }

        public void setHomePageVoiceSo(int homePageVoiceSo) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(homePageVoiceSo);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.homePageVoiceSo = homePageVoiceSo;
        }

        public int getHomeOrigVoiceSo() {
            return homeOrigVoiceSo;
        }

        public void setHomeOrigVoiceSo(int homeOrigVoiceSo) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(homeOrigVoiceSo);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.homeOrigVoiceSo = homeOrigVoiceSo;
        }

        public int getRoamOrigVoiceSo() {
            return roamOrigVoiceSo;
        }

        public void setRoamOrigVoiceSo(int roamOrigVoiceSo) throws InvalidParameterException {
            try {
                PrimitiveParser.checkShort(roamOrigVoiceSo);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.roamOrigVoiceSo = roamOrigVoiceSo;
        }

        @Override
        public int getSize() {
            return NAM_ID_SIZE + EVRC_CAPABILITY_SIZE + HOME_PAGE_VOICE_SO_SIZE
                    + HOME_ORIG_VOICE_SO_SIZE + ROAM_ORIG_VOICE_SO_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(PrimitiveParser.parseByte(namId));
            buf.put(PrimitiveParser.parseByte(evrcCapability));
            buf.putShort(PrimitiveParser.parseShort(homePageVoiceSo));
            buf.putShort(PrimitiveParser.parseShort(homeOrigVoiceSo));
            buf.putShort(PrimitiveParser.parseShort(roamOrigVoiceSo));
            return buf.array();
        }

        @Override
        public String toString() {
            return String
                    .format(
                            "namId=%d, evrcCapability=%d, homePageVoiceSo=%d, homeOrigVoiceSo=%d, roamOrigVoiceSo=%d",
                            namId, evrcCapability, homePageVoiceSo, homeOrigVoiceSo,
                            roamOrigVoiceSo);
        }
    }

    public static class AmrStatus extends BaseQmiItemType {
        private short gsmAmrStatus, wcdmaAmrStatus;

        public static final int GSM_AMR_STATUS_SIZE = 1;

        public static final int WCDMA_AMR_STATUS_SIZE = 1;

        public AmrStatus() {
            this((short)0, (short)0);
        }

        public AmrStatus(short gsmAmrStatus, short wcdmaAmrStatus) throws InvalidParameterException {
            try {
                PrimitiveParser.checkByte(gsmAmrStatus);
                PrimitiveParser.checkByte(wcdmaAmrStatus);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            this.gsmAmrStatus = gsmAmrStatus;
            this.wcdmaAmrStatus = wcdmaAmrStatus;
        }

        public AmrStatus(byte[] bArray) throws InvalidParameterException {
            if (bArray.length < getSize()) {
                throw new InvalidParameterException();
            }
            ByteBuffer buf = createByteBuffer(bArray);
            this.gsmAmrStatus = PrimitiveParser.toUnsigned(buf.get());
            this.wcdmaAmrStatus = PrimitiveParser.toUnsigned(buf.get());
        }

        public short getGsmAmrStatus() {
            return gsmAmrStatus;
        }

        public short getWcdmaAmrStatus() {
            return wcdmaAmrStatus;
        }

        @Override
        public int getSize() {
            return GSM_AMR_STATUS_SIZE + WCDMA_AMR_STATUS_SIZE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(PrimitiveParser.parseByte(gsmAmrStatus));
            buf.put(PrimitiveParser.parseByte(gsmAmrStatus));
            return buf.array();
        }

        @Override
        public String toString() {
            return String
                    .format("gsmAmrStatus=%d, wcdmaAmrStatus=%d", gsmAmrStatus, wcdmaAmrStatus);
        }

    }

    public static class VoiceConfig extends BaseQmiStructType {
        private boolean inSetMode;

        private AutoAnswer autoAnswerStatus;

        private TimerCount airTimerCount, roamTimerCount;

        private QmiByte currentTtyMode;

        private PreferredVoiceSo preferredVoiceSo;

        private AmrStatus amrStatus;

        private QmiByte voicePrivacy;

        public static final short AUTO_ANSWER_STATUS_TYPE = 0x10;

        public static final short AIR_TIMER_TYPE = 0x11;

        public static final short ROAM_TIMER_TYPE = 0x12;

        public static final short CURRENT_TTY_MODE_TYPE = 0x13;

        public static final short PREFERRED_VOICE_SO_TYPE = 0x14;

        public static final short AMR_STATUS_TYPE = 0x15;

        public static final short VOICE_PRIVACY_TYPE = 0x16;

        public VoiceConfig(AutoAnswer autoAnswerStatus, TimerCount airTimerCount,
                TimerCount roamTimerCount, short currentTtyMode, PreferredVoiceSo preferredVoiceSo) {
            this.autoAnswerStatus = autoAnswerStatus;
            this.airTimerCount = airTimerCount;
            this.roamTimerCount = roamTimerCount;
            this.currentTtyMode = new QmiByte(currentTtyMode);
            this.preferredVoiceSo = preferredVoiceSo;
            this.inSetMode = true;
        }

        public VoiceConfig(AutoAnswer autoAnswerStatus, TimerCount airTimerCount,
                TimerCount roamTimerCount, short currentTtyMode, PreferredVoiceSo preferredVoiceSo,
                AmrStatus amrStatus, short voicePrivacy) {
            this(autoAnswerStatus, airTimerCount, roamTimerCount, currentTtyMode, preferredVoiceSo);
            this.amrStatus = amrStatus;
            this.voicePrivacy = new QmiByte(voicePrivacy);
            this.inSetMode = false;
        }

        public VoiceConfig(byte[] qmiMsg) {
            this.inSetMode = true;
            ByteBuffer buf = ByteBuffer.wrap(qmiMsg);
            buf.order(BaseQmiItemType.QMI_BYTE_ORDER);
            int size = qmiMsg.length;
            while (size > 0) {
                short type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());
                byte[] data = parseData(buf, length);
                size -= BaseQmiItemType.TLV_TYPE_SIZE + BaseQmiItemType.TLV_LENGTH_SIZE + length;
                if (inSetMode) {
                    switch (type) {
                        case AUTO_ANSWER_STATUS_TYPE:
                            this.autoAnswerStatus = new AutoAnswer(data);
                            break;

                        case AIR_TIMER_TYPE:
                            this.airTimerCount = new TimerCount(data);
                            break;

                        case ROAM_TIMER_TYPE:
                            this.roamTimerCount = new TimerCount(data);
                            break;

                        case CURRENT_TTY_MODE_TYPE:
                            this.currentTtyMode = new QmiByte(data);
                            break;

                        case PREFERRED_VOICE_SO_TYPE:
                            this.preferredVoiceSo = new PreferredVoiceSo(data);
                            break;

                        case AMR_STATUS_TYPE:
                            this.amrStatus = new AmrStatus(data);
                            this.inSetMode = false;
                            break;

                        case VOICE_PRIVACY_TYPE:
                            this.voicePrivacy = new QmiByte(data);
                            this.inSetMode = false;
                            break;
                    }
                }
            }
        }

        public AutoAnswer getAutoAnswerStatus() {
            return autoAnswerStatus;
        }

        public void setAutoAnswerStatus(AutoAnswer autoAnswerStatus) {
            this.inSetMode = true;
            this.autoAnswerStatus = autoAnswerStatus;
        }

        public TimerCount getAirTimerCount() {
            return airTimerCount;
        }

        public void setAirTimerCount(TimerCount airTimerCount) {
            this.inSetMode = true;
            this.airTimerCount = airTimerCount;
        }

        public TimerCount getRoamTimerCount() {
            return roamTimerCount;
        }

        public void setRoamTimerCount(TimerCount roamTimerCount) {
            this.inSetMode = true;
            this.roamTimerCount = roamTimerCount;
        }

        public short getCurrentTtyMode() {
            return currentTtyMode.toShort();
        }

        public void setCurrentTtyMode(short currentTtyMode) {
            this.inSetMode = true;
            this.currentTtyMode = new QmiByte(currentTtyMode);
        }

        public PreferredVoiceSo getPreferredVoiceSo() {
            return preferredVoiceSo;
        }

        public void setPreferredVoiceSo(PreferredVoiceSo preferredVoiceSo) {
            this.inSetMode = true;
            this.preferredVoiceSo = preferredVoiceSo;
        }

        public AmrStatus getAmrStatus() {
            if (inSetMode) {
                return new AmrStatus();
            } else {
                return amrStatus;
            }
        }

        public short getVoicePrivacy() {
            if (inSetMode) {
                return 0;
            }
            return voicePrivacy.toShort();
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return getItems(this.inSetMode);
        }

        public BaseQmiItemType[] getItems(boolean inSetMode) {
            if (inSetMode) {
                return new BaseQmiItemType[] {
                        autoAnswerStatus, airTimerCount, roamTimerCount, currentTtyMode,
                        preferredVoiceSo
                };
            } else {
                return new BaseQmiItemType[] {
                        autoAnswerStatus, airTimerCount, roamTimerCount, currentTtyMode,
                        preferredVoiceSo, amrStatus, voicePrivacy
                };
            }
        }

        @Override
        public short[] getTypes() {
            return getTypes(this.inSetMode);
        }

        public static short[] getTypes(boolean inSetMode) {
            if (inSetMode) {
                return new short[] {
                        AUTO_ANSWER_STATUS_TYPE, AIR_TIMER_TYPE, ROAM_TIMER_TYPE,
                        CURRENT_TTY_MODE_TYPE, PREFERRED_VOICE_SO_TYPE
                };
            } else {
                return new short[] {
                        AUTO_ANSWER_STATUS_TYPE, AIR_TIMER_TYPE, ROAM_TIMER_TYPE,
                        CURRENT_TTY_MODE_TYPE, PREFERRED_VOICE_SO_TYPE, AMR_STATUS_TYPE,
                        VOICE_PRIVACY_TYPE
                };
            }
        }

        public static BaseQmiItemType[] generateRequest() {
            return new BaseQmiItemType[] {
                    new QmiByte((short) 1), new QmiByte((short) 1), new QmiByte((short) 1),
                    new QmiByte((short) 1), new QmiByte((short) 1), new QmiByte((short) 1),
                    new QmiByte((short) 1)
            };
        }
    }
}
