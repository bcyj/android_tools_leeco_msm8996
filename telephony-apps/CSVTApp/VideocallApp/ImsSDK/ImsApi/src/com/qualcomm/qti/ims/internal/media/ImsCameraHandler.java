/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.media;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.view.WindowManager;

import java.io.IOException;
import java.util.List;

import com.qualcomm.qti.ims.internal.CameraHandlerBase;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in
 * {@code android.hardware.Camera}.
 */

public class ImsCameraHandler extends CameraHandlerBase {
    public static final int CAMERA_UNKNOWN = -1;
    private static final String TAG = "VideoCallCameraManager";
    private static final boolean DBG = true;
    private ImsCamera mCameraDevice;
    private int mNumberOfCameras;
    private int mCameraId = CAMERA_UNKNOWN; // current camera id
    private int mBackCameraId = CAMERA_UNKNOWN,
            mFrontCameraId = CAMERA_UNKNOWN;
    private CameraInfo[] mInfo;
    private CameraState mCameraState = CameraState.CAMERA_CLOSED;
    private Context mContext;

    // Use a singleton.
    private static ImsCameraHandler mInstance;

    // Check if device policy has disabled the camera.
    DevicePolicyManager mDpm;
    // Get display rotation
    WindowManager mWindowManager;

    /**
     * This method returns the single instance of CameraManager object
     *
     * @param mContext
     */
    public static synchronized ImsCameraHandler getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new ImsCameraHandler(context);
        }
        return mInstance;
    }

    /**
     * Private constructor for CameraManager
     *
     * @param mContext
     */
    private ImsCameraHandler(Context context) {
        mContext = context;
        mNumberOfCameras = android.hardware.Camera.getNumberOfCameras();
        log("Number of cameras supported is: " + mNumberOfCameras);
        mInfo = new CameraInfo[mNumberOfCameras];
        for (int i = 0; i < mNumberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            android.hardware.Camera.getCameraInfo(i, mInfo[i]);
            if (mBackCameraId == CAMERA_UNKNOWN
                    && mInfo[i].facing == CameraInfo.CAMERA_FACING_BACK) {
                mBackCameraId = i;
                log("Back camera ID is: " + mBackCameraId);
            }
            if (mFrontCameraId == CAMERA_UNKNOWN
                    && mInfo[i].facing == CameraInfo.CAMERA_FACING_FRONT) {
                mFrontCameraId = i;
                log("Front camera ID is: " + mFrontCameraId);
            }
        }
        mDpm = (DevicePolicyManager) mContext
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
        // Get display rotation
        mWindowManager = (WindowManager) mContext
                .getSystemService(Context.WINDOW_SERVICE);
    }

    /**
     * Return the number of cameras supported by the device
     *
     * @return number of cameras
     */
    public int getNumberOfCameras() {
        return mNumberOfCameras;
    }

    /**
     * Open the camera hardware
     *
     * @param cameraId
     *            front or the back camera to open
     * @return true if the camera was opened successfully
     * @throws Exception
     */
    public synchronized boolean open(int cameraId) throws Exception {
        if (mDpm == null) {
            throw new Exception("DevicePolicyManager not available");
        }

        if (mDpm.getCameraDisabled(null)) {
            throw new Exception("Camera is disabled");
        }

        if (mCameraDevice != null && mCameraId != cameraId) {
            mCameraDevice.release();
            mCameraDevice = null;
            mCameraId = CAMERA_UNKNOWN;
        }
        if (mCameraDevice == null) {
            try {
                if (DBG)
                    log("opening camera " + cameraId);
                mCameraDevice = ImsCamera.open(cameraId,
                        mContext.getPackageName());
                mCameraId = cameraId;
            } catch (Exception e) {
                loge("fail to connect Camera" + e);
                throw e;
            }
        }
        mCameraState = CameraState.PREVIEW_STOPPED;
        return true;
    }

    /**
     * Start the camera preview if camera was opened previously
     *
     * @param mSurfaceTexture
     *            Surface on which to draw the camera preview
     * @throws IOException
     */
    public void startPreview(SurfaceTexture mSurfaceTexture) {
        if (mCameraState != CameraState.PREVIEW_STOPPED) {
            loge("startPreview: Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }
        if (mCameraDevice != null) {
            if (DBG)
                log("starting preview");

            // Set the SurfaceTexture to be used for preview
            mCameraDevice.setPreviewTexture(mSurfaceTexture);

            setDisplayOrientation();
            mCameraDevice.startPreview();
            mCameraState = CameraState.PREVIEW_STARTED;
        }
    }

    /**
     * Close the camera hardware if the camera was opened previously
     */
    public synchronized void close() {
        if (mCameraState == CameraState.CAMERA_CLOSED) {
            loge("close: Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }

        if (mCameraDevice != null) {
            if (DBG)
                log("closing camera");
            if (mCameraState == CameraState.PREVIEW_STARTED) {
                mCameraDevice.stopPreview();
            }
            mCameraDevice.release();
        }
        mCameraDevice = null;
        mCameraId = CAMERA_UNKNOWN;
        mCameraState = CameraState.CAMERA_CLOSED;
    }

    /**
     * Stop the camera preview if the camera is open and the preview is not
     * already started
     */
    public void stopPreview() {
        if (mCameraState != CameraState.PREVIEW_STARTED) {
            loge("stopPreview: Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }
        if (mCameraDevice != null) {
            if (DBG)
                log("stopping preview");
            mCameraDevice.stopPreview();
        }
        mCameraState = CameraState.PREVIEW_STOPPED;
    }

    public void startCameraRecording() {
        if (mCameraDevice != null
                && mCameraState == CameraState.PREVIEW_STARTED) {
            mCameraDevice.startRecording();
        }
    }

    public void stopCameraRecording() {
        if (mCameraDevice != null) {
            mCameraDevice.stopRecording();
        }
    }

    /**
     * Get the camera ID for the back camera
     *
     * @return camera ID
     */
    public int getBackCameraId() {
        return mBackCameraId;
    }

    /**
     * Get the camera ID for the front camera
     *
     * @return camera ID
     */
    public int getFrontCameraId() {
        return mFrontCameraId;
    }

    /**
     * Return the current camera state
     *
     * @return current state of the camera state machine
     */
    public CameraState getCameraState() {
        return mCameraState;
    }

    /**
     * Set the display texture for the camera
     *
     * @param surfaceTexture
     */
    public void setDisplay(SurfaceTexture surfaceTexture) {
        // Set the SurfaceTexture to be used for preview
        if (mCameraDevice == null)
            return;
        mCameraDevice.setPreviewTexture(surfaceTexture);

    }

    /**
     * Set the texture view for the camera
     *
     * @param textureView
     */
    public void setDisplay(TextureView textureView) {
        setDisplay(textureView.getSurfaceTexture());
    }

    /**
     * Returns the direction of the currently open camera
     *
     * @return one of the following possible values -
     *         CameraInfo.CAMERA_FACING_FRONT - CameraInfo.CAMERA_FACING_BACK -
     *         CAMERA_UNKNOWN - No Camera active
     */
    public int getCameraDirection() {
        if (mCameraDevice == null)
            return CAMERA_UNKNOWN;

        return (mCameraId == mFrontCameraId) ? CameraInfo.CAMERA_FACING_FRONT
                : CameraInfo.CAMERA_FACING_BACK;
    }

    /**
     * Set the camera display orientation based on the screen rotation and the
     * camera direction
     */
    public void setDisplayOrientation() {
        android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
        int result;
        int degrees = 0;
        int rotation = 0;

        if (mWindowManager == null) {
            loge("WindowManager not available");
            return;
        }

        rotation = mWindowManager.getDefaultDisplay().getRotation();
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

        android.hardware.Camera.getCameraInfo(mCameraId, info);
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        mCameraDevice.setDisplayOrientation(result);
    }

    public ImsCamera getImsCameraInstance() {
        return mCameraDevice;
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }

    private void loge(String msg) {
        Log.e(TAG, msg);
    }

    @Override
    public void getCameraParameters() {
        // TODO Auto-generated method stub

    }

    @Override
    public void getLastFrame() {
        // TODO Auto-generated method stub

    }

    @Override
    public void setCameraFMT() {
        // TODO Auto-generated method stub

    }

    @Override
    public void setCameraParameters(Parameters param) {
        // TODO Auto-generated method stub

    }
}
