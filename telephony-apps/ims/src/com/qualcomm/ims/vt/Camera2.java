/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import com.qualcomm.ims.vt.Camera.ConfigIms;

import android.content.Context;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraCharacteristics.Key;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.util.Range;
import android.util.Log;

/**
 * Wrapper class for Camera2.
 *
 */
class Camera2 extends Camera {
    private static final String TAG = "VideoCall_Camera2";
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

    private CameraManager mCameraManager;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCameraSession;
    private Builder mCaptureBuilder;

    private Surface mPreviewSurface;
    private Surface mRecordingSurface;

    private int mOpenState;
    private boolean mIsPreviewStarted;
    private boolean mIsRecordingStarted;
    private boolean mIsCreateSessionPending;

    private Rect mCropRegion;
    private Range<Integer> mFps;

    private int mPendingStartRequest;
    private int mSessionId;

    public static final int CAMERA_STATE_CLOSED = 0;
    public static final int CAMERA_STATE_OPENING = 1;
    public static final int CAMERA_STATE_OPEN = 2;

    public static final int CAMERA_REQUEST_START_NONE = 0;
    public static final int CAMERA_REQUEST_START_PREVIEW = 1;
    public static final int CAMERA_REQUEST_START_RECORDING = 2;


    private static final int ZOOM_CONSTANT_NOT_SUPPORTED = 1;
    private static final int GENERIC_CONSTANT_INVALID_VALUE = -1;

