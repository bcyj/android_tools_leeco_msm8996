/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.telecom.CameraCapabilities;
import android.util.Size;
import android.util.Log;
import android.view.Surface;

import org.codeaurora.ims.CallModify;
import org.codeaurora.ims.ImsCallSessionImpl;

import java.util.ArrayList;

/**
 * The class is responsible for managing camera related requests.
 */
public class CameraController implements ImsMedia.CameraListener,
                                         ImsCallSessionImpl.Listener,
                                         Camera.Listener {
    private static String TAG = "VideoCall_CameraController";
    private static final boolean DBG = true;
    private static CameraController sInstance;

    private Context mContext;
    private ImsMedia mMedia;
    private Camera mCamera;
    private boolean mIsRecordingEnabled = false;

    // Flag to indicate whether recording has started or not False: recording hasn't started True:
    // recording has started
    private static boolean mIsRecordingStarted = false;

    private Surface mPreviewSurface;
    private Surface mRecordingSurface;
    private Camera.ConfigIms mCameraConfigIms=new Camera.ConfigIms(320, 240, 20);

    private final Object mSyncObject = new Object();
    private ImsCallSessionImpl mCameraOwner;

    private CameraController(Context context, ImsMedia media) {
        mContext = context;
        mMedia = media;
        mMedia.setCameraListener(this);
        CameraManager.init(context);
    }

    /**
     * Opens the requested camera and closes all other camera.
     *
     * Re-opening camera is relatively expensive (time consuming) operation which results in
     * noticeable delay at UI side. To minimize this delay we don't re-open (close/open)
     * the camera again if it is already opened by another session. We simply adjust the camera
     * owner, that is silently remove the old owner (without closing the camera) and add a
     * new owner.
     *
     * @param cameraId Camera to open.
     * @param session Session which requests to open the camera.
     */
    private void doOpen(String cameraId, ImsCallSessionImpl session)
            throws CameraAccessException {
        log("doOpen mCamera =" + mCamera + " cameraId=" + cameraId +
                " Session=" + getSessionInfo(session));

        releaseCurrentOwner();
        if (mCamera != null && !mCamera.getId().equals(cameraId)) {
            closeCamera();
        }

        final boolean reOpen = mCamera == null;
        if (reOpen) {
            mCamera = CameraManager.open(cameraId, this);
        }

        setOwner(session);

        if (reOpen && mCameraConfigIms != null) {
            mCamera.reconfigure(mCameraConfigIms);
        }
    }

    /**
     * closes all open cameras.
     */
    private void closeCamera() {
        if (mCamera != null) {
            mCamera.close();
        }
        mCamera = null;
    }

    /**
     * Initializes CameraController singleton. This must be called once.
     * @param context Application context to use.
     * @return The instance of CameraController.
     */
    public static synchronized CameraController init(Context context, ImsMedia media) {
        if (sInstance == null) {
            sInstance = new CameraController(context, media);
        } else {
            throw new RuntimeException("CameraController: Multiple initialization");
        }
        return sInstance;
    }

    /**
     * Provides access to CameraController.
     * @return The instance of CameraController.
     * @see CameraController#init
     */
    public static synchronized CameraController getInstance() {
        if (sInstance != null) {
            return sInstance;
        } else {
            throw new RuntimeException("CameraController: Not initialized");
        }
    }

    /**
     * Opens the requested camera and closes all other camera. if the requested camera is already
     * open the request will be ignored.
     * @param cameraId Camera to open.
     * @param session IMS session which requests access to camera.
     * TODO Check if same camera ids can be used for both ImsCamera and Camera2.
     */
    public void open(String cameraId, ImsCallSessionImpl session) {
        log("open: camearId=" + cameraId + " session=" + getSessionInfo(session));
        if (cameraId == null) {
            loge("open: Invalid camera id, " + cameraId);
            throw new RuntimeException("CameraController: Invalid camera id.");
        }

        try {
            doOpen(cameraId, session);
        } catch (CameraAccessException e) {
            log("open: Failed to open, cameraId=" + cameraId + " Exception=" + e);
            sendCameraStatus(session, e.getReason());
        }
    }

    /**
     * Removes the session from the list owners and closes the camera if the list is empty.
     * @param session IMS session to be removed from the list.
     */
    public void close(ImsCallSessionImpl session) {
        log("close");
        doClose(session);
    }


    /**
     * Sets the preview surface and updates (starts/stops) camera session (preview/recording)
     *
     * @param surface Preview surface.
     */
    public void setPreviewSurface(Surface surface, ImsCallSessionImpl session) {
        synchronized (mSyncObject) {
            if (DBG) log("setPreviewSurface, surface=" + surface +
                        " Session=" + getSessionInfo(session));

            if (!isOwner(session)) {
                log("setPreviewSurface: " + getSessionInfo(session) + " is not a camera owner."
                        + " Camera owner=" + getSessionInfo(mCameraOwner));
                return;
            }

            if (surface != null) {
                log("setPreviewSurface: GenerationId=" + surface.getGenerationId());
            }

            try {
                if (mPreviewSurface != null && surface != null) {
                    log("setPreviewSurface: Restarting camera preview...");
                    mPreviewSurface = null;
                    updateCameraSession();
                }
                mPreviewSurface = surface;
                updateCameraSession();
            } catch (CameraAccessException e) {
                sendCameraStatus(e.getReason());
            }
        }
    }

    public void setZoom(float value) {
        if (mCamera == null || !mCamera.isOpen()) {
            log("setZoom: Camera is not in correct state, camera=" + mCamera);
            return;
        }

        try {
            mCamera.setZoom(value);
        } catch (CameraAccessException e) {
            loge("Failed to change zoom, value=" + value + "Exception=" + e);
        }
    }

    CameraCapabilities getCameraCapabilities() {
        log("getCameraCapabilities mCamera=" + mCamera);
        if (mCamera != null) {
            try {
                final Size size = mCamera.getPreviewSize();
                final Boolean isZoomSupported = mCamera.isZoomSupported();
                final float maxZoom = mCamera.getMaxZoom();
                log("getCameraCapabilities: PreviewSize=" + size + " isZoomSupported="
                        + isZoomSupported + " maxZoom=" + maxZoom);
                return new CameraCapabilities(isZoomSupported, maxZoom,
                        size.getWidth(), size.getHeight());
            } catch (CameraAccessException e) {
                sendCameraStatus(e.getReason());
                return null;
            }
        } else {
            return null;
        }
    }

    /**
     * Sets a session as a camera owner and subscribes for session events.
     * @param session IMS session to be added to the list.
     */
    private void setOwner(ImsCallSessionImpl session) {
        synchronized (mSyncObject) {
            log("setOwner: Session=" + getSessionInfo(session));
            if (!isOwner(session)) {
                releaseCurrentOwner();
                mCameraOwner = session;
                mCameraOwner.addListener(this);
            } else {
                log("setOwner: The session is already registered as camera owner, session=" +
                        session);
            }
        }
    }

    private void removeOwner(ImsCallSessionImpl session) {
        synchronized (mSyncObject) {
            log("removeOwner: Session=" + getSessionInfo(session));
            if (isOwner(session)) {
                releaseCurrentOwner();
            } else {
                log("removeOwner: Ignoring... The session is not a camera owner, cameraOwner=" +
                        getSessionInfo(session));
            }
        }
    }

    private boolean isOwner(ImsCallSessionImpl session) {
        synchronized (mSyncObject) {
            return session != null && session == mCameraOwner;
        }
    }

    private void releaseCurrentOwner() {
        synchronized (mSyncObject) {
            log("releaseCurrentOwner: Session=" + getSessionInfo(mCameraOwner));
            if (mCameraOwner != null) {
                mCameraOwner.removeListener(this);
                mCameraOwner = null;
            }
        }
    }


    private void doClose(ImsCallSessionImpl session) {
        log("doClose: Camera=" + mCamera + " Session=" + getSessionInfo(session));
        synchronized (mSyncObject) {
            removeOwner(session);

            if (mCameraOwner == null) {
                closeCamera();
                mPreviewSurface = null;
            } else {
                log("doClose: Not closing camera=" + mCamera +
                        " another session still requires it, session="
                        + getSessionInfo(mCameraOwner));
            }
        }
    }

    /**
     * startPreview does setPreviewSurface, Starting Preview and recording
     * @param surface
     */
    private void updateCameraSession() throws CameraAccessException {
        if (DBG) log("updateCameraSession");
        if (mCamera == null || !mCamera.isOpen()) {
            log("updateCameraSession: Camera is not in correct state, camera=" + mCamera);
            return;
        }

        final boolean isPreviewStarted = mCamera.isPreviewStarted();
        final boolean isRecordingStarted = mCamera.isRecordingStarted();
        final boolean canStartPreview = mPreviewSurface != null;
        final boolean canStartRecording = mIsRecordingEnabled &&
                        canStartPreview;

        log("updateCameraSession mPreviewSurface=" + mPreviewSurface + " mIsRecordingEnabled="
                                + mIsRecordingEnabled + " mRecordingSurface= " + mRecordingSurface);
        log("updateCameraSession canStartRecording=" + canStartRecording + " isRecordingStarted=" +
                                isRecordingStarted);

        if (canStartRecording && !isRecordingStarted) {
            mCamera.startRecording(mPreviewSurface, mRecordingSurface);
        } else if (canStartPreview && !isPreviewStarted) {
            mCamera.startPreview(mPreviewSurface);
        } else if (!canStartPreview && isPreviewStarted) {
            mCamera.stopPreview();
        }
    }


    public void onRecordingEnabled() {
        if (DBG) log("onRecordingEnabled");
        try {
            mIsRecordingEnabled = true;
            updateCameraSession();
        } catch(CameraAccessException e) {
            sendCameraStatus(e.getReason());
        }
    }

    public void onRecordingDisabled() {
        if (DBG) log("onStopReadyEvent");
        mIsRecordingEnabled = false;
    }

    public void onMediaDeinitialized() {
        if (DBG) log("onMediaDeInitialized. closing Camera");
        if (mCameraOwner != null) {
            close(mCameraOwner);
        }
        // In general, calling close() is enough. However, it has some internal checks which
        // might prevent closing of camera in some cases, from the other side closeCamera() has
        // no checks, so it is guaranteed that the camera will be closed.
        closeCamera();
    }


    public void onCameraConfigChanged(int w, int h, short fps, Surface surface) {
        if (DBG) log("onCameraParamsReady");

        mCameraConfigIms = new Camera.ConfigIms(w, h, fps);
        mRecordingSurface = surface;
        if (DBG) log("onCameraParamsReady, cameraConfig=" + mCameraConfigIms);

        if ( mCamera == null || !mCamera.isOpen()) {
            loge("onParamReadyEvent: Camera is not open deferring configuration...");
            return;
        }

        try {
            if (mCamera.isPreviewStarted()) {
                mCamera.stopPreview();
            }
            mCamera.reconfigure(mCameraConfigIms);
            updateCameraSession();
        } catch (CameraAccessException e) {
            sendCameraStatus(e.getReason());
        }
    }

    private String getSessionInfo(ImsCallSessionImpl session) {
        if (session == null) {
            return null;
        }
        return session.toSimpleString();
    }

    private void sendCameraStatus(ImsCallSessionImpl session, int error) {
        if (session == null) {
            loge("sendCameraStatus: session is null. CameraStatus=" + error);
            return;
        }

        log("sendCameraStatus: Notifying Session=" + session.getCallId());
        ImsVideoCallProviderImpl provider = session.getImsVideoCallProviderImpl();
        provider.sendCameraStatus(true);
    }

    private void sendCameraStatus(int error) {
        synchronized (mSyncObject) {
            if (mCameraOwner != null) {
                sendCameraStatus(mCameraOwner, error);
            }
        }
    }

    @Override
    public void onError(Camera camera, int error) {
        log("CameraFailed: cameraId=" + camera.getId() + " Error=" + error);
        sendCameraStatus(error);
    }

    @Override
    public void onDisconnected(ImsCallSessionImpl session) {
    }

    @Override
    public void onClosed(ImsCallSessionImpl session) {
        log("onClosed: Session=" + getSessionInfo(session));
        close(session);
    }

    @Override
    public void onUnsolCallModify(ImsCallSessionImpl session, CallModify callModify) {
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
