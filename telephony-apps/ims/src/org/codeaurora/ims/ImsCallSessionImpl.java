/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2013 The Android Open Source Project
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

import org.codeaurora.ims.conference.ConfInfo;


import com.android.ims.ImsCallProfile;
import com.android.ims.ImsConferenceState;
import com.android.ims.ImsReasonInfo;
import com.android.ims.ImsServiceClass;
import com.android.ims.ImsStreamMediaProfile;
import com.android.ims.ImsSuppServiceNotification;
import com.android.ims.internal.ImsCallSession;
import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsCallSessionListener;
import com.android.ims.internal.IImsVideoCallProvider;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.gsm.SuppServiceNotification;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

import com.qualcomm.ims.vt.ImsVideoCallProviderImpl;

import android.telecom.Connection;

public class ImsCallSessionImpl extends IImsCallSession.Stub {
    private static final String LOG_TAG = "ImsCallSessionImpl";
    private static final int EVENT_DIAL = 1;
    private static final int EVENT_ACCEPT = 2;
    private static final int EVENT_HANGUP = 3;
    private static final int EVENT_HOLD = 4;
    private static final int EVENT_RESUME = 5;
    private static final int EVENT_CONFERENCE = 6;
    private static final int EVENT_REJECT = 7;
    private static final int EVENT_DEFLECT = 8;
    private static final int EVENT_ADD_PARTICIPANT = 9;
    private static final int EVENT_RINGBACK_TONE = 10;
    private static final int EVENT_REMOVE_PARTICIPANT = 11;
    private static final int EVENT_CLOSE_SESSION = 12;

    public static final int SUPP_SVC_CODE_INVALID   = -1;
    public static final int SUPP_SVC_CODE_MT_HOLD   = 2;
    public static final int SUPP_SVC_CODE_MT_RESUME = 3;

    private ImsSenderRxr mCi;
    private DriverCallIms mDc = null;
    private int mCallId = 0;
    private Context mContext;
    private ImsCallProfile mLocalCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
    private ImsCallProfile mRemoteCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
    private ImsCallProfile mCallProfile = new ImsCallProfile();
    private ImsCallSessionListenerProxy mListenerProxy = new ImsCallSessionListenerProxy();
    private int mState = ImsCallSession.State.IDLE;
    private boolean mInCall;
    private Handler mHandler = new ImsCallSessionImplHandler();
    private String mCallee = null; //Remote party's number
    protected boolean mIsConfInProgress = false;
    private ImsCallSessionImpl newSession = null; // new session for conference calls
    private int mDisconnCause = ImsReasonInfo.CODE_UNSPECIFIED;
    private int mSuppSvcCode = SUPP_SVC_CODE_INVALID;
    private ConfInfo mConfInfo = null;
    private ImsConferenceState mImsConferenceState = null;
    private boolean mRingbackToneRequest = false;

    private ImsCallModification mImsCallModification;
    private ImsVideoCallProviderImpl mImsVideoCallProviderImpl;

    private List<Listener> mListeners = new CopyOnWriteArrayList<>();
    private boolean mIsVtGloballyAllowed = false;

    private ImsServiceClassTracker mTracker = null;
    private ImsCallSessionImpl mConfCallSession = null;

    public interface Listener {
        void onDisconnected(ImsCallSessionImpl session);
        void onClosed(ImsCallSessionImpl session);
        void onUnsolCallModify(ImsCallSessionImpl session, CallModify callModify);
    }

    //Contructor for MO Call
    public ImsCallSessionImpl(ImsCallProfile profile, IImsCallSessionListener listener,
            ImsSenderRxr senderRxr, Context context, ImsServiceClassTracker tracker) {
        mCi = senderRxr;
        setListener(listener);
        mListenerProxy.mListener = listener;
        mCallProfile = profile;
        mContext = context;
        mTracker = tracker;
        mConfInfo = new ConfInfo();

        mImsCallModification = new ImsCallModification(this, mCi);
        // TODO (ims-vt) Maybe check VT capabilities before instantiating this class.
        mImsVideoCallProviderImpl = new ImsVideoCallProviderImpl(this, mImsCallModification);
        addListener(mImsVideoCallProviderImpl);
        mCi.registerForRingbackTone(mHandler, EVENT_RINGBACK_TONE, null);
    }

    // Constructor for MT call and Conference Call
    public ImsCallSessionImpl(DriverCallIms call, ImsSenderRxr senderRxr, Context context,
            ImsServiceClassTracker tracker) {
        mCi = senderRxr;
        //TODO update member variables in this class based on dc
        mDc = new DriverCallIms(call);
        mCallId = mDc.index;
        mContext = context;
        mTracker = tracker;
        mCallee = call.number;
        updateImsCallProfile(mDc);

        setCapabilitiesInProfiles(mDc);

        mConfInfo = new ConfInfo();

        // TODO (ims-vt) Maybe check VT capabilities before instantiating this class.
        mImsCallModification = new ImsCallModification(this, mCi);
        mImsVideoCallProviderImpl = new ImsVideoCallProviderImpl(this, mImsCallModification);
        addListener(mImsVideoCallProviderImpl);
    }

    /**
     * Registers call listener.
     * @param listener Listener to registered
     * @see ImsCallSessionImpl#Listener
     */
    public void addListener(Listener listener) {
        if (!isSessionValid()) return;

        if (listener == null) {
            throw new IllegalArgumentException("listener is null.");
        }

        // Note: This will do linear search, O(N).
        // This is acceptable since arrays size is small.
        synchronized (mListeners) {
            if (!mListeners.contains(listener)) {
                mListeners.add(listener);
            } else {
                loge("Duplicate listener, " + listener);
            }
        }
    }

    /**
     * Unregisters call listener.
     * @param listener Listener to unregistered
     * @see ImsCallSessionImpl#Listener
     */
    public void removeListener(Listener listener) {
        if (!isSessionValid()) return;

        if (listener == null) {
            throw new IllegalArgumentException("listener is null.");
        }

        // Note: This will do linear search, O(N).
        // This is acceptable since arrays size is small.
        synchronized (mListeners) {
            if (mListeners.contains(listener)) {
                mListeners.remove(listener);
            } else {
                loge("Listener not found, " + listener);
            }
        }
    }

    public ImsVideoCallProviderImpl getImsVideoCallProviderImpl() {
        if (!isSessionValid()) return null;
        return mImsVideoCallProviderImpl;
    }

    private void notifySessionDisconnected() {
        synchronized (mListeners) {
            for( Listener l : mListeners) {
                l.onDisconnected(this);
            }
        }
    }

    private void notifySessionClosed() {
        synchronized (mListeners) {
            for( Listener l : mListeners) {
                l.onClosed(this);
            }
        }
    }