    /* package */
    Camera2(Context context, String id, Camera.Listener listener) {
        super(context, id, listener);
        if (DBG) log("Camera2, id=" + id);
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        reset();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void open() throws CameraAccessException {
        if (DBG) log("open");

        List<String> ids = Arrays.asList(mCameraManager.getCameraIdList());
        log("open: Availalbe camera IDs=" + ids);
        if (!ids.contains(mCameraId)) {
            throw new CameraException("Incorrect id, " + mCameraId);
        } else if (!isClosed()) {
            log("open: camere is already opened, state" + mOpenState);
            return;
        }

        try {
            mOpenState = CAMERA_STATE_OPENING;
            mCameraManager.openCamera(mCameraId, mStateListener, null);
        } catch (CameraAccessException e) {
            loge("open: Failed to open camera, id=" + mCameraId + "Exception=" + e);
            mOpenState = CAMERA_STATE_CLOSED;
            throw e;
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() {
        if (DBG) log("close");

        try {
            stopPreview();
        } catch (Exception e) {
            loge("close: Failed to close camera preview/recording, exception=" + e);
        }

        if (mCameraSession != null) {
            mCameraSession.close();
        }

        if (mCameraDevice != null) {
            mCameraDevice.close();
        }
        reset();
    }

    private void reset() {
        if (VDBG) log("reset");

        mCameraDevice = null;
        mOpenState = CAMERA_STATE_CLOSED;
        mIsPreviewStarted = false;
        mIsRecordingStarted = false;
        mPendingStartRequest = CAMERA_REQUEST_START_NONE;
        mPreviewSurface = null;
        mRecordingSurface = null;
        mFps = null;
        mCropRegion = null;
        mCameraSession = null;
        mSessionId = 0;
        mIsCreateSessionPending= false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setZoom(float v) throws CameraAccessException {
        if (DBG) log("setZoom " + v);
        failIfClosed();

        if (v < 1 || v > getMaxZoom() ) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        mCropRegion = computeCropRegion(v);
        if (mCameraSession != null) {
            updateParams(mCaptureBuilder);
            restartRepeatingSession(mCaptureBuilder.build());
        } else {
            log("Setting zoom is deferred.");
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void reconfigure(ConfigIms cfg) throws CameraAccessException {
        if (DBG) log("reconfigure " + cfg);
        failIfClosed();

        mFps = computeBestFps(cfg.fps);

        if (mCameraSession != null) {
            updateParams(mCaptureBuilder);
            restartRepeatingSession(mCaptureBuilder.build());
        } else {
            log("Reconfiguration is deferred.");
        }
    }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isZoomSupported() throws CameraAccessException {
            // Note: ImsCamera has to be open to query for zoom.
            // However, it is not required for Camera2. To be consistent
            // we either need to enforce the same for camera2 (throw an exception),
            // or we should update ImsCamera to open (if it's not already open) and
            // and then query for zoom.
            // TODO Check if UI opens the camera before querying for camera capabilities.
            return getMaxZoom() > ZOOM_CONSTANT_NOT_SUPPORTED;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public float getMaxZoom() throws CameraAccessException {
            try {
                return getCameraCharacteristic(mCameraId,
                        CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
            } catch (CameraAccessException e) {
                loge("getMaxZoom: Failed to retrieve Max Zoom, " + e);
                throw e;
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Size getPreviewSize() throws CameraAccessException {
            StreamConfigurationMap map = getCameraCharacteristic(mCameraId,
                    CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            return map.getOutputSizes(SurfaceTexture.class)[0];
        }

    /**
     * {@inheritDoc}
     */
    @Override
    public void startPreview(Surface surface) throws CameraAccessException {
        if (DBG) log("startPreview, surface=" + surface);

        if (isPreviewStarted()) {
            log("startPreview: Preview has already started.");
            return;
        }

        failIfClosed();
        failIfSurfaceNull(surface, "Error preview surface is null");


        mPreviewSurface = surface;
        mIsPreviewStarted = true;
        mSessionId = genSessionId();

        if (shallDefer()) {
            log("startPreview: Deferring startPreview request, currRequest=" + mPendingStartRequest);
            mPendingStartRequest = CAMERA_REQUEST_START_PREVIEW;
        } else {
            doStartPreview();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void stopPreview() throws CameraAccessException {
        if (DBG) log("stopPreview");

        mIsPreviewStarted = mIsRecordingStarted = false;
        clearStartRequest();
        cancelSession();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void startRecording(Surface previewSurface, Surface recordingSurface)
            throws CameraAccessException {
        if (DBG) log("startRecording: PreviewSurface=" + previewSurface + " RecordingSurface="
                    + recordingSurface);

        if (isRecordingStarted()) {
            log("startRecording: Recording has already started.");
            return;
        }

        failIfClosed();
        failIfSurfaceNull(previewSurface, "Error preview surface is null");
        failIfSurfaceNull(recordingSurface, "Error recording surface is null");


        mRecordingSurface = recordingSurface;
        mPreviewSurface = previewSurface;
        mIsRecordingStarted = mIsPreviewStarted = true;
        mSessionId = genSessionId();

        if (shallDefer()) {
            log("startRecording: Deferring startRecording request, currRequest=" + mPendingStartRequest);
            mPendingStartRequest = CAMERA_REQUEST_START_RECORDING;
        } else {
            doStartRecording();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void stopRecording() throws CameraAccessException {
        mIsRecordingStarted = false;
        if (mIsPreviewStarted) {
            startPreview(mPreviewSurface);
        } else {
            cancelSession();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isOpen() {
        return mOpenState != CAMERA_STATE_CLOSED;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isPreviewStarted() {
        return mIsPreviewStarted;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isRecordingStarted() {
        return mIsRecordingStarted;
    }

    /**
     * Sets output FPS.
     * @param fps New FPS value.
     * @throws CameraAccessException
     */
    private android.util.Range<Integer> computeBestFps(int v) throws CameraAccessException {
        if (DBG) log("computeBestFps " + v);

        android.util.Range<Integer>[] fpsRange = getCameraCharacteristic(mCameraId,
                CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
        if (DBG) log("computeBestFps: Available FPS Ranges: " + fpsRange);
        if (fpsRange == null || fpsRange.length < 1) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        android.util.Range<Integer> bestFps = fpsRange[0];
        int minDelta = Math.abs(v - bestFps.getUpper());
        for (android.util.Range<Integer> curr : fpsRange) {
            int delta = Math.abs(v - curr.getUpper());
            if (delta < minDelta) {
                bestFps = curr;
            }
        }

        return bestFps;
    }

    private void updateParams(CaptureRequest.Builder builder) {
        builder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);

        if (mFps!=null) {
            if (VDBG) log("Setting FPS_RANGE=" + mFps);
            builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, mFps);
        }

        if (mCropRegion != null) {
            if (VDBG) log("Setting CROP_REGION=" + mCropRegion);
            builder.set(CaptureRequest.SCALER_CROP_REGION, mCropRegion);
        }
    }

    private android.graphics.Rect computeCropRegion(float v) throws CameraAccessException {
        android.graphics.Rect rect = getCameraCharacteristic(mCameraId,
                CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        if (VDBG) log("computeCropRegion: ActiveArraySize=" + rect);
        float dx = rect.width() / (2 * v), dy = rect.height() / (2 * v);
        rect.inset((int) Math.floor(dx), (int) Math.floor(dy));
        if (VDBG) log("computeCropRegion: CropRegion=" + rect);
        return rect;
    }

    private void exectutePendingRequests() {
        if (DBG) log("exectutePendingRequests: request=" + mPendingStartRequest);

        try {
            switch (mPendingStartRequest) {
                case CAMERA_REQUEST_START_PREVIEW:
                    doStartPreview();
                    break;
                case CAMERA_REQUEST_START_RECORDING:
                    doStartRecording();
                    break;
            }
        } catch (CameraAccessException e) {
            // TODO Notify
        }
    }


    private <T> T getCameraCharacteristic(String cameraId, Key<T> key)
            throws CameraAccessException {
        CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
        return characteristics.get(key);
    }

    /**
     * {@link CameraDevice.StateListener} is called when {@link CameraDevice} changes its status.
     */
    private CameraDevice.StateListener mStateListener = new CameraDevice.StateListener() {

        @Override
        public void onOpened(CameraDevice cameraDevice) {
            log("onOpened: CameraDevice is opened, id=" + getId());
            mCameraDevice = cameraDevice;
            mOpenState = CAMERA_STATE_OPEN;
            exectutePendingRequests();
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            log("onDisconnected: CameraDevice is disconnected, id=" + getId());
            close();
        }

        @Override
        public void onError(CameraDevice cameraDevice, int error) {
            log("onError: CameraDevice is disconnected, id=" + getId() + "error=" + error);
            close();
        }

    };

    private void abortCaptures() {
        log("abortCaptures: Aborting captures..., id=" + getId());

        try {
            if (mCameraSession != null) {
                mCameraSession.abortCaptures();
                mCameraSession = null;
            }
        } catch (CameraAccessException e) {
            loge("abortCaptures: Failed to abort capture, " + e);
            close();
            // TODO Rethrow
        }
    }

    private void cancelSession() {
        log("cancelSession: Canceling session..., id=" + getId());

        try {
            mSessionId = genSessionId();
            if (mCameraSession != null) {
                mCameraSession.stopRepeating();
                mCameraSession = null;
            }
        } catch (CameraAccessException e) {
            loge("stopSession: Failed to stop repeating session, " + e);
            close();
            // TODO Rethrow
        }
    }

    private void startRepeatingSession(CaptureRequest request) {
        log("startRepeatingSession: Starting session..., id=" + getId());

        try {
            if (mCameraSession != null) {
                loge("startRepeatingSession: session=" + mCameraSession);
                mCameraSession.setRepeatingRequest(request, null, null);
            } else {
                loge("startRepeatingSession: Session is null.");
            }
        } catch (CameraAccessException e) {
            loge("startRepeatingSession: Failed to start repeating session, " + e);
            close();
            // TODO Rethrow
        }
    }

    private void restartRepeatingSession(CaptureRequest request) {
        log("restartRepeatingSession: Re-starting session... id=" + getId());

        try {
            if (mCameraSession != null) {
                log("restartRepeatingSession: Stoping repating session...");
                mCameraSession.stopRepeating();
                log("restartRepeatingSession: Starting repating session...");
                mCameraSession.setRepeatingRequest(request, null, null);
            } else {
                loge("restartRepeatingSession: Session is null.");
            }
        } catch (CameraAccessException e) {
            loge("restartRepeatingSession: Failed to start restart repeating session Ex=, " + e);
            close();
            // TODO Rethrow
        }
    }

    private void doStartPreview() throws CameraAccessException {
        if (DBG) log("doStartPreview, surface=" + mPreviewSurface);

        failIfSurfaceNull(mPreviewSurface, "Error preview surface is null");

        try {
            clearStartRequest();
            mCaptureBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mCaptureBuilder.addTarget(mPreviewSurface);

            final List<Surface> surfaces = Arrays.asList(mPreviewSurface);

            mCameraSession = null;
            doCreateCaptureSession(surfaces, new CaptureSessionListener(mSessionId));
        } catch (CameraAccessException e) {
            loge("startPreview: Failed to start preview, " + e);
            close();
        }
    }

    private void doStartRecording() throws CameraAccessException {
        if (DBG) log("doStartRecording: Surface=" + mRecordingSurface);

        failIfSurfaceNull(mPreviewSurface, "Error preview surface is null");
        failIfSurfaceNull(mRecordingSurface, "Error recording surface is null");

        try {
            clearStartRequest();
            mCaptureBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
            mCaptureBuilder.addTarget(mRecordingSurface);
            mCaptureBuilder.addTarget(mPreviewSurface);

            final List<Surface> surfaces = Arrays.asList(mRecordingSurface, mPreviewSurface);

            mCameraSession = null;
            doCreateCaptureSession(surfaces, new CaptureSessionListener(mSessionId));
        } catch (Exception e) {
            loge("startRecording: Failed to start recording, " + e);
            close();
            // TODO: Notify...
        }
    }

    private void clearStartRequest() {
        if (DBG) log("clearStartRequest: request=" + mPendingStartRequest);
        mPendingStartRequest = CAMERA_REQUEST_START_NONE;
    }

    private void stopRepeatingSession() {
        if (DBG) log("stopRepeatingSession: session=" + mCameraSession);

        try {
            if (mCameraSession != null) {
                mCameraSession.stopRepeating();
            }
        } catch (CameraAccessException e) {
            loge("stopSession: Failed to stop session, " + e);
            close();
            // TODO Rethrow
        }
    }

    private boolean shallDefer() {
        return mCameraDevice==null || mIsCreateSessionPending;
    }

    private void doCreateCaptureSession(List<Surface> o, CameraCaptureSession.StateListener l)
            throws CameraAccessException {
        if (mIsCreateSessionPending) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR,
                    "Only one session request is allowed.");
        }
        if (mCameraDevice==null) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR,
                    "Camera device is not ready (null).");
        }

        mIsCreateSessionPending = true;
        mCameraDevice.createCaptureSession(o, l, null);
    }

    private boolean isOpened() {
        return mOpenState == CAMERA_STATE_OPEN;
    }

    private class CaptureSessionListener extends CameraCaptureSession.StateListener {
        private int id;

        public CaptureSessionListener(int v) {
            id = v;
        }

        @Override
        public void onConfigured(CameraCaptureSession session) {
            log("onConfigured: SessionId=" + id + " GlobalSessionId=" +mSessionId);
            mIsCreateSessionPending = false;
            if (!isRequestPending()) {
                if (id == mSessionId) Camera2.this.onConfigured(session);
            } else {
                exectutePendingRequests();
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            log("onConfigureFailed: SessionId=" + id + " GlobalSessionId=" + mSessionId);
            mIsCreateSessionPending = false;
            if (!isRequestPending()) {
                if (id == mSessionId) Camera2.this.onConfigureFailed(session);
            } else {
                exectutePendingRequests();
            }
        }
    }

    private void onConfigured(CameraCaptureSession session) {
        log("onConfigured: session=" + session);
        mCameraSession = session;

        if (mCaptureBuilder != null) {
            updateParams(mCaptureBuilder);
            startRepeatingSession(mCaptureBuilder.build());
        } else {
            loge("onConfigured: Builder is null.");
        }
    }

    private void onConfigureFailed(CameraCaptureSession session) {
        loge("onConfigureFailed: onConfigureFailed.");
        close();
    }

    private boolean isRequestPending() {
        return mPendingStartRequest != CAMERA_REQUEST_START_NONE;
    }

    private int genSessionId() {
        if (++mSessionId == Integer.MAX_VALUE) {
            mSessionId = 0;
        }
        if (DBG) log("generateSessionId: SessionId=" + mSessionId);
        return mSessionId;
    }

    private boolean isOpening() {
        return mOpenState == CAMERA_STATE_OPENING;
    }

    private boolean isClosed() {
        return mOpenState == CAMERA_STATE_CLOSED;
    }

    private void failIfClosed() throws CameraAccessException {
        if (!isClosed()) {return;}

        log("failIfClosed: Camera is closed...no operation is allowed.");
        throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
    }

    private void failIfSurfaceNull(Surface surface, String msg) throws CameraAccessException {
        if (surface!=null) {return;}

        log("failIfSurfaceNull: " + msg);
        throw new CameraAccessException(CameraAccessException.CAMERA_ERROR, msg);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
