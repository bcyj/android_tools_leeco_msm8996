/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2013 The Android Open Source Project
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

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.preference.PreferenceManager;
import android.telecom.VideoProfile;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.util.Log;

import com.android.ims.ImsCall;
import com.android.ims.ImsCallProfile;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsReasonInfo;
import com.android.ims.ImsServiceClass;
import com.android.ims.ImsUtInterface;
import com.android.internal.telephony.CallStateException;

import org.codeaurora.ims.CallDetails;


/**
 * {@hide}
 */
public class CsvtCallTracker extends Handler {

    private static final String TAG = "CsvtCallTracker";

    private static final String ACTION_NEW_CSVT_RINGING_CALL =
            "org.codeaurora.ims.csvt.NEW_CSVT_RINGING_CALL";

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive : intent = " + intent);
            if (intent.getAction().equals(ACTION_NEW_CSVT_RINGING_CALL)) {
                CsvtCallTracker.this.sendMessage(
                        obtainMessage(EVENT_NEW_INCOMING_RINGING, intent));
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_UP)) {
                int phoneId = -1;
                if (intent.hasExtra(ImsManager.EXTRA_PHONEID)) {
                    phoneId = intent.getIntExtra(ImsManager.EXTRA_PHONEID, -1);
                }
                Log.d(TAG, "onReceive : intent phoneId = " + phoneId + " mPhoneId = " + mPhoneId);
                if (phoneId != -1 && phoneId != mPhoneId) {
                    cleanupCalls();
                    mImsManager = null;
                    mPhoneId = phoneId;
                }
                initializeImsService();
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_DOWN)) {
                cleanupCalls();
                mImsManager = null;
            }
        }
    };

    private CsvtConnection mRingingConn;
    private CsvtConnection mForegroundConn;
    private Context mContext;
    private int mState = CsvtConstants.CALL_STATE_IDLE;
    private ImsManager mImsManager;
    private int mPhoneId = 0;
    private int mServiceId = -1;

    private final RegistrantList mPhoneStateRegistrants
            = new RegistrantList();
    private final RegistrantList mCallStatusRegistrants
            = new RegistrantList();
    private final RegistrantList mNewRingingConnectionRegistrants
            = new RegistrantList();
    private final RegistrantList mDisconnectRegistrants
            = new RegistrantList();
    private final RegistrantList mAlertingStateRegistrants
            = new RegistrantList();

    private static final int EVENT_NEW_INCOMING_RINGING = 0;
    private static final int EVENT_CALL_STATE_CHANGED   = 1;
    private static final int EVENT_CALL_DISCONNECTED    = 2;
    private static final int EVENT_DIAL_REQUEST = 3;
    private static final int EVENT_OPEN_IMS_SERVICE_REQ = 4;

    private static class CallMsg {
        private ImsCall call;
        private CsvtConnection.State state;
        private ImsReasonInfo reasonInfo;
    }

    //***** Constructors

    CsvtCallTracker(Context context, int phoneId) {
        if (context == null) {
            Log.e(TAG, "Cannot construct with null context!");
            return;
        }

        mContext = context;
        mPhoneId = phoneId;

        IntentFilter intentfilter = new IntentFilter();
        intentfilter.addAction(ACTION_NEW_CSVT_RINGING_CALL);
        intentfilter.addAction(ImsManager.ACTION_IMS_SERVICE_UP);
        intentfilter.addAction(ImsManager.ACTION_IMS_SERVICE_DOWN);
        mContext.registerReceiver(mReceiver, intentfilter);

        initializeImsService();
    }

    private PendingIntent createIncomingCallPendingIntent() {
        Intent intent = new Intent(ACTION_NEW_CSVT_RINGING_CALL);
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        return PendingIntent.getBroadcast(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
    }

    private void initializeImsService() {
        sendMessage(obtainMessage(EVENT_OPEN_IMS_SERVICE_REQ));
    }

    private void openImsService() {
        Log.d(TAG, "openImsService");
        if (mImsManager != null) {
            Log.d(TAG, "openImsService: ImsManager already opened");
            return;
        }

        mImsManager = ImsManager.getInstance(mContext, mPhoneId);
        try {
            mServiceId = mImsManager.open(ImsServiceClass.CSVT,
                    createIncomingCallPendingIntent(),
                    mImsConnectionStateListener);
            Log.d(TAG, "openImsService: serviceid = " + mServiceId);
        } catch (ImsException e) {
            Log.e(TAG, "openImsService: " + e);
            mImsManager = null;
        }
    }

    public void dispose() {
        Log.i(TAG, "dispose");
        mContext.unregisterReceiver(mReceiver);
    }

    public int getPhoneState() {
        return mState;
    }

    public void registerForPhoneStateChanged(Handler h, int what, Object obj) {
        mPhoneStateRegistrants.addUnique(h, what, obj);
    }

    public void unegisterForPhoneStateChanged(Handler h) {
        mPhoneStateRegistrants.remove(h);
    }

    private void notifyPhoneStateChanged(int state) {
        mPhoneStateRegistrants.notifyResult(new Integer(state));
    }

    public void registerForCallStatus(Handler h, int what, Object obj) {
        mCallStatusRegistrants.addUnique(h, what, obj);
    }

    public void unregisterForCallStatus(Handler h) {
        mCallStatusRegistrants.remove(h);
    }

    private void notifyCallStatus(int status) {
        mCallStatusRegistrants.notifyResult(new Integer(status));
    }

    public void registerForNewRingingConnection(
            Handler h, int what, Object obj) {
        mNewRingingConnectionRegistrants.addUnique(h, what, obj);
    }

    public void unregisterForNewRingingConnection(Handler h) {
        mNewRingingConnectionRegistrants.remove(h);
    }

    /*package*/ void notifyNewRingingConnection(CsvtConnection conn) {
        mNewRingingConnectionRegistrants.notifyResult(conn);
    }

    public void registerForDisconnect(Handler h, int what, Object obj) {
        mDisconnectRegistrants.addUnique(h, what, obj);
    }

    public void unregisterForDisconnect(Handler h) {
        mDisconnectRegistrants.remove(h);
    }

    /*package*/ void notifyDisconnected(CsvtConnection conn) {
        mDisconnectRegistrants.notifyResult(conn);
    }

    public void registerForAlertingState(Handler h, int what, Object obj) {
        mAlertingStateRegistrants.addUnique(h, what, obj);
    }

    public void unregisterForAlertingState(Handler h) {
        mAlertingStateRegistrants.remove(h);
    }

    /*package*/ void notifyAlertingState(boolean isAlerting) {
        mAlertingStateRegistrants.notifyResult(new Boolean(isAlerting));
    }

    private void updatePhoneState() {
        int newState = CsvtConstants.CALL_STATE_IDLE;
        if (mRingingConn != null && mRingingConn.isRinging()) {
            newState = CsvtConstants.CALL_STATE_RINGING;
        } else if (mForegroundConn != null && mForegroundConn.isAlive()) {
            newState = CsvtConstants.CALL_STATE_OFFHOOK;
        }

        Log.i(TAG, "updatePhonState: Old state = " + mState + " new state = " + newState);
        if (newState != mState) {
            mState = newState;
            notifyPhoneStateChanged(mState);
        }
    }

    /*package*/ void onConnectionStateChanged(CsvtConnection conn) {
        if (conn != null && conn == mForegroundConn
                && mForegroundConn.getState() == CsvtConnection.State.ACTIVE) {
            notifyCallStatus(CsvtConstants.CALL_STATUS_CONNECTED);
        }
        updatePhoneState();
    }

    @Override
    protected void finalize() {
        Log.i(TAG, "CsvtCallTracker finalized");
    }

    synchronized void dial(String dialString) {
        Log.d(TAG, "dial");

        if (mImsManager == null) {
            Log.e(TAG, "dial: Ims service not opened");
            notifyCallStatus(CsvtConstants.DIAL_FAILED);
        } else {
            sendMessage(obtainMessage(EVENT_DIAL_REQUEST, dialString));
        }
    }

    private CsvtConnection onDial(String dialString) {
        mForegroundConn = null;
        try {
            String[] numbers = new String[] { dialString };
            ImsCallProfile profile = mImsManager.createCallProfile(mServiceId,
                    ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VT);
            profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
            profile.setCallExtraInt(ImsCallProfile.EXTRA_CALL_DOMAIN,
                    CallDetails.CALL_DOMAIN_CS);
            ImsCall call = mImsManager.makeCall(mServiceId, profile, numbers,
                    mImsCallListener);
            Log.i(TAG, "makeCall: " + call);
            mForegroundConn = new CsvtConnection(this, call);
            mForegroundConn.setState(CsvtConnection.State.DIALING);
        } catch (Exception e) {
            Log.e(TAG, "dial: " + e);
        }
        if (mForegroundConn == null) {
            notifyCallStatus(CsvtConstants.DIAL_FAILED);
        }
        return mForegroundConn;
    }


    void accept() throws CallStateException {
        Log.d(TAG, "acceptCall: ringin conn = " + mRingingConn);

        if (mRingingConn == null || mRingingConn.getCall() == null) {
            throw new CallStateException("No ringing call to accept!");
        }

        try {
            ImsCall call = mRingingConn.getCall();
            call.accept(ImsCallProfile.CALL_TYPE_VT);
        } catch (ImsException e) {
            throw new CallStateException("Failed to accept call. " + e);
        }
    }

    void reject(int reason) throws CallStateException {
        Log.d(TAG, "reject reason =" + reason);

        if (mRingingConn == null || mRingingConn.getCall() == null) {
            mRingingConn = null;
            throw new CallStateException("No ringing call to reject!");
        }

        try {
            ImsCall call = mRingingConn.getCall();
            call.reject(reason);
        } catch (ImsException e) {
            throw new CallStateException("reject failed. " + e);
        }
    }

    void hangup() throws CallStateException {
        Log.d(TAG, "hangup");
        CsvtConnection conn = null;
        if (mRingingConn != null && mRingingConn.getCall() != null) {
            conn = mRingingConn;
        } else if (mForegroundConn != null && mForegroundConn.getCall() != null) {
            conn = mForegroundConn;
        }

        if (conn == null ) {
            throw new CallStateException("No valid call to hangup!");
        }

        hangup(conn);
    }

    private void hangup(CsvtConnection conn) throws CallStateException {
        Log.d(TAG, "hangup conn:" + conn);

        if (conn == null || conn.getCall() == null) {
            throw new CallStateException("No valid call to hangup!");
        }

        try {
            ImsCall call = conn.getCall();
            call.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        } catch (ImsException e) {
            throw new CallStateException("hangup failed. " + e);
        }
    }

    public boolean hasActiveCall() {
        return (mForegroundConn != null &&
                mForegroundConn.mState == CsvtConnection.State.ACTIVE);
    }

    private CsvtConnection findConnFromCall(ImsCall call) {
        CsvtConnection conn = null;
        if (mRingingConn != null && mRingingConn.getCall() == call) {
            conn = mRingingConn;
        } else if (mForegroundConn != null &&
                mForegroundConn.getCall() == call) {
            conn = mForegroundConn;
        }
        return conn;
    }

    @Override
    public void handleMessage (Message msg) {
        Log.d(TAG, "handleMessage what=" + msg.what);
        CallMsg callObj;
        switch(msg.what) {
            case EVENT_NEW_INCOMING_RINGING:
                onNewIncomingIntent((Intent)msg.obj);
                break;

            case EVENT_CALL_STATE_CHANGED:
                callObj = (CallMsg)msg.obj;
                onCallStateChanged(callObj.call, callObj.state);
                break;

            case EVENT_CALL_DISCONNECTED:
                callObj = (CallMsg)msg.obj;
                onCallDisconnected(callObj.call, callObj.reasonInfo);
                break;

            case EVENT_DIAL_REQUEST:
                onDial((String)msg.obj);
                break;

            case EVENT_OPEN_IMS_SERVICE_REQ:
                openImsService();
                break;
        }

    }

    private void onNewIncomingIntent(Intent intent) {

        if (mImsManager == null || mServiceId < 0) {
            Log.i(TAG, "onNewIncomingIntent: ImsManager not initialised");
        }

        try {
            ImsCall call = mImsManager.takeCall(mServiceId, intent, mImsCallListener);

            CsvtConnection.State state = (mForegroundConn != null)?
                    CsvtConnection.State.WAITING:
                    CsvtConnection.State.INCOMING;
            mRingingConn = new CsvtConnection(CsvtCallTracker.this, call);
            Log.i(TAG, "onNewIncomingIntent: new ringing conn:" + mRingingConn
                    + " state:" + state);
            mRingingConn.setState(state);
        } catch (Exception e) {
            Log.e(TAG, "onReceive : exception " + e);
        }

    }

    private void onCallStateChanged(ImsCall call, CsvtConnection.State state) {
        Log.i(TAG, "onCallStateChanged: call = " + call + " state = " + state);
        CsvtConnection conn = findConnFromCall(call);
        Log.i(TAG, "onCallStateChanged: ringingConn = " + mRingingConn +
                " fgConn = "  + mForegroundConn +
                " changed conn = " + conn);

        if (conn == null) {
            Log.e(TAG, "onCallStateChanged: No connection found!");
            return;
        }

        if (state == CsvtConnection.State.ACTIVE) {
            if (conn == mRingingConn) {
                Log.i(TAG, "onCallStateChanged: Ringing call moved to active");
                mForegroundConn = conn;
                mRingingConn = null;
            }
        }

        if (mRingingConn != null) {
            Log.e(TAG, "onCallStateChanged: Ringing connection still available!");
            /**
             * onCallStateChanged is called only with ACTIVE or ALERTING.
             * So a ringing connection should not be available at this point.
             */
            mRingingConn = null;
        }
        //setState calls callStateChanged internally
        conn.setState(state);
    }

    private void onCallDisconnected(ImsCall call, ImsReasonInfo reasonInfo) {
        Log.i(TAG, "onCallDisconnected: call = " + call + " reason = " + reasonInfo);
        CsvtConnection conn = findConnFromCall(call);
        Log.i(TAG, "onCallDisconnected: ringingConn = " + mRingingConn +
                " fgConn = "  + mForegroundConn +
                " changed conn = " + conn);
        if (conn == null) {
            Log.e(TAG, "onCallDisconnected: No connection found!");
            return;
        }

        conn.onDisconnect(reasonInfo);
        if (conn == mRingingConn) {
            Log.i(TAG, "onCallDisconnected: Ringing call disconnected.");
            mRingingConn = null;
        } else if (conn == mForegroundConn) {
            Log.i(TAG, "onCallDisconnected: FG call disconnected.");
            mForegroundConn = null;
        }
        updatePhoneState();
    }

    private void sendCallStateChangedEvent(ImsCall call, CsvtConnection.State state) {
        CallMsg msgObj = new CallMsg();
        msgObj.call = call;
        msgObj.state = state;
        sendMessage(obtainMessage(EVENT_CALL_STATE_CHANGED, msgObj));
    }

    private void sendCallDisconnectedEvent(ImsCall call, ImsReasonInfo reasonInfo) {
        CallMsg msgObj = new CallMsg();
        msgObj.call = call;
        msgObj.reasonInfo = reasonInfo;
        sendMessage(obtainMessage(EVENT_CALL_DISCONNECTED, msgObj));
    }

    /**
     * Cleanup calls when IMS service is down.
     */
    private void cleanupCalls() {
        mRingingConn = null;
        mForegroundConn = null;
        updatePhoneState();
    }

    /**
     * Listen to the call state changes
     */
    private ImsCall.Listener mImsCallListener = new ImsCall.Listener() {
        @Override
        public void onCallProgressing(ImsCall imsCall) {
            Log.d(TAG, "onCallProgressing");

            sendCallStateChangedEvent(imsCall, CsvtConnection.State.ALERTING);

        }

        @Override
        public void onCallStarted(ImsCall imsCall) {
            Log.d(TAG, "onCallStarted");

            sendCallStateChangedEvent(imsCall, CsvtConnection.State.ACTIVE);
        }

        @Override
        public void onCallStartFailed(ImsCall imsCall, ImsReasonInfo reasonInfo) {
            Log.d(TAG, "onCallStartFailed reasonCode=" + reasonInfo.getCode());

            sendCallDisconnectedEvent(imsCall, reasonInfo);
        }

        @Override
        public void onCallTerminated(ImsCall imsCall, ImsReasonInfo reasonInfo) {
            Log.d(TAG, "onCallTerminated reasonCode=" + reasonInfo.getCode());
            sendCallDisconnectedEvent(imsCall, reasonInfo);
        }

        @Override
        public void onCallUpdated(ImsCall imsCall) {
            Log.d(TAG, "onCallUpdated");
        }

    };

        /**
     * Listen to the IMS service state change
     *
     */
    private ImsConnectionStateListener mImsConnectionStateListener =
        new ImsConnectionStateListener() {
        @Override
        public void onImsConnected() {
            Log.d(TAG, "onImsConnected");
        }

        @Override
        public void onImsProgressing() {
            Log.d(TAG, "onImsProgressing");
        }

        @Override
        public void onImsDisconnected(ImsReasonInfo imsReasonInfo) {
            Log.d(TAG, "onImsDisconnected imsReasonInfo:" + imsReasonInfo);
        }

        @Override
        public void onImsResumed() {
            Log.d(TAG, "onImsResumed");
        }

        @Override
        public void onImsSuspended() {
            Log.d(TAG, "onImsSuspended");
        }

    };


    /* package */
    ImsUtInterface getUtInterface() throws ImsException {
        if (mImsManager == null) {
            throw new ImsException("No ims manager", ImsReasonInfo.CODE_UNSPECIFIED);
        }

        ImsUtInterface ut = mImsManager.getSupplementaryServiceConfiguration(mServiceId);
        return ut;
    }

}
