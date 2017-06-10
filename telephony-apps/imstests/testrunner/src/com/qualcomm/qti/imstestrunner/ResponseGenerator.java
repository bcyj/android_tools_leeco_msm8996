/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.imstestrunner;

import java.util.Random;
import android.util.Log;

public abstract class ResponseGenerator {
    static final String LOGTAG = "ResponseGenerator";

    protected boolean isAndroidL = true;
    protected int mCallIndex = 1;
    protected ImsQmiIF.CallList mCallList;
    protected int mtCallType;

    /**
     * @return registration -- byte[] of IMS registration
     */
    public abstract byte[] generateImsRegistrationState();

    /**
     * @return registration -- byte[] of IMS registration
     */
    public abstract byte[] generateSrvStatusList();

    /**
     * @return ringback tone -- byte[] of a ringback tone
     */
    public abstract byte[] generateRingBackTone();

    /**
     * @return lastCallFailCause -- byte[] of a lastCallFailCause message
     */
    public abstract byte[] generateLastCallFailCause();

    /**
     * Generates a response payload for the getCurrentCalls request
     * @return byte[] of mCallList
     */
    public abstract byte[] generateGetCurrentCallsResponse();

    /**
     * Change state of a radiostate and return it
     * @return byte[] of radiostate message
     */
    public abstract byte[] generateUnsolRadioStateChanged();

    public byte[] generateUnsolToDialing() {
        return generateCallListResponse();
    }

    public byte[] generateUnsolDialingToAlerting() {
        return generateCallListResponse();
    }

    public byte[] generateUnsolAlertingToActive() {
        return generateCallListResponse();
    }

    public byte[] generateUnsolToEnd() {
        return generateCallListResponse();
    }

    public byte[] generateUnsolMtCallResponse() {
        return generateCallListResponse();
    }

    /**
     * In Android L, mCallList is the payload for unsol state changes, so a byte[] of mCallList
     * needs to be returned here. If we are in earlier Android releases, the payload is null.
     *
     * @return mCallList
     */
    protected byte[] generateCallListResponse() {
        if (isAndroidL && mCallList != null) {
            return mCallList.toByteArray();
        }
        return null;
    }

    /**
     * @param msgId -- message ID of message received from ImsSenderRxr
     * @return error -- error ID that should be sent to ImsSenderRxr
     */
    public abstract int getError(int msgId);

    public void setIsAndroidL(boolean is) {
        isAndroidL = is;
    }

    public int getMtCallType() {
        return mtCallType;
    }

    /**
     * This method should create an MO call that is dialing and add it to mCallList, which keeps
     * track of the current calls. The phone number of the MO call should match the number dialed on
     * ImsPhone.
     *
     * @param state -- is the call incoming or waiting.
     */
    public abstract boolean callStateDialing(byte[] msg);

    /**
     * This method should create an MT call that is incoming/waiting and add it to mCallList, which
     * keeps track of the current calls.
     *
     * @param state -- is the call incoming or waiting.
     */
    public abstract void callStateIncomingOrWaiting(int state);

    /**
     * @param isMT -- is the call an MT call
     * @param state -- is the state dialing, incoming, waiting, etc.
     * @param number -- phone number of other phone
     * @param isMpty -- is the call a multiparty call
     * @return A voice call with attributes that match the params
     */
    protected abstract ImsQmiIF.CallList.Call createVoiceCall(boolean isMT, int state,
            String number,
            ImsQmiIF.CallDetails details, boolean isMpty);

    /**
     * Add a participant to an active call in mCallList
     *
     * @param msg
     * @return whether or not adding a participant was allowed
     */
    public abstract boolean addParticipant(byte[] msg);

    /**
     * isAndroidL starts out as 'true' and only turns to 'false' if a GET_CURRENT_CALLS message is
     * received
     *
     * @return true if this is in Android L
     */
    public boolean getIsAndroidL() {
        return isAndroidL;
    }

    /**
     * Returns current list of calls
     *
     * @return mCallList
     */
    public ImsQmiIF.CallList getCallList() {
        return mCallList;
    }

    public ImsQmiIF.CallList.Call getActiveCall() {
        return getCallWithState(ImsQmiIF.CALL_ACTIVE);
    }

    public ImsQmiIF.CallList.Call getHoldingCall() {
        return getCallWithState(ImsQmiIF.CALL_HOLDING);
    }

    public ImsQmiIF.CallList.Call getWaitingCall() {
        return getCallWithState(ImsQmiIF.CALL_WAITING);
    }

    public ImsQmiIF.CallList.Call getDialingCall() {
        return getCallWithState(ImsQmiIF.CALL_DIALING);
    }

    public ImsQmiIF.CallList.Call getAlertingCall() {
        return getCallWithState(ImsQmiIF.CALL_ALERTING);
    }

    public ImsQmiIF.CallList.Call getIncomingCall() {
        return getCallWithState(ImsQmiIF.CALL_INCOMING);
    }

    protected ImsQmiIF.CallList.Call getCallWithState(int state) {
        if (mCallList != null) {
            for (ImsQmiIF.CallList.Call call : mCallList.getCallAttributesList()) {
                if (call.getState() == state) {
                    return call;
                }
            }
        }
        return null;
    }

    public void clearCallList() {
        mCallList.clear();
    }

    /**
     * If there is an active call, put it on hold
     *
     * @return true if there is an active call that can be put on hold
     */
    public abstract boolean holdActiveCall();

    /**
     * If there is a dialing call, put it in the alerting state
     *
     * @return true if there is a dialing call that can be set to alerting
     */
    public abstract boolean callDialingToAlerting();

    /**
     * Hang up (and delete from mCallList) an incoming/waiting call
     *
     * @return return true if there is an incoming/waiting call to hang up
     */
    public abstract boolean hangupIncomingOrWaitingCall();

    /**
     * If there is an alerting call, put it in the active state
     *
     * @return true if there is an alerting call that can be set to active
     */
    public abstract boolean callAlertingToActive();

    /**
     * If there is an active call, put it in the end state
     *
     * @return true if there is an active call that can be set to end
     */
    public abstract boolean callActiveToEnd();

    /**
     * If there is an active call and a call on hold, the active call will hang up and the call on
     * hold will be set to active
     *
     * @return true if there is an active call to hang up and a call on hold to set to active
     */
    public abstract boolean hangupFgCallResumeBgCall();

    /**
     * Set call on hold to active
     *
     * @return true if there is a call on hold that can be set to active
     */
    public abstract boolean resumeBgCall();

    /**
     * Set incoming call to active
     *
     * @return true if there is an incoming call that can be set to active
     */
    public abstract boolean answerIncomingCall();

    /**
     * Set waiting call to active
     *
     * @return true if there is an waiting call that can be set to active
     */
    public abstract boolean answerWaitingCall();

    protected void log(String str) {
        Log.d(LOGTAG, str);
    }

    protected void logData(byte[] data, int length) {
        StringBuilder s = new StringBuilder();
        s.append("[");
        for (int i = 0; i < length; i++) {
            s.append(i + ":" + data[i]);
            if (i < length - 1) {
                s.append(", ");
            }
        }
        s.append("]");
        log(s.toString());
    }

}
