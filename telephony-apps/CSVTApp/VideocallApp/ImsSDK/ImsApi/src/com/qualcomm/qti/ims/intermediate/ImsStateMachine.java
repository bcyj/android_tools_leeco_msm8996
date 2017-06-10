/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.view.TextureView;
import android.view.View;
import android.view.TextureView.SurfaceTextureListener;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.Phone;
import com.android.internal.util.AsyncChannel;
import com.android.internal.util.IState;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.qualcomm.qti.ims.intermediate.ICameraHandlerInterface.CameraState;
import com.qualcomm.qti.ims.internal.media.CvoHandler;
import com.qualcomm.qti.ims.internal.media.MediaHandler.MediaEventListener;
import com.qualcomm.qti.ims.internal.telephony.ImsCallProxy;

public class ImsStateMachine extends StateMachine implements
        SurfaceTextureListener {
    private static final String TAG = "ImsStateMachine";
    private static final int CMD_DIALOUT = 1;
    private static final int CMD_REJECT = 2;
    private static final int CMD_ANSWER = 3;
    private static final int CMD_HANGUP = 4;
    private static final int CMD_MODIFY = 5;
    private static final int CMD_ADD_PARTICIPENT = 9;
    private static final int CMD_HANGUPURI = 10;
    private static final int CMD_SWITCH = 11;
    private static final int CMD_HOLD = 12;
    private static final int CMD_MUTE = 13;
    private static final int CMD_DTMF = 14;
    private static final int CMD_CONFERENCE_CALL = 16;
    private static final int CMD_SET_REGISTRATION = 17;
    private static final int CMD_GET_REGISTRATION = 18;
    private static final int CMD_MERGECALLS = 19;
    private static final int CMD_GETLISTINCONF = 20;
    private static final int CMD_END_CONFERENCE = 21;
    private static final int CMD_ACCEPT_CALLTYPE_CHANGE = 22;

    private static final int MSG_INCOMINGCALL = 31;
    private static final int MSG_CALL_STATE_CHANGE = 32;
    private static final int MSG_REMOTE_MODIFY_CALL = 33;
    private static final int MSG_DISCONNECT = 34;
    private static final int MSG_EVO_EVENT = 35;
    private static final int MSG_MODIFY_CALL_RESULT = 36;
    private static final int MSG_AVP_UPGRADE_FAILURE = 37;
    private static final int MSG_REGSTATECHANGED = 38;

    private List<IUIStateListener> mStateListeners = new ArrayList<IUIStateListener>();
    /**
     * This table is for deciding whether consent is required while
     * upgrade/downgrade from one calltype to other Read calltype transition
     * from row to column 1 => Consent of user is required 0 => No consent
     * required eg. from VOLTE to VT-TX, consent is needed so row 0, col 1 is
     * set to 1
     *
     * User consent is needed for all upgrades and not needed for downgrades
     *
     * VOLTE VT-TX VT-RX VT VOLTE | 0 | 1 | 1 | 1 VT-TX | 0 | 0 | 1 | 1 VT-RX |
     * 0 | 1 | 0 | 1 VT | 0 | 0 | 0 | 0
     */

    private int[][] mVideoConsentTable = { { 0, 1, 1, 1 }, { 0, 0, 1, 1 },
            { 0, 1, 0, 1 }, { 0, 0, 0, 0 } };

    protected ImsStateMachine(String name, Context context) {
        super(name);
        mContext = context;
        mCallProxy = ImsCallProxy.getInstance(context);
        mCallProxy.registerCallback(mCallListener);
        mVideoCallManager = VideoCallManager.getInstance(context);
        mVideoCallManager.setOnParamReadyListener(mMediaListener);
        addState(mIdleState);
        addState(mOutgoingState, mIdleState);
        addState(mIncomingState, mIdleState);
        addState(mIncallState, mIdleState);
        addState(mVideoTransmitState, mIncallState);
        addState(mVideoReceiveState, mIncallState);
        addState(mVideoCallState, mIncallState);
        addState(mVolteState, mIncallState);
        setInitialState(mIdleState);
    }

    private Context mContext;

    private State mIdleState = new IdleState();
    private State mOutgoingState = new OutgoingState();
    private State mIncomingState = new IncomingState();
    private State mIncallState = new IncallState();
    private State mVideoTransmitState = new VideoTransmitState();
    private State mVideoReceiveState = new VideoReceiveState();
    private State mVideoCallState = new VideoCallState();
    private State mVolteState = new VolteState();

    private TextureView mCameraSurface;
    private TextureView mFarEndSurface;
    private IUIStateListener mStateListener;

    private AsyncChannel mReplyChannel = new AsyncChannel();
    private int mNumberOfCameras;
    private int mCameraId;
    private int mRegstate;

    private ICallInterface mCallProxy;

    private VideoCallManager mVideoCallManager;

    int prevCallType = Phone.CALL_TYPE_UNKNOWN;

    private boolean mIsDisconnected = false;

    private ICallInterfaceListener mCallListener = new ICallInterfaceListener() {

        @Override
        public void imsRegStateChanged(int regstate) {
            mRegstate = regstate;
            if (regstate == ImsConstants.REGINSTRATION_UNREGISTERED) {
                notifyStateListener(ImsConstants.UNREGISTARATION, null);
            }
        }

        @Override
        public void imsRegStateChangeReqFailed() {

        }

        @Override
        public void onNewRingingConnection(String address, int callType) {
            sendMessage(MSG_INCOMINGCALL, callType, 0, address);
        }

        @Override
        public void onPhoneStateChanged(int state) {
            Log.d(TAG, "onPhoneStateChanged state = " + state);
            sendMessage(MSG_CALL_STATE_CHANGE, state);
            if (state == ImsConstants.ImsState.OFFHOOK || state == ImsConstants.ImsState.RINGING) {
                mIsDisconnected = false;
                mVideoCallManager.earlyMediaInit();
            }
        }

        @Override
        public void onDisconnect(int status) {
            mIsDisconnected = true;
            mVideoCallManager.mediaDeInit();
            sendMessage(MSG_DISCONNECT, status);
        }

        @Override
        public void onAvpUpgradeFailure(String error) {
            sendMessage(MSG_AVP_UPGRADE_FAILURE, error);
        }

        @Override
        public void onRemoteModifyCallRequest(int callType) {
            sendMessage(MSG_REMOTE_MODIFY_CALL, callType);
        }

        @Override
        public void onModifyCallDone(boolean success, String result) {
            sendMessage(MSG_MODIFY_CALL_RESULT, success ? 1 : 0, 0, result);
        }

    };

    private MediaEventListener mMediaListener = new MediaEventListener() {

        @Override
        public void onParamReadyEvent() {
            CameraState cameraState = mVideoCallManager.getCameraState();
            Log.d(TAG, "onParamReadyEvent cameraState= " + cameraState);
            if (cameraState == CameraState.PREVIEW_STARTED) {
                // If camera is already capturing stop preview, reset the
                // parameters and then start preview again
                mVideoCallManager.stopCameraRecording();
                mVideoCallManager.stopCameraPreview();
                mVideoCallManager.initializeCameraParams();
                mVideoCallManager.startPreviewAndRecording(mCameraSurface
                        .getSurfaceTexture());
            }
        }

        @Override
        public void onDisplayModeEvent() {

        }

        @Override
        public void onStartReadyEvent() {

        }

        @Override
        public void onStartPlayEvent() {

        }

        @Override
        public void onStopPlayEvent() {

        }

    };

    class IdleState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "IdleState processMessage:" + msg.what);
            String number;
            int state;
            switch (msg.what) {
            case CMD_DIALOUT:
                mVideoCallManager.earlyMediaInit();
                number = (String) msg.obj;
                int callType = msg.arg1;
                int result = mCallProxy.dial(number, callType);
                mReplyChannel.replyToMessage(msg, CMD_DIALOUT, result);
                if (result == ImsConstants.CALL_STATUS_DIALED) {
                    transitionTo(mOutgoingState);
                    notifyStateListener(ImsConstants.DIALING, null);
                }
                break;

            case MSG_INCOMINGCALL:
                number = (String) msg.obj;
                transitionTo(mIncomingState);
                notifyStateListener(ImsConstants.INCOMING, number);
                break;

            case CMD_SET_REGISTRATION:
                state = msg.arg1;
                if (state != mRegstate) {
                    mCallProxy.setRegistrationState(state);
                }
                break;
            case CMD_GET_REGISTRATION:
                state = mCallProxy.getRegistrationState();
                mRegstate = state;

            default:
                if (msg.replyTo != null) {
                    mReplyChannel.replyToMessage(msg, 0);
                }
                return NOT_HANDLED;
            }

            return HANDLED;
        }

    }

    class OutgoingState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "OutgoingState processMessage:" + msg.what);
            int state;
            switch (msg.what) {
            case MSG_CALL_STATE_CHANGE:
                state = msg.arg1;
                if (state == ImsConstants.ImsState.IDLE) {
                    // notifyStateListener(ImsConstants.IDLE, null);
                } else if (state == ImsConstants.ImsState.OFFHOOK) {
                    int type = mCallProxy.getActiveCallType();
                    Log.d(TAG, "OutgoingState type:" + type);
                    prevCallType = type;
                    if (type == Phone.CALL_TYPE_VT_TX) {
                        notifyStateListener(ImsConstants.INCALL_TX, null);
                        transitionTo(mVideoTransmitState);
                    } else if (type == Phone.CALL_TYPE_VT_RX) {
                        notifyStateListener(ImsConstants.INCALL_RX, null);
                        transitionTo(mVideoReceiveState);
                    } else if (type == Phone.CALL_TYPE_VT) {
                        notifyStateListener(ImsConstants.INCALL_VT, null);
                        transitionTo(mVideoCallState);
                    } else {
                        notifyStateListener(ImsConstants.INCALL_VOLTE, null);
                        transitionTo(mIncallState);
                    }
                }
                break;

            case MSG_DISCONNECT:
                state = msg.arg1;
                transitionTo(mIdleState);
                notifyStateListener(ImsConstants.DISCONNECTED, state);
                break;
            case CMD_HANGUP:
                boolean result = mCallProxy.hangup();
                mReplyChannel.replyToMessage(msg, CMD_HANGUP, result);
                break;
            default:
                return NOT_HANDLED;
            }

            return HANDLED;
        }
    }

    class IncomingState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "IncomingState processMessage:" + msg.what);
            String number;
            boolean result = false;
            switch (msg.what) {
            case CMD_ANSWER:
                int callType = msg.arg1;
                result = mCallProxy.acceptCall(callType);
                mReplyChannel.replyToMessage(msg, CMD_ANSWER, result);
                break;
            case CMD_HANGUP:
                result = mCallProxy.hangup();
                mReplyChannel.replyToMessage(msg, CMD_HANGUP, result);
                break;
            case MSG_CALL_STATE_CHANGE:
                int state = msg.arg1;
                if (state == ImsConstants.ImsState.IDLE) {
                    notifyStateListener(ImsConstants.IDLE, null);
                } else if (state == ImsConstants.ImsState.OFFHOOK) {
                    int type = mCallProxy.getActiveCallType();
                    prevCallType = type;
                    if (type == Phone.CALL_TYPE_VT_TX) {
                        notifyStateListener(ImsConstants.INCALL_TX, null);
                        transitionTo(mVideoTransmitState);
                    } else if (type == Phone.CALL_TYPE_VT_RX) {
                        notifyStateListener(ImsConstants.INCALL_RX, null);
                        transitionTo(mVideoReceiveState);
                    } else if (type == Phone.CALL_TYPE_VT) {
                        notifyStateListener(ImsConstants.INCALL_VT, null);
                        transitionTo(mVideoCallState);
                    } else {
                        notifyStateListener(ImsConstants.INCALL_VOLTE, null);
                        transitionTo(mIncallState);
                    }
                }
                break;
            case MSG_DISCONNECT:
                state = msg.arg1;
                transitionTo(mIdleState);
                notifyStateListener(ImsConstants.DISCONNECTED, state);
            default:
                return NOT_HANDLED;
            }

            return HANDLED;
        }
    }

    class IncallState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "IncallState processMessage:" + msg.what);
            String number;
            switch (msg.what) {
            case CMD_MODIFY:
                mCallProxy.changeConnectionType(msg.arg1);
                break;
            case CMD_HANGUP:
                boolean result = mCallProxy.hangup();
                mReplyChannel.replyToMessage(msg, CMD_HANGUP, result);
                break;

            case MSG_CALL_STATE_CHANGE:
                int type = mCallProxy.getActiveCallType();
                prevCallType = type;
                int state = msg.arg1;
                if (state == ImsConstants.ImsState.IDLE) {
                    notifyStateListener(ImsConstants.DISCONNECTED, null);
                    transitionTo(mIdleState);
                    } else if (state == ImsConstants.ImsState.OFFHOOK) {
                        if (type == Phone.CALL_TYPE_VT_TX) {
                            setVisibleifNotNull(mCameraSurface, View.VISIBLE);
                            setVisibleifNotNull(mFarEndSurface, View.GONE);
                            notifyStateListener(ImsConstants.INCALL_TX, null);
                            transitionTo(mVideoTransmitState);
                        } else if (type == Phone.CALL_TYPE_VT_RX) {
                            setVisibleifNotNull(mCameraSurface, View.GONE);
                            setVisibleifNotNull(mFarEndSurface, View.VISIBLE);
                            notifyStateListener(ImsConstants.INCALL_RX, null);
                            transitionTo(mVideoReceiveState);
                        } else if (type == Phone.CALL_TYPE_VT) {
                            setVisibleifNotNull(mCameraSurface, View.VISIBLE);
                            setVisibleifNotNull(mFarEndSurface, View.VISIBLE);
                            notifyStateListener(ImsConstants.INCALL_VT, null);
                            transitionTo(mVideoCallState);
                        } else {
                            setVisibleifNotNull(mCameraSurface, View.GONE);
                            setVisibleifNotNull(mFarEndSurface, View.GONE);
                            notifyStateListener(ImsConstants.INCALL_VOLTE, null);
                            transitionTo(mVolteState);
                        }
                    }
                break;

            case CMD_MUTE:
                boolean muted = (Boolean) msg.obj;
                mCallProxy.setMute(muted);
                break;
            case CMD_DTMF:
                mCallProxy.startDtmf((Character) msg.obj);
                break;
            case CMD_HOLD:
                mCallProxy.switchHoldingAndActive();
                break;
            case CMD_ACCEPT_CALLTYPE_CHANGE:
                boolean accepted = (Boolean) msg.obj;
                mCallProxy.acceptConnectionTypeChange(accepted);
                break;
            case MSG_DISCONNECT:
                state = msg.arg1;
                transitionTo(mIdleState);
                notifyStateListener(ImsConstants.DISCONNECTED, state);
                break;
            default:
                if (msg.replyTo != null) {
                    mReplyChannel.replyToMessage(msg, 0);
                }
                return NOT_HANDLED;
            }

            return HANDLED;
        }
    }

    private void setVisibleifNotNull(final View view, final int visibility) {
        if (view != null) {
            view.post(new Runnable() {
                @Override
                public void run() {
                    view.setVisibility(visibility);
                }
            });

        }}

    class VideoTransmitState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "VideoTransmitState processMessage:" + msg.what);
            switch (msg.what) {
            case MSG_REMOTE_MODIFY_CALL:
                int type = msg.arg1;
                notifyStateListener(ImsConstants.MODIFYING, type);
                break;
                default:
                return NOT_HANDLED;
            }

            return HANDLED;
        }
    }

    class VideoReceiveState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "VideoReceiveState processMessage:" + msg.what);
            switch (msg.what) {
            case MSG_REMOTE_MODIFY_CALL:
                int type = msg.arg1;
                notifyStateListener(ImsConstants.MODIFYING, type);
                break;
            default:
                return NOT_HANDLED;
            }

            return HANDLED;
        }
    }

    class VideoCallState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "VideoCallState processMessage:" + msg.what);
            switch (msg.what) {
            default:
                return NOT_HANDLED;
            }
            // return HANDLED;
        }
    }

    class VolteState extends State {

        @Override
        public boolean processMessage(Message msg) {
            Log.d(TAG, "VolteState processMessage:" + msg.what);
            switch (msg.what) {
            case CMD_MERGECALLS:
                mCallProxy.mergeCalls();
                break;
            case CMD_ADD_PARTICIPENT:
                String dialString = (String) msg.obj;
                boolean isConference = msg.arg1 == 1;
                mCallProxy.addParticipant(dialString, isConference);
                break;
            case CMD_HANGUPURI:
                String uri = (String) msg.obj;
                mCallProxy.hangupUri(uri);
                break;
            case CMD_GETLISTINCONF:
                String[] result = mCallProxy.getUriListConf();
                mReplyChannel.replyToMessage(msg, CMD_GETLISTINCONF, result);
                break;
            case CMD_END_CONFERENCE:
                mCallProxy.endConference();
                break;
            case MSG_REMOTE_MODIFY_CALL:
                int type = msg.arg1;
                notifyStateListener(ImsConstants.MODIFYING, type);
                break;
            default:
                return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    public void setLocalTexture(TextureView localTexture) {
        mCameraSurface = localTexture;
        mCameraSurface.setSurfaceTextureListener(this);
    }

    public void setRemoteTexture(TextureView remoteTexture) {
        mFarEndSurface = remoteTexture;
        mFarEndSurface.setSurfaceTextureListener(this);
    }

    public boolean isCameraInitNeeded() {
        return mCameraSurface != null
                && mVideoCallManager.getCameraState() == CameraState.CAMERA_CLOSED;
    }

    public void switchCamera() {
        // Stop camera preview if already running
        if (mVideoCallManager.getCameraState() != CameraState.CAMERA_CLOSED) {
            mVideoCallManager.stopRecordingAndPreview();
            mVideoCallManager.closeCamera();
        }
        if (mCameraId == mVideoCallManager.getFrontCameraId()) {
            mCameraId = mVideoCallManager.getBackCameraId();
        } else if (mCameraId == mVideoCallManager.getBackCameraId()) {
            mCameraId = mVideoCallManager.getBackCameraId();
        }
        // Restart camera if camera doesn't need to stay off
        if (isCameraInitNeeded()) {
            mVideoCallManager.initializeCamera(
                    mCameraSurface.getSurfaceTexture(), mCameraId);
        }
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
            int height) {
        if (surface.equals(mCameraSurface.getSurfaceTexture())) {
            Log.d(TAG, "Camera surface texture created");
            if (isCameraInitNeeded()) {
            if (1 == mVideoCallManager.getNumberOfCameras()) {
                mCameraId = mVideoCallManager.getBackCameraId();
            } else {
                mCameraId = mVideoCallManager.getFrontCameraId();
            }
                mVideoCallManager.initializeCamera(surface, mCameraId);
            }
        } else if (surface.equals(mFarEndSurface.getSurfaceTexture())) {
            log("Video surface texture created");
            mVideoCallManager.setFarEndSurface(surface);
            mVideoCallManager.setCvoEventListener(new CvoListener());
            mVideoCallManager.startOrientationListener(true);
        }
        Log.d(TAG, "onSurfaceTextureAvailable surface=" + surface);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        if ((mCameraSurface != null)
                && (surface.equals(mCameraSurface.getSurfaceTexture()))) {
            Log.d(TAG, "CameraPreview surface texture destroyed");
            if (mVideoCallManager.getCameraState() != CameraState.CAMERA_CLOSED) {
                mVideoCallManager.stopRecordingAndPreview();
                mVideoCallManager.closeCamera();
            }
        } else if ((mFarEndSurface != null)
                && (surface.equals(mFarEndSurface.getSurfaceTexture()))) {
            Log.d(TAG, "FarEndView surface texture destroyed");
            mVideoCallManager.setFarEndSurface(null);
        }

        Log.d(TAG, "onSurfaceTextureDestroyed surface=" + surface);
        return true;

    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture arg0, int arg1,
            int arg2) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture arg0) {

    }

    public int requestDial(String number, int callType) {
        int result = mCallProxy.dial(number, callType);
        if (result == ImsConstants.CALL_STATUS_DIALED) {
            sendMessage(CMD_DIALOUT);
        }
        return result;
    }

    public void answerCall(int callType) {
        sendMessage(CMD_ANSWER, callType);
    }

    public void hangup() {
        sendMessage(CMD_HANGUP);
    }

    public boolean requestModifyCall(int type) {
        if (isUserConsentRequired(type, prevCallType)) {
            sendMessage(CMD_MODIFY, type);
            return true;
        }
        return false;
    }

    public void requestMute(boolean mute) {
        sendMessage(CMD_MUTE, mute);
    }

    public void requestDtmf(char c) {
        if (PhoneNumberUtils.is12Key(c)) {
            sendMessage(CMD_DTMF, c);
        }
    }

    private boolean isUserConsentRequired(int callType, int prevCallType) {
        return true;// mVideoConsentTable[prevCallType][callType] == 1;
    }

    private void notifyStateListener(int state, Object result) {
        for (IUIStateListener listener : mStateListeners) {
            listener.onUIStateChange(state, result);
        }
    }

    public void registerForStateChange(IUIStateListener listener) {
        mStateListeners.add(listener);
    }

    public int syncDialout(String address, int callType, AsyncChannel channel) {
        Message resultMsg = channel.sendMessageSynchronously(CMD_DIALOUT,
                callType, 0, address);
        int result = resultMsg.arg1;
        resultMsg.recycle();
        return result;
    }

    public boolean syncAccept(int callType, AsyncChannel channel) {
        Message resultMsg = channel.sendMessageSynchronously(CMD_ANSWER,
                callType);
        boolean result = false;
        if (resultMsg.what == CMD_ANSWER) {
            result = (Boolean) resultMsg.obj;
        }
        resultMsg.recycle();
        return result;
    }

    public boolean syncHangup(AsyncChannel channel) {
        Message resultMsg = channel.sendMessageSynchronously(CMD_HANGUP);
        boolean result = false;
        if (resultMsg.what == CMD_HANGUP) {
            result = (Boolean) resultMsg.obj;
        }
        resultMsg.recycle();
        return result;
    }

    public void switcholdCall() {
        sendMessage(CMD_HOLD);
    }

    public void mergeCalls() {
        sendMessage(CMD_MERGECALLS);
    }

    public void addParticipant(String dialString, boolean isConference) {
        sendMessage(CMD_ADD_PARTICIPENT, isConference ? 1 : 0, 0, dialString);
    }

    public String[] syncgetUriListInConf(AsyncChannel channel) {
        Message resultMsg = channel.sendMessageSynchronously(CMD_GETLISTINCONF);
        String[] result = null;
        if (resultMsg.what == CMD_GETLISTINCONF) {
            result = (String[]) resultMsg.obj;
        }
        resultMsg.recycle();
        return result;
    }

    public void endConferenceConnection() {
        sendMessage(CMD_END_CONFERENCE);
    }

    public void hangupUri(String uri) {
        sendMessage(CMD_HANGUPURI, uri);
    }

    public void setRegistrationState(int imsRegState) {
        sendMessage(CMD_SET_REGISTRATION, imsRegState);
    }

    public int syncGetRegistrationState(AsyncChannel channel) {
        Message resultMsg = channel
                .sendMessageSynchronously(CMD_GET_REGISTRATION);
        int result = -1;
        if (resultMsg.what == CMD_GET_REGISTRATION) {
            result = resultMsg.arg1;
        }
        resultMsg.recycle();
        return result;
    }

    public void init() {
        Log.d(TAG, "init");
        mCallProxy.createSdkService();
    }

    public void acceptConnectionTypeChange(boolean accept) {
        Log.d(TAG, "acceptConnectionTypeChange accept = "+ accept);
        sendMessage(CMD_ACCEPT_CALLTYPE_CHANGE, accept);
    }

    public class CvoListener implements CvoHandler.CvoEventListener {
        @Override
        public void onDeviceOrientationChanged(int rotation) {
            int requiredSurfaceRotation = 360 - rotation;
            Log.d(TAG, "onDeviceOrientationChanged: Local sensor rotation ="
                    + rotation + " Rotate far end based on local sensor by "
                    + requiredSurfaceRotation);
            if (mFarEndSurface != null){
                mFarEndSurface.setRotation(requiredSurfaceRotation);
            }
        }
    }
}
