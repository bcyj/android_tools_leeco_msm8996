/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
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

import android.app.Service;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.Registrant;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.content.Context;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import com.android.ims.ImsCallProfile;
import com.android.ims.ImsException;
import com.android.ims.ImsUtInterface;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;

import com.qualcomm.ims.csvt.CsvtConstants;
import com.qualcomm.ims.csvt.CsvtIntents;

import org.codeaurora.ims.csvt.ICsvtServiceListener;
import org.codeaurora.ims.csvt.ICsvtService;
import org.codeaurora.ims.csvt.CallForwardInfoP;
import org.codeaurora.ims.ImsPhoneCommandsInterface;

import java.util.ArrayList;
import java.util.List;


public class CsvtService extends Service {

    private static final boolean DBG = true;
    private static final boolean VDBG = true;
    private static final String TAG = "CsvtService";

    final private RemoteCallbackList<ICsvtServiceListener> mListeners =
            new RemoteCallbackList<ICsvtServiceListener>();

    private CsvtCallTracker mCt;
    private int mPhoneId = 0;

    private Handler mHandler;
    private int mPhoneCount;
    private TelephonyManager mTelephonyManager;
    final private List<PhoneStateListener> mTelephonyListeners =
            new ArrayList<PhoneStateListener>();
    final private List<Integer> mPhonesStates =
            new ArrayList<Integer>();

    private static final int EVENT_NEW_RINGING_CONNECTION = 1;
    private static final int EVENT_PHONE_STATE_CHANGED = 2;
    private static final int EVENT_CALL_STATUS_CHANGED = 3;
    private static final int EVENT_CALL_DISCONNECTED = 4;
    private static final int EVENT_CALL_ALERTING = 5;

