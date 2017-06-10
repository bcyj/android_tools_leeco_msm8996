/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.view.Surface;
import android.view.WindowManager;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraCharacteristics.Key;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.util.Log;
import android.util.Size;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in
 * {@code android.hardware.Camera}.
 */

public class ImsCamera extends Camera {
    private static final String TAG = "VideoCall_ImsCamera";
    private static final boolean DBG = true;
    private static final short IMS_CAMERA_OPERATION_SUCCESS = 0;
    private static final Size INVALID_SIZE = new Size(-1, -1);

    static {
        System.loadLibrary("imscamera_jni");
    }

    private String mPackageName;
    private WindowManager mWindowManager;
    private CameraManager mCameraManager;
    private boolean mIsOpen;
    private boolean mIsPreviewStarted;
    private boolean mIsRecordingStarted;
    private Surface mPreviewSurface;

    // @deprecated Use overloaded variant and explicitly pass the package name.
    public static native short native_open(int cameraId);
    public static native short native_open(int cameraId, String packageName);
    public native short native_release();
    public native short native_startPreview();
    public native short native_stopPreview();
    public native short native_startRecording();
    public native short native_stopRecording();
    public native short native_setPreviewTexture(Surface surface);
    public native short native_setDisplayOrientation(int rotation);
    public native boolean native_isZoomSupported();
    public native int native_getMaxZoom();
    public native void native_setZoom(int zoomValue);
    public native short native_setPreviewSize(int width, int height);
    public native short native_setPreviewFpsRange(short fps);

    /* package */
    ImsCamera(Context context, String id, Camera.Listener listener) {
        super(context, id, listener);
        mPackageName = context.getPackageName();
        // TODO Maybe make this static.
        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);

