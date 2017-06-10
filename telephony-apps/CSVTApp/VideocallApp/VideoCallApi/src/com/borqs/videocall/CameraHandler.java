/*Copyright (c) 2013 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.borqs.videocall;

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
import android.os.SystemProperties;
import java.lang.String;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in
 * {@code android.hardware.Camera}.
 */

public class CameraHandler implements Camera.PreviewCallback {
    private static final String TAG = "VideoCallCameraManager";
    private static final boolean DBG = true;
    private android.hardware.Camera mCameraDevice;
    private int mNumberOfCameras;
    private int mCameraId = -1; // current camera id
    private int mBackCameraId = -1, mFrontCameraId = -1;
    private CameraInfo[] mInfo;
    private CameraState mCameraState = CameraState.CAMERA_CLOSED;
    private Parameters mParameters;
    private Context mContext;

    // Use a singleton.
    private static CameraHandler sHolder;
    private static byte[] mLastFrame;
    private static byte[] mLastFrameProcessed;
    private static boolean sendMess = false;
    private static int mCount = 0;

    /**
     * Enum that defines the various camera states
     */
    public enum CameraState {
        CAMERA_CLOSED, // Camera is not yet opened or is closed
        PREVIEW_STOPPED, // Camera is open and preview not started
        PREVIEW_STARTED, // Preview is active
    };

    /**
     * This method returns the single instance of CameraManager object
     *
     * @param mContext
     */
    public static synchronized CameraHandler getInstance(Context context) {
        if (sHolder == null) {
            sHolder = new CameraHandler(context);
        }
        return sHolder;
    }

    /**
     * Private constructor for CameraManager
     *
     * @param mContext
     */
    private CameraHandler(Context context) {
        mContext = context;
        mLastFrame = null;
        mNumberOfCameras = android.hardware.Camera.getNumberOfCameras();
        mInfo = new CameraInfo[mNumberOfCameras];
        for (int i = 0; i < mNumberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            android.hardware.Camera.getCameraInfo(i, mInfo[i]);
            if (mBackCameraId == -1 && mInfo[i].facing == CameraInfo.CAMERA_FACING_BACK) {
                mBackCameraId = i;
            }
            if (mFrontCameraId == -1 && mInfo[i].facing == CameraInfo.CAMERA_FACING_FRONT) {
                mFrontCameraId = i;
            }
        }
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
        // Check if device policy has disabled the camera.
        DevicePolicyManager dpm = (DevicePolicyManager) mContext
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm == null) {
            throw new Exception("DevicePolicyManager not available");
        }

        if (dpm.getCameraDisabled(null)) {
            throw new Exception("Camera is diabled");
        }

