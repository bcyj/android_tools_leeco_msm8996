/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.telecom.CameraCapabilities;
import android.telecom.Connection;
import android.telecom.Connection.VideoProvider;
import android.telecom.VideoProfile;
import android.util.Log;
import android.view.Surface;

import com.android.ims.internal.ImsVideoCallProvider;

import org.codeaurora.ims.CallDetails;
import org.codeaurora.ims.CallModify;
import org.codeaurora.ims.ImsCallModification;
import org.codeaurora.ims.ImsCallSessionImpl;
import org.codeaurora.ims.ImsCallUtils;
import org.codeaurora.ims.ImsRilException;

public class ImsVideoCallProviderImpl extends ImsVideoCallProvider implements
        ImsCallSessionImpl.Listener {
    private static final String TAG = "VideoCall_ImsVideoCallProviderImpl";

    private static final int EVENT_SEND_SESSION_MODIFY_REQUEST_DONE = 0;
    private ImsCallSessionImpl mCallSession;
    private boolean mIsOpen;

    private CameraController mCamera;
    private MediaController mMedia;
    private ImsCallModification mImsCallModification;
    VideoProfile mRequestProfile;
    VideoProfile mResponseProfile;
    private Handler mHandler;
    private static final boolean DBG = true;

    public ImsVideoCallProviderImpl(ImsCallSessionImpl callSession, ImsCallModification imsCallMod) {
        log("ImsVideocallProviderImpl instance created callSession=" + callSession + " imsCallMod="
                + imsCallMod);
        mCallSession = callSession;
        mCamera = CameraController.getInstance();
        mMedia = MediaController.getInstance();
        mImsCallModification = imsCallMod;
        mIsOpen = true;
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                log("Message received: what = " + msg.what);
                switch (msg.what) {
                    case EVENT_SEND_SESSION_MODIFY_REQUEST_DONE:
                        handleSessionModifyDone(msg);
                        break;
                    default:
                        log("Unknown message = " + msg.what);
                }
            }
        };
    }

    boolean isOpen() {
        log(" isOpen " + mIsOpen);
        return mIsOpen;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetCamera(String cameraId) {
        log("onSetCamera, " + cameraId);
        if (!isSessionValid()) return;

        // TODO: Maybe add logic for validating requests. i.e. Provider#2 cannot open
        // the camera unless Provider#1 closes it. That is, prevent resource "stealing"...
        if (cameraId != null) {
            mCamera.open(cameraId, getCallSession());
        } else {
            mCamera.close(getCallSession());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetPreviewSurface(Surface surface) {
        log("onSetPreviewSurface, " + surface);
        if (!isSessionValid()) return;

        mCamera.setPreviewSurface(surface, getCallSession());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetDisplaySurface(Surface surface) {
        log("onSetDisplaySurface, " + surface);
        if (!isSessionValid()) return;

        mMedia.setDisplaySurface(surface);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetDeviceOrientation(int rotation) {
        log("onSetDeviceOrientation, " + rotation);
        if (!isSessionValid()) return;

        mMedia.setDeviceOrientation(rotation);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetZoom(float value) {
        log("onSetZoom, " + value);
        if (!isSessionValid()) return;

        mCamera.setZoom(value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSendSessionModifyRequest(VideoProfile requestProfile) {
        log("onSendSessionModifyRequest, videoState=" + requestProfile.getVideoState()
                + " quality= " + requestProfile.getQuality());
        mRequestProfile = requestProfile;
        if (!isSessionValid()) return;

        // If video pause is requested ignore the call type
        if (isVideoPauseRequested(requestProfile)) {
            mImsCallModification.changeConnectionType(null, CallDetails.CALL_TYPE_VT_PAUSE, null);
        } else if (mImsCallModification.isLocallyPaused()) {
            // If UE is locally paused, means that this is a resume request
            mImsCallModification.changeConnectionType(null, CallDetails.CALL_TYPE_VT_RESUME, null);
        } else {
            // Neither pause or resume, so this is upgrade/downgrade request
            Message newMsg = mHandler.obtainMessage(EVENT_SEND_SESSION_MODIFY_REQUEST_DONE);
            int callType = ImsCallUtils.convertVideoStateToCallType(requestProfile.getVideoState());
            mImsCallModification.changeConnectionType(newMsg, callType, null);
        }
    }

    /**
     * EVENT_SEND_SESSION_MODIFY_REQUEST_DONE is completed
     */
    private void handleSessionModifyDone(Message msg) {
        if (DBG) log("handleSessionModifyDone msg.what=" + msg.what);

        AsyncResult ar = (AsyncResult) msg.obj;
        int status = VideoProvider.SESSION_MODIFY_REQUEST_FAIL;
        if (ar != null) {
            if (ar.exception == null) {

                log("Session modify success");
                status = VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                mResponseProfile = mRequestProfile;
            } else if(ar.exception instanceof ImsRilException){
                loge("Session modify error");
                ImsRilException imsRilException = (ImsRilException) ar.exception;
                status = ImsCallUtils.getUiErrorCode(imsRilException.getErrorCode());
                mResponseProfile = ImsCallUtils.convertToVideoProfile(
                        mImsCallModification.mImsCallSessionImpl.getInternalCallType(),
                        VideoProfile.QUALITY_DEFAULT);
            }
        } else {
            loge("handleSessionModifyDone:null message object");
        }
        receiveSessionModifyResponse(status, mRequestProfile, mResponseProfile);
    }

    private boolean isVideoPauseRequested(VideoProfile requestProfile) {
        log("isVideoPauseRequested requestProfile=" + requestProfile);
        return VideoProfile.VideoState.isPaused(requestProfile.getVideoState());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSendSessionModifyResponse(VideoProfile responseProfile) {
        log("onSendSessionModifyResponse, responseProfile state=" + responseProfile.getVideoState()
                + " quality= " + responseProfile.getQuality());
        if (!isSessionValid()) return;

        int callType = ImsCallUtils.convertVideoStateToCallType(responseProfile.getVideoState());
        // Accept upgrade request
        mImsCallModification.acceptConnectionTypeChange(callType, null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onRequestCameraCapabilities() {
        log("onRequestCameraCapabilities");
        if (!isSessionValid()) return;

        CameraCapabilities cc = mCamera.getCameraCapabilities();
        if (cc != null) {
            changeCameraCapabilities(cc);
        } else {
            loge("Error onRequestCameraCapabilities camera capabilities is null");
        }
    }

    public void onCallDataUsageChanged(long dataUsage) {
        log("onCallDataUsageChanged: dataUsage = " + dataUsage);

        changeCallDataUsage((int)dataUsage);
    }

    public void onUpdatePeerDimensions(int width, int height) {
        log("onUpdatePeerDimensions width=" + width + " height= " + height);
        if (!isSessionValid()) return;

        changePeerDimensions(width, height);
    }

    public void sendCameraStatus(boolean hasFailed) {
        log("sendCameraFailure");
        final int status = hasFailed ? Connection.VideoProvider.SESSION_EVENT_CAMERA_FAILURE :
            Connection.VideoProvider.SESSION_EVENT_CAMERA_READY;

        handleCallSessionEvent(status);
    }

    public void onUpdateVideoQuality(int videoQuality) {
        log("onUpdateVideoQuality video quality is=" + videoQuality);
        if (!isSessionValid()) return;

        changeVideoQuality(videoQuality);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onRequestCallDataUsage() {
        log("onRequestCallDataUsage");
        if (!isSessionValid()) return;

        int mediaId = mCallSession.getMediaId();
        if (mCallSession.hasMediaIdValid()) {
            mMedia.requestCallDataUsage(mediaId);
        } else {
            loge("onRequestCallDataUsage: Invalid MediaId = " + mediaId);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSetPauseImage(String uri) {
        log("onSetPauseImage, " + uri);
        if (!isSessionValid()) return;

        // Not supported.
    }

    @Override
    public void onDisconnected(ImsCallSessionImpl session) {
        log("onDisconnected session" + session);
        mIsOpen = false;
    }

    @Override
    public void onClosed(ImsCallSessionImpl session) {
        mIsOpen = false;
        mMedia = null;
        mCallSession = null;
        mCamera = null;
    }

    /**
     * Unsol modify call indication
     * TODO: Change parameter from callModify to newCallType
     */
    @Override
    public void onUnsolCallModify(ImsCallSessionImpl session, CallModify callModify) {
        log("onCallTypeChanged session=" + session + " callModify= " + callModify);
        int newVideoState = ImsCallUtils
                .convertCallTypeToVideoState(callModify.call_details.call_type);
        VideoProfile vcp = new VideoProfile(newVideoState, VideoProfile.QUALITY_DEFAULT);
        if (callModify.error()) {
            // Notify UI regarding error
            int uiError = ImsCallUtils.convertImsErrorToUiError(callModify.error);
            receiveSessionModifyResponse(uiError, vcp, null);
        } else {
            receiveSessionModifyRequest(vcp);
        }
    }

    /* package */
    ImsCallSessionImpl getCallSession() {
        return mCallSession;
    }

    private String logString(String msg) {
        return "(" + (mCallSession != null ? mCallSession.getCallId() : "null") + ") " + msg;
    }

    private boolean isSessionValid() {
        boolean isValid = mIsOpen;
        if (!mIsOpen) {
            loge("Session is closed." + this);
        }
        return isValid;
    }

    private void log(String msg) {
        Log.d(TAG, logString(msg));
    }

    private void loge(String msg) {
        Log.e(TAG, logString(msg));
    }
}
