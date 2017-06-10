/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 * Not a Contribution
 *
 * Copyright (C) 2006 The Android Open Source Project
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

package com.qualcomm.qti.ims.internal.telephony;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.IBluetoothHeadsetPhone;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.net.sip.SipManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.MSimTelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.CallerInfo;
import com.android.internal.telephony.CallerInfoAsyncQuery;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.MmiCode;
import com.android.internal.telephony.MSimConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyCapabilities;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.cdma.CdmaConnection;
import com.android.internal.telephony.sip.SipPhone;
import com.google.android.collect.Maps;
import com.android.internal.util.Objects;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Misc utilities for the Phone app.
 */
public class PhoneUtils {
    private static final int IMS_MEDIA_INIT_SUCCESS = 0;
    private static final String LOG_TAG = "imsPhoneUtils";
    private static final boolean DBG = true;

    // Do not check in with VDBG = true, since that may write PII to the system
    // log.
    private static final boolean VDBG = false;

    // Return codes from placeCall()
    static final int CALL_STATUS_DIALED = 0; // The number was successfully
                                                // dialed
    static final int CALL_STATUS_DIALED_MMI = 1; // The specified number was an
                                                    // MMI code
    static final int CALL_STATUS_FAILED = 2; // The call failed

    // State of the Phone's audio modes
    // Each state can move to the other states, but within the state only
    // certain
    // transitions for AudioManager.setMode() are allowed.
    static final int AUDIO_IDLE = 0;
    /** audio behaviour at phone idle */
    static final int AUDIO_RINGING = 1;
    /** audio behaviour while ringing */
    static final int AUDIO_OFFHOOK = 2;
    /** audio behaviour while in call. */

    // USSD string length for MMI operations
    static final int MIN_USSD_LEN = 1;
    static final int MAX_USSD_LEN = 160;

    /** Speaker state, persisting between wired headset connection events */
    private static boolean sIsSpeakerEnabled = false;

    /** Hash table to store mute (Boolean) values based upon the connection. */
    private static Hashtable<Connection, Boolean> sConnectionMuteTable
                                              = new Hashtable<Connection, Boolean>();

    /** Static handler for the connection/mute tracking */
    private static ConnectionHandler mConnectionHandler;

    /** Phone state changed event */
    private static final int PHONE_STATE_CHANGED = -1;

    /** Define for not a special CNAP string */
    private static final int CNAP_SPECIAL_CASE_NO = -1;

    /** Noise suppression status as selected by user */
    private static boolean sIsNoiseSuppressionEnabled = true;

    /**
     * Handler that tracks the connections and updates the value of the Mute
     * settings for each connection as needed.
     */
    private static class ConnectionHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case PHONE_STATE_CHANGED:
                if (DBG)
                    log("ConnectionHandler: updating mute state for each connection");

                CallManager cm = (CallManager) ar.userObj;

                // update the foreground connections, if there are new
                // connections.
                // Have to get all foreground calls instead of the active one
                // because there may two foreground calls co-exist in shore
                // period
                // (a racing condition based on which phone changes firstly)
                // Otherwise the connection may get deleted.
                List<Connection> fgConnections = new ArrayList<Connection>();
                for (Call fgCall : cm.getForegroundCalls()) {
                    if (!fgCall.isIdle()) {
                        fgConnections.addAll(fgCall.getConnections());
                    }
                }
                for (Connection cn : fgConnections) {
                    if (sConnectionMuteTable.get(cn) == null) {
                        sConnectionMuteTable.put(cn, Boolean.FALSE);
                    }
                }

                // mute is connection based operation, we need loop over
                // all background calls instead of the first one to update
                // the background connections, if there are new connections.
                List<Connection> bgConnections = new ArrayList<Connection>();
                for (Call bgCall : cm.getBackgroundCalls()) {
                    if (!bgCall.isIdle()) {
                        bgConnections.addAll(bgCall.getConnections());
                    }
                }
                for (Connection cn : bgConnections) {
                    if (sConnectionMuteTable.get(cn) == null) {
                        sConnectionMuteTable.put(cn, Boolean.FALSE);
                    }
                }

                // Check to see if there are any lingering connections here
                // (disconnected connections), use old-school iterators to avoid
                // concurrent modification exceptions.
                Connection cn;
                for (Iterator<Connection> cnlist = sConnectionMuteTable
                        .keySet().iterator(); cnlist.hasNext();) {
                    cn = cnlist.next();
                    if (!fgConnections.contains(cn)
                            && !bgConnections.contains(cn)) {
                        if (DBG)
                            log("connection '" + cn
                                    + "' not accounted for, removing.");
                        for (Connection fgcn : fgConnections) {
                            if (Objects.equal(cn.getAddress(),
                                    fgcn.getAddress())) {
                                Boolean bMute = sConnectionMuteTable.get(cn);
                                log("updating fg conn '" + fgcn
                                        + "' wth mute value: " + bMute
                                        + " address: " + fgcn.getAddress());
                                sConnectionMuteTable.put(fgcn, bMute);
                                break;
                            }
                        }

                        for (Connection bgcn : bgConnections) {
                            if (Objects.equal(cn.getAddress(),
                                    bgcn.getAddress())) {
                                Boolean bMute = sConnectionMuteTable.get(cn);
                                log("updating bg conn '" + bgcn
                                        + "' wth mute value: " + bMute
                                        + " address: " + bgcn.getAddress());
                                sConnectionMuteTable.put(bgcn, bMute);
                                break;
                            }
                        }
                        cnlist.remove();
                    }
                }

                // Restore the mute state of the foreground call if we're not
                // IDLE,
                // otherwise just clear the mute state. This is really saying
                // that
                // as long as there is one or more connections, we should update
                // the mute state with the earliest connection on the foreground
                // call, and that with no connections, we should be back to a
                // non-mute state.
                if (cm.getState() != PhoneConstants.State.IDLE) {
                    restoreMuteState();
                } else {
                    setMuteInternal(cm.getFgPhone(), false);
                }

