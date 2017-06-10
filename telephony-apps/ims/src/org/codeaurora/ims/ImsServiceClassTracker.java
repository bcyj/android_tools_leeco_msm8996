/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.List;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import org.codeaurora.ims.ImsSenderRxr;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.Message;
import android.telephony.VoLteServiceState;
import android.util.Log;

import com.android.ims.ImsCallProfile;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;
import com.android.ims.ImsSuppServiceNotification;
import com.android.ims.internal.IImsCallSessionListener;
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.internal.ImsCallSession;
import com.android.internal.telephony.Call;

public class ImsServiceClassTracker implements ImsCallSessionImpl.Listener{
    private static final String LOG_TAG = "ImsServiceClassTracker";

    private int mServiceId; /* Key for Unique element in the object */
    private int mServiceClass; /* Key for Unique element in the object */
    /* TODO: When tracker object registers with ImsSenderRxr for call state change and registration,
     * the incomingCallIntent and regListener can be private.
     */
    public PendingIntent mIncomingCallIntent; //TODO: Change Private in future
    public IImsRegistrationListener mRegListener; //TODO: Change Private in future
    // Call Id, Dial String --> Call Session
    private Map<String, ImsCallSessionImpl> mCallList;
    private ArrayList<ImsCallSessionImpl> mPendingSessionList;
    private List<ICallListListener> mCallListListeners;
    private static int currentMaxServiceId = 0;
    private ImsSenderRxr mCi = null;
    private Context mContext;
    private boolean mIsVtSupportedGlobally = false;
    private boolean mNeedIgnoreCalls = false;
    private Call.SrvccState mSrvccStateFromIms = Call.SrvccState.NONE;

    /**
     * Action for the incoming call intent for the Phone app.
     * Internal use only.
     * @hide
     */
    private static final String ACTION_IMS_INCOMING_CALL = "com.android.ims.volte.incoming_call";

    /**
     * Part of the ACTION_IMS_INCOMING_CALL intents.
     * An integer value; service identifier obtained from {@link ImsManager#open}.
     * Internal use only.
     * @hide
     */
    public static final String EXTRA_SERVICE_ID = "android:imsServiceId";

    /**
     * Part of the ACTION_IMS_INCOMING_CALL intents.
     * An boolean value; Flag to indicate that the incoming call is a normal call or call for USSD.
     * The value "true" indicates that the incoming call is for USSD.
     * Internal use only.
     * @hide
     */
    public static final String EXTRA_USSD = "android:ussd";

    public static final String CONF_URI_DC_NUMBER = "Conference Call";

    //Constructor
    public ImsServiceClassTracker(int serviceClass, PendingIntent intent,
            IImsRegistrationListener listener, ImsSenderRxr ci, Context context) {
        mServiceClass = serviceClass;
        mIncomingCallIntent = intent;
        mRegListener = listener;
        mServiceId = createServiceId();
        mCi = ci;
        mContext = context;
        mCallList = new HashMap<String, ImsCallSessionImpl>();
        mPendingSessionList = new ArrayList<ImsCallSessionImpl>();
        mCallListListeners = new CopyOnWriteArrayList<>();
    }

    private static class HandoverInfo {
        private int mType = 0;
        private int mSrcTech = 0;
        private int mTargetTech = 0;
        private int mExtraType = 0;
        private byte[] mExtraInfo = null;
        private String mErrorCode = null;
        private String mErrorMessage = null;
    }

    private static int createServiceId() {
        return ++currentMaxServiceId; //TODO: check roundtrip
    }

    /**
     * Get the service Id associated with this tracker object
     * @return service id
     */
    public int getServiceId() {
        return mServiceId;
    }

    /**
     * Get the service Class associated with this tracker object
     * @return service class
     */
    public int getServiceClass() {
        return mServiceClass;
    }

