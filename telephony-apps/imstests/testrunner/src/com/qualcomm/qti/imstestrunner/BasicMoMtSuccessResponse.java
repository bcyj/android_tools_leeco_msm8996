/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.imstestrunner;

import java.util.Random;
import android.util.Log;

/**
 * ResponseGenerator implementation that contains methods that return payloads for successful voice
 * calls
 *
 * @author Douglas Sigelbaum
 */
public class BasicMoMtSuccessResponse extends ResponseGenerator {

    public BasicMoMtSuccessResponse() {
        mtCallType = ImsQmiIF.CALL_TYPE_VOICE;
    }

    @Override
    public byte[] generateImsRegistrationState() {
        ImsQmiIF.Registration registration = new ImsQmiIF.Registration();
        registration.setState(ImsQmiIF.Registration.REGISTERED);
        return registration.toByteArray();
    }

    @Override
    public byte[] generateSrvStatusList() {
        ImsQmiIF.SrvStatusList srvStatusList = new ImsQmiIF.SrvStatusList();
        ImsQmiIF.Info info = new ImsQmiIF.Info();
        info.setIsValid(true);
        info.setType(ImsQmiIF.CALL_TYPE_VOICE);
        ImsQmiIF.StatusForAccessTech status = new ImsQmiIF.StatusForAccessTech();
        status.setStatus(ImsQmiIF.STATUS_ENABLED);
        status.setRegistered((new ImsQmiIF.Registration())
                .setState(ImsQmiIF.Registration.REGISTERED));
        status.setNetworkMode(ImsQmiIF.RADIO_TECH_LTE);
        status.setRestrictionCause(0);
        info.addAccTechStatus(status);
        srvStatusList.addSrvStatusInfo(info);
        return srvStatusList.toByteArray();
    }

    @Override
    public byte[] generateRingBackTone() {
        ImsQmiIF.RingBackTone ringBackTone = new ImsQmiIF.RingBackTone();
        ringBackTone.setFlag(ImsQmiIF.RingBackTone.START);
        return ringBackTone.toByteArray();
    }

    @Override
    public byte[] generateLastCallFailCause() {
        ImsQmiIF.CallFailCauseResponse callFailCause = new ImsQmiIF.CallFailCauseResponse();
        callFailCause.setFailcause(ImsQmiIF.Hangup.FAILCAUSERESPONSE_FIELD_NUMBER);
        return callFailCause.toByteArray();
    }

    @Override
    public byte[] generateGetCurrentCallsResponse() {
        byte[] response = null;
        if (mCallList != null && !(mCallList.getCallAttributesList().isEmpty())) {
            response = mCallList.toByteArray();
        }
        return response;
    }

    @Override
    public byte[] generateUnsolRadioStateChanged() {
        ImsQmiIF.RadioStateChanged radioStateChanged = new ImsQmiIF.RadioStateChanged();
        radioStateChanged.setState(ImsQmiIF.RADIO_STATE_ON);
        return radioStateChanged.toByteArray();
    }

    @Override
    public boolean callStateDialing(byte[] msg) {
        if (msg != null) {
            if (mCallList == null) {
                mCallList = new ImsQmiIF.CallList();
            }
            ImsQmiIF.Dial dial = new ImsQmiIF.Dial();
            try {
                dial = ImsQmiIF.Dial.parseFrom(msg);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException while parsing Dial msg");
                return false;
            }
            ImsQmiIF.CallList.Call call = createVoiceCall(false, ImsQmiIF.CALL_DIALING,
                    dial.getAddress(), dial.getCallDetails(), false);
            mCallList.addCallAttributes(call);
            boolean hold = holdActiveCall();
            log("Hold active call == " + hold);
            log("Calls count: " + mCallList.getCallAttributesCount());
            log("Dial address: " + dial.getAddress());
            return true;
        }
        return false;
    }

    @Override
    public void callStateIncomingOrWaiting(int state) {
        if (mCallList == null) {
            mCallList = new ImsQmiIF.CallList();
        }
        // generate random phone number for MT call
        Random rand = new Random();
        String randomNumber = Integer.toString(rand.nextInt(80000) + 20000);
        ImsQmiIF.CallDetails callDetails = new ImsQmiIF.CallDetails();
        callDetails.setCallType(ImsQmiIF.CALL_TYPE_VOICE);
        callDetails.setCallDomain(ImsQmiIF.CALL_DOMAIN_PS);
        callDetails.setExtrasLength(0);
        ImsQmiIF.CallList.Call call = createVoiceCall(true, state, randomNumber, callDetails, false);
        mCallList.addCallAttributes(call);
    }

    @Override
    protected ImsQmiIF.CallList.Call createVoiceCall(boolean isMT, int state, String number,
            ImsQmiIF.CallDetails details, boolean isMpty) {
        ImsQmiIF.CallList.Call call = new ImsQmiIF.CallList.Call();
        call.setIndex(mCallIndex++);
        call.setToa(0); // Type of address
        call.setIsMpty(isMpty);
        call.setState(state);
        call.setNumber(number);
        call.setCallDetails(details);
        call.setIsMT(isMT);
        call.setAls(0); // ALS line indicator if available
        call.setIsVoice(true);
        call.setIsVoicePrivacy(false);
        call.setNumberPresentation(0);
        return call;
    }

