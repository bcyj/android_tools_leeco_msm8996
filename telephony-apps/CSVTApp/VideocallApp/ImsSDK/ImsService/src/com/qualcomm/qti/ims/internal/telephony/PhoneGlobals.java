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
 * Not a Contribution.
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

import android.app.Activity;
import android.app.Dialog;
import android.app.KeyguardManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.IPowerManager;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UpdateLock;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.provider.Settings.System;
import android.telephony.MSimTelephonyManager;

import android.telephony.ServiceState;
import android.text.TextUtils;
import android.util.Log;
import android.util.Slog;
import android.view.KeyEvent;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.MmiCode;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyCapabilities;
import com.android.internal.telephony.TelephonyIntents;
import org.codeaurora.ims.IImsService;
import static com.android.internal.telephony.MSimConstants.DEFAULT_SUBSCRIPTION;

import java.util.List;

/**
 * Global state for the telephony subsystem when running in the primary phone
 * process.
 */
public class PhoneGlobals extends ContextWrapper {
    /* package */static final String LOG_TAG = "imsPhoneGlobals";

    // Message codes; see mHandler below.
    private static final int EVENT_CALL_STATE_CHANGED = 3;
    private static final int EVENT_MODIFY_CALL = 4;
    private static final int PHONE_NEW_RINGING_CONNECTION = 5;
    private static final int PHONE_DISCONNECT = 6;
    private static final int PHONE_UNKNOWN_CONNECTION_APPEARED = 7;
    private static final int PHONE_INCOMING_RING = 8;
    private static final int PHONE_STATE_DISPLAYINFO = 9;
    private static final int PHONE_RINGBACK_TONE = 10;
    private static final int PHONE_RESEND_MUTE = 11;
    private static final int PHONE_MODIFY_CALL_DONE = 12;

    protected static PhoneGlobals sMe;

    Ringer ringer;
    CallManager mCM;
    static boolean sVoiceCapable = true;

    public IImsService mImsService;

    private Connection mModifyedConnection;

    // Last phone state seen by updatePhoneState()
    protected PhoneConstants.State mLastPhoneState = PhoneConstants.State.IDLE;

