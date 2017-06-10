/*
 * Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;

import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;

public class QmiPrimitiveTypes {

    private static final String LOG_TAG = "QmiPrimitiveTypes";

    // size of basic data types
    public static final int SIZE_OF_BYTE = 1;

    public static final int SIZE_OF_SHORT = 2;

    public static final int SIZE_OF_INT = 4;

    public static final int SIZE_OF_LONG = 8;

    // null byte stream
    public static class QmiNull extends BaseQmiItemType {
        public QmiNull() {
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

        public QmiByte() {
            this.mVal = 0;
        }

        public QmiByte(byte val) {
            this.mVal = val;
        }

        public QmiByte(short val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseByte(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiByte(int val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseByte(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiByte(char val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseByte(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiByte(byte[] bArray) throws InvalidParameterException {
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

        public QmiShort() {
            this.mVal = 0;
        }

        public QmiShort(int val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseShort(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiShort(byte[] bArray) throws InvalidParameterException {
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

    public static class QmiArray<T extends BaseQmiItemType> extends BaseQmiItemType {
        private T[] mVal;

        private short mArrayLength;
        private short mNumOfElements = 0;

        /*
         * size of length within V field of TLV. It can be 1 or 2 bytes
         * depending on max number of elements in the array.
         */
        private short vLenSize;

        public QmiArray(T[] arr, short maxArraySize, Class<T> c) throws InvalidParameterException {
            try {
                this.mVal = arr;
                mArrayLength = (short) arr.length;
                if (maxArraySize > 255)
                    vLenSize = 2; // 2 bytes is max size of length within V
                // field of TLV
                else
                    vLenSize = 1;
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        /*
         * @param valueSize The size of the length within V of TLV.
         *
         */
        public QmiArray(T[] arr, Class<T> c, short valueSize) throws InvalidParameterException {
            try {
                this.mVal = arr;
                mArrayLength = (short) arr.length;
                vLenSize = valueSize;
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        /*
         * @param valueSize The size of the length within V of TLV.
         * @param numOfElements The number of elements in one set of V in TLV.
         *
         */
        public QmiArray(T[] arr, Class<T> c, short valueSize, short numOfElements)
                throws InvalidParameterException {
            try {
                this.mVal = arr;
                mArrayLength = (short) arr.length;
                vLenSize = valueSize;
                mNumOfElements = (short) numOfElements;
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public int getSize() {
            int actualArrayBytesSize = 0;
            for (int i = 0; i < mArrayLength; i++)
                actualArrayBytesSize += mVal[i].getSize();
            return vLenSize + actualArrayBytesSize;
        }

        public String toString() {
            StringBuffer s = new StringBuffer();
            for (int i = 0; i < mArrayLength; i++)
                s.append(mVal[i].toString());
            return s.toString();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());

            /* This is the length field within the V field in a TLV */
            short numberOfSets = 0;
            if (mNumOfElements != 0) {
                numberOfSets = (short) (mArrayLength/mNumOfElements);
            } else {
                numberOfSets = mArrayLength;
            }
            if (vLenSize == 2)
                buf.putShort(numberOfSets);
            else
                buf.put(PrimitiveParser.parseByte(numberOfSets));

            for (int i = 0; i < mArrayLength; i++) {
                buf.put(mVal[i].toByteArray());
            }

            return buf.array();
        }
    }

    public static class QmiEnum extends BaseQmiItemType {
        private short mVal;

        public QmiEnum(int val, int allowedValues[]) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseShort(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        @Override
        public int getSize() {
            return SIZE_OF_BYTE;
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putShort(mVal);
            return buf.array();
        }

        @Override
        public String toString() {
            return String.format("val=%d", mVal);
        }

    }

    public static class QmiInteger extends BaseQmiItemType {
        private int mVal;

        public QmiInteger() {
            this.mVal = 0;
        }

        public QmiInteger(long val) throws InvalidParameterException {
            try {
                this.mVal = PrimitiveParser.parseInt(val);
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public QmiInteger(byte[] bArray) throws InvalidParameterException {
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

        public QmiLong(long mVal) {
            this.mVal = mVal;
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
            if (mVal.length() > 65536) {
                throw new InvalidParameterException();
            }
            this.mVal = mVal;
        }

        public QmiString(byte[] bArray) throws InvalidParameterException {
            try {
                mVal = new String(bArray, QMI_CHARSET);
            } catch (UnsupportedEncodingException e) {
                throw new InvalidParameterException(e.toString());
            }
        }

        public String toStringValue() {
            return mVal;
        }

        @Override
        public int getSize() {
            return mVal.length();
        }

        @Override
        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
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
}