    @Override
    public boolean addParticipant(byte[] msg) {
        if (msg != null) {
            if (mCallList == null) {
                log("Attempt to add participant when there are no active calls");
                return false;
            }
            ImsQmiIF.Dial dial = new ImsQmiIF.Dial();
            try {
                dial = ImsQmiIF.Dial.parseFrom(msg);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException while parsing Dial msg");
            }
            getActiveCall().setIsMpty(true);
            ImsQmiIF.CallList.Call call = createVoiceCall(false, ImsQmiIF.CALL_DIALING,
                    dial.getAddress(), dial.getCallDetails(), true);
            mCallList.addCallAttributes(call);
            log("Calls count: " + mCallList.getCallAttributesCount());
            log("Dial address: " + dial.getAddress());
            return true;
        }
        return false;
    }

    @Override
    public boolean holdActiveCall() {
        ImsQmiIF.CallList.Call call = getActiveCall();
        if (call != null) {
            call.setState(ImsQmiIF.CALL_HOLDING);
            return true;
        }
        return false;
    }

    @Override
    public boolean callDialingToAlerting() {
        ImsQmiIF.CallList.Call call = getDialingCall();
        if (call != null) {
            call.setState(ImsQmiIF.CALL_ALERTING);
            return true;
        }
        return false;
    }

    @Override
    public boolean hangupIncomingOrWaitingCall() {
        ImsQmiIF.CallList.Call incomingCall = getIncomingCall();
        ImsQmiIF.CallList.Call waitingCall = getWaitingCall();
        if (incomingCall != null) {
            mCallList.getCallAttributesList().remove(incomingCall);
            return true;
        }
        if (waitingCall != null) {
            mCallList.getCallAttributesList().remove(waitingCall);
            return true;
        }
        return false;
    }

    @Override
    public boolean callAlertingToActive() {
        ImsQmiIF.CallList.Call call = getAlertingCall();
        if (call != null) {
            call.setState(ImsQmiIF.CALL_ACTIVE);
            return true;
        }
        return false;
    }

    @Override
    public boolean callActiveToEnd() {
        ImsQmiIF.CallList.Call call = getActiveCall();
        if (call != null) {
            call.setState(ImsQmiIF.CALL_END);
            return true;
        }
        return false;
    }

    @Override
    public boolean hangupFgCallResumeBgCall() {
        mCallList.getCallAttributesList().remove(getActiveCall());
        return resumeBgCall();
    }

    @Override
    public boolean resumeBgCall() {
        ImsQmiIF.CallList.Call holdingCall = getHoldingCall();
        if (holdingCall != null) {
            holdingCall.setState(ImsQmiIF.CALL_ACTIVE);
            return true;
        }
        return false;
    }

    @Override
    public boolean answerIncomingCall() {
        holdActiveCall();
        ImsQmiIF.CallList.Call incomingCall = getIncomingCall();
        if (incomingCall != null) {
            incomingCall.setState(ImsQmiIF.CALL_ACTIVE);
            return true;
        }
        return false;
    }

    @Override
    public boolean answerWaitingCall() {
        holdActiveCall();
        ImsQmiIF.CallList.Call waitingCall = getWaitingCall();
        if (waitingCall != null) {
            waitingCall.setState(ImsQmiIF.CALL_ACTIVE);
            return true;
        }
        return false;
    }

    /**
     * In this ResponseGenerator implementation, all error ID's sent to ImsSenderRxr should signify
     * a successful call, hence the returning of "ImsQmiIF.E_SUCCESS" regardless of the msgId.
     */
    @Override
    public int getError(int msgId) {
        int error = -1;
        switch (msgId) {
        // TODO take out cases that should not happen if the call is a simple MO
        // call
            case ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE:
            case ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION:
            case ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS:
            case ImsQmiIF.REQUEST_GET_CURRENT_CALLS:
            case ImsQmiIF.REQUEST_DIAL:
            case ImsQmiIF.REQUEST_ANSWER:
            case ImsQmiIF.REQUEST_HANGUP:
            case ImsQmiIF.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
            case ImsQmiIF.REQUEST_LAST_CALL_FAIL_CAUSE:
            case ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
            case ImsQmiIF.REQUEST_ADD_PARTICIPANT:
            case ImsQmiIF.REQUEST_HANGUP_WAITING_OR_BACKGROUND:
            case ImsQmiIF.REQUEST_HOLD:
            case ImsQmiIF.REQUEST_RESUME:
            case ImsQmiIF.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
            case ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED:
            case ImsQmiIF.UNSOL_RINGBACK_TONE:
            case ImsQmiIF.UNSOL_CALL_RING:
                error = ImsQmiIF.E_SUCCESS;
                break;
            default:
                log("Unrecognized message ID " + msgId);
                break;
        }
        return error;
    }

}
