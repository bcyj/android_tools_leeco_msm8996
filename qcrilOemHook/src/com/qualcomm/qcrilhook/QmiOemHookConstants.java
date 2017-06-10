/*
 * Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

public interface QmiOemHookConstants {

    public static final String OEM_IDENTIFIER = "QOEMHOOK";
    public static final String INSTANCE_ID = "INSTANCE_ID";

    /*
     * Sizes of the various Qmi byte streams
     */
    public static final int RESPONSE_BUFFER_SIZE = 2048;

    public static final int INT_SIZE = 4;

    public static final int BYTE_SIZE = 1;

    public static final int SHORT_SIZE = 2;

    public static final short RESULT_CODE_TYPE = 0x02;

    public static final int SERVICE_ID_SIZE = 2;

    public static final int MESSAGE_ID_SIZE = 2;

    /*
     * The response byte stream is decoded and put into a hashmap. These are the
     * keys in the hashMap
     */
    public static final int REQUEST_ID = 1;

    public static final int RESPONSE_TLV_SIZE = 2;

    public static final int SUCCESS_STATUS = 3;

    public static final int MESSAGE = 4;

    public static final int RESPONSE_TYPE = 5;

    public static final int RESPONSE_BUFFER = 6;

    public static final int SERVICE_ID = 7;

    public static final int MESSAGE_ID = 8;

    public static final int PHONE_ID = 9;

    public static enum ResponseType {
        IS_SYNC_RESPONSE, IS_ASYNC_RESPONSE, IS_UNSOL
    }
}
