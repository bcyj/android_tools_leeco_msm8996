/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

public class ImsConstants {
    public final static int UNREGISTARATION = 0;
    public final static int IDLE = 1;
    public final static int DIALING = 2;
    public final static int INCOMING = 3;
    public final static int INCALL_VOLTE = 4;
    public final static int INCALL_RX = 5;
    public final static int INCALL_TX = 6;
    public final static int INCALL_VT = 7;
    public final static int DISCONNECTED = 8;
    public final static int MODIFYING = 9;

    // PhoneConstants.state;
    public static class ImsState {
        public final static int IDLE = 0;
        public final static int RINGING = 1;
        public final static int OFFHOOK = 2;
    }

    public static final int REGINSTRATION_REGISTERED = 1;
    public static final int REGINSTRATION_UNREGISTERED = 1;

    public static final int CALL_STATUS_DIALED = 0; // The number was
                                                    // successfully dialed
    public static final int CALL_STATUS_DIALED_MMI = 1; // The specified number
                                                        // was an MMI code
    public static final int CALL_STATUS_FAILED = 2; // The call failed
}
