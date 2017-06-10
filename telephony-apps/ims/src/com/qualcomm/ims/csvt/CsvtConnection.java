/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 *
 */

package com.qualcomm.ims.csvt;

import com.android.ims.ImsCall;
import com.android.ims.ImsReasonInfo;

import android.util.Log;

/**
 * {@hide}
 */
public class CsvtConnection {
    protected final String LOG_TAG = "CsvtConnection";

    public enum State {
        IDLE, ACTIVE, DIALING, ALERTING, INCOMING, WAITING, DISCONNECTED, DISCONNECTING;

        public boolean isAlive() {
            return !(this == IDLE || this == DISCONNECTED || this == DISCONNECTING);
        }

        public boolean isRinging() {
            return this == INCOMING || this == WAITING;
        }
    }

    public State mState = State.IDLE;

    private CsvtCallTracker mCt;
    private ImsCall mCall;
    private int mCallFailCause = CsvtConstants.CALL_STATUS_CONNECTED;
    boolean mIsAlerting = false;

    CsvtConnection(CsvtCallTracker ct, ImsCall call, State state) {
        this(ct, call);
        setState(state);
    }

    CsvtConnection(CsvtCallTracker ct, ImsCall call) {
        mCt = ct;
        mCall = call;
    }

    public State getState() {
        return mState;
    }

    public boolean isRinging() {
        return mState.isRinging();
    }

    public boolean isAlive() {
        return mState.isAlive();
    }

    public void setState(State state) {
        if (mState != state) {
            mState = state;
            if (isRinging()) {
                mCt.notifyNewRingingConnection(this);
            }

            // Notify alerting state start and end for ring back tone.
            if (mState == State.ALERTING) {
                mIsAlerting = true;
                mCt.notifyAlertingState(mIsAlerting);
            } else if (mIsAlerting) {
                mIsAlerting = false;
                mCt.notifyAlertingState(mIsAlerting);
            }
            // For disconnected state, notifyDisconnected will be called instead
            if (mState != State.DISCONNECTED) {
                mCt.onConnectionStateChanged(this);
            }
        }
    }

    public ImsCall getCall() {
        return mCall;
    }

    public void setCall(ImsCall call) {
        mCall = call;
    }

    public int getDisconnectStatus() {
        return mCallFailCause;
    }

    public void onDisconnect(ImsReasonInfo reasonInfo) {
        if (mState != State.DISCONNECTED) {
            mCallFailCause = convertImsReasonToDisconnectStatus(reasonInfo);
            if (mState.isRinging() && (mCallFailCause
                    == CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL ||
                    mCallFailCause == CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL_UNSPECIFIED)) {
                mCallFailCause = CsvtConstants.CALL_STATUS_DISCONNECTED_INCOMING_MISSED;
            }
            setState(State.DISCONNECTED);
            mCt.notifyDisconnected(this);
        }
    }

    private int convertImsReasonToDisconnectStatus(ImsReasonInfo reason) {
        if (reason != null) {
            switch (reason.mCode) {
                case ImsReasonInfo.CODE_USER_TERMINATED:
                    return CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL;
                case ImsReasonInfo.CODE_UNSPECIFIED:
                default:
                    return convertImsErrorInfoToCsvtDisconnectStatus(reason.mExtraMessage);
            }
        }
        return CsvtConstants.CALL_STATUS_DISCONNECTED_ERROR_UNSPECIFIED;
    }

    private int convertImsErrorInfoToCsvtDisconnectStatus(String errorInfo) {
        int extraCode = -1;
        if (errorInfo != null) {
            try {
                extraCode = Integer.parseInt(errorInfo);
            } catch(Exception ex) {
                Log.e(LOG_TAG, "Exception while parsing error info " + ex);
            }
        }
        switch (extraCode) {
            case CallFailCause.LOCAL_PHONE_OUT_OF_3G_Service:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_LCOAL_PHONE_OUT_OF_3G_SERVICE;
            case CallFailCause.USER_ALERTING_NO_ANSWER:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NO_ANSWER;
            case CallFailCause.INCOMPATIBILITY_DESTINATION:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_INCOMPATIBILITY_DESTINATION;
            case CallFailCause.RESOURCES_UNAVAILABLE:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_RESOURCES_UNAVAILABLE;
            case CallFailCause.BEARER_NOT_AUTHORIZATION:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_NOT_AUTHORIZATION;
            case CallFailCause.BEARER_NOT_AVAIL:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_NOT_AVAIL;
            case CallFailCause.NUMBER_CHANGED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NUMBER_CHANGED;
            case CallFailCause.NORMAL_UNSPECIFIED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL_UNSPECIFIED;
            case CallFailCause.PROTOCOL_ERROR_UNSPECIFIED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_PROTOCOL_ERROR_UNSPECIFIED;
            case CallFailCause.BEARER_SERVICE_NOT_IMPLEMENTED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_SERVICE_NOT_IMPLEMENTED;
            case CallFailCause.SERVICE_OR_OPTION_NOT_IMPLEMENTED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_SERVICE_OR_OPTION_NOT_IMPLEMENTED;
            case CallFailCause.NO_USER_RESPONDING:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NO_USER_RESPONDING;
            case CallFailCause.NORMAL_CLEARING:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL;
            case CallFailCause.USER_BUSY:
            case CallFailCause.CALL_REJECTED:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_BUSY;
            case CallFailCause.INVALID_NUMBER:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_INVALID_NUMBER;
            case CallFailCause.NO_CIRCUIT_AVAIL:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_NETWORK_CONGESTION;
            case CallFailCause.UNOBTAINABLE_NUMBER:
                return CsvtConstants.CALL_STATUS_DISCONNECTED_UNASSIGNED_NUMBER;

        }
        return CsvtConstants.CALL_STATUS_DISCONNECTED_ERROR_UNSPECIFIED;
    }

    @Override
    public String toString() {
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("CsvtConnection: state = ");
        strBuilder.append(mState);
        strBuilder.append(" Call = ");
        strBuilder.append(mCall);
        strBuilder.append(" fail cause code = ");
        strBuilder.append(mCallFailCause);
        return strBuilder.toString();
    }
}