    /**
     * Creates a intent for incoming call
     * @param callId - Unique ID for the incoming call
     * @param isUssd - If this is a USSD request
     * @return Intent
     */
    private Intent createIncomingCallIntent(String callId, boolean isUssd, boolean isUnknown,
                DriverCallIms.State state, String number) {
        Intent intent = new Intent();
        intent.putExtra(ImsManager.EXTRA_CALL_ID, callId);
        intent.putExtra(ImsManager.EXTRA_USSD, isUssd);
        intent.putExtra(ImsManager.EXTRA_SERVICE_ID, getServiceId());
        intent.putExtra(ImsManager.EXTRA_IS_UNKNOWN_CALL, isUnknown);
        intent.putExtra(ImsManager.EXTRA_UNKNOWN_CALL_ADDRESS, number);
        intent.putExtra(ImsManager.EXTRA_UNKNOWN_CALL_STATE, convertDriverStateToInt(state));
        return intent;
    }

    private int convertDriverStateToInt(DriverCallIms.State state) {
        switch (state) {
            case ACTIVE:
                return ImsManager.CALL_ACTIVE;
            case HOLDING:
                return ImsManager.CALL_HOLD;
            case DIALING:
                return ImsManager.CALL_DIALING;
            case ALERTING:
                return ImsManager.CALL_ALERTING;
            case INCOMING:
                return ImsManager.CALL_INCOMING;
            case WAITING:
                return ImsManager.CALL_WAITING;
            case END:
                return ImsManager.CALL_END;
            default:
                {
                    log("illegal call state in convertDriverStateToInt" + state);
                    return ImsManager.CALL_INCOMING;
                }
        }
    }

    public void updateVtCapability(boolean isVtEnabled) {
        log("updateVtCapability " + isVtEnabled);
        mIsVtSupportedGlobally = isVtEnabled;
        synchronized(mCallList) {
            for (Map.Entry<String, ImsCallSessionImpl> e : mCallList.entrySet()) {
                final ImsCallSessionImpl session = e.getValue();
                session.updateVtGlobalCapability(isVtEnabled);
            }
        }
    }

    /*
     * Calculates overall srvcc state and determines whether new calls
     * need to be ignored.
     * When SRVCC state on IMS pipe is COMPLETED, it means there
     * are no more call state changes to be expected on ims pipe.
     * So we can reset ignore calls flag.
     * When SRVCC state on CS pipe is COMPLETED and on IMS pipe is not
     * COMPLETED we need to ignore any new calls arriving on ims pipe.
     */
    public void calculateOverallSrvccState(int[] ret) {
        int state = -1;
        if (ret != null && ret.length != 0) {
            state = ret[0];
        }
        log("calculateOverallSrvccState imsSrvccState = " + mSrvccStateFromIms +
                " CS SRVCC state = " + state);
        if (!isSrvccCompleted(mSrvccStateFromIms) &&
                state == VoLteServiceState.HANDOVER_COMPLETED) {
            //IMS srvcc status is not completed but RILD handover is done.
            //We may get call indication on IMS pipe which need to ignore,
            //so set the flag true.
            mNeedIgnoreCalls = true;
        } else if (state != VoLteServiceState.HANDOVER_COMPLETED) {
            //IMS srvcc is completed or CS srvcc status is started or failed/cancel.
            //In such cases, reset mSrvccStateFromIms.
            mSrvccStateFromIms = Call.SrvccState.NONE;
        }
    }

    private boolean isSrvccCompleted(Call.SrvccState state) {
        return (state == Call.SrvccState.COMPLETED ||
                state == Call.SrvccState.FAILED ||
                state == Call.SrvccState.CANCELED);
    }