    public void notifyUnsolCallModify(CallModify callModify) {
        if (!isSessionValid()) return;

        synchronized (mListeners) {
            for (Listener l : mListeners) {
                l.onUnsolCallModify(this, callModify);
            }
        }
    }

    public ImsCallModification getImsCallModification() {
        if (!isSessionValid()) return null;
        return mImsCallModification;
    }

    private boolean isServiceAllowed(int srvType, ServiceStatus[] ability) {
        boolean allowed = true;
        if (ability != null) {
            for (ServiceStatus srv : ability) {
                if (srv != null && srv.type == srvType) {
                    if (srv.status == ImsQmiIF.STATUS_DISABLED ||
                            srv.status == ImsQmiIF.STATUS_NOT_SUPPORTED) {
                        allowed = false;
                    }
                    break;
                }
            }
        }
        return allowed;
    }

    /**
     * Utility to get restrict cause from Service Status Update
     * @param srvType - VoLTE, VT
     * @param ability - Consolidated Service Status Ability
     * @return cause - restrict cause for the service type
     */
    private int getRestrictCause(int srvType, ServiceStatus[] ability) {
        int cause = CallDetails.CALL_RESTRICT_CAUSE_NONE;
        if (ability != null) {
            for (ServiceStatus srv : ability) {
                if (srv != null && srv.type == srvType && srv.accessTechStatus != null &&
                        srv.accessTechStatus.length > 0) {
                    cause = srv.accessTechStatus[0].restrictCause;
                    break;
                }
            }
        }
        return cause;
    }

    /**
     * Update local driver call copy with the new update
     * @param dcUpdate - the new Driver Call Update
     * @return true if the update is different from previous update
     */
    private boolean updateLocalDc(DriverCallIms dcUpdate) {
        boolean hasChanged = false;
        if (mDc == null) {
            mDc = new DriverCallIms(dcUpdate);
            hasChanged = true;
        } else {
            hasChanged = mDc.update(dcUpdate);
        }
        Log.d(LOG_TAG, "updateLocalDc is " + hasChanged);
        return hasChanged;
    }

    /**
     * Update the current ImsCallSession object based on Driver Call
     * @param dcUpdate
     */
    public void updateCall(DriverCallIms dcUpdate) {
        //TODO - update member variables before calling notification
        Log.d(LOG_TAG, "updateCall for " + dcUpdate);

        if (!isSessionValid()) return;

        final boolean areStatesSame = mDc != null && dcUpdate != null
                && mDc.state == dcUpdate.state;

        updateImsCallProfile(dcUpdate);
        setCapabilitiesInProfiles(dcUpdate);

        mImsCallModification.update(dcUpdate);

        switch (dcUpdate.state) {
            case ACTIVE:
                if (mDc == null) {
                    // TODO:: PHANTOM CALL!!
                    Log.e(LOG_TAG, "Phantom call!");
                    mState = ImsCallSession.State.ESTABLISHED;
                    mCallId = dcUpdate.index;
                    mListenerProxy.callSessionStarted((IImsCallSession) this, mCallProfile);
                } else if (mDc.state == DriverCallIms.State.DIALING ||
                        mDc.state == DriverCallIms.State.ALERTING ||
                        mDc.state == DriverCallIms.State.INCOMING ||
                        mDc.state == DriverCallIms.State.WAITING) {
                    mState = ImsCallSession.State.ESTABLISHED;
                    mDc.state = DriverCallIms.State.ACTIVE;
                    // Extract Call Details into ImsCallProfile.
                    extractCallDetailsIntoCallProfile(dcUpdate);
                    mListenerProxy.callSessionStarted((IImsCallSession) this, mCallProfile);
                }
                // Check if the call is being resumed from a HOLDING state.
                else if (mDc.state == DriverCallIms.State.HOLDING && !mIsConfInProgress) {
                    Log.d(LOG_TAG, "Call being resumed.");
                    mIsConfInProgress = false;
                    mListenerProxy.callSessionResumed((IImsCallSession)this, mCallProfile);
                } else {
                    Log.d(LOG_TAG, "Call resumed skipped, conf status = " + mIsConfInProgress);
                }
                break;
            case HOLDING:
                if (mDc.state != DriverCallIms.State.HOLDING) {
                    Log.d(LOG_TAG, "Call being held.");
                    if (!mIsConfInProgress) { //Notify Held state for non-conference scenario
                        mListenerProxy.callSessionHeld(this, mCallProfile);
                    }
                }
                break;
            case DIALING:
                if (mDc == null) { // No DC yet for pending MO
                    Log.d(LOG_TAG, "MO Dialing call!");
                    mCallId = dcUpdate.index;
                    mListenerProxy.callSessionProgressing((IImsCallSession) this,
                            new ImsStreamMediaProfile());
                }
                break;
            case ALERTING:
                //TODO: Stream media profile with more details.
                mState = ImsCallSession.State.NEGOTIATING;
                if (mDc == null) {
                    Log.d(LOG_TAG, "MO Alerting call!");
                    mCallId = dcUpdate.index;
                }
                if (mDc.state != DriverCallIms.State.ALERTING) {
                    // Extract Call Details into ImsCallProfile.
                    extractCallDetailsIntoCallProfile(dcUpdate);
                    ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile();
                    if (mRingbackToneRequest == true) {
                            mediaProfile.mAudioDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
                    }
                    mListenerProxy.callSessionProgressing((IImsCallSession)this, mediaProfile);
                }
                break;
            case INCOMING:
            case WAITING:
                // Extract Call Details into ImsCallProfile.
                extractCallDetailsIntoCallProfile(dcUpdate);
                //TODO: Send Incoming call intent
                break;
            case END:
                mState = ImsCallSession.State.TERMINATED;
                if (mDisconnCause == ImsReasonInfo.CODE_UNSPECIFIED) {
                    if (dcUpdate != null) {
                        mListenerProxy.callSessionTerminated((IImsCallSession) this,
                                dcUpdate.callFailCause);
                    }
                } else {
                    mListenerProxy.callSessionTerminated((IImsCallSession) this,
                            new ImsReasonInfo(mDisconnCause, 0));
                }
                // In 4 way merge, during conference success, fg call is ended
                // & background conference call as resumed (when bg is moty call).
                // Since we are blocking this resume before, process it now
                // as we received conference response
                mTracker.onNotifyCallResumed();

                notifySessionDisconnected();
                break;
        }
        // Notify listeners of call updated when anything changes in the call.
        boolean hasChanged = updateLocalDc(dcUpdate);
        final boolean isCallNotEnded = dcUpdate.state!= DriverCallIms.State.END;
        if (areStatesSame && isCallNotEnded && hasChanged) {
            log("Call details updated. currentCallDetails= "
                    + mDc.callDetails + " to newCallDetails= " + dcUpdate.callDetails);
            mListenerProxy.callSessionUpdated(this, mCallProfile);
        }
    }