        if (mCameraDevice != null && mCameraId != cameraId) {
            mCameraDevice.release();
            mCameraDevice = null;
            mCameraId = -1;
        }
        if (mCameraDevice == null) {
            try {
                if (DBG)
                    log("opening camera " + cameraId);
                mCameraDevice = android.hardware.Camera.open(cameraId);
                mCameraId = cameraId;
            } catch (RuntimeException e) {
                loge("fail to connect Camera" + e);
                throw new Exception(e);
            }
            mParameters = mCameraDevice.getParameters();
        } else {
            try {
                mCameraDevice.reconnect();
            } catch (IOException e) {
                loge("reconnect failed.");
                throw new Exception(e);
            }
            mCameraDevice.setParameters(mParameters);
        }
        mCameraState = CameraState.PREVIEW_STOPPED;
        mCameraDevice.setPreviewCallback(this);
        return true;
    }

    /**
     * Start the camera preview if camera was opened previously
     *
     * @param mSurfaceTexture
     *            Surface on which to draw the camera preview
     * @throws IOException
     */
    public void startPreview(SurfaceTexture mSurfaceTexture) throws IOException {
        if (mCameraState != CameraState.PREVIEW_STOPPED) {
            loge("Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }
        if (DBG)
            log("starting preview");

        // Set the SurfaceTexture to be used for preview
        mCameraDevice.setPreviewTexture(mSurfaceTexture);

        setDisplayOrientation();
        mCameraDevice.startPreview();
        mCameraState = CameraState.PREVIEW_STARTED;
    }

    /**
     * Close the camera hardware if the camera was opened previously
     */
    public synchronized void close() {
        if (mCameraState == CameraState.CAMERA_CLOSED) {
            loge("Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }

        if (DBG)
            log("closing camera");
        mCameraDevice.stopPreview(); // Stop preview
        mCameraDevice.release();
        mCameraDevice = null;
        mParameters = null;
        mCameraId = -1;
        mCameraState = CameraState.CAMERA_CLOSED;
    }

    /**
     * Stop the camera preview if the camera is open and the preview is not
     * already started
     */
    public void stopPreview() {
        if (mCameraState != CameraState.PREVIEW_STARTED) {
            loge("Camera state " + mCameraState
                    + " is not the right camera state for this operation");
            return;
        }
        if (DBG)
            log("stopping preview");
        mCameraDevice.setPreviewCallback(null);
        mCameraDevice.stopPreview();
        mCameraState = CameraState.PREVIEW_STOPPED;
        sendMess = true;
    }

    /**
     * Return the camera parameters that specifies the current settings of the
     * camera
     *
     * @return camera parameters
     */
    public Parameters getCameraParameters() {
        if (mCameraDevice == null) {
            return null;
        }
        return mParameters;
    }

    /**
     * Set the camera parameters
     *
     * @param parameters
     *            to be set
     */
    public void setCameraParameters(Parameters parameters) {
        if (mCameraDevice == null) {
            return;
        }
        mCameraDevice.setParameters(parameters);
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
        try {
            mCameraDevice.setPreviewTexture(surfaceTexture);
        } catch (IOException e) {
            throw new RuntimeException("setPreviewDisplay failed", e);
        }
    }

    /**
     * Set the texture view for the camera
     *
     * @param surfaceTexture
     */
    public void setDisplay(TextureView textureView) {
        // Set the SurfaceTexture to be used for preview
        if (mCameraDevice == null)
            return;
        try {
            mCameraDevice.setPreviewTexture(textureView.getSurfaceTexture());
        } catch (IOException e) {
            throw new RuntimeException("setPreviewDisplay failed", e);
        }
    }

    /**
     * Gets the supported preview sizes.
     *
     * @return a list of Size object. This method will always return a list with
     *         at least one element.
     */
    public List<Size> getSupportedPreviewSizes() {
        if (mCameraDevice == null)
            return null;
        return mCameraDevice.getParameters().getSupportedPreviewSizes();
    }

    /**
     * Returns the direction of the currently open camera
     *
     * @return one of the following possible values -
     *         CameraInfo.CAMERA_FACING_FRONT - CameraInfo.CAMERA_FACING_BACK -
     *         -1 - No Camera active
     */
    public int getCameraDirection() {
        if (mCameraDevice == null)
            return -1;

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

        // Get display rotation
        WindowManager wm = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        if (wm == null) {
            loge("WindowManager not available");
        }

        rotation = wm.getDefaultDisplay().getRotation();
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
        }

        android.hardware.Camera.getCameraInfo(mCameraId, info);
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360 - 90) % 360;
            result = result + 90;
        }
        mCameraDevice.setDisplayOrientation(result);
    }

    /**
     * Called as preview frames are displayed. The frames are passed to IMS DPL
     * layer to be sent to the far end device
     */
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (sendMess) {
            if (mCount > 5) {
                MediaHandler.getVTManager().sendEventToVTService(VTManager.EventType.EVENT_INFO,
                        VTManager.InfoType.CAMERA_FRAME_START, 0, null);
                sendMess = false;
                mCount = 0;
                log(" onPreviewFrame Send CAMERA_FRAME_START to vt Service");
            }
            // wait for 5 frames to avoid flicker
            mCount++;
        }
        if (MediaHandler.canSendPreview()) {
            mLastFrame = data;
            MediaHandler.sendPreviewFrame(data);
        }
    }

    public byte[] getLastFrame() {
        //process the last frame before stop camera;
        int height =144;
        int width =176;
        int half_height = 72;
        int half_width =88;
        byte[] src_luma = mLastFrame;
        //byte[] src_color = mLastFrame[176*144];
        byte[] dst_luma = mLastFrameProcessed;
        //byte[] dst_color = mLastFrameProcessed[176*144];
        int i=0,j=0;
        if(SystemProperties.get("persist.radio.rotate","0").equals("2")){//save the back-camera last frame
            for(i=0;i<height;i++){
                for(j=0;j<width;j++){
                    dst_luma[i*width+j]   = src_luma[j*height+(height-1-i)];
                }
            }
            for(i=0;i<half_height;i++){
                for(j=0;j<half_width;j++){
                    dst_luma[width*height+i*width+2*j]     = src_luma[width*height+j*height+2*(half_height-1-i)];
                    dst_luma[width*height+i*width+2*j+1] =  src_luma[width*height+j*height+2*(half_height-1-i)+1];
                }
            }
        }
        if(SystemProperties.get("persist.radio.rotate","0").equals("1")){// save the front-camera last frame
            for(i=0;i<height;i++){
                for(j=0;j<width;j++){
                    dst_luma[i*width+j]   = src_luma[(width-1-j)*height+i];
                }
            }
            for(i=0;i<half_height;i++){
                for(j=0;j<half_width;j++){
                    dst_luma[width*height+2*j]     = src_luma[width*height+(half_width-1-j)*height+2*i];
                    dst_luma[width*height+2*j+1] = src_luma[width*height+(half_width-1-j)*height+2*i+1];
                }
            }
        }
        return mLastFrameProcessed;

        //return mLastFrame;
    }

    public native void setCameraFMT();

    private native final void native_setup(Object CameraHandler_this) throws IllegalStateException;

    private native final void native_finalize();

    private void log(String msg) {
        Log.d(TAG, msg);
    }

    private void loge(String msg) {
        Log.e(TAG, msg);
    }
}