        mIsOpen = false;
        mIsPreviewStarted = false;
        mIsRecordingStarted = false;
        mPreviewSurface = null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Size getPreviewSize() throws CameraAccessException {
        // Eventhough ImsCamera implementation should not access Camera2 APIs,
        // we query some of camera characteristics using Camera2 APIs.
        // Camera folks confirmed that this is OK, since all characteristics are
        // cached when the camera service comes up, so this won't really make any
        // access to camera.
        StreamConfigurationMap map = getCameraCharacteristic(mCameraId,
                CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        return map.getOutputSizes(SurfaceTexture.class)[0];
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void open() throws CameraAccessException {
        if (DBG) log("open");
        if (isOpen()) {
            log("open: Camera is already open.");
            return;
        }

        final int id = Integer.parseInt(getId());
        final short error = native_open(id, mPackageName);
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            loge("open: error=" + error);
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }
        mIsOpen = true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() {
        if (DBG) log("close");
        if (!isOpen()) {
            log("close: Camera is already closed.");
            return;
        }

        try {
            stopPreview();
        } catch (Exception e) {
            loge("close: Failed to close camera preview/recording, exception=" + e);
        }

        short error = native_release();
        logIfError("release", error);

        mIsOpen = false;
        mIsPreviewStarted = false;
        mIsRecordingStarted = false;
        mPreviewSurface = null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setZoom(float v) throws CameraAccessException {
        if (DBG) log("setZoom " + v);
        if (!isOpen()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        native_setZoom((int) v);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void reconfigure(ConfigIms cfg) throws CameraAccessException {
        if (DBG) log("reconfigure " + cfg);
        if (!isOpen()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        setPreviewFps(cfg.fps);
        setFrameDimension(cfg.width, cfg.height);
    }

    /**
     * Sets output (recording) frames dimension.
     * @param w Width of the frame.
     * @param h Height of the frame.
     * @throws CameraAccessException
     */
    private void setFrameDimension(int w, int h) throws CameraAccessException {
        if (DBG) log("setPreviewSize");
        if (!isOpen() || isPreviewStarted() || isRecordingStarted()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        short error = native_setPreviewSize(w, h);
        logIfError("setPreviewSize", error);
    }

    /**
     * Sets output FPS.
     * @param fps New FPS value.
     * @throws CameraAccessException
     */
    private void setPreviewFps(int fps) throws CameraAccessException {
        if (DBG) log("setPreviewFps");
        if (!isOpen() || isPreviewStarted() || isRecordingStarted()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        short error = native_setPreviewFpsRange((short)fps);
        logIfError("setPreviewFpsRange", error);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isZoomSupported() throws CameraAccessException {
        if (!isOpen()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        boolean result = native_isZoomSupported();
        if (DBG) log("isZoomSupported result=" + result);
        return result;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public float getMaxZoom() throws CameraAccessException {
        if (!isOpen()) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        int result = native_getMaxZoom();
        if (DBG) log("getMaxZoom result = " + result);
        return result;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void startPreview(Surface surface) throws CameraAccessException {
        if (DBG) log("startPreview: Surface=" + surface);
        if (!isOpen()) {
            log("startPreview: Error camera is closed");
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        } else if (isPreviewStarted()) {
            log("startPreview: Camera preview already started.");
            return;
        }

        mPreviewSurface = surface;
        short error = native_setPreviewTexture(surface);
        logIfError("setPreviewTexture", error);

        if (error == IMS_CAMERA_OPERATION_SUCCESS) {
            error = native_startPreview();
            logIfError("startPreview", error);
        }
        setDisplayOrientation();

        if (error == IMS_CAMERA_OPERATION_SUCCESS) {
            mIsPreviewStarted = true;
        } else {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }
    }

    private void doStopPreview() throws CameraAccessException {
        if (DBG) log("doStopPreview");
        short error = native_stopPreview();
        logIfError("doStopPreview", error);
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }
        mIsPreviewStarted = false;
    }

    private void doStopRecording() throws CameraAccessException {
        if (DBG) log("doStopRecording");
        short error = native_stopRecording();
        logIfError("doStopRecording", error);
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }
        mIsRecordingStarted = false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void stopPreview() throws CameraAccessException {
        if (!isPreviewStarted()) {
            log("stopPreview: Camera preview already stopped.");
            return;
        }

        if (DBG) log("stopPreview");

        if (isRecordingStarted()) {
            doStopRecording();
        }
        doStopPreview();
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
            log("startRecording: Camera recording already started.");
            return;
        }

        mPreviewSurface = previewSurface;
        if (mPreviewSurface == null) {
            log("startRecording: Preview surface is null.");
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }

        if (!isPreviewStarted()) {
            startPreview(mPreviewSurface);
        }

        if (DBG) log("startRecording");
        short error = native_startRecording();
        logIfError("startRecording", error);
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR);
        }
        mIsRecordingStarted = true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void stopRecording() throws CameraAccessException {
        if (!isRecordingStarted()) {
            log("stopRecording: Camera recording already stopped.");
            return;
        }

        if (DBG) log("stopRecording");
        doStopRecording();
        if (isPreviewStarted()) {
            doStopPreview();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isOpen() {
        return mIsOpen;
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

    private <T> T getCameraCharacteristic(String cameraId, Key<T> key)
            throws CameraAccessException {
        CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
        return characteristics.get(key);
    }

    /**
     * Set the camera display orientation based on the screen rotation and the camera direction
     */
    private void setDisplayOrientation() {
        if (mWindowManager == null) {
            loge("WindowManager not available");
            return;
        }

        android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
        int result = 0, degrees = 0, rotation = 0;

        // We assume that the device will always be in it's native orientation.
        // The actual rotation is done at UI side. This way we avoid stoping and starting camera
        // preview and recording everytime UI gets rotated.
        rotation = Surface.ROTATION_0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
            default:
                loge("setDisplayOrientation: Unexpected rotation: " + rotation);
        }

        final int id = Integer.parseInt(getId());
        android.hardware.Camera.getCameraInfo(id, info);
        if (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }

        if (DBG) log("setDisplayOrientation rotation=" + result);
        short error = native_setDisplayOrientation(result);
        logIfError("setDisplayOrientation", error);
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }

    private void loge(String msg) {
        Log.e(TAG, msg);
    }

    private void logIfError(String methodName, short error) {
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            loge(methodName + " failed with error=" + error);
        }
    }
}