    private IImsNotifier mNotifier;

    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            PhoneConstants.State phoneState;
            switch (msg.what) {
            case EVENT_CALL_STATE_CHANGED:
                onPhoneStateChanged((AsyncResult) msg.obj);
                break;
            case PHONE_DISCONNECT:
                onDisconnect((AsyncResult) msg.obj);
                break;
            case PHONE_NEW_RINGING_CONNECTION:
                handleNewRingingConnection((AsyncResult) msg.obj);
                break;
            case PHONE_UNKNOWN_CONNECTION_APPEARED:
                onUnknownConnectionAppeared((AsyncResult) msg.obj);
                break;
            case PHONE_RINGBACK_TONE:
                break;
            case PHONE_RESEND_MUTE:
                onResendMute();
                break;
            case EVENT_MODIFY_CALL:
                onModifyCall((AsyncResult) msg.obj);
                break;

            case PHONE_MODIFY_CALL_DONE:
                onModifyCallDone((AsyncResult) msg.obj);
            }
        }

        private void onModifyCallDone(AsyncResult r) {
            Throwable ex = r.exception;
            boolean result = ex == null;
            String errorStr = "";
            if (ex != null) {
                errorStr = ex.getMessage();
                Log.e(LOG_TAG, "videocall modify call request failed "
                        + errorStr);

            } else {
                Log.d(LOG_TAG,
                        "IMS Modify call request to RIL returned without exception");
            }
            mNotifier.notifyImsModifyCallDone(result, errorStr);
        }
    };

    private boolean mCallNotifiedRegisterd = false;

    public void setImsService(IImsService service) {
        mImsService = service;
    }

    protected void onDisconnect(AsyncResult r) {
        Connection c = (Connection) r.result;
        final Phone phone = c.getCall().getPhone();
        if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            Connection.DisconnectCause cause = c.getDisconnectCause();
            int result = cause.ordinal();
            mNotifier.notifyImsDisconnect(result);

            try {
                phone.unregisterForModifyCallRequest(mHandler);
                mCallNotifiedRegisterd = false;
            } catch (Exception e) {
                Log.e(LOG_TAG, e.getMessage());
            }

        }
    }

    protected void onModifyCall(AsyncResult r) {
        mModifyedConnection = (Connection) r.result;
        final Phone phone = mModifyedConnection.getCall().getPhone();

        if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
            try {
                int callType = phone.getProposedConnectionType(mModifyedConnection);
                // boolean error =
                // phone.getProposedConnectionFailed(conn.getIndex());
                // if (!error) {
                mNotifier.notifyImsRemoteModifyCallRequest(callType);
                // }
            } catch (CallStateException e) {
                Log.e(LOG_TAG, "onModifyCall exception=" + e.getMessage());
            }
        }
    }

    private void handleNewRingingConnection(AsyncResult r) {

        Connection c = (Connection) r.result;
        Log.d(LOG_TAG, "onNewRingingConnection(): state = " + mCM.getState()
                + ", conn = { " + c + " }");
        Call ringing = c.getCall();
        Phone phone = ringing.getPhone();
        if ((phone == null)
                || (phone.getPhoneType() != PhoneConstants.PHONE_TYPE_IMS)) {
            Log.d(LOG_TAG, "not imsNewRingingConnect just return");
            return;
        }

        if (!c.isRinging()) {
            Log.d(LOG_TAG,
                    "CallNotifier.onNewRingingConnection(): connection not ringing!");
            return;
        }
        String address = c.getAddress();

        try {
            int callType = phone.getCallType(ringing);
            mNotifier.notifyNewRingingConnection(address, callType);
            phone.registerForModifyCallRequest(mHandler, EVENT_MODIFY_CALL,
                    null);
            mCallNotifiedRegisterd = true;
        } catch (Exception e) {
            Log.e(LOG_TAG, "getCallType e=" + e.getMessage());
        }
    }

    private void onUnknownConnectionAppeared(AsyncResult r) {
        PhoneConstants.State state = mCM.getState();

        if (state == PhoneConstants.State.OFFHOOK) {
            // basically do onPhoneStateChanged + display the incoming call UI
            onPhoneStateChanged(r);

        }
    }

    public void onCreate() {
        Log.v(LOG_TAG, "onCreate()...");

        // Start TelephonyDebugService After the default phone is created.

        mCM = CallManager.getInstance();

        // Make sure the audio mode (along with some
        // audio-mode-related state of our own) is initialized
        // correctly, given the current state of the phone.
        PhoneUtils.setAudioMode(mCM);

        if (PhoneUtils.isCallOnImsEnabled()) {
            registerForNotifications();
        }
    }

    public void setIImsNotifier(IImsNotifier notifier) {
        mNotifier = notifier;
    }

    private void onPhoneStateChanged(AsyncResult r) {
        PhoneConstants.State state = mCM.getState();
        Log.d(LOG_TAG, "onPhoneStateChanged: state = " + state);
        mLastPhoneState = state;

        Phone fgPhone = mCM.getFgPhone();
        if (fgPhone.getPhoneType() != PhoneConstants.PHONE_TYPE_IMS) {
            Log.d(LOG_TAG, "onPhoneStateChanged phonetype not ims");
            return;
        }
        if (state == PhoneConstants.State.OFFHOOK) {
            Log.d(LOG_TAG, "onPhoneStateChanged: OFF HOOK");
            // make sure audio is in in-call mode now
            PhoneUtils.setAudioMode(mCM);
            Call call = mCM.getActiveFgCall();
            if ((call != null) && (PhoneUtils.isImsVideoCall(call))) {
                if (!PhoneUtils.isSpeakerOn(this)){
                    PhoneUtils.turnOnSpeaker(this, true, true);
                }
            }

            if (mCallNotifiedRegisterd == false) {
                try {
                    fgPhone.registerForModifyCallRequest(mHandler,
                            EVENT_MODIFY_CALL, null);
                    mCallNotifiedRegisterd = true;
                } catch (Exception e) {
                    Log.e(LOG_TAG, "registerForModifyCallRequestFail");
                }
            }
            Log.d(LOG_TAG, "stopRing()... (OFFHOOK state)");
        }
        mNotifier.notifyImsPhoneStateChanged(state.ordinal());
    }

    protected void registerForNotifications() {

        mCM.registerForNewRingingConnection(mHandler,
                PHONE_NEW_RINGING_CONNECTION, null);
        mCM.registerForPreciseCallStateChanged(mHandler,
                EVENT_CALL_STATE_CHANGED, null);
        mCM.registerForDisconnect(mHandler, PHONE_DISCONNECT, null);
        mCM.registerForUnknownConnection(mHandler,
                PHONE_UNKNOWN_CONNECTION_APPEARED, null);
        mCM.registerForIncomingRing(mHandler, PHONE_INCOMING_RING, null);
        mCM.registerForRingbackTone(mHandler, PHONE_RINGBACK_TONE, null);
        mCM.registerForResendIncallMute(mHandler, PHONE_RESEND_MUTE, null);

        /*
         * imsPhone.registerForPreciseCallStateChanged(mHandler,
         * EVENT_CALL_STATE_CHANGED, null);
         * imsPhone.registerForDisconnect(mHandler, PHONE_DISCONNECT, null);
         * imsPhone.registerForNewRingingConnection(mHandler,
         * PHONE_NEW_RINGING_CONNECTION, null);
         * imsPhone.registerForUnknownConnection(mHandler,
         * PHONE_UNKNOWN_CONNECTION_APPEARED, null);
         * imsPhone.registerForIncomingRing(mHandler, PHONE_INCOMING_RING,
         * null); imsPhone.registerForRingbackTone(mHandler,
         * PHONE_RINGBACK_TONE, null);
         * imsPhone.registerForResendIncallMute(mHandler, PHONE_RESEND_MUTE,
         * null);
         */
        // try {
        // Phone imsPhone = PhoneUtils.getImsPhone(mCM);
        // imsPhone.registerForModifyCallRequest(mHandler, EVENT_MODIFY_CALL,
        // null);
        // } catch (Exception e) {
        // Log.e(LOG_TAG, e.getMessage());
        // }
    }

    public PhoneGlobals(Context context) {
        super(context);
        sMe = this;
        Log.d(LOG_TAG, "assign sMe to this=" + this);
    }

    /**
     * Returns the singleton instance of the PhoneApp.
     */
    static PhoneGlobals getInstance() {
        if (sMe == null) {
            throw new IllegalStateException("No PhoneGlobals here!");
        }
        return sMe;
    }

    private void handleServiceStateChanged(Intent intent) {
        /**
         * This used to handle updating EriTextWidgetProvider this routine and
         * and listening for ACTION_SERVICE_STATE_CHANGED intents could be
         * removed. But leaving just in case it might be needed in the near
         * future.
         */

        // If service just returned, start sending out the queued messages
        ServiceState ss = ServiceState.newFromBundle(intent.getExtras());
    }

    private void onResendMute() {
        boolean muteState = PhoneUtils.getMute();
        PhoneUtils.setMute(!muteState);
    }

	public void acceptConnectionType(boolean accept) {
		Log.d(LOG_TAG, "acceptConnectionType accept = " + accept);
		if (mModifyedConnection == null){
			mModifyedConnection = mCM.getFgCallLatestConnection();
		}
		Phone phone = mModifyedConnection.getCall().getPhone();
		if (phone != null
				&& phone.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS) {
			Log.d(LOG_TAG, "modifyCallConfirm");
			try {
				if (accept) {
					phone.acceptConnectionTypeChange(mModifyedConnection, null);
				} else {
					phone.rejectConnectionTypeChange(mModifyedConnection);
				}
			} catch (CallStateException e) {
				Log.e(LOG_TAG, "Exception in modifyCallConfirm" + e);
			}
		}

	}

    public void changeConnectionType(int callType) {
        Message msg = mHandler.obtainMessage(PHONE_MODIFY_CALL_DONE);
        PhoneUtils.changeConnectionType(callType, msg);
    }

}
