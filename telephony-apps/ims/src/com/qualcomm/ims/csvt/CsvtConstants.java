/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package com.qualcomm.ims.csvt;

public interface CsvtConstants {
    public static final int ERROR_SUCCESS = 0;
    public static final int ERROR_FAILED = 1;

    public static final int DIAL_FAILED = -1;
    public static final int HANGUP_FAILED = -2;
    public static final int FALLBACK_FAILED = -3;
    public static final int ACCEPT_CALL_FAILED = -4;
    public static final int REJECT_CALL_FAILED = -5;

    public static final int CALL_STATE_IDLE = 0;
    public static final int CALL_STATE_RINGING = 1;
    public static final int CALL_STATE_OFFHOOK = 2;

    public static final int CALL_STATE_ACTIVE = 3;
    public static final int CALL_STATE_DISCONNECTED = 4;

    public static final int CALL_STATUS_DISCONNECTED_NO_ANSWER = 10;
    public static final int CALL_STATUS_DISCONNECTED_BUSY = 11;
    public static final int CALL_STATUS_DISCONNECTED_INVALID_NUMBER = 12;
    public static final int CALL_STATUS_DISCONNECTED_INCOMING_REJECTED = 13;
    public static final int CALL_STATUS_DISCONNECTED_POWER_OFF = 14;
    public static final int CALL_STATUS_DISCONNECTED_OUT_OF_SERVICE = 15;
    public static final int CALL_STATUS_DISCONNECTED_UNASSIGNED_NUMBER = 16;
    public static final int CALL_STATUS_DISCONNECTED_INCOMPATIBILITY_DESTINATION = 17;
    public static final int CALL_STATUS_DISCONNECTED_RESOURCES_UNAVAILABLE = 18;
    public static final int CALL_STATUS_DISCONNECTED_BEARER_NOT_AUTHORIZATION = 19;
    public static final int CALL_STATUS_DISCONNECTED_BEARER_NOT_AVAIL = 20;
    public static final int CALL_STATUS_DISCONNECTED_NUMBER_CHANGED = 21;

    public static final int CALL_STATUS_CONNECTED = 22;
    public static final int CALL_STATUS_DISCONNECTED = 23;

    public static final int CALL_STATUS_DISCONNECTED_LOST_SIGNAL = 24;
    public static final int CALL_STATUS_DISCONNECTED_NORMAL_UNSPECIFIED = 25;
    public static final int CALL_STATUS_DISCONNECTED_PROTOCOL_ERROR_UNSPECIFIED = 26;
    public static final int CALL_STATUS_DISCONNECTED_BEARER_SERVICE_NOT_IMPLEMENTED = 27;
    public static final int CALL_STATUS_DISCONNECTED_SERVICE_OR_OPTION_NOT_IMPLEMENTED = 28;
    public static final int CALL_STATUS_DISCONNECTED_NO_USER_RESPONDING = 29;
    public static final int CALL_STATUS_DISCONNECTED_NETWORK_CONGESTION = 30;
    public static final int CALL_STATUS_DISCONNECTED_INCOMING_MISSED = 31;
    public static final int CALL_STATUS_DISCONNECTED_LCOAL_PHONE_OUT_OF_3G_SERVICE = 32;
    public static final int CALL_STATUS_DISCONNECTED_NORMAL = 33;
    public static final int CALL_STATUS_DISCONNECTED_ERROR_UNSPECIFIED = 0xffff;

    public static final String CONNECTION_ADDRESS_KEY  = "connectionAddress";
}
