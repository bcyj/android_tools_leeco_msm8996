/******************************************************************************
 * @file    BaseQmiItemTypes.java
 * @brief   Contains base QMI item type definitions to be passed to the Modem.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcnvitems;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;

public class BaseQmiTypes {

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

        protected static ByteBuffer createByteBuffer(int size) {
            ByteBuffer buf = ByteBuffer.allocate(size);
            buf.order(QMI_BYTE_ORDER);
            return buf;
        }

        protected static ByteBuffer createByteBuffer(byte[] bytes) {
            ByteBuffer buf = ByteBuffer.wrap(bytes);
            buf.order(QMI_BYTE_ORDER);
            return buf;
        }
    }

    public static abstract class BaseQmiItemType extends QmiBase {

        public abstract byte[] toByteArray();

        public abstract int getSize();

        protected byte[] toTlv(short type) throws InvalidParameterException {
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

        protected static byte[] parseTlv(byte[] tlv) {
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

        protected static byte[] parseData(ByteBuffer buf, int length) {
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

    static final short DEFAULT_NAM = 0;
}
