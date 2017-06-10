/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may21 obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */




package org.codeaurora.btmultisim;

import android.app.Service;
import android.content.Intent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.IBinder;
import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;


import com.android.internal.telephony.Call;
import android.telecom.CallState;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

/**
 * Bluetooth headset manager for the DSDA call state changes.
 * @hide
 */
public class BluetoothDsdaService extends Service {
    private static final String TAG = "BluetoothDsdaService";
    private static final boolean DBG = true;
    private static final boolean VDBG = false;

    private static final String MODIFY_PHONE_STATE = android.Manifest.permission.MODIFY_PHONE_STATE;

    private BluetoothAdapter mAdapter;
    private TelecomManager mTelecomManager = null;
    private BluetoothHeadset mBluetoothHeadset;

   //At any time we can update only one active and one held
    private int mNumActive = 0;
    private int mNumHeld = 0;
    private int mDsdaCallState = CALL_STATE_IDLE;
    private boolean mFakeMultiParty = false;
    private BluetoothSub[] mSubscriptions;
    private boolean mCallSwitch = false;

    /* At present the PhoneIDs are valued as 0 and 1 for DSDA*/
    private static final int PHONE_ID1 = 0;
    private static final int PHONE_ID2 = 1;
    private static final int MAX_SUBS = 2;
    private static final int INVALID_SUBID = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    //Stores the current SUB on which call state changed happened.
    private int mCurrentSub = INVALID_SUBID;

    // match up with bthf_call_state_t of bt_hf.h
    final static int CALL_STATE_ACTIVE = 0;
    final static int CALL_STATE_HELD = 1;
    final static int CALL_STATE_DIALING = 2;
    final static int CALL_STATE_ALERTING = 3;
    final static int CALL_STATE_INCOMING = 4;
    final static int CALL_STATE_WAITING = 5;
    final static int CALL_STATE_IDLE = 6;
    final static int CHLD_TYPE_RELEASEHELD = 0;
    final static int CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD = 1;
    final static int CHLD_TYPE_HOLDACTIVE_ACCEPTHELD = 2;
    final static int CHLD_TYPE_ADDHELDTOCONF = 3;