                break;
            }
        }
    }

    /**
     * Register the ConnectionHandler with the phone, to receive connection
     * events
     */
    public static void initializeConnectionHandler(CallManager cm) {
        if (mConnectionHandler == null) {
            mConnectionHandler = new ConnectionHandler();
        }
        // pass over cm as user.obj
        cm.registerForPreciseCallStateChanged(mConnectionHandler,
                PHONE_STATE_CHANGED, cm);
    }

    /** This class is never instantiated. */
    private PhoneUtils() {
    }

    /**
     * Answer the currently-ringing call.
     *
     * @return true if we answered the call, or false if there wasn't actually a
     *         ringing incoming call, or some other error occurred.
     *
     * @see #answerAndEndHolding(CallManager, Call)
     * @see #answerAndEndActive(CallManager, Call)
     */
    /* package */static boolean answerCall(Call ringingCall) {
        return answerCall(ringingCall, Phone.CALL_TYPE_UNKNOWN);
    }

    static boolean answerCall(int callType) {
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Phone imsPhone = getImsPhone(cm);
        Call ringing = imsPhone.getRingingCall();
        return answerCall(ringing, callType);
    }

    /**
     * Answer the currently-ringing call.
     *
     * @return true if we answered the call, or false if there wasn't actually a
     *         ringing incoming call, or some other error occurred.
     *
     * @see #answerAndEndHolding(CallManager, Call)
     * @see #answerAndEndActive(CallManager, Call)
     */
    /* package */static boolean answerCall(Call ringingCall, int answerCallType) {
        log("answerCall(" + ringingCall + ")..." + "calltype:" + answerCallType);
        // If the ringer is currently ringing and/or vibrating, stop it
        // right now (before actually answering the call.)

        final Phone phone = ringingCall.getPhone();
        final boolean phoneIsCdma = (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA);
        boolean answered = false;
        IBluetoothHeadsetPhone btPhone = null;

        if (ringingCall != null && ringingCall.isRinging()) {
            if (DBG)
                log("answerCall: call state = " + ringingCall.getState());
            try {

                final boolean isRealIncomingCall = isRealIncomingCall(ringingCall
                        .getState());

                // if (DBG) log("sPhone.acceptCall");
                CallManager.getInstance().acceptCall(ringingCall,
                        answerCallType);
                answered = true;

                // Always reset to "unmuted" for a freshly-answered call
                setMute(false);

                //setAudioMode();

                PhoneGlobals app = PhoneGlobals.getInstance();
                // When answering a phone call, the user will move the phone
                // near to her/his ear
                // and start conversation, without checking its speaker status.
                // If some other
                // application turned on the speaker mode before the call and
                // didn't turn it off,
                // Phone app would need to be responsible for the speaker phone.
                // Here, we turn off the speaker if
                // - the phone call is the first in-coming call,
                // - we did not activate speaker by ourselves during the process
                // above, and
                // - Bluetooth headset is not in use.
                if (isRealIncomingCall && isSpeakerOn(app)) {
                    // This is not an error but might cause users' confusion.
                    // Add log just in case.
                    Log.i(LOG_TAG,
                            "Forcing speaker off due to new incoming call...");
                    turnOnSpeaker(app, false, true);
                }
            } catch (CallStateException ex) {
                Log.w(LOG_TAG, "answerCall: caught " + ex, ex);

            }
        }
        return answered;
    }

    /**
     * Smart "hang up" helper method which hangs up exactly one connection,
     * based on the current Phone state, as follows:
     * <ul>
     * <li>If there's a ringing call, hang that up.
     * <li>Else if there's a foreground call, hang that up.
     * <li>Else if there's a background call, hang that up.
     * <li>Otherwise do nothing.
     * </ul>
     *
     * @return true if we successfully hung up, or false if there were no active
     *         calls at all.
     */
    static boolean hangup(CallManager cm) {
        boolean hungup = false;
        Call ringing = cm.getFirstActiveRingingCall();
        Call fg = cm.getActiveFgCall();
        Call bg = cm.getFirstActiveBgCall();

        if (!ringing.isIdle()) {
            log("hangup(): hanging up ringing call");
            hungup = hangupRingingCall(ringing);
        } else if (!fg.isIdle()) {
            log("hangup(): hanging up foreground call");
            hungup = hangup(fg);
        } else if (!bg.isIdle()) {
            log("hangup(): hanging up background call");
            hungup = hangup(bg);
        } else {
            // No call to hang up! This is unlikely in normal usage,
            // since the UI shouldn't be providing an "End call" button in
            // the first place. (But it *can* happen, rarely, if an
            // active call happens to disconnect on its own right when the
            // user is trying to hang up..)
            log("hangup(): no active call to hang up");
        }
        if (DBG)
            log("==> hungup = " + hungup);

        return hungup;
    }

    static boolean hangupRingingCall(Call ringing) {
        if (DBG)
            log("hangup ringing call");
        int phoneType = ringing.getPhone().getPhoneType();
        Call.State state = ringing.getState();

        if (state == Call.State.INCOMING) {
            // Regular incoming call (with no other active calls)
            log("hangupRingingCall(): regular incoming call: hangup()");
            return hangup(ringing);
        } else if (state == Call.State.WAITING) {
            // Call-waiting: there's an incoming call, but another call is
            // already active.
            // TODO: It would be better for the telephony layer to provide
            // a "hangupWaitingCall()" API that works on all devices,
            // rather than us having to check the phone type here and do
            // the notifier.sendCdmaCallWaitingReject() hack for CDMA phones.
            if (phoneType != PhoneConstants.PHONE_TYPE_CDMA) {
                // handleWaitingCallOnLchSub(ringing.getPhone().getSubscription(),
                // false);
                // Otherwise, the regular hangup() API works for
                // call-waiting calls too.
                log("hangupRingingCall(): call-waiting call: hangup()");
                return hangup(ringing);
            }
        } else {
            // Unexpected state: the ringing call isn't INCOMING or
            // WAITING, so there's no reason to have called
            // hangupRingingCall() in the first place.
            // (Presumably the incoming call went away at the exact moment
            // we got here, so just do nothing.)
            Log.w(LOG_TAG, "hangupRingingCall: no INCOMING or WAITING call");
            return false;
        }
        return false;
    }

    static boolean hangupActiveCall(Call foreground) {
        if (DBG)
            log("hangup active call");
        return hangup(foreground);
    }

    static boolean hangupHoldingCall(Call background) {
        if (DBG)
            log("hangup holding call");
        return hangup(background);
    }

    /**
     * Used in CDMA phones to end the complete Call session
     *
     * @param phone
     *            the Phone object.
     * @return true if *any* call was successfully hung up
     */
    static boolean hangupRingingAndActive(Phone phone) {
        boolean hungUpRingingCall = false;
        boolean hungUpFgCall = false;
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Call ringingCall = cm.getFirstActiveRingingCall();
        Call fgCall = cm.getActiveFgCall();

        // Hang up any Ringing Call
        if (!ringingCall.isIdle()) {
            log("hangupRingingAndActive: Hang up Ringing Call");
            hungUpRingingCall = hangupRingingCall(ringingCall);
        }

        // Hang up any Active Call
        if (!fgCall.isIdle()) {
            log("hangupRingingAndActive: Hang up Foreground Call");
            hungUpFgCall = hangupActiveCall(fgCall);
        }

        return hungUpRingingCall || hungUpFgCall;
    }

    /**
     * Trivial wrapper around Call.hangup(), except that we return a boolean
     * success code rather than throwing CallStateException on failure.
     *
     * @return true if the call was successfully hung up, or false if the call
     *         wasn't actually active.
     */
    static boolean hangup(Call call) {
        try {
            CallManager cm = PhoneGlobals.getInstance().mCM;

            if (call.getState() == Call.State.ACTIVE && cm.hasActiveBgCall()) {
                // handle foreground call hangup while there is background call
                log("- hangup(Call): hangupForegroundResumeBackground...");
                cm.hangupForegroundResumeBackground(cm.getFirstActiveBgCall());
            } else {
                log("- hangup(Call): regular hangup()...");
                call.hangup();
            }
            return true;
        } catch (CallStateException ex) {
            Log.e(LOG_TAG, "Call hangup: caught " + ex, ex);
        }

        return false;
    }

    /**
     * Trivial wrapper around Connection.hangup(), except that we silently do
     * nothing (rather than throwing CallStateException) if the connection
     * wasn't actually active.
     */
    static void hangup(Connection c) {
        try {
            if (c != null) {
                c.hangup();
            }
        } catch (CallStateException ex) {
            Log.w(LOG_TAG, "Connection hangup: caught " + ex, ex);
        }
    }

    static boolean answerAndEndHolding(CallManager cm, Call ringing) {
        if (DBG)
            log("end holding & answer waiting: 1");
        if (!hangupHoldingCall(cm.getFirstActiveBgCall())) {
            Log.e(LOG_TAG, "end holding failed!");
            return false;
        }

        if (DBG)
            log("end holding & answer waiting: 2");
        return answerCall(ringing);

    }

    public static void hangupWithReason(int callId, String userUri,
            boolean mpty, int failCause, String errorInfo, CallManager cm) {
        Phone phone = getImsPhone(cm);
        if (phone != null) {
            Log.d(LOG_TAG, "hangupWithReason");
            try {
                phone.hangupWithReason(callId, userUri, mpty, failCause,
                        errorInfo);
            } catch (CallStateException e) {
                Log.e("PhoneUtils", "Exception in hangupWithReason" + e);
            }
        }
    }

    /**
     * Answers the incoming call specified by "ringing", and ends the currently
     * active phone call.
     *
     * This method is useful when's there's an incoming call which we cannot
     * manage with the current call. e.g. when you are having a phone call with
     * CDMA network and has received a SIP call, then we won't expect our
     * telephony can manage those phone calls simultaneously. Note that some
     * types of network may allow multiple phone calls at once; GSM allows to
     * hold an ongoing phone call, so we don't need to end the active call. The
     * caller of this method needs to check if the network allows multiple phone
     * calls or not.
     *
     * @see #answerCall(Call)
     * @see InCallScreen#internalAnswerCall()
     */
    /* package */static boolean answerAndEndActive(CallManager cm, Call ringing) {
        if (DBG)
            log("answerAndEndActive()...");

        // Unlike the answerCall() method, we *don't* need to stop the
        // ringer or change audio modes here since the user is already
        // in-call, which means that the audio mode is already set
        // correctly, and that we wouldn't have started the ringer in the
        // first place.

        // hanging up the active call also accepts the waiting call
        // while active call and waiting call are from the same phone
        // i.e. both from GSM phone
        if (!hangupActiveCall(cm.getActiveFgCall())) {
            Log.w(LOG_TAG, "end active call failed!");
            return false;
        }

        return true;
    }

    public static int placeImsCall(Context context, String number, int callType) {
        boolean isEmergencyNumber = PhoneNumberUtils.isLocalEmergencyNumber(
                number, context);
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Phone imsphone = getImsPhone(cm);
        return placeCall(context, imsphone, number, null, isEmergencyNumber,
                null, callType, null);
    }

    /**
     * Dial the number using the phone passed in.
     *
     * If the connection is establised, this method issues a sync call that may
     * block to query the caller info. TODO: Change the logic to use the async
     * query.
     *
     * @param context
     *            To perform the CallerInfo query.
     * @param phone
     *            the Phone object.
     * @param number
     *            to be dialed as requested by the user. This is NOT the phone
     *            number to connect to. It is used only to build the call card
     *            and to update the call log. See above for restrictions.
     * @param contactRef
     *            that triggered the call. Typically a 'tel:' uri but can also
     *            be a 'content://contacts' one.
     * @param isEmergencyCall
     *            indicates that whether or not this is an emergency call
     * @param gatewayUri
     *            Is the address used to setup the connection, null if not using
     *            a gateway
     *
     * @return either CALL_STATUS_DIALED or CALL_STATUS_FAILED
     */
    public static int placeCall(Context context, Phone phone, String number,
            Uri contactRef, boolean isEmergencyCall, Uri gatewayUri) {
        if (VDBG) {
            log("placeCall()... number: '" + number + "'" + ", GW:'"
                    + gatewayUri + "'" + ", contactRef:" + contactRef
                    + ", isEmergencyCall: " + isEmergencyCall);
        } else {
            log("placeCall()... number: " + toLogSafePhoneNumber(number)
                    + ", GW: " + (gatewayUri != null ? "non-null" : "null")
                    + ", emergency? " + isEmergencyCall);
        }
        return placeCall(context, phone, number, contactRef, isEmergencyCall,
                gatewayUri, Phone.CALL_TYPE_VOICE, null);
    }

    /**
     * Dial the number using the phone passed in.
     *
     * If the connection is establised, this method issues a sync call that may
     * block to query the caller info. TODO: Change the logic to use the async
     * query.
     *
     * @param context
     *            To perform the CallerInfo query.
     * @param phone
     *            the Phone object.
     * @param number
     *            to be dialed as requested by the user. This is NOT the phone
     *            number to connect to. It is used only to build the call card
     *            and to update the call log. See above for restrictions.
     * @param contactRef
     *            that triggered the call. Typically a 'tel:' uri but can also
     *            be a 'content://contacts' one.
     * @param isEmergencyCall
     *            indicates that whether or not this is an emergency call
     * @param gatewayUri
     *            Is the address used to setup the connection, null if not using
     *            a gateway
     * @param callType
     *            indicates that type of call, used mainly for IMS calls
     * @param extras
     *            callDetails indicating if current call is VoLTE IMS call
     *
     * @return either CALL_STATUS_DIALED or CALL_STATUS_FAILED
     */
    public static int placeCall(Context context, Phone phone, String number,
            Uri contactRef, boolean isEmergencyCall, Uri gatewayUri,
            int callType, String[] extras) {
        if (DBG) {
            log("placeCall '" + number + "' GW:'" + gatewayUri + "'"
                    + " CallType:" + callType);
        }

        final PhoneGlobals app = PhoneGlobals.getInstance();

        boolean useGateway = false;
        if (null != gatewayUri && !isEmergencyCall
                && PhoneUtils.isRoutableViaGateway(number)) { // Filter out MMI,
                                                                // OTA and other
                                                                // codes.
            useGateway = true;
        }

        int status = CALL_STATUS_DIALED;
        Connection connection;
        String numberToDial;
        if (useGateway) {
            // TODO: 'tel' should be a constant defined in framework base
            // somewhere (it is in webkit.)
            if (null == gatewayUri || !"tel".equals(gatewayUri.getScheme())) {
                Log.e(LOG_TAG, "Unsupported URL:" + gatewayUri);
                return CALL_STATUS_FAILED;
            }

            // We can use getSchemeSpecificPart because we don't allow #
            // in the gateway numbers (treated a fragment delim.) However
            // if we allow more complex gateway numbers sequence (with
            // passwords or whatnot) that use #, this may break.
            // TODO: Need to support MMI codes.
            numberToDial = gatewayUri.getSchemeSpecificPart();
        } else {
            numberToDial = number;
        }

        // Return CALL_STATUS_FAILED when there are already two calls or
        // conference call
        Call fgCall = app.mCM.getActiveFgCall();
        if (fgCall != null
                && fgCall.getPhone().getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            if (isConferenceCall(fgCall)
                    || (fgCall.getState() == Call.State.ACTIVE && app.mCM
                            .hasActiveBgCall())) {
                return CALL_STATUS_FAILED;
            }
        }

        // Remember if the phone state was in IDLE state before this call.
        // After calling CallManager#dial(), getState() will return different
        // state.
        boolean initiallyIdle = true;
        if (MSimTelephonyManager.getDefault().isMultiSimEnabled()) {
            for (int i = 0; i < MSimTelephonyManager.getDefault()
                    .getPhoneCount(); i++) {
                if (app.mCM.getState(i) != PhoneConstants.State.IDLE) {
                    initiallyIdle = false;
                    break;
                }
            }
        } else {
            initiallyIdle = app.mCM.getState() == PhoneConstants.State.IDLE;
        }

        // If VT/VS cal is initiated, perform dpl media init.
        // If media init fails then make a voice call instead of VT
        if (callType == Phone.CALL_TYPE_VT || callType == Phone.CALL_TYPE_VT_TX
                || callType == Phone.CALL_TYPE_VT_RX) {
            int error = earlyMediaInit();
            if (error != IMS_MEDIA_INIT_SUCCESS) {
                // Dpl init failed so continue with VoLTE call
                callType = Phone.CALL_TYPE_VOICE;
                Log.e(LOG_TAG,
                        "videocall init failed. Downgrading to VoLTE call");
                // Toast.makeText(app, R.string.vt_init_fail_str,
                // Toast.LENGTH_SHORT)
                // .show();
            }
        }
        try {
            connection = app.mCM.dial(phone, numberToDial, callType, extras);
        } catch (CallStateException ex) {
            // CallStateException means a new outgoing call is not currently
            // possible: either no more call slots exist, or there's another
            // call already in the process of dialing or ringing.
            Log.w(LOG_TAG, "Exception from app.mCM.dial()", ex);
            return CALL_STATUS_FAILED;

            // Note that it's possible for CallManager.dial() to return
            // null *without* throwing an exception; that indicates that
            // we dialed an MMI (see below).
        }

        int phoneType = phone.getPhoneType();

        // On GSM phones, null is returned for MMI codes
        if (null == connection) {
            if ((phoneType == PhoneConstants.PHONE_TYPE_GSM
                    || phoneType == PhoneConstants.PHONE_TYPE_IMS)
                    && gatewayUri == null) {
                if (DBG)
                    log("dialed MMI code: " + number);
                status = CALL_STATUS_DIALED_MMI;
            } else {
                status = CALL_STATUS_FAILED;
            }
        } else {
            // The phone on whilch dial request for voice call is initiated
            // set it as active subscription
            // setActiveSubscription(phone.getSubscription());
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            }

            // Clean up the number to be displayed.
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                number = CdmaConnection.formatDialString(number);
            }
            number = PhoneNumberUtils.extractNetworkPortion(number);
            number = PhoneNumberUtils.convertKeypadLettersToDigits(number);
            number = PhoneNumberUtils.formatNumber(number);

            if (gatewayUri == null) {
                // phone.dial() succeeded: we're now in a normal phone call.
                // attach the URI to the CallerInfo Object if it is there,
                // otherwise just attach the Uri Reference.
                // if the uri does not have a "content" scheme, then we treat
                // it as if it does NOT have a unique reference.
                String content = context.getContentResolver().SCHEME_CONTENT;
                if ((contactRef != null)
                        && (contactRef.getScheme().equals(content))) {
                    Object userDataObject = connection.getUserData();
                    if (userDataObject == null) {
                        connection.setUserData(contactRef);
                    } else {
                        // TODO: This branch is dead code, we have
                        // just created the connection which has
                        // no user data (null) by default.
                        if (userDataObject instanceof CallerInfo) {
                            ((CallerInfo) userDataObject).contactRefUri = contactRef;
                        }
                    }
                }
            } else {
                // Get the caller info synchronously because we need the final
                // CallerInfo object to update the dialed number with the one
                // requested by the user (and not the provider's gateway
                // number).
                CallerInfo info = null;
                String content = phone.getContext().getContentResolver().SCHEME_CONTENT;
                if ((contactRef != null)
                        && (contactRef.getScheme().equals(content))) {
                    info = CallerInfo.getCallerInfo(context, contactRef);
                }

                // Fallback, lookup contact using the phone number if the
                // contact's URI scheme was not content:// or if is was but
                // the lookup failed.
                if (null == info) {
                    info = CallerInfo.getCallerInfo(context, number);
                }
                info.phoneNumber = number;
                connection.setUserData(info);
            }
            setAudioMode();

            if (DBG)
                log("about to activate speaker");
            // Check is phone in any dock, and turn on speaker accordingly

            // See also similar logic in answerCall().
            if (initiallyIdle && isSpeakerOn(app)) {
                // This is not an error but might cause users' confusion. Add
                // log just in case.
                Log.i(LOG_TAG,
                        "Forcing speaker off when initiating a new outgoing call...");
                PhoneUtils.turnOnSpeaker(app, false, true);
            }
        }

        return status;
    }

    /* package */static String toLogSafePhoneNumber(String number) {
        // For unknown number, log empty string.
        if (number == null) {
            return "";
        }

        if (VDBG) {
            // When VDBG is true we emit PII.
            return number;
        }

        // Do exactly same thing as Uri#toSafeString() does, which will enable
        // us to compare
        // sanitized phone numbers.
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < number.length(); i++) {
            char c = number.charAt(i);
            if (c == '-' || c == '@' || c == '.') {
                builder.append(c);
            } else {
                builder.append('x');
            }
        }
        return builder.toString();
    }

    /**
     * @param heldCall
     *            is the background call want to be swapped
     */
    static void switchHoldingAndActive(Call heldCall) {
        log("switchHoldingAndActive()...");
        try {
            CallManager cm = PhoneGlobals.getInstance().mCM;
            if (heldCall.isIdle()) {
                // no heldCall, so it is to hold active call
                cm.switchHoldingAndActive(cm.getFgPhone().getBackgroundCall());
            } else {
                // has particular heldCall, so to switch
                cm.switchHoldingAndActive(heldCall);
            }
            setAudioMode(cm);
        } catch (CallStateException ex) {
            Log.w(LOG_TAG, "switchHoldingAndActive: caught " + ex, ex);
        }
    }

    /**
     * Restore the mute setting from the earliest connection of the foreground
     * call.
     */
    static Boolean restoreMuteState() {
        Phone phone = PhoneGlobals.getInstance().mCM.getFgPhone();

        // get the earliest connection
        Connection c = phone.getForegroundCall().getEarliestConnection();

        // only do this if connection is not null.
        if (c != null) {

            int phoneType = phone.getPhoneType();

            // retrieve the mute value.
            Boolean shouldMute = null;

            // In CDMA, mute is not maintained per Connection. Single mute apply
            // for
            // a call where call can have multiple connections such as
            // Three way and Call Waiting. Therefore retrieving Mute state for
            // latest connection can apply for all connection in that call
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                shouldMute = sConnectionMuteTable.get(phone.getForegroundCall()
                        .getLatestConnection());
            } else if ((phoneType == PhoneConstants.PHONE_TYPE_GSM)
                    || (phoneType == PhoneConstants.PHONE_TYPE_SIP)
                    || (phoneType == PhoneConstants.PHONE_TYPE_IMS)) {
                shouldMute = sConnectionMuteTable.get(c);
            }
            if (shouldMute == null) {
                if (DBG)
                    log("problem retrieving mute value for this connection.");
                shouldMute = Boolean.FALSE;
            }

            // set the mute value and return the result.
            setMute(shouldMute.booleanValue());
            return shouldMute;
        }
        return Boolean.valueOf(getMute());
    }

    public static void mergeCalls() {
        mergeCalls(PhoneGlobals.getInstance().mCM);
    }

    static void mergeCalls(CallManager cm) {
        int phoneType = cm.getFgPhone().getPhoneType();
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
        } else {
            try {
                log("mergeCalls(): calling cm.conference()...");
                cm.conference(cm.getFirstActiveBgCall());
            } catch (CallStateException ex) {
                Log.w(LOG_TAG, "mergeCalls: caught " + ex, ex);
            }
        }
    }

    static void separateCall(Connection c) {
        try {
            if (DBG)
                log("separateCall: " + toLogSafePhoneNumber(c.getAddress()));
            c.separate();
        } catch (CallStateException ex) {
            Log.w(LOG_TAG, "separateCall: caught " + ex, ex);
        }
    }

    public static class VoiceMailNumberMissingException extends Exception {
        VoiceMailNumberMissingException() {
            super();
        }

        VoiceMailNumberMissingException(String msg) {
            super(msg);
        }
    }

    /**
     * Returns true if the specified Call is a "conference call", meaning that
     * it owns more than one Connection object. This information is used to
     * trigger certain UI changes that appear when a conference call is active
     * (like displaying the label "Conference call", and enabling the
     * "Manage conference" UI.)
     *
     * Watch out: This method simply checks the number of Connections, *not*
     * their states. So if a Call has (for example) one ACTIVE connection and
     * one DISCONNECTED connection, this method will return true (which is
     * unintuitive, since the Call isn't *really* a conference call any more.)
     *
     * @return true if the specified call has more than one connection (in any
     *         state.)
     */
    static boolean isConferenceCall(Call call) {
        // CDMA phones don't have the same concept of "conference call" as
        // GSM phones do; there's no special "conference call" state of
        // the UI or a "manage conference" function. (Instead, when
        // you're in a 3-way call, all we can do is display the "generic"
        // state of the UI.) So as far as the in-call UI is concerned,
        // Conference corresponds to generic display.
        final PhoneGlobals app = PhoneGlobals.getInstance();
        int phoneType = call.getPhone().getPhoneType();
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
        } else {
            return call.isMultiparty();
        }
        return false;

        // TODO: We may still want to change the semantics of this method
        // to say that a given call is only really a conference call if
        // the number of ACTIVE connections, not the total number of
        // connections, is greater than one. (See warning comment in the
        // javadoc above.)
        // Here's an implementation of that:
        // if (connections == null) {
        // return false;
        // }
        // int numActiveConnections = 0;
        // for (Connection conn : connections) {
        // if (DBG) log("  - CONN: " + conn + ", state = " + conn.getState());
        // if (conn.getState() == Call.State.ACTIVE) numActiveConnections++;
        // if (numActiveConnections > 1) {
        // return true;
        // }
        // }
        // return false;
    }

    /**
     * Turns on/off speaker.
     *
     * @param context
     *            Context
     * @param flag
     *            True when speaker should be on. False otherwise.
     * @param store
     *            True when the settings should be stored in the device.
     */
    /* package */static void turnOnSpeaker(Context context, boolean flag,
            boolean store) {
        if (DBG)
            log("turnOnSpeaker(flag=" + flag + ", store=" + store + ")...");
        final PhoneGlobals app = PhoneGlobals.getInstance();

        AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);
        audioManager.setSpeakerphoneOn(flag);

        // record the speaker-enable value
        if (store) {
            sIsSpeakerEnabled = flag;
        }

        app.mCM.setEchoSuppressionEnabled(flag);
    }

    /**
     * Restore the speaker mode, called after a wired headset disconnect event.
     */
    static void restoreSpeakerMode(Context context) {
        if (DBG)
            log("restoreSpeakerMode, restoring to: " + sIsSpeakerEnabled);

        // change the mode if needed.
        if (isSpeakerOn(context) != sIsSpeakerEnabled) {
            turnOnSpeaker(context, sIsSpeakerEnabled, false);
        }
    }

    static boolean isSpeakerOn(Context context) {
        AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);
        return audioManager.isSpeakerphoneOn();
    }

    static void turnOnNoiseSuppression(Context context, boolean flag,
            boolean store) {
        if (DBG)
            log("turnOnNoiseSuppression: " + flag);
        AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);

        if (flag) {
            audioManager.setParameters("noise_suppression=auto");
        } else {
            audioManager.setParameters("noise_suppression=off");
        }

        // record the speaker-enable value
        if (store) {
            sIsNoiseSuppressionEnabled = flag;
        }

        // TODO: implement and manage ICON

    }

    static void restoreNoiseSuppression(Context context) {
        if (DBG)
            log("restoreNoiseSuppression, restoring to: "
                    + sIsNoiseSuppressionEnabled);

        // change the mode if needed.
        if (isNoiseSuppressionOn(context) != sIsNoiseSuppressionEnabled) {
            turnOnNoiseSuppression(context, sIsNoiseSuppressionEnabled, false);
        }
    }

    static boolean isNoiseSuppressionOn(Context context) {

        AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);
        String noiseSuppression = audioManager
                .getParameters("noise_suppression");
        if (DBG)
            log("isNoiseSuppressionOn: " + noiseSuppression);
        if (noiseSuppression.contains("off")) {
            return false;
        } else {
            return true;
        }
    }

    /**
     *
     * Mute / umute the foreground phone, which has the current foreground call
     *
     * All muting / unmuting from the in-call UI should go through this wrapper.
     *
     * Wrapper around Phone.setMute() and setMicrophoneMute(). It also updates
     * the connectionMuteTable and mute icon in the status bar.
     *
     */
    static void setMute(boolean muted) {
        CallManager cm = PhoneGlobals.getInstance().mCM;

        // make the call to mute the audio
        setMuteInternal(cm.getFgPhone(), muted);

        // update the foreground connections to match. This includes
        // all the connections on conference calls.
        for (Connection cn : cm.getActiveFgCall().getConnections()) {
            if (sConnectionMuteTable.get(cn) == null) {
                if (DBG)
                    log("problem retrieving mute value for this connection.");
            }
            sConnectionMuteTable.put(cn, Boolean.valueOf(muted));
        }
    }

    /**
     * Internally used muting function.
     */
    private static void setMuteInternal(Phone phone, boolean muted) {
        final PhoneGlobals app = PhoneGlobals.getInstance();
        Context context = app;
        boolean routeToAudioManager = false;
        if (routeToAudioManager) {
            AudioManager audioManager = (AudioManager) context
                    .getSystemService(Context.AUDIO_SERVICE);
            if (DBG)
                log("setMuteInternal: using setMicrophoneMute(" + muted
                        + ")...");
            audioManager.setMicrophoneMute(muted);
        } else {
            if (DBG)
                log("setMuteInternal: using phone.setMute(" + muted + ")...");
            phone.setMute(muted);
        }
    }

    /**
     * Get the mute state of foreground phone, which has the current foreground
     * call
     */
    static boolean getMute() {
        final PhoneGlobals app = PhoneGlobals.getInstance();

        boolean routeToAudioManager = false;
        if (routeToAudioManager) {
            AudioManager audioManager = (AudioManager) app
                    .getSystemService(Context.AUDIO_SERVICE);
            return audioManager.isMicrophoneMute();
        } else {
            return app.mCM.getMute();
        }
    }

    /**
     * Do DPL initialization if the call is a VT call
     */
    /* package */static int earlyMediaInit() {
        if (DBG)
            Log.d(LOG_TAG, "mediaInit()...");
        Context context = PhoneGlobals.getInstance().getApplicationContext();
        // VideoCallManager mVideoCallManager =
        // VideoCallManager.getInstance(context);
        // int error = mVideoCallManager.mediaInit();
        // if (error == VideoCallManager.MEDIA_INIT_SUCCESS) {
        // mVideoCallManager.setFarEndSurface();
        // }
        return 0;
    }

    /* package */static void setAudioMode() {
        setAudioMode(PhoneGlobals.getInstance().mCM);
    }

    /**
     * Sets the audio mode per current phone state.
     */
    /* package */static void setAudioMode(CallManager cm) {
        if (DBG)
            Log.d(LOG_TAG, "setAudioMode()..." + cm.getState());
        Context context = PhoneGlobals.getInstance();
        AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);
        int modeBefore = audioManager.getMode();
        cm.setAudioMode();
        int modeAfter = audioManager.getMode();

        if (modeBefore != modeAfter) {
            // Enable stack dump only when actively debugging ("new Throwable()"
            // is expensive!)
            Log.d(LOG_TAG, "Stack:", new Throwable("stack dump"));
        } else {
            if (DBG)
                Log.d(LOG_TAG, "setAudioMode() no change: "
                        + audioModeToString(modeBefore));
        }
    }

    private static String audioModeToString(int mode) {
        switch (mode) {
        case AudioManager.MODE_INVALID:
            return "MODE_INVALID";
        case AudioManager.MODE_CURRENT:
            return "MODE_CURRENT";
        case AudioManager.MODE_NORMAL:
            return "MODE_NORMAL";
        case AudioManager.MODE_RINGTONE:
            return "MODE_RINGTONE";
        case AudioManager.MODE_IN_CALL:
            return "MODE_IN_CALL";
        default:
            return String.valueOf(mode);
        }
    }

    /**
     * Handles the wired headset button while in-call.
     *
     * This is called from the PhoneApp, not from the InCallScreen, since the
     * HEADSETHOOK button means "mute or unmute the current call" *any* time a
     * call is active, even if the user isn't actually on the in-call screen.
     *
     * @return true if we consumed the event.
     */
    /* package */static boolean handleHeadsetHook(Phone phone, KeyEvent event) {
        if (DBG)
            log("handleHeadsetHook()..." + event.getAction() + " "
                    + event.getRepeatCount());
        final PhoneGlobals app = PhoneGlobals.getInstance();

        // If the phone is totally idle, we ignore HEADSETHOOK events
        // (and instead let them fall through to the media player.)
        if (phone.getState() == PhoneConstants.State.IDLE) {
            return false;
        }

        // Ok, the phone is in use.
        // The headset button button means "Answer" if an incoming call is
        // ringing. If not, it toggles the mute / unmute state.
        //
        // And in any case we *always* consume this event; this means
        // that the usual mediaplayer-related behavior of the headset
        // button will NEVER happen while the user is on a call.

        final boolean hasRingingCall = !phone.getRingingCall().isIdle();
        final boolean hasActiveCall = !phone.getForegroundCall().isIdle();
        final boolean hasHoldingCall = !phone.getBackgroundCall().isIdle();

        if (hasRingingCall && event.getRepeatCount() == 0
                && event.getAction() == KeyEvent.ACTION_UP) {
            // If an incoming call is ringing, answer it (just like with the
            // CALL button):
            int phoneType = phone.getPhoneType();
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                answerCall(phone.getRingingCall());
            } else if ((phoneType == PhoneConstants.PHONE_TYPE_GSM)
                    || (phoneType == PhoneConstants.PHONE_TYPE_SIP)
                    || (phoneType == PhoneConstants.PHONE_TYPE_IMS)) {
                if (hasActiveCall && hasHoldingCall) {
                    if (DBG)
                        log("handleHeadsetHook: ringing (both lines in use) ==> answer!");
                    answerAndEndActive(app.mCM, phone.getRingingCall());
                } else {
                    if (DBG)
                        log("handleHeadsetHook: ringing ==> answer!");
                    // answerCall() will automatically hold the current
                    // active call, if there is one.
                    answerCall(phone.getRingingCall());
                }
            } else {
                throw new IllegalStateException("Unexpected phone type: "
                        + phoneType);
            }
        } else {
            // No incoming ringing call.
            if (event.isLongPress()) {
                if (DBG)
                    log("handleHeadsetHook: longpress -> hangup");
                hangup(app.mCM);
            } else if (event.getAction() == KeyEvent.ACTION_UP
                    && event.getRepeatCount() == 0) {
                Connection c = phone.getForegroundCall().getLatestConnection();
                // If it is NOT an emg #, toggle the mute state. Otherwise,
                // ignore the hook.
                if (c != null
                        && !PhoneNumberUtils.isLocalEmergencyNumber(
                                c.getAddress(), PhoneGlobals.getInstance())) {
                    if (getMute()) {
                        if (DBG)
                            log("handleHeadsetHook: UNmuting...");
                        setMute(false);
                    } else {
                        if (DBG)
                            log("handleHeadsetHook: muting...");
                        setMute(true);
                    }
                }
            }
        }

        // Even if the InCallScreen is the current activity, there's no
        // need to force it to update, because (1) if we answered a
        // ringing call, the InCallScreen will imminently get a phone
        // state change event (causing an update), and (2) if we muted or
        // unmuted, the setMute() call automagically updates the status
        // bar, and there's no "mute" indication in the InCallScreen
        // itself (other than the menu item, which only ever stays
        // onscreen for a second anyway.)
        // TODO: (2) isn't entirely true anymore. Once we return our result
        // to the PhoneApp, we ask InCallScreen to update its control widgets
        // in case we changed mute or speaker state and phones with touch-
        // screen [toggle] buttons need to update themselves.

        return true;
    }

    //
    // Misc UI policy helper functions
    //

    /**
     * @return true if we're allowed to swap calls, given the current state of
     *         the Phone.
     */
    /* package */static boolean okToSwapCalls(CallManager cm) {
        int phoneType = cm.getFgPhone().getPhoneType();
        if ((phoneType == PhoneConstants.PHONE_TYPE_GSM)
                || (phoneType == PhoneConstants.PHONE_TYPE_SIP)
                || (phoneType == PhoneConstants.PHONE_TYPE_IMS)) {
            // GSM: "Swap" is available if both lines are in use and there's no
            // incoming call. (Actually we need to verify that the active
            // call really is in the ACTIVE state and the holding call really
            // is in the HOLDING state, since you *can't* actually swap calls
            // when the foreground call is DIALING or ALERTING.)
            return !cm.hasActiveRingingCall()
                    && (cm.getActiveFgCall().getState() == Call.State.ACTIVE)
                    && (cm.getFirstActiveBgCall().getState() == Call.State.HOLDING);
        } else {
            return true;
        }

    }

    /**
     * @return true if we're allowed to merge calls, given the current state of
     *         the Phone.
     */
    /* package */static boolean okToMergeCalls(CallManager cm) {
        int phoneType = cm.getFgPhone().getPhoneType();
        int bgPhoneType = cm.getBgPhone().getPhoneType();

        if (phoneType != bgPhoneType) {
            // Merging calls on different technologies is not supported
            return false;
        }

        if (phoneType != PhoneConstants.PHONE_TYPE_CDMA) {
            // GSM: "Merge" is available if both lines are in use and there's no
            // incoming call, *and* the current conference isn't already
            // "full".
            // TODO: shall move all okToMerge logic to CallManager
            return !cm.hasActiveRingingCall() && cm.hasActiveFgCall()
                    && cm.hasActiveBgCall()
                    && cm.canConference(cm.getFirstActiveBgCall());
        }
        return false;
    }

    /**
     * @return true if the UI should let you add a new call, given the current
     *         state of the Phone.
     */
    /* package */static boolean okToAddCall(CallManager cm) {
        if (!isCallOnImsEnabled()) {
            Phone phone = cm.getActiveFgCall().getPhone();

            // "Add call" is never allowed in emergency callback mode (ECM).
            if (isPhoneInEcm(phone)) {
                return false;
            }

            int phoneType = phone.getPhoneType();
            final Call.State fgCallState = cm.getActiveFgCall().getState();
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {

            } else if ((phoneType == PhoneConstants.PHONE_TYPE_GSM)
                    || (phoneType == PhoneConstants.PHONE_TYPE_SIP)) {
                // GSM: "Add call" is available only if ALL of the following are
                // true:
                // - There's no incoming ringing call
                // - There's < 2 lines in use
                // - The foreground call is ACTIVE or IDLE or DISCONNECTED.
                // (We mainly need to make sure it *isn't* DIALING or ALERTING.)
                final boolean hasRingingCall = cm.hasActiveRingingCall();
                final boolean hasActiveCall = cm.hasActiveFgCall();
                final boolean hasHoldingCall = cm.hasActiveBgCall();
                final boolean allLinesTaken = hasActiveCall && hasHoldingCall;

                return !hasRingingCall
                        && !allLinesTaken
                        && ((fgCallState == Call.State.ACTIVE)
                                || (fgCallState == Call.State.IDLE)
                                || (fgCallState == Call.State.DISCONNECTED));
            } else {
                throw new IllegalStateException("Unexpected phone type: "
                        + phoneType);
            }
        } else {
            return okToAddCallForIms(cm);
        }
        return false;
    }

    /* package */static boolean okToAddCallForIms(CallManager cm) {
        Phone phone = cm.getPhoneInCall();

        // "Add call" is never allowed in emergency callback mode (ECM).
        if (isPhoneInEcm(phone)) {
            return false;
        }

        // For IMS scenarios, add call should be allowed in any of the below
        // combination
        // Gsm+Lte, Cdma+Lte, Lte+Lte
        // Gsm+Gsm, Cdma+Cdma
        final Call.State fgCallState = cm.getActiveFgCall().getState();
        final boolean hasRingingCall = cm.hasActiveRingingCall();
        final boolean hasActiveCall = cm.hasActiveFgCall();
        final boolean hasHoldingCall = cm.hasActiveBgCall();
        final boolean allLinesTaken = hasActiveCall && hasHoldingCall;

        return !hasRingingCall
                && !allLinesTaken
                && ((fgCallState == Call.State.ACTIVE)
                        || (fgCallState == Call.State.IDLE) || (fgCallState == Call.State.DISCONNECTED));
    }

    /**
     * Check if a phone number can be route through a 3rd party gateway. The
     * number must be a global phone number in numerical form (1-800-666-SEXY
     * won't work).
     *
     * MMI codes and the like cannot be used as a dial number for the gateway
     * either.
     *
     * @param number
     *            To be dialed via a 3rd party gateway.
     * @return true If the number can be routed through the 3rd party network.
     */
    /* package */static boolean isRoutableViaGateway(String number) {
        if (TextUtils.isEmpty(number)) {
            return false;
        }
        number = PhoneNumberUtils.stripSeparators(number);
        if (!number.equals(PhoneNumberUtils
                .convertKeypadLettersToDigits(number))) {
            return false;
        }
        number = PhoneNumberUtils.extractNetworkPortion(number);
        return PhoneNumberUtils.isGlobalPhoneNumber(number);
    }

    /**
     * Returns whether the phone is in ECM ("Emergency Callback Mode") or not.
     */
    /* package */static boolean isPhoneInEcm(Phone phone) {
        if ((phone != null) && TelephonyCapabilities.supportsEcm(phone)) {
            // For phones that support ECM, return true iff PROPERTY_INECM_MODE
            // == "true".
            // TODO: There ought to be a better API for this than just
            // exposing a system property all the way up to the app layer,
            // probably a method like "inEcm()" provided by the telephony
            // layer.
            String ecmMode = SystemProperties
                    .get(TelephonyProperties.PROPERTY_INECM_MODE);
            if (ecmMode != null) {
                return ecmMode.equals("true");
            }
        }
        return false;
    }

    public static Phone getImsPhone(CallManager cm) {
        if (DBG) {
            log("Find IMS phone:");
        }
        for (Phone phone : cm.getAllPhones()) {
            log("getImsPhone phone.type=" + phone.getPhoneType() + ";phone="
                    + phone);
            if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
                log("found IMSPhone = " + phone + ", " + phone.getClass());
                return phone;
            }
        }
        if (DBG)
            log("IMS phone not present");
        return null;
    }

    /**
     * Returns true when the given call is in INCOMING state and there's no
     * foreground phone call, meaning the call is the first real incoming call
     * the phone is having.
     */
    public static boolean isRealIncomingCall(Call.State state) {
        return (state == Call.State.INCOMING && !PhoneGlobals.getInstance().mCM
                .hasActiveFgCallAnyPhone());
    }

    /**
     * Return true if this is a video call
     *
     * @param call
     * @return
     */
    public static boolean isImsVideoCall(Call call) {
        if (DBG)
            log("In isImsVideoCall call=" + call);
        Phone phone = call.getPhone();
        if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            try {
                int callType = phone.getCallType(call);
                if (callType == Phone.CALL_TYPE_VT
                        || callType == Phone.CALL_TYPE_VT_RX
                        || callType == Phone.CALL_TYPE_VT_TX) {
                    return true;
                }
            } catch (CallStateException ex) {
                Log.e(LOG_TAG, "isIMSVideoCall: caught " + ex, ex);
            }
        }
        return false;
    }

    /**
     * Return true if this is an active video call
     *
     * @param cm
     *            CallManager
     * @return
     */
    public static boolean isImsVideoCallActive(Call call) {
        if (call == null)
            return false;

        // Check if there is an active call
        if (call.getState() != Call.State.ACTIVE) {
            if (DBG)
                log("Call is not active");
            return false;
        }

        // Check if this is a video call
        return isImsVideoCall(call);
    }

    //
    // General phone and call state debugging/testing code
    //

    /* package */static void dumpCallState(Phone phone) {
        PhoneGlobals app = PhoneGlobals.getInstance();
        Log.d(LOG_TAG, "dumpCallState():");
        Log.d(LOG_TAG, "- Phone: " + phone + ", name = " + phone.getPhoneName()
                + ", state = " + phone.getState());

        StringBuilder b = new StringBuilder(128);

        Call call = phone.getForegroundCall();
        b.setLength(0);
        b.append("  - FG call: ").append(call.getState());
        b.append(" isAlive ").append(call.getState().isAlive());
        b.append(" isRinging ").append(call.getState().isRinging());
        b.append(" isDialing ").append(call.getState().isDialing());
        b.append(" isIdle ").append(call.isIdle());
        b.append(" hasConnections ").append(call.hasConnections());
        Log.d(LOG_TAG, b.toString());

        call = phone.getBackgroundCall();
        b.setLength(0);
        b.append("  - BG call: ").append(call.getState());
        b.append(" isAlive ").append(call.getState().isAlive());
        b.append(" isRinging ").append(call.getState().isRinging());
        b.append(" isDialing ").append(call.getState().isDialing());
        b.append(" isIdle ").append(call.isIdle());
        b.append(" hasConnections ").append(call.hasConnections());
        Log.d(LOG_TAG, b.toString());

        call = phone.getRingingCall();
        b.setLength(0);
        b.append("  - RINGING call: ").append(call.getState());
        b.append(" isAlive ").append(call.getState().isAlive());
        b.append(" isRinging ").append(call.getState().isRinging());
        b.append(" isDialing ").append(call.getState().isDialing());
        b.append(" isIdle ").append(call.isIdle());
        b.append(" hasConnections ").append(call.hasConnections());
        Log.d(LOG_TAG, b.toString());

        final boolean hasRingingCall = !phone.getRingingCall().isIdle();
        final boolean hasActiveCall = !phone.getForegroundCall().isIdle();
        final boolean hasHoldingCall = !phone.getBackgroundCall().isIdle();
        final boolean allLinesTaken = hasActiveCall && hasHoldingCall;
        b.setLength(0);
        b.append("  - hasRingingCall ").append(hasRingingCall);
        b.append(" hasActiveCall ").append(hasActiveCall);
        b.append(" hasHoldingCall ").append(hasHoldingCall);
        b.append(" allLinesTaken ").append(allLinesTaken);
        Log.d(LOG_TAG, b.toString());

    }

    /**
     * Returns true if Android supports VoLTE/VT calls on IMS
     */
    public static boolean isCallOnImsEnabled() {
        return CallManager.isCallOnImsEnabled();
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    static void dumpCallManager() {
        Call call;
        CallManager cm = PhoneGlobals.getInstance().mCM;
        StringBuilder b = new StringBuilder(128);

        Log.d(LOG_TAG, "############### dumpCallManager() ##############");
        // TODO: Don't log "cm" itself, since CallManager.toString()
        // already spews out almost all this same information.
        // We should fix CallManager.toString() to be more minimal, and
        // use an explicit dumpState() method for the verbose dump.
        // Log.d(LOG_TAG, "CallManager: " + cm
        // + ", state = " + cm.getState());
        Log.d(LOG_TAG, "CallManager: state = " + cm.getState());
        b.setLength(0);
        call = cm.getActiveFgCall();
        b.append(" - FG call: ").append(cm.hasActiveFgCall() ? "YES " : "NO ");
        b.append(call);
        b.append("  State: ").append(cm.getActiveFgCallState());
        b.append("  Conn: ").append(cm.getFgCallConnections());
        Log.d(LOG_TAG, b.toString());
        b.setLength(0);
        call = cm.getFirstActiveBgCall();
        b.append(" - BG call: ").append(cm.hasActiveBgCall() ? "YES " : "NO ");
        b.append(call);
        b.append("  State: ").append(cm.getFirstActiveBgCall().getState());
        b.append("  Conn: ").append(cm.getBgCallConnections());
        Log.d(LOG_TAG, b.toString());
        b.setLength(0);
        call = cm.getFirstActiveRingingCall();
        b.append(" - RINGING call: ").append(
                cm.hasActiveRingingCall() ? "YES " : "NO ");
        b.append(call);
        b.append("  State: ").append(cm.getFirstActiveRingingCall().getState());
        Log.d(LOG_TAG, b.toString());

        for (Phone phone : CallManager.getInstance().getAllPhones()) {
            if (phone != null) {
                Log.d(LOG_TAG,
                        "Phone: " + phone + ", name = " + phone.getPhoneName()
                                + ", state = " + phone.getState());
                b.setLength(0);
                call = phone.getForegroundCall();
                b.append(" - FG call: ").append(call);
                b.append("  State: ").append(call.getState());
                b.append("  Conn: ").append(call.hasConnections());
                Log.d(LOG_TAG, b.toString());
                b.setLength(0);
                call = phone.getBackgroundCall();
                b.append(" - BG call: ").append(call);
                b.append("  State: ").append(call.getState());
                b.append("  Conn: ").append(call.hasConnections());
                Log.d(LOG_TAG, b.toString());
                b.setLength(0);
                call = phone.getRingingCall();
                b.append(" - RINGING call: ").append(call);
                b.append("  State: ").append(call.getState());
                b.append("  Conn: ").append(call.hasConnections());
                Log.d(LOG_TAG, b.toString());
            }
        }

        Log.d(LOG_TAG, "############## END dumpCallManager() ###############");
    }

    public static String[] getExtrasFromMap(Map<String, String> newExtras) {
        String[] extras = null;

        if (newExtras == null) {
            return null;
        }

        // TODO: Merge new extras into extras. For now, just serialize and set
        // them
        extras = new String[newExtras.size()];

        if (extras != null) {
            int i = 0;
            for (Entry<String, String> entry : newExtras.entrySet()) {
                extras[i] = "" + entry.getKey() + "=" + entry.getValue();
            }
        }
        return extras;
    }

    // This method is called when user does which SUB from UI.
    public static void switchToLocalHold(int subscription, boolean switchTo) {
        Log.d(LOG_TAG, "Switch to local hold  = ");
        CallManager.getInstance().switchToLocalHold(subscription, switchTo);
    }

    public static boolean isDsdaEnabled() {
        boolean dsdaEnabled = MSimTelephonyManager.getDefault()
                .getMultiSimConfiguration() == MSimTelephonyManager.MultiSimVariants.DSDA;

        return dsdaEnabled;
    }

    public static void changeConnectionType(int callType, Message msg) {
        CallManager cm = PhoneGlobals.getInstance().mCM;
        final Connection conn = cm.getFgCallLatestConnection();
        Phone imsPhone = getImsPhone(cm);
        try {
            imsPhone.changeConnectionType(msg, conn, callType, null);
        } catch (Exception ex) {
            Log.e(LOG_TAG, "Ims Service changeConnectionType exception", ex);
        }
    }

    public static void endConference() {
        Call fgcall = PhoneGlobals.getInstance().mCM.getActiveFgCall();
        if (fgcall != null) {
            Connection connection = fgcall.getLatestConnection();
            PhoneUtils.hangup(connection);
        }
    }

    public static String[] getCallDetailsExtrasinCall(int callId, CallManager cm) {
        // Phone phone = getImsPhone(cm);
        // if (phone != null) {
        // Log.d(LOG_TAG, "getCallDetailsExtrasinCall");
        // try {
        // return phone.getCallDetailsExtrasinCall(callId);
        // } catch (CallStateException e) {
        // Log.e("PhoneUtils", "Exception in hangupWithReason" + e);
        // return null;
        // }
        // }
        return null;
    }

    public static String getImsDisconnectCauseInfo(int callId) {
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Phone imsPhone = getImsPhone(cm);
        // to-do
        return "";
    }

    public static String[] getUriListinConf() {
        // TODO Auto-generated method stub
        return null;
    }

    public static boolean isVTModifyAllowed() {
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Phone imsPhone = getImsPhone(cm);
        boolean ret = false;
        try {
            ret = imsPhone.isVTModifyAllowed();
        } catch (CallStateException e) {
            Log.e("PhoneUtils", "Exception in isVTModifyAllowed" + e);
        }
        return ret;
    }

    public static boolean getProposedConnectionFailed(int connIndex) {
        // TODO Auto-generated method stub
        return false;
    }

    public static boolean isAddParticipantAllowed() {
        // TODO Auto-generated method stub
        return false;
    }

    public static void addParticipant(String dialString, int clir,
            int callType, String[] extras) {
        CallManager cm = PhoneGlobals.getInstance().mCM;
        Phone phone = getImsPhone(cm);
        if (phone != null) {
            Log.d(LOG_TAG, "addParticipant");
            try {
                phone.addParticipant(dialString, clir, callType, extras);
            } catch (CallStateException e) {
                Log.e("PhoneUtils", "Exception in addParticipant" + e);
            }
        }

    }

    public static boolean isImsPhoneIdle() {
        // TODO Auto-generated method stub
        return false;
    }

}