    /**
     * Handle a list of calls updated by the IMS stack
     * @param dcList
     */
    public void handleCalls(ArrayList<DriverCallIms> dcList) {
        Map <String, DriverCallIms> dcMap = new HashMap<String, DriverCallIms>();
        boolean isConfInProgress = false;
        // First pass is to look at every call in dc and update the Call Session List
        for (int i = 0; dcList!= null && i < dcList.size(); i++) {
            ImsCallSessionImpl callSession = null;
            DriverCallIms dc = dcList.get(i);
            if (dc.isMpty) {
                isConfInProgress = true;
            }
            if (mPendingSessionList != null) {
                synchronized(mPendingSessionList) {
                    for (Iterator<ImsCallSessionImpl> it = mPendingSessionList.iterator();
                            it.hasNext();) {
                        ImsCallSessionImpl s = it.next();
                        if (dc.state == DriverCallIms.State.DIALING) {
                            // Add to call list as we now have call id, remove from
                            // temp list
                            Log.d(LOG_TAG, "Found match call session in temp list, s = " + s);
                            Log.d(LOG_TAG, "Index in call list is " + dc.index);
                            addCall(dc.index, s, false);
                            // Remove from mPendingSessionList
                            it.remove();
                        }
                    }
                }
            }
            synchronized(mCallList) {
                callSession = mCallList.get(Integer.toString(dc.index));
            }
            if (callSession != null){
                // Pending MO, active call updates
                // update for a existing call - no callID but MO number for a dial request
                // Call collision scenario
                callSession.updateCall(dc);
            } else {
                boolean isUnknown = false;
                if (dc.state == DriverCallIms.State.END) {
                    //This is an unknown call probably already cleaned up as part of SRVCC
                    //just ignore this dc and continue with the dc list
                    continue;
                }
                callSession = new ImsCallSessionImpl(dc, mCi, mContext, this);
                callSession.addListener(this);
                callSession.updateVtGlobalCapability(mIsVtSupportedGlobally);
                if (mNeedIgnoreCalls) {
                    Log.d(LOG_TAG, "SRVCC is in progress ignore call indication on IMS");
                }
                if (dc.isMT && !mNeedIgnoreCalls) {
                    Log.d(LOG_TAG, "MT Call creating a new call session");
                    sendIncomingCallIntent(callSession, dc.index, false, dc.state, dc.number);
                } else if (dc.isMpty && dc.state == DriverCallIms.State.DIALING) {
                    Log.d(LOG_TAG, "Conference Call creating a new call session");
                    isUnknown = true;
                    //Get the old session
                    synchronized(mCallList) {
                        for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it =
                                mCallList.entrySet().iterator(); it.hasNext();) {
                            Map.Entry<String, ImsCallSessionImpl> e = it.next();
                            ImsCallSessionImpl oldSession = (ImsCallSessionImpl) e.getValue();
                            if ( oldSession.isConfInProgress() ) {
                                Log.d(LOG_TAG, "Set New Session as Id " + callSession.getCallId());
                                callSession.setConfInfo(oldSession.getConfInfo());
                                oldSession.setNewSession(callSession);
                                oldSession.reportNewConferenceCallSession(callSession);
                                // This is new conference call created as part of 3 way merge.
                                isUnknown = false;
                                break;
                            }
                        }
                    }
                    addCall(dc.index, callSession, true);
                    callSession.updateConfSession(dc);
                    if (isUnknown) {
                        Log.d(LOG_TAG, "Phantom conference call Scenario");
                    }
                } else if (dc.state != DriverCallIms.State.END) {
                   Log.d(LOG_TAG, "MO phantom Call Scenario. State = " + dc.state);
                   isUnknown = true;
                }
                if (isUnknown && !mNeedIgnoreCalls) {
                    sendIncomingCallIntent(callSession, dc.index, true, dc.state, dc.number);
                }
            }
            // If state is not END then add call to list of active calls
            if ( dc.state != DriverCallIms.State.END ) {
                dcMap.put(Integer.toString(dc.index), dc);
            }
        }
        // Second pass to check if all Call Sessions are still active, dc will not contain
        // a disconnected call in the dc List, remove any call sessions that are not present
        // in dc List.
        synchronized(mCallList) {
            for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it =
                    mCallList.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, ImsCallSessionImpl> e = it.next();
                if (dcMap.get(e.getValue().getCallId()) == null) { // Call Dropped!
                    // CallStartFailed/CallTerminated are triggered during updateCall
                    // when call state is END. Also callsession close is triggered by the
                    // component that creates the session.
                    // Here just remove the session from tracker.
                    it.remove();
                    notifyCallRemoved(e.getValue());
                }
            }
        }
    }

    public void onNotifyCallResumed() {
        Log.d(LOG_TAG, "onNotifyCallResumed");
        ImsCallSessionImpl callSession = getMptySession();
        if ((callSession != null) && (callSession.mIsConfInProgress)) {
            callSession.invokeCallResume(callSession, callSession.getCallProfile());
        }
        setConfInProgress(false);
    }

    public void setConfInProgress(boolean value) {
        Log.d(LOG_TAG, "setConfInProgress value = " + value);
        ImsCallSessionImpl callSession = getMptySession();
        if (callSession != null) {
            callSession.mIsConfInProgress = value;
        }
    }

    private ImsCallSessionImpl getMptySession() {
        synchronized(mCallList) {
            for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it =
                    mCallList.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, ImsCallSessionImpl> e = it.next();
                ImsCallSessionImpl callSession = (ImsCallSessionImpl) e.getValue();
                if (callSession.isMultiparty()) {
                    return callSession;
                }
            }
        }
        return null;
    }

    @Override
    public void onDisconnected(ImsCallSessionImpl session) {
    }

    @Override
    public void onClosed(ImsCallSessionImpl session) {
        Log.d(LOG_TAG, "onClosed for session " + session);
        // For all the states after DIALING state is reported by RIL
        if (mCallList != null) {
            synchronized(mCallList) {
                for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it
                        = mCallList.entrySet().iterator(); it.hasNext();) {
                    Map.Entry<String, ImsCallSessionImpl> e = it.next();
                    Log.d (LOG_TAG, "List is " + e.getValue() + " session is " + session);
                    if (e.getValue() == session) {
                        // Here just remove the session from tracker.
                        Log.d(LOG_TAG, "Removing session on close " + session);
                        it.remove();
                        notifyCallRemoved(e.getValue());
                    }
                }
            }
        }
        // When there is a pending session waiting for a call object with DIALING state
        if (mPendingSessionList != null) {
            synchronized(mPendingSessionList) {
                for (Iterator<ImsCallSessionImpl> it = mPendingSessionList.iterator();
                        it.hasNext();) {
                    ImsCallSessionImpl s = it.next();
                    if (s == session) {
                        // Here just remove the session from tracker.
                        Log.d(LOG_TAG, "Removing session on close " + session);
                        it.remove();
                        notifyCallRemoved(s);
                    }
                }
            }
        }
    }

    @Override
    public void onUnsolCallModify(ImsCallSessionImpl session, CallModify callModify) {
    }

    public void handleModifyCallRequest(CallModify cm) {
        if (cm != null) {
            ImsCallSessionImpl callSession = null;
            synchronized(mCallList) {
                callSession = mCallList.get(Integer.toString(cm.call_index));
            }
            if (callSession != null) {
                callSession.getImsCallModification().onReceivedModifyCall(cm);
            } else {
                loge("handleModifyCallRequest Error: callSession is null");
            }
        } else {
            loge("handleModifyCallRequest Error: Null Call Modify request ");
        }
    }

    /**
     * Create a call session
     * @param profile - ImsCallProfile associated with this call
     * @param listener - listner for the call session
     * @return IImsCallSession object or null on failure
     */
    public ImsCallSessionImpl createCallSession(ImsCallProfile profile,
            IImsCallSessionListener listener) {
        ImsCallSessionImpl session = new ImsCallSessionImpl(profile, listener, mCi, mContext, this);
        session.addListener(this);
        session.updateVtGlobalCapability(mIsVtSupportedGlobally);
        synchronized(mPendingSessionList) {
            mPendingSessionList.add(session);
            notifyCallAdded(session);
        }
        return session;
    }

    /**
     * Get a call session associated with the callId
     * @param callId
     * @return IImsCallSession object
     */
    public ImsCallSessionImpl getCallSession(String callId) {
        ImsCallSessionImpl session = null;
        synchronized(mCallList) {
            session = mCallList.get(callId);
        }
        return session;
    }

    /**
     * Parse the phone numbers from forwarded call history string to be displayed to end user
     * @param history
     * @return String[] object
     */
    private void parsePhoneNumbers(String[] history) {
        if (history == null) {
            loge("parsePhoneNumbers: History is null.");
            return;
        }

        final Pattern p = Pattern.compile("(.*?)(\\+?\\d+)((?s).*)");
        for (int i = 0; i < history.length; i++) {
            final Matcher m = p.matcher(history[i]);
            if (m.matches() ) {
                history[i] = m.group(2);
            } else {
                history[i] = "";
                loge("parsePhoneNumbers: string format incorrect" + history[i]);
            }
        }
    }

    /**
     * Handle the call state changes for incoming (MT) Hold/Resume as part of
     * the UNSOL_SUPP_SVC_NOTIFICATION message.
     * TODO: Handle other supp_svc info here?
     *
     * @param info
     */
    public void handleSuppSvcUnsol(ImsQmiIF.SuppSvcNotification info) {
        Log.d(LOG_TAG, "handleSuppSvcUnsol connId= " + info.getConnId());

        synchronized (mCallList) {
            ImsCallSessionImpl callSession = mCallList.get(Integer.toString(info.getConnId()));
            if (callSession != null) {
                ImsSuppServiceNotification suppServiceInfo = new ImsSuppServiceNotification();
                suppServiceInfo.notificationType = info.getNotificationType();
                suppServiceInfo.code = info.getCode();
                suppServiceInfo.index = info.getIndex();
                suppServiceInfo.number = info.getNumber();

                final String forwardedCallHistory = info.getHistoryInfo();
                if (forwardedCallHistory != null && !forwardedCallHistory.isEmpty() ) {
                    suppServiceInfo.history = forwardedCallHistory.split("\r\n");
                    parsePhoneNumbers(suppServiceInfo.history);
                }
                Log.d(LOG_TAG, "handleSuppSvcUnsol suppNotification= " + suppServiceInfo);
                callSession.updateSuppServiceInfo(suppServiceInfo);
            } else {
                Log.e(LOG_TAG, "No call session found for number: " + info.getNumber());
            }
        }
    }

    /**
     * Handles Conference refresh Info notified through UNSOL_REFRESH_CONF_INFO message
     * @param ar - the AsyncResult object that contains the refresh Info information
     */
    public void handleRefreshConfInfo(ImsQmiIF.ConfInfo confRefreshInfo) {
        byte[] confInfoBytes = null;
        int state = -1;
        ImsCallSessionImpl callSession = null;
        if (confRefreshInfo != null) {
            final com.google.protobuf.micro.ByteStringMicro refreshConfInfoUri = confRefreshInfo
                    .getConfInfoUri();
            if (refreshConfInfoUri != null
                    && refreshConfInfoUri.size() >= 1) {
                confInfoBytes = new byte[refreshConfInfoUri.size()];
                refreshConfInfoUri.copyTo(confInfoBytes, 0);
                if (confRefreshInfo.hasConfCallState()) {
                    state = confRefreshInfo.getConfCallState();
                } else {
                    /*
                     * defaulting to foreground call for backward compatibility before call state
                     * was added to the interface
                     */
                    state = ImsQmiIF.FOREGROUND;
                }
                callSession = getCallSessionWithMptyBitSet(state);
            }
        }

        Log.d(LOG_TAG, "handleRefreshConfInfo: confCallState = " + state + ", callSession = "
                + callSession + ", confInfoBytes: " + confInfoBytes);
        /*
         * UE subscribes for conference xml as soon as it establishes session with conference
         * server.Multiparty bit will be updated only through Get_current_calls after all the
         * participants are merged to the call. So refresh info can be received during the interval
         * in which the conference request is sent and before the conference call reflects in the
         * Get_current-calls
         */
        if (confInfoBytes != null && callSession != null) {
            Log.d(LOG_TAG, "Update UI for Conference");
            callSession.notifyConfInfo(confInfoBytes);
        } else {
            Log.e(LOG_TAG, "No CallSession with Multiparty bit set is found. Some Error!!!");
        }
    }

    /**
     * Handle the TTY mode changes as part of the UNSOL_TTY_NOTIFICATION message.
     *
     * @param mode the mode informed via the indication
     */
    public void handleTtyModeChangeUnsol(int mode) {
        ImsCallSessionImpl callSession = null;

        // Check if any call session is active.
        synchronized(mCallList) {
            for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it =
                    mCallList.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, ImsCallSessionImpl> e = it.next();
                if (e.getValue().getState() == ImsCallSession.State.ESTABLISHED) { // Call active
                    callSession = (ImsCallSessionImpl) e.getValue();
                    callSession.notifyTtyModeChange(mode);
                    break;
                }
            }
        }

        if (callSession == null) {
            Log.e(LOG_TAG, "No Active call session found for TTY mode change");
        }
    }

    private ImsCallSessionImpl getCallSessionWithMptyBitSet(int state) {
        synchronized(mCallList) {
            for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it = mCallList.entrySet()
                    .iterator(); it.hasNext();) {
                Map.Entry<String, ImsCallSessionImpl> e = it.next();
                ImsCallSessionImpl session = (ImsCallSessionImpl) e.getValue();
                DriverCallIms.State dcState = session.getDriverCallState();
                Log.d(LOG_TAG, "getCallSessionWithMptyBitSet:: ImsCallSession state = "
                        + session.getState() + ", isMultiparty = " + session.isMultiparty());
                if (session.isMultiparty() == true) {
                    Log.d(LOG_TAG, "ImsCallSession found with Multiparty bit set");
                    if ((dcState == DriverCallIms.State.DIALING ||
                            dcState == DriverCallIms.State.ALERTING ||
                            dcState == DriverCallIms.State.ACTIVE)
                            && (state == ImsQmiIF.FOREGROUND)) {
                        Log.d(LOG_TAG, "Foreground Conference callSession found");
                        return session;
                    } else if ((dcState == DriverCallIms.State.HOLDING)
                            && (state == ImsQmiIF.BACKGROUND)) {
                        Log.d(LOG_TAG, "Background Conference callSession found");
                        return session;
                    } else if ((dcState == DriverCallIms.State.INCOMING ||
                            dcState == DriverCallIms.State.WAITING)
                            && (state == ImsQmiIF.RINGING)) {
                        Log.d(LOG_TAG, "Ringing Conference callSession found");
                        return session;
                    }
                }
            }

            for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it = mCallList.entrySet()
                    .iterator(); it.hasNext();) {
                Map.Entry<String, ImsCallSessionImpl> e = it.next();
                ImsCallSessionImpl session = (ImsCallSessionImpl) e.getValue();
                if (session.isConfInProgress()) {
                    if (state == ImsQmiIF.FOREGROUND) {
                        Log.d(LOG_TAG, "Foreground ImsCallSession found during Conference setup");
                        return session;
                    }
                } else {
                    if (state == ImsQmiIF.BACKGROUND) {
                        Log.d(LOG_TAG, "Background ImsCallSession found");
                    }
                }
            }
            return null;
        }
    }

    /**
     * Gets list of call sessions that are in the given state.
     * @param state State of the call.
     * @return List of call session objects that have {@code state}
     */
    /* package */
    List<ImsCallSessionImpl> getCallSessionByState(DriverCallIms.State state) {
        List<ImsCallSessionImpl> sessionList = new ArrayList<ImsCallSessionImpl>();
        if (state == null) return sessionList;
        synchronized(mCallList) {
            for (Map.Entry<String, ImsCallSessionImpl> e : mCallList.entrySet()) {
                final ImsCallSessionImpl session = e.getValue();
                if (session.getInternalState() == state) {
                    sessionList.add(session);
                }
            }
        }

        // TODO Do we need to iterate through mPendingSessionList as well??
        return sessionList;
    }

    /**
     * Gets a call session with give media id.
     * @param mediaId Media id of the session to be searched.
     * @return Call session with {@code mediaId}
     */
    public ImsCallSessionImpl findSessionByMediaId(int mediaId) {
        synchronized(mCallList) {
            for (Map.Entry<String, ImsCallSessionImpl> e : mCallList.entrySet()) {
                final ImsCallSessionImpl session = e.getValue();
                if (session.getMediaId() == mediaId) {
                    return session;
                }
            }
        }
        return null;
    }

    public void handleHandover(ImsQmiIF.Handover handover) {
        Log.d(LOG_TAG, "in handleHandover");
        HandoverInfo response = new ImsServiceClassTracker.HandoverInfo();
        response.mType = handover.getType();

        if (handover.hasSrcTech()) {
            response.mSrcTech = handover.getSrcTech();
        }

        if (handover.hasTargetTech()) {
            response.mTargetTech = handover.getTargetTech();
        }

        if (!handover.hasSrcTech() && !handover.hasTargetTech()) {
            handleSrvccStateChanged(response.mType);
            return;
        }
        if (handover.hasHoExtra() && handover.getHoExtra() != null) {
            ImsQmiIF.Extra extra = handover.getHoExtra();
            if (extra.hasType()) {
                response.mExtraType = extra.getType();
            }

            if (extra.hasExtraInfo()) {
                response.mExtraInfo = new byte[extra.getExtraInfo().size()];
                extra.getExtraInfo().copyTo(response.mExtraInfo, 0);
            }
        }
        if (handover.hasErrorCode()) {
            response.mErrorCode = handover.getErrorCode();
        }

        if (handover.hasErrorMessage()) {
            response.mErrorMessage = handover.getErrorMessage();
        }

        Log.d(LOG_TAG, "hoState: " + response.mType + " srcTech: " + response.mSrcTech +
                " tarTech: " + response.mTargetTech + " extraType: " + response.mExtraType +
                " extraInfo: " + response.mExtraInfo + " ErrorCode: " +
                response.mErrorCode + " errorMessage: " + response.mErrorMessage);
        for (Iterator<Map.Entry<String, ImsCallSessionImpl>> it = mCallList.entrySet()
                .iterator(); it.hasNext();) {
             Map.Entry<String, ImsCallSessionImpl> e = it.next();
             ImsCallSessionImpl callSession = (ImsCallSessionImpl) e.getValue();
             if (callSession != null) {
                 callSession.handleHandover(response.mType, response.mSrcTech, response.mTargetTech,
                         response.mExtraType, response.mExtraInfo, response.mErrorCode,
                         response.mErrorMessage);
             } else {
                 Log.d(LOG_TAG, "No more call sessions found");
             }
        }
    }

    /**
     * Registers call list listener.
     * @param listener Listener to registered
     * @see ICallListListener
     */
    /* package */void addListener(ICallListListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("addListener error: listener is null.");
        }

        // Note: This will do linear search, O(N).
        // This is acceptable since arrays size is small.
        if (!mCallListListeners.contains(listener)) {
            mCallListListeners.add(listener);
        } else {
            loge("addListener error: Duplicate listener, " + listener);
        }
    }

    /**
     * Unregisters call list listener. Note: Only {@code ImsServiceClass.MMTEL} is supported.
     * @param listener Listener to unregistered
     * @see ICallListListener
     */
    /* package */void removeListener(ICallListListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("removeListener error: listener is null.");
        }

        // Note: This will do linear search, O(N).
        // This is acceptable since arrays size is small.
        if (mCallListListeners.contains(listener)) {
            mCallListListeners.remove(listener);
        } else {
            loge("removeListener error: Listener not found, " + listener);
        }
    }

    // TODO Create CallList class and hide listeners, registration, notification in that class.
    private void notifyCallAdded(ImsCallSessionImpl session) {
        for (ICallListListener listener : mCallListListeners) {
            listener.onCallSessionAdded(session);
        }
    }

    private void addCall(Integer id, ImsCallSessionImpl session, boolean notify) {
        synchronized (mCallList) {
            mCallList.put(id.toString(), session);
        }
        if (notify) {
            notifyCallAdded(session);
        }
    }

    private void notifyCallRemoved(ImsCallSessionImpl session) {
        for (ICallListListener listener : mCallListListeners) {
            listener.onCallSessionRemoved(session);
        }
    }

    public void sendIncomingCallIntent(ImsCallSessionImpl session, int index, boolean isUnknown,
            DriverCallIms.State state, String number) {
        try {
            mIncomingCallIntent.send(mContext, ImsManager.INCOMING_CALL_RESULT_CODE,
                    createIncomingCallIntent(Integer.toString(index), false, isUnknown,
                    state, number));
            addCall(index, session, true);
        } catch (PendingIntent.CanceledException e) {
            Log.e (LOG_TAG, "Incoming Call intent Canceled " + e);
        }
    }

    private void handleSrvccStateChanged(int state) {
        switch (state) {
            case ImsQmiIF.START:
                mSrvccStateFromIms = Call.SrvccState.STARTED;
                break;
            case ImsQmiIF.COMPLETE_SUCCESS:
                mSrvccStateFromIms = Call.SrvccState.COMPLETED;
                break;
            case ImsQmiIF.COMPLETE_FAIL:
                mSrvccStateFromIms = Call.SrvccState.FAILED;
                break;
            case ImsQmiIF.CANCEL:
                mSrvccStateFromIms = Call.SrvccState.CANCELED;
                break;
        }
        Log.d (LOG_TAG, "handleSrvccStateChanged mSrvccStateFromIms = " + mSrvccStateFromIms);
        // When srvcc state completed received on IMS reset all flags
        if (isSrvccCompleted(mSrvccStateFromIms)) {
            mNeedIgnoreCalls = false;
        }
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(LOG_TAG, msg);
    }
}
