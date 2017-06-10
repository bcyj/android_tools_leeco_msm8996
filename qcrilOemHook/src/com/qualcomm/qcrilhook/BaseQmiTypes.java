/******************************************************************************
 * @file    BaseQmiItemTypes.java
 * @brief   Contains base QMI item type definitions to be passed to the Modem.
 *
 * Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/

package com.qualcomm.qcrilhook;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;

public class BaseQmiTypes {

    public static final short DEFAULT_NAM = 0;

    public static abstract class QmiBase {
        // size of TLV header items

        public static final int TLV_TYPE_SIZE = 1;

        public static final int TLV_LENGTH_SIZE = 2;

        /*
         * QMI_BYTE_ORDER must be LITTLE_ENDIAN to comply with QMI standards.
         * ByteOrder.nativeOrder() was previously used with the NV items.
         */
        public static final ByteOrder QMI_BYTE_ORDER = ByteOrder.LITTLE_ENDIAN;

        public static final String QMI_CHARSET = "US-ASCII";

        public abstract String toString();

        public static ByteBuffer createByteBuffer(int size) {
            ByteBuffer buf = ByteBuffer.allocate(size);
            buf.order(QMI_BYTE_ORDER);
            return buf;
        }

        public static ByteBuffer createByteBuffer(byte[] bytes) {
            ByteBuffer buf = ByteBuffer.wrap(bytes);
            buf.order(QMI_BYTE_ORDER);
            return buf;
        }
    }

    public static abstract class BaseQmiItemType extends QmiBase {

        public abstract byte[] toByteArray();

        public abstract int getSize();

        public byte[] toTlv(short type) throws InvalidParameterException {
            ByteBuffer buf = createByteBuffer(TLV_TYPE_SIZE + TLV_LENGTH_SIZE + getSize());
            try {
                buf.put(PrimitiveParser.parseByte(type));
            } catch (NumberFormatException e) {
                throw new InvalidParameterException(e.toString());
            }
            buf.putShort(PrimitiveParser.parseShort(getSize()));
            buf.put(toByteArray());
            return buf.array();
        }

        public static byte[] parseTlv(byte[] tlv) {
            ByteBuffer buf = createByteBuffer(tlv);
            buf.get(); // type generally unnecessary here
            short size = buf.getShort();
            ByteBuffer bArray = ByteBuffer.allocate(size);
            for (int i = 0; i < size; i++) {
                bArray.put(buf.get());
            }
            return bArray.array();
        }
    }

    public static abstract class BaseQmiStructType extends QmiBase {

        public abstract short[] getTypes();

        public abstract BaseQmiItemType[] getItems();

        /*
         * Converts Bytebuffer to byte array.
         */
        public static byte[] parseData(ByteBuffer buf, int length) {
            byte[] data = new byte[length];
            for (int i = 0; i < length; i++) {
                data[i] = buf.get();
            }
            return data;
        }

        @Override
        public String toString() {
            String temp = "";
            boolean isFirstItem = true;
            for (BaseQmiItemType i : getItems()) {
                if (isFirstItem) {
                    isFirstItem = false;
                } else {
                    temp += ", ";
                }
                temp += i.toString();
            }
            return temp;
        }
    }

}
