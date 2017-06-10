/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

import java.util.Map;

import com.android.internal.util.AsyncChannel;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;
import android.view.TextureView;
import android.view.TextureView.SurfaceTextureListener;

public final class ImsPhoneManager {
    private static final String TAG = "ImsPhoneManager";
    private static ImsPhoneManager mInstance;

    private ImsStateMachine mImsStateMachine;

    private TextureView mCameraSurface;
    private TextureView mFarEndSurface;
    private Context mContext;
    private boolean inited = false;

    private ImsPhoneManager(Context context) {
        mImsStateMachine = new ImsStateMachine("ImsPhone", context);
        mImsStateMachine.start();
        mContext = context;
    }

    public static ImsPhoneManager getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new ImsPhoneManager(context);
        }
        return mInstance;
    }

    public void init() {
        if (!inited) {
            mImsStateMachine.init();
            HandlerThread imsThread = new HandlerThread("imsPhoneManager");
            imsThread.start();
            mImsSMHandler = new ImsStateMachineHandler(imsThread);
        }
        inited = true;
    }

    public void deinit() {
        inited = false;
        //
    }

    public void setLocalTexture(TextureView localTexture) {
        mImsStateMachine.setLocalTexture(localTexture);
    }

    public void setRemoteTexture(TextureView remoteTexture) {
        mImsStateMachine.setRemoteTexture(remoteTexture);
    }

    public void setOnDisplayModeChanged() {

    }

    public String[] getUriListinConf() {
        return mImsStateMachine.syncgetUriListInConf(mImsMachineChannel);
    }

    public void hangupUri(String uri) {
        mImsStateMachine.hangupUri(uri);
    }

    // always end forground connection
    public void endConferenceConnection() {
        mImsStateMachine.endConferenceConnection();
    }

    public int dial(String address, int callType) {
        return mImsStateMachine.syncDialout(address, callType,
                mImsMachineChannel);
    }

    public boolean accept(int callType) {
        return mImsStateMachine.syncAccept(callType, mImsMachineChannel);
    }

    public boolean hangup() {
        return mImsStateMachine.syncHangup(mImsMachineChannel);
    }

    public void startTone(char c) {
        mImsStateMachine.requestDtmf(c);
    }

    public boolean modifyCall(int type) {
        return mImsStateMachine.requestModifyCall(type);
    }

    public void acceptConnectionTypeChange(boolean accept) {
        mImsStateMachine.acceptConnectionTypeChange(accept);
    }

    public void holdCall() {
        mImsStateMachine.switcholdCall();
    }

    public void mergeCalls() {
        mImsStateMachine.mergeCalls();
    }

    public void addParticipant(String dialString, boolean isConference) {
        mImsStateMachine.addParticipant(dialString, isConference);
    }

    public void setMute(boolean mute) {
        mImsStateMachine.requestMute(mute);
    }

    public void registerForStateChange(IUIStateListener listener) {
        mImsStateMachine.registerForStateChange(listener);
    }

    public void switchCamera() {
        mImsStateMachine.switchCamera();
    }

    public void setRegistrationState(int imsRegState) {
        mImsStateMachine.setRegistrationState(imsRegState);
    }

    public int getRegistrationState() {
        return mImsStateMachine.syncGetRegistrationState(mImsMachineChannel);
    }

    private AsyncChannel mImsMachineChannel;

    private class ImsStateMachineHandler extends Handler {
        private AsyncChannel mImsChannel;

        ImsStateMachineHandler(HandlerThread imsThread) {
            super(imsThread.getLooper());
            mImsChannel = new AsyncChannel();
            mImsChannel.connect(mContext, this, mImsStateMachine.getHandler());
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage msg=" + msg.what);
            switch (msg.what) {
            case AsyncChannel.CMD_CHANNEL_HALF_CONNECTED: {
                if (msg.arg1 == AsyncChannel.STATUS_SUCCESSFUL) {
                    mImsMachineChannel = mImsChannel;
                } else {
                    Log.e(TAG, "ImsStateMachine connection failure, error="
                            + msg.arg1);
                    mImsMachineChannel = null;
                }
                break;
            }
            case AsyncChannel.CMD_CHANNEL_DISCONNECTED: {
                Log.e(TAG, "ImsStateMachine channel lost, msg.arg1 ="
                        + msg.arg1);
                mImsMachineChannel = null;
                // Re-establish connection to state machine
                mImsChannel.connect(mContext, this,
                        mImsStateMachine.getHandler());
                break;
            }
            default: {
                Log.d(TAG, "ImsStateMachineHandler.handleMessage ignoring msg="
                        + msg);
                break;
            }
            }
        }
    }

    private ImsStateMachineHandler mImsSMHandler;

}