    private boolean mIsThreewayCallOriginated = false;
    @Override
    public void onCreate() {
        Log.d(TAG, "BluetoothDsdaStateService created");
        super.onCreate();
        mNumActive = 0;
        mNumHeld = 0;
        mFakeMultiParty = false;
        Context context = this;
        mTelecomManager = (TelecomManager)
                context.getSystemService(context.TELECOM_SERVICE);
        if (mTelecomManager == null) {
            if (VDBG) Log.d(TAG, "mTelecomManager null");
            return;
        }
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mAdapter == null) {
            if (VDBG) Log.d(TAG, "mAdapter null");
            return;
        }
        mSubscriptions = new BluetoothSub[MAX_SUBS];
        for (int i = 0; i < MAX_SUBS; i++) {
            mSubscriptions[i] = new BluetoothSub(i);
        }
        //Get the HeadsetService Profile proxy
        mAdapter.getProfileProxy(this, mProfileListener, BluetoothProfile.HEADSET);
    }

    @Override
    public void onStart(Intent intent, int startId) {
        if (mAdapter == null) {
            Log.w(TAG, "Stopping Bluetooth BluetoothPhoneService Service: device does not have BT");
            stopSelf();
        }
        if (VDBG) Log.d(TAG, "BluetoothDsdaState started");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (DBG) log("Stopping Bluetooth Dsda Service");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private static final int DSDA_CALL_STATE_CHANGED = 1;
    private static final int DSDA_PHONE_SUB_CHANGED = 2;
    private static final int DSDA_SWITCH_SUB = 3;
    private static final int DSDA_QUERY_PHONE_STATE_CHANGED = 4;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (VDBG) Log.d(TAG, "handleMessage: " + msg.what);
            switch(msg.what) {
                case DSDA_CALL_STATE_CHANGED:
                    MultisimCallState multiSimCallstate = (MultisimCallState) msg.obj;
                    handleDsdaPreciseCallStateChange(multiSimCallstate);
                    break;
                case DSDA_PHONE_SUB_CHANGED:
                    handlePhoneSubChanged();
                    break;
                case DSDA_SWITCH_SUB:
                    switchSub();
                    break;
                case DSDA_QUERY_PHONE_STATE_CHANGED:
                    queryPhoneStateChanged();
                    break;
                default:
                    Log.d(TAG, "Unknown event : ");
                    break;
            }
        }
    };

    /* Handles call state changes on each subscription. */
    public void handleDsdaPreciseCallStateChange(MultisimCallState multiSimCallstate) {
        Log.d(TAG, "handleDSDAPreciseCallStateChange");
        //Handle call state changes of both subs separately
        int SubId = getCurrentSub();
        int PhoneId = SubscriptionManager.getPhoneId(SubId);
        Log.d(TAG, "Call change of SUB: " + SubId + " Phone Id : " + PhoneId);
        mSubscriptions[PhoneId].mSubscription = SubId;

        mSubscriptions[PhoneId].handleSubscriptionCallStateChange(
                multiSimCallstate.mForegroundCallState, multiSimCallstate.mRingingCallState,
                multiSimCallstate.mRingingNumber, multiSimCallstate.mRingNumberType,
                multiSimCallstate.mBackgroundCallState, multiSimCallstate.mNumHeldCallsonSub);
    }

    /* Called to notify the Subscription change event from telephony.*/
    private void handlePhoneSubChanged() {
        /*Could be used to notify switch SUB to headsets*/
        int sub = mTelecomManager.getActiveSubscription();
        if (INVALID_SUBID == sub) {
            Log.e(TAG, "Invalid sub id, returning");
            return;
        }
        Log.d(TAG, "Phone SUB changed, Active: " + sub);
        if (isSwitchSubAllowed() == true) {
            Log.d(TAG, "Update headset about switch sub");
            if (mBluetoothHeadset != null) {
                mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                    CALL_STATE_IDLE, null, 0);
            }
        }
    }

    /* Do the SwithSuB. */
    private void switchSub() {
        log("SwitchSub");
        int activeSub = mTelecomManager.getActiveSubscription();
        int otherSub = getOtherActiveSub(activeSub);
        if (INVALID_SUBID == otherSub) {
            Log.e(TAG, "Invalid activeSub id, returning");
            return;
        }
        switchToOtherActiveSub(otherSub);
    }

    private void switchToOtherActiveSub(int sub) {
        mTelecomManager.switchToOtherActiveSub(sub);
    }

    /* Called to query the Phone change event from Bluetooth.*/
    private void queryPhoneStateChanged() {
        int PhoneId = 0;
        int sub = mTelecomManager.getActiveSubscription();
        if (INVALID_SUBID == sub) {
            Log.e(TAG, "Invalid sub id");
        } else {
            PhoneId = SubscriptionManager.getPhoneId(sub);
        }
        log("Query on Active Sub  = " + sub + "PhoneId: " + PhoneId);
        if (mBluetoothHeadset != null) {
            mBluetoothHeadset.phoneStateChanged(mSubscriptions[PhoneId].mActive,
                mSubscriptions[PhoneId].mHeld, mDsdaCallState,
                mSubscriptions[PhoneId].mRingNumber,
                mSubscriptions[PhoneId].mRingNumberType);
        }
    }

    //This will also register for getting the Bluetooth Headset Profile proxy
    private BluetoothProfile.ServiceListener mProfileListener =
            new BluetoothProfile.ServiceListener() {
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            Log.d(TAG, "Got the headset proxy for DSDA" );
            mBluetoothHeadset = (BluetoothHeadset) proxy;
        }
        public void onServiceDisconnected(int profile) {
            mBluetoothHeadset = null;
            Log.d(TAG, "Released the headset proxy for DSDA");
        }
    };

    /* get the current SUB*/
    private int getCurrentSub() {
        return mCurrentSub;
    }

    /* SUB switch is allowed only when each sub has max
    of one call, either active or held*/
    public boolean isSwitchSubAllowed() {
        boolean allowed = false;
        log("isSwitchSubAllowed");
        log("mActive(PHONE_ID1) " + mSubscriptions[PHONE_ID1].mActive + " mHeld(PHONE_ID1) " +
                mSubscriptions[PHONE_ID1].mHeld + " mCallState(PHONE_ID1) " +
                mSubscriptions[PHONE_ID1].mCallState);
        log("mActive(PHONE_ID2) " + mSubscriptions[PHONE_ID2].mActive + " mHeld(PHONE_ID2) " +
                mSubscriptions[PHONE_ID2].mHeld + " mCallState(PHONE_ID2) " +
                mSubscriptions[PHONE_ID2].mCallState);

        if ((((mSubscriptions[PHONE_ID1].mActive + mSubscriptions[PHONE_ID1].mHeld) == 1)
                && (mSubscriptions[PHONE_ID1].mCallState == CALL_STATE_IDLE))
                && (((mSubscriptions[PHONE_ID2].mActive +
                mSubscriptions[PHONE_ID2].mHeld) == 1)
                && (mSubscriptions[PHONE_ID2].mCallState == CALL_STATE_IDLE))) {
            allowed = true;
        }
        log("Is switch SUB allowed: " + allowed);
        return allowed;
    }


    /**
    * Check whether any other sub is in active state other than
    * provided subscription, if yes return the other active sub.
    * @return subscription which is active, if no other sub is in
    * active state return INVALID_SUBID.
    */
    private int getOtherActiveSub(int subscription) {
        log("getOtherActiveSub, sub =  " + subscription);
        int otherSub = INVALID_SUBID;
        String subId = Integer.toString(subscription);

        for (PhoneAccountHandle ph : mTelecomManager.getCallCapablePhoneAccounts()) {
            if (subId != null && !subId.equals(ph.getId())) {
                otherSub = Integer.parseInt(ph.getId());
            }
        }
        log("getOtherActiveSub: otherSub =  " + otherSub);
        return otherSub;
    }

    /* Called to notify the Subscription change event from telephony.*/
    private class BluetoothSub {
        private int mForegroundCallState;
        private int mRingingCallState;
        private String mRingNumber;
        private int mRingNumberType;
        private int mSubscription;
        private int mPhoneId;
        private int mActive;
        private int mHeld;
        private int mCallState;
        private int mPhonetype;
        private long mBgndEarliestConnectionTime = 0;

        private BluetoothSub(int PhoneId) {
            Log.d(TAG, "Creating Bluetooth SUB for Phone Id " + PhoneId);
            mForegroundCallState = CallState.NEW;
            mRingingCallState = CallState.NEW;
            mRingNumber = "";
            mRingNumberType = 0;
            mSubscription = INVALID_SUBID;
            mPhoneId = PhoneId;
            mActive = 0;
            mHeld = 0;
            mCallState = CALL_STATE_IDLE;

            Log.d(TAG, "Bluetooth SUB for PhoneId: " + PhoneId);
        }
        /* Handles the single subscription call state changes.*/
        private void handleSubscriptionCallStateChange(int foregroundCallState,
                int ringingCallState, String ringingNumber, int ringNumberType,
                int backgroundCallState, int numHeldCallsonSub) {
            // get foreground call state
            int oldNumActive = mActive;
            int oldNumHeld = mHeld;
            int oldRingingCallState = mRingingCallState;
            int oldForegroundCallState = mForegroundCallState;
            String oldRingNumber = mRingNumber;

            Log.d(TAG, "[Previous] mActive: " + mActive + " mHeld: " + mHeld);

            mForegroundCallState = foregroundCallState;
            /* if in transition, do not update */
            if (mForegroundCallState == CallState.DISCONNECTING) {
                Log.d(TAG, "Call disconnecting,wait before update");
                return;
            } else {
                mActive = (mForegroundCallState == CallState.ACTIVE) ? 1 : 0;
            }

            mRingingCallState = ringingCallState;
            mRingNumber = ringingNumber;
            mRingNumberType = ringNumberType;

            Log.d(TAG, "Number of held calls on this SUB = " + numHeldCallsonSub);
            mHeld = (numHeldCallsonSub == 2) ? 1 : numHeldCallsonSub;

            Log.d(TAG, "[New] active:= " + mActive + " held:= " + mHeld
                    + " foregroundCallState: " + foregroundCallState
                    + "ringingCallState: " + ringingCallState);

            boolean sendDialingFirst = mCallState !=
                    getBluetoothCallStateForUpdate(mRingingCallState, mForegroundCallState) &&
                    getBluetoothCallStateForUpdate(mRingingCallState, mForegroundCallState) ==
                    CALL_STATE_ALERTING;
            mCallState = getBluetoothCallStateForUpdate(mRingingCallState, mForegroundCallState);

            boolean callsSwitched = false;
            callsSwitched = (numHeldCallsonSub == 2);

            Log.d(TAG, "Call switch value: " + callsSwitched);

            if ((mActive != oldNumActive || mHeld != oldNumHeld ||
                    mRingingCallState != oldRingingCallState ||
                    mForegroundCallState != oldForegroundCallState ||
                    !mRingNumber.equals(oldRingNumber)) && !callsSwitched) {
                Log.d(TAG, "Call handleSendcallStates for Sub: " + mSubscription +
                        " to update headset");
                if (sendDialingFirst) {
                    handleSendcallStates(mSubscription, mActive, mHeld,
                            CALL_STATE_DIALING, mRingNumber, mRingNumberType);
                    sendDialingFirst = false;
                }
                handleSendcallStates(mSubscription, mActive, mHeld,
                        mCallState, mRingNumber, mRingNumberType);
            }
            mCallSwitch = callsSwitched;
        }

        private int getActive() {
            return mActive;
        }

        private int getHeld() {
            return mHeld;
        }

    } /* BluetoothSub Class*/


    // DSDA state machine which takes care of sending indicators
    private void handleSendcallStates(int SUB, int numActive, int numHeld,
            int callState, String number,int type) {
        //SUB will give info that for which SUB these changes have to be updated
        //Get the states of other SUB..
        int otherSubActive;
        int otherSubHeld;
        int otherSubCallState;
        int otherPhoneId;
        int PhoneId = SubscriptionManager.getPhoneId(SUB);
        Log.d(TAG, "mNumActive: " + mNumActive + " mNumHeld: " + mNumHeld);
        Log.d(TAG, "numActive: " + numActive + " numHeld: " + numHeld + " Callstate: " +
                callState + " Number: " + number + " type: " + type);

        otherPhoneId = (PhoneId == PHONE_ID2) ? PHONE_ID1: PHONE_ID2;
        otherSubActive = mSubscriptions[otherPhoneId].getActive();
        otherSubHeld = mSubscriptions[otherPhoneId].getHeld();
        Log.d(TAG, "Other Sub call states-> Active: " +
                otherSubActive + " Held: " + otherSubHeld);

        if ((mNumActive + mNumHeld) == 2) {
            //Meaning, we are already in a state of max calls
            //Check the current call state.Already sent 4,1
            switch(callState){
                case CALL_STATE_INCOMING:
                    //This makes sure that the
                    // current SUB is not running in max calls
                    if ((numActive + numHeld) < 2) {
                        //Fake Indicator first about call join (callheld =0)
                        mNumHeld = 0;
                        mFakeMultiParty = true;
                        if (mBluetoothHeadset != null) {
                            mBluetoothHeadset.phoneStateChanged(1, 0,
                                CALL_STATE_IDLE, null, 0);
                            //Send new incoming call notification
                            mBluetoothHeadset.phoneStateChanged(1, 0,
                                CALL_STATE_INCOMING, number, type);
                        }
                    } else if ((numActive + numHeld) == 2) {
                        //Notify the same .HS may reject this call
                        //If this call is accepted, we fall in a case where
                        // telephony will drop one of the current call
                        if (mBluetoothHeadset != null)
                            mBluetoothHeadset.phoneStateChanged(1, 1,
                                CALL_STATE_INCOMING, number, type);
                    }
                    break;

                case CALL_STATE_IDLE:
                    //Could come when calls are being dropped/hanged OR
                    //This state could be reached when HF tried to receive the
                    // third call and telephony would have dropped held call on
                    // currnt SUB..OR. HS just rejected the call
                    //TODO
                    //This state is also seen in call switch on same sub
                    if ((numActive + numHeld + otherSubActive + otherSubHeld) >= 2) {
                        // greater than 2 means we have atleast one active one held
                        //no need to update the headset on this
                        //Add log to see which call is dropped
                        if (((numActive + numHeld) == 2) &&
                            (mCallSwitch == true)) {
                            log("Call switch happened on this SUB");
                            if (mBluetoothHeadset != null) {
                                log("update hs");
                                mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                                    callState, number, type);
                            } else Log.d(TAG, "No need as headset is null");
                        } else if ((otherSubActive + otherSubHeld) >= 1) {
                            log("No update is needed");
                        } else if ((numActive + numHeld) == 1) {
                            log("Call position changed on this sub having single call");
                            //We dont get callSwitch true when call comes frm
                            // held to active
                            if (mBluetoothHeadset != null) {
                                log("update hs for this switch");
                                mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                                                          callState,number, type);
                            } else Log.d(TAG, "No need as headset is null");
                        } else if (mDsdaCallState != CALL_STATE_IDLE) {
                            log("New call setup failed");
                            if (mBluetoothHeadset != null) {
                                mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                                    callState,number, type);
                            } else Log.d(TAG, "Headset is null");
                        } else log("No need to update this call drop");
                    } else {
                         log("Some call may have dropped in current Sub");
                        //Send proper updates
                        // All calls on this SUB are ended now.
                        if (numActive + numHeld == 0) {
                            if (otherSubActive == 1)
                                mNumActive = 1;
                            else mNumActive = 0;
                            if (otherSubHeld == 1)
                                mNumHeld = 1;
                            else mNumHeld = 0;
                        } else {
                             log("the current SUB has more than one call,update");
                            //The current SUB has more than one call
                            //just update the same.
                            mNumActive = numActive;
                            mNumHeld = numHeld;
                        }
                        if (mBluetoothHeadset != null)
                            mBluetoothHeadset.phoneStateChanged(mNumActive,
                                              mNumHeld,CALL_STATE_IDLE,number, type);
                    }
                    break;

                case CALL_STATE_DIALING:
                    //Check if we can honor the dial from this SUB
                    if ((numActive + numHeld) < 2) {
                        // we would have sent 4,1 already before this
                        // It is slight different again compared to incoming call
                        // scenario. Need to check if even in Single SIM, if
                        //dial is allowed when we already have two calls.
                        //In this case we can send 4,0 as it is valid on this sub
                        //Very less chance to have a headset doing this, but if the
                        //user explicitly tries to dial, we may end up here.
                        //Even is Single SIM , this scenario is not well known and
                        if (mBluetoothHeadset != null) {
                            log("call dial,Call join first");
                            mFakeMultiParty = true;
                            if (mBluetoothHeadset != null) {
                                mBluetoothHeadset.phoneStateChanged(1, 0,
                                                  CALL_STATE_IDLE, null, 0);
                                log("call dial,Send dial with held call");
                                mBluetoothHeadset.phoneStateChanged(0, 1,
                                                  callState, number, type);
                            }
                        }
                        mNumActive = 0;
                        mNumHeld = 1;
                    } else if (numActive + numHeld == 2) {
                        // Tossed up case.
                    }
                    break;

                case CALL_STATE_ALERTING:
                    //numHeld may be 1 here
                    if ((numActive + numHeld) < 2) {
                        if (mBluetoothHeadset != null) {
                            //Just update the call state
                            log("update hs with call state");
                            mBluetoothHeadset.phoneStateChanged(0, 1,
                                    callState, number, type);
                        } else {
                            Log.d(TAG, "Headset is null");
                        }
                    }
                    break;

                default:
                    break;
            }
        } else if ((mNumActive == 1) || (mNumHeld == 1)) {
            //We have atleast one call.It could be active or held
            //just notify about the incoming call.
            switch(callState){
                case CALL_STATE_INCOMING:
                    //No change now, just send the new call states
                    Log.d(TAG,"Incoming call while we have active or held already present");
                    Log.d(TAG, " just update the new incoming call state");
                    //Update the active call and held call only when they are zero
                    if ((numActive + otherSubActive) == 0) {
                        Log.d(TAG,"No active call now on SUB: " + SUB);
                        mNumActive = 0;
                    } else {
                        mNumActive = 1;
                    }
                    if ((numHeld + otherSubHeld) == 0) {
                        Log.d(TAG,"No held call now on SUB: " + SUB);
                        mNumHeld = 0;
                    } else {
                        mNumHeld = 1;
                    }

                    if (mBluetoothHeadset != null) {
                        mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                        callState,number, type);
                    } else Log.d(TAG, "No need as headset is null");
                    break;

                case CALL_STATE_DIALING:
                    //We should see that active call is put on hold
                    //Since, we alread have one call, make sure q
                    //we are getting dial in already having call
                    if ((numActive == 0) && (numHeld == 1)) {
                        Log.d(TAG, "we are getting dial in already having call");
                        mNumActive = numActive;
                        mNumHeld = numHeld;
                    }
                    if(((otherSubActive == 1) || (otherSubHeld == 1)) &&
                        (numActive == 0)) {
                        log("This is new dial on this sub when a call is active on other sub");
                        //Make sure we send held=1 and active = 0
                        //Before dialing, the active call becomes held.
                        //In DSDA we have to fake it
                        mNumActive = 0;
                        mNumHeld = 1;
                    }
                    //Send the update
                    if (mBluetoothHeadset != null) {
                        mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                        callState,number, type);
                    } else Log.d(TAG, "No need as headset is null here");

                    break;

                case CALL_STATE_ALERTING:
                    //Just send update
                    Log.d(TAG, "Just send update for ALERT");
                    if (mBluetoothHeadset != null) {
                        mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                        callState,number, type);
                    } else Log.d(TAG, "No need as headset is null");

                    break;

                case CALL_STATE_IDLE:
                    //Call could be dropped by now.
                    //Here we have to decide if we need to update held call for
                    // switch SUB etc. Idle could be call accept or reject.Act
                    // properly
                    //Update the active call and held call only when they are zero
                    if (mNumActive == 0) {
                        if ((numActive == 1) || (otherSubActive == 1)) {
                            mNumActive = 1;
                            Log.d(TAG,"New active call on SUB: " + SUB);
                        }
                    } else if (mNumActive == 1) { /* Call dropped, update mNumActive properly*/
                        if(numActive == 0) {
                            Log.d(TAG,"Active Call state changed to 0: " + SUB);
                            if(otherSubActive == 0)
                                mNumActive = numActive;
                        }
                    }
                    if (mNumHeld == 1) {
                        //Update the values properly
                        log("Update the values properly");
                        if ((numActive + numHeld + otherSubActive +
                             otherSubHeld) < 2)
                            mNumHeld = ((numHeld + otherSubHeld) > 0)? 1:0;
                    } else {
                        //There is no held call
                        log("There was no held call previously");
                        if (((otherSubActive == 1) || (otherSubHeld == 1)) &&
                        ((numActive == 1) || (numHeld == 1))) {
                            // Switch SUB happened
                            //This will come for single sub case of 1 active, 1
                            // new call
                            Log.d(TAG,"Switch SUB happened, fake callheld");
                            mNumHeld = 1; // Fake 1 active , 1held, TRICKY
                        } else if (mNumHeld == 0) {
                            Log.d(TAG,"Update Held as value on this sub: " + numHeld);
                            mNumHeld = numHeld;
                        }
                    }
                    //This could be tricky as we may move suddenly from 4,0 t0 4,1
                    // even when  the new call was rejected.
                    if (mBluetoothHeadset != null) {
                        Log.d(TAG, "updating headset");
                        mBluetoothHeadset.phoneStateChanged(mNumActive,mNumHeld,
                                          callState,number, type);
                    } else Log.d(TAG, "No need as headset is null");

                    break;
            }
        } else{
            //This is first of the calls, update properly
            Log.d(TAG, "This is first of the calls, update properly");
            mNumActive = numActive;
            mNumHeld = numHeld;
            if (mBluetoothHeadset != null) {
                Log.d(TAG, "updating headset");
                mBluetoothHeadset.phoneStateChanged(mNumActive, mNumHeld,
                                 callState,number, type);
            } else Log.d(TAG, "No need as headset is null");
        }
        if (mFakeMultiParty == true) {
            if (((mSubscriptions[PHONE_ID1].mActive + mSubscriptions[PHONE_ID1].mHeld
                + mSubscriptions[PHONE_ID2].mActive + mSubscriptions[PHONE_ID2].mHeld) <= 2)
                && ((mSubscriptions[PHONE_ID1].mCallState == CALL_STATE_IDLE)
                && ((mSubscriptions[PHONE_ID2].mCallState == CALL_STATE_IDLE)))) {
                log("Reset mFakeMultiParty");
                mFakeMultiParty = false;
            }
        }
        mDsdaCallState = callState; //Know the call state
    }

    private int getBluetoothCallStateForUpdate(int ringingState, int foregroundState) {
        //CallState
        // !! WARNING !!
        // You will note that CALL_STATE_WAITING, CALL_STATE_HELD, and CALL_STATE_ACTIVE are not
        // used in this version of the call state mappings.  This is on purpose.
        // phone_state_change() in btif_hf.c is not written to handle these states. Only with the
        // listCalls*() method are WAITING and ACTIVE used.
        // Using the unsupported states here caused problems with inconsistent state in some
        // bluetooth devices (like not getting out of ringing state after answering a call).
        //
        int bluetoothCallState = CALL_STATE_IDLE;
        if (ringingState == CallState.RINGING) {
            bluetoothCallState = CALL_STATE_INCOMING;
        } else if (foregroundState == CallState.DIALING) {
            bluetoothCallState = CALL_STATE_ALERTING;
        }
        return bluetoothCallState;
    }

    private static void log(String msg) {
        if (DBG) Log.d(TAG, msg);
    }

    private final IBluetoothDsdaService.Stub mBinder = new IBluetoothDsdaService.Stub() {

        /* Handles call state changes on each subscription. */
        public void handleMultiSimPreciseCallStateChange(int foregroundCallState,
                int ringingCallState, String ringingAddress, int ringingAddressType,
                int backgroundCallState, int numHeldCallsonSub) {
            enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, null);

            MultisimCallState mMsimCallState = new MultisimCallState(foregroundCallState,
                    ringingCallState, ringingAddress, ringingAddressType,
                    backgroundCallState, numHeldCallsonSub);

            Message msg = Message.obtain(mHandler, DSDA_CALL_STATE_CHANGED);
            msg.obj = mMsimCallState;
            mHandler.sendMessage(msg);
        }

        /* Set the current SUB*/
        public void setCurrentSub(int sub) {
            log("Call state changed on SUB: " + sub);
            mCurrentSub = sub;
        }

        public void phoneSubChanged() {
            enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, null);
            Message msg = Message.obtain(mHandler, DSDA_PHONE_SUB_CHANGED);
            mHandler.sendMessage(msg);
        }

        /* when HeadsetService is created,it queries for current phone
           state. This function provides the current state*/
        public void processQueryPhoneState() {
            log("Query Call state change");
            enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, null);
            Message msg = Message.obtain(mHandler, DSDA_QUERY_PHONE_STATE_CHANGED);
            mHandler.sendMessage(msg);
        }

        public int getTotalCallsOnSub(int subId) {
            int PhoneId = SubscriptionManager.getPhoneId(subId);
            return ((mSubscriptions[PhoneId].mActive +
                    mSubscriptions[PhoneId].mHeld));
        }

        public boolean isSwitchSubAllowed() {
            boolean allowed = false;
            log("isSwitchSubAllowed:= ");
            log("mActive(PHONE_ID1) " + mSubscriptions[PHONE_ID1].mActive +
                    "mHeld(PHONE_ID1) " + mSubscriptions[PHONE_ID1].mHeld +
                    " mCallState(PHONE_ID1) " + mSubscriptions[PHONE_ID1].mCallState);
            log("mActive(PHONE_ID2) " + mSubscriptions[PHONE_ID2].mActive +
                    "mHeld(PHONE_ID2) " + mSubscriptions[PHONE_ID2].mHeld +
                    " mCallState(PHONE_ID2) " + mSubscriptions[PHONE_ID2].mCallState);

            if ((((mSubscriptions[PHONE_ID1].mActive + mSubscriptions[PHONE_ID1].mHeld) == 1)
                    && (mSubscriptions[PHONE_ID1].mCallState == CALL_STATE_IDLE))
                    && (((mSubscriptions[PHONE_ID2].mActive +
                    mSubscriptions[PHONE_ID2].mHeld) == 1)
                    && (mSubscriptions[PHONE_ID2].mCallState == CALL_STATE_IDLE))) {
                allowed = true;
            }
            log("Is switch SUB allowed: " + allowed);
            return allowed;
        }

        public boolean isFakeMultiPartyCall() {
            log("isFakeMultiParty = " + mFakeMultiParty);
            return mFakeMultiParty;
        }

        public void switchSub() {
            enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, null);
            Message msg = Message.obtain(mHandler, DSDA_SWITCH_SUB);
            mHandler.sendMessage(msg);
        }

        /* Check if call swap can be done on active SUB*/
        public boolean canDoCallSwap() {
            int active = mTelecomManager.getActiveSubscription();
            if (INVALID_SUBID == active) {
                Log.e(TAG, "Invalid active id, returning");
                return false;
            }
            if (getTotalCallsOnSub(active) > 1)
                return true;
            return false;
        }

        public boolean hasCallsOnBothSubs() {
            if (((mSubscriptions[PHONE_ID1].mActive + mSubscriptions[PHONE_ID1].mHeld) >= 1)
                    && ((mSubscriptions[PHONE_ID2].mActive + mSubscriptions[PHONE_ID2].mHeld)
                    >= 1)) {
                log("hasCallsOnBothSubs is true");
                return true;
            }
            return false;
        }

        public boolean answerOnThisSubAllowed() {
            log("answerOnThisSubAllowed.");
            int activeSub = mTelecomManager.getActiveSubscription();
            if (INVALID_SUBID == activeSub) {
                Log.e(TAG, "Invalid activeSub id, returning");
                return false;
            }
            int bgSub =  INVALID_SUBID;
            for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
                int[] subId = SubscriptionManager.getSubId(i);
                if (subId[0] != activeSub) {
                    Log.i(TAG, "other Sub: " + subId[0]);
                    bgSub = subId[0];
                }
            }

            if (bgSub == INVALID_SUBID) /* No calls on bg sub*/
                return false;

            if(getTotalCallsOnSub(bgSub) >= 1)
                return true;
            return false;
        }
    };

    private class MultisimCallState {
        int mForegroundCallState = 0;
        int mRingingCallState = 0;
        String mRingingNumber = "";
        int mRingNumberType = 0;
        int mBackgroundCallState = 0;
        int mNumHeldCallsonSub = 0;

        private MultisimCallState(int foregroundCallState, int ringingCallState,
                String ringingAddress, int ringingAddressType,
                int backgroundCallState, int numHeldCallsonSub) {
            this.mForegroundCallState = foregroundCallState;
            this.mRingingCallState = ringingCallState;
            this.mRingingNumber = ringingAddress;
            this.mRingNumberType = ringingAddressType;
            this.mBackgroundCallState = backgroundCallState;
            this.mNumHeldCallsonSub = numHeldCallsonSub;
        }
    }

}
