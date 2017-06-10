/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

import android.os.Handler;

interface IVTConnection {
    public final static int VTCALL_BASE = 2009;

    public final static int VTCALL_STATE_IDLE = VTCALL_BASE + 1;
    public final static int VTCALL_STATE_OFFHOOK = VTCALL_BASE + 2;
    public final static int VTCALL_STATE_RINGING = VTCALL_BASE + 3;
    public final static int VTCALL_STATE_ACTIVE = VTCALL_BASE + 4;

    public final static int VTCALL_RESULT_CONNECTED = VTCALL_BASE + 10;
    public final static int VTCALL_RESULT_DISCONNECTED = VTCALL_BASE + 11;
    public final static int VTCALL_RESULT_DISCONNECTED_LOST_SIGNAL = VTCALL_BASE + 12;
    public static final int VTCALL_RESULT_DISCONNECTED_NO_ANSWER = VTCALL_BASE + 13;
    public static final int VTCALL_RESULT_DISCONNECTED_BUSY = VTCALL_BASE + 14;
    public static final int VTCALL_RESULT_DISCONNECTED_INVALID_NUMBER = VTCALL_BASE + 15;
    public static final int VTCALL_RESULT_DISCONNECTED_INCOMING_REJECTED = VTCALL_BASE + 16;
    public static final int VTCALL_RESULT_DISCONNECTED_POWER_OFF = VTCALL_BASE + 17;
    public static final int VTCALL_RESULT_DISCONNECTED_OUT_OF_SERVICE = VTCALL_BASE + 18;
    public static final int VTCALL_RESULT_DISCONNECTED_UNASSIGNED_NUMBER = VTCALL_BASE + 19;
    public static final int VTCALL_RESULT_DISCONNECTED_NUMBER_CHANGED = VTCALL_BASE + 20;
    public static final int VTCALL_RESULT_DISCONNECTED_INCOMING_MISSED = VTCALL_BASE + 21;
    public static final int VTCALL_RESULT_DISCONNECTED_LOCAL_OUT_OF_3G_SERVICE = VTCALL_BASE + 22;

    public final static int VTCALL_RESULT_FALLBACK_47 = VTCALL_BASE + 30;
    public final static int VTCALL_RESULT_FALLBACK_57 = VTCALL_BASE + 31;
    public final static int VTCALL_RESULT_FALLBACK_58 = VTCALL_BASE + 32;
    public final static int VTCALL_RESULT_FALLBACK_88 = VTCALL_BASE + 33;

    // new result info
    public final static int VTCALL_RESULT_NORMAL_UNSPECIFIED = VTCALL_BASE + 34; // network
                                                                                 // busy
    public final static int VTCALL_RESULT_PROTOCOL_ERROR_UNSPECIFIED = VTCALL_BASE + 35; // network
                                                                                         // busy
    public final static int VTCALL_RESULT_NO_USER_RESPONDING = VTCALL_BASE + 36; // no
                                                                                 // user
                                                                                 // responding
    public final static int VTCALL_RESULT_BEARER_NOT_SUPPORTED_65 = VTCALL_BASE + 37; // bearer
                                                                                      // not
                                                                                      // supported
    public final static int VTCALL_RESULT_BEARER_NOT_SUPPORTED_79 = VTCALL_BASE + 38; // bearer
                                                                                      // not
                                                                                      // supported
    public final static int VTCALL_RESULT_NETWORK_CONGESTION = VTCALL_BASE + 39; // network
                                                                                 // congestion

    // Exception notification
    public final static int VTCALL_RESULT_CALL_EXCEPTION = VTCALL_BASE + 50; //
    public final static int VTCALL_RESULT_ENDCALL_EXCEPTION = VTCALL_BASE + 51; //
    public final static int VTCALL_RESULT_FALLBACK_EXCEPTION = VTCALL_BASE + 52; //
    public final static int VTCALL_RESULT_ANSWERCALL_EXCEPTION = VTCALL_BASE + 53; //
    public final static int VTCALL_RESULT_REJECTCALL_EXCEPTION = VTCALL_BASE + 54; //

    public static final int MESSAGE_GET_CALL_WAITING = VTCALL_BASE + 55;
    public static final int MESSAGE_GET_CF = VTCALL_BASE + 56;
    public static final int MESSAGE_SET_CF = VTCALL_BASE + 57;
    public static final int MESSAGE_RINGBACK_TONE = VTCALL_BASE + 58;

    // public void refuseCall();
    public void clear();

    public void acceptCall();

    public void endSession();

    public void rejectSession();

    public void fallBack();

    public void setHandler(Handler h);
}