    private static final int EVENT_SET_CALL_WAITING = 6;
    private static final int EVENT_SET_CALL_FORWARDING = 7;
    private static final int EVENT_GET_CALL_WAITING = 8;
    private static final int EVENT_GET_CALL_FORWARDING = 9;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mCt = new CsvtCallTracker(this, mPhoneId);
        createHandler();
        acquireTelephonyManager();
        registerForTelephonyEvents();
        registerForCallEvents();
        Log.d(TAG, "onCreate: Initialization is completed");
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "Csvt Service bound");
        return mBinder;
    }

    private void acquireTelephonyManager() {
        Log.d(TAG, "acquireTelephonyManager");

        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        if (mTelephonyManager == null) {
            Log.e(TAG, "Failed to acquire TelephonyManager");
            return;
        }
        mPhoneCount = mTelephonyManager.getPhoneCount();
    }

    private PhoneStateListener createPhoneStateListener(int phoneId) {
        int subs[] = SubscriptionManager.getSubId(phoneId);
        if (subs != null && subs[0] > SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return new PhoneStateListener(subs[0]) {
                @Override
                public void onCallStateChanged(int s, String number) {
                    int phoneId = SubscriptionManager.getPhoneId(mSubId);
                    Log.d(TAG, "PhoneStateListener: CallState: " + s +
                            " sub = " + mSubId + " phoneId = " + phoneId);
                    if (phoneId >=0 && phoneId < mPhonesStates.size()) {
                        mPhonesStates.set(phoneId, s );
                    }
                }
            };
        }
        return null;
    }

    private void registerForTelephonyEvents() {
        if (mTelephonyManager != null) {
            for (int i = 0; i < mPhoneCount; ++i) {
                mPhonesStates.add(TelephonyManager.CALL_STATE_IDLE);
                PhoneStateListener l = createPhoneStateListener(i);
                if (l != null ) {
                    mTelephonyListeners.add(l);
                    mTelephonyManager.listen(l, PhoneStateListener.LISTEN_CALL_STATE);
                }
            }
        } else {
            Log.e(TAG, "Failed to obtain TelephonyManager");
        }
    }

    private void unregisterFromTelephonyEvents() {
        if (mTelephonyManager != null) {
            for (PhoneStateListener l : mTelephonyListeners) {
                mTelephonyManager.listen(l, PhoneStateListener.LISTEN_NONE);
            }
        }
        mTelephonyListeners.clear();
        mPhonesStates.clear();
    }

    private void createHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                CsvtService.this.handleMessage(msg);
            }
        };
    }

    private void registerForCallEvents() {
        if (mCt == null) {
            Log.e(TAG,"Failed to register for events.");
            return;
        }
        mCt.registerForNewRingingConnection(mHandler, EVENT_NEW_RINGING_CONNECTION
                , null);
        mCt.registerForPhoneStateChanged(mHandler, EVENT_PHONE_STATE_CHANGED, null);
        mCt.registerForCallStatus(mHandler, EVENT_CALL_STATUS_CHANGED, null);
        mCt.registerForDisconnect(mHandler, EVENT_CALL_DISCONNECTED, null);
        mCt.registerForAlertingState(mHandler, EVENT_CALL_ALERTING, null);
    }

    void handleMessage(Message msg) {
        switch ( msg.what ) {
            case EVENT_NEW_RINGING_CONNECTION:
                onNewRingingConnection(msg);
                break;
            case EVENT_PHONE_STATE_CHANGED:
                onPhoneStateChanged(msg);
                break;
            case EVENT_CALL_STATUS_CHANGED:
                onCallStatus(msg);
                break;
            case EVENT_CALL_DISCONNECTED:
                onDisconnect(msg);
                break;
            case EVENT_CALL_ALERTING:
                onAlertingStateChanged(msg);
                break;
            case EVENT_SET_CALL_WAITING:
                onSetCallWaiting(msg);
                break;
            case EVENT_SET_CALL_FORWARDING:
                onSetCallForwarding(msg);
                break;
            case EVENT_GET_CALL_WAITING:
                onGetCallWaiting(msg);
                break;
            case EVENT_GET_CALL_FORWARDING:
                onGetCallForwarding(msg);
                break;
        }

    }

    private void onNewRingingConnection(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        CsvtConnection conn = (CsvtConnection) ar.result;
        String number = "";
        if (conn.getCall() != null && conn.getCall().getCallProfile()
                != null) {
            number =  conn.getCall().getCallProfile()
                    .getCallExtra(ImsCallProfile.EXTRA_OI);
            String tmp = (VDBG? number: "xxxxx");
            Log.d(TAG, "New incoming call from: " + tmp);
        }
        Intent i = new Intent(CsvtIntents.ACTION_NEW_CSVT_RINGING_CONNECTION);
        i.putExtra(CsvtConstants.CONNECTION_ADDRESS_KEY, number);
        sendBroadcast(i);
    }

    private void onPhoneStateChanged(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        Integer state = (Integer)ar.result;
        Log.d(TAG, "onPhoneStateChanged: state = " + state);
        notifyPhoneStateChanged(state.intValue());
    }

    private void onCallStatus(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        Integer status = (Integer)ar.result;
        Log.d(TAG, "onCallStatus: status = " + status);
        notifyCallStatus(status.intValue());
    }

    private void onDisconnect(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        CsvtConnection conn = (CsvtConnection) ar.result;

        int status = (conn != null) ? conn.getDisconnectStatus():
                CsvtConstants.CALL_STATE_DISCONNECTED;
        if (DBG) Log.d(TAG, "onDisconnect: conn = " + conn + " status = " + status);
        notifyCallStatus(status);
    }

    private void onAlertingStateChanged(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        Boolean isAlerting = (Boolean)ar.result;
        onRingbackTone(isAlerting.booleanValue());
    }

    private void onRingbackTone(final boolean playTone) {
        if (DBG) Log.d(TAG, "onRingbackTone");

        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(ICsvtServiceListener l) throws RemoteException {
                l.onRingbackTone(playTone);
            }
        });
    }

    private void onSetCallWaiting(Message msg) {
        Log.d(TAG, "OnSetCallWaiting:");

        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar.exception != null) {
            Log.d(TAG, "SetCallWaiting: Exception: " + ar.exception);
        }

        sendRequestStatus(ar);
    }

    private void onGetCallWaiting(Message msg) {
        Log.d(TAG, "OnGetCallWaiting:");

        AsyncResult ar = (AsyncResult) msg.obj;

        sendRequestStatus(ar);

        if (ar.exception != null) {
            Log.d(TAG, "GetCallWaiting: Exception: " + ar.exception);
        } else {
            final boolean enabled = CsvtUtils.getCwEnabledFromUtResult(ar.result);
            if (DBG) Log.d(TAG, "GetCallWaiting: Enabled: " + enabled);

            notifyListeners( new INotifyEvent() {
                @Override
                public void onNotify(ICsvtServiceListener l)throws RemoteException {
                    l.onCallWaiting(enabled);
                }
            });
        }
    }

    private void onSetCallForwarding(Message msg) {
        Log.d(TAG, "onSetCallForwarding:");

        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar.exception != null) {
            Log.d(TAG, "onSetCallForwarding: Exception: " + ar.exception);
        }

        sendRequestStatus(ar);

    }

    private void sendErrorStatus(Message onComplete, Exception ex) {
        if (onComplete != null) {
            sendRequestStatus(AsyncResult.forMessage(onComplete, null, ex));
        }
    }

    private void sendRequestStatus(AsyncResult ar) {
        Message uo = (Message) ar.userObj;
        if (uo != null && (uo.replyTo instanceof Messenger) ) {
            uo.arg1 = (ar.exception == null) ? CsvtConstants.ERROR_SUCCESS :
                CsvtConstants.ERROR_FAILED;
            try {
                uo.replyTo.send(uo);
            } catch (RemoteException e) {
                Log.e(TAG, "Reply failed, " + e);
            }
        }
    }

    private void onGetCallForwarding(Message msg) {
        Log.d(TAG, "onGetCallForwarding:");

        AsyncResult ar = (AsyncResult) msg.obj;

        sendRequestStatus(ar);

        if (ar.exception != null) {
            Log.d(TAG, "SetCallForwarding: Exception: " + ar.exception);
        } else {
            notifyCallForwardingOptions(CsvtUtils.getCfInfoFromUtResult(ar.result));
        }
    }

    private void notifyError(int code) {
        notifyCallStatus(code);
    }

    void dial(String number) {
        String tmp = (VDBG? number: "xxxxx");
        if(DBG) Log.d(TAG, "dial: " + tmp);

        final boolean nonCsvtIdle = isNonCsvtIdle();
        final boolean csvtIdle = isIdle();
        if ( ! (nonCsvtIdle || csvtIdle) ) {
            Log.e(TAG, "Cannot dial: "
                    + "nonCsvtIdle: " + nonCsvtIdle
                    + "csvtIdle: " + csvtIdle );
            notifyError(CsvtConstants.DIAL_FAILED);
            return;
        }

        try {
            mCt.dial(number);
        } catch (Exception ex) {
            Log.e(TAG, "Dial failed: " + ex);
            notifyError(CsvtConstants.DIAL_FAILED);
        }
    }

    void hangup() {
        if (DBG) Log.v(TAG, "hangup ");

        try {
            mCt.hangup();
        } catch (Exception e) {
            Log.e(TAG, "hangup failed. " + e);
            notifyError(CsvtConstants.HANGUP_FAILED);
        }
    }

    void rejectCall() {
        if(DBG) Log.v(TAG,"Reject call");

        try {
            mCt.reject(CallFailCause.USER_BUSY);
        } catch (Exception e) {
            Log.e(TAG, "Reject call failed. " + e);
            notifyError(CsvtConstants.REJECT_CALL_FAILED);
        }
    }

    void acceptCall() {
        if(DBG) Log.d(TAG,"acceptCall");

        try {
            mCt.accept();
        } catch(Exception e) {
            Log.e(TAG, "acceptCall failed. " + e);
            notifyError(CsvtConstants.ACCEPT_CALL_FAILED);
        }
    }

    public void fallback() {
        try {
            mCt.reject(CallFailCause.INCOMPATIBILITY_DESTINATION);
        } catch (CallStateException e) {
            Log.e(TAG, "Fallback failed: " + e);
            notifyError(CsvtConstants.FALLBACK_FAILED);
        }
    }

    public void registerListener(ICsvtServiceListener l){
        if(DBG) Log.v(TAG, "Registering listener l = " + l);
        if ( l == null ) {
            Log.e(TAG, "Listener registration failed. Listener is null");
            return;
        }

        synchronized (this) {
            mListeners.register(l);
        }
    }

    public  void unregisterListener(ICsvtServiceListener l){
        if(DBG) Log.v(TAG, "Unregistering listener l = " + l);
        if ( l == null ) {
            Log.e(TAG, "Listener unregistration failed. Listener is null");
            return;
        }

        synchronized (this) {
            mListeners.unregister(l);
        }
    }

    private void notifyCallForwardingOptions(final List<CallForwardInfoP> cfl ) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(ICsvtServiceListener l) throws RemoteException {
                l.onCallForwardingOptions(cfl);
            }
        });
    }

    private void notifyPhoneStateChanged(final int state) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(ICsvtServiceListener l) throws RemoteException {
                l.onPhoneStateChanged(state);
            }
        });
    }

    void notifyCallStatus(final int status) {
        notifyListeners(new INotifyEvent() {
            @Override
            public void onNotify(ICsvtServiceListener l) throws RemoteException {
                l.onCallStatus(status);
            }
        });
    }

    private interface INotifyEvent {
        void onNotify(ICsvtServiceListener l) throws RemoteException;
    }

    void notifyListeners(INotifyEvent n) {
        synchronized (this) {
            mListeners.beginBroadcast();
            final int size = mListeners.getRegisteredCallbackCount();
            for (int i = 0; i < size; ++i) {
                try {
                    n.onNotify( mListeners.getBroadcastItem(i) );
                } catch (RemoteException e) {
                    Log.e(TAG, "Broadcast failed. idx: " + i + " Ex: " + e);
                }
            }
            mListeners.finishBroadcast();
        }
    }

    public boolean isIdle() {
        return (mCt.getPhoneState() == CsvtConstants.CALL_STATE_IDLE);
    }

    public boolean isActive() {
        return mCt.hasActiveCall();
    }

    public boolean isNonCsvtIdle() {
        boolean isIdle = true;
        for (int callState: mPhonesStates) {
            if (callState != TelephonyManager.CALL_STATE_IDLE) {
                isIdle = false;
                break;
            }
        }
        return isIdle;
    }

    public void setCallForwardingOption(int commandInterfaceCFReason,
            int commandInterfaceCFAction,
            String dialingNumber,
            int timerSeconds,
            Message onComplete) {
        Log.d(TAG, "setCallForwardingOption action=" + commandInterfaceCFAction
                + ", reason=" + commandInterfaceCFReason);
        Message resp;
        resp = mHandler.obtainMessage(EVENT_SET_CALL_FORWARDING, onComplete);

        try {
            ImsUtInterface ut = mCt.getUtInterface();
            ut.updateCallForward(CsvtUtils.getUtActionFromCFAction(commandInterfaceCFAction),
                    CsvtUtils.getUtConditionFromCFReason(commandInterfaceCFReason),
                    dialingNumber,
                    ImsPhoneCommandsInterface.SERVICE_CLASS_DATA_SYNC,
                    timerSeconds,
                    resp);
        } catch (ImsException e) {
            Log.e(TAG, "setCallForwardingOption: Exception = " + e);
            sendErrorStatus(onComplete,
                    new CommandException(CommandException.Error.GENERIC_FAILURE));
        }
    }


    public void getCallForwardingOption(int commandInterfaceCFReason,
                Message onComplete) {
        Log.d(TAG, "getCallForwardingOption reason=" + commandInterfaceCFReason);
        Message resp;
        resp = mHandler.obtainMessage(EVENT_GET_CALL_FORWARDING, onComplete);

        try {
            ImsUtInterface ut = mCt.getUtInterface();
            ut.queryCallForward(CsvtUtils.
                    getUtConditionFromCFReason(commandInterfaceCFReason), null,
                    ImsPhoneCommandsInterface.SERVICE_CLASS_DATA_SYNC, resp);
        } catch (ImsException e) {
            Log.e(TAG, "getCallForwardingOption: Exception = " + e);
            sendErrorStatus(onComplete,
                    new CommandException(CommandException.Error.GENERIC_FAILURE));
        }
    }

    public void getCallWaiting(Message onComplete) {
        Log.d(TAG, "getCallWaiting");
        Message resp;
        resp = mHandler.obtainMessage(EVENT_GET_CALL_WAITING, onComplete);

        try {
            ImsUtInterface ut = mCt.getUtInterface();
            ut.queryCallWaiting(resp);
        } catch (ImsException e) {
            Log.e(TAG, "getCallWaiting: Exception = " + e);
            sendErrorStatus(onComplete,
                    new CommandException(CommandException.Error.GENERIC_FAILURE));
        }
    }

    public void setCallWaiting(boolean enable, Message onComplete) {
        Log.d(TAG, "setCallWaiting enable=" + enable);
        Message resp;
        resp = mHandler.obtainMessage(EVENT_SET_CALL_WAITING, onComplete);

        try {
            ImsUtInterface ut = mCt.getUtInterface();
            ut.updateCallWaiting(enable, ImsPhoneCommandsInterface.SERVICE_CLASS_DATA_SYNC, resp);
        } catch (ImsException e) {
            Log.e(TAG, "setCallWaiting: Exception = " + e);
            sendErrorStatus(onComplete,
                    new CommandException(CommandException.Error.GENERIC_FAILURE));
        }
    }

    final private ICsvtService.Stub mBinder = new ICsvtService.Stub() {

        /**
         * Initiate a new Csvt connection. This happens asynchronously, so you
         * cannot assume the audio path is connected (or a call index has been
         * assigned) until PhoneStateChanged notification has occurred.
         */
        public void dial(String number) {
            CsvtService.this.dial(number);
        }

        /**
         * Hang up the foreground call. Reject occurs asynchronously,
         * and final notification occurs via PhoneStateChanged callback.
         */
        public void hangup() {
            CsvtService.this.hangup();
        }

        /**
         * Answers a ringing.
         * Answering occurs asynchronously, and final notification occurs via
         * PhoneStateChanged callback.
         */
        public void acceptCall() {
            CsvtService.this.acceptCall();
        }

        /**
         * Reject (ignore) a ringing call. In GSM, this means UDUB
         * (User Determined User Busy). Reject occurs asynchronously,
         * and final notification occurs via  PhoneStateChanged callback.
         */
        public void rejectCall() {
            CsvtService.this.rejectCall();
        }

        /**
         * Reject (ignore) a ringing call and sends Incompatible Destination
         * fail cause to the remote party. Reject occurs asynchronously,
         * and final notification occurs via  PhoneStateChanged callback.
         */
        public void fallBack() {
            CsvtService.this.fallback();
        }

        /**
         * Checks if there is an active or ringing Csvt call.
         * @return false if there is an active or ringing Csvt call.
         */
        public boolean isIdle() {
            return CsvtService.this.isIdle();
        }

        /**
         * Checks if there is an active Csvt call.
         * @return true if there is an active Csvt call.
         */
        public boolean isActive() {
            return CsvtService.this.isActive();
        }


        /**
         * Checks if all non-Csvt calls are idle.
         * @return true if all non-Csvt calls are idle.
         */
        public boolean isNonCsvtIdle() {
            return CsvtService.this.isNonCsvtIdle();
        }

        /**
         * getCallForwardingOptions
         * Call Forwarding options are returned via
         * ICsvtServiceListener.onCallForwardingOptions callback.
         *
         * @param commandInterfaceCFReason is one of the valid call forwarding
         *        CF_REASONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param onComplete a callback message when the action is completed.
         *        onComplete.arg1 is set to zero (0) if the action is completed
         *        successfully.
         * @see   ICsvtServiceListener.onCallForwardingOptions
         */
        public void getCallForwardingOption(int commandInterfaceCFReason,
                                     Message onComplete) {
            CsvtService.this.getCallForwardingOption(commandInterfaceCFReason
                    ,onComplete);
        }

        /**
         * setCallForwardingOptions
         * sets a call forwarding option.
         *
         * @param commandInterfaceCFReason is one of the valid call forwarding
         *        CF_REASONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param commandInterfaceCFAction is one of the valid call forwarding
         *        CF_ACTIONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param dialingNumber is the target phone number to forward calls to
         * @param timerSeconds is used by CFNRy to indicate the timeout before
         *        forwarding is attempted.
         * @param onComplete a callback message when the action is completed.
         *        onComplete.arg1 is set to zero (0) if the action is completed
         *        successfully.
         */
        public void setCallForwardingOption(int commandInterfaceCFReason,
                                     int commandInterfaceCFAction,
                                     String dialingNumber,
                                     int timerSeconds,
                                     Message onComplete) {
            CsvtService.this.setCallForwardingOption(commandInterfaceCFReason,
                    commandInterfaceCFAction, dialingNumber, timerSeconds,
                    onComplete);
        }

        /**
         * getCallWaiting
         * gets call waiting activation state. The call waiting activation state
         * is returned via ICsvtServiceListener.onCallWaiting callback.
         *
         * @param onComplete a callback message when the action is completed.
         *        onComplete.arg1 is set to zero (0) if the action completed
         *        successfully.
         * @see   ICsvtServiceListener.onCallWaiting
         */
        public void getCallWaiting(Message onComplete) {
            CsvtService.this.getCallWaiting(onComplete);
        }

        /**
         * setCallWaiting
         * sets a call forwarding option.
         *
         * @param enable is a boolean representing the state that you are
         *        requesting, true for enabled, false for disabled.
         * @param onComplete a callback message when the action is completed.
         *        onComplete.arg1 is set to zero (0) if the action is completed
         *        successfully.
         */
        public void setCallWaiting(boolean enable, Message onComplete) {
            CsvtService.this.setCallWaiting(enable, onComplete);
        }

        public void registerListener(ICsvtServiceListener l){
            CsvtService.this.registerListener(l);
        }

        public  void unregisterListener(ICsvtServiceListener l){
            CsvtService.this.unregisterListener(l);
        }

    };
}