    protected void invokeCallResume(ImsCallSessionImpl callSession, ImsCallProfile profile) {
        Log.d(LOG_TAG, "invokeCallResume");
        mListenerProxy.callSessionResumed((IImsCallSession)callSession, profile);
    }

    private boolean isCallModifiableOnHold() {
        return mContext.getResources().
                getBoolean(R.bool.config_hold_call_modifiable);
    }

    private void setCapabilitiesInProfiles(DriverCallIms dcUpdate) {
        if (mLocalCallProfile != null) {
            mLocalCallProfile.mMediaProfile = new ImsStreamMediaProfile();
            mLocalCallProfile.mMediaProfile.mAudioQuality = mapAudioCodecFromExtras(
                    dcUpdate.callDetails.getValueForKeyFromExtras(dcUpdate.callDetails.extras,
                            CallDetails.EXTRAS_CODEC));
            if (isServiceAllowed(CallDetails.CALL_TYPE_VT, dcUpdate.callDetails.localAbility)
                    && mIsVtGloballyAllowed) {
                mLocalCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT;
            } else {
                mLocalCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
            }

            if (!isCallModifiableOnHold() &&
                    (dcUpdate.state == DriverCallIms.State.HOLDING ||
                    getSuppSvcCode() == SUPP_SVC_CODE_MT_HOLD) &&
                    mLocalCallProfile.mCallType == ImsCallProfile.CALL_TYPE_VT) {
                mLocalCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
            }
        }
        if (mRemoteCallProfile != null) {
            if (isServiceAllowed(CallDetails.CALL_TYPE_VT, dcUpdate.callDetails.peerAbility)) {
                mRemoteCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT;
            } else {
                mRemoteCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
            }
            if (dcUpdate.callDetails.peerAbility != null) {
                mRemoteCallProfile.mMediaProfile = new ImsStreamMediaProfile();
                mRemoteCallProfile.mRestrictCause = getRestrictCause (
                        mCallProfile.mCallType == ImsCallProfile.CALL_TYPE_VT ?
                                CallDetails.CALL_TYPE_VT : CallDetails.CALL_TYPE_VOICE,
                                dcUpdate.callDetails.peerAbility);
            }
        }
    }

    public void updateConfSession(DriverCallIms dc) {
        Log.d(LOG_TAG, "updateConfSession for " + dc);

        if (!isSessionValid()) return;

        if (dc.state == DriverCallIms.State.ACTIVE && dc.isMpty) {
            mState = ImsCallSession.State.ESTABLISHED;
            mCallId = mDc.index;
        }
    }

    private void setSuppSvcCode(int code) {
        mSuppSvcCode = code;
    }

    private int getSuppSvcCode() {
        return mSuppSvcCode;
    }

    /**
     * Call appropriate callbacks for updating call info based on
     * UNSOL_SUPP_SVC_NOTIFICATION for the call.
     *
     * @param code
     */
    /** package-private */
    void updateSuppServiceInfo(ImsSuppServiceNotification suppSvcNotification) {
        Log.d(LOG_TAG, "updateSuppSvcInfo: suppSvcNotification= " + suppSvcNotification);

        if (!isSessionValid()) return;

        setSuppSvcCode(suppSvcNotification.code);
        mListenerProxy.callSessionSuppServiceReceived(this, suppSvcNotification);

        boolean isChanged = false;
        if (!isCallModifiableOnHold()) {
            if (suppSvcNotification.code == SUPP_SVC_CODE_MT_HOLD) {
                if (mRemoteCallProfile.mCallType != ImsCallProfile.CALL_TYPE_VOICE) {
                    mRemoteCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
                    isChanged = true;
                }
            } else if (suppSvcNotification.code == SUPP_SVC_CODE_MT_RESUME) {
                if (isServiceAllowed(CallDetails.CALL_TYPE_VT, mDc.callDetails.peerAbility) &&
                        mRemoteCallProfile.mCallType == ImsCallProfile.CALL_TYPE_VOICE) {
                    mRemoteCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT;
                    isChanged = true;
                }
            }
            if (isChanged) {
                 mListenerProxy.callSessionUpdated(this, mCallProfile);
            }
        }

        // This is a legacy code which is kept here for backward compatibility reason.
        // The code will be removed in next release.
        switch (suppSvcNotification.code) {
            case SUPP_SVC_CODE_MT_HOLD:
                // Call put on hold by remote party.
                mListenerProxy.callSessionHoldReceived((IImsCallSession) this, mCallProfile);
                break;
            case SUPP_SVC_CODE_MT_RESUME:
                // Held call retrieved by remote party.
                mListenerProxy.callSessionResumeReceived((IImsCallSession) this, mCallProfile);
                break;
            default:
                Log.d(LOG_TAG, "Non-Hold/Resume supp svc code received.");
        }
    }

    /**
     * Call appropriate callbacks for notifying TTY mode info based on
     * UNSOL_TTY_NOTIFICATION for the call.
     *
     * @param mode
     */
    public void notifyTtyModeChange(int mode) {
        Log.d(LOG_TAG, "TTY mode = " + mode);

        if (!isSessionValid()) return;

        if (mListenerProxy != null) {
            // TTY mode notified by remote party.
            mListenerProxy.callSessionTtyModeReceived((IImsCallSession) this, mode);
        } else {
            Log.e (LOG_TAG, "notifyTtyModeChange ListenerProxy null! ");
        }
    }

    public void handleHandover(int hoType, int srcTech, int targetTech,
            int extraType, byte[] extraInfo, String errorCode, String errorMessage) {
        Log.d(LOG_TAG, "hoType : " + hoType + "srcTech: " + srcTech +
                " targetTech: " + targetTech);

        if (!isSessionValid()) return;

        final int error = parseErrorCode(errorCode);

        switch (hoType) {
            case ImsQmiIF.START:
                break;
            case ImsQmiIF.COMPLETE_SUCCESS:
                mListenerProxy.callSessionHandover((IImsCallSession)this, srcTech,
                        targetTech, new ImsReasonInfo(error, 0, errorMessage));
                break;
            case ImsQmiIF.CANCEL:
            case ImsQmiIF.COMPLETE_FAIL:
                Log.d(LOG_TAG, "HO Failure for WWAN->IWLAN " + extraType + extraInfo);
                if (extraType == ImsQmiIF.LTE_TO_IWLAN_HO_FAIL) {
                    mCallProfile.setCallExtraInt(CallDetails.EXTRAS_HANDOVER_INFORMATION,
                            CallDetails.EXTRA_TYPE_LTE_TO_IWLAN_HO_FAIL);
                }
                mListenerProxy.callSessionHandoverFailed((IImsCallSession)this, srcTech,
                        targetTech, new ImsReasonInfo(error, 0, errorMessage));
                break;
            case ImsQmiIF.NOT_TRIGGERED:
                mListenerProxy.callSessionHandoverFailed((IImsCallSession)this, srcTech,
                        targetTech, new ImsReasonInfo(error, 0, errorMessage));
                break;
            default:
                Log.e(LOG_TAG, "Unhandled hoType: " + hoType);
        }
    }

