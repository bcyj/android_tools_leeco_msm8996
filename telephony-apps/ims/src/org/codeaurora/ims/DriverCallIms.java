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

package org.codeaurora.ims;

import com.android.ims.ImsReasonInfo;
import com.android.internal.telephony.ATParseEx;
import com.android.internal.telephony.DriverCall;

/**
 * {@hide}
 */
public class DriverCallIms extends DriverCall {
    static final String LOG_TAG = "DRIVERCALL-IMS";

    public enum State {
        ACTIVE,
        HOLDING,
        DIALING, // MO call only
        ALERTING, // MO call only
        INCOMING, // MT call only
        WAITING, // MT call only
        END;
    }

    public CallDetails callDetails;
    public State state;
    public ImsReasonInfo callFailCause;

    // Copy Constructor
    public DriverCallIms(DriverCallIms dc) {
        callDetails = new CallDetails(dc.callDetails);
        callFailCause = new ImsReasonInfo(dc.callFailCause.mCode,
                dc.callFailCause.mExtraCode,
                dc.callFailCause.mExtraMessage);
        state = dc.state;
        index = dc.index;
        number = dc.number;
        isMT = dc.isMT;
        TOA = dc.TOA;
        isMpty = dc.isMpty;
        als = dc.als;
        isVoice = dc.isVoice;
        isVoicePrivacy = dc.isVoicePrivacy;
        numberPresentation = dc.numberPresentation;
        name = dc.name;
        namePresentation = dc.namePresentation;
    }

    public DriverCallIms() {
        callDetails = new CallDetails();
    }

    public static State stateFromCall(int state) throws ATParseEx {
        switch (state) {
            case ImsQmiIF.CALL_ACTIVE:
                return State.ACTIVE;
            case ImsQmiIF.CALL_HOLDING:
                return State.HOLDING;
            case ImsQmiIF.CALL_DIALING:
                return State.DIALING;
            case ImsQmiIF.CALL_ALERTING:
                return State.ALERTING;
            case ImsQmiIF.CALL_INCOMING:
                return State.INCOMING;
            case ImsQmiIF.CALL_WAITING:
                return State.WAITING;
            case ImsQmiIF.CALL_END:
                return State.END;
            default:
                throw new ATParseEx("illegal call state " + state);
        }
    }

    //Update members of the object with the update, return true if changed
    public boolean update(DriverCallIms update) {
        if (update == null) {
            return false;
        }
        boolean hasChanged = false;
        if (state != update.state) {
            state = update.state;
            hasChanged = true;
        }
        if (index != update.index) {
            index = update.index;
            hasChanged = true;
        }
        if (number != update.number) {
            number = update.number;
            hasChanged = true;
        }
        if (isMT != update.isMT) {
            isMT = update.isMT;
            hasChanged = true;
        }
        if (isMpty != update.isMpty) {
            isMpty = update.isMpty;
            hasChanged = true;
        }
        if (update.callFailCause != null) {
            if (callFailCause == null) {
                callFailCause = new ImsReasonInfo(update.callFailCause.mCode,
                        update.callFailCause.mExtraCode,
                        update.callFailCause.mExtraMessage);
            } else {
                if (callFailCause.mCode != update.callFailCause.mCode) {
                    callFailCause.mCode = update.callFailCause.mCode;
                }
                if (callFailCause.mExtraCode != update.callFailCause.mExtraCode) {
                    callFailCause.mExtraCode = update.callFailCause.mExtraCode;
                }
                if (callFailCause.mExtraMessage != update.callFailCause.mExtraMessage) {
                    callFailCause.mExtraMessage = update.callFailCause.mExtraMessage;
                }
            }
        }
        if(callDetails.update(update.callDetails) && !hasChanged) {
            hasChanged = true;
        }
        return hasChanged;
    }

    public String toString() {
        return "id=" + index + "," + state + "," + "toa=" + TOA + ","
                + (isMpty ? "conf" : "norm") + "," + (isMT ? "mt" : "mo") + ","
                + als + "," + (isVoice ? "voc" : "nonvoc") + ","
                + (isVoicePrivacy ? "evp" : "noevp") + ","
                /* + "number=" + number */+ ",cli=" + numberPresentation + ","
                /* + "name="+ name */+ "," + namePresentation
                + "Call Details =" + callDetails + "," + "CallFailCause Code= "
                + callFailCause.mCode + "," + "CallFailCause String= "
                + callFailCause.mExtraMessage;
    }
}
