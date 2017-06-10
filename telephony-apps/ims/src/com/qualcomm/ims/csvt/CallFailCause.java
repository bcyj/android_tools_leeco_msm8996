/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (c) 2012 Code Aurora Forum. All rights reserved.
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

package com.qualcomm.ims.csvt;

//TBD edit header
/**
 * CDMA Call fail causes covering all the possible failures that are needed to
 * be distinguished by the UI. CDMA call failure reasons are derived from the
 * possible call failure scenarios described in
 * "CDMA IS2000 - Release A (C.S0005-A v6.0)" standard. {@hide}
 */
public interface CallFailCause {
    // Unassigned/Unobtainable number
    static final int UNOBTAINABLE_NUMBER = 1;
    static final int OPERATOR_DETERMINED_BARRING = 8;

    static final int NORMAL_CLEARING = 16;
    // Busy Tone
    static final int USER_BUSY = 17;
    static final int NO_USER_RESPONDING = 18;
    static final int USER_ALERTING_NO_ANSWER = 19;
    static final int CALL_REJECTED = 21;

    // No Tone
    static final int NUMBER_CHANGED = 22;
    static final int DESTINATION_OUT_OF_ORDER = 27;
    static final int INVALID_NUMBER = 28;
    static final int STATUS_ENQUIRY = 30;

    static final int NORMAL_UNSPECIFIED = 31;

    // Congestion Tone
    static final int NO_CIRCUIT_AVAIL = 34;
    static final int TEMPORARY_FAILURE = 41;
    static final int SWITCHING_CONGESTION = 42;
    static final int CHANNEL_NOT_AVAIL = 44;
    static final int RESOURCES_UNAVAILABLE = 47;
    static final int QOS_NOT_AVAIL = 49;
    static final int LOCAL_PHONE_OUT_OF_3G_Service = 52;
    static final int BEARER_NOT_AUTHORIZATION = 57;
    static final int BEARER_NOT_AVAIL = 58;
    // others
    static final int BEARER_SERVICE_NOT_IMPLEMENTED = 65;
    static final int ACM_LIMIT_EXCEEDED = 68;
    static final int REQUESTED_FACILITY_NOT_IMPLEMENTED = 69;
    static final int SERVICE_OR_OPTION_NOT_IMPLEMENTED = 79;
    static final int INCOMPATIBILITY_DESTINATION = 88;

    static final int PROTOCOL_ERROR_UNSPECIFIED = 111;

    static final int CALL_BARRED = 240;
    static final int FDN_BLOCKED = 241;

    static final int ANSWERED_ELSEWHERE = 1014;

    static final int ERROR_UNSPECIFIED = 0xffff;
    // TODO add ims specific eror codes
}
