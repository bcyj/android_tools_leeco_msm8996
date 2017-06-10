/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.android.internal.telephony.Phone;
import com.qualcomm.qti.ims.intermediate.ICallInterface;
import com.qualcomm.qti.ims.intermediate.ICallInterfaceListener;
import com.qualcomm.qti.ims.intermediate.ImsConstants;

public class FakeCallProxy implements ICallInterface {
    private static String LOG_TAG = "FakeCallProxy";
    private ICallInterfaceListener mListener;

    private boolean mImsActive = false;

    private static final int MSG_ACTIVE = 1;
    private static final int MSG_IDLE = 2;
    private static final int MSG_RINGING = 3;
    private static final int MSG_DISCONNECT = 3;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_ACTIVE:
                mListener.onPhoneStateChanged(ImsConstants.ImsState.OFFHOOK);
                break;
            case MSG_DISCONNECT:
                mListener.onDisconnect(3);// DisconnectCause.Local
                break;

            }
        }

    };

    @Override
    public int dial(String number, int callType) {
        Log.d(LOG_TAG, "dial number = " + number + "; callType = " + callType);
        mImsActive = true;
        return 0;
    }

    @Override
    public boolean acceptCall(int callType) {
        Log.d(LOG_TAG, "acceptCall calltype=" + callType);
        return true;
    }

    @Override
    public boolean hangup() {
        Log.d(LOG_TAG, "hangup()");
        mImsActive = false;
        return true;
    }

    @Override
    public void registerCallback(ICallInterfaceListener listener) {
        Log.d(LOG_TAG, "registerCallback");
        mListener = listener;
    }

    @Override
    public int getActiveCallType() {
        Log.d(LOG_TAG, "getActiveCallType");
        return Phone.CALL_TYPE_VT;
    }

    @Override
    public void setRegistrationState(int imsRegState) {
        Log.d(LOG_TAG, "setRegistrationState imsRegState=" + imsRegState);

    }

    @Override
    public void changeConnectionType(int callType) {
        Log.d(LOG_TAG, "changeConnectionType callType=" + callType);
    }

    @Override
    public void acceptConnectionTypeChange(boolean accept) {
        Log.d(LOG_TAG, "acceptConnectionTypeChange accept = accept");

    }

    @Override
    public void addParticipant(String dialString, boolean isConference) {
        Log.d(LOG_TAG, "addParticipant dialString = " + dialString
                + " isConference=" + isConference);
    }

    @Override
    public boolean isImsPhoneActive() {
        Log.d(LOG_TAG, "isImsPhoneActive");
        return mImsActive;
    }

    @Override
    public boolean isImsPhoneIdle() {
        Log.d(LOG_TAG, "isImsPhoneIdle");
        return !mImsActive;
    }

    @Override
    public void mergeCalls() {
        Log.d(LOG_TAG, "mergeCalls");

    }

    @Override
    public void setMute(boolean muted) {
        Log.d(LOG_TAG, "setMute muted= " + muted);

    }

    @Override
    public void startDtmf(char c) {
        Log.d(LOG_TAG, "startDtmf char= " + c);

    }

    @Override
    public String[] getUriListConf() {
        Log.d(LOG_TAG, "getUriListConf");
        return new String[] { "11111", "22222", "333333" };
    }

    @Override
    public void switchHoldingAndActive() {
        Log.d(LOG_TAG, "switchHoldingAndActive");

    }

    @Override
    public void hangupUri(String uri) {
        Log.d(LOG_TAG, "hangupUri uri=" + uri);

    }

    @Override
    public void endConference() {
        Log.d(LOG_TAG, "endConference");

    }

    @Override
    public void createSdkService() {
        Log.d(LOG_TAG, "createSdkService");

    }

    @Override
    public int getRegistrationState() {
        // REGISTERED 1 NOT_REGISTERED 2
        return 1;
    }

}