    private int parseErrorCode(String errorCode) {
        // CD-04 is the operator specific error code for the call drop case where the user is at
        // the edge of Wifi coverage on a Wifi call and there is no LTE network available to
        // handover to.
        if ("CD-04".equals(errorCode)) {
            return ImsReasonInfo.CODE_CALL_DROP_IWLAN_TO_LTE_UNAVAILABLE;
        }
        return ImsReasonInfo.CODE_UNSPECIFIED;
    }

    //Handler for events tracking requests sent to ImsSenderRxr
    private class ImsCallSessionImplHandler extends Handler {
        ImsCallSessionImplHandler() {
            super();
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d (LOG_TAG, "Message received: what = " + msg.what);
            if (!isSessionValid()) return;

            AsyncResult ar;

            switch (msg.what) {
                case EVENT_DIAL:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Dial error");
                        //TODO: The Reason should convey more granular information
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionStartFailed((IImsCallSession)ar.userObj,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0,
                                            "Dial Failed"));
                        }
                    }
                    break;
                case EVENT_ADD_PARTICIPANT:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Add Participant error");
                        //TODO: The Reason should convey more granular information
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionInviteParticipantsRequestFailed(
                                    (IImsCallSession)ar.userObj,
                                            new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED,
                                                    0, "Add Participant Failed"));
                        }
                    } else {
                        mListenerProxy.callSessionInviteParticipantsRequestDelivered(
                                (IImsCallSession)ar.userObj);
                    }
                    break;
                case EVENT_ACCEPT:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Accept error: " + ar.exception);
                    }
                    break;
                case EVENT_HANGUP:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Hangup error: " + ar.exception);
                    }
                    mDisconnCause = ImsReasonInfo.CODE_USER_TERMINATED;
                    break;
                case EVENT_HOLD:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Hold error");
                        //TODO: The Reason should convey more granular information
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionHoldFailed((IImsCallSession)ar.userObj,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0,
                                            "Hold Failed"));
                        }
                    }
                    break;
                case EVENT_RESUME:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Resume error");
                        //TODO: The Reason should convey more granular information
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionResumeFailed((IImsCallSession)ar.userObj,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0,
                                            "Resume Failed"));
                        }
                    }
                    break;
                case EVENT_CONFERENCE:
                    ar = (AsyncResult) msg.obj;
                    if ( ar != null && ar.exception != null ) {
                        Log.d(LOG_TAG, "Conference error");
                        // TODO: The Reason should convey more granular information
                        if ( ar.userObj != null ) {
                            mListenerProxy.callSessionMergeFailed((IImsCallSession) ar.userObj,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0,
                                            "Conference Failed"));
                            mTracker.setConfInProgress(false);
                        }
                    } else if (ar != null && ar.exception == null) {
                            mListenerProxy.callSessionMergeComplete(mConfCallSession);
                            // In 3 way merge, when refer for active call fails,
                            // it comes back in Held state along with conference call as active.
                            // Process this call as phantom call
                            if (mDc.state == DriverCallIms.State.HOLDING) {
                                mTracker.sendIncomingCallIntent(ImsCallSessionImpl.this,
                                        mDc.index, true, mDc.state, mDc.number);
                            }
                        Log.d(LOG_TAG, "Conference in progress");
                    } else {
                        Log.d(LOG_TAG, "EVENT_CONFERENCE: shouldn't reach here");
                    }
                    mIsConfInProgress = false;
                    break;
                case EVENT_REJECT:
                    ar = (AsyncResult) msg.obj;
                    if ( ar != null ) {
                        mListenerProxy.callSessionStartFailed((IImsCallSession) ar.userObj,
                                new ImsReasonInfo(ImsReasonInfo.CODE_LOCAL_CALL_DECLINE, 0,
                                        "User Rejected"));
                        mDisconnCause = ImsReasonInfo.CODE_LOCAL_CALL_DECLINE;
                    }
                    break;
                case EVENT_DEFLECT:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception != null) {
                        Log.d(LOG_TAG, "Deflect error");
                        //TODO: The Reason should convey more granular information
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionDeflectFailed((IImsCallSession)ar.userObj,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0,
                                            "Deflect Failed"));
                        }
                    } else {
                        Log.d(LOG_TAG, "Deflect success");
                        if(ar.userObj != null) {
                            mListenerProxy.callSessionDeflected((IImsCallSession)ar.userObj);
                        }
                    }
                    break;
                    case EVENT_RINGBACK_TONE:
                        ar = (AsyncResult) msg.obj;
                        if (ar != null) {
                        mRingbackToneRequest = (boolean) (ar.result);
                        Log.d(LOG_TAG, "EVENT_RINGBACK_TONE, playTone = " + mRingbackToneRequest);
                        if (mDc.state == DriverCallIms.State.ALERTING) {
                            ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile();
                            if (mRingbackToneRequest == true) {
                                mediaProfile.mAudioDirection =
                                        ImsStreamMediaProfile.DIRECTION_INACTIVE;
                            }
                            mListenerProxy.callSessionProgressing(
                                    ImsCallSessionImpl.this, mediaProfile);
                        }
                    }
                    break;
                case EVENT_REMOVE_PARTICIPANT:
                     //TODO need to trigger callbacks in success and failure cases
                    break;
                case EVENT_CLOSE_SESSION:
                    doClose();
                    break;
            }
        }
    }

    private boolean isSessionValid() {
        boolean isValid = (mState != ImsCallSession.State.INVALID);
        if (!isValid) {
            Log.e(LOG_TAG, "Session is closed");
        }
        return isValid;
    }
    /**
     * Utility function that returns
     * TRUE if there is an ongoing session,
     * FALSE otherwise
     */
    private boolean isImsCallSessionAlive() {
        return !(mState == ImsCallSession.State.TERMINATED ||
                 mState == ImsCallSession.State.TERMINATING ||
                 mState == ImsCallSession.State.IDLE ||
                 mState == ImsCallSession.State.INVALID);
    }

    private void doClose() {
        Log.d(LOG_TAG, "doClose!");

        if (isImsCallSessionAlive()) {
            Log.d(LOG_TAG, "Received Session Close request while it is alive");
        }

        if ( mState != ImsCallSession.State.INVALID ) {
            if (mListenerProxy != null) {
                mListenerProxy.dispose();
                mListenerProxy = null;
            }
            if (mDc != null && mDc.isMT == false && mCi != null) {
                mCi.unregisterForRingbackTone(mHandler);
            }
            notifySessionClosed();
            synchronized (mListeners) {
                mListeners.clear();
            }
            mCi = null;
            mDc = null;
            mCallId = 0;
            mLocalCallProfile = null;
            mRemoteCallProfile = null;
            mCallProfile = null;
            mState = ImsCallSession.State.INVALID;
            mInCall = false;
            mHandler = null;
            mIsConfInProgress = false;
            newSession = null;
            mImsVideoCallProviderImpl = null;
            mImsCallModification = null;
            mCallee = null;
            mConfInfo = null;
        }
    }

    /**
     * Closes the object. This object is not usable after being closed.
     * This function is called when onCallTerminated is triggered from client side.
     */
    public void close() {
        Log.d(LOG_TAG, "Close!");
        if (mHandler != null) {
            mHandler.obtainMessage(EVENT_CLOSE_SESSION).sendToTarget();
        }
    }

    /**
     * Gets the call ID of the session.
     *
     * @return the call ID
     */
    public String getCallId() {
        return Integer.toString(mCallId);
    }

    /**
     * Gets the media ID of the session.
     *
     * @return the media ID
     */

    public int getMediaId() {
        if (!isSessionValid()) return CallDetails.MEDIA_ID_UNKNOWN;
        return (mDc != null ? mDc.callDetails.callMediaId : CallDetails.MEDIA_ID_UNKNOWN);
    }

    /**
     * Checks if mediaId of the call session is valid
     *
     * @return true if the mediaId is valid
     */

    public boolean hasMediaIdValid() {
        if (!isSessionValid()) return false;
        return (mDc != null ? mDc.callDetails.hasMediaIdValid() : false);
    }

    /**
     * Gets the call profile that this session is associated with
     *
     * @return the call profile that this session is associated with
     */
    public ImsCallProfile getCallProfile() {
        if (!isSessionValid()) return null;
        return mCallProfile;
    }

    /**
     * Gets the local call profile that this session is associated with
     *
     * @return the local call profile that this session is associated with
     */
    public ImsCallProfile getLocalCallProfile() {
        if (!isSessionValid()) return null;
        return mLocalCallProfile;
    }

    /**
     * Gets the remote call profile that this session is associated with
     *
     * @return the remote call profile that this session is associated with
     */
    public ImsCallProfile getRemoteCallProfile() {
        if (!isSessionValid()) return null;
        return mRemoteCallProfile;
    }

    /**
     * Gets the value associated with the specified property of this session.
     *
     * @return the string value associated with the specified property
     */
    public String getProperty(String name) {
        if (!isSessionValid()) return null;

        String value = null;

        if (mCallProfile != null) {
            value = mCallProfile.getCallExtra(name);
        } else {
            Log.e (LOG_TAG, "Call Profile null! ");
        }
        return value;
    }

    /**
     * Gets the session state. The value returned must be one of the states in
     * {@link ImsCallSession#State}.
     *
     * @return the session state
     */
    public int getState() {
        if (!isSessionValid()) return ImsCallSession.State.INVALID;
        return mState;
    }

    /* package */
    DriverCallIms.State getInternalState() {
        if (!isSessionValid()) return DriverCallIms.State.END;

        DriverCallIms.State state = null;
        if (mDc != null) {
            state = mDc.state;
        } else if (mState == ImsCallSession.State.INITIATED) {
            state = DriverCallIms.State.DIALING;
        }
        return state;
    }

    public int getInternalCallType() {
        if (!isSessionValid()) return CallDetails.CALL_TYPE_UNKNOWN;

        int callType = CallDetails.CALL_TYPE_UNKNOWN;
        if(mDc != null && mDc.callDetails != null){
            callType = mDc.callDetails.call_type;
        } else if (mState == ImsCallSession.State.INITIATED && mCallProfile != null) {
            callType = ImsCallUtils.convertToInternalCallType(mCallProfile.mCallType);
        }
        return callType;
    }

    public int getCallDomain() {
        if (!isSessionValid()) return CallDetails.CALL_DOMAIN_AUTOMATIC;

        int callDomain = CallDetails.CALL_DOMAIN_AUTOMATIC;
        if (mDc != null && mDc.callDetails != null) {
            callDomain = mDc.callDetails.call_domain;
        }
        return callDomain;
    }

    /**
     * Gets the call substate for this session.
     *
     * @return the call substate for this session.
     */
    public int getCallSubstate() {
        if (!isSessionValid()) return Connection.CALL_SUBSTATE_NONE;

        int callSubstate= Connection.CALL_SUBSTATE_NONE;
        if (mDc != null && mDc.callDetails != null) {
            callSubstate = mDc.callDetails.callsubstate;
        }
        return callSubstate;
    }

    public boolean isMultipartyCall() {
        if (!isSessionValid()) return false;
        return (mDc != null) ? mDc.isMpty : false;
    }

    /**
     * Gets the Callee address for a MT Call
     *
     * @return the callee address
     */
    public String getCallee() {
        if (!isSessionValid()) return null;
        return mCallee;
    }

    public DriverCallIms.State getDriverCallState() {
        if (!isSessionValid()) return DriverCallIms.State.END;
        return mDc.state;
    }

    /**
     * Determines if the current session is multiparty.
     *
     * @return {@code True} if the session is multiparty.
     */
    public boolean isMultiparty() {
        if (!isSessionValid()) return false;
        if (mDc == null) {
            return false;
        }
        return mDc.isMpty;
    }

    /**
     * Checks if the session is in a call.
     *
     * @return true if the session is in a call
     */
    public boolean isInCall() {
        if (!isSessionValid()) return false;

        boolean isInCall = false;
        switch (mDc.state) {
            case ACTIVE:
            case HOLDING:
            case DIALING:
            case ALERTING:
            case INCOMING:
            case WAITING:
                isInCall = true;
                break;
        }
        return isInCall;
    }

    /**
     * Sets the listener to listen to the session events. A {@link IImsCallSession}
     * can only hold one listener at a time. Subsequent calls to this method
     * override the previous listener.
     *
     * @param listener to listen to the session events of this object
     */
    public void setListener(IImsCallSessionListener listener) {
        if (!isSessionValid()) return;
        mListenerProxy.mListener = listener;
    }

    /**
     * Mutes or unmutes the mic for the active call.
     *
     * @param muted true if the call is muted, false otherwise
     */
    public void setMute(boolean muted) {
        if (!isSessionValid()) return;
        //TODO:
    }

    /**
     * Method used to report a call to the IMS conference server that is
     * created when we want to merge two calls into a conference call.
     * This call is reported through the current foreground call.
     *
     * @param confCallSession The newly created conference call session.
     */
    public void reportNewConferenceCallSession(ImsCallSessionImpl confCallSession) {
        if (confCallSession != null) {
            Log.d(LOG_TAG, "Calling callSessionMergeStarted");
            mConfCallSession = confCallSession;
            mListenerProxy.callSessionMergeStarted((IImsCallSession) this,
                    confCallSession,
                    confCallSession.getCallProfile());
        } else {
            Log.e (LOG_TAG,
                    "Null confCallSession! Not calling callSessionMergeStarted");
        }
    }

    private void extractCallDetailsIntoCallProfile(DriverCallIms dcUpdate) {
        if (dcUpdate == null) {
            Log.e(LOG_TAG, "Null dcUpdate in extractCallDetailsIntoCallProfile");
            return;
        }
        updateImsCallProfile(dcUpdate);

        // Check for extra info. Example format provided.
        // Call Details = 0 2
        // Codec=AMR_NB AdditionalCallInfo=
        // P-Asserted-Identity: <sip:+15857470175@10.174.9.1;user=phone>
        // Call type = 0 , domain = 2
        // Extras[0] " Codec=AMR_NB
        // Extras[1]" AdditionalCallInfo=
        // P-Asserted-Identity: <sip:+15857470175@10.174.9.1;user=phone>"
        if (dcUpdate.callDetails.extras != null &&
                dcUpdate.callDetails.extras.length > 0) {
            String key = null;
            String[] keyAndValue = null;
            String[] namespaceAndKey = null;

            for (int i = 0; i < dcUpdate.callDetails.extras.length; i++) {
                if (dcUpdate.callDetails.extras[i] != null) {
                    keyAndValue = dcUpdate.callDetails.extras[i].split("=");
                    // Check for improperly formatted extra string.
                    if (keyAndValue[0] != null) {
                        // Check key string to check if namespace is present.
                        // If so, extract just the key.
                        // Example (key=value): "ChildNum=12345"
                        // Namespace example: "OEM:MyPhoneId=11"
                        if (keyAndValue[0].contains(":")) {
                            namespaceAndKey = keyAndValue[0].split(":");
                            key = namespaceAndKey[1];
                        } else {
                            key = keyAndValue[0];
                        }
                    } else {
                        Log.e(LOG_TAG, "Bad extra string from lower layers!");
                        return;
                    }
                } else {
                    Log.e(LOG_TAG, "Element " + i + " is null in CallDetails Extras!");
                    return;
                }

                Log.d(LOG_TAG, "CallDetails Extra key= " + key
                        + " value= " + keyAndValue[1]);
                mCallProfile.setCallExtra(key, keyAndValue[1]);
            }
        }

    }

    /**
     * Private Utility to translate Extras Codec to ImsStreamMediaProfile audio quality
     * @param codec string
     * @return int - audio quality
     */
    private static int mapAudioCodecFromExtras(String codec) {
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        if (codec == null) {
            return ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        }
        switch (codec) {
            case "QCELP13K":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
                break;
            case "EVRC":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_EVRC;
                break;
            case "EVRC_B":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_EVRC_B;
                break;
            case "EVRC_WB":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_EVRC_WB;
                break;
            case "EVRC_NW":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_EVRC_NW;
                break;
            case "AMR_NB":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_AMR;
                break;
            case "AMR_WB":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB;
                break;
            case "GSM_EFR":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_GSM_EFR;
                break;
            case "GSM_FR":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_GSM_FR;
                break;
            case "GSM_HR":
                audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_GSM_HR;
                break;
            default:
                Log.e (LOG_TAG, "Unsupported codec " + codec);
                break;
        }
        Log.d (LOG_TAG, "AudioQuality is " + audioQuality);
        return audioQuality;
    }

    /**
     * Temporary private Utility to translate ImsCallProfile to CallDetails call type
     * @param callType
     * @return int - call type
     */
    private int mapCallTypeFromProfile(int callType) {
        int type = CallDetails.CALL_TYPE_VOICE;
        switch (callType) {
            case ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO:
                type = CallDetails.CALL_TYPE_UNKNOWN;
                break;
            case ImsCallProfile.CALL_TYPE_VOICE:
                type = CallDetails.CALL_TYPE_VOICE;
                break;
            case ImsCallProfile.CALL_TYPE_VT:
                type = CallDetails.CALL_TYPE_VT;
                break;
            case ImsCallProfile.CALL_TYPE_VT_TX:
                type = CallDetails.CALL_TYPE_VT_TX;
                break;
            case ImsCallProfile.CALL_TYPE_VT_RX:
                type = CallDetails.CALL_TYPE_VT_RX;
                break;
            case ImsCallProfile.CALL_TYPE_VT_NODIR:
                type = CallDetails.CALL_TYPE_VT_NODIR;
                break;
        }
        return type;
    }

    /**
     * Update mCallProfile as per Driver call.
     * @param Drivercall
     */
    private void updateImsCallProfile(DriverCallIms dc) {
        if(dc == null) {
            loge("updateImsCallProfile called with dc null");
            return;
        }

        if(mCallProfile == null) {
            mCallProfile = new ImsCallProfile();
        }

        mCallProfile.setCallExtra(ImsCallProfile.EXTRA_OI, dc.number);
        mCallProfile.setCallExtra(ImsCallProfile.EXTRA_CNA, dc.name);
        mCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_OIR,
                ImsCallProfile.presentationToOIR(dc.numberPresentation));
        mCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP,
                ImsCallProfile.presentationToOIR(dc.namePresentation));

        switch (dc.callDetails.call_type) {
            case CallDetails.CALL_TYPE_UNKNOWN:
                mCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO;
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_INVALID;
                break;
            case CallDetails.CALL_TYPE_VOICE:
                mCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_INVALID;
                break;
            case CallDetails.CALL_TYPE_VT:
                mCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT;
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
                break;
            case CallDetails.CALL_TYPE_VT_TX:
                mCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT_TX;
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_SEND;
                break;
            case CallDetails.CALL_TYPE_VT_RX:
                mCallProfile.mCallType = ImsCallProfile.CALL_TYPE_VT_RX;
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_RECEIVE;
                break;
            case CallDetails.CALL_TYPE_VT_NODIR:
                // Not propagating VT_NODIR call type to UI
                mCallProfile.mMediaProfile.mVideoDirection
                    = ImsStreamMediaProfile.DIRECTION_INACTIVE;
                break;
        }
    }

    /**
     * Utility method to return when true when conference call is in progress
     * @return boolean - true if conference is in progress
     */
    public boolean isConfInProgress() {
        if (!isSessionValid()) return false;
        return mIsConfInProgress;
    }

    /**
     * Utility method to set new Call Session for conference call scenario
     */
    public void setNewSession(ImsCallSessionImpl session) {
        if (!isSessionValid()) return;
        newSession = session;
    }

    /**
     * Initiates an IMS call with the specified target and call profile. The session listener is
     * called back upon defined session events. The method is only valid to call when the session
     * state is in {@link ImsCallSession#State#IDLE}.
     * @param callee dialed string to make the call to
     * @param profile call profile to make the call with the specified service type, call type and
     *            media information
     * @see Listener#callSessionStarted, Listener#callSessionStartFailed
     */
    public void start(String callee, ImsCallProfile profile) {
        if (!isSessionValid()) return;

        mCallProfile.mCallType = profile.mCallType;
        mCallProfile.mMediaProfile = profile.mMediaProfile;

        mState = ImsCallSession.State.INITIATED;
        mCallee = callee;
        //TODO add domain selection from ImsPhone
        //TODO emergency calls -lookup profile

        int clir = profile.getCallExtraInt(ImsCallProfile.EXTRA_OIR);
        int domain = CallDetails.CALL_DOMAIN_AUTOMATIC;
        if (profile.getCallExtraInt(ImsCallProfile.EXTRA_CALL_DOMAIN, -1) != -1) {
            domain = profile.getCallExtraInt(ImsCallProfile.EXTRA_CALL_DOMAIN, -1);
            Log.d(LOG_TAG, "start: domain from extra = " + domain);
        }
        CallDetails details = new CallDetails(mapCallTypeFromProfile(profile.mCallType),
                domain,
                null);
        extractProfileExtrasIntoCallDetails(profile, details);

        mCi.dial(callee, clir, details,
                mHandler.obtainMessage(EVENT_DIAL, this));
    }

    private void extractProfileExtrasIntoCallDetails(ImsCallProfile profile,
            CallDetails details) {
        Bundle callExtras = profile.mCallExtras.getBundle(ImsCallProfile.EXTRA_OEM_EXTRAS);
        if (callExtras != null) {
            String extraString = null;
            String[] extras = new String[callExtras.size()];
            int i = 0;

            // Pack the extras in a 'key=value' format in the extras String[]
            // in CallDetails.
            for (String key : callExtras.keySet()) {
                extraString = new String(key + "=" + (callExtras.get(key) == null ? "" :
                        callExtras.get(key).toString()));
                Log.d(LOG_TAG, "Packing extra string: " + extraString);
                extras[i] = extraString;
                i++;
            }
            details.setExtras(extras);
        } else {
            Log.d(LOG_TAG, "No extras in ImsCallProfile to map into CallDetails.");
        }
    }

    /**
     * Initiates an IMS call with the specified participants and call profile.
     * The session listener is called back upon defined session events.
     * The method is only valid to call when the session state is in
     * {@link ImsCallSession#State#IDLE}.
     *
     * @param participants participant list to initiate an IMS conference call
     * @param profile call profile to make the call with the specified service type,
     *      call type and media information
     * @see Listener#callSessionStarted, Listener#callSessionStartFailed
     */
    public void startConference(String[] participants, ImsCallProfile profile) {
        if (!isSessionValid()) return;

        mCallProfile = profile; // TODO update profile and do not replace
        mState = ImsCallSession.State.INITIATED;
        mCallee = participants[0];
        // TODO add domain selection from ImsPhone
        // TODO clir & emergency calls -lookup profile
        final java.util.Map<String, String> extrasMap =
                com.google.android.collect.Maps.newHashMap();
        extrasMap.put(CallDetails.EXTRAS_IS_CONFERENCE_URI,
                Boolean.toString(true));
        String[] mMoExtras = CallDetails.getExtrasFromMap(extrasMap);

        CallDetails details = new CallDetails(mapCallTypeFromProfile(profile.mCallType),
                CallDetails.CALL_DOMAIN_AUTOMATIC, mMoExtras);
        mCi.dial(participants[0], CommandsInterface.CLIR_DEFAULT, details,
                mHandler.obtainMessage(EVENT_DIAL, this));
    }

    /**
     * Accepts an incoming call or session update.
     *
     * @param callType call type specified in {@link ImsCallProfile} to be answered
     * @param profile stream media profile {@link ImsStreamMediaProfile} to be answered
     * @see Listener#callSessionStarted
     */
    public void accept(int callType, ImsStreamMediaProfile profile) {
        if (!isSessionValid()) return;

        // this caluse null pointer exception!! this.mLocalCallProfile.mMediaProfile = profile;
        mCi.acceptCall(mHandler.obtainMessage(EVENT_ACCEPT, this),
                mapCallTypeFromProfile(callType));
    }

    /**
     * Deflects an incoming call to given number.
     *
     * @param deflectNumber number to deflect the call
     */
    public void deflect(String deflectNumber) {
        if (!isSessionValid()) return;

        mCi.deflectCall(mCallId, deflectNumber,
                mHandler.obtainMessage(EVENT_DEFLECT, this));
    }

    /**
     * Rejects an incoming call or session update.
     *
     * @param reason reason code to reject an incoming call
     * @see Listener#callSessionStartFailed
     */
    public void reject(int reason) {
        if (!isSessionValid()) return;

        Log.d(LOG_TAG, "reject " + reason);
        mCi.hangupWithReason(mCallId, null, null, false, reason, null,
                mHandler.obtainMessage(EVENT_REJECT, this));
    }

    public boolean isCallActive() {
        if (!isSessionValid()) return false;
        return getInternalState() == DriverCallIms.State.ACTIVE;
    }

    /**
     * Terminates a call.
     *
     * @see Listener#callSessionTerminated
     */
    public void terminate(int reason) {
        if (!isSessionValid()) return;

        Log.d(LOG_TAG, "terminate " + reason);
        mCi.hangupWithReason(mCallId, null, null, false, reason, null,
                mHandler.obtainMessage(EVENT_HANGUP, this));
    }

    /**
     * Puts a call on hold. When it succeeds, {@link Listener#callSessionHeld} is called.
     *
     * @param profile stream media profile {@link ImsStreamMediaProfile} to hold the call
     * @see Listener#callSessionHeld, Listener#callSessionHoldFailed
     */
    public void hold(ImsStreamMediaProfile profile) {
        if (!isSessionValid()) return;

        Log.d (LOG_TAG, "hold requested.");
        mCi.hold(mHandler.obtainMessage(EVENT_HOLD, this), mDc.index);
    }

    /**
     * Continues a call that's on hold. When it succeeds, {@link Listener#callSessionResumed}
     * is called.
     *
     * @param profile stream media profile {@link ImsStreamMediaProfile} to resume the call
     * @see Listener#callSessionResumed, Listener#callSessionResumeFailed
     */
    public void resume(ImsStreamMediaProfile profile) {
        if (!isSessionValid()) return;

        Log.d (LOG_TAG, "resume requested.");
        mCi.resume(mHandler.obtainMessage(EVENT_RESUME, this), mDc.index);
    }

    /**
     * Merges the active & hold call. When it succeeds, {@link Listener#callSessionMerged} is
     * called.
     * @see Listener#callSessionMerged, Listener#callSessionMergeFailed
     */
    public void merge() {
        if (!isSessionValid()) return;

        mIsConfInProgress = true;
        // For certain operators, when merge is triggered for 4-way merge
        // existing conference call is resumed first and then refer for active call
        // is sent to conference server
        // If this resume is sent to frwks, it corrupts the fg & bg calls
        // Hence use mIsConfInProgress flag to postpone the resume until response is
        // received. Update mIsConfInProgress to true, even for bg call
        // if it is multiparty call
        mTracker.setConfInProgress(true);
        mCi.conference(mHandler.obtainMessage(EVENT_CONFERENCE, this));
    }

    /**
     * Updates the current call's properties (ex. call mode change: video upgrade / downgrade).
     *
     * @param callType call type specified in {@link ImsCallProfile} to be updated
     * @param profile stream media profile {@link ImsStreamMediaProfile} to be updated
     * @see Listener#callSessionUpdated, Listener#callSessionUpdateFailed
     */
    public void update(int callType, ImsStreamMediaProfile profile) {
        if (!isSessionValid()) return;
    }

    /**
     * Extends this call to the conference call with the specified recipients.
     *
     * @participants participant list to be invited to the conference call after extending the call
     * @see Listener#sessionConferenceExtened, Listener#sessionConferenceExtendFailed
     */
    public void extendToConference(String[] participants) {
        if (!isSessionValid()) return;
        //TODO
    }

    /**
     * Requests the conference server to invite an additional participants to the conference.
     *
     * @participants participant list to be invited to the conference call
     * @see Listener#sessionInviteParticipantsRequestDelivered,
     *      Listener#sessionInviteParticipantsRequestFailed
     */
    public void inviteParticipants(String[] participants) {
        if (!isSessionValid()) return;

        mCallee = participants[0];
        mCi.addParticipant(mCallee, CommandsInterface.CLIR_DEFAULT, null,
                mHandler.obtainMessage(EVENT_ADD_PARTICIPANT, this));
    }

    /**
     * Requests the conference server to remove the specified participants from the conference.
     *
     * @param participants participant list to be removed from the conference call
     * @see Listener#sessionRemoveParticipantsRequestDelivered,
     *      Listener#sessionRemoveParticipantsRequestFailed
     */
    public void removeParticipants(String[] participants) {
        if (!isSessionValid()) return;
        mCallee = participants[0];
        Log.d(LOG_TAG, "removeParticipants user: " + mCallee);
        mCi.hangupWithReason(-1, mCallee, null, true, ImsReasonInfo.CODE_USER_TERMINATED,
                null, mHandler.obtainMessage(EVENT_REMOVE_PARTICIPANT, this));
    }

    /**
     * Sends a DTMF code. According to <a href="http://tools.ietf.org/html/rfc2833">RFC 2833</a>,
     * event 0 ~ 9 maps to decimal value 0 ~ 9, '*' to 10, '#' to 11, event 'A' ~ 'D' to 12 ~ 15,
     * and event flash to 16. Currently, event flash is not supported.
     *
     * @param c the DTMF to send. '0' ~ '9', 'A' ~ 'D', '*', '#' are valid inputs.
     * @param result.
     */
    public void sendDtmf(char c, Message result) {
        if (!isSessionValid()) return;

        mCi.sendDtmf(c, result);
    }

    /**
     * Start a DTMF code. According to <a href="http://tools.ietf.org/html/rfc2833">RFC 2833</a>,
     * event 0 ~ 9 maps to decimal value 0 ~ 9, '*' to 10, '#' to 11, event 'A' ~ 'D' to 12 ~ 15,
     * and event flash to 16. Currently, event flash is not supported.
     *
     * @param c the DTMF to send. '0' ~ '9', 'A' ~ 'D', '*', '#' are valid inputs.
     */
    public void startDtmf(char c) {
        if (!isSessionValid()) return;
        mCi.startDtmf(c, null);
    }

    /**
     * Stop a DTMF code.
     */
    public void stopDtmf() {
        if (!isSessionValid()) return;
        mCi.stopDtmf(null);
    }

    public IImsVideoCallProvider getVideoCallProvider() {
        if (!isSessionValid()) return null;

        return mImsVideoCallProviderImpl.getInterface();
    }

    /**
     * Sends an USSD message.
     *
     * @param ussdMessage USSD message to send
     */
    public void sendUssd(String ussdMessage) {
        if (!isSessionValid()) return;
        //TODO
    }

    public void notifyConfInfo(byte[] confInfoBytes) {
        if (!isSessionValid()) return;

        mConfInfo.updateConfXmlBytes(confInfoBytes);
        mImsConferenceState = mConfInfo.getConfUriList();
        mListenerProxy.callSessionConferenceStateUpdated((IImsCallSession) this,
                mImsConferenceState);

    }

    public void setConfInfo(ConfInfo confInfo) {
        if (!isSessionValid()) return;

        this.mConfInfo = confInfo;
    }

    public ConfInfo getConfInfo() {
        if (!isSessionValid()) return null;
        return mConfInfo;
    }

    public void updateVtGlobalCapability(boolean isVtEnabled) {
        log("updateVtGlobalCapability " + isVtEnabled);
        if (!isSessionValid()) return;

        if (mIsVtGloballyAllowed != isVtEnabled) {
            mIsVtGloballyAllowed = isVtEnabled;
            if (mDc!= null && mDc.state != DriverCallIms.State.END) {
                setCapabilitiesInProfiles(mDc);
                mListenerProxy.callSessionUpdated(this, mCallProfile);
            }
        }
    }

    public String toString() {
        return " callid= " + mCallId + " mediaId=" + getMediaId() + " mState= " + mState + " mDc="
                + mDc + " mCallProfile=" + mCallProfile + " mLocalCallProfile=" + mLocalCallProfile
                + " mRemoteCallProfile=" + mRemoteCallProfile;
    }

    public String toSimpleString() {
        return super.toString();
    }

    private void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    private void loge(String msg) {
        Log.e(LOG_TAG, msg);
    }

    /** TODO: Implement this function if needed, may come in handy later
    private DriverCall.State mapSessionToDcState(int state) {

        ImsCallSession states
        public static final int IDLE = 0;
        public static final int INITIATED = 1;
        public static final int NEGOTIATING = 2;
        public static final int ESTABLISHING = 3;
        public static final int ESTABLISHED = 4;

        public static final int RENEGOTIATING = 5;
        public static final int REESTABLISHING = 6;

        public static final int TERMINATING = 7;
        public static final int TERMINATED = 8;

        DC states
        ACTIVE,
        HOLDING,
        DIALING,    // MO call only
        ALERTING,   // MO call only
        INCOMING,   // MT call only
        WAITING;

    } */
}
